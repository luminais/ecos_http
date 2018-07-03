#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <sys/uio.h>
//#include <linux/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdarg.h>
#include <stddef.h>
//#include <strings.h>
#include <string.h>
#include <signal.h>
#include "udpechoserverlib.h"
#include <arpa/inet.h>
#include <netinet/in.h>

#include <apmib.h>

#define CYGNUM_UDPECHOSERVER_THREADOPT_PRIORITY 10
#define CYGNUM_UDPECHOSERVER_THREADOPT_STACKSIZE 0x00002800

int cyg_udpechoserver_init = 0;
cyg_thread   cyg_udpechoserver_thread_object;
cyg_handle_t cyg_udpechoserver_thread_handle;
cyg_uint8    cyg_udpechoserver_thread_stack[CYGNUM_UDPECHOSERVER_THREADOPT_STACKSIZE];

//int	 ues_argc;
//char ues_argv[16][24];
int ues_sockfd;

#ifdef _PRMT_TR143_
struct ECHOSET
{
	char		*interface;
	struct in_addr	cliaddr;
	unsigned short	port;
	char		echoplus;
};

struct ECHOSET gEchoSet;
struct ECHORESULT *gEchoResult=NULL;

int handle_echo( struct ECHOSET *pset )
{
	struct sockaddr_in	servaddr;
	struct sockaddr_in	cliaddr;
	socklen_t		fromlen;
	struct timeval	tv_recv, tv_reply, tv_sel;
	char			mesg[MAXLINE];
	int ret, n = 1;
	int bindloop=10;
	int bindsleep=3;
	fd_set rfds;

	ues_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if( ues_sockfd<0 )
	{
		diag_printf("handle_echo> socket error ret=%d\n", ues_sockfd);
		return -1;
	}

	// not supported
	//if(fcntl(ues_sockfd, F_SETFL, O_NONBLOCK)<0) {
	//	perror( "handle_echo> O_NONBLOCK" );
	//}
	
	if(pset->interface)
	{
		ret = setsockopt(ues_sockfd, SOL_SOCKET, SO_BINDTODEVICE, pset->interface, strlen(pset->interface)+1);
		if(ret)
		{
			diag_printf("handle_echo> setsockopt error interface=%s, ret=%d\n", pset->interface, ret);
			//return -1;
		}
	}
	
//	if(setsockopt(ues_sockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &n, sizeof(n))<0)
//		perror( "handle_echo> SO_REUSEADDR" );

//	if(setsockopt(ues_sockfd, SOL_SOCKET, SO_REUSEPORT, (char *) &n, sizeof(n))<0)
//		perror( "handle_echo> SO_REUSEPORT" );
	
//	struct linger so_linger;
//	so_linger.l_onoff = 1;
//	so_linger.l_linger = 0;
//	if (setsockopt(ues_sockfd, SOL_SOCKET, SO_LINGER, &so_linger, sizeof(so_linger)))
//		perror( "handle_echo> SO_LINGER" );

//#if defined(CONFIG_RTL_819X) && defined(CONFIG_RTL_SORAPIDRECYCLE)
//	if (setsockopt(ues_sockfd, SOL_SOCKET, SO_RAPIDRECYCLE, (char*)&n, sizeof(int)))
//		perror( "handle_echo> SO_RAPIDRECYCLE" );
//#endif

	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(pset->port);

	while(bindloop--)
	{
		ret = bind(ues_sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
		if( ret<0 )
			perror( "handle_echo> bind\n" );
		else
			break;
				
		while(bindsleep!=0)
			bindsleep=sleep(bindsleep);
	}
	if(bindloop==0)
	{
		diag_printf("handle_echo> bind error\n");
		return -1;
	}

	tv_sel.tv_sec = 5;
	tv_sel.tv_usec = 0;
	
	for(;;)
	{
		FD_ZERO(&rfds);
		FD_SET(ues_sockfd, &rfds);
		fromlen = sizeof(cliaddr);
		
		if(select(ues_sockfd+1, &rfds, NULL, NULL, &tv_sel)>0) { // If data arrived, receive.
			n = recvfrom(ues_sockfd, mesg, MAXLINE, 0, (struct sockaddr *)&cliaddr, &fromlen);
			if(n<=0)
			{ 
				diag_printf("handle_echo> recvfrom error ret=%d\n", n);
				continue;
			}
		} else {
			diag_printf("handle_echo> select time out\n");
		}
//		diag_printf( "handle_echo> Get from %s:%d, len=%d\n", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), n );	

		if(pset->cliaddr.s_addr)
		{
			if( pset->cliaddr.s_addr!=cliaddr.sin_addr.s_addr )
			{
				continue;
			}
		}

		gettimeofday( &tv_recv, NULL );
		time(&tv_recv.tv_sec); // because gettimeofday here cannot get the correct seconds since 1970-01-01 00:00:00 +0000 (UTC)
		if(gEchoResult->TimeFirstPacketReceived.tv_sec==0&&gEchoResult->TimeFirstPacketReceived.tv_usec==0)
			gEchoResult->TimeFirstPacketReceived=tv_recv;
		gEchoResult->TimeLastPacketReceived=tv_recv;
		gEchoResult->PacketsReceived++;
		gEchoResult->BytesReceived+=n;
				
		if(pset->echoplus)
		{
			struct ECHOPLUS *pplus;
			if( n>=sizeof(struct ECHOPLUS) )
			{			
				pplus=(struct ECHOPLUS *)mesg;
				pplus->TestRespSN=htonl(gEchoResult->TestRespSN);
				pplus->TestRespRecvTimeStamp=htonl( tv_recv.tv_sec*1000000+tv_recv.tv_usec );
				pplus->TestRespReplyFailureCount=htonl( gEchoResult->TestRespReplyFailureCount );
				gettimeofday( &tv_reply, NULL );
				time(&tv_reply.tv_sec); // because gettimeofday here cannot get the correct seconds since 1970-01-01 00:00:00 +0000 (UTC)
				pplus->TestRespReplyTimeStamp=htonl( tv_reply.tv_sec*1000000+tv_reply.tv_usec );
#if 0
			}else{
				gEchoResult->TestRespReplyFailureCount++;
				continue;
#endif
			}
		}
		
		ret = sendto(ues_sockfd, mesg, n, 0, (struct sockaddr *)&cliaddr, fromlen);
		if( ret!=n )
		{
			gEchoResult->TestRespReplyFailureCount++;
			diag_printf("handle_echo> sendto error ret=%d\n", ret);
		}else{
			gEchoResult->TestRespSN++;
			gEchoResult->PacketsResponded++;
			gEchoResult->BytesResponded+=ret;
		}
	}	
		
	return 0;
}

extern char *strItf[];
extern unsigned short udp_echo_last_port; // store the last port number for TR143 UDP echo test

#if !defined(ECOS)
int parseArg( int argc, char **argv )
#else
int parseArg(void)
#endif
{
	char interface[16]={0};
//	struct in_addr cliaddr;
	unsigned int port; // in mib_get() port must be int type
	unsigned char udpecho_ip[4];
	int udpechoplus_enabled;
	int itftype;
	int echoplus=0;
	int	cnt, got_inf, got_cliaddr, got_port, got_echoplus;
		
	cnt=1;
	got_inf=0;
	got_cliaddr=0;
	got_port=0;
	got_echoplus=0;
	
//	while( cnt<ues_argc )
//	{
//		diag_printf("[%s %d] ues_argv[%d]=%s\n", __FUNCTION__, __LINE__, cnt, ues_argv[cnt]);
//		if( strcmp( "-i", ues_argv[cnt] )==0 )
//		{
//			if( cnt+1<ues_argc )
//			{
//				cnt++;
//				//interface=ues_argv[cnt];
//				memcpy(interface, ues_argv[cnt], sizeof(interface)-1);
//				interface[sizeof(interface)-1] = '\0';
//				diag_printf("[%s %d] interface=%s\n", __FUNCTION__, __LINE__, interface);
//				got_inf=1;
//			}else 
//				return -1;
//		}else if( strcmp( "-addr", ues_argv[cnt] )==0 )
//		{
//			if( cnt+1<ues_argc )
//			{
//				char *a=ues_argv[cnt+1];
//				diag_printf("[%s %d] addr=%s\n", __FUNCTION__, __LINE__, a);
//				if( inet_aton( a, &cliaddr ) )
//				{
//					got_cliaddr=1;
//					cnt++;
//				}else
//					return -1;
//			}else 
//				return -1;
//		}else if( strcmp( "-port", ues_argv[cnt] )==0 )
//		{
//			if( cnt+1<ues_argc )
//			{
//				long int tmp_port;
//				char *s=ues_argv[cnt+1];
//				diag_printf("[%s %d] port=%s\n", __FUNCTION__, __LINE__, s);
//				tmp_port = strtol(s, (char **)NULL, 10);
//				if( tmp_port==LONG_MIN || tmp_port==LONG_MAX )
//					return -1;
//				if( tmp_port<=0 || tmp_port>=65535 )
//					return -1;
//				port=tmp_port;
//				diag_printf("[%s %d] port=%d\n", __FUNCTION__, __LINE__, port);
//				got_port=1;
//				cnt++;
//			}else 
//				return -1;
//		}else if( strcmp( "-plus", ues_argv[cnt] )==0 )
//		{
//			got_echoplus=1;
//			echoplus=1;
//		}else{
//			return -1;
//		}

//		cnt++;
//	}

	mib_get( MIB_TR143_UDPECHO_SRCIP, udpecho_ip );
	//diag_printf("udpecho_ip=%08x\n", udpecho_ip);
	if( udpecho_ip[0]!=0 || udpecho_ip[1]!=0 || udpecho_ip[2]!=0 || udpecho_ip[3]!=0  )
	{
		got_cliaddr=1;
	}
	
	mib_get( MIB_TR143_UDPECHO_PORT, &port );
	//diag_printf("port=%d\n", port);
	if(port>0) {
		if( port==LONG_MIN || port==LONG_MAX )
			return -1;
		if( port<=0 || port>=65535 )
			return -1;
		got_port=1;
	}
	
	mib_get( MIB_TR143_UDPECHO_PLUS, &udpechoplus_enabled );
	//diag_printf("udpechoplus_enabled=%d\n", udpechoplus_enabled);
	if( udpechoplus_enabled )
	{
		got_echoplus=1;
		echoplus=1;
	}
	
	mib_get( MIB_TR143_UDPECHO_ITFTYPE, (void *)&itftype );
	//diag_printf("[%s %d] itftype=%d\n", __FUNCTION__, __LINE__, itftype);
	if( itftype )
	{
		strcpy( interface, strItf[itftype] );
		//diag_printf("[%s %d] interface=%s\n", __FUNCTION__, __LINE__, interface);
		got_inf=1;
	}

	if( !got_port )
		return -1;
	
	memset( &gEchoSet, 0, sizeof(gEchoSet) );
	if(got_inf)
	{
		gEchoSet.interface=strdup( interface );
		//diag_printf( "got interface %s\n", gEchoSet.interface );
	}
	if(got_cliaddr)
	{
		memcpy( &gEchoSet.cliaddr, udpecho_ip, sizeof(udpecho_ip) );
		//memcpy( &gEchoSet.cliaddr, &cliaddr, sizeof(cliaddr) );
		//diag_printf( "got cliaddr %s\n", inet_ntoa(gEchoSet.cliaddr) );
	}
	if(got_port)
	{
		gEchoSet.port=port;
		//diag_printf( "got port %d\n", gEchoSet.port );
		//diag_printf( "last port %d\n", udp_echo_last_port );
	
		/* Bellow add firewall exception for the UDP echo port */
		if(strncmp(interface, "eth1", 4)==0 // WAN
			&& port!=udp_echo_last_port // not already added
			)
		{
			char ipfw_index[10];
			char port_index[10];
			
			sprintf(ipfw_index, "%d", 20003);
			sprintf(port_index, "%d", port);
			// add cmd: ipfw add 20003 allow udp from any to me 4578 in recv eth1
			RunSystemCmd(0, "ipfw", "add", ipfw_index, "allow", "udp", "from", "any", "to", "me", port_index, "in", "recv", interface, "");

			udp_echo_last_port = port;
		}
	}
	if(got_echoplus)
	{
		gEchoSet.echoplus=echoplus;
		//diag_printf( "got enable echo plus %d\n", gEchoSet.echoplus );
	}

	return 0;
}

#if 0
#define ECHOSERVER_RUNFILE "/var/run/udpechoserver.pid"
static void log_pid()
{
	FILE *f;
	pid_t pid;
	char *pidfile = ECHOSERVER_RUNFILE;

	pid = getpid();
	if((f = fopen(pidfile, "w")) == NULL)
		return;
	fprintf(f, "%d\n", pid);
	fclose(f);
}
static void clr_pid()
{
	FILE *f;
	char *pidfile = ECHOSERVER_RUNFILE;

	if((f = fopen(pidfile, "r")) != NULL){
		fclose(f);
		unlink(pidfile);
	}
}

//void handle_term()
//{
//	clr_pid();
//	exit(0);
//}
#endif

#if !defined(ECOS)
int main(int argc, char **argv)
#else
int udp_echo_server_main(void)
#endif
{
//	log_pid();

//	signal( SIGTERM,handle_term);
//	signal( SIGHUP,SIG_IGN);
//	signal( SIGINT,SIG_IGN);
//	signal( SIGPIPE,SIG_IGN);
//	signal( SIGALRM,SIG_IGN);
//	signal( SIGUSR1,SIG_IGN);
//	signal( SIGUSR2,SIG_IGN);


//	int i;
//	diag_printf("ues_argc: %d\n", ues_argc);
//	for(i=0; i<ues_argc; i++) {
//		diag_printf("ues_argv[%d]: %s\n", i, ues_argv[i]);
//	}

#if !defined(ECOS)
	if( parseArg( argc, argv ) < 0 )
#else
	if( parseArg() < 0 )
#endif
	{
		//diag_printf( "\nudpechoserver [-i <interface>] [-addr <client address>] -port <port number> [-plus]\n" );
		diag_printf( "[%s:%d] parseArg error\n", __FUNCTION__, __LINE__ );
		return -1;
	}
	
//	diag_printf( "sizeof(struct ECHORESULT)=%d\n", sizeof(struct ECHORESULT) );
	if( initShmem( (void**)&gEchoResult, sizeof(struct ECHORESULT), ECHOTOK )<0 )
	{
		diag_printf( "initShmem() error\n" );
		return -1;
	}
	//printf( "gEchoResult=0x%x\n", gEchoResult );

	return handle_echo( &gEchoSet );
}
#endif

void cyg_udpechoserver_start(void)
{
#ifdef _PRMT_TR143_
	int i, len;
	
    if (cyg_udpechoserver_init) {
        //diag_printf("UDP echo server is already running!\n");
        return;
    }
    cyg_udpechoserver_init = 1;
    
    cyg_thread_create(CYGNUM_UDPECHOSERVER_THREADOPT_PRIORITY,
                      (cyg_thread_entry_t *)udp_echo_server_main,
                      (cyg_addrword_t)0,
                      "UDP Echo Server",
                      (void *)cyg_udpechoserver_thread_stack,
                      CYGNUM_UDPECHOSERVER_THREADOPT_STACKSIZE,
                      &cyg_udpechoserver_thread_handle,
                      &cyg_udpechoserver_thread_object);
    cyg_thread_resume(cyg_udpechoserver_thread_handle);
    
	sleep(3);
#endif
}

void cyg_udpechoserver_exit(void)
{
#ifdef _PRMT_TR143_
	int i, ret;
	
	if(cyg_udpechoserver_init) {
		ret = shutdown(ues_sockfd, SHUT_RDWR);
    	//diag_printf("shutdown(ues_sockfd) ret=%d\n", ret);
		ret = close(ues_sockfd);
    	//diag_printf("close(ues_sockfd)ret=%d\n", ret);
		cyg_thread_kill(cyg_udpechoserver_thread_handle);
		cyg_thread_delete(cyg_udpechoserver_thread_handle);
		cyg_udpechoserver_init = 0;
    	diag_printf("UDP Echo Server Exit!\n");
	}
#endif
}
