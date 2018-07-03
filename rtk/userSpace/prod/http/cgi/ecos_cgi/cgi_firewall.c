
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <typedefs.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/radix.h>
#include <net/route.h>
#include <net/if_dl.h>
#include <net/ethernet.h>
#include <net/netdb.h>
#include <arpa/inet.h>
#include "webs.h"
#include "uemf.h"
#include "flash_cgi.h"
#include "router_net.h"
#include  "cJSON.h"
#ifdef __CONFIG_STREAM_STATISTIC__
#include "tc.h"
#endif
#include  "arp_clients.h"
#include "tenda_arp.h"
#include "pi_common.h"

#include "cgi_common.h"


/*added by yp,for A9,  2016-3-17 */
#ifndef __CONFIG_STREAM_STATISTIC__
typedef struct stream_list{
	char hostname[64];
	char remark[128];
	in_addr_t  ip;
	char  mac[20];
	unsigned int type;
	unsigned int  up_byte_pers;
	unsigned int  down_byte_pers;
	unsigned int  set_pers;
	float		up_limit;
	float		down_limit;
	float		limit;
	int  access;
}stream_list_t;

typedef struct statistic_list_ip_index{
	stream_list_t *index;
}statistic_list_ip_index_t;


typedef struct qosList{
	stream_list_t qosInfo;
	struct qosList *next;
}qos_list_t;
#endif

#define	STATIC_LIST_LOCK()		cyg_scheduler_lock()
#define	STATIC_LIST_UNLOCK()	cyg_scheduler_unlock()

extern int webs_Wan_OpenListen(int port,char *HostIP);
extern void webs_Wan_CloseListen();
extern int is_mac_filter( char* mac ,char* filter_mode);
extern int get_macfilter_mode();
extern int tenda_arp_is_wireless_client(unsigned char *mac);

/****************************************************************/

unsigned int g_localip,g_localmask,g_remoteIPStart,g_remoteIPEnd;
int g_enable_remote_acl,g_remote_port;

void 		LoadWanListen(int load);
void 		LoadManagelist();

typedef struct mac_ip_tbl{
	struct ether_addr hw_addr;
	struct	in_addr ip_addr;
}arp_tbl_enty;

struct parentCtl_devices_list{
	in_addr_t 	ip;
	unsigned char  mac[6];
	int mac_filter_status;
	struct parentCtl_devices_list *next;
};

struct parentCtl_url_list{
	char url[256];
	struct parentCtl_url_list *next;
};

struct parentCtl_config{
	struct parentCtl_devices_list  *devlist;
	time_t stime;
	time_t etime;
	unsigned char wday[8];
	int mode;
	struct parentCtl_url_list  *urllist;
	int url_filter_status;
};

struct parentCtl_config gParentCtlConfig;
int in_time_area = 0;

struct backlist_device_list{
	struct backlist_device_list  *next;
	unsigned char  mac[ETHER_ADDR_LEN];
	unsigned char  flag;
};


extern struct detec *det[TENDA_ARP_MAX_NUM];

enum {
	MODE_DISABLED = 0,//禁用
	MODE_DENY,//仅禁止
	MODE_PASS//仅允许
};

static void parentCtl_devices_list_flush()
{
	struct parentCtl_devices_list *entry, *next;

	if(gParentCtlConfig.devlist == NULL)
		return;
	
	entry = gParentCtlConfig.devlist;
	gParentCtlConfig.devlist = NULL;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
	
	return;
}

static void parentCtl_url_list_flush()
{
	struct parentCtl_url_list *entry, *next;
	if(gParentCtlConfig.urllist == NULL)
		return;
	
	entry = gParentCtlConfig.urllist;
	gParentCtlConfig.urllist = NULL;
	while (entry) {
		next = entry->next;
		free(entry);
		entry = next;
	}
	
	return;
}

