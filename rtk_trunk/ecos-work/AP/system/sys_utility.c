#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wrapper.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/hal/hal_if.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <cyg/kernel/kapi.h>
#include <cyg/compress/zlib.h>
#ifdef CYGPKG_CPULOAD
#include <cyg/cpuload/cpuload.h>
#endif
#include <sys/stat.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include "apmib.h"
#include "hw_settings.h"
#include "net_api.h"
#include "sys_utility.h"
#include "sys_init.h"

//extern int flash_main(unsigned int argc, unsigned char *argv[]);
#ifdef HAVE_BRIDGE
extern int brconfig_main(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef STATIC_ROUTE
extern int route_main(int argc,char **argv);
#endif
extern int icmp_main(int argc, char **argv);
extern int ifconfig_main(unsigned int argc, unsigned char *argv[]);
extern int iwpriv_main(unsigned int argc, unsigned char *argv[]);
#ifdef HAVE_WLAN_SCHEDULE
extern int wlschedule_main(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef HAVE_TELNETD
extern int telnetd_start(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef HAVE_NBSERVER
extern int nbserver_start(unsigned int argc, unsigned char *argv[]);
#endif

#if defined(CONFIG_RTL_819X)&&defined(HAVE_FIREWALL)	//jwj:20120626
extern int ipfw_init_main(unsigned int argc, unsigned char *argv[]);
#endif
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
extern void convert_hex_to_ascii(unsigned long code, char *out);
extern int compute_pin_checksum(unsigned long int PIN);
extern int validate_pin_code(unsigned long code);
#endif
#endif
extern void shutdown_netdev(void);

time_t sys_settime = 0;

#ifdef SYS_INIT_USING_MBOX
cyg_mbox sys_mbox;
cyg_handle_t sys_mbox_hdl;
#else
cyg_flag_t sys_flag;
#endif



int SetWlan_idx(char * wlan_iface_name)
{
	int idx;
	idx = atoi(&wlan_iface_name[4]);
        if (idx >= NUM_WLAN_INTERFACE) {
        printf("invalid wlan interface index number!\n");
       		return 0;
        }
        apmib_set_wlanidx(idx);
        apmib_set_vwlanidx(0);

#ifdef MBSSID           

        if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
        	wlan_iface_name[6] == 'v' && wlan_iface_name[7] == 'a') {
                idx = atoi(&wlan_iface_name[8]);
                if (idx >= NUM_VWLAN_INTERFACE) {
                	printf("invalid virtual wlan interface index number!\n");
                	return 0;
                }

                apmib_set_vwlanidx(idx+1);
                idx = atoi(&wlan_iface_name[4]);
                apmib_set_wlanidx(idx);
        }
#endif

#ifdef UNIVERSAL_REPEATER
        if (strlen(wlan_iface_name) >= 9 && wlan_iface_name[5] == '-' &&
        	!memcmp(&wlan_iface_name[6], "vxd", 3)) {
                apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
                idx = atoi(&wlan_iface_name[4]);
                apmib_set_wlanidx(idx);
        }
#endif

//printf("\r\n wlan_iface_name=[%s],wlan_idx=[%u],vwlan_idx=[%u],__[%s-%u]\r\n",wlan_iface_name,wlan_idx,vwlan_idx,__FILE__,__LINE__);
	return 1;
}


void do_reset(int type)
{
	

	unsigned long flags;
	
	if (type) {
		diag_printf("Resetting ...\n");
#ifdef HOME_GATEWAY
		ppp_shutdown();
#endif	
		diag_printf("intf ...\n");
		HAL_DISABLE_INTERRUPTS(flags);
		shutdown_all_interfaces();
	}
	else {
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_HAVE_WRAPPER
		//should be safe in the exception context
		shutdown_netdev();
		HAL_DISABLE_INTERRUPTS(flags);
#endif
	}
	//HAL_DELAY_US(2*100000);
	if (type)
		diag_printf("...\n");
	/*disable interrupts to fix reboot fail */
	//HAL_DISABLE_INTERRUPTS(flags);
	HAL_PLATFORM_RESET();
}

int RunSystemCmd(char *filepath, ...)
{
	va_list argp;
	unsigned char *argv[30]={0};
	int status = 0;
	char *para;
	int argno = 0;
	
	
	va_start(argp, filepath);
	while (1) { 
		para = va_arg( argp, char*);
		if ( strcmp(para, "") == 0 )
			break;
		argv[argno] = (unsigned char *)para;
		//printf("Parameter %d is: %s\n", argno, para); 
		argno++;
	} 
	argv[argno+1] = NULL;

	/*int i;
	printf("argno=%d\n", argno);
	for (i=0; i<argno; i++) {
		printf("argv[%d]=%s\n", i, argv[i]);
	}*/
	
	//status = DoCmd(argv, filepath);
	if (strcmp((char *)argv[0], "iwpriv")==0) {
		iwpriv_main((argno-1), &argv[1]);
	}
#ifdef HAVE_WLAN_SCHEDULE
	else if(strcmp((char*)argv[0],"wlschedule")==0){
		wlschedule_main((argno-1), &argv[1]);
	}
#endif
#ifdef HAVE_TELNETD
	else if(strcmp((char*)argv[0],"telnetd")==0){
		telnetd_start((argno-1), &argv[1]);
	}
#endif
#ifdef HAVE_TR069
	else if(strcmp((char*)argv[0],"cwmpClient")==0){
		printf("sys_utility.c: argc=%d\n", argno);
		tr069_start((argno), &argv[0]);	
	}
#endif
#ifdef HAVE_NBSERVER
	else if(strcmp((char*)argv[0],"nmbserver")==0){
		nbserver_start((argno-1), &argv[1]);
	}
#endif

#ifdef HAVE_BRIDGE
	else if (strcmp((char *)argv[0], "brconfig")==0) {
		brconfig_main((argno-1), &argv[1]);
	}
#endif
	else if (strcmp((char *)argv[0], "ifconfig")==0) {
		ifconfig_main(argno, &argv[0]);
	}
	else if (strcmp((char *)argv[0], "ping")==0) {
		icmp_main((argno-1), &argv[1]);
	}
#ifdef STATIC_ROUTE
	else if (strcmp((char *)argv[0], "route")==0) {
		route_main((argno-1), &argv[1]);
	}
#endif
#ifdef CONFIG_IPV6_ROUTE6
	else if (strcmp((char *)argv[0], "route6")==0) {
		route6_main((argno), &argv[0]);
	}
#endif
	else if ((strcmp((char *)argv[0], "reset")==0) ||
		 (strcmp((char *)argv[0], "reboot")==0)) {
		do_reset(1);
	}
#if defined(CONFIG_RTL_819X)&&defined(HAVE_FIREWALL)	//jwj:20120627
	else if (strcmp((char *)argv[0], "ipfw")==0) {
		ipfw_init_main(argno, &argv[0]);
	}
#endif
//#ifdef HAVE_APMIB
#if 0				//add by yp 2016-2-27
	else if (strcmp((char *)argv[0], "flash")==0) {
		flash_main((argno-1), &argv[1]);
	}
#endif
	else if (strcmp((char *)argv[0], "echo")==0) {
		extern int create_file(char *name, char *buf, int len);
		status = create_file(filepath, (char *)argv[1], strlen((char *)argv[1]));
	}
	else if (strcmp((char *)argv[0], "eth")==0) {
		#ifndef HAVE_NOETH
		eth_cmd_dispatch(argno-1, &argv[1]);
		#endif
	}
#if defined(CONFIG_RTL_819X)&&defined(HAVE_IPV6FIREWALL)	
	else if (strcmp((char *)argv[0], "ip6fw")==0) {
		ip6fw_init_main(argno, &argv[0]);
	}
#endif
#ifdef ULINK_DHCP_AUTO
	else if(strcmp((char *)argv[0], "ulink_dhcp_auto")==0){
			ulink_dhcp_auto_startup(argno-1, &argv[1]);
		}
#endif
#ifdef DHCP_AUTO_SUPPORT
		else if(strcmp((char *)argv[0], "dhcp_auto")==0){
				dhcp_auto_startup(argno-1, &argv[1]);
			}
#endif

	else {
		printf("%s: Unknow command \"%s\"\n", __FUNCTION__, argv[0]);
		status = -1;
	}
	va_end(argp);
	return status;
}

/////////////////////////////////////////////////////////////////////////////////
int rtk_flash_read(char *buf, int offset, int len)
{
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
	
	if ((stat = cyg_flash_read((FLASH_BASE_ADDR+offset), (void *)buf, len, &err_addr)) != CYG_FLASH_ERR_OK) {
            printf("FLASH: read failed: %s %p\n", flash_errmsg(stat), (void *)err_addr);
            return 0;
        }
#endif
	return 1;
}

unsigned char syscmd_stack[2048];
cyg_handle_t  syscmd_stack_thread;
cyg_thread  syscmd_stack_thread_object;

typedef struct cmd_param {
	int argc;
	char *argv[24];
} cmd_param_T;
static cmd_param_T param;

int syscmd_main(cyg_addrword_t data)
{
	cmd_param_T *pcmd;
	if(NULL==data)
		return -1;
	pcmd=(cmd_param_T *)data;
	if ((strcmp((char *)pcmd->argv[0], "reset")==0) ||
		 (strcmp((char *)pcmd->argv[0], "reboot")==0)) {
		sleep(2);
		do_reset(1);
	}
	return 0;
}

int RunSystemCmd_thread(char *filepath, ...)
{
	va_list argp;
	
	int status = 0;
	char *para;
	int argno = 0;
	//int i;
	bzero(&param,sizeof(param));
	va_start(argp, filepath);
	while (1) { 
		para = va_arg( argp, char*);
		if ( strcmp(para, "") == 0 )
			break;
		param.argv[argno] = (unsigned char *)para;
		//printf("Parameter %d is: %s\n", argno, para); 
		argno++;
	} 
	param.argv[argno+1] = NULL;
	param.argc=argno;	
	va_end(argp);
	/*
	printf("argno=%d\n", argno);
	for (i=0; i<argno; i++) {
		printf("argv[%d]=%s\n", i, argv[i]);
	}
	*/
	//status = DoCmd(argv, filepath);

	cyg_thread_create(SYSCMD_THREAD_PRIORITY,
		syscmd_main,
		(unsigned long)&param,
		"sys cmd",
		syscmd_stack,
		sizeof(syscmd_stack),
		&syscmd_stack_thread,
		&syscmd_stack_thread_object);
	
	cyg_thread_resume(syscmd_stack_thread);
	return status;
}



////////////////////////////////////////////////////////////////////////////////
int rtk_flash_write(char *buf, int offset, int len)
{
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
	//erase
	/*
	if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+offset), len, &err_addr)) != CYG_FLASH_ERR_OK) {
            printf("FLASH: erase failed: %s %p\n", flash_errmsg(stat), (void *)err_addr);
            return 0;
        }
    */
   
        //program
	if ((stat = cyg_flash_program((FLASH_BASE_ADDR+offset), (void *)buf, len, &err_addr)) != CYG_FLASH_ERR_OK) {
            printf("FLASH: program failed: %s %p\n", flash_errmsg(stat), (void *)err_addr);
            return 0;
        }
#endif
	return 1;
}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
void generate_pin_code(char *pinbuf)
{
	struct timeval tod;
	unsigned long num;
	HW_SETTING_T *hs;

	gettimeofday(&tod , NULL);
	// read hw settings from flash
	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs != NULL) {
		tod.tv_sec += hs->wlan[0].macAddr[4]+hs->wlan[0].macAddr[5];
		free(hs);
	}
	srand(tod.tv_sec+tod.tv_usec);
	num = rand() % 10000000;
	num = num*10 + compute_pin_checksum(num);
	convert_hex_to_ascii((unsigned long)num, pinbuf);
}

int check_pin_code_of_hw_settings(void)
{
	HW_SETTING_T *hs;
	int code, ret=1;
	
	// read hw settings from flash
	hs = (HW_SETTING_T *)read_hw_settings();
	if (hs != NULL) {
		if ((hs->wlan[0].wscPin[0] == '\0') ||
	    	    (strcmp((char*)hs->wlan[0].wscPin, "00000000")==0)) {
			ret = 0;
		}
		else {
			code = atoi((char*)hs->wlan[0].wscPin);
			if (!validate_pin_code(code))
				ret = 0;
		}
		free(hs);
	}
	return ret;
}
#endif
#endif

/* ===========================================================================
 * Uncompress input to output then close both files.
 */
#ifdef CONFIG_IRES_WEB_ADVANCED_SUPPORT
static char *web_ptr;
static char *buf_ptr=NULL;
static int web_size;
static char *config_dat_ptr;
static int config_dat_size;
#ifdef CONFIG_RESERVE_WEB_CACHE
void set_buf_ptr(char *ptr)
{
	buf_ptr=ptr;
}
void free_buf_ptr()
{
	if(buf_ptr) {
		free(buf_ptr);
		buf_ptr=NULL;
	}	
}

#endif
void set_web_ptr(char *ptr)
{
	web_ptr=ptr;
}
void set_web_size(int  size)
{
	web_size=size;
}
char * get_web_ptr()
{
	return web_ptr;
}

void free_web_ptr()
{
	if(web_ptr) {
		free(web_ptr);
		web_ptr=NULL;
	}
			
}
void clear_web_size()
{
	web_size=0;
}
int list_webpags(char *buf, int size)
{
	int i;
	FILE_ENTRY_Tp pEntry;
	if (size < sizeof(FILE_ENTRY_T) ) {
		printf("Invalid decompress file size %ld!\n", size);
		return -1;
	}
	
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];

#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
#endif
		diag_printf("name:%s\n",pEntry->name);
		diag_printf("size:%d\n",pEntry->size);
#ifdef CONFIG_WEB_COMP_TWICE
		i += (pEntry->size + sizeof(FILE_ENTRY_T));
#else
		i += (sizeof(FILE_ENTRY_T));
#endif
	}
	return 1;
}

