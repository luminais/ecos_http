//=============================================================================
// Copyright (c) 2006 Realtek Semiconductor Corporation.	All Rights Reserved.
//
//	Title:
//		UDPserver.c
//	Desc:
//		UDP server : accepts MP commands from the client
//=============================================================================

#ifdef __ECOS
#include <network.h>
#include <unistd.h>
#include <ctype.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __ECOS
#include <sys/param.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#ifdef __ECOS
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#else
#include <linux/wireless.h>
#endif
#include <fcntl.h>

#ifndef WIN32
#define __PACK__			__attribute__ ((packed))
#else
#define __PACK__
#endif


#define MYPORT 9034                    // the port users will be connecting to
#define BUFLEN 1024                      // length of the buffer
#define MP_TX_PACKET 0x8B71
#define MP_BRX_PACKET 0x8B73
#define MP_QUERY_STATS 	0x8B6D
#define RTL8190_IOCTL_WRITE_REG				0x89f3
#define RTL8190_IOCTL_READ_REG				0x89f4
#define MP_CONTIOUS_TX	0x8B66
#define MP_TXPWR_TRACK	0x8B6E
#define MP_QUERY_TSSI	0x8B6F
#define MP_QUERY_THER 0x8B77

#define FLASH_DEVICE_NAME		("/dev/mtd")
#define FLASH_DEVICE_NAME1		("/dev/mtdblock1")
#define HW_SETTING_HEADER_TAG		((char *)"hs")
#define HW_SETTING_OFFSET		0x6000
#define DEFAULT_SETTING_OFFSET		0x8000
#define CURRENT_SETTING_OFFSET		0xc000

#if 1
#define TAG_LEN					2
#define SIGNATURE_LEN			4
#define HW_SETTING_VER			3	// hw setting version
/* Config file header */
typedef struct param_header {
	unsigned char signature[SIGNATURE_LEN];  // Tag + version
	unsigned short len __PACK__;
} PARAM_HEADER_T, *PARAM_HEADER_Tp;
//PARAM_HEADER_T hsHeader;
#endif

#ifdef __ECOS
#define MP_TMP_FILE	"/tmp/MP.txt"
#define usleep(x)	do {} while(0)

extern int run_clicmd(char *command_buf);
extern int create_file(char *name, char *buf, int len);
extern int is_interface_up(const char *ifname);
#endif

#if 0
/* Do checksum and verification for configuration data */
#ifndef WIN32
static inline unsigned char CHECKSUM(unsigned char *data, int len)
#else
__inline unsigned char CHECKSUM(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	sum = ~sum + 1;
	return sum;
}
#ifndef WIN32
static inline int CHECKSUM_OK(unsigned char *data, int len)
#else
__inline int CHECKSUM_OK(unsigned char *data, int len)
#endif
{
	int i;
	unsigned char sum=0;

	for (i=0; i<len; i++)
		sum += data[i];

	if (sum == 0)
		return 1;
	else
		return 0;
}
void get_read_reg_value( FILE *fp, char *buf, int maxlen );
/////////////////////////////////////////////////////////////////////////////////
static int flash_read(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);
	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( read(fh, buf, len) != len)
		ok = 0;

	close(fh);

	return ok;
}


////////////////////////////////////////////////////////////////////////////////
static int flash_write(char *buf, int offset, int len)
{
	int fh;
	int ok=1;

	fh = open(FLASH_DEVICE_NAME, O_RDWR);

	if ( fh == -1 )
		return 0;

	lseek(fh, offset, SEEK_SET);

	if ( write(fh, buf, len) != len)
		ok = 0;

	close(fh);
	sync();

	return ok;
}