#if 0 //后续海外版本再打开
#ifdef CONFIG_RTL_HARDWARE_NAT
int is_ParentCtlConfig_ip (unsigned long ip)
{
	
	struct parentCtl_devices_list  *ptr;
	
	
	/* Now check ip */
	for(ptr = gParentCtlConfig.devlist;ptr != NULL; ptr = ptr->next)
	{
		if(ip == ptr->ip)
		{
			printf ("%s %d  ip=%x=\n", __FUNCTION__, __LINE__, ip);
			return 1;
		}
	}
	
	printf ("%s %d  ip=%x=\n", __FUNCTION__, __LINE__, ip);
	return 0;		
			
}
#endif
#endif
extern in_addr_t tenda_arp_mac_to_ip(char *mac);
static int add_to_devices_list(char *mac, char  *ip)
{
	in_addr_t current_ip = 0;
	if( !mac || !ip)
		return -1;
	if(strlen(mac) != 17)
	{
		return -1;
	}

	struct ether_addr *hw_addr;
	hw_addr = ether_aton(mac);
	if(!hw_addr)
		return -1;
	
	struct parentCtl_devices_list *node;
	node = (struct parentCtl_devices_list *)malloc(sizeof(struct parentCtl_devices_list));
	if(NULL == node)
	{
		printf("malloc error .\n");
		return -1;
	}
        
	memset(node, 0, sizeof(struct parentCtl_devices_list));
	current_ip = tenda_arp_mac_to_ip(mac);
	if(current_ip)
	{
		node->ip =  current_ip;
	}else
	{
		node->ip =  inet_addr(ip);
	}
	node->mac_filter_status = 0;
	memcpy(node->mac, hw_addr->octet, sizeof(node->mac));
	
	node->next = gParentCtlConfig.devlist;
	gParentCtlConfig.devlist = node;
	
       return 0; 
}

static void get_parentCtl_devices_list()
{
	
	char *mib_val;
	int i = 0, client_mib_count = 0;
	char hostname[TPI_BUFLEN_64] = {0};
	char mark[TPI_BUFLEN_64] = {0};
	char ip[TPI_IP_STRING_LEN] = {0};
	char mac[TPI_MAC_STRING_LEN] = {0};
	char limitenable[8] = {0};
	int   macfiler_mode = get_macfilter_mode();
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_CLIENTNUM, mib_val);
	client_mib_count = atoi(mib_val);

	for(i=0;i<client_mib_count;i++)
	{
		_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_RULE(i+1), mib_val);//hostname \t ip \t limitEn
		if(strcmp(mib_val,"") == 0)
			continue;
		memset(hostname, 0x0, sizeof(hostname));
		memset(mark, 0x0, sizeof(mark));
		memset(ip, 0x0, sizeof(ip));
		memset(mac, 0x0, sizeof(mac));
		memset(limitenable, 0x0, sizeof(limitenable));
		if(parsePerParentControlClientConfig(mib_val, hostname, mark, mac, ip, limitenable) ==-1)
			continue;
		
		if(strcmp(limitenable, "false") == 0)
			continue;
		if(macfiler_mode == MODE_DENY)
		{
			if(is_mac_filter(mac,"deny") != -1 )
				continue;
		}
			
		if(strcmp(limitenable, "true") == 0)
			add_to_devices_list( mac , ip);
		else
			continue;
		
	}
	return;
}


