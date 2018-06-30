/***********************************************************
	Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.
	FileName: al_security.c
	Description:tenda alibaba api
	Author: Lvliang;
	Version : 1.0
	Date: 2015.2.6
	Function List:
	History:
	<author>   <time>     <version >   <desc>
	Lvliang    2015.1.27   1.0          new
************************************************************/
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/mbuf.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/in_var.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/tcp_fsm.h>
#include <netinet/udp.h>
#include <ip_compat.h>
#include <ip_fil.h>
#include <ip_nat.h>
#include <ip_frag.h>
#include <net/if_var.h>
#include <router_net.h>
//#include <cbbconfig.h>
#include <netinet/ip_var.h>
#include <stdio.h>
#include <arpa/nameser.h>
#include <bcmnvram.h>

#include "al_security.h"
#include "al_table.h"
#include "ali_ns_network.h"


#define ALI_NS_OK    0

#define ALI_NS_ERR  -1

#define PK_REDIRECT			1

#define PK_PASS				0

#define DNS_DEST_PORT		53

#define HTTP_DEST_PORT		80

#define HTTP_LEAST_LEN		20

#define LAN_IP_LEN			16

#define HTTP_HEAD			"http://"

#define HTTP_REDIRECT_URL	"http://%s/warning.html#mac=%02x:%02x:%02x:%02x:%02x:%02x&url=%s"

#define REDIRECT_TEMP_LEN	300

#define MAX_FULL_URL_LEN				1500

#define ARRAY_LEN_1024					1024

char htmhead[MAX_FULL_URL_LEN] = {0}; 

int al_security_open_success = 0 ;

char lan_ip[LAN_IP_LEN] = {0} ;

typedef HEADER dnshdr ;

extern int dns_fish_times ;

extern int dns_hijack_times ;

extern int report_ali_sec_info(void) ;

extern int ali_ns_query(unsigned int query_flag,const char *domain, unsigned char mac_addr[6], unsigned int dns_srv_ip);

extern int dnsmasq_parse_request(dnshdr *dnsheader, unsigned int qlen, char *name) ;

extern int ip_output(struct mbuf *m0, struct mbuf *opt, struct route *ro, int flags, struct ip_moptions *imo);

extern int ether_atoe(const char *a, unsigned char *e) ;

extern void ali_security_reprot(void);

int find_host_in_white_list(unsigned char* mac , char * host ) ;


#define ALINKGW_URLP_REPORT		"{\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\" , \"url\":\"%s\"}"

char globle_alsec_info[REDIRECT_TEMP_LEN]  = {0} ;

int init_black_list(void)
{
	black_list.position = 3 ;
	strcpy(black_list.host_url[0] , "www.baidu.com");
	strcpy(black_list.host_url[1] , "www.qq.com");
	strcpy(black_list.host_url[2] , "www.jd.com");
	return 0 ;
}

int show_black_list(void)
{
	int i = 0 ; 
	for(i = 0 ; i < black_list.position ; i++ )
	{
		printf("black_list.host_url[%d]:%s\n" , i ,black_list.host_url[i]  );
	}
	return 0 ;
}

int set_al_security_status(void)
{
	al_security_open_success = 1 ;

	return 0 ;
}


int get_al_security_status(void)
{

	return al_security_open_success ;
}

int get_lan_ip_addr(void)
{
	strncpy(lan_ip , nvram_safe_get("lan_ipaddr") , LAN_IP_LEN) ;
	
	return 0 ;
}

int insert_into_black_list(const char* domain)
{
	if(domain == NULL  )
	{
		return -1 ;
	}
	memset(black_list.host_url[black_list.position] , 0 , NAME_LEN_256 ) ;

	strncpy(black_list.host_url[black_list.position] , domain , NAME_LEN_256 ) ;

	black_list.position = (black_list.position + 1) % BLACK_LIST_LEN ;

	return 0 ;
}