unsigned int  find_max_webpage_size_in_buf(char *path_name,char *buf, int size)
{
	FILE_ENTRY_Tp pEntry;
	int i;
	unsigned int len=0;
	if (size < sizeof(FILE_ENTRY_T) ) {
		printf("Invalid decompress file size %ld!\n", size);
		return -1;
	}
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];
#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
		pEntry->size_uncomp= DWORD_SWAP(pEntry->size_uncomp);
#endif
		if(len < pEntry->size_uncomp)
			len = pEntry->size_uncomp;
#ifdef CONFIG_WEB_COMP_TWICE
       	i += (pEntry->size + sizeof(FILE_ENTRY_T));
#else
       	i += sizeof(FILE_ENTRY_T);
#endif
	}
	return len;
}

FILE_ENTRY_Tp  find_webpage_in_buf(char *path_name,char *buf, int size)
{
	FILE_ENTRY_Tp pEntry;
	int i;	
	if (size < sizeof(FILE_ENTRY_T) ) {
		printf("Invalid decompress file size %ld!\n", size);
		return -1;
	}	
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];
#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
#endif
		if (!strcasecmp(path_name, pEntry->name))
            		return pEntry;
#ifdef CONFIG_WEB_COMP_TWICE
		i += (pEntry->size + sizeof(FILE_ENTRY_T));
#else
		i += sizeof(FILE_ENTRY_T);
#endif
	}
	return NULL;
}

#if defined(HTTP_FILE_SERVER_SUPPORTED)
int get_usbdisk_dat_ires(char* path_name,char **ptr, int *size)
{	
	struct stat configStat={0};
	char	tmpBuf[128] = {0};
	snprintf(tmpBuf,128,"/tmp/usb%s",path_name);
	
	FILE* fh = fopen(tmpBuf,"r");

	if(NULL == fh){
		printf("open file fail\n");
		return (-1);
	}
	stat(tmpBuf,&configStat);
	
	config_dat_size=configStat.st_size;
	config_dat_ptr=(char*)malloc(config_dat_size);
	//diag_printf("%s:%d:size:%d\n", __func__, __LINE__,config_dat_size);
	
	if(!config_dat_ptr)
	{
		printf("fail malloc %s\n",config_dat_size);
		close(fh);
		return -1;
	}
	if(fread(config_dat_ptr,1,config_dat_size,fh)!=config_dat_size)
	{
		fclose(fh);
		printf("fail read %d\n",config_dat_size);
		return -1;
	}
	*ptr=config_dat_ptr;
	*size=config_dat_size;
	//diag_printf("%p %d\n",config_dat_ptr,config_dat_size);
	fclose(fh);
	return 0;
}
#endif

int get_config_dat_ires(char **ptr, int *size)
{
	struct stat configStat={0};
	FILE* fh = fopen("/web/config.dat","r");

	if(NULL == fh){
		printf("open file fail\n");
		return (-1);
	}
	stat("/web/config.dat",&configStat);
	
	config_dat_size=configStat.st_size;
	config_dat_ptr=(char*)malloc(config_dat_size);
	if(!config_dat_ptr)
	{
		printf("fail malloc %s\n",config_dat_size);
		close(fh);
		return -1;
	}
	if(fread(config_dat_ptr,1,config_dat_size,fh)!=config_dat_size)
	{
		fclose(fh);
		printf("fail read %d\n",config_dat_size);
		return -1;
	}
	*ptr=config_dat_ptr;
	*size=config_dat_size;
	//diag_printf("%p %d\n",config_dat_ptr,config_dat_size);
	fclose(fh);
	return 0;
}

int get_syslog_log_ires(char **ptr, int *size)
{
	struct stat configStat={0};
	FILE* fh = fopen("/tmp/syslog.log","r");

	if(NULL == fh){
		printf("open file fail\n");
		return (-1);
	}
	stat("/tmp/syslog.log",&configStat);
	
	config_dat_size=configStat.st_size;
	config_dat_ptr=(char*)malloc(config_dat_size);
	if(!config_dat_ptr)
	{
		printf("fail malloc %s\n",config_dat_size);
		close(fh);
		return -1;
	}
	if(fread(config_dat_ptr,1,config_dat_size,fh)!=config_dat_size)
	{
		fclose(fh);
		printf("fail read %d\n",config_dat_size);
		return -1;
	}
	*ptr=config_dat_ptr;
	*size=config_dat_size;
	//diag_printf("%p %d\n",config_dat_ptr,config_dat_size);
	fclose(fh);
	return 0;
}

#if defined(HAVE_LZMA) && !defined(CONFIG_WEB_COMP_TWICE)
int httpd_find_ires_advanced(char *path_name, char **ptr, int *size)
{
	FILE_ENTRY_Tp pEntry;
	char* buf=NULL;
	unsigned int buf_size;
	
	if(!strcmp(path_name,"/config.dat"))
	{
		if(get_config_dat_ires(ptr,size)<0)
		{
			printf("fail to load config.dat");
			return -1;
		}
		return 0;
	}

	if(!strcmp(path_name,"/tmp/syslog.log"))
	{
		if(get_syslog_log_ires(ptr,size)<0)
		{
			printf("fail to load /tmp/syslog.log");
			return -1;
		}
		return 0;
	}
	
	if(NULL == web_ptr)
	{
		//diag_printf("please do read webpage first\n");
		//return -1;
		if (read_flash_webpage("/web", "") < 0)
		{
			printf("fail to load web pages\n");
			return -1;
		}
	}	
	pEntry=find_webpage_in_buf(path_name,web_ptr,web_size);
	if(NULL == pEntry)
	{
		diag_printf("can find the webpage %s\n",path_name);
		return -1;
	}
	
	/*read web page from flash to tmp buffer*/
	buf = malloc(pEntry->size);
	if(buf == NULL){
		printf("malloc memory to store compressed web page error %d\n",pEntry->size);
		return -1;
	}
	if ( rtk_flash_read(buf, WEB_PAGE_OFFSET+sizeof(WEB_HEADER_T)+pEntry->offset+sizeof(FILE_ENTRY_T), 
		pEntry->size) == 0) {
		printf("Read web page failed!\n");
		free(buf);
		return -1;
	}
	
	/*save decompressed web page*/
	if(buf_ptr == NULL){
		buf_size = find_max_webpage_size_in_buf(path_name,web_ptr,web_size);
		//printf("max webpage size is %d\n",buf_size);
		buf_ptr = malloc(buf_size);
		
		if(NULL == buf_ptr){
			printf("malloc memory for buf_ptr error\n");
			return -1;
		}		
	}	
	// decompress file to buf_ptr
	if(lzma_decode_to_buf(buf,pEntry->size,buf_ptr,pEntry->size_uncomp)){
		printf("lzma decompress to buf_ptr failed\n");	
		if(buf)
			free(buf);
		return -1;
	}
	if(buf)
		free(buf);
	*ptr=buf_ptr;
	*size=pEntry->size_uncomp;	
	
	return 0;
}


#else
int httpd_find_ires_advanced(char *path_name, char **ptr, int *size)
{
	FILE_ENTRY_Tp pEntry;
	char* tmpFile[10];
#ifndef CONFIG_WEB_COMP_TWICE
	char* buf=NULL;
#endif
	int fh;
	unsigned int buf_size;

#if defined(HTTP_FILE_SERVER_SUPPORTED)
	extern unsigned char http_file_server_dir_type;
	if(http_file_server_dir_type == 2){
		if(get_usbdisk_dat_ires(path_name,ptr,size)<0)
		{
			printf("fail to load /tmp/usb%s",path_name);
			return -1;
		}
		return 0;		
	}
#endif

	if(!strcmp(path_name,"/config.dat"))
	{
		if(get_config_dat_ires(ptr,size)<0)
		{
			printf("fail to load config.dat");
			return -1;
		}
		return 0;
	}

	if(!strcmp(path_name,"/tmp/syslog.log"))
	{
		if(get_syslog_log_ires(ptr,size)<0)
		{
			printf("fail to load /tmp/syslog.log");
			return -1;
		}
		return 0;
	}
	
	if(NULL == web_ptr)
	{
		//diag_printf("please do read webpage first\n");
		//return -1;
		if (read_flash_webpage("/web", "") < 0)
		{
			printf("fail to load web pages\n");
			return -1;
		}
	}	
	pEntry=find_webpage_in_buf(path_name,web_ptr,web_size);
	if(NULL == pEntry)
	{
		diag_printf("can find the webpage %s\n",path_name);
		return -1;
	}

	/*open tmp file*/
	strcpy(tmpFile,"tmp_file");		
	fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh < 0) {
		printf("Create output file error %s!\n", tmpFile );
		return -1;
	}

#ifndef CONFIG_WEB_COMP_TWICE
	/*read web page from flash to tmp buffer*/
	buf = malloc(pEntry->size);
	if(buf == NULL){
		printf("malloc memory to store compressed web page error %d\n",pEntry->size);
		close(fh);
		return -1;
	}
	if ( rtk_flash_read(buf, WEB_PAGE_OFFSET+sizeof(WEB_HEADER_T)+pEntry->offset+sizeof(FILE_ENTRY_T), 
		pEntry->size) == 0) {
		printf("Read web page failed!\n");
		close(fh);
		free(buf);
		return -1;
	}
#endif
#ifdef CONFIG_WEB_COMP_TWICE
	if ( write(fh, (char *)((char *)pEntry+sizeof(FILE_ENTRY_T)), pEntry->size) != pEntry->size) {
		printf("write file error %s!\n", tmpFile);
		close(fh);
		unlink(tmpFile);
		return -1;
	}
#else
	/*write tmp buffer to file*/
	if ( write(fh, buf, pEntry->size) != pEntry->size) {
		printf("write file error %s!\n", tmpFile);
		close(fh);
		free(buf);
		unlink(tmpFile);
		return -1;
	}
#endif
	close(fh);
#ifndef CONFIG_WEB_COMP_TWICE
	free(buf);
