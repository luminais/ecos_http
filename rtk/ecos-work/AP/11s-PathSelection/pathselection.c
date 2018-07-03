#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>

#include <net/if_arp.h>

#ifndef __ECOS
#include <linux/wireless.h>
#else
#include <net/if.h>			/* for IFNAMSIZ and co... */
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#endif


#ifndef __ECOS
#if __GLIBC__ >= 2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>     /* the L2 protocols */
#else
#include <linux/if_packet.h>
#include <linux/if_ether.h>   /* The L2 protocols */
#endif
#else
#include <net/if_dl.h>
#endif



#include <time.h>
#include <sys/time.h>
#include <network.h>


//#include "1x_ioctl.h"

#ifdef _RTL8187B_ 
#include "1x_pathsel.h"
#endif			

#ifdef _RTL8186_
#include "../auth/include/1x_ioctl.h"
#endif

#include "pathselection.h"
#include "Tree.h"
#include "util_PathUpdate.h"

char interface_ioctl[IFNAMSIZ] = {0};
char interface_rawsocket[IFNAMSIZ] = {0};
unsigned char my_address[MACADDRLEN];


static struct pathsel_context pathsel_cnt;
static struct pathsel_context *hapd = &pathsel_cnt;

int pathSwitchThreshold = 20;

int pathsel_sndpkt_timeout = 100;
struct callout pathsel_timer;

cyg_flag_t pathsel_flag;
cyg_handle_t pathsel_alarm_hdl;
cyg_alarm pathsel_alarm;


#define IPC_BASED_PATHSEL
#define PATHSEL_TIMER_INTERVAL 1

//===================================================================================================
//following added by shlu_2007_01_16
unsigned char PreMAC[MACADDRLEN]={0};
//unsigned short Hello_random=0;
//#define  PATH_OnDemand_Debug 1                   //Path selection debug used

void TimerFunc();
int My_interFace=0;
int gen_rreq(unsigned char *DesAdd,unsigned char TTL,unsigned short SeqNum);
int gen_rerr(unsigned char* DesAddr,unsigned char* SrcAddr,unsigned char TTL,unsigned short SeqNum,unsigned char RERR_flag); 
int recv_rreq(struct rreq *input,unsigned char* My_Prehop,unsigned int My_Prehop_metric,int My_interFace,unsigned char TTL,unsigned short SeqNum);

int recv_rrep(struct rrep *input_rrep, unsigned char* My_Prehop,unsigned int My_Prehop_metric,int My_interFace,unsigned char TTL,unsigned short SeqNum, unsigned char Is6AddrFormat, unsigned char *addr5 ,unsigned char *addr6 );

int recv_rerr(struct rerr *input_rerr, unsigned char* PreHopAdd,unsigned char TTL,unsigned short SeqNum, unsigned char* DesAdd);
//int recv_rrep_ack(struct rrep_ack *input_rrep_ack, unsigned char* My_Address,unsigned char TTL,unsigned short SeqNum);


// int update_proxy_table(unsigned char *STAaddr, unsigned char *OWNERaddr);
extern void reset_timer();

int recv_rann(struct rann *, unsigned char*, unsigned int, int, int, int );
int recv_pann(struct pann *, unsigned char*, unsigned int, int, int, int);
int init_pann(void);


u_int64_t macAddr_atoi(unsigned char *a) {
    u_int64_t i = 0;
    int k;
    for (k = 0; k < MACADDRLEN; k++) {
        i |= (a[k] << ((MACADDRLEN-k-1)*8));
    }
    return i;
}

#ifndef __ECOS
static int pidfile_acquire(char *pidfile)
{
    int pid_fd;

    if(pidfile == NULL)
        return -1;

    pid_fd = open(pidfile, O_CREAT | O_WRONLY, 0644);
    if (pid_fd < 0) 
        printf("Unable to open pidfile %s\n", pidfile);
    else 
        lockf(pid_fd, F_LOCK, 0);

    return pid_fd;
}

static void pidfile_write_release(int pid_fd)
{
    FILE *out;

    if(pid_fd < 0)
        return;

    if((out = fdopen(pid_fd, "w")) != NULL) {
        fprintf(out, "%d\n", getpid());
        fclose(out);
    }
    lockf(pid_fd, F_UNLCK, 0);
    close(pid_fd);
}
#endif


static int get_iface_name(int fd, const char* interface_name, unsigned char *mac_addr){
	struct ifreq ifr;
	memset(&ifr, 0, sizeof(ifr));
	strcpy(ifr.ifr_name, interface_name);
	if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)  {
		// printf("Get MAC addr failed\n");
		return -1;
	}
	memcpy(mac_addr, ifr.ifr_hwaddr.sa_data, 6);
	return 1;
}


//====================================================================================================
/*------------------------------------------------------------------*/

/*

 * Open a socket.

 * Depending on the protocol present, open the right socket. The socket

 * will allow us to talk to the driver.

 */