int insert_into_white_list( char* mac , char* domain)
{
	if(NULL == mac || NULL == domain )
	{
		return -1 ;
	}

	if(1 == find_host_in_white_list((unsigned char*)mac , domain))
	{
		printf("have add to white list\n");
		return 1 ;
	}
	memset(white_list.mac[white_list.position] , 0 , ETHER_ADDR_LEN ) ;
	memset(white_list.host_url[white_list.position] , 0 , NAME_LEN_256 ) ;
	
	ether_atoe(mac , white_list.mac[white_list.position] ) ;


	//printf("***********%02x:%02x:%02x:%02x:%02x:%02x********\n" ,white_list.mac[white_list.position][0]&0xFF , white_list.mac[white_list.position][1]&0xFF,
		//white_list.mac[white_list.position][2]&0xFF , white_list.mac[white_list.position][3]&0xFF, white_list.mac[white_list.position][4]&0xFF , white_list.mac[white_list.position][5]&0xFF);
	
	strncpy(white_list.host_url[white_list.position] , domain , strlen(domain) );

	//LLDEBUG("[white_list.position:%d][white_list.host_url[white_list.position]:%s]\n" , 
		//white_list.position , white_list.host_url[white_list.position] ) ;

	white_list.position = (white_list.position + 1) % WHITE_LIST_LEN ;

	return 1 ;
}


/*************************************************************************
Description:   云端威胁查询结果回调函数类型。
Input:         domain：     域名，以'\0'结束。
               mac_addr： 接入设备的MAC地址
               dns_srv_ip：DNS服务器IP地址。
Output:        result：   阿里安全检查结果，共有如下取值可能：
                 ALI_NS_RESULT_MASK_unavailable： “阿里安全”功能未开启
                 ALI_NS_RESULT_MASK_normal:      正常域名
                 ALI_NS_RESULT_MASK_phishing:    命中钓鱼网站
                 ALI_NS_RESULT_MASK_hijacking:   仅被DNS劫持
                 ALI_NS_RESULT_MASK_phishing | ALI_NS_RESULT_MASK_hijacking: 
                  访问钓鱼网站且被DNS劫持
Return:        ALI_NS_OK： 检查成功，具体检查结果参见result参数。
               其他：      检查出错
导致该错误。       
Others:
*************************************************************************/


void ali_ns_reply_callback_function(unsigned int query_flag, 
									   const char *domain,  
                                       const unsigned char *mac, 
                                       unsigned int dns_srv_ip, 
                                       unsigned int result)
{

	switch(result)
	{
					
		case ALI_NS_RESULT_MASK_phishing :
			insert_into_black_list(domain) ;
			//dns_fish_times ++ ;
			//report_ali_sec_info();
			break ;

		case ALI_NS_RESULT_MASK_hijacking :
			dns_hijack_times ++ ;
			report_ali_sec_info();
			break ;
			
		case ALI_NS_RESULT_MASK_normal :			
		case ALI_NS_RESULT_MASK_unavailable :
		default :
			break ;
	}

	
	return ;
}

int al_security_handle_prepare(void)
{
	if (ali_ns_set_reply_cb(ali_ns_reply_callback_function) != ALI_NS_OK) 
	{
		return -1;
	}

	if (ali_ns_open() != ALI_NS_OK) 
	{
	    return -1;
	}
	
	set_al_security_status();

	printf("@@@@@@@@@@@@@@@ali_ns_open success , al_security_open_success[%d] @@@@@@@@@\n" , al_security_open_success);

	//init_black_list();

	//show_black_list();

	return 0 ;
}

int al_security_handle_after(void)
{

	ali_ns_close() ;
	
	return 0 ;
}


int get_packet_mac(char* head , unsigned char * s_mac )
{
	memcpy(s_mac , &head[6] , ETHER_ADDR_LEN) ;
	
	return 0;
}

int find_dns_in_white_list(unsigned char* mac , char * domain)
{
	int i = 0 ;
	for(i = 0 ; i < WHITE_LIST_LEN ; i++)
	{
		if(0 == memcmp(white_list.mac[i] , mac , ETHER_ADDR_LEN) &&
			0 == strncmp(white_list.host_url[i] , domain , strlen(domain)))
		{
			SHOWDEBUG("find dns in white list");
			return 1 ;
		}
	}
	return 0 ;
}




int find_dns_in_black_list(char* domain)
{
	int i = 0 ;
	for(i = 0 ; i < BLACK_LIST_LEN ; i++)
	{
		if(0 == strncmp(black_list.host_url[i] , domain , strlen(domain)))
		{
			SHOWDEBUG("find dns in black list") ;
			
			return 1 ;
		}
	}
	return 0 ;
}