#endif	
	/*save decompressed web page*/
	if(buf_ptr == NULL){
		buf_size = find_max_webpage_size_in_buf(path_name,web_ptr,web_size);
		//printf("max webpage size is %d\n",buf_size);
		buf_ptr = malloc(buf_size);
		
		if(NULL == buf_ptr){
			printf("malloc memory for buf_ptr error\n");
			unlink(tmpFile);
			return -1;
		}		
	}	
	// decompress file to buf_ptr
	if(file_gz_uncompress_to_buf(tmpFile,buf_ptr,pEntry->size_uncomp)<0){
		printf("decompress to buf_ptr failed\n");
		unlink(tmpFile);
		return -1;
	}
	unlink(tmpFile);
	*ptr=buf_ptr;
	*size=pEntry->size_uncomp;	
	
	return 0;
}
#endif

int get_uncompress_length(char *infile)
{
    gzFile in;
    int len;
    int buf_len=16384;
    char * buf = malloc(buf_len);
    if(buf == NULL)
    {
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
    }
	memset(buf, 0, buf_len);
    int ret=0;
    in = gzopen(infile, "rb");
    if (in == NULL) {
        printf("%s: can't gzopen %s\n", __FUNCTION__, infile);
        free(buf);
        return -1;
    }
    for (;;) 
    {
        len = gzread(in, buf, buf_len);
        if (len < 0) 
        {
        	printf("%s: failed gzread\n", __FUNCTION__);
			ret = -1;
			goto uncomp_done;
        }
        if (len == 0)
        	break;
	 	ret+=len;
    }
uncomp_done:

    if (gzclose(in) != Z_OK)
    	printf("%s: failed gzclose\n", __FUNCTION__);
	free(buf);	
    return ret;
}

int file_gz_uncompress_to_buf(char *infile, char *web_buf, int web_size)
{
    gzFile in;
    int len;
    int ret=0;

    in = gzopen(infile, "rb");
    if (in == NULL) {
        printf("%s: can't gzopen %s\n", __FUNCTION__, infile);
        return -1;
    }
	
    len = gzread(in, web_buf, web_size);
	
    if (len < 0) {
    	printf("%s: failed gzread\n", __FUNCTION__);
	ret =  -1;
	goto uncomp_err;
    }
	
    if (len != web_size) {
    	printf("%s: failed gzread all\n", __FUNCTION__);
	ret = -1;
    }

uncomp_err:
    if (gzclose(in) != Z_OK) 	
    	printf("%s: failed gzclose\n", __FUNCTION__);

    return ret;
}

#else
int file_gz_uncompress(char *infile, char *outfile)
{
    FILE  *out;
    gzFile in;
    int buf_len=16384;
    char * buf = malloc(buf_len);
    if(buf == NULL)
    {
    	printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
        return -1;
    }
    memset(buf, 0, buf_len);
    int len;
    int ret=0;

    in = gzopen(infile, "rb");
    if (in == NULL) {
        printf("%s: can't gzopen %s\n", __FUNCTION__, infile);
		free(buf);
        return -1;
    }
    out = fopen(outfile, "wb");
    if (out == NULL) {
        printf("%s: can't fopen %s\n", __FUNCTION__, infile);
        gzclose(in);
        free(buf);
        return -1;
    }
    
    for (;;) {
        len = gzread(in, buf, buf_len);
        if (len < 0) {
        	printf("%s: failed gzread\n", __FUNCTION__);
		ret = -1;
		goto uncomp_done;
        }
        if (len == 0)
        	break;

        if ((int)fwrite(buf, 1, (unsigned)len, out) != len) {
		printf("%s: failed fwrite\n", __FUNCTION__);
		ret = -1;
		goto uncomp_done;
        }
    }

uncomp_done:
    if (fclose(out))
    	printf("%s: failed fclose\n", __FUNCTION__);

    if (gzclose(in) != Z_OK)
    	printf("%s: failed gzclose\n", __FUNCTION__);

    free(buf);	
    return ret;
}
#endif
//-----------------------------------------------------------------------------
#define CP0_STATUS $12
#define CP0_CAUSE $13

/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define read_32bit_cp0_register(source)                         \
({ int __res;                                                   \
        __asm__ __volatile__(                                   \
	".set\tpush\n\t"					\
	".set\treorder\n\t"					\
        "mfc0\t%0,"STR(source)"\n\t"                            \
	".set\tpop"						\
        : "=r" (__res));                                        \
        __res;})

#define RTL_R32(addr)		(*(volatile unsigned int *)(addr))

/*
 * generates a table showing which interrupts
 * have an ISR attached.
 */
__attribute__((nomips16)) 
void show_interrupt_table(void)
{
	int i;
	cyg_bool_t inuse;
	int maxint = CYGNUM_HAL_ISR_MAX;

	printf("ISR\tCount\tState\n");
	for( i = CYGNUM_HAL_ISR_MIN; i <= maxint ; i++ ) {
                HAL_INTERRUPT_IN_USE( i, inuse );
		printf("%3d\t%ld\t%s\n", i, rtk_interrupt_count[i], inuse?"In Use":"Free");
	}

	{
		unsigned int res;
		res = read_32bit_cp0_register(CP0_STATUS);
		printf("STATUS=%08x\n", res);
		res = read_32bit_cp0_register(CP0_CAUSE);
		printf("CAUSE=%08x\n", res);
		printf("GIMR=%08x\n", RTL_R32(0xB8003000));
		printf("GISR=%08x\n", RTL_R32(0xB8003004));
		printf("IRR0=%08x\n", RTL_R32(0xB8003008));
		printf("IRR1=%08x\n", RTL_R32(0xB800300C));
	}
}

//-----------------------------------------------------------------------------

time_t get_epoch_build_time(void)
{
	extern unsigned char build_time[];
	struct tm tm_time;
	char *month_index[]={"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	char week_date[20], month[20], date[20], year[20], tmp1[20];
	int i;
	
	//Mon Nov 10 16:42:19 CST 2008
	memset(&tm_time, 0 , sizeof(tm_time));
	sscanf((char *)build_time,"%s %s %s %s %d:%d:%d %s", week_date, date, month, year, &(tm_time.tm_hour), &(tm_time.tm_min), &(tm_time.tm_sec), tmp1);
	tm_time.tm_isdst = -1;  /* Be sure to recheck dst. */
	for (i=0; i< 12; i++) {
		if (strcmp(month_index[i], month)==0) {
			tm_time.tm_mon = i;
			break;
		}
	}
	tm_time.tm_year = atoi(year) - 1900;
	tm_time.tm_mday =atoi(date);
	return mktime_rewrite(&tm_time);
}

double get_uptime(void)
{
	time_t tm;
	
	time(&tm);
#if 0
#ifdef HAVE_SNTP
	/*diff is for sntp time adjust,new time-now, maybe postive or negative*/
	sys_settime += get_sntp_diff();
#endif
#endif
	return difftime(tm, sys_settime);
}
void update_sys_settime(void)
{
#ifdef HAVE_SNTP
    /*diff is for sntp time adjust,new time-now, maybe postive or negative*/
	sys_settime += get_sntp_diff();
#endif

}

unsigned long get_time(void)
{
	return (unsigned long)time(NULL);
}
//-----------------------------------------------------------------------------

int get_thread_info_by_name(char *name, cyg_thread_info *pinfo)
{
	cyg_handle_t thandle = 0;
	cyg_uint16 id = 0;

	while (cyg_thread_get_next(&thandle, &id)) {
		cyg_thread_get_info(thandle, id, pinfo);
		//printf("ID: %04x name: %10s pri: %d\n",
                //	pinfo->id, pinfo->name ? pinfo->name:"----", pinfo->set_pri);
		if (strcmp(name, pinfo->name)==0)
			return 1;
	}
	return 0;
}

//-----------------------------------------------------------------------------
#ifdef CYGPKG_CPULOAD
static int cpuload_sleep_time = 1; //in seconds
static int cpuload_measuring = 0;
static char cpuload_measure_stack[4096];
static cyg_handle_t cpuload_measure_handle, cpuload_measure_thread_handle;
static cyg_thread cpuload_measure_thread;
static cyg_cpuload_t cpuload;
extern cyg_uint32 cpuload_calibration_value;

void do_cpuload_measurement(cyg_addrword_t data)
{
	cyg_uint32 average_point1s;
	cyg_uint32 average_1s;
	cyg_uint32 average_10s;

	cyg_cpuload_create(&cpuload,cpuload_calibration_value,&cpuload_measure_handle);
	cpuload_measuring = 1;
	printf("cpuload measurement(%ds) starts.\n", cpuload_sleep_time);
  
	while (cpuload_measuring) {
		//cyg_thread_delay(cpuload_sleep_time*100); //in ticks
		sleep(cpuload_sleep_time); //in seconds
		cyg_cpuload_get(cpuload_measure_handle,&average_point1s,&average_1s,&average_10s);
		printf("%d %d %d\n",average_point1s,average_1s,average_10s);
	}
	cyg_thread_exit();
}

void cpuload_measurement_start(void)
{
	if (cpuload_measuring) {
		printf("cpuload measurement already started.\n");
	}
	else {	
		cyg_thread_create(30,do_cpuload_measurement, 0,
			"cpuload measurement", cpuload_measure_stack,sizeof(cpuload_measure_stack),
			&cpuload_measure_thread_handle,&cpuload_measure_thread);
		cyg_thread_resume(cpuload_measure_thread_handle);
	}
}

void cpuload_measurement_end(void)
{
	cpuload_measuring = 0;
	sleep(1);
	cyg_cpuload_delete(cpuload_measure_handle);
	cyg_thread_kill(cpuload_measure_thread_handle);
	cyg_thread_delete(cpuload_measure_thread_handle);
	printf("cpuload measurement ends.\n");
}

void cpuload_set_sleep_time(int sleep_time)
{
	cpuload_sleep_time = sleep_time;
}
#endif
int isValidName(char *str)
{
	int i, len=strlen(str);

	for (i=0; i<len; i++) {
		if (str[i] == ' ' || str[i] == '"' || str[i] == '\x27' || str[i] == '\x5c' || str[i] == '$')
			return 0;
	}
	return 1;
}
int isValidSsidName(char *str)
{
	int i, len=strlen(str);

	for (i=0; i<len; i++) {
		if (str[i] == '"' || str[i] == '\x27' || str[i] == '\x5c' || str[i] == '$')
			return 0;
	}
	return 1;
}

#ifdef HOME_GATEWAY
char getIntfPPPRunOn(char* lanIface,char* wanIface)
{
	int opmode=-1;
	char eth_ifname[8]="eth1";
	if(!apmib_get(MIB_OP_MODE, (void *)&opmode))
		return -1;
	strcpy(lanIface,"eth0");
	
//	if(opmode==WISP_MODE)
//		strcpy(wanIface,"wlan0-vxd0");

	if(opmode == WISP_MODE)
	{
		int wisp_wan_id=0, wlan_mode=0;
		if(!apmib_get(MIB_WISP_WAN_ID, (void *)&wisp_wan_id))
			return -1;
		
		//for smart repeater
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		apmib_set_wlanidx(wisp_wan_id);
		
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		
		if(wlan_mode == CLIENT_MODE)
			sprintf(wanIface, "wlan%d", wisp_wan_id);
		else
			sprintf(wanIface, "wlan%d-vxd0", wisp_wan_id);
		
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
	}	
	else
		strcpy(wanIface,eth_ifname);
	return 0;
}
char getInterfaces(char* lanIface,char* wanIface)
{
	int opmode=-1,wisp_wanid=0;
	
	//fix the issue of when change wan type from pppoe to dhcp, the wan interface is still ppp0, not eth1 (amy reported, realsil can not reproduce)
	char eth_ifname[8]="eth1";
	char ppp_ifname[8]="ppp0";
	
	DHCP_T dhcp;
	if(!apmib_get( MIB_WAN_DHCP, (void *)&dhcp))
		return -1;
	if(!apmib_get(MIB_OP_MODE, (void *)&opmode))
		return -1;
	if(!lanIface||!wanIface)
	{
		fprintf(stderr,"invalid input!!\n");
		return -1;
	}
	strcpy(lanIface,"eth0");
	switch(dhcp)
	{
		case STATIC_IP:
		case DHCP_CLIENT:
		case DHCP_SERVER:        
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        case DHCP_PLUS:
        #endif
			if(opmode==WISP_MODE)
			{
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
				apmib_get(MIB_WISP_WAN_ID,(void*)&wisp_wanid);
#endif

#if defined(CONFIG_SMART_REPEATER)				
				int wlan_mode,i=0;	
#ifdef CONFIG_WLANIDX_MUTEX
				int s = apmib_save_idx();
#else
				apmib_save_idx();
#endif
				for(i=0;i<NUM_WLAN_INTERFACE;i++)
				{
					apmib_set_wlanidx(i);
					apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
					if(wlan_mode == CLIENT_MODE)
					{
						if(i==wisp_wanid)
							sprintf(wanIface, "wlan%d",i);													
					}else
					{
						if(i==wisp_wanid)
							sprintf(wanIface, "wlan%d-vxd0",i);
					}
				}
#ifdef CONFIG_WLANIDX_MUTEX
				apmib_revert_idx(s);
#else
				apmib_revert_idx();
#endif

#else
				if(wisp_wanid==0)
					strcpy(wanIface,"wlan0");
				else
					strcpy(wanIface,"wlan1");
#endif
			}
			else
				//strcpy(wanIface,"eth1");
				strcpy(wanIface,eth_ifname);
			break;
		case PPPOE:
		case L2TP:
		case PPTP:
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        case PPPOE_HENAN:
        case PPPOE_NANCHANG:
        case PPPOE_OTHER1:
        case PPPOE_OTHER2:
        #endif

	#ifdef CONFIG_RTL_DHCP_PPPOE
		if(!isFileExist("/etc/ppp_link"))
			strcpy(wanIface,eth_ifname);
		else
			strcpy(wanIface,ppp_ifname);			
	#else		
		strcpy(wanIface,ppp_ifname);
	#endif
			break;
	}
	return (0);
}
#endif
#if defined(ROUTE_SUPPORT)
void addStaticRoute(STATICROUTE_Tp pEntry)
{
	char lanIface[16]={0},wanIface[16]={0};
	char ip[30], netmask[30], gateway[30],metric[4]={0},*tmpStr=NULL,*iface=NULL;
	getInterfaces(lanIface,wanIface);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->dstAddr)));
	strcpy(ip,tmpStr);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->netmask)));
	strcpy(netmask,tmpStr);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->gateway)));
	strcpy(gateway,tmpStr);
	if(pEntry->interface)
		iface=wanIface;
	else
		iface=lanIface;
	sprintf(metric,"%d",pEntry->metric);
	//diag_printf("route add -net %s -gateway %s -netmask %s -interface %s -metric %s\n",ip,gateway,netmask,iface,metric);
	RunSystemCmd(NULL_FILE, "route", "add", "-net", ip,"-gateway", gateway, "-netmask",netmask, "-interface", iface,"-metric",metric, NULL_STR);
}
void delStaticRoute(STATICROUTE_Tp pEntry)
{
	char lanIface[16]={0},wanIface[16]={0};
	char ip[30], netmask[30], gateway[30],metric[4]={0},*tmpStr=NULL,*iface=NULL;
	getInterfaces(lanIface,wanIface);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->dstAddr)));
	strcpy(ip,tmpStr);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->netmask)));
	strcpy(netmask,tmpStr);
	tmpStr=inet_ntoa(*((struct in_addr *)(pEntry->gateway)));
	strcpy(gateway,tmpStr);
	if(pEntry->interface)
		iface=wanIface;
	else
		iface=lanIface;
	sprintf(metric,"%d",pEntry->metric);
	//diag_printf("route delete -net %s -gateway %s -netmask %s -interface %s -metric %s\n",ip,gateway,netmask,iface,metric);
	RunSystemCmd(NULL_FILE, "route", "delete", "-net", ip,"-gateway", gateway, "-netmask",netmask, "-interface", iface,"-metric",metric, NULL_STR);
}
#endif