int
sockets_open(void)
{

    int ipx_sock = -1;              /* IPX socket                   */

    int ax25_sock = -1;             /* AX.25 socket                 */

    int inet_sock = -1;             /* INET socket                  */

    int ddp_sock = -1;              /* Appletalk DDP socket         */



    /*
    * Now pick any (exisiting) useful socket family for generic queries
    * Note : don't open all the socket, only returns when one matches,
    * all protocols might not be valid.
    * Workaround by Jim Kaba <jkaba@sarnoff.com>
    * Note : in 99% of the case, we will just open the inet_sock.
    * The remaining 1% case are not fully correct...
    */

    inet_sock=socket(AF_INET, SOCK_DGRAM, 0);

    if(inet_sock!=-1)
        return inet_sock;

    ipx_sock=socket(AF_IPX, SOCK_DGRAM, 0);

    if(ipx_sock!=-1)
        return ipx_sock;

#ifndef __ECOS
    ax25_sock=socket(AF_AX25, SOCK_DGRAM, 0);

    if(ax25_sock!=-1)
        return ax25_sock;
#endif
    ddp_sock=socket(AF_APPLETALK, SOCK_DGRAM, 0);

    /*
    * If this is -1 we have no known network layers and its time to jump.
    */

    return ddp_sock;

}

#define printMac(addr)	printf("%02x%02x%02x%02x%02x%02x", \
		0xff& addr[0],			0xff& addr[1],\
		0xff& addr[2],			0xff& addr[3],\
		0xff& addr[4],			0xff& addr[5] );

//#define GALLARDO_TEST



//Add by bc5678 2007.01.19, for 11's path selection
//mark by chuangch 2007.09.14
/*
int dump_table()
{
	struct iwreq          wrq;
	if(g_rawsocket_fd < 0) {
		// printf("socket fd in query_table is not ready\n");
		return -1;
	}
	// Get wireless name 
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
 	
	if((wrq.u.data.pointer = (caddr_t)malloc(PATH_SEL_TBL_SIZE*sizeof(struct path_sel_entry)))==NULL)
	{
    	// printf("dump_path_table  Error:%s\n\a",strerror(errno)); 
		return -1;
	}
	wrq.u.data.length = PATH_SEL_TBL_SIZE*sizeof(struct path_sel_entry);
	
  	if(ioctl(g_ioctrl_socket_fd, DUMP_PATHTABLE, &wrq) < 0) {
    	// If no wireless name : no wireless extensions
    	free(wrq.u.data.pointer);
    	// printf("dump_path_table  Error:%s\n\a",strerror(errno)); 
    	return(-1);
	}
	else {
		int k, m;
		struct path_sel_entry *line_entry = (struct path_sel_entry *)wrq.u.data.pointer;
//		for (k = 0; k < PATH_SEL_TBL_SIZE; k++) {
		for (k = 0; k < 16; k++) {
			if (line_entry != 0) {
				// print a line of path selection table entry
				// printf("Path Table Entry %d : \n",k);
    			// printf("   isvalid = %d", line_entry->isvalid);
    			// printf("   destMAC = ");
				for (m = 0; m < MACADDRLEN; m++) {
    				// printf("%2X ", line_entry->destMAC[m]);
				}
    			// printf("   dsn = %d", line_entry->dsn);
    			// printf("   nexthopMAC = ");
				for (m = 0; m < MACADDRLEN; m++) {
    				// printf("%2X ", line_entry->nexthopMAC[m]);
				}
    			// printf("   metric = %d", line_entry->metric);
    			// printf("   hopcount = %d", line_entry->hopcount);
    			// printf("   modify_time = ");
				for (m = 0; m < 8; m++) {
    				// printf("%2X ", line_entry->modify_time[m]);
				}
				// printf("\n");
			}
			line_entry += 1;
		}
	}
	
	free(wrq.u.data.pointer);
	return 1;
}

int dump_neighbor_table()
{
	struct iwreq          wrq;
	if(g_rawsocket_fd < 0) {
		// printf("socket fd in query_table is not ready\n");
		return -1;
	}
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
 	
	if((wrq.u.data.pointer = (caddr_t)malloc(NUM_STAT*sizeof(struct MESH_Neighbor_Entry)))==NULL)
	{
		// printf("dump_nbr_table  Error:%s\n\a",strerror(errno)); 
		return(-1);
	}
	wrq.u.data.length = NUM_STAT*sizeof(struct MESH_Neighbor_Entry);
	
	if(ioctl(g_ioctrl_socket_fd, DUMP_NBRTABLE, &wrq) < 0) {
		// If no wireless name : no wireless extensions
		free(wrq.u.data.pointer);
		// printf("dump_nbr_table  Error:%s\n\a",strerror(errno)); 
		return(-1);
	}
	else {
		int k; // , m;
		struct MESH_Neighbor_Entry *line_entry = (struct MESH_Neighbor_Entry *)wrq.u.data.pointer;
		
		for (k = 0; k < 16; k++) {
			if (line_entry != 0) {
				// print a line of path selection table entry
				// printf("Neighbor Table Entry %d : \n",k);
				// printf("   State = %d", line_entry->State);
				// printf("   Directionality = %ud", line_entry->Directionality);
				// printf("   Operating Channel = %d", line_entry->Co);
				// printf("   CPI = %d", line_entry->Pl);
				// printf("   byte rate = %d", line_entry->r);
				// printf("   error rate = %d", line_entry->ept);
				// printf("   Strenth or quality = %d", line_entry->Q);
				// printf("   Alive time = %d", (int)line_entry->expire);
				// printf("\n");
			}
			line_entry += 1;
		}
	}
	
	free(wrq.u.data.pointer);
	return 1;
}
*/