int al_handle_dnspackets(dnshdr *dnsh , int len ,unsigned char* s_mac , unsigned int d_ip)
{
	char domain_name[NAME_LEN_256] = { 0 } ;
	
	dnsmasq_parse_request(dnsh, len, domain_name) ;
	
	if(strlen(domain_name) == 0)
	{
		return PK_PASS ;
	}

	if(1 == find_dns_in_white_list(s_mac , domain_name))
	{
		return 1 ;
	}
	else
	{
		if(1 != find_dns_in_black_list(domain_name))
		{
			ali_ns_query(0 , domain_name , s_mac , d_ip);
		}
		
		return 0 ;
	}
	
}



const char *get_next_line(const char *data, int *len)
{
	int i;

	char c ;

	if(data == NULL || *len <=0 )
	{
		return NULL ;
	}

	for (i=0; i < *len; i++ )
	{
		c = data[i] ;
		if (('\n' == c) || ('\r' == c))
		{
		    	i++;
		    	if (('\r' == c) && (i < *len -1) && ('\n' == data[i]))
		    	{
					i++;

					*len -= i ;
					
			   		return &(data[i]);
		    	}
	    	}
	}

	return NULL;
	
}


int find_host_in_white_list(unsigned char* mac , char * host )
{
	int i = 0 ;
	for(i = 0 ; i < WHITE_LIST_LEN ; i++)
	{
		if(0 ==memcmp(white_list.mac[i] , mac , ETHER_ADDR_LEN) &&
			0 == strncmp(host , white_list.host_url[i] , strlen(host)))
		{
			printf("find_host_in_white_list\n");
			return 1 ;
		}
	}
	return 0 ;
}


int find_host_in_black_list(char* host)
{
	int i = 0 ;
	for(i = 0 ; i <  BLACK_LIST_LEN; i++)
	{
		if(0 == strncmp(black_list.host_url[i] , host , strlen(host)))
		{
			LLDEBUG("find %s in black_list" , host) ;
			return 1 ;
		}
	}
	return 0 ;
}


static void http_init_pkt(char *url)
{
	char htmbody[ARRAY_LEN_1024] = {0};

	int n = 0,len=0;
	char *p = htmbody;

	//printf("http_redirection_url [%s]\n",url);
	
	//build html body
	n = sprintf(p, "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\n");
	p = p + n;
	n = sprintf(p, "<html><head>\n");
	p = p + n;
	n = sprintf(p, "<title>302 Moved Temporarily</title>\n");
	p = p + n;
	n = sprintf(p, "</head><body>\n");
	p = p + n;
	n = sprintf(p, "<h1>Moved Temporarily</h1>\n");
	p = p + n;
	n = sprintf(p,"<p>The document has moved <a href=\"%s\">here</a>.</p>\n", url);
	p = p + n;
	n = sprintf(p, "<h1></body></html></h1>\n");
	p = p + n;
	n = sprintf(p, "\n");
	p = p + n;

	len = p - htmbody;

	p = htmhead;

	n = sprintf(p, "HTTP/1.1 302 Moved Temporarily\r\n");
	p = p+n;
	n = sprintf(p,"Location: %s\r\n", url);
	p = p+n;

	n= sprintf(p, "Content-Type: text/html; charset=iso-8859-1\r\n");
	p = p+n;
	n= sprintf(p,"Content-Length: %d\r\n", len);
	p = p+n;
	n= sprintf(p, "\r\n");
	p = p+n;
	
	n= sprintf(p, "%s", htmbody);

}