void delDefaultRoute(void)
{	
	RunSystemCmd(NULL_FILE, "route", "delete", "-net", "0.0.0.0", "-netmask","0.0.0.0",NULL_STR);
}

void flushIpfw(void)
{
	RunSystemCmd(NULL_FILE, "ipfw", "flush",NULL_STR);
}

#define INTFNMAE_STR "interface"
#define IP_STR "ip"
#define MASK_STR "netmask"
#define GATEWAY_STR "gateway"
#define DNS1_STR "dns1"
#define DNS2_STR "dns2"
#define TMP_BUF_SIZE 256

#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
void appendDnsAddr(char *dns_addr, char *resolv_file)
{	
	if(!dns_addr || !resolv_file)
		return ;
	
	FILE *fp=NULL;	
	
	char tmpbuf[64], linebuf[64];
	int found=0, i;		
	
	if((fp=fopen(resolv_file, "r+"))==NULL)
		goto OUT;
		
	while(fgets(linebuf, sizeof(linebuf), fp))
	{		
		if(strcmp(dns_addr, linebuf)==0)
		{
			found=1;
			break;
		}
	}
	
	if(found==0)
		write_line_to_file(resolv_file, 2, linebuf);
	fclose(fp);
	fp=NULL;
	
OUT:
	
	if(fp!=NULL)
	{
		fclose(fp);
		fp=NULL;
	}
	return;
}
#endif
int save_WanInfo(char *filename, char* intfname, u_int32_t  ip,
	u_int32_t  mask,u_int32_t  gateway,u_int32_t  dns1,u_int32_t  dns2 )
{
	FILE *fp;
	if(NULL == filename) {
		diag_printf("filename is NULL!!\n");
		return -1;
	}
	fp=fopen(filename,"w");
	if(NULL == fp) {
		return -1;
	}
	fprintf(fp,"%s:%s\n",INTFNMAE_STR,intfname);
	fprintf(fp,"%s:%s\n",IP_STR,inet_ntoa(*(struct in_addr *)(&ip)));
	fprintf(fp,"%s:%s\n",MASK_STR,inet_ntoa(*(struct in_addr *)(&mask)));	
	fprintf(fp,"%s:%s\n",GATEWAY_STR,inet_ntoa(*(struct in_addr *)(&gateway)));
	fprintf(fp,"%s:%s\n",DNS1_STR,inet_ntoa(*(struct in_addr *)(&dns1)));
	fprintf(fp,"%s:%s\n",DNS2_STR,inet_ntoa(*(struct in_addr *)(&dns2)));
	fclose(fp);
	u_int8_t tmpbuf[32];
	
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER	
	u_int8_t flag;
	if(isFileExist("/etc/resolv.conf"))
		flag=2;
	else
		flag=1;
	
	sprintf(tmpbuf, "nameserver %s\n", inet_ntoa(*(struct in_addr *)(&dns1)));
	if(flag==1)
		write_line_to_file("/etc/resolv.conf", flag, tmpbuf);
	else
		appendDnsAddr(tmpbuf, "/etc/resolv.conf");
	
	sprintf(tmpbuf, "nameserver %s\n", inet_ntoa(*(struct in_addr *)(&dns2)));
	appendDnsAddr(tmpbuf, "/etc/resolv.conf");
#else
	sprintf(tmpbuf, "nameserver %s\n", inet_ntoa(*(struct in_addr *)(&dns1)));
	write_line_to_file("/etc/resolv.conf", 1, tmpbuf);

	sprintf(tmpbuf, "nameserver %s\n", inet_ntoa(*(struct in_addr *)(&dns2)));
	write_line_to_file("/etc/resolv.conf", 2, tmpbuf);	
#endif
	return 1;
}


int get_WanInfo(char *filename,  char * intfname, u_int32_t  *ip,
	u_int32_t  *mask,u_int32_t  *gateway,u_int32_t  *dns1,u_int32_t  *dns2 )
{
	FILE *fp;
	char buffer[TMP_BUF_SIZE];
	char *name;
	char *value;
	if(NULL == filename) {
		diag_printf("filename is NULL!!\n");
		return -1;
	}
	fp=fopen(filename,"r");
	if(NULL == fp) {
		return -1;
	}
	while(fgets(buffer,TMP_BUF_SIZE,fp) !=NULL)
	{
		name=strtok(buffer,":");
		value=strtok(NULL,":");
		/*overrid \n*/
		value[strlen(value)-1]='\0';
		if(!strcmp(INTFNMAE_STR,name))
		{
			if(intfname)
			strcpy(intfname,value);
		}
		else if(!strcmp(IP_STR,name))
		{
			inet_aton(value,(struct in_addr *)ip);
		}
		else if(!strcmp(MASK_STR,name))
		{
			inet_aton(value,(struct in_addr *)mask);
		}
		else if(!strcmp(GATEWAY_STR,name))
		{
			inet_aton(value,(struct in_addr *)gateway);
		}
		else if(!strcmp(DNS1_STR,name))
		{
			inet_aton(value,(struct in_addr *)dns1);
		}
		else if(!strcmp(DNS2_STR,name))
		{
			inet_aton(value,(struct in_addr *)dns2);
		}
	}
	fclose(fp);
	return 1;	
}
int isFileExist(char *file_name)
{
	struct stat status;

	if ( stat(file_name, &status) < 0)
		return 0;

	return 1;
}
#ifdef CONFIG_RTL_VLAN_SUPPORT

char delExistVlan(unsigned short vlanid)
{
	VLAN_T VlanEntry={0};
	int i=0,entryNum=0;
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)&VlanEntry) = (char)i;
		if ( !apmib_get(MIB_VLAN_TBL, (void *)&VlanEntry))
			return -1;
		if(vlanid == VlanEntry.vlanId)
		{
			if(!apmib_set(MIB_VLAN_DEL, (void *)&VlanEntry))
				return -1;
			i--;
			entryNum--;
		}			
	}
	
	return 0;
}
char delVlanBindEntry(unsigned short vlanid,unsigned short netIfId)
{
	VLAN_NETIF_BIND_T vlanNetifBind={0};
	int i=0,entryNum=0,vlanBindCount=0;
	if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)&vlanNetifBind) = (char)i;
		if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL, (void *)&vlanNetifBind))
			return -1;
//		printf("%s:%d  vlanid=%d netIfid=%d \n",__FILE__,__LINE__,vlanNetifBind.vlanId,vlanNetifBind.netifId);
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)&vlanNetifBind) = (char)i;
		if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL, (void *)&vlanNetifBind))
			return -1;
		if(vlanid == vlanNetifBind.vlanId)
		{
			if(netIfId==vlanNetifBind.netifId)
			{
				if(!apmib_set(MIB_VLAN_NETIF_BIND_DEL, (void *)&vlanNetifBind))
					return -1;
				i--;
				entryNum--;
			}else
				vlanBindCount++;
		}			
	}
	if(vlanBindCount==0)
	{//vlanid bind no netif now,del the vlan
		delExistVlan(vlanid);
	}
	return 0;
}
int disable_allVlanInterface()
{
	NETIFACE_T netIf[2]={0};
	int netIfEntryNum=0,i=0;
	
	if(!apmib_get(MIB_NETIFACE_TBL_NUM,(void*)&netIfEntryNum))
	{
		printf("get MIB_NETIFACE_TBL_NUM fail!\n");
		return -1;
	}
	for(i=1;i<=netIfEntryNum;i++)
	{
		*((char*)netIf)=(char)i;
		if(!apmib_get(MIB_NETIFACE_TBL,(void*)netIf))
		{
			printf("get MIB_NETIFACE_TBL item fail!\n");
			return -1;
		}
		memcpy(&(netIf[1]),&(netIf[0]),sizeof(NETIFACE_T));
		netIf[1].netifEnable=0;
		if(!apmib_set(MIB_NETIFACE_MOD,(void*)netIf))
		{
			printf("set MIB_NETIFACE_MOD item fail!\n");
			return -1;
		}
		//delVlanBindEntry(netIf[0].netifPvid,netIf[0].netifId);
	}
}