/**********************************************************
Function:		find_ParentControl_config

Description:	查找指定的设备是否开启了家长控制
				
Input:			const char* mac		需要查找的设备的MAC
				
Output: 		无

Return: 		0	设备未开启了家长控制
				1	设备开启了家长控制
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
#define MAX_CLIENT_NUMBER 		255
int find_ParentControl_config( const char * mac )
{
	int i = 0, client_num = 0;
	struct client_info * clients_list ;
	
	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list == NULL )
	{
		return 0;
	}
	memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
	
	client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);
	
	for(i=0; i< client_num; i++)
	{	
		qosMacToLower(clients_list[i].mac);
		
		if(strcmp(clients_list[i].mac , mac ) == 0 ){
			if(clients_list[i].limitenable == 1){
				free(clients_list);
				return 1;
			}else{
				free(clients_list);
				return 0;
			}
		}
		
	}
	
	free(clients_list);
	return 0;
}



static int add_to_url_list(char *url)
{
	if( !url )
		return -1;
	
	if(strlen(url) +1 > 256)
	{
		return -1;
	}

	char *sp = url, *ep = url;
	struct parentCtl_url_list *node;
	do
	{
		node = (struct parentCtl_url_list *)malloc(sizeof(struct parentCtl_url_list));
		if(NULL == node)
		{
			printf("malloc error .\n");
			return -1;
		}
	        
		memset(node, 0, sizeof(struct parentCtl_url_list));

		/*单条规则用逗号分割，可以过滤多个网址(如:qq,baidu),llm add*/
		ep = strchr(sp, ',');
		if(ep)
		{
			*ep = '\0';
			strcpy(node->url , sp);
			sp = ep + 1;
		}
		else
		{
			strcpy(node->url , sp);
			sp += strlen(sp);
		}

		/*跳过连续的逗号(baidu,,,qq,,,)*/
		while(*sp == ',')
		{
			sp++;
		}
		
		node->next = gParentCtlConfig.urllist;
		gParentCtlConfig.urllist = node;
	}while(*sp != '\0');
	
    return 0; 
}

static int get_parentCtl_url_list()
{
	char *value;
	
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_URLFILTER_LIST, value);

	if(strcmp(value, "") == 0)
		return 0;

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("getParentControlUrlFilterList2web: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	char *sp,*ep;

	sp = list;
	while((ep = strchr(sp, '\n'))) {
		*ep++ = '\0';
		if(strlen(sp) > 0)
			add_to_url_list(sp);
		sp = ep;
	}
	
	if(strlen(sp) > 0)
		add_to_url_list(sp);

	free(list);
	list = NULL;
	return 0;
}


static int get_parentCtl_time()
{
	char *value;
	char shour[4] = {0};
	char smin[4] = {0};
	char ehour[4] = {0};
	char emin[4] = {0};
	
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_TIME, value);
	if(strcmp(value, "") == 0)
		return 0;

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("getParentControlUrlFilterList2web: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	if (sscanf(list,"%[^:]:%[^-]-%[^:]:%s",shour,smin,ehour,emin) !=4)
	{
		printf("get_parentCtl_time wrong,return!\n");
		free(list);
		list = NULL;
		return -1;
	}
	gParentCtlConfig.stime = atoi(shour)*60*60 + atoi(smin)*60;
	gParentCtlConfig.etime = atoi(ehour)*60*60 + atoi(emin)*60;
	
	free(list);
	list = NULL;
	return 0;
}
/*
	"parentCtrlOnlineDate: "10011010" //第一个为每天，第二个为星期一,
*/
static int get_parentCtl_date()
{
	char *value;
	char date[16] = {0};
	int i = 0;
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_DATE, value);
	if(strcmp(value, "") == 0)
		return 0;
	if(strlen(value) >8)
		return 0;

	strcpy(date, value);

	if(date[7] == '1') //星期天
		gParentCtlConfig.wday[0] = 1;
	
	for(i=1; i<7; i++) //星期一到星期六
	{	
		if(date[i] == '1')
			gParentCtlConfig.wday[i] = 1;
	}

	return 0;
}
/*
	"parentCtrlURLFilterMode: disable | permit | forbid  //关闭 允许 禁止
*/
static int get_parentCtl_mode()
{
	char *value;
	int mode = MODE_DISABLED;
	_SAFE_GET_VALUE(ADVANCE_PARENT_CONTROL_URLFILTER_MODE, value);
	if(strcmp(value, "") == 0)
		return 0;

	if(strcmp(value,"forbid") == 0){
		mode = MODE_DENY;//仅禁止
	}
	else if(strcmp(value,"permit") == 0){
		mode = MODE_PASS;//仅允许
	}
	else if(strcmp(value,"disable") == 0){
		mode = MODE_DISABLED;//禁用
	}
	
	gParentCtlConfig.mode =mode;
	return mode;
	
}