void return_http_redirection(struct mbuf *m , char *http_redirection_url)
{
	struct tcphdr *tcph = NULL;
    	struct ip *ip = NULL;
	struct route ro;   

	int iphlen = 0;
	int off = 0,olen=0,nlen = 0;
	int inc = 0;
	unsigned long src_addr = 0 ,dest_addr = 0 ;
	unsigned short dest_port = 0 ;

	ip = mtod(m, struct ip *);
	if(ip == NULL)
	{
		return ;
	}

	iphlen = ip->ip_hl << 2;
	
	http_init_pkt(http_redirection_url);
	
	tcph = (struct tcphdr *)((unsigned char*)ip + iphlen);
	if(tcph == NULL)
	{
		return ;
	}

	off = iphlen + (tcph->th_off << 2);	
	olen = ntohs(ip->ip_len) -off;
	//printf("===========> \n\n %s \n\n" , htmhead);
	nlen = strlen(htmhead);
	inc = nlen - olen;
	
//must do this
	if (inc < 0)
	{
		m_adj(m, inc);
	}
	
//learn form ip_ftp_pxy.c->ippr_ftp_port()
	m_copyback(m, off, nlen, htmhead);

	src_addr = ip->ip_dst.s_addr;
	dest_addr = ip->ip_src.s_addr;

	bzero(ip,iphlen);

	//make presudo uip header,learn form l2tp_usrreq.c->ifl2tp_output()
	ip->ip_p = IPPROTO_TCP;
	//ip_len = tcp head len+ data_len
	ip->ip_len = htons((tcph->th_off << 2) + nlen);
	ip->ip_src.s_addr = src_addr ;
	ip->ip_dst.s_addr = dest_addr;


	dest_port = tcph->th_sport;
	tcph->th_sport = tcph->th_dport;
	tcph->th_dport = dest_port;

	src_addr = tcph->th_seq;
	dest_addr = tcph->th_ack;

	tcph->th_seq = dest_addr;
	tcph->th_ack = htonl(ntohl(src_addr)+olen);
	tcph->th_win = htons(ntohs(tcph->th_win) - nlen);

	//cksum = tcp_sum((unsigned short *)tcph, ip->ip_len, tcp_pseudo_sum(ip));

	tcph->th_sum = 0;//very important
    	tcph->th_sum =in_cksum(m, iphlen+ntohs(ip->ip_len));
	

	ip->ip_len = iphlen + ntohs(ip->ip_len);
	ip->ip_ttl= 128;
	ip->ip_off|=IP_DF;

	bzero(&ro, sizeof ro);
	
	ip_output(m, 0, &ro, 0, 0);
	
	return;
}


int al_handle_httppackets(struct mbuf *m , char * data ,unsigned char* src_mac ,  int pk_len )
{	

	int i = 0 ;

	const char* http_data = data ;
	
	int http_len = pk_len;
	
	char http_host[NAME_LEN_256] = {0} ;

	char temp_redirect[REDIRECT_TEMP_LEN]  = {0} ;
	
	switch (http_data[0])
	{
		case 'G':
			
			if (memcmp(http_data, "GET ", 4) == 0)
			{
				http_len -= 4 ;
				http_data += 4;
			}
			break;
			
		case 'P':
			if (memcmp(http_data, "POST ", 5) == 0)
			{
				http_len -= 5;
				http_data += 5;
			}
			break;
		case 'H':
			if (memcmp(http_data, "HEAD ", 5) == 0)
			{
				http_len -= 5;
				http_data += 5;
			}
			break;
		default:
			return 0 ;

	}

	while(http_len > 0 )
	{
		http_data = get_next_line(http_data , &http_len) ;
		
		if(NULL == http_data || http_len <= 0)
		{
			break ;
		}
		if (  ('H' == http_data[0]) && (0 == memcmp(http_data, "Host: " , strlen("Host: "))))
		{
			http_data += strlen("Host: ");
			
			http_len -= strlen("Host: ");

			if(NULL == http_data || http_len <= 0)
			{
				SHOWDEBUG("http_data or http_len is null") ;
				return 0 ;
			}

			for (i=0; i< http_len; i++) 
			{
       			if (('\r' == http_data[i]) || ('\n' == http_data[i]) || i >= NAME_LEN_256 -1) 
				{
					http_host[i] = '\0';
					break ;
				}
				else 
				{
					http_host[i] = http_data[i] ;
				}
			}

			break;

		}
		
	}
	
	if(strlen(http_host) == 0 )
	{
		return PK_PASS ;
	}

	if(1 == find_host_in_white_list( src_mac , http_host))
	{
		return PK_PASS ;
	}
	
	if(1 == find_host_in_black_list(http_host))//在黑名单中才重定向
	{
 
		//report
		memset(globle_alsec_info , 0x0 , sizeof(globle_alsec_info));
		
		snprintf(globle_alsec_info , sizeof(globle_alsec_info) , ALINKGW_URLP_REPORT, src_mac[0]&0xFF , src_mac[1]&0xFF , src_mac[2]&0xFF , src_mac[3]&0xFF , src_mac[4]&0xFF , src_mac[5]&0xFF , http_host );

		dns_fish_times ++ ;

		ali_security_reprot() ;
		
		//http重定向
				
		memset(temp_redirect , 0x0 , sizeof(temp_redirect)) ;
		
		snprintf(temp_redirect ,sizeof(temp_redirect) , HTTP_REDIRECT_URL , lan_ip , src_mac[0]&0xFF , src_mac[1]&0xFF , src_mac[2]&0xFF , src_mac[3]&0xFF , src_mac[4]&0xFF , src_mac[5]&0xFF , http_host );

		LLDEBUG("[temp_redirect:%s]\n" , temp_redirect );
		
		return_http_redirection(m , temp_redirect) ;
		
		return PK_REDIRECT ;
	}

	return PK_PASS ;

}