int enable_vlanInterface(char*interFaceName)
{
	NETIFACE_T netIf[2]={0};
	int netIfEntryNum=0,i=0;
	
	if(!apmib_get(MIB_NETIFACE_TBL_NUM,(void*)&netIfEntryNum))
	{
		printf("get MIB_NETIFACE_TBL_NUM fail!\n");
		return -1;
	}
	for(i=1;i<=netIfEntryNum;i++)
	{
		*((char*)netIf)=(char)i;
		if(!apmib_get(MIB_NETIFACE_TBL,(void*)netIf))
		{
			printf("get MIB_NETIFACE_TBL item fail!\n");
			return -1;
		}
		if(!strcmp(netIf[0].netifName,interFaceName))
		{
			memcpy(&(netIf[1]),&(netIf[0]),sizeof(NETIFACE_T));
			netIf[1].netifEnable=1;
			if(!apmib_set(MIB_NETIFACE_MOD,(void*)netIf))
			{
				printf("set MIB_NETIFACE_MOD item fail!\n");
				return -1;
			}
		}
	}
}
char checkVlanExist(unsigned short vid)
{
	int vlanNum=0,i=0;
	VLAN_T vlan={0};
	if ( !apmib_get(MIB_VLAN_TBL_NUM, (void *)&vlanNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	
	for(i=1;i<=vlanNum;i++)
	{
		*((char *)&vlan) = (char)i;
		if ( !apmib_get(MIB_VLAN_TBL, (void *)&vlan))
			return -1;
		if(vid==vlan.vlanId)
			return 1;//find
	}
	//not find,add it!
	vlan.vlanId=vid;
	if(!apmib_set(MIB_VLAN_ADD,(void*)&vlan))
	{
		fprintf(stderr, "Set table MIB_VLAN_ADD error!\n");
		return -1;
	}
	return 0;
}
int getBindEntry(int vid,int netifId,VLAN_NETIF_BIND_T* pBindEntry)
{
	int i=0,entryNum=0;
	if(!pBindEntry || vid<0 || netifId<0)
	{
		printf("invalid input!\n");
		return -1;
	}
	if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}

	for(i=1;i<=entryNum;i++)
	{
		*((char *)pBindEntry) = (char)i;
		if ( !apmib_get(MIB_VLAN_NETIF_BIND_TBL, (void *)pBindEntry))
			return -1;
		if(pBindEntry->vlanId==vid && pBindEntry->netifId==netifId)
			return 1;//find
	}	
	return 0;//not find
}
/************************
** check interface pvid bind
** It should always add to vlan
************************/
char checkIfPvidVlanBind()
{
	int netIfNum=0,i=0,ret=0;
	NETIFACE_T netifEntry={0};
	
	VLAN_NETIF_BIND_T bindEntry={0};
	
	if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&netIfNum)) 
	{
  		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}	
	for(i=1;i<=netIfNum;i++)
	{
		*((char *)&netifEntry) = (char)i;
		if ( !apmib_get(MIB_NETIFACE_TBL, (void *)&netifEntry))
			return -1;
		if(netifEntry.netifPvid<1 || netifEntry.netifPvid>4094 || !netifEntry.netifEnable)
			continue;
		if((ret=getBindEntry(netifEntry.netifPvid,netifEntry.netifId,&bindEntry))<0)
			return -1;
	//	diag_printf("%s:%d pvid=%d netifid=%d ret=%d\n",__FILE__,__LINE__,netifEntry.netifPvid,netifEntry.netifId,ret);
		if(ret==0)
		{//not find the bind entry,should add
			if(checkVlanExist(netifEntry.netifPvid)<0)
				return -1;
			bindEntry.vlanId=netifEntry.netifPvid;
			bindEntry.netifId=netifEntry.netifId;
			bindEntry.tagged=0;
			if(!apmib_set(MIB_VLAN_NETIF_BIND_ADD,(void*)&bindEntry))
			{
				fprintf(stderr, "Set MIB_VLAN_NETIF_BIND_ADD error!\n");
				return -1;
			}
		}
	}
	return 0;
}
int getNetifEntry(int netifId,NETIFACE_Tp pNetifEntry)
{
	int i=0,entryNum=0;
	if(!pNetifEntry || netifId<0)
	{
		printf("invalid input!\n");
		return -1;
	}
	if ( !apmib_get(MIB_NETIFACE_TBL_NUM, (void *)&entryNum)) 
	{
		fprintf(stderr, "Get table entry error!\n");
		return -1;
	}
	for(i=1;i<=entryNum;i++)
	{
		*((char *)pNetifEntry) = (char)i;
		if ( !apmib_get(MIB_NETIFACE_TBL, (void *)pNetifEntry))
			return -1;
		if(pNetifEntry->netifId==netifId)
			return 0;
	}	
	return 0;
}

#if defined(CONFIG_RTL_92D_SUPPORT)	|| defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)/*For dual band*/
int rtl_checkWlanVAEnable(int wlan_no, int va_num)
{
	int intVal = 1;
	
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	apmib_set_wlanidx(wlan_no);
	apmib_set_vwlanidx(va_num+1);

	if (!apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal))
		return -1;
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
	return intVal;
}
#else
int rtl_checkWlanVAEnable(int va_num)
{
	int intVal = 1;

#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	apmib_set_wlanidx(0);

	apmib_set_vwlanidx(va_num+1);

	if (!apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal))
		return -1;
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
	return intVal;
}
#endif
#endif

int	opterr = 1,		/* if error message should be printed */
	optind = 1,		/* index into parent argv vector */
	optopt,			/* character checked for validity */
	optreset;		/* reset getopt */
char	*optarg;		/* argument associated with option */

char	optswi = '-';
static char * my_strnset(char * str,int val,int count)

{

   char *p = str;

   while (count-- && *p)

      *p++ = (char)val;

   return(p);

}

int	getopt(int argc, char* const * argv, const char* opts)
{
	char		*cp;
	char		noswitch[3];
	static int	sp = 1;
	int			c;

	my_strnset(noswitch, optswi, 2);
	noswitch[2]=0;
	if (sp == 1) {
		if (optind >= argc ||
			argv[optind][0] != optswi || argv[optind][1] == '\0') {
			return(EOF);
		}
		else if (strcmp(argv[optind], noswitch) == 0) {
			optind++;
			return(EOF);
		}
	}
	optopt = c = argv[optind][sp];
	if (c == ':' || (cp = strchr(opts, (char) c)) == NULL) {
		if (argv[optind][++sp] == '\0') {
			optind++;
			sp = 1;
		}
		return('?');
	}
	if (*++cp == ':') {
		if (argv[optind][sp+1] != '\0') {
			optarg = &argv[optind++][sp+1];
		} else if (++optind >= argc) {
			sp = 1;
			return('?');
		} else {
			optarg = argv[optind++];
		}
		sp = 1;
	} else {
		if (argv[optind][++sp] == '\0') {
			sp = 1;
			optind++;
		}
		optarg = NULL;
	}
	return(c);
}

#ifdef 	ECOS_DBG_STAT
static dbg_stat dbg_info[DBG_TYPE_MAX][DBG_MODULE_MAX];
static int module_index=0;

int dbg_stat_add(int module_index,int dbg_type, int action_type, unsigned int size)
{
	if(module_index >=DBG_MODULE_MAX){
		diag_printf("dbg_stat wrong module_type:%d\n",module_index);
		return -2;
	}
	else if(dbg_type >= DBG_TYPE_MAX ){
		diag_printf("dbg_stat wrong dbg_type:%d\n",dbg_type);
		return -1;
	}
	
	switch(action_type){
		case DBG_ACTION_ADD:
			dbg_info[dbg_type][module_index].count_add++;
			dbg_info[dbg_type][module_index].size+=size;
			break;
		case DBG_ACTION_DEL:
			dbg_info[dbg_type][module_index].count_del++;
			dbg_info[dbg_type][module_index].size-=size;
			break;
		default:
			diag_printf("dbg_stat wrong action_type:%d\n",action_type);
			return -3;
	}	

	return 0;
}

void dbg_stat_init(){
	int i,j;
	for(i=0;i<DBG_TYPE_MAX;i++){
		for(j=0;j<DBG_MODULE_MAX;j++){
			memset(&dbg_info[i][j],0,sizeof(dbg_stat));
		}
	}
}

int dbg_stat_register(char *name)
{
	int i;
	int ret;
	
	if(name==NULL){
		diag_printf("module name is NULL\n");
		return -1;
	}
	
	if(strlen(name) >= MODULE_NAME_SIZE){
		diag_printf("module name overflow\n");
		return -2;
	}


	if(module_index >= DBG_MODULE_MAX || module_index<0){
		diag_printf("module index overflow\n");
		return -3;
	}
	
	for(i=0;i<module_index;i++)
	{
		if(strcmp(dbg_info[0][i].name,name)==0)	
		{
			diag_printf("module already registered\n");
			return i;
		}
	}	
		
	for(i=0;i<DBG_TYPE_MAX;i++)
	{
		strcpy(dbg_info[i][module_index].name,name);			
	}
	
	ret = module_index;
	module_index++;
	
	return ret;
}

void dbg_stat_show(){
	int i,j;
	
	for(i=0;i<DBG_TYPE_MAX;i++){
		switch(i){
			case DBG_TYPE_MALLOC:
				diag_printf("\nHeap statistics:\n");
				diag_printf("Index \t Module \t malloc_count \t free_count \t size\n");
				break;
			case DBG_TYPE_FILE:
				diag_printf("File statistics:\n");
				diag_printf("Index \t Module \t open_count \t  close_count \t size\n");
				break;
			case DBG_TYPE_SOCKET:
				diag_printf("Socket statistics:\n");
				diag_printf("Index \t Module \t open_count \t  close_count \t size\n");
				break;
			case DBG_TYPE_MBUF:
				diag_printf("Mbuf statistics:\n");
				diag_printf("Index \t Module \t malloc_count \t  free_count \t size\n");
				break;

			case DBG_TYPE_SKB:
				diag_printf("Skb statistics:\n");
				diag_printf("Index \t Module \t malloc_count \t  free_count \t size\n");
				break;	
				
			default:
				break;
		}
		
		for(j=0;j<DBG_MODULE_MAX;j++){
			if(j>=module_index)
			{
				break;
			}
			diag_printf("%d \t %-16s \t %d \t %7d \t %d \n",j,
				dbg_info[i][j].name,dbg_info[i][j].count_add,
				dbg_info[i][j].count_del,dbg_info[i][j].size);			
		}
	}
}
#endif

#if defined(ECOS_MEM_CHAIN_API)
static unsigned char wan_connected = 0;
static Mem_Chain_Upgrade_Header* mem_chain_list_ptr[MEM_CHAIN_UPGRADE_LIST_NUM];

#define MEM_CHAIN_UPGRADE_ERR_ENABLE
#ifdef MEM_CHAIN_UPGRADE_ERR_ENABLE
#define MEM_CHAIN_UPGRADE_ERR		diag_printf
#else
#define MEM_CHAIN_UPGRADE_ERR
#endif

static int mem_chain_upgrade_MemListInfo_Get(void* s,int n,void* address_info)
{
	int i,block,offset,ret = -1;
	struct mem_chain_upgrade_list* heap_malloc=NULL,*misc_malloc=NULL,*clust_start=NULL;
	Men_Chain_Upgrade_Address_Info *tmp = (Men_Chain_Upgrade_Address_Info*)address_info;
	
	for(i = 0;i < MEM_CHAIN_UPGRADE_LIST_NUM;i++){
		if(mem_chain_list_ptr[i] == NULL)
			continue;
	
		if(((char*)s + n) <= (mem_chain_list_ptr[i]->header->mem_addr+mem_chain_list_ptr[i]->size)
			&& (char*)s >= mem_chain_list_ptr[i]->header->mem_addr){
			offset = (char*)s - mem_chain_list_ptr[i]->header->mem_addr;
			
			if(mem_chain_list_ptr[i]->header != NULL){
				heap_malloc = mem_chain_list_ptr[i]->header;
				if(heap_malloc->next != NULL){
					misc_malloc = heap_malloc->next;
					if(misc_malloc->next != NULL){
						clust_start = misc_malloc->next;
					}
				}
			}

			if(offset < heap_malloc->size)
				block = 0;
			else if(offset < (heap_malloc->size + misc_malloc->size))
				block = 1;
			else if((offset +1 - (heap_malloc->size + misc_malloc->size)) % (clust_start->size) == 0)
				block = (offset +1 - (heap_malloc->size + misc_malloc->size))/(clust_start->size) + 1;
			else
				block = (offset - (heap_malloc->size + misc_malloc->size)+1)/(clust_start->size) + 2;

			ret = 0;
			break;
			
		}
	}

	tmp->block_num = block;
	tmp->offset = offset;
	tmp->mem_list_num = i;

	return ret;
}