int ReadSinguture(void)
{
	int ver;
	char *buff;
	// Read hw setting
	if ( flash_read((char *)&hsHeader, HW_SETTING_OFFSET, sizeof(hsHeader))==0 ) {
		printf("Read hw setting header failed!\n");
		return NULL;
	}

	if ( sscanf(&hsHeader.signature[TAG_LEN], "%02d", &ver) != 1)
		ver = -1;

	if ( memcmp(hsHeader.signature, HW_SETTING_HEADER_TAG, TAG_LEN) || // invalid signatur
		(ver != HW_SETTING_VER)  ) { // length is less than current
		printf("Invalid hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],
			hsHeader.signature[1], ver, hsHeader.len);
		return NULL;
	}
	//printf("hw setting signature or version number [sig=%c%c, ver=%d, len=%d]!\n", hsHeader.signature[0],	hsHeader.signature[1], ver, hsHeader.len);
	buff = calloc(1, hsHeader.len);
	if ( buff == 0 ) {
		printf("Allocate buffer failed!\n");
		return NULL;
	}
	if ( flash_read(buff, HW_SETTING_OFFSET+sizeof(hsHeader), hsHeader.len)==0 ) {
		printf("Read hw setting failed!\n");
		free(buff);
		return NULL;
	}
	if ( !CHECKSUM_OK(buff, hsHeader.len) ) {
		printf("Invalid checksum of hw setting!\n");
		free(buff);
		return NULL;
	}
	//printf("CS=%x\n",buff[hsHeader.len-1]);
}
#endif
/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
static inline int iw_get_ext(int skfd,    /* Socket to the kernel */
           			char *ifname,        	/* Device name */
           			int request,        		/* WE ID */
           			struct iwreq *pwrq)    /* Fixed part of the request */
{
  	strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);	/* Set device name */
  	return(ioctl(skfd, request, pwrq));			/* Do the request */
}

int MP_get_ext(char *ifname, char *buf, unsigned int ext_num)
{
    	int skfd;
    	struct iwreq wrq;

    	skfd = socket(AF_INET, SOCK_DGRAM, 0);
    	wrq.u.data.pointer = (caddr_t)buf;
    	wrq.u.data.length = strlen(buf);

    	if (iw_get_ext(skfd, ifname, ext_num, &wrq) < 0) {
    		printf("MP_get_ext failed\n");
		return -1;
    	}
	
    	close(skfd);
    	return 0;
}
//the value is included in the 2nd line
//and need transfer to decimal from hex (match MP_TEST.exe's format)
void get_read_reg_value( FILE *fp, char *buf, int maxlen )
{
	int cget,value,start;
	unsigned char *p, *p2;

	p=(unsigned char *)strchr( buf, '\n' );
	if(p==NULL) return;
	p2=p;
		
	value=0;start=0;
	while( (cget=fgetc(fp))!=EOF )
	{
		//printf( "get=%c\n", cget );
		if( (cget=='\n') || (cget==' ') )
		{
			if(start) p2 += sprintf((char *)p2, "%d ", value );
			//printf( "start=%d, value=%d, buf=(%s)\n", start, value, buf );
			value=0;
			start=0;			
		}else if( isxdigit(cget) )
		{
			start=1;
			//printf( "value=%d,", value );
			if( cget>='0' && cget<='9' )
				value=value*16+(cget-'0');
			else if( cget>='a' && cget<='f' )
				value=value*16+(10+cget-'a');
			else if( cget>='A' && cget<='F' )
				value=value*16+(10+cget-'A');
			//printf( "new value=%d\n", value );
		}else{
			//error
			//sprintf( p, "\n", value );
			sprintf((char *)p, "\n");
			return;
		}
	}
	*p2=0;
}