int rtl_urlfilter_check(struct ip *pip)
{
	struct parentCtl_devices_list  *ptr;
	 int skip = 0;
	struct in_addr ina;
	if(in_time_area == 0)
		return skip;
	for(ptr = gParentCtlConfig.devlist;ptr != NULL; ptr = ptr->next)
	{
		if(pip->ip_dst.s_addr == ptr->ip)
		{
			//printf("=======%s========%s [%d]\n",inet_ntoa(pip->ip_dst), __FUNCTION__, __LINE__);
			skip = 1;
			break;
		}
	}
	return skip;
}

void parent_control_config_update(void)
{
	parentCtl_devices_list_flush();
	parentCtl_url_list_flush();
	memset(&gParentCtlConfig, 0x0, sizeof(struct parentCtl_config));
	get_parentCtl_devices_list();

	if(gParentCtlConfig.devlist)
	{
		get_parentCtl_time();
		get_parentCtl_date();
		if(get_parentCtl_mode() != MODE_DISABLED)
			get_parentCtl_url_list();
	}
}

enum {
	REMARK_NOT_NEED_SAVE = 0,
	REMARK_NEED_SAVE,
};
typedef struct device_remark_list
{
     int index;		//标识nvram索引
    char mac[TPI_MAC_STRING_LEN];
    char remark[TPI_BUFLEN_64];
    struct device_remark_list *next;
}Device_remark_list, *pDevice_remark_list;

struct device_remark_table
{
     int count;
    struct device_remark_list *gDevice_remark_list;
};

struct device_remark_table gDevice_remark_table;




static int add_remark_to_table(char *mac, char *remark, int need_save)
{
	pDevice_remark_list node;
	pDevice_remark_list ptr;
	int i = 0;
	char value[128] = {0};
	if(mac == NULL || remark == NULL)
		return -1;
	if(strlen(mac) != 17 ||strlen(remark) == 0)
		return -1;
	
	node = (pDevice_remark_list)malloc(sizeof(Device_remark_list));
	if(NULL == node)
	{
		printf("add_remark malloc error .\n");
		return -1;
	}
        
	memset(node, 0, sizeof(Device_remark_list));
	strncpy(node->mac, mac, TPI_MAC_STRING_LEN-1);
	strncpy(node->remark, remark, TPI_BUFLEN_64-1);
	
	if(need_save)
	{
		for(ptr = gDevice_remark_table.gDevice_remark_list; ptr != NULL; ptr = ptr->next)
		{
			if(strncasecmp(ptr->mac, node->mac, TPI_MAC_STRING_LEN-1) == 0)
			{
				printf("find sta [%s] index is %d ,update it.\n",ptr->mac, ptr->index);
				strncpy(ptr->remark, node->remark, TPI_BUFLEN_64-1);
				memset(value, 0, sizeof(value));
				sprintf(value, "%s %s", ptr->mac, ptr->remark);
				_SET_VALUE(ADVANCE_DEVICE_REMARK(ptr->index), value);
				if(node)
				{
					free(node);
			           	node = NULL;
				}
				return 0;	
			}
		}
	}

	if (gDevice_remark_table.count >= MAX_PARENTCTL_NUM)
	{
		if(node){
			free(node);
			node = NULL;
		}
		
		for(ptr = gDevice_remark_table.gDevice_remark_list; ptr != NULL; ptr = ptr->next){
			if( ptr->index == 1 ){
				ptr->index = MAX_PARENTCTL_NUM ;
				strncpy(ptr->remark, remark, TPI_BUFLEN_64-1);
				strncpy(ptr->mac, mac, TPI_MAC_STRING_LEN-1);
				
				memset(value, 0, sizeof(value));
				sprintf(value, "%s %s", ptr->mac, ptr->remark);
				_SET_VALUE(ADVANCE_DEVICE_REMARK(ptr->index), value);
				
			}else if(ptr->index > 1 && ptr->index <= MAX_PARENTCTL_NUM){
				ptr->index -= 1 ;
				
				memset(value, 0, sizeof(value));
				sprintf(value, "%s %s", ptr->mac, ptr->remark);
				_SET_VALUE(ADVANCE_DEVICE_REMARK(ptr->index), value);
			}
		}   
	}else{
		gDevice_remark_table.count++;
		node->index = gDevice_remark_table.count;
		node->next = gDevice_remark_table.gDevice_remark_list;
		gDevice_remark_table.gDevice_remark_list = node;
		printf("add sta [%s] to remark,now index is %d \n",node->mac, gDevice_remark_table.count);
		if(need_save)
		{
			memset(value, 0, sizeof(value));
			sprintf(value, "%d", gDevice_remark_table.count);
			_SET_VALUE(ADVANCE_DEVICE_REMARK_NUM, value);
			
			memset(value, 0, sizeof(value));
			sprintf(value, "%s %s", node->mac, node->remark);
			_SET_VALUE(ADVANCE_DEVICE_REMARK(node->index), value);
		}
	}
		
	return 0;
        
} 