static void* mem_chain_upgrade_offset_mem_convert(char* data)
{
	int i,offset,offset_start,start_loc,node_prevsize = 0;
	struct mem_chain_upgrade_list* heap_malloc=NULL,*misc_malloc=NULL,*clust_start=NULL,*ptr;
	
	Mem_Chain_Upgrade_Offset_Mem* mem_offset = (Mem_Chain_Upgrade_Offset_Mem*)malloc(sizeof(Mem_Chain_Upgrade_Offset_Mem));
	if(mem_offset == NULL)
		return NULL;

	for(i = 0;i < MEM_CHAIN_UPGRADE_LIST_NUM;i++)
	{
		if(mem_chain_list_ptr[i] == NULL)
			continue;
		
		if(data <= (mem_chain_list_ptr[i]->header->mem_addr + mem_chain_list_ptr[i]->size)
			&& data >=  mem_chain_list_ptr[i]->header->mem_addr){
			offset = data - mem_chain_list_ptr[i]->header->mem_addr;

			if(mem_chain_list_ptr[i]->header != NULL){
				heap_malloc = mem_chain_list_ptr[i]->header;
				if(heap_malloc->next != NULL){
					misc_malloc = heap_malloc->next;
					if(misc_malloc->next != NULL){
						clust_start = misc_malloc->next;
					}
				}
			}

			if(heap_malloc != NULL && offset < heap_malloc->size)
				start_loc = 0;
			else if(misc_malloc != NULL && offset < (heap_malloc->size + misc_malloc->size))
				start_loc = 1;
			else if(clust_start != NULL){
				if((offset - (heap_malloc->size + misc_malloc->size)+1) % (clust_start->size) == 0)
					start_loc = (offset - (heap_malloc->size + misc_malloc->size)+1)/(clust_start->size) + 1;
				else
					start_loc = (offset - (heap_malloc->size + misc_malloc->size)+1)/(clust_start->size) + 2;//modify
			}
				

			ptr = mem_chain_list_ptr[i]->header;
			while(start_loc--){
				node_prevsize += ptr->size;
				ptr = ptr->next;
			}
			
			offset_start = (offset - node_prevsize) % ptr->size;
			mem_offset->mem = ptr->mem_addr+offset_start;
			mem_offset->offset = offset_start;
			mem_offset->size = ptr->size;

			return mem_offset;
		}
	}

	return NULL;
	
}

void *mem_chain_upgrade_malloc(unsigned int num_bytes,unsigned int heap_keep,unsigned int miscpool_keep,unsigned int clust_keep)
{
	int i,j,clust_num;
	unsigned int heap_freesize,miscpoll_freesize,clust_blocksize,clust_freesize;
	unsigned int heap_mallocsize,misc_mallocsize,clust_mallocnum;
	unsigned char mem_using = 0;
	int keep_size;

	int head_size = sizeof(Mem_Chain_Upgrade_Header);
	int node_size = sizeof(struct mem_chain_upgrade_list);
	
	{
		/*calcu free size in heap miscpool and clustpool*/
		extern void mem_chain_upgrade_get_meminfo(int* misc_max,int* cluster_max,int* cluster_block);
		mem_chain_upgrade_get_meminfo(&miscpoll_freesize,&clust_freesize,&clust_blocksize);		
		extern void mem_chain_upgrade_get_heap_size(int* heap_size);
		mem_chain_upgrade_get_heap_size(&heap_freesize);
		clust_num = clust_freesize/clust_blocksize;

		if(heap_freesize < heap_keep)
			return NULL;

		if(miscpoll_freesize < miscpool_keep)
			return NULL;

		if(clust_num < clust_keep)
			return NULL;
			
		heap_freesize -= heap_keep;
		miscpoll_freesize -= miscpool_keep;
		clust_num -= clust_keep;
		clust_freesize -= clust_keep*clust_blocksize;
		
		keep_size = head_size + node_size *(2+clust_num) + sizeof(IMG_HEADER_T) +  sizeof(Mem_Chain_Upgrade_Offset_Mem) + 128;
		
		//test
#if 0
		heap_freesize = 500 + keep_size;
		//miscpoll_freesize = 500;
#endif
		
#if 0
		heap_freesize = 235 + keep_size;
		miscpoll_freesize = 456;
#endif
		
		if(heap_freesize < keep_size){
			//MEM_CHAIN_UPGRADE_ERR("heap max free size is too small,heap max free size:%d.\n",heap_freesize);
			return NULL;
		}
		if(num_bytes > (heap_freesize + miscpoll_freesize + clust_freesize - keep_size)){
			//MEM_CHAIN_UPGRADE_ERR("dut have not enough size for upgrade\n");
			return NULL;
		}
		
		heap_mallocsize = heap_freesize - keep_size;		
		misc_mallocsize = miscpoll_freesize;
		if((num_bytes - heap_mallocsize - misc_mallocsize) % clust_blocksize == 0)
			clust_mallocnum = (num_bytes - heap_mallocsize - misc_mallocsize)/clust_blocksize;
		else
			clust_mallocnum = (num_bytes - heap_mallocsize - misc_mallocsize)/clust_blocksize + 1;

		if(num_bytes <=  heap_mallocsize){
			heap_mallocsize = num_bytes;
			mem_using |=  NEED_HEAP;
		}
		else if(num_bytes >  heap_mallocsize && 
				num_bytes <= (heap_mallocsize + misc_mallocsize)){
			heap_mallocsize = heap_freesize - keep_size;
			misc_mallocsize = num_bytes - heap_mallocsize;
			mem_using |= (NEED_HEAP | NEED_MISCPOOL);
		}
		else{
			heap_mallocsize = heap_freesize - keep_size;
			misc_mallocsize = miscpoll_freesize;

			if((num_bytes - heap_mallocsize - misc_mallocsize)%clust_blocksize == 0)
				clust_mallocnum = (num_bytes - heap_mallocsize - misc_mallocsize)/clust_blocksize;
			else
				clust_mallocnum = (num_bytes - heap_mallocsize - misc_mallocsize)/clust_blocksize + 1;
			mem_using |= (NEED_HEAP | NEED_MISCPOOL | NEED_CLUSTPOOL);
		}
		
		for(i = 0;i < MEM_CHAIN_UPGRADE_LIST_NUM;i++){
			if(mem_chain_list_ptr[i] == NULL){
				mem_chain_list_ptr[i] = (Mem_Chain_Upgrade_Header*)malloc(sizeof(Mem_Chain_Upgrade_Header));
				if(mem_chain_list_ptr[i] == NULL){
					//MEM_CHAIN_UPGRADE_ERR("malloc %d from heap error\n",sizeof(Mem_Chain_Upgrade_Header));
					return NULL;
				}
				mem_chain_list_ptr[i]->size = num_bytes;
				mem_chain_list_ptr[i]->header = NULL;
				break;
			}
		}

		if(mem_using & NEED_HEAP){
			/* malloc from heap*/
			struct mem_chain_upgrade_list* heap_malloc = (struct mem_chain_upgrade_list*)malloc(sizeof(struct mem_chain_upgrade_list));
			if(heap_malloc == NULL){
				//MEM_CHAIN_UPGRADE_ERR("malloc %d from heap error\n",sizeof(struct mem_chain_upgrade_list));
				if(mem_chain_list_ptr[i] != NULL){
					free(mem_chain_list_ptr[i]);
					mem_chain_list_ptr[i] = NULL;
				}
				
				return NULL;
			}
			heap_malloc->mem_addr = (char*)malloc(heap_mallocsize);

			if(heap_malloc->mem_addr == NULL){
				//MEM_CHAIN_UPGRADE_ERR("malloc %d size from heap error\n",heap_mallocsize);
				if(heap_malloc != NULL){
					free(heap_malloc);
					heap_malloc = NULL;
				}

				if(mem_chain_list_ptr[i] != NULL){
					mem_chain_list_ptr[i]->header = NULL;
					free(mem_chain_list_ptr[i]);
					mem_chain_list_ptr[i] = NULL;
				}
				
				return NULL;
			}
			memset(heap_malloc->mem_addr,0,heap_mallocsize);
			heap_malloc->next = NULL;
			heap_malloc->size = heap_mallocsize;
			mem_chain_list_ptr[i]->header = heap_malloc;

			if(mem_using & NEED_MISCPOOL){
				/*malloc from miscpoll*/
				struct mem_chain_upgrade_list* misc_malloc = (struct mem_chain_upgrade_list*)malloc(sizeof(struct mem_chain_upgrade_list));
				if(misc_malloc == NULL){
					//MEM_CHAIN_UPGRADE_ERR("malloc %d from heap error\n",sizeof(struct mem_chain_upgrade_list));
					if(mem_chain_list_ptr[i]->header->mem_addr != NULL)
						mem_chain_upgrade_free(mem_chain_list_ptr[i]->header->mem_addr);
					
					return NULL;
				}

				extern void mem_chain_upgrade_malloc_miscpoll(void ** buf,int len);
				mem_chain_upgrade_malloc_miscpoll(&(misc_malloc->mem_addr),misc_mallocsize);
				
				//misc_malloc->mem_addr = (char*)cyg_net_malloc(misc_mallocsize,M_DEVBUF,M_NOWAIT);
				if(misc_malloc->mem_addr == NULL){
					//MEM_CHAIN_UPGRADE_ERR("mallco %d size from miscpoll error\n",misc_mallocsize);
					if(misc_malloc != NULL){
						free(misc_malloc);
						misc_malloc = NULL;
					}

					if(mem_chain_list_ptr[i]->header->mem_addr != NULL)
						mem_chain_upgrade_free(mem_chain_list_ptr[i]->header->mem_addr);
					return NULL;
				}
				memset(misc_malloc->mem_addr,0,misc_mallocsize);
				misc_malloc->next = NULL;
				misc_malloc->size = misc_mallocsize;
				heap_malloc->next = misc_malloc;

				if(mem_using & NEED_CLUSTPOOL){
					/*malloc from clustpoll*/
					for(j = 0;j < clust_mallocnum;j++){
						struct mem_chain_upgrade_list* tmp1 = (struct mem_chain_upgrade_list*)malloc(sizeof(struct mem_chain_upgrade_list));
						if(tmp1 == NULL){
							//MEM_CHAIN_UPGRADE_ERR("malloc %d from heap error\n",sizeof(struct mem_chain_upgrade_list));
							if(mem_chain_list_ptr[i]->header->mem_addr != NULL)
								mem_chain_upgrade_free(mem_chain_list_ptr[i]->header->mem_addr);
							
							return NULL;
						}

						tmp1->next = NULL;

						extern void mem_chain_upgrade_malloc_cluster(void ** buf,int * len);
						mem_chain_upgrade_malloc_cluster(&(tmp1->mem_addr),&(tmp1->size));
#if 0
						extern void* cyg_net_cluster_alloc(void);
						cluster = (union mcluster *)cyg_net_cluster_alloc();
						tmp1->mem_addr = cluster->mcl_buf;
						tmp1->size = MCLBYTES;
#endif	
						if(tmp1->mem_addr == NULL){
							//MEM_CHAIN_UPGRADE_ERR("malloc one cluster error from clusterpoll\n");
							if(mem_chain_list_ptr[i]->header->mem_addr != NULL)
								mem_chain_upgrade_free(mem_chain_list_ptr[i]->header->mem_addr);

							if(tmp1){
								free(tmp1);
								tmp1 = NULL;
							}
							
							return NULL;
						}
						
						struct mem_chain_upgrade_list* tmp2 = misc_malloc;
						while(tmp2->next != NULL)
							tmp2 = tmp2->next;
						tmp2->next = tmp1;

					}
				}
			}
		}			
	} 
	
	return mem_chain_list_ptr[i]->header->mem_addr;	
	
}	

