#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <sys/malloc.h>
#ifdef INET6
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#endif

#include <ip_compat.h>
#include <net/netisr.h>
#include <urlf.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/queue.h>
//#include <cyg/compress/zlib.h>
#include <bcmnvram.h>
//#include "../../cbb/src/login_keep/lm_login_keep.h"
//#include "../../cbb/src/login_keep/lm_aes.h"
#include "sdw_filter.h"


/*根据host调用gethostbyname获取对应的IP*/
int get_ip(char *host , char *ip)
{
	struct hostent *hent = NULL;

	if(host == NULL || ip == NULL)
	{
		return -1 ;
	}

	hent = gethostbyname(host);

	if (NULL == hent)
	{
		perror("Can't get IP.\n");
		return 1;
	}

	if (NULL == inet_ntop(AF_INET, (void *)hent->h_addr, ip, IP_LEN_16))
	{
		perror("Can't resolv host.\n");
		return -1;
	}
	//printf("%d:%s========>ip[%s]\n" ,__LINE__ , __FUNCTION__, ip);
	return 0 ;
	
}

int create_tcp_socket(void)
{
	int sock;
	if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		perror("Can't create TCP socket.\n");

		return -1;
	}

	return sock;
}

int get_url_path(char *full_path, char *url, char *path, int *port)
{
	char *p = NULL, *char_p = full_path;
	int ret = 0;

	if(NULL==full_path || NULL==url || NULL==path)
	{
		diag_printf("[%s]invalid parameter\n", __FUNCTION__);
		return -1;
	}
	
	*port = 80;
	
	if(0 == memcmp(char_p, "http://", strlen("http://")))
	{
		char_p += strlen("http://");
	}
	
	p = strchr(char_p, '/');
	if(NULL == p)
	{
		diag_printf("[%s]error full_path\n", __FUNCTION__);
		strcpy(url, char_p);
		strcpy(path, "/");
		return 0;
	}

	if(strstr(char_p, ":"))
		ret = sscanf(char_p, "%[^:]:%d%s", url, port, path);
	else
		ret = sscanf(char_p, "%[^/]%s", url, path);
	
	return 0;
}