int add_remark(char *mac, char *remark)
{
	return add_remark_to_table(mac, remark, REMARK_NEED_SAVE);
}

char *get_remark(char *mac)
{
	pDevice_remark_list ptr;
	static char remark[TPI_BUFLEN_64+1] = {0};
	if(mac == NULL)
		return NULL;
	if(strlen(mac) != 17)
		return NULL;
	
	for(ptr = gDevice_remark_table.gDevice_remark_list; ptr != NULL; ptr = ptr->next)
	{
		if(strncasecmp(ptr->mac,mac, 17) == 0)
		{
			memset(remark, 0, sizeof(remark));
			snprintf(remark, TPI_BUFLEN_64+1 ,"%s",ptr->remark);	
			return remark;
		}
	}

	return NULL;
}

void load_remark_config()
{	
	char *mib_val;
	int i = 0, num = 0;
	char remark[TPI_BUFLEN_64]={0};
	char mac[TPI_MAC_STRING_LEN]={0};
	    
	memset(&gDevice_remark_table, 0, sizeof(struct device_remark_table));
	
	_SAFE_GET_VALUE(ADVANCE_DEVICE_REMARK_NUM, mib_val);
	num = atoi(mib_val);
	if( num == 0)
		return;	

	for(i = 1; i <= num; i++)
	{
		_GET_VALUE(ADVANCE_DEVICE_REMARK(i), mib_val);
		memset(mac, 0, sizeof(mac));
		memset(remark, 0, sizeof(remark));
		if(sscanf(mib_val, "%[^ ] %s", mac, remark) !=2)
			continue;
		if(strlen(mac) == 17)
		{
			strcpy(remark,mib_val+TPI_MAC_STRING_LEN);
			add_remark_to_table( mac, remark, REMARK_NOT_NEED_SAVE); 
		}
	}		
	return;
}


/*******************MAC地址绑定接口 ****start*******************************/

#ifndef DHCPD_STATIC_LEASE_NU
#define DHCPD_STATIC_LEASE_NU 19
#endif 

void __static_dhcp_config_delete_all( void )
{
	
	int i = 0 ;
	for( i = 0 ; i <= DHCPD_STATIC_LEASE_NU ; i++){
		_SET_VALUE(ADVANCE_STATIC_IP_MAPPING_RULE(i),"");
	}
	
	return;
}

static int  tpi_same_net(unsigned int one_ip,unsigned int other_ip,unsigned int net_mask)
{
	if((one_ip&net_mask) == (other_ip&net_mask))
		return 1;
	else
		return 0;
}

static int dump_all_client(struct client_info *clients_list,int client_num)
{
	int i = 0;
	
	printf("\nno.\tip\t\tmac\t\t\ttype\ttime\tenable\thostname\tmark\n");

	for (i = 0; i < client_num; i++)
	{
		if(1 == clients_list[i].in_use)
		{
			printf("%d\t%s\t%s\t%d\t%d\t%d\t%s\t\t%s\n",
					i+1, clients_list[i].ip, clients_list[i].mac, clients_list[i].l2type,time(0) - clients_list[i].time, clients_list[i].limitenable, clients_list[i].hostname, clients_list[i].mark);
		}else{
			break;	
		}
	}
	printf("\n");
	return i;

}