int al_security_handle_hook(struct ifnet *ifp, char *head, struct mbuf *m) 
{

	struct ip * iph = NULL ;

	struct tcphdr *tcph = NULL ;

	struct udphdr *udph = NULL;

	dnshdr *dnsh = NULL ;

	char *httph = NULL ;

	int pk_left_len = 0 ;
	
	unsigned char src_mac[ETHER_ADDR_LEN] = {0} ;
	
	unsigned int dst_ip = 0 ;

	if(1 != get_al_security_status())
	{
		return PK_PASS;
	}
	
	if(head == NULL || ifp == NULL || m == NULL)
	{
		return PK_PASS;
	}
	
	if ((((struct ifnet *)m->m_pkthdr.rcvif)->if_fltflags & IFFLT_NAT) == 0)
	{
		return PK_PASS ;
	}

	if (m->m_pkthdr.len < 40 )
	{
		return PK_PASS;
	}

	if (m->m_len < sizeof (struct ip))
	{
		return PK_PASS ;
	}

	iph = mtod(m, struct ip *);
	
	if(iph == NULL)
	{
		return PK_PASS ;
	}

	if (iph->ip_v != 4 ||
	    iph->ip_hl != sizeof(struct ip)/4 ||
	    m->m_pkthdr.len < ntohs(iph->ip_len) ||
	    (iph->ip_off & htons(0x3fff)) != 0 ||
	    iph->ip_ttl < 0)
	{
		return PK_PASS ;
	}


	get_packet_mac(head , src_mac);

	
#if 0
	printf("the packet len is %d: \n" , m->m_pkthdr.len);
	int i = 0 ;
	char * test =  (char*)iph ;
	for(i = 0  ; i < m->m_pkthdr.len ; i++ )
	{
		printf("%02x " , *(test + i) & 0xFF);
	}
	printf("the packet end\n\n");
#endif

	//handlen dns
	if(iph->ip_p == IPPROTO_UDP)
	{
		if (m->m_len < sizeof(struct ip) + sizeof(struct udphdr))
		{
			return PK_PASS ;
		}

		udph = (struct udphdr *)((char *)iph + (iph->ip_hl << 2));

		if(udph == NULL)
		{
			return PK_PASS ;
		}

		if( htons(DNS_DEST_PORT) != udph->uh_dport)
		{
			return PK_PASS ;
		}

		pk_left_len =  m->m_pkthdr.len -(iph->ip_hl << 2) - sizeof(struct udphdr) ;
		if(pk_left_len <= sizeof(dnshdr))
		{
			return PK_PASS ;
		}

		dnsh = (dnshdr*)((char*)udph + sizeof(struct udphdr)) ;
		if(dnsh == NULL)
		{
			return PK_PASS ;
		}

		dst_ip =  ntohl(iph->ip_dst.s_addr) ;

		al_handle_dnspackets(dnsh , pk_left_len , src_mac , dst_ip );
		
	}	
	//handle http
	else if(iph->ip_p == IPPROTO_TCP)
	{
		
		tcph = (struct tcphdr *)((char *)iph + (iph->ip_hl << 2));

		if(tcph == NULL)
		{
			return PK_PASS ;
		}

		if (m->m_len < (iph->ip_hl << 2) + (tcph->th_off<<2))
		{
			return PK_PASS ;
		}

		if( htons(HTTP_DEST_PORT) != tcph->th_dport)
		{
			return PK_PASS ;
		}

		pk_left_len =  m->m_pkthdr.len -(iph->ip_hl << 2) - sizeof(struct tcphdr) ;

		if(pk_left_len < HTTP_LEAST_LEN)
		{
			return PK_PASS ;
		}
		httph = (char*)((char*)tcph + (tcph->th_off<< 2)) ;
		if(NULL == httph)
		{
			return PK_PASS ;
		}
		
		if(PK_REDIRECT == al_handle_httppackets(m , httph , src_mac , pk_left_len))
		{
			return PK_REDIRECT;
		}
	}
	
	return PK_PASS ;
}