/*

#define RTL8651_IOCTL_GETLANLINKSTATUSALL 2105

static int re865xIoctl(char *name, unsigned int arg0, unsigned int arg1, unsigned int arg2, unsigned int arg3)
{
	unsigned int args[4];
	struct ifreq ifr;
	int sockfd;
	args[0] = arg0;
	args[1] = arg1;
	args[2] = arg2;
	args[3] = arg3;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {      perror("fatal error socket\n");
                return -3;
    }
	strcpy((char*)&ifr.ifr_name, name);
	((unsigned int *)(&ifr.ifr_data))[0] = (unsigned int)args;
	if (ioctl(sockfd, SIOCDEVPRIVATE, &ifr)<0)
    {
        perror("device ioctl:");
        close(sockfd);
        return -1;
    }
	close(sockfd);
	return 0;
} 

int getEtherPortLink()
{
	int    ret;
	unsigned int    args[0];

    re865xIoctl("eth0", RTL8651_IOCTL_GETLANLINKSTATUSALL, (unsigned int)(args), 0, &ret);

	//fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>INIT check port status %d\n", ret);
	
	if (ret<0)
		return 0;
	else 
		return 1;

}

*/



#define BRCTL_SET_MESH_PATHSELPID 111
#define BRCTL_GET_PORTSTAT 112
#define CNT_REFRESH_STATION 3

/*
int update_proxy_table(unsigned char STAaddr[MACADDRLEN],unsigned char OWNERaddr[MACADDRLEN])
{
	char buf[2*MACADDRLEN];
	
	struct iwreq          wrq;
	if(g_rawsocket_fd < 0) {
		// printf("socket fd in query_table is not ready\n");
		return -1;
	}
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
 	
	wrq.u.data.pointer = (caddr_t)buf;
	wrq.u.data.length = sizeof(buf);
	
	memcpy(wrq.u.data.pointer,STAaddr,MACADDRLEN);
	memcpy(wrq.u.data.pointer+MACADDRLEN,OWNERaddr,MACADDRLEN);
	
	if(ioctl(g_ioctrl_socket_fd, UPDATE_PROXY_TABLE, &wrq) < 0) {
		// If no wireless name : no wireless extensions
		// printf("Update proxy table error:%s\n\a",strerror(errno)); 
		return(-1);
	}
	
	return 0;
}
*/

int update_root_info(unsigned char root_addr[MACADDRLEN])
{
	
	//printf("ioctl notify_path_found\n\a"); 
	struct iwreq          wrq;
	char buf[MACADDRLEN];
	//int mem_size = 0;
	

	// Get wireless name 
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
	wrq.u.data.length = MACADDRLEN ;
	wrq.u.data.pointer = (caddr_t)buf;
	
	memcpy(wrq.u.data.pointer, root_addr, MACADDRLEN);

	if(ioctl(g_ioctrl_socket_fd, SIOC_UPDATE_ROOT_INFO, &wrq) < 0)
	{
		// If no wireless name : no wireless extensions
		// printf("Update_Root  Error:%s\n\a",strerror(errno)); 
		return(-1);
	}
	
	return 0;
}

#if defined(CONFIG_RTL_MESH_AUTOPORTAL_SUPPORT)
int ini_route_setting(int* pIsPortal,int* pToBeRoot)
{

#ifdef _RTL8187B_ 
	; // use arg, hence do nothing
#else // use ioctl
	
	unsigned char buf[128]; //should the same with the size defined in wireless_ag's ioctl.c		
	struct iwreq          wrq;

	if(g_rawsocket_fd < 0)
	{
		// printf("socket fd in query_table is not ready\n");
		return -1;
    }
    
	// Get wireless name 
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
  
	wrq.u.data.length = sizeof(buf);
	wrq.u.data.pointer = (caddr_t)buf;
	
	if(ioctl(g_ioctrl_socket_fd, SIOC_GET_ROUTING_INFO, &wrq) < 0)
	{
		// printf("SIOC_GET_ROUTING_INFO  Error:%s\n\a",strerror(errno)); 
		return(-2);
	}
	else
	{
		//memcpy(buf, wrq.u.data.pointer, 128);
		*pToBeRoot = (int)buf[0];
		*pIsPortal = (int)buf[1];
//		pathSwitchThreshold = (int)buf[2];
		// printf(" \n************pathSwitchThreshold=%d\n ", pathSwitchThreshold);
	}

#endif

	return 0;
}
#endif