static int tenda_dump_arp(struct radix_node *rn, void *req)
{
	struct rtentry *rt = (struct rtentry *)rn;
  	struct sockaddr *dst;
  	struct sockaddr_dl *sdl;
	char *lan_ip, *lan_netmask;
	arp_tbl_enty new_arp_item;
	
	int i;
	

    dst = rt_key(rt);
    sdl = (struct sockaddr_dl *)rt->rt_gateway;
    if ((rt->rt_flags & (RTF_UP | RTF_LLINFO)) == (RTF_UP | RTF_LLINFO) && sdl->sdl_alen != 0) 
    {
    	
			memset(&new_arp_item, 0, sizeof(arp_tbl_enty));
			
			/* hw addr */
			memcpy(&(new_arp_item.hw_addr), (struct ether_addr *)LLADDR(sdl), ETHER_ADDR_LEN);
			
			//mac is zero?
			if((new_arp_item.hw_addr.octet[0] | new_arp_item.hw_addr.octet[1] |
				new_arp_item.hw_addr.octet[2] | new_arp_item.hw_addr.octet[3] |
				new_arp_item.hw_addr.octet[4] | new_arp_item.hw_addr.octet[5]) == 0)
				return 0;
			
			/* ip addr */
			memcpy(&(new_arp_item.ip_addr), (struct in_addr *)&(((struct sockaddr_in *)dst)->sin_addr), 
				sizeof(struct in_addr));
	
			/* get lan_ip and lan_mask through tpi inferface */
			_SAFE_GET_VALUE(LAN_IPADDR,lan_ip);
			_SAFE_GET_VALUE(LAN_NETMASK,lan_netmask);
		
			{
				/* judge as the same network segment */				
				if (1 != tpi_same_net(new_arp_item.ip_addr.s_addr, inet_addr(lan_ip), inet_addr(lan_netmask)))
				{
					return 0;
				}
				
				/* the ipaddr is not allowed as the same as GW addr */
				if (new_arp_item.ip_addr.s_addr == inet_addr(lan_ip))
				{
					return 0;
				}
				/* end */	
			}	
			
			arp_tbl_enty *arp_tmp = (arp_tbl_enty *) req;
		
			for(i = 0; i< MAX_CLIENT_LIST_NUM; i++)
			{
					if(arp_tmp[i].ip_addr.s_addr == 0)
					{
						memcpy((void *)&arp_tmp[i], (void *)&new_arp_item, sizeof(arp_tbl_enty));	
						break;
					}	
			}
			
			if(i >= MAX_CLIENT_LIST_NUM)
			{
					printf("tenda_dump_arp: arp_count reach max!!!\n");
			}	

    }

    return 0;
}


extern void show_network_tables2(int (*mtenda_routing_entry)(struct radix_node *, void *),void *pr);
static int get_arp_clients( struct client_info * clients_list,  int max_client_num)
{
	int i,client_count,arp_idx,arp_num;
	
	struct client_info * all_tmp;
	
	int list_size = (MAX_CLIENT_LIST_NUM) * sizeof(arp_tbl_enty);
	arp_tbl_enty *arp_tmp_buf = (arp_tbl_enty *)malloc(list_size);
	
	if (NULL == arp_tmp_buf)
	{
		return -1;
	}
	
	memset(arp_tmp_buf, 0x0, list_size);
	
	show_network_tables2(tenda_dump_arp, (void *)arp_tmp_buf);
	
	for(i = 0; i< MAX_CLIENT_LIST_NUM; i++)
	{
			if(arp_tmp_buf[i].ip_addr.s_addr == 0)
			{
				break;
			}	
	}
	
	arp_num = (i >= MAX_CLIENT_LIST_NUM)? (i-1):i;
	arp_idx = 0;
	client_count = 0;
	all_tmp = clients_list;
	
	while(client_count < max_client_num && arp_idx < arp_num)
	{
			
			if(0 == all_tmp->in_use){
				//no in dhcp client list, may be static ip
					all_tmp->in_use = 1;
					strcpy(all_tmp->mac,ether_ntoa((struct ether_addr *)&arp_tmp_buf[arp_idx].hw_addr));
					inet_ntoa_r(arp_tmp_buf[arp_idx].ip_addr,all_tmp->ip);
					strcpy(all_tmp->hostname,"");//no hostname
					all_tmp->l2type = 0;//defautl
					arp_idx++;
		
			}
			all_tmp++;
			client_count++;					
	}
	
	if(client_count >= max_client_num)
	{
			printf("get_arp_clients: client_count reach max!!!\n");
	}
	
	free(arp_tmp_buf);
	return client_count;
}