#ifdef __ECOS
void rtk_system(char *cmdbuf, char *grepstr, char *log_filename)
{
	int     logfd;
	int     savefd;
	int     fd1;

	if (log_filename) {
		unlink(log_filename);
		logfd = open(log_filename, O_WRONLY|O_CREAT);
		if (logfd < 0) {
			printf("open error\n");
			exit(-1);
		}
	
		//duplicate stdout file descriptor
		savefd = dup(STDOUT_FILENO);
		if (savefd < 0) {
			printf("err in dup\n");
		}
		//redirect stdout to logfd
		fd1 = dup2(logfd, STDOUT_FILENO);
		if (fd1 < 0) {
			printf("err in dup2\n");
		}
		close(logfd);
	}

	//fprintf(stderr, "%s %d cmdbuf=%s\n", __FUNCTION__,__LINE__, cmdbuf);
	run_clicmd(cmdbuf);

	if (log_filename) {
		//recover stdout
		if (dup2(savefd, STDOUT_FILENO) < 0) {
			printf("err2 in dup2\n");
		}
		close(savefd);
	}
	
	if (grepstr) {
		int i;
		FILE *fp = NULL;
		char buf[BUFLEN]="";
		char *ret = NULL;
		
		if ((fp = fopen(log_filename, "r")) == NULL) {
			fprintf(stderr, "opening MP.txt failed !\n");
		}

		if (fp) {
			do {
				ret = fgets(buf, BUFLEN, fp);
				buf[BUFLEN-1] = '\0';
				if (ret) {
					if (strlen(buf) < 5)
						continue;
					if (buf[0] == '\r') { //0x0d
						for (i=0; i<strlen(buf); i++) {
							buf[i] = buf[i+1];
						}
					}
					//printf("###%s###\n", buf);
					if (strstr(buf, grepstr))
						break;
				}
			} while (ret != NULL);
			//printf("@@@%s@@@\n", buf);
			fclose(fp);
		}
		unlink(log_filename);
		create_file(log_filename, buf, strlen(buf));
	}
}
#endif