#ifndef __ECOS
#ifndef IPC_BASED_PATHSEL
static int pathsel_init_fifo()
{

	/* Here is an assumption that the fifo is created already by iwcontrol */
 
	int flags;
	struct stat status;

	if(stat(PATHSEL_FIFO, &status) == 0)
		unlink(PATHSEL_FIFO);
	if((mkfifo(PATHSEL_FIFO, FILE_MODE) < 0)){
		// printf("mkfifo %s fifo error: %s!\n", DAEMON_FIFO, strerror(errno));
		return -1;
	}
	if((hapd->readfifo = open(PATHSEL_FIFO, O_RDONLY | O_NONBLOCK, 0)) < 0)
	{
		// printf("pathsel_init_fifo readfifo error\n");
	}

	HOSTAPD_DEBUG(HOSTAPD_DEBUG_MSGDUMPS, "hapd->readfifo = %d\n", hapd->readfifo);

	if ((flags = fcntl(hapd->readfifo, F_GETFL, 0)) < 0) {
		return -1;
	}
	else {
		flags |= O_NONBLOCK;
		if ((flags = fcntl(hapd->readfifo, F_SETFL, flags)) < 0) {
			// printf("F_SETFL: error\n");
			return -1;
		}
	}

	return 0;
}
#endif
#endif

//#ifdef IPC_BASED_PATHSEL

int gHasData = 0;
int iw_sockets_open_pathsel(void)
{
#ifndef __ECOS
	static const int families[] = {
		AF_INET, AF_IPX, AF_AX25, AF_APPLETALK
	};
#else
	static const int families[] = {
    	AF_INET
  	};
#endif
	unsigned int  i;
	int           sock;


	/* Try all families we support */
	for(i = 0; i < sizeof(families)/sizeof(int); ++i)
	{
		/* Try to open the socket, if success returns it */
		sock = socket(families[i], SOCK_DGRAM, 0);
		if(sock >= 0)
			return sock;
	}

	return -1;
}

#ifdef IPC_BASED_PATHSEL
void callback_usr1()
{
	gHasData = 1;
#ifndef __ECOS
	signal(SIGUSR1, SIG_IGN);
#endif
}

int SendPid()
{
	struct iwreq pidwrq;
    int ret;
	char pidbuf[6];
		
	int sd;
	int pid = getpid();
	if (pid == 0)
		return -1;
    sd = iw_sockets_open_pathsel();
    if(sd == -1) {
        return -1;
    }
    
	pidwrq.u.data.pointer = (caddr_t)pidbuf;	
	sprintf(pidbuf,"%d",pid);
	pidwrq.u.data.length = (__u16)strlen(pidbuf);
 	strncpy(pidwrq.ifr_name, interface_ioctl, IFNAMSIZ);
    ret = ioctl(sd, SAVEPID_IOCTL, &pidwrq);
	if(sd>=0)
		close(sd);				      
    return ret;
}

void dequeue_from_driver(struct iwreq wrq, char* buf)
{	
	int sd;	
	sd = iw_sockets_open_pathsel();	   
    strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);
    ioctl(sd, DEQUEUEDATA_IOCTL, &wrq);
    if( strcmp(buf, "Queue is empty") == 0 ){
        gHasData = 0;
#ifndef __ECOS
        signal(SIGUSR1, callback_usr1);
#endif
	}    
	if(sd>=0)
		close(sd);		
}


struct iwreq datawrq;
unsigned char databuf[256];


#endif // IPC_BASE_PATHSEL