//s need convert to memory chain address
void *mem_chain_upgrade_memset(void *s, char ch, int n)
{
	int i,block,size = 0,offset,offset2,res_n = n,copy_len = 0;
	Men_Chain_Upgrade_Address_Info address_info;
	struct mem_chain_upgrade_list* mem;
	if(mem_chain_upgrade_MemListInfo_Get(s,n,&address_info) == -1){
		return NULL;
	};

	i = address_info.mem_list_num;
	block = address_info.block_num;
	offset = address_info.offset;
	
	mem = mem_chain_list_ptr[i]->header;
	while(block--){
		size += mem->size;
		mem = mem->next;
	}

	while(res_n){
		offset2 = (offset-size) % mem->size;
		if((mem->size - offset2) >= res_n){
			copy_len = res_n;
			memset(mem->mem_addr+offset2,ch,res_n);
			res_n -= copy_len;
		}else{
			copy_len = (mem->size - offset2);
			memset(mem->mem_addr+offset2,ch,copy_len);
			res_n -= copy_len;

			size += mem->size;
			offset += copy_len;
			mem = mem->next;
		}
	}
	return s;
}

//dest need to covert to mem_chain address but src is not 
void *mem_chain_upgrade_memcpy(void *dest, const void *src, size_t n)
{
	int i,res_n = n,pos = 0,copy_len;
	int	block = -1,offset,offset2,size = 0;
	struct mem_chain_upgrade_list* mem=NULL;
	Men_Chain_Upgrade_Address_Info address_info;
	char* mem_src = src;
	
	if(mem_chain_upgrade_MemListInfo_Get(dest,n,&address_info) == -1){
		return NULL;
	};

	i = address_info.mem_list_num;
	block = address_info.block_num;
	offset = address_info.offset;
	
	mem = mem_chain_list_ptr[i]->header;
	while(block--){
		size += mem->size;
		mem = mem->next;
	}

	while(res_n){
		offset2= (offset - size) % mem->size;
		if((mem->size - offset2) >= res_n){
			copy_len = res_n;
			memcpy(mem->mem_addr+offset2,mem_src+pos,copy_len);
			res_n -= copy_len;
			pos += copy_len;
		}else{
			copy_len = (mem->size - offset2);
			memcpy(mem->mem_addr+offset2,mem_src+pos,copy_len);
			res_n -= copy_len;
			pos += copy_len;

			size += mem->size;
			offset += copy_len;
			mem = mem->next;
		}
	}

	return dest;
		
}


//buf need to convert to memory chain address
int mem_chain_upgrade_read(int fd,void *buf,size_t count)
{
	int i,block,size = 0,offset,offset2;
	int total_read = 0,total_data,post_data_available;
	Men_Chain_Upgrade_Address_Info address_info;
	struct mem_chain_upgrade_list* mem;
	
	if(mem_chain_upgrade_MemListInfo_Get(buf,count,&address_info) == -1){
		return -1;
	};

	i = address_info.mem_list_num;
	block = address_info.block_num;
	offset = address_info.offset;

	mem =  mem_chain_list_ptr[i]->header;
	while(block--){
		size += mem->size;
		mem = mem->next;
	}
	offset2 = (offset-size) % mem->size;

	if(count <= (mem->size - offset2))
		total_data = count;
	else
		total_data = (mem->size - offset2);	

	while(total_read < total_data){		
		post_data_available = read(fd,mem->mem_addr+offset2+total_read,total_data - total_read);
		if (post_data_available < 0){
        	return -1;
		}

		total_read += post_data_available;	
	}
	
	return total_data;
}

void* mem_chain_upgrade_mem_convert(char* data)
{
	char* ptr;
	Mem_Chain_Upgrade_Offset_Mem* mem_offset = mem_chain_upgrade_offset_mem_convert(data);

	if(mem_offset == NULL){
		return NULL;
	}
	
	ptr = mem_offset->mem;

	if(mem_offset){
		free(mem_offset);
		mem_offset = NULL;
	}
	return ptr;
}

int mem_chain_upgrade_node_size(char* data)
{
	int node_size;
	Mem_Chain_Upgrade_Offset_Mem* mem_offset = mem_chain_upgrade_offset_mem_convert(data);

	if(mem_offset == NULL){
		return -1;
	}
	
	node_size = mem_offset->size;

	if(mem_offset){
		free(mem_offset);
		mem_offset = NULL;
	}
	return node_size;
}

int mem_chain_upgrade_value_set(char* data,char value)
{
	char* addr = (char*)mem_chain_upgrade_mem_convert(data);
	if(addr ==  NULL)
		return -1;
	*addr = value;
	return 0;
}

char *mem_chain_upgrade_memstr(char *membuf, char *param, int memsize)
{
	int i,k = 0;
	int j = 0;

	int offset_start,offset_end,offset;
	int block_start,block_end;

	struct mem_chain_upgrade_list* ptr,*ptr2;
	struct mem_chain_upgrade_list* ptr_end,*ptr_end2;

	char charfind;
	char charmem;
	char charfindfisrt;
	char *findpos;
	int  mmsz;

	char* mem_start,*mem_start1;
	char* mempos;
	char* mem_end,*mem_end1;
	char* const_mem_start;

	int node_startsize = 0,node_endsize = 0;
	int start_loc = -1,end_loc;
	struct mem_chain_upgrade_list* heap_malloc=NULL,*misc_malloc=NULL,*clust_start=NULL;

	int len = strlen(param)+1;

	if(memsize < strlen(param))
		return NULL;

	for(i = 0;i < MEM_CHAIN_UPGRADE_LIST_NUM;i++){
		if(mem_chain_list_ptr[i] ==  NULL)
			continue;
		
		if(membuf >= mem_chain_list_ptr[i]->header->mem_addr &&
			(membuf + memsize)<= (mem_chain_list_ptr[i]->header->mem_addr + mem_chain_list_ptr[i]->size) ){
			offset = membuf - mem_chain_list_ptr[i]->header->mem_addr;

			if(mem_chain_list_ptr[i]->header != NULL){
				heap_malloc = mem_chain_list_ptr[i]->header;
				if(heap_malloc->next != NULL){
					misc_malloc = heap_malloc->next;
					if(misc_malloc->next != NULL){
						clust_start = misc_malloc->next;
					}
				}
			}

			if(heap_malloc != NULL && offset < heap_malloc->size)
				start_loc = 0;
			else if(misc_malloc != NULL && offset < (heap_malloc->size + misc_malloc->size))
				start_loc = 1;
			else if(clust_start != NULL){
				if((offset - (heap_malloc->size + misc_malloc->size)+1) % (clust_start->size) == 0)
					start_loc = (offset - (heap_malloc->size + misc_malloc->size)+1)/(clust_start->size) + 1;
				else
					start_loc = (offset - (heap_malloc->size + misc_malloc->size)+1)/(clust_start->size) + 2;//modify
			}

			if(heap_malloc != NULL && (offset+memsize) < heap_malloc->size)
				end_loc = 0;
			else if(misc_malloc != NULL && (offset+memsize) < (heap_malloc->size + misc_malloc->size))
				end_loc = 1;
			else if(clust_start != NULL){
				if(((offset+memsize) - (heap_malloc->size + misc_malloc->size) + 1) % (clust_start->size) == 0)
					end_loc = ((offset+memsize) - (heap_malloc->size + misc_malloc->size) + 1)/(clust_start->size) + 1;
				else
					end_loc = ((offset+memsize) - (heap_malloc->size + misc_malloc->size) + 1)/(clust_start->size) + 2;
			}

			ptr = mem_chain_list_ptr[i]->header;
			ptr_end = mem_chain_list_ptr[i]->header;

			break;
		}
	}

	if(start_loc == -1)
		return NULL;//may be modify

	while(end_loc--){
		node_endsize += ptr_end->size;
		ptr_end = ptr_end->next;
	}
	while(start_loc--){
		node_startsize += ptr->size;
		ptr = ptr->next;
	}

	offset_start = (offset-node_startsize) % ptr->size;
	offset_end = (offset + memsize - node_endsize) %ptr_end->size;
	
	mem_start = ptr->mem_addr + offset_start;
	mempos = ptr->mem_addr + offset_start;
	if(ptr ==  ptr_end)
		mem_end = ptr->mem_addr + offset_end;
	else
		mem_end = ptr->mem_addr + ptr->size;
	const_mem_start = membuf;

	if ((charfindfisrt = *param++) == 0) {
        	return mempos;
    }

	while(memsize-- > 0){
		charmem = *mem_start;

		if(++mem_start >= mem_end){
			ptr = ptr->next;
			mem_start = ptr->mem_addr;
			if(ptr == ptr_end)
				mem_end = ptr->mem_addr + offset_end;
			else
				mem_end = ptr->mem_addr + ptr->size;
		}

		if(charmem == charfindfisrt){
			mem_start1 = mem_start;
			mem_end1 = mem_end;
			findpos = param;
			mempos = mem_start1;
			ptr2 = ptr;

			while((charfind = *findpos++) != 0){
				if(mempos >= mem_end1){
					ptr2 = ptr2->next;
					mem_start1 =  ptr2->mem_addr;
					mempos = mem_start1;
					if(ptr2 == ptr_end)
						mem_end1 = ptr2->mem_addr + offset_end;
					else
						mem_end1 = ptr2->mem_addr + ptr2->size;
				}

				charmem = *mempos++;
				k++;

				if(charmem != charfind){
					break;
				}
			}

			if(charfind == 0){
				return (const_mem_start+j);
			}
			k = 0;
		}
		j++;
	}

	return NULL;
}

//s1 need convert to memory chain address
int mem_chain_upgrade_memcmp( const void *s1, const void *s2, size_t n)
{
	char* 	mem;
	int 	offset,size;
	int res_n = n;
	char* start_p = (char*)s1;
	char* end_p = (char*)s2;
	
	Mem_Chain_Upgrade_Offset_Mem* mem_offset;	

	while(res_n){
		mem_offset =(Mem_Chain_Upgrade_Offset_Mem*)mem_chain_upgrade_offset_mem_convert(start_p);
		if(mem_offset == NULL){
			return -1;//may be modify
		}
		
		mem = mem_offset->mem;
		offset = mem_offset->offset;
		size = mem_offset->size;

		if(mem_offset){
			free(mem_offset);
			mem_offset =NULL;
		}

		if((size - offset) >= res_n){
			return memcmp(mem,end_p,res_n);
		}
		else{
			if(memcmp(mem,end_p,size-offset) != 0){
				return memcmp(mem,end_p,size-offset);
			}
			else{
				end_p += size-offset;
				start_p += size-offset;
				res_n -= size-offset;
			}
		}
	
	}
}

void* mem_chain_upgrade_mem_str_convert(char* data,int size)
{
	int offset,node_size,start_pos = 0,res_n = size;
	char* mem,*ptr,*start_p = (char*)data;
	Mem_Chain_Upgrade_Offset_Mem* mem_offset;

	ptr = (char*)malloc(size);
	if(ptr == NULL)
		return NULL;
		
	while(res_n){
		mem_offset =(Mem_Chain_Upgrade_Offset_Mem*)mem_chain_upgrade_offset_mem_convert(start_p);
		if(mem_offset ==  NULL)
			return NULL;
		
		mem = mem_offset->mem;
		offset = mem_offset->offset;
		node_size = mem_offset->size;
		
		if(mem_offset){
			free(mem_offset);
			mem_offset =NULL;
		}

		if((node_size - offset) >= res_n){
			memcpy(ptr+start_pos,mem,res_n);
			return ptr;
		}
		else{
			memcpy(ptr+start_pos,mem,node_size - offset);
			start_pos += node_size - offset;
			start_p += node_size - offset;
			res_n -= node_size - offset;
		}
	
	}
	
}