void LoadWanListen(int load)
{
	char WanIP[24] = {0};
	int port;
	char_t *WanPort,*pHost;

	LoadManagelist();

	if(load){

		webs_Wan_CloseListen();

		_SAFE_GET_VALUE(ADVANCE_RM_WEB_PORT,WanPort);
		port = atoi(WanPort);
		if(SYS_wan_up != 1/*CONNECTED*/)
			return ;

		strcpy(WanIP,NSTR(SYS_wan_ip));
		pHost = WanIP;
		
		webs_Wan_OpenListen(port,pHost);
	}
	else{
		webs_Wan_CloseListen();
	}

}

void LoadManagelist()
{
	char bufTmp[128]; //enable[4];
	//char remote_port[32];
	char  *enable, *lan_ip, *lan_msk, *rm_ip;
	char *remote_port;
//	char *ip1,*ip2;
	g_enable_remote_acl = 0;
	memset(bufTmp,0,sizeof(bufTmp));
	
	_SAFE_GET_VALUE(ADVANCE_RM_WEB_EN, enable);
	_SAFE_GET_VALUE(ADVANCE_RM_WEB_PORT, remote_port);
	_SAFE_GET_VALUE(LAN_IPADDR, lan_ip);
	_SAFE_GET_VALUE(LAN_NETMASK, lan_msk);
	
	strcpy(bufTmp,lan_ip);
	inet_aton(bufTmp,&g_localip);

	strcpy(bufTmp,lan_msk);
	inet_aton(bufTmp,&g_localmask);
	g_remote_port = atoi(remote_port);
	
	if(atoi(enable) == 1)
	{//remote
		g_enable_remote_acl = 1;
		
		_SAFE_GET_VALUE(ADVANCE_RM_WEB_IP,rm_ip);
		strcpy(bufTmp,rm_ip);
		if(strlen(bufTmp) >=3/*0-0*/){
#if 0
			ip1 = strchr(bufTmp,';');
			if(ip1){
				*ip1 = '\0';
				ip2 = ip1+1;
				ip1 = bufTmp;
				inet_aton(ip1,&g_remoteIPStart);
				inet_aton(ip2,&g_remoteIPEnd);
			}else{
				g_enable_remote_acl = 0;/*wrong format*/	
			}
#else
			inet_aton(bufTmp,&g_remoteIPStart);
#endif
		}else{
			g_enable_remote_acl = 0;
		}
	}	
}

int checkAcl(unsigned int ip,int port)
{
	if((ip & g_localmask) == (g_localip & g_localmask))
	{
		return 1;
	} 
	else//wan ip
	{
		if(g_enable_remote_acl != 1)
		{
			return 0;
		}

		if((g_remoteIPStart == 0) /*&& (g_remoteIPEnd == 0)*/ && port == g_remote_port)
		{
			return 1;
		}
		//if((ip >= g_remoteIPStart) && (ip <= g_remoteIPEnd) && port == g_remote_port) 
		if(ip == g_remoteIPStart && port == g_remote_port) 
		{
			return 1;
		} else
		{
			return 0;
		}
	}
}