static void do_daemon()
{
#ifndef IPC_BASED_PATHSEL
	int  nRead;
#endif
	unsigned char DesAdd[MACADDRLEN]={0};
	unsigned char SrcAdd[MACADDRLEN]={0};
	
	char *RecvQueueBuf;
#ifdef __ECOS
	int val;
	cyg_flag_init(&pathsel_flag);
#endif	
	while (1)
	{
#ifdef IPC_BASED_PATHSEL
#ifndef __ECOS
		pause();
#endif
		RecvQueueBuf = (char *)databuf;

#ifdef __ECOS
		val = cyg_flag_wait(&pathsel_flag, 0x7f, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
		if(val & 0x01)
#else			
		while(gHasData == 1)
#endif
		{	
			datawrq.u.data.length = sizeof(databuf);
			datawrq.u.data.pointer = (caddr_t) databuf;
			dequeue_from_driver(datawrq, (char *)databuf);
#else
			RecvQueueBuf = (char *)&hapd->RecvBuf[5];
			nRead = read(hapd->readfifo, hapd->RecvBuf, MAX_MSG_SIZE);
			if (nRead > 0)
			{
#endif						
				if(RecvQueueBuf[0]> 53) {
				struct rreq *ptr1;
				struct rrep *ptr2;
				struct rerr *ptr3;
				struct pann *pann_ptr;
				struct rann *rann_ptr;
				
				unsigned char PreHopAdd[MACADDRLEN]={0},TTL=0,RERR_flag=0;
				unsigned char Addr5[MACADDRLEN]={0}, Addr6[MACADDRLEN]={0};
				unsigned short SeqNum=0;
				unsigned int PreMetric=0;
				
				DOT11s_GEN_RREQ_PACKET *ptrA = NULL;
				DOT11s_GEN_RERR_PACKET *ptrB = NULL;
				DOT11s_RECV_PACKET *ptrC = NULL;
				
				ptrA = (DOT11s_GEN_RREQ_PACKET *)(RecvQueueBuf);
				ptrB = (DOT11s_GEN_RERR_PACKET *)(RecvQueueBuf);
				ptrC = (DOT11s_RECV_PACKET *)(RecvQueueBuf);
				
				memcpy(PreHopAdd, ptrC->PreHopMACAddr,MACADDRLEN);
				PreMetric=ptrC->Pre_Hop_Metric;
				TTL = ptrC->TTL;
				SeqNum = ptrC->Seq_num;
				//printf("*************%s %d :val=%d RecvQueueBuf[0]=%d******************\n",__FUNCTION__,__LINE__,val,RecvQueueBuf[0]);
				switch(RecvQueueBuf[0])
				{
					case DOT11_EVENT_PATHSEL_GEN_RREQ:								//gen rreq   54
#ifdef PATHSEL_DAEMON_DEBUG
						//printf("gen RREQ\n%d  %d\n",ptrA->TTL,ptrA->Seq_num);
#endif
						memcpy(DesAdd,ptrA->destMACAddr,MACADDRLEN);
						TTL = ptrA->TTL;
						SeqNum = ptrA->Seq_num;
						gen_rreq(DesAdd,TTL,SeqNum);
						break;

					case DOT11_EVENT_PATHSEL_GEN_RERR:								//gen rerr   55
#ifdef PATHSEL_DAEMON_DEBUG
						//printf("gen RERR\n");
#endif
						memcpy(DesAdd,ptrB->SorNMACAddr,MACADDRLEN);
						memcpy(SrcAdd,ptrB->DataDestMAC,MACADDRLEN);
						memcpy(PreMAC,ptrB->PrehopMAC,MACADDRLEN);
						RERR_flag = ptrB->Flag;
						TTL = ptrB->TTL;
						SeqNum = ptrB->Seq_num;
						gen_rerr(DesAdd,SrcAdd,TTL,SeqNum,RERR_flag);   
						break;
					case DOT11_EVENT_PATHSEL_RECV_RREQ:                  //Recv RREQ  56
#ifdef PATHSEL_DAEMON_DEBUG
						//printf("Receive RREQ\n");
#endif
						ptr1=(struct rreq*) (ptrC->ReceiveData);
						recv_rreq(ptr1, PreHopAdd, PreMetric, My_interFace,TTL,SeqNum);
						break;

					case DOT11_EVENT_PATHSEL_RECV_RREP:									//Recv RREP  57		
#ifdef PATHSEL_DAEMON_DEBUG
						//printf("Receive RREP\n");
#endif
						ptr2=(struct rrep*)(ptrC->ReceiveData);	
						memcpy(Addr5,ptrC->MACAddr5,MACADDRLEN);
						memcpy(Addr6,ptrC->MACAddr6,MACADDRLEN);
						recv_rrep(ptr2, PreHopAdd, PreMetric, My_interFace,TTL,SeqNum,ptrC->Is6AddrFormat,Addr5,Addr6);
						break;
							
					case DOT11_EVENT_PATHSEL_RECV_RERR:									//Recv RERR  58
#ifdef PATHSEL_DAEMON_DEBUG
						//printf("Receive RERR\n");
#endif
						ptr3=(struct rerr*)(ptrC->ReceiveData);	
						memcpy(DesAdd,ptrC->DesMACAddr,MACADDRLEN);
						recv_rerr(ptr3, PreHopAdd, TTL, SeqNum, DesAdd);
						break;		
					
					case DOT11_EVENT_PATHSEL_RECV_PANN:		//add by mcinnis 20070415
						pann_ptr = (struct pann *)(ptrC->ReceiveData);
						recv_pann(pann_ptr, PreHopAdd, PreMetric, My_interFace, TTL, SeqNum);
						break;
					
					case DOT11_EVENT_PATHSEL_RECV_RANN:		//add by chuangch_ 20070507
						rann_ptr = (struct rann *)(ptrC->ReceiveData);
						recv_rann(rann_ptr, PreHopAdd, PreMetric, My_interFace, TTL, SeqNum);
						break;
							
					default:
//#ifdef PATHSEL_DAEMON_DEBUG
						printf("Receive undefine frame in pathslection.c\n");
//#endif
						break;
				}
#ifdef PATH_OnDemand_Debug
					
				//dump_table();
 				//chuangch 2007.09.14
#endif
			} //end of switch
		} //end of if
	}   //end of while
	
}



/*
void test_rawsocket_send(int signo)
{

	//printf("Catch a signal -- SIGALRM1 \n");
	

//------------!!! filled in correct number !!!-----------------//	
//======= the destination is means destination for RREQ want to find============//
// change by bc5678 2007.1.16
	unsigned char dest_mac_addr[6] = {0x00, 0xE0, 0x4C, 0x86, 0x01, 0x97};
	unsigned char nexthop_mac_addr[6] = {0x00, 0xE0, 0x4C, 0x86, 0x01, 0x97};
	unsigned char src_mac_addr[6] = {0x00, 0xE0, 0x4C, 0x86, 0x90, 0x5D};
	unsigned char *ra, *ta, *da, *sa;
	int i;
	// unsigned char broadcast_mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 

	// create socket
	/ *
	if ((s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		// fprintf(stderr, "Error: can't socket()\n");
	}
	* /
/ *
	if ((g_rawsocket_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		// fprintf(stderr, "Error: can't socket()\n");
	}
	* /
	struct sockaddr_ll sll; // target address
	int send_result = 0;

	memset(&sll, 0, sizeof(sll));

	// prepare sockaddr_ll

	// index of the network device
	//sll.sll_ifindex	= get_iface_index(g_rawsocket_fd, interface_ioctl);
	sll.sll_ifindex	= get_iface_index(g_rawsocket_fd, interface_rawsocket);
	// address length
	sll.sll_halen = ETH_ALEN; // 6 

	sll.sll_family = PF_PACKET;

//	unsigned char meshframe[1024]; // buffer for mesh frame
//	memset(meshframe, 0, sizeof(meshframe));
	
	// MAC - begin
	memset(sll.sll_addr, 0, 8);
	memset(sll.sll_addr, 0xff, 6);
	memcpy(sll.sll_addr, src_mac_addr, 6);
	// MAC - end

//	sll.sll_protocol = htons(ETH_P_IP);	
//	sll.sll_protocol = ETH_P_ALL;	
	// sll.sll_protocol = htons(ETH_P_ALL);	
	sll.sll_protocol = 0;	


	// ARP hardware identifier is ethernet
	// sll.sll_hatype = ARPHRD_ETHER;
	sll.sll_hatype = 0;

	// target is another host
	//sll.sll_pkttype = PACKET_OTHERHOST;
	sll.sll_pkttype = 0;

// ====================== change by bc5678
	int zero_offset = 14;
	unsigned char meshframe[256]; // buffer for mesh frame
	memset(meshframe, 0x0, sizeof(meshframe));

	// Type & Subtype
    SetFrameSubType(meshframe+zero_offset, WIFI_11S_MESH_ACTION);
    SetToDs(meshframe+zero_offset);
    SetFrDs(meshframe+zero_offset);

    //      next hop
    ra = GetAddr1Ptr(meshframe+zero_offset);
    memcpy(ra, nexthop_mac_addr, 6);

    // transmitter = source = self
    ta = GetAddr2Ptr(meshframe+zero_offset);
    sa = GetAddr4Ptr(meshframe+zero_offset);
    for (i = 0; i < 6;i++) {
            ta[i] = sa[i] = src_mac_addr[i];
    }

    // destination addr
    da = GetAddr3Ptr(meshframe+zero_offset);
    memcpy(da, dest_mac_addr, 6);


	// rreq	
	
 

	struct rreq * ptr = (struct rreq*)(meshframe+zero_offset+34);

	//modify by chuangch 2007/04/03

  ptr->Category     =5;
	ptr->Action     =2;
	ptr->Id         =0;    //T.B.D
	ptr->Length         =32;
	ptr->ModeFlags  =1;
	ptr->HopCount       =0;
	ptr->ttl        =255;
	//ptr->DesCount       =1;
	ptr->RreqId     = 0;
	//ptr->SourceAddress = macAddr_atoi(src_mac_addr);
	ptr->SourceSeqNum   = 0;
	ptr->Lifetime = 255;
	ptr->Metric     =0;


//	memset(meshframe, 0xD1, 6);
//	memcpy(meshframe+6, src_mac_addr, 6);
//	memset(meshframe+12, 0xEA, 2);
//	memset(meshframe+14, 0xFD, 50);

	// send the packet
	if ((send_result = sendto(g_rawsocket_fd, meshframe, 14+34+sizeof(struct rreq), 0, (struct sockaddr*)&sll, sizeof(sll))) == -1) { 
		// fprintf(stderr, "Error: can't sendto()\n");
	} else {
		// printf(".");
	}
	
	
  //  signal(SIGALRM, test_rawsocket_send);
	return;
}

// The following function is written by Jason
void test_rawsocket_unicast(int signo)
{

	//printf("Catch a signal -- SIGALRM1 \n");
	

//------------!!! filled in correct number !!!-----------------//	
//======= the destination is means destination for RREQ want to find============//
// change by bc5678 2007.1.16
	unsigned char dest_mac_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char nexthop_mac_addr[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	unsigned char src_mac_addr[6] = {0x00, 0xE0, 0x4C, 0x88, 0x88, 0x88};
	unsigned char *ra, *ta, *da, *sa;
	int i;
	// unsigned char broadcast_mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff}; 

	// create socket
	/ *
	if ((s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		// fprintf(stderr, "Error: can't socket()\n");
	}
	* /
/ *
	if ((g_rawsocket_fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		// fprintf(stderr, "Error: can't socket()\n");
	}
	* /
	struct sockaddr_ll sll; // target address
	int send_result = 0;

	memset(&sll, 0, sizeof(sll));

	// prepare sockaddr_ll

	// index of the network device
	
	sll.sll_ifindex	= get_iface_index(g_rawsocket_fd, interface_rawsocket);
	//sll.sll_ifindex	= get_iface_index(g_rawsocket_fd, interface_ioctl);
	// address length
	sll.sll_halen = ETH_ALEN; // 6 

	sll.sll_family = PF_PACKET;

//	unsigned char meshframe[1024]; // buffer for mesh frame
//	memset(meshframe, 0, sizeof(meshframe));
	
	// MAC - begin
	memset(sll.sll_addr, 0, 8);
	memset(sll.sll_addr, 0xff, 6);
	memcpy(sll.sll_addr, src_mac_addr, 6);
	// MAC - end

//	sll.sll_protocol = htons(ETH_P_IP);	
//	sll.sll_protocol = ETH_P_ALL;	
	// sll.sll_protocol = htons(ETH_P_ALL);	
	sll.sll_protocol = 0;	


	// ARP hardware identifier is ethernet
	// sll.sll_hatype = ARPHRD_ETHER;
	sll.sll_hatype = 0;

	// target is another host
	//sll.sll_pkttype = PACKET_OTHERHOST;
	sll.sll_pkttype = 0;

// ====================== change by bc5678
	int zero_offset = 14;
	unsigned char meshframe[256]; // buffer for mesh frame
	memset(meshframe, 0x0, sizeof(meshframe));

	// Type & Subtype
    SetFrameSubType(meshframe+zero_offset, WIFI_11S_MESH_ACTION);
    SetToDs(meshframe+zero_offset);
    SetFrDs(meshframe+zero_offset);

    //      next hop
    ra = GetAddr1Ptr(meshframe+zero_offset);
    memcpy(ra, nexthop_mac_addr, 6);

    // transmitter = source = self
    ta = GetAddr2Ptr(meshframe+zero_offset);
    sa = GetAddr4Ptr(meshframe+zero_offset);
    for (i = 0; i < 6;i++) {
            ta[i] = sa[i] = src_mac_addr[i];
    }

    // destination addr
    da = GetAddr3Ptr(meshframe+zero_offset);
    memcpy(da, dest_mac_addr, 6);


	// rreq	
	
 

	struct rreq * ptr = (struct rreq*)(meshframe+zero_offset+34);

  ptr->Category     =5;
	ptr->Action     =2;
	ptr->Id         =0;    //T.B.D
	ptr->Length         =32;
	ptr->ModeFlags  =1;
	ptr->ttl        =255;
	//ptr->DesCount       =1;
	ptr->HopCount       =0;
	ptr->RreqId     = 0;
	//ptr->SourceAddress = macAddr_atoi(src_mac_addr);
	ptr->SourceSeqNum   = 0;
	ptr->Metric     =0;


//	memset(meshframe, 0xD1, 6);
//	memcpy(meshframe+6, src_mac_addr, 6);
//	memset(meshframe+12, 0xEA, 2);
//	memset(meshframe+14, 0xFD, 50);

	// send the packet
	if ((send_result = sendto(g_rawsocket_fd, meshframe, 14+34+sizeof(struct rreq), 0, (struct sockaddr*)&sll, sizeof(sll))) == -1) { 
		// fprintf(stderr, "Error: can't sendto()\n");
	} else {
		// printf(".");
	}
	
	
  //  signal(SIGALRM, test_rawsocket_send);
	return;
}

*/

int init_root(void) {
	unsigned char buf[128] = {0}; //should the same with the size defined in wireless_ag's ioctl.c
	unsigned char zeromac[MACADDRLEN] = {0};
	struct iwreq wrq;
	int ret=-1;        

	if( isPortal == 1 && toBeRoot == 1 )
	{
		//system("brctl stp br0 on");
		
		if( memcmp(rann_root_entry.mac, my_address, MACADDRLEN) > 0 || !memcmp(rann_root_entry.mac, zeromac, MACADDRLEN) )
		{
			isRoot = 1;
			memcpy(rann_root_entry.mac, my_address, MACADDRLEN);
	                rann_root_entry.timeout = ROOT_ANNOUNCEMENT_TIMEOUT;
	                rann_root_entry.flag = 1;
			update_root_info(my_address);
		}
	}
	else
	{
		//system("brctl stp br0 off");

		if( memcmp(rann_root_entry.mac, my_address, MACADDRLEN) == 0 )
		{
			isRoot = 0;
			memset(rann_root_entry.mac, 0, MACADDRLEN);
                        rann_root_entry.flag = 0;
			update_root_info(zeromac);
		}
	}
	reset_timer();
    
	// Get wireless name 
	memset(wrq.ifr_name, 0, sizeof wrq.ifr_name);
	strncpy(wrq.ifr_name, interface_ioctl, IFNAMSIZ);

	buf[0] = toBeRoot;
	buf[1] = isPortal;
  
	wrq.u.data.length = sizeof(buf);
	wrq.u.data.pointer = (caddr_t)buf;

#ifndef __ECOS		
	if(ioctl(g_ioctrl_socket_fd, SIOC_SET_ROUTING_INFO, &wrq) < 0)
	{
		// printf("SIOC_SET_ROUTING_INFO  Error:%s\n\a",strerror(errno)); 
		goto init_root_err;
	}
#endif
	ret = 0;
init_root_err:
	return ret;
}

#if defined(CONFIG_RTL_MESH_AUTOPORTAL_SUPPORT)
int getEtherPortLink()
{
	struct ifreq ifr;
	int sockfd;
	unsigned long i,status=0;

	for( i=0;i<4;i++ ) {	//port0,port1,port2,port3
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		{
			perror("fatal error socket\n");
			return -3;
		}

		strcpy((char*)&ifr.ifr_name, "eth0");
		status = i;
		ifr.ifr_data = &status;

		if (ioctl(sockfd, SIOCDEVPRIVATE+0x1, &ifr)<0)
		{
			perror("device ioctl:");
			close(sockfd);
			return -1;
		}
		close(sockfd);

		if( status > 1 )
			return 1;
	}
	return 0;
}

void callback_usr2()
{
	//chris - init: check port status
	if( getEtherPortLink()==1 ) {
		isPortal = 1;
		toBeRoot = 1;
		//printf("signaled, portal/root enabled\n");
	} else {
		isPortal = 0;
		toBeRoot = 0;
		//printf("signaled, portal/root disabled\n");
	}

	init_root();
}
#endif

int get_my_mac_addr(unsigned char *mac)
{
	int fd;
	struct ifreq ifr;
	unsigned char zero_mac[6]={0}, broadcat_mac[6]={0xff};

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		printf("socket() fail\n");
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, interface_ioctl);
	if( ioctl(fd, SIOCGIFHWADDR, &ifr) < 0 ) {
		close(fd);
		return -1;
	}

	close(fd);
	if( !memcmp(ifr.ifr_hwaddr.sa_data,zero_mac,6) || !memcmp(ifr.ifr_hwaddr.sa_data,broadcat_mac,6) )
		return -1;

	memcpy(mac,ifr.ifr_hwaddr.sa_data,6);
	return 0;
}


int pathsel_main(int argc, char *argv[])
{
    memset(hapd->wlan_iface, 0, sizeof(hapd->wlan_iface));
	hapd->debug = HOSTAPD_DEBUG_NO;
	hapd->is_daemon = 1;
	toBeRoot = 0;
	isPortal = 1;
	strcpy(interface_ioctl, "wlan-msh0");  
	
    if ((g_ioctrl_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        fprintf(stderr, "Error: can't socket()\n");
        return -1;
    }

	if(get_iface_name(g_ioctrl_socket_fd, interface_ioctl, my_address) <0) {
		printf("Get rann address failed!\n");
		return -1;
	}
			
    if( init_root() < 0 ) {
        printf("Init root failed\n");
        return -1;
    }
	
	do_daemon();
    close(g_ioctrl_socket_fd);
    g_ioctrl_socket_fd = -1;
    return 0;
}

/*
void HelloMessage()
{	
	unsigned char MyAdd[MACADDRLEN]={0};
	
	if(get_iface_name(g_rawsocket_fd, interface_rawsocket, MyAdd) <0) {
		// printf("Get my address errror!\n");
	}
	else {
		Hello_random++;
		unsigned char Hello[4] ={5,9,8,2};
		send_packet(Hello,MyAdd,broadcast_mac_addr,MyAdd,1,1,Hello_random);
		Hello_random  = Hello_random % 10000;	
	}
	
	return;
}
*/

void TimerFunc()
{
	static int i = 0;		
	i = (i+1)%16;

	if( i == 0x08)  	// every 16 secs
	{
#ifdef IPC_BASED_PATHSEL	
#ifndef __ECOS
		SendPid();	
#endif
#endif	
	}
	pann_handler();		// every 1 sec
	cyg_callout_reset(&pathsel_timer,pathsel_sndpkt_timeout, TimerFunc, NULL);
}
	



#define PATHSEL_THREAD_STACK_SIZE 0x00006000
#define PATHSEL_THREAD_PRIORITY 16
cyg_uint8  pathsel_stack[PATHSEL_THREAD_STACK_SIZE];
cyg_handle_t pathsel_thread;
cyg_thread  pathsel_thread_object;
void create_pathsel(void)
{
	//Create the thread 
	cyg_thread_create(PATHSEL_THREAD_PRIORITY,
		      pathsel_main,
		      0,
		      "pathsel",
		      &pathsel_stack,
		      sizeof(pathsel_stack),
		      &pathsel_thread,
		      &pathsel_thread_object);
	//Let the thread run when the scheduler starts 
	cyg_thread_resume(pathsel_thread);
	cyg_callout_init(&pathsel_timer);
	cyg_callout_reset(&pathsel_timer,pathsel_sndpkt_timeout, TimerFunc, NULL);
}