int mem_chain_upgrade_CHECKSUM_OK(unsigned char *data, int len)
{
	int i,j,block,size = 0,offset,offset2,res_n = len,sum_len;
	Men_Chain_Upgrade_Address_Info address_info;
	struct mem_chain_upgrade_list* mem;
	unsigned char* data2;
	unsigned char sum = 0;
	
	if(mem_chain_upgrade_MemListInfo_Get(data,len,&address_info) == -1){
		return 0;
	};

	i = address_info.mem_list_num;
	block = address_info.block_num;
	offset = address_info.offset;

	mem = mem_chain_list_ptr[i]->header;
	while(block--){
		size +=  mem->size;
		mem = mem->next;
	}	

	while(res_n){
		offset2 = (offset-size) % mem->size;
		sum_len = (mem->size - offset2) >= res_n ? res_n : (mem->size - offset2);
		data2 = mem->mem_addr + offset2;
		
		for(j = 0;j < sum_len; j++)
			sum += data2[j];
		
		if(res_n <= (mem->size - offset2)){
			goto done;
		}
		else{
			res_n -= sum_len;
			size += mem->size;
			offset += sum_len;
			mem = mem->next;
		}
	}

done:
	if ((sum) == 0)
		return 1;
	else
		return 0;
}


int mem_chain_upgrade_fwChecksumOk(char *data, int len)
{	
	int i,block,size = 0,offset,offset2,check_len = 0,cache_size,copy_len,pos = 0;
	Men_Chain_Upgrade_Address_Info address_info;
	struct mem_chain_upgrade_list* mem;
	unsigned short sum=0;
	char mem_chain_fw_cache[512];
	memset(mem_chain_fw_cache,0,512);
	
	if(mem_chain_upgrade_MemListInfo_Get(data,len,&address_info) == -1){
		return 0;
	};

	i = address_info.mem_list_num;
	block = address_info.block_num;
	offset = address_info.offset;

	mem = mem_chain_list_ptr[i]->header;
	while(block--){
		size +=  mem->size;
		mem = mem->next;
	}

	while(check_len < len){
		cache_size = (len - check_len) >= 512 ? 512:(len-check_len);

		while(pos < cache_size){
			offset2 = (offset - size) % mem->size;
			
			if((mem->size - offset2) > (cache_size-pos)){
				copy_len = cache_size - pos;
				memcpy(mem_chain_fw_cache + pos,mem->mem_addr+offset2,copy_len);
				offset += copy_len;
				pos += copy_len;
			}else{
				copy_len = (mem->size - offset2);
				memcpy(mem_chain_fw_cache + pos,mem->mem_addr+offset2,copy_len);
				offset += copy_len;
				pos += copy_len;
				
				size += mem->size;
				mem = mem->next;
			}

		}

		for(i = 0;i < cache_size;i += 2){
			#ifdef _LITTLE_ENDIAN_
				sum += WORD_SWAP( *((unsigned short *)&mem_chain_fw_cache[i]) );
			#else
				sum += *((unsigned short *)&mem_chain_fw_cache[i]);
			#endif
		}

		pos = 0;
		check_len += cache_size;
		memset(mem_chain_fw_cache,0,512);	
		
	}
		
	return( (sum==0) ? 1 : 0);
	
}

int  mem_chain_upgrade_flash_write_count(unsigned int block_size,unsigned int* offset,unsigned char* ram,unsigned int write_count,unsigned int* this_write)
{
	char* mem;
	int flash_block,mem_block,orig_offset = *offset;
	
	Mem_Chain_Upgrade_Offset_Mem* mem_offset = mem_chain_upgrade_offset_mem_convert((char*)ram);
	if(mem_offset == NULL)
		return -1;
	
	flash_block = block_size - orig_offset;
	mem_block = mem_offset->size - mem_offset->offset;
	mem = mem_offset->mem;

	if(mem_offset){
		free(mem_offset);
		mem_offset = NULL;
	}

	if(mem_block <= flash_block){
		*this_write = mem_block;
	}else{
		*this_write = flash_block;
	}
	
	if(*this_write > write_count)
		*this_write = write_count;
	
	*offset = (*this_write + orig_offset) % block_size;
	return 0;
	
}

void mem_chain_upgrade_free(void* p)
{
	int i,j = 1;
	//Mem_Chain_Upgrade_Header*		head;
	struct mem_chain_upgrade_list*	heap_malloc = NULL,*misc_malloc = NULL;
	struct mem_chain_upgrade_list*	tmp;
	union  mcluster* cluster;

	for(i = 0;i< MEM_CHAIN_UPGRADE_LIST_NUM;i++){
		if(mem_chain_list_ptr[i] == NULL)
			continue;
	
		if((char*)p == mem_chain_list_ptr[i]->header->mem_addr){
			//head = mem_chain_list_ptr[i];
			heap_malloc = mem_chain_list_ptr[i]->header;
			tmp = heap_malloc;
			break;
		}
	}

	if(i == MEM_CHAIN_UPGRADE_LIST_NUM){
		MEM_CHAIN_UPGRADE_ERR("%x is not malloc by memory chain,free error,Please check\n",(unsigned long)p);
		return;
	}

	while(heap_malloc->next != NULL){
		while(tmp->next != NULL){
			if(tmp->next->next == NULL)
				break;
			else
				tmp = tmp->next;
		}

		if(heap_malloc->next->next == NULL){
			/*cyg_net_free*/
			extern void mem_chain_upgrade_free_miscpoll(void * buf);
			mem_chain_upgrade_free_miscpoll(heap_malloc->next->mem_addr);
		}else{
			/*cluster free*/			
			extern void mem_chain_upgrade_free_cluster(void* buf);
			mem_chain_upgrade_free_cluster(tmp->next->mem_addr);
		}
		
		free(tmp->next);
		tmp->next = NULL;
		
		tmp = heap_malloc;
	}

	if(heap_malloc !=  NULL){
		if(heap_malloc->mem_addr != NULL){
			free(heap_malloc->mem_addr);
			heap_malloc->mem_addr = NULL;
		}
		free(heap_malloc);
		heap_malloc = NULL;
	}

	if(mem_chain_list_ptr[i] != NULL){
		mem_chain_list_ptr[i]->header = NULL;
		free(mem_chain_list_ptr[i]);
		mem_chain_list_ptr[i] = NULL;
	}
	
}

int mem_chain_support_rtk_flash_write(char *buf, int offset, int len)
{
	int stat;
	cyg_flashaddr_t err_addr;
	//erase
	/*
	if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+offset), len, &err_addr)) != CYG_FLASH_ERR_OK) {
            printf("FLASH: erase failed: %s %p\n", flash_errmsg(stat), (void *)err_addr);
            return 0;
        }
   */
    //program
	if ((stat = mem_chain_support_cyg_flash_program((FLASH_BASE_ADDR+offset), (void *)buf, len, &err_addr)) != CYG_FLASH_ERR_OK) {
            printf("FLASH: program failed: %s %p\n", flash_errmsg(stat), (void *)err_addr);
            return 0;
        }
	return 1;
}

#if defined(HAVE_FIREWALL)
void reset_firewall_forward()
{
	//diag_printf("in reset_firewall_forward,wan_status:%d.\n",wan_connected);
	extern cyg_flag_t sys_flag;
	if(wan_connected == 1){
		kick_event(FIREWARE_EVENT);

#if defined(CYGPKG_NET_OPENBSD_STACK)
		extern int ipforwarding;
		ipforwarding = 1;
#elif defined(CYGPKG_NET_FREEBSD_STACK)
		extern int cyg_ipforwarding;
		cyg_ipforwarding = 1;
#endif
	}
}

void set_firewall_forward(void)
{
	#define NULL_FILE 0
	#define NULL_STR ""
	#define IP_ADDR	0

	struct in_addr addr;
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];
	getInterfaces(lan_intf,wan_intf);
	
	if ((getInAddr(wan_intf, IP_ADDR, (void *)&addr)==0) || (addr.s_addr == 0))
		wan_connected = 0;
	else
		wan_connected = 1;

	if(wan_connected == 1){
		RunSystemCmd(NULL_FILE,"ipfw", "flush",NULL_STR);
		RunSystemCmd(NULL_FILE,"ipfw", "add","allow","ip","from","any","to","any",NULL_STR);
#if defined(CYGPKG_NET_OPENBSD_STACK)
		extern int ipforwarding;
		ipforwarding = 0;
#elif defined(CYGPKG_NET_FREEBSD_STACK)
		extern int cyg_ipforwarding;
		cyg_ipforwarding = 0;
#endif
	}
}
#endif
#endif

#ifdef CONFIG_RTL_PHY_POWER_CTRL
void powerOnOffWlanInf()
{
	 int i;
	 char wlan_name[16] = {0};
	 char virtual_interface[64] = {0};
	 char *token=NULL, *savestr1=NULL;
	 char tmpBuff[64];
	 int wlan_disabled;
	 int wlanBand2G5GSelect;
	 int op_mode;
	 
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif	 
	 apmib_get(MIB_WLAN_BAND2G5G_SELECT,(void *)&wlanBand2G5GSelect);
	apmib_get(MIB_OP_MODE,(void*)&op_mode);

	 if(op_mode!=WISP_MODE)
	 {
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0-va0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0-vxd0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1-va0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1-vxd0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1", "down", NULL_STR);

			for(i=0;i<NUM_WLAN_INTERFACE;i++)
			{
				if( (wlanBand2G5GSelect != BANDMODEBOTH) && (i==1))
				{
					break;
				}

				//up root interface
				sprintf(wlan_name, "wlan%d",i);
				SetWlan_idx(wlan_name);
				apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);
				if(wlan_disabled)
				{
					continue;
				}
#ifdef HAVE_WLAN_SCHEDULE
				if(check_wlSchedule() != 1)
				{
					continue;
				}
#endif
				RunSystemCmd(NULL_FILE, "ifconfig", wlan_name, "up", NULL_STR);

				// up vxd interface
				sprintf(wlan_name, "wlan%d-vxd0",i);
				SetWlan_idx(wlan_name);
				apmib_get(MIB_WLAN_WLAN_DISABLED,(void *)&wlan_disabled);
				if(!wlan_disabled)
				{
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_name, "up", NULL_STR);
				}
				
				//up virtual AP interface
				sprintf(wlan_name,"wlan%d-va0",i);
				SetWlan_idx(wlan_name);
				apmib_get(MIB_WLAN_WLAN_DISABLED,(void *)&wlan_disabled);
				if(!wlan_disabled)
				{
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_name, "up", NULL_STR);
				}			
				
			}
	 }

#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif	 
}

void powerOnOffLanInf()
{
	int op_mode;
	apmib_get(MIB_OP_MODE,(void*)&op_mode);
	
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "off", NULL_STR);

#ifndef KLD_ENABLED
	int wlan_idx, wlan_mode;
	char wlan_ifname[16];
#endif

#if 0
	if(op_mode!=WISP_MODE)
	{
#ifndef KLD_ENABLED
		if(op_mode==WISP_MODE)
		{		
			apmib_save_idx();
			for(wlan_idx=0; wlan_idx<NUM_WLAN_INTERFACE; wlan_idx++)
			{			
				apmib_set_wlanidx(wlan_idx);			
				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				if(wlan_mode!=CLIENT_MODE)
				{
					sprintf(wlan_ifname, "wlan%d", wlan_idx);
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_ifname, "down", NULL_STR);
				}
			}
			apmib_revert_idx();
		}	
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1", "down", NULL_STR);
		}
	}
#endif

	if(op_mode==BRIDGE_MODE)
		RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "off", NULL_STR);
	
	sleep(3);
	
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "on", NULL_STR);
#if 0
	if(op_mode!=WISP_MODE)
	{
#ifndef KLD_ENABLED
		if(op_mode==WISP_MODE)
		{		
			apmib_save_idx();
			for(wlan_idx=0; wlan_idx<NUM_WLAN_INTERFACE; wlan_idx++)
			{			
				apmib_set_wlanidx(wlan_idx);			
				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				if(wlan_mode!=CLIENT_MODE)
				{
					sprintf(wlan_ifname, "wlan%d", wlan_idx);
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_ifname, "up", NULL_STR);
				}
			}
			apmib_revert_idx();
		}	
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "up", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1", "up", NULL_STR);
		}
	}
#endif
	if(op_mode==BRIDGE_MODE)
		RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "on", NULL_STR);
}
#endif