#ifdef __ECOS
void UDPserver_main(cyg_addrword_t data)
#else
int main(void)
#endif
{
	int sockfd;                     				// socket descriptors
	struct sockaddr_in my_addr;     		// my address information
	struct sockaddr_in their_addr;  			// connector¡¦s address information
	int addr_len, numbytes;
	FILE *fp;
	char buf[BUFLEN], buf_tmp[BUFLEN], 
	pre_result[BUFLEN];				// buffer that stores message
	static char cmdWrap[500];
	static int rwHW=0;
	//static int ret_value=0;
	// create a socket
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}
	bzero(&my_addr,sizeof(struct sockaddr_in));
	my_addr.sin_family = AF_INET;         		// host byte order
	my_addr.sin_port = htons(MYPORT);     	// short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; 	// automatically fill with my IP
	//memset(&(my_addr.sin_zero), '\0', 8); 	// zero the rest of the struct
	
	// bind the socket with the address
	if (bind(sockfd, (struct sockaddr *)&my_addr,
		sizeof(struct sockaddr)) == -1) {
		perror("bind");
		close(sockfd);
		exit(1);
	}

	addr_len = sizeof(struct sockaddr);

	printf("MP AUTOMATION daemon (ver 1.2)\n");
	//Self reading flash!!!
	#if 	0
	if(!ReadSinguture())
	{
		printf("HW Settting Error!!\n");
	}
	#endif	
	// main loop : wait for the client
	while (1) {
		//receive the command from the client
		memset(buf, 0, BUFLEN);
		memset(cmdWrap, 0, 500);
		rwHW = 0;
		if ((numbytes = recvfrom(sockfd, buf, BUFLEN, 0,
			(struct sockaddr *)&their_addr, (socklen_t *)&addr_len)) == -1) {
			fprintf(stderr,"Receive failed!!!\n");
			close(sockfd);
			exit(1);
		}
		
		printf("received command (%s) from IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));

		if(!memcmp(buf, "orf", 3)){
#ifdef __ECOS
			rtk_system(buf, NULL, MP_TMP_FILE);
#else
                        strcat(buf, " > /tmp/MP.txt");
                        system(buf);
#endif
                }
                if(!memcmp(buf, "irf", 3)){
#ifdef __ECOS
			rtk_system(buf, NULL, MP_TMP_FILE);
#else
                        strcat(buf, " > /tmp/MP.txt");
                        system(buf);
#endif
                }	
		if (!memcmp(buf, "ther", 4)) {
                        strcpy(buf, pre_result);
                }
		if (!memcmp(buf, "tssi", 4)) {
			strcpy(buf, pre_result);
		}
		if (!memcmp(buf, "query", 5)) {

			strcpy(buf, pre_result);
		}
#ifndef __ECOS
		if(!memcmp(buf, "cat", 3)){

			strcat(buf, " > /tmp/MP.txt");
			system(buf);		
		}
#endif
	#if 1	
		if (!memcmp(buf, "iwpriv wlan0 mp_tssi", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_TSSI);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan0 mp_ther", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_THER);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan0 mp_query", 21)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan0", buf_tmp, MP_QUERY_STATS);
			strcpy(buf, buf_tmp);
			usleep(1000);
			printf("w0 2b= %s\n",buf);
		}
	#endif
	#if 1	//wlan1 
		else if (!memcmp(buf, "iwpriv wlan1 mp_tssi", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan1", buf_tmp, MP_QUERY_TSSI);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan1 mp_ther", 20)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan1", buf_tmp, MP_QUERY_THER);
			strcpy(buf, buf_tmp);
			printf("buf= %s\n",buf);
			usleep(1000);
		}
		else if (!memcmp(buf, "iwpriv wlan1 mp_query", 21)) {

			strcpy(buf, pre_result);
			MP_get_ext("wlan1", buf_tmp, MP_QUERY_STATS);
			strcpy(buf, buf_tmp);
			usleep(1000);
			printf("w1 2b= %s\n",buf);
		}
	#endif	
		else {
#ifdef __ECOS
			int bypass_else = 0;
#endif

			if ( (!memcmp(buf, "flash read", 10)) ){
				if ((fp = fopen("/tmp/MP.txt", "r")) == NULL)
					fprintf(stderr, "opening MP.txt failed !\n");
	
				if (fp) {
#ifdef __ECOS
					int i;
					do {
						fgets(buf, BUFLEN, fp);
						buf[BUFLEN-1] = '\0';
						if(strlen(buf)==1 && buf[0]==0xD)
						{
							printf("[%s %d] Wrong file:MP.txt\n", __FUNCTION__,__LINE__);
							break;
						}
					} while (strlen(buf) < 5);

					if (buf[0] == '\r') { //0x0d
						for (i=0; i<strlen(buf); i++) {
							buf[i] = buf[i+1];
						}
					}
					//for (i=0; i<strlen(buf); i++) {
					//	printf("###%x %d###\n", buf[i], buf[i]);
					//}
#else
				fgets(buf, BUFLEN, fp);
#endif
					buf[BUFLEN-1] = '\0';
					{	//fix read_reg bug
						char strread[]="wlan0     read_reg:\n";
						char strreadrf[]="wlan0     read_rf:\n";
						char strpsd[]="wlan0     mp_psd:\n";
						if( strncmp(buf,strread,strlen(strread))==0 )
							get_read_reg_value( fp, buf, BUFLEN );
						if( strncmp(buf,strreadrf,strlen(strreadrf))==0 )
							get_read_reg_value( fp, buf, BUFLEN );
						if( strncmp(buf,strpsd,strlen(strpsd))==0 ) {
							get_read_reg_value( fp, buf, BUFLEN );
						}
					}
					fclose(fp);
				}	
				sprintf(pre_result, "data:%s", buf);
#ifdef __ECOS
				bypass_else = 1;
#endif
			}
			//ack to the client
			else if (!memcmp(buf, "flash get", 9))
				sprintf(pre_result, "%s > /tmp/MP.txt ok", buf);
			else {
				sprintf(pre_result, "%s ok", buf);;
			}
			
			if (!memcmp(buf, "iwpriv wlan0 mp_brx stop", 24)) {
				strcpy(buf, "stop");
				MP_get_ext("wlan0", buf, MP_BRX_PACKET);
			}
			else if (!memcmp(buf, "iwpriv wlan0 mp_tx", 18) && buf[18] == ' ') {
				memcpy(buf_tmp, buf+19, strlen(buf)-19);
				MP_get_ext("wlan0", buf_tmp, MP_TX_PACKET);
				strcpy(buf, buf_tmp);
			}
			
			else if (!memcmp(buf, "iwpriv wlan0 mp_ctx", 19) && buf[19] == ' ') {
				memcpy(buf_tmp, buf+20, strlen(buf)-20);
				MP_get_ext("wlan0", buf_tmp, MP_CONTIOUS_TX);
				strcpy(buf, buf_tmp);;
			}
			else if(!memcmp(buf, "iwpriv wlan0 read_reg", 21)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}
			else if(!memcmp(buf, "iwpriv wlan0 read_rf", 20)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}
			else if(!memcmp(buf, "iwpriv wlan0 efuse_get", 22)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}
			else if(!memcmp(buf, "iwpriv wlan0 efuse_set", 22)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
                                strcat(buf, " > /tmp/MP.txt");
                                system(buf);
#endif
            		}
			else if(!memcmp(buf, "iwpriv wlan0 efuse_sync", 23)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
                                strcat(buf, " > /tmp/MP.txt");
                                system(buf);
#endif
            		}
            
#if 1  //wlan 1
			
			else if (!memcmp(buf, "iwpriv wlan1 mp_brx stop", 24)) {
				strcpy(buf, "stop");
				MP_get_ext("wlan1", buf, MP_BRX_PACKET);
			}
			else if (!memcmp(buf, "iwpriv wlan1 mp_tx", 18) && buf[18] == ' ') {
				memcpy(buf_tmp, buf+19, strlen(buf)-19);
				MP_get_ext("wlan1", buf_tmp, MP_TX_PACKET);
				strcpy(buf, buf_tmp);
			}
			
			else if (!memcmp(buf, "iwpriv wlan1 mp_ctx", 19) && buf[19] == ' ') {
				memcpy(buf_tmp, buf+20, strlen(buf)-20);
				MP_get_ext("wlan1", buf_tmp, MP_CONTIOUS_TX);
				strcpy(buf, buf_tmp);;
			}
			else if(!memcmp(buf, "iwpriv wlan1 read_reg", 21)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}
			else if(!memcmp(buf, "iwpriv wlan1 read_rf", 20)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}			
			else if(!memcmp(buf, "iwpriv wlan1 efuse_get", 22)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);
#endif
			}
			else if(!memcmp(buf, "iwpriv wlan1 efuse_set", 22)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
                                strcat(buf, " > /tmp/MP.txt");
                                system(buf);
#endif
            		}
	else if(!memcmp(buf, "iwpriv wlan0 mp_psd", 19)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
				strcat(buf, " > /tmp/MP.txt");
				system(buf);		
#endif
			} 	
			else if(!memcmp(buf, "iwpriv wlan1 efuse_sync", 23)){
#ifdef __ECOS
				rtk_system(buf, NULL, MP_TMP_FILE);
#else
                                strcat(buf, " > /tmp/MP.txt");
                                system(buf);
#endif
            		}

#endif            
			else if (!memcmp(buf, "probe", 5))
				strcpy(buf, "ack");
			else if (!memcmp(buf, "verify_flw", 10)) {
				if ((fp = fopen("/tmp/MP.txt", "r")) == NULL)
					fprintf(stderr, "opening MP.txt failed !\n");
	
				if (fp) {
					fgets(buf, BUFLEN, fp);
					buf[BUFLEN-1] = '\0';
					fclose(fp);
				}
			}
#ifdef __ECOS
			else if (!memcmp(buf, "iwpriv wlan0 get_mib trsw_pape_C9 > //tmp//MP.txt", 49)) {
				char cmd[64];
				strcpy(cmd, "iwpriv wlan0 get_mib trsw_pape_C9");
				rtk_system(cmd, NULL, MP_TMP_FILE);
			}
			else if(!memcmp(buf, "cat .//proc//wlan0//mib_rf |grep chipVersion", 44)) {
				char cmd[64];
				strcpy(cmd, "wlan0 mib_rf");
				rtk_system(cmd, "chipVersion", MP_TMP_FILE);
			}
			else if(!memcmp(buf, "cat .//proc//wlan1//mib_rf |grep chipVersion", 44)) {
				char cmd[64];
				strcpy(cmd, "wlan1 mib_rf");
				rtk_system(cmd, "chipVersion", MP_TMP_FILE);
			}
			else if(!memcmp(buf, "cat .//proc//wlan0//mib_rf |grep MIMO_TR_hw_support", 51)) {
				char cmd[64];
				strcpy(cmd, "wlan0 mib_rf");
				rtk_system(cmd, "MIMO_TR_hw_support", MP_TMP_FILE);
			}
			else if(!memcmp(buf, "cat .//proc//wlan1//mib_rf |grep MIMO_TR_hw_support", 51)) {
				char cmd[64];
				strcpy(cmd, "wlan1 mib_rf");
				rtk_system(cmd, "MIMO_TR_hw_support", MP_TMP_FILE);
			}
			else if (bypass_else) {
				//do nothing
			}
#endif
			else {
#if 0
				if (!memcmp(buf, "flash get", 9))
					strcat(buf, " > /tmp/MP.txt");
#endif
					if (!memcmp(buf, "flash get", 9)){
					sprintf(cmdWrap, "flash gethw %s", buf+10);
					rwHW = 1;
					////strcat(buf, " > /tmp/MP.txt");
					strcat(cmdWrap, " > /tmp/MP.txt");
				}
				if (!memcmp(buf, "flash set", 9)) {
					sprintf(cmdWrap, "flash sethw %s", buf+10);
					rwHW = 1;
					//printf("1 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
					if ((numbytes = sendto(sockfd, pre_result, strlen(pre_result), 0,
						(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
						fprintf(stderr, "send failed\n");
						close(sockfd);
						exit(1);
					}
					//printf("2 sent command (%s) to IP:%s\n", pre_result, inet_ntoa(their_addr.sin_addr));
				}
				if(rwHW == 1){
#ifdef __ECOS
					char *ptr;
					//printf("###%s %d cmdWrap=%s###\n", __FUNCTION__,__LINE__, cmdWrap);
					ptr = strstr(cmdWrap, " > /tmp/MP.txt");
					if (ptr) {
						*ptr = '\0';
						//printf("###%s %d cmdWrap=%s###\n", __FUNCTION__,__LINE__, cmdWrap);
						rtk_system(cmdWrap, NULL, MP_TMP_FILE);
					}
					else {
						rtk_system(cmdWrap, NULL, NULL);
					}
#else
					system(cmdWrap);
#endif
				}else{
#ifdef __ECOS
					//printf("###%s %d buf=%s###\n", __FUNCTION__,__LINE__, buf);
					if (!memcmp(buf, "ifconfig wlan0 up", 17)) {
						if (!is_interface_up("wlan0")) {
							rtk_system(buf, NULL, NULL);
						}
					}
					else if (!memcmp(buf, "ifconfig wlan1 up", 17)) {
						if (!is_interface_up("wlan1")) {
							rtk_system(buf, NULL, NULL);
						}
					}
					else {
					#if 1
						char * pos=buf;
						if((pos = strchr(buf, '>')) != NULL)
						{
							if((pos != buf)&&(*(pos-1) != ' '))
							{
								*pos = ' ';
								sprintf(pos+1,">%s", MP_TMP_FILE);
								//printf("#######[%s]  %d:  buf = %s *pos = %c\n", __FUNCTION__,__LINE__,buf, *(pos+1));
							}
							rtk_system(buf, NULL, MP_TMP_FILE);
						}
						else
					#endif
						rtk_system(buf, NULL, NULL);
					}
#else
					system(buf);
#endif
				}
				
				//delay
				//open(/tmp/abc.txt)
				
			}
			
			strcpy(buf_tmp, pre_result);
			strcpy(pre_result, buf);
			strcpy(buf, buf_tmp);
		}

		if (memcmp(buf, "flash set", 9) != 0) {
			printf("1 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
			if ((numbytes = sendto(sockfd, buf, strlen(buf), 0,
				(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
				fprintf(stderr, "send failed\n");
				close(sockfd);
				exit(1);
			}
			//printf("2 sent command (%s) to IP:%s\n", buf, inet_ntoa(their_addr.sin_addr));
		}
      }
#ifndef __ECOS
	return 0;
#endif
}

#ifdef __ECOS
unsigned char mpd_stack[64*1024];
cyg_handle_t mpd_thread_handle;
cyg_thread mpd_thread_obj;

void create_mp_daemon(void)
{
	/* Create the thread */
	cyg_thread_create(16,
		      UDPserver_main,
		      0,
		      "UDPserver",
		      &mpd_stack,
		      sizeof(mpd_stack),
		      &mpd_thread_handle,
		      &mpd_thread_obj);
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(mpd_thread_handle);
}
#endif

