
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
#include "route_cfg.h"
#include "flash_cgi.h"
#include "include/router_net.h"
#include  "cJSON.h"
#ifdef __CONFIG_STREAM_STATISTIC__
#include "../tc/tc.h"
#endif

/********************************firewall define********************/
#define FLT_MAC_MAX_NUM_1		10
#define URL_FILTER_COUNT 		10
#define MAC_FILTER_COUNT 		10
/****************************************************************/

extern int webs_Wan_OpenListen(int port,char *HostIP);
extern void webs_Wan_CloseListen();
//roy +++2010/09/19
extern void sys_restart(void);
extern void sys_reboot(void);
//+++
/****************************************************************/

#define VALUEMAXLEN			128
#define NAMEMAXLEN			32

//static char do_cmd_go[128];
unsigned int g_localip,g_localmask,g_remoteIPStart,g_remoteIPEnd;
int g_enable_remote_acl,g_remote_port;

void 		LoadWanListen(int load);
void 		LoadManagelist();
static int 		getfirewall(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetIP(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetUrl(int eid, webs_t wp, int argc, char_t **argv);
static int 		aspmAclGetMacFilter(int eid, webs_t wp, int argc, char_t **argv);

static void 	fromSafeClientFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeUrlFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeMacFilter(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeWanPing(webs_t wp, char_t *path, char_t *query);
static void	fromSafeWanWebMan(webs_t wp, char_t *path, char_t *query);
static void 	fromSafeNetAttack(webs_t wp, char_t *path, char_t *query);

#define TPI_BUFLEN_64       64    //!< buffer length 64
#define TPI_BUFLEN_16       16    
#define TPI_BUFLEN_8      	    8   
#define TPI_MACLEN	6					/* MAC长度 */
#define TPI_MAC_STRING_LEN	18			/* MAC字符串长度 */
#define TPI_IP_STRING_LEN	16			/* IP字符串长度 */
#define TPI_IFNAME_LEN		16			/* 接口名称长度 */
#define MAX_CLIENT_LIST_NUM   128
#define MAX_PARENTCTL_NUM   20

typedef struct client_info{
        int in_use;                                //now in use
        unsigned char mac[TPI_MAC_STRING_LEN];     //mac address
        unsigned char ip[TPI_IP_STRING_LEN];       //ip address
	unsigned char hostname[TPI_BUFLEN_64];     //hostname
	unsigned char mark[TPI_BUFLEN_64];     //mark
	int l2type;                                //layer2 type: wired or wireless 
	time_t time; //sta第一次接入路由器的时间
	time_t interval_time;//sntp更新前的时间间隔	
	int limitenable;
}arp_client_info;


typedef struct mac_ip_tbl{
	struct ether_addr hw_addr;
	struct	in_addr ip_addr;
}arp_tbl_enty;

struct detec{
        char inuse;
        char auth;
        unsigned char mac[6];
        in_addr_t ip;
	char hostname[64];
	int flag;
	time_t time; //sta第一次接入路由器的时间
	time_t interval_time;//sntp更新前的时间间隔
	time_t update_time;
};
extern struct detec det[255];

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

struct backlist_device_list{
	struct backlist_device_list  *next;
	unsigned char  mac[ETHER_ADDR_LEN];
	unsigned char  flag;
};

struct backlist_device_list *backlist_device_list_head = NULL;


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

int is_backlist(char *mac)
{
	struct backlist_device_list *ptr = NULL;
	struct ether_addr *hw_addr = NULL;
	
	if(NULL == mac)
		return -1;
	if(strlen(mac) != 17)
		return -1;
	
	hw_addr = ether_aton(mac);
	if(!hw_addr)
		return -1;

	for(ptr = backlist_device_list_head; ptr != NULL; ptr = ptr->next)
	{
		if(memcmp(hw_addr->octet, ptr->mac, ETHER_ADDR_LEN) == 0) 
		{
			printf("find mac in backlist  %02x:%02x:%02x:%02x:%02x:%02x\n",ptr->mac[0],ptr->mac[1],ptr->mac[2],ptr->mac[3],
			ptr->mac[4],ptr->mac[5]);
			return 1;
		}
	}

	return 0;
}

int init_backlist(void)
{
	
	char name[] = "filter_macXXXXXXXXXX", value[1000];
	char *blacklist_mac = NULL, *desc = NULL;
	struct backlist_device_list *backlist_device_list = NULL;

	int which = 0;
	struct ether_addr *hw_addr = NULL;

	clean_filter_mac_list();
	
	for (which = 0; which < 20; which++) {
		
		snprintf(name, sizeof(name), "filter_mac%d", which);
		strncpy(value, nvram_safe_get(name), sizeof(value));
		
		desc = value;
		blacklist_mac = strsep(&desc, ",");
		if (!blacklist_mac){
			continue;
		}
		
		hw_addr = ether_aton(blacklist_mac);
		if(!hw_addr){
			continue;
		}
		
		if(backlist_device_list_head == NULL ){
			backlist_device_list_head = (struct backlist_device_list *)malloc(sizeof(struct backlist_device_list));
			if(backlist_device_list_head ==NULL ){
				return FALSE;
			}
			backlist_device_list = backlist_device_list_head;
			memset(backlist_device_list, 0, sizeof(struct backlist_device_list));
		}else{
			backlist_device_list = backlist_device_list_head;
			while(backlist_device_list->next != NULL){
				backlist_device_list = backlist_device_list->next;
			}
			backlist_device_list->next = (struct backlist_device_list *)malloc(sizeof(struct backlist_device_list));
			if(backlist_device_list->next ==NULL ){
				return FALSE;
			}
			backlist_device_list = backlist_device_list->next;
			memset(backlist_device_list, 0, sizeof(struct backlist_device_list));
		}
		memcpy(backlist_device_list->mac, hw_addr->octet, sizeof(hw_addr->octet));
		
	}
	return TRUE;
	
}



void clean_filter_mac_list( void )
{
	struct backlist_device_list *backlist_device_list = NULL , *backlist_device_list_next= NULL ;
	
	backlist_device_list = backlist_device_list_head;

	backlist_device_list_head = NULL;

	while(backlist_device_list!=NULL){
		
		backlist_device_list_next = backlist_device_list->next;
		free(backlist_device_list);
		backlist_device_list = backlist_device_list_next;
	}
}

int add_filter_mac_list(const char *m_mac)
{	
	struct ether_addr *hw_addr;
	struct backlist_device_list *backlist_device_list;
	
	if( m_mac == NULL ){
		return FALSE;
	}
	
	hw_addr = ether_aton(m_mac);

	if(!hw_addr){
		return FALSE;
	}

	if(backlist_device_list_head == NULL){
		backlist_device_list_head = (struct backlist_device_list *)malloc(sizeof(struct backlist_device_list));
		if(backlist_device_list_head ==NULL ){
			return FALSE;
		}
		backlist_device_list = backlist_device_list_head;
		memset(backlist_device_list, 0, sizeof(struct backlist_device_list));
	}else{
		backlist_device_list = backlist_device_list_head;
		while(backlist_device_list->next != NULL){
			backlist_device_list = backlist_device_list->next;
		}
		backlist_device_list->next = (struct backlist_device_list *)malloc(sizeof(struct backlist_device_list));
		if(backlist_device_list->next ==NULL ){
			return FALSE;
		}
		backlist_device_list = backlist_device_list->next;
		memset(backlist_device_list, 0, sizeof(struct backlist_device_list));
	}
	memcpy(backlist_device_list->mac, hw_addr->octet, sizeof(hw_addr->octet));
	
	return TRUE;
}


static int add_to_devices_list(char *mac, char  *ip)
{
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
	
	node->ip =  inet_addr(ip);
	node->mac_filter_status = 0;
	memcpy(node->mac, hw_addr->octet, sizeof(node->mac));
	
	node->next = gParentCtlConfig.devlist;
	gParentCtlConfig.devlist = node;
	
       return 0; 
}

static int parsePerParentControlClientConfig(char *value, char *hostname, char *mark,char *mac, char *ip, char *enable)
{
	char *sp,*ep;
	
	if( !value || !hostname ||  !mark || !mac || !ip ||!enable)
		return -1;

	if(strlen(value) < 28) //最小长度31
	{
		return -1;
	}

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("parsePerParentControlClientConfig: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	sp = list;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	if(strlen(sp) > TPI_BUFLEN_64-1)
		strncpy(hostname, sp, TPI_BUFLEN_64-1);
	else
		strncpy(hostname, sp, strlen(sp));

	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	if(strlen(sp) > TPI_BUFLEN_64-1)
		strncpy(mark, sp, TPI_BUFLEN_64-1);
	else
		strncpy(mark, sp, strlen(sp));

	
	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	strcpy(mac, sp);
	
	sp = ep;
	if((ep = strchr(sp, '\t')) == NULL)
	{
		free(list);
		list = NULL;
		return -1;
	}
	*ep++ = '\0';
	strcpy(ip, sp);
	
	sp = ep;
	strcpy(enable, sp);

	free(list);
	list = NULL;
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

	_SAFE_GET_VALUE(PARENT_CONTROL_CLIENT_NUM, mib_val);
	client_mib_count = atoi(mib_val);

	for(i=0;i<client_mib_count;i++)
	{
		_SAFE_GET_VALUE(PARENT_CONTROL_CLIENT_LIST(i+1), mib_val);//hostname \t ip \t limitEn
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
		if(is_backlist(mac) != 0 )
			continue;
			
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
	
	struct parentCtl_url_list *node;
	node = (struct parentCtl_url_list *)malloc(sizeof(struct parentCtl_url_list));
	if(NULL == node)
	{
		printf("malloc error .\n");
		return -1;
	}
        
	memset(node, 0, sizeof(struct parentCtl_url_list));
	
	strcpy(node->url , url);
	
	node->next = gParentCtlConfig.urllist;
	gParentCtlConfig.urllist = node;
	
       return 0; 
}

static int get_parentCtl_url_list()
{
	char *value;
	
	_SAFE_GET_VALUE(PARENT_CONTROL_URL_FILTER_LIST, value);

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
	
	_SAFE_GET_VALUE(PARENT_CONTROL_TIME, value);
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
	_SAFE_GET_VALUE(PARENT_CONTROL_DATE, value);
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
	_SAFE_GET_VALUE(PARENT_CONTROL_URL_FILTER_MODE, value);
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

void show_parentCtl_config(void)
{
	printf("\n##########show_parentCtl_config###########\n");
	struct parentCtl_devices_list  *ptr;
	for(ptr = gParentCtlConfig.devlist;ptr != NULL; ptr = ptr->next)
	{
		printf("ip:%s  mac %02x:%02x:%02x:%02x:%02x:%02x next:%p\n", inet_ntoa_tenda(ptr->ip),ptr->mac[0],ptr->mac[1],ptr->mac[2],ptr->mac[3],
			ptr->mac[4],ptr->mac[5],ptr->next);
	}
	printf("stime:%d etime:%d\n",gParentCtlConfig.stime, gParentCtlConfig.etime);
	
	printf("sunday select=%d\n",gParentCtlConfig.wday[0]);
	printf("Monday select=%d\n",gParentCtlConfig.wday[1]);
	printf("Tuesday select=%d\n",gParentCtlConfig.wday[2]);
	printf("Wednesday select=%d\n",gParentCtlConfig.wday[3]);
	printf("Thursday select=%d\n",gParentCtlConfig.wday[4]);
	printf("Friday select=%d\n",gParentCtlConfig.wday[5]);
	printf("Saturday select=%d\n",gParentCtlConfig.wday[6]);
	
	printf("mode:%d\n",gParentCtlConfig.mode);
	
	struct parentCtl_url_list  *uptr;
	for(uptr = gParentCtlConfig.urllist;uptr != NULL; uptr = uptr->next)
	{
		printf("url:%s next:%p\n",uptr->url, uptr->next);
	}
	
	printf("##############################\n");
	
}
void show_backlist_device_list_config(void)
{
	printf("\n##########show_backlist_device_list_config###########\n");
	struct backlist_device_list *ptr;
	for(ptr = backlist_device_list_head;ptr != NULL; ptr = ptr->next)
	{
		printf("mac %02x:%02x:%02x:%02x:%02x:%02x next:%p\n",ptr->mac[0],ptr->mac[1],ptr->mac[2],ptr->mac[3],
			ptr->mac[4],ptr->mac[5],ptr->next);
	}
	printf("##############################\n");
}

extern void macfilter_flush(void);
void parent_control_config_update(void)
{
//	macfilter_flush();
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
	show_parentCtl_config();
}

/*
sntp同步后时间有变化，保存ntp更新前的在线时间
*/
void setArpStaIntervalTime()
{
	int i = 0;
	time_t now_time = time(0);
	 for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
                if( 1 == det[i].inuse)                                                                                                  
                {                                                                                                                                                              
                  	det[i].interval_time = now_time - det[i].time ; 
			 
                }                                                                                                                                                              
        }
	 printf("%s \n",__func__);
	return ;
}

/*
sntp同步后保存时间更新导致的误差,重新计算sta接入时间
*/
void updateArpStaTime()
{
	int i = 0;
	time_t now_time = time(0);
	 for(i=0; i<255; i++)                                                                                                                                                   
        {                                                                                                                                                                      
                if( 1 == det[i].inuse)                                                                                                  
                {                                                                                                                                                              
                  	det[i].time = now_time - det[i].interval_time ; 
                }                                                                                                                                                              
        }
	  printf("%s \n",__func__);
	return ;
}

enum {
	REMARK_ADD = 0,
	REMARK_DEL,
	REMARK_UPDATE
};
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


void show_remark()
{
	pDevice_remark_list ptr;
	printf("#####show_remark#####\n");
	printf("count:%d\n",gDevice_remark_table.count);
	for(ptr = gDevice_remark_table.gDevice_remark_list; ptr != NULL; ptr = ptr->next)
	{
		printf("index:%d mac:%s mark:%s\n",ptr->index, ptr->mac, ptr->remark);
	}
}

static int add_remark_to_table(char *mac, char *remark, int need_save)
{
	pDevice_remark_list node;
	pDevice_remark_list ptr;
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

	if (gDevice_remark_table.count >= MAX_PARENTCTL_NUM)
       {
            printf ("remark count is over %d !\n",MAX_PARENTCTL_NUM);
            if(node)
		{
			free(node);
		       node = NULL;
		}
		return -1;   
       }
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
				_SET_VALUE(DEVICE_REMARK_LIST(ptr->index), value);
				if(node)
				{
					free(node);
			           	node = NULL;
				}
				return 0;	
			}
		}
	}
	
	gDevice_remark_table.count++;
	node->index = gDevice_remark_table.count;
	node->next = gDevice_remark_table.gDevice_remark_list;
	gDevice_remark_table.gDevice_remark_list = node;
	printf("add sta [%s] to remark,now index is %d \n",node->mac, gDevice_remark_table.count);
	if(need_save)
	{
		memset(value, 0, sizeof(value));
		sprintf(value, "%d", gDevice_remark_table.count);
		_SET_VALUE(DEVICE_REMARK_NUM, value);
		
		memset(value, 0, sizeof(value));
		sprintf(value, "%s %s", node->mac, node->remark);
		_SET_VALUE(DEVICE_REMARK_LIST(node->index), value);
		return 0;
	}
        
} 

int add_remark(char *mac, char *remark)
{
	return add_remark_to_table(mac, remark, REMARK_NEED_SAVE);
}

char *get_remark(char *mac)
{
	pDevice_remark_list ptr;
	static char remark[TPI_BUFLEN_64] = {0};
	if(mac == NULL)
		return NULL;
	if(strlen(mac) != 17)
		return NULL;
	
	for(ptr = gDevice_remark_table.gDevice_remark_list; ptr != NULL; ptr = ptr->next)
	{
		if(strncasecmp(ptr->mac,mac, 17) == 0)
		{
			memset(remark, 0, sizeof(remark));
			strncpy(remark, ptr->remark, TPI_BUFLEN_64-1);	
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
	
	_SAFE_GET_VALUE(DEVICE_REMARK_NUM, mib_val);
	num = atoi(mib_val);
	if( num == 0)
		return;	

	for(i = 1; i <= num; i++)
	{
		_GET_VALUE(DEVICE_REMARK_LIST(i), mib_val);
		memset(mac, 0, sizeof(mac));
		memset(remark, 0, sizeof(remark));
		if(sscanf(mib_val, "%s %s", mac, remark) !=2)
			continue;
		if(strlen(mac) == 17)
			add_remark_to_table( mac, remark, REMARK_NOT_NEED_SAVE); 
	}		
	return;
}


/*******************MAC地址绑定接口 ****start*******************************/
typedef struct static_dhcp_list
{
    int  index;		//标识nvram索引
    char mac[TPI_MAC_STRING_LEN];
    char ip[TPI_IP_STRING_LEN];
    struct static_dhcp_list *next;
}Static_dhcp_list, *pStatic_dhcp_list;

struct static_dhcp_table
{
    int count;
    struct static_dhcp_list *gStatic_dhcp_list;
};

struct static_dhcp_table gStatic_dhcp_table;

#ifndef DHCPD_STATIC_LEASE_NU
#define DHCPD_STATIC_LEASE_NU 19
#endif 

/**********************************************************
Function:		load_static_dhcp_config_init

Description:	将静态DHCP配置初始化到全局链表当中，方便快速
				查找和保存
				
Input:			无	
Output: 		无
Return: 		无
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
void load_static_dhcp_config_init()
{
	int  i = 0 ;
	char *value = NULL;
	char *arglists[9]= {NULL};
	char static_value[256] = {0};
	struct static_dhcp_list *Static_dhcp_list_node = NULL;

	gStatic_dhcp_table.count = 0 ; 
	gStatic_dhcp_table.gStatic_dhcp_list = NULL ;
	
	for ( i = 1; i <= DHCPD_STATIC_LEASE_NU+1 ; ++i )
	{
		_GET_VALUE(LAN0_DHCP_SATIC(i),value);
		sprintf(static_value , "%s" , value);
		if (str2arglist(static_value, arglists, ';', 5) != 5)
			continue;

		/*mac 转换为小写字符串格式*/
		qosMacToLower(arglists[2]);
		
		if( Static_dhcp_list_node != NULL){
			Static_dhcp_list_node->next = (pStatic_dhcp_list)malloc(sizeof(struct static_dhcp_list));
			if(Static_dhcp_list_node->next == NULL ){
				break;
			}
			
			Static_dhcp_list_node = Static_dhcp_list_node->next;
		}else{
			Static_dhcp_list_node = (pStatic_dhcp_list)malloc(sizeof(struct static_dhcp_list));
			if(Static_dhcp_list_node == NULL ){
				break;
			}
			
			gStatic_dhcp_table.gStatic_dhcp_list = Static_dhcp_list_node;
		}
		gStatic_dhcp_table.count++;
		Static_dhcp_list_node->index = i ;
		snprintf(Static_dhcp_list_node->ip , TPI_IP_STRING_LEN , "%s" , arglists[1]);
		snprintf(Static_dhcp_list_node->mac, TPI_MAC_STRING_LEN, "%s" , arglists[2]);
		Static_dhcp_list_node->next = NULL;

	}
}


/**********************************************************
Function:		static_dhcp_config_cp_all

Description:	将DHCP静态分配的配置拷贝一份
				
Input:			无	
Output: 		struct static_dhcp_table *Static_dhcp_table

Return: 		0	成功
				非0	失败
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
int static_dhcp_config_cp_all(struct static_dhcp_table *Static_dhcp_table)
{
	//留给修改IP时更新配置使用


}

/**********************************************************
Function:		static_dhcp_config_get

Description:	将DHCP静态分配的全局表返回
				
Input:			无	
Output: 		无
Return: 		struct static_dhcp_table 

Others: 		无
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
struct static_dhcp_table static_dhcp_config_get(void)
{
	return gStatic_dhcp_table;
}

/**********************************************************
Function:		static_dhcp_config_find

Description:	从链表中查找指定项目的配置

Input:			const char* mac		需要查找的设备的MAC
				const char * ip		需要查找的设备的IP			
Output: 		无

Return: 		非NULL	查找到的设置的配置
				NULL	为查找到
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
struct static_dhcp_list * static_dhcp_config_find( const char* mac , const char * ip)
{
	struct static_dhcp_list *static_dhcp_list_node = NULL ;
	
	static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;
	while(static_dhcp_list_node != NULL ){

		if(strcmp(static_dhcp_list_node->mac , mac) == 0){
			return static_dhcp_list_node;
		}
		static_dhcp_list_node= static_dhcp_list_node->next;
	}
	return NULL;
}

/**********************************************************
Function:		static_dhcp_config_change_one

Description:	修改DHCP配置中的一项
				
Input:			const char* mac		需要修改的设备的MAC
				const char* ip		需要修改的设备的IP
				
Output: 		无

Return: 		0	查找到指定项并修改成功
				1	为查找到指定项但添加成功
				-1	失败
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
int static_dhcp_config_change_one(const char* mac , const char* ip)
{
	char listBuf[256] = {0};
	struct static_dhcp_list *static_dhcp_list_node = NULL ;

	//删除被占用的IP。
	static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;
	while(static_dhcp_list_node != NULL ){
		
		if((strcmp(static_dhcp_list_node->ip , ip) == 0)
		 && ((strcmp(static_dhcp_list_node->mac , mac) != 0))){
			static_dhcp_config_delete(static_dhcp_list_node->mac);
		}
		static_dhcp_list_node= static_dhcp_list_node->next;
	}
	
	static_dhcp_list_node = static_dhcp_config_find(mac , ip);
	if(static_dhcp_list_node == NULL ){
		static_dhcp_config_add(mac , ip);
		return 1;
	}else{
		snprintf( static_dhcp_list_node->ip , TPI_IP_STRING_LEN , "%s" , ip );
		
		sprintf(listBuf," ;%s;%s;1;60",ip,mac);
		_SET_VALUE(LAN0_DHCP_SATIC(static_dhcp_list_node->index),listBuf);
		return 0;
	}
	return -1;
}


/**********************************************************
Function:		static_dhcp_config_add

Description:	添加一项配置到链表当中
				
Input:			const char* mac		需要添加的设备的MAC
				const char* ip		需要添加的设备的IP
				
Output: 		无

Return: 		0	添加成功
				非0	添加失败
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
int static_dhcp_config_add( const char* mac , const char* ip)
{
	int i = 0 ;
	int index = 0;
	char listBuf[256] = {0};
	struct static_dhcp_list *static_dhcp_list_node = NULL ;
	struct static_dhcp_list *static_dhcp_list_next = NULL ;

	if(gStatic_dhcp_table.count >= DHCPD_STATIC_LEASE_NU+1){
		printf ("[LCC] %s %d : add static dhcp config error \n" , __FUNCTION__ , __LINE__  );
		return -1;
	}

	static_dhcp_list_next = (pStatic_dhcp_list)malloc(sizeof(struct static_dhcp_list));
	
	if( static_dhcp_list_next == NULL ){
		printf ("[LCC] %s %d : malloc  struct static_dhcp_list error\n" , __FUNCTION__ , __LINE__  );
		return -1;
	}
	
	if( gStatic_dhcp_table.gStatic_dhcp_list == NULL ){
		gStatic_dhcp_table.gStatic_dhcp_list = static_dhcp_list_next;
		static_dhcp_list_next->index  = 1;
	}else {
	
		static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;
		while(static_dhcp_list_node->next != NULL ){
			static_dhcp_list_node = static_dhcp_list_node->next;
		}
		static_dhcp_list_node->next = static_dhcp_list_next;
		static_dhcp_list_next->index =  static_dhcp_list_node->index +1;
	}

	static_dhcp_list_next->next = NULL;
	snprintf(static_dhcp_list_next->ip , TPI_IP_STRING_LEN  , "%s" , ip);
	snprintf(static_dhcp_list_next->mac, TPI_MAC_STRING_LEN , "%s" , mac);
	
	gStatic_dhcp_table.count++;
	
	sprintf(listBuf," ;%s;%s;1;60",ip,mac);

	_SET_VALUE(LAN0_DHCP_SATIC(static_dhcp_list_next->index),listBuf);

	return 0;
}


/**********************************************************
Function:		static_dhcp_config_delete

Description:	从链表中删除一个配置，并更新链表和配置
				
Input:			const char* mac		需要删除的设备的MAC
							
Output: 		无

Return: 		0	查找到指定项目，并删除成功
				1	为查找到指定项目
				非0	删除失败
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
int static_dhcp_config_delete( const char* mac )
{
	int i = 0;
	char listBuf[256] = {0};
	struct static_dhcp_list *static_dhcp_list_node = NULL ;
	struct static_dhcp_list *static_dhcp_list_node_ago = NULL ;

	static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;

	while(static_dhcp_list_node != NULL ){
		if(strcmp(static_dhcp_list_node->mac , mac) == 0){
			break;
		}
		static_dhcp_list_node_ago = static_dhcp_list_node;
		static_dhcp_list_node = static_dhcp_list_node->next;
	}

	if(static_dhcp_list_node == NULL ){
		return 1;
	}

	if(static_dhcp_list_node_ago == NULL){
		static_dhcp_list_node_ago = static_dhcp_list_node;
		gStatic_dhcp_table.gStatic_dhcp_list = static_dhcp_list_node_ago->next;
	}else{
		static_dhcp_list_node_ago->next = static_dhcp_list_node->next;
	}

	free(static_dhcp_list_node);
	gStatic_dhcp_table.count--;

	for( i = 0 ; i <= DHCPD_STATIC_LEASE_NU ; i++){
		_SET_VALUE(LAN0_DHCP_SATIC(i+1),"");
	}

	static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;
	for( i = 0 ; i < gStatic_dhcp_table.count ; i++ ){
		if(static_dhcp_list_node == NULL ){
			continue;
		}
		sprintf(listBuf," ;%s;%s;1;60",static_dhcp_list_node->ip ,static_dhcp_list_node->mac);
		_SET_VALUE(LAN0_DHCP_SATIC(i+1),listBuf);
		static_dhcp_list_node->index = i+1;
	}
	return 0;
}

/**********************************************************
Function:		static_dhcp_config_delete_all

Description:	清空所有配置
				
Input:			const char* mac		需要删除的设备的MAC
							
Output: 		无

Return: 		0	清空成功
				非0	清空失败
				
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
int static_dhcp_config_delete_all( void )
{
	int i = 0 ;
	struct static_dhcp_list *static_dhcp_list_node = NULL ;
	struct static_dhcp_list *static_dhcp_list_next = NULL ;
	
	for( i = 0 ; i <= DHCPD_STATIC_LEASE_NU ; i++){
		_SET_VALUE(LAN0_DHCP_SATIC(i+1),"");
	}
	static_dhcp_list_node = gStatic_dhcp_table.gStatic_dhcp_list;
	gStatic_dhcp_table.count = 0;
	gStatic_dhcp_table.gStatic_dhcp_list= NULL ;

	while(static_dhcp_list_node != NULL ){
		static_dhcp_list_next = static_dhcp_list_node->next;
		free(static_dhcp_list_node);
		static_dhcp_list_node = static_dhcp_list_next;
	}
	return 0;
	
}

/**********************************************************
Function:		static_dhcp_config_updata

Description:	更新链表并更新配置
				
Input:			struct static_dhcp_table * Static_dhcp_table
							
Output: 		无
Return: 		无			
Others: 		无
	
Author: 		刘乘驰
change date:	2015-07-10
**************************************************************/
void static_dhcp_config_updata( struct static_dhcp_table * Static_dhcp_table)
{



}



/*******************MAC地址绑定接口 ****end*******************************/

/*
保存onlineList，返回list个数

onlineList:	192.168.100.100	false 192.168.100.100	false
存值范例：onlineList"=hostname \t ip \t limitEn \n hostname \t ip \t limitEn
*/
static int saveParentControlOnlineList(char *list)
{
	int num = 0;
	char str_num[8] = {0};
	char *sp,*ep;
	char hostname[TPI_BUFLEN_64] = {0};
	char mark[TPI_BUFLEN_64] = {0};
	char ip[TPI_IP_STRING_LEN] = {0};
	char mac[TPI_MAC_STRING_LEN] = {0};
	char limitenable[8] = {0};
	
	if(NULL == list)
		return -1;
	
	if(strlen(list) < 28)//onlineList的最小字符串，mac(17)+ip(7)+enable(4)+3*\t =31
	{
		_SET_VALUE(PARENT_CONTROL_CLIENT_NUM, "0");
		return -1;
	}
	num++;
	sp = list;
	while((ep = strchr(sp, '\n'))) {
		*ep++ = '\0';
		memset(hostname, 0x0, sizeof(hostname));
		memset(mark, 0x0, sizeof(mark));
		memset(ip, 0x0, sizeof(ip));
		memset(mac, 0x0, sizeof(mac));
		memset(limitenable, 0x0, sizeof(limitenable));
		if(parsePerParentControlClientConfig(sp, hostname, mark, mac, ip, limitenable) !=0)
			return -1;
		
		qosMacToLower(mac);
		if(strlen(mark) > 0)
		{
			if( add_remark(mac, mark) != 0)
				return -1;
		}else if(get_remark(mac) != NULL){	
			add_remark(mac, hostname);
		}
			
		_SET_VALUE(PARENT_CONTROL_CLIENT_LIST(num),  sp);
		if(strcmp("false" , limitenable ) != 0){
			static_dhcp_config_change_one(mac, ip);
		}else{
			//删除静态列表中的配置
			stream_list_t  *qosInfo = (stream_list_t *)malloc(sizeof(stream_list_t));
			if(qosInfo != NULL && ((qosGetMib(qosInfo,mac) == 0) || 
	 			(qosInfo->up_limit == 301 && qosInfo->down_limit == 301))){
				static_dhcp_config_delete(mac);
			}
			if(qosInfo != NULL){
				free(qosInfo);
			}
		}
		sp = ep;
		num++;	
	}
	/* 只有一条或最后一条*/
	memset(hostname, 0x0, sizeof(hostname));
	memset(mark, 0x0, sizeof(mark));
	memset(ip, 0x0, sizeof(ip));
	memset(mac, 0x0, sizeof(mac));
	memset(limitenable, 0x0, sizeof(limitenable));
	if(parsePerParentControlClientConfig(sp, hostname, mark, mac, ip, limitenable) !=0)
		return -1;

	qosMacToLower(mac);
	if(strlen(mark) > 0)
	{
		if( add_remark(mac, mark) != 0)
			return -1;
	}else if(get_remark(mac) != NULL){
		add_remark(mac, hostname);
	}
	_SET_VALUE(PARENT_CONTROL_CLIENT_LIST(num),  sp);		
	if(strcmp("false" , limitenable ) != 0){
		static_dhcp_config_change_one(mac, ip);
	}else{
		//删除静态列表中的配置
		stream_list_t  *qosInfo = (stream_list_t *)malloc(sizeof(stream_list_t));
		if(qosInfo != NULL && ((qosGetMib(qosInfo,mac) == 0) || 
 			(qosInfo->up_limit == 301 && qosInfo->down_limit == 301))){
			static_dhcp_config_delete(mac);
		}
		if(qosInfo != NULL){
			free(qosInfo);
		}
	}

	/*保存条目个数*/
	sprintf(str_num, "%d", num);
	if(num > MAX_PARENTCTL_NUM)
	{
		_SET_VALUE(PARENT_CONTROL_CLIENT_NUM, "20");
		return MAX_PARENTCTL_NUM;
	}
	_SET_VALUE(PARENT_CONTROL_CLIENT_NUM, str_num);

	return num;
}

/*
onlineList:	192.168.100.100	false 192.168.100.100	false
parentCtrlOnlineTime:05:10-17:35
parentCtrlOnlineDate:01100011
parentCtrlURLFilterMode:true
urlList:http://www.baidu.com qq.com

存值范例：onlineList"=hostname \t ip \t limitEn \n hostname \t ip \t limitEn
& parentCtrlOnlineTime=19:00-11:20
& parentCtrlOnlineDate=1;0;1;1;0;1;0;1
& parentCtrlURLFilterMode= permit
& urlList = http://www.baidu.com \n  http://www.baidu.com

*/
#define SUCCESS_RESULT	"{\"errCode\":\"0\"}"
#define ERROR_RESULT	"{\"errCode\":\"1\"}"

static void fromSetParentControl(webs_t wp, char_t *path, char_t *query)
{	
	int ret = 0;
	char *onlineList, *onlineTime, *onlineDate, *urlFilterMode, *urlList;
	onlineList = websGetVar(wp, T("onlineList"), T("")); //!!!注意:后续添加默认值
	onlineTime = websGetVar(wp, T("parentCtrlOnlineTime"), T("")); 
	onlineDate = websGetVar(wp, T("parentCtrlOnlineDate"), T("11111111")); 
	urlFilterMode = websGetVar(wp, T("parentCtrlURLFilterMode"), T("disable")); 
	urlList = websGetVar(wp, T("urlList"), T("")); 
	ret = saveParentControlOnlineList(onlineList);
	_SET_VALUE(PARENT_CONTROL_TIME, onlineTime);
	_SET_VALUE(PARENT_CONTROL_DATE, onlineDate);
	_SET_VALUE(PARENT_CONTROL_URL_FILTER_MODE, urlFilterMode);
	_SET_VALUE(PARENT_CONTROL_URL_FILTER_LIST, urlList);
	_COMMIT();
	
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	if(ret)
		websWrite(wp, T("%s"),SUCCESS_RESULT);
	else
		websWrite(wp, T("%s"),ERROR_RESULT);

	websDone(wp, 200);

	/*配置*/
	update_firewall();
	
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
			_SAFE_GET_VALUE(_LAN0_IP,lan_ip);
			_SAFE_GET_VALUE(_LAN0_NETMASK,lan_netmask);
		
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

static void updateParentControlClientInfo(struct client_info *clients_list,int client_num)
{
	int i = 0, j = 0;
	struct ether_addr *hw_addr;
	char *remark;

	for (i = 0; i < client_num; i++)
	{
		if( 1 == clients_list[i].in_use )                                                                                                  
		{ 
			remark = get_remark(clients_list[i].mac);
			if(remark)
				strncpy(clients_list[i].mark, remark,TPI_BUFLEN_64-1);
			hw_addr = ether_aton(clients_list[i].mac);
			if(!hw_addr)
				continue ;
			for(j=0; j<255; j++)                                                                                                                                                   
			{ 
				if( 1 == det[j].inuse && 0 == memcmp(det[j].mac, hw_addr->octet, TPI_MACLEN) && det[j].ip == inet_addr(clients_list[i].ip))
				{
					if(strlen(det[j].hostname) > 0)
					{
						memset(clients_list[i].hostname, 0, TPI_BUFLEN_64);
						strncpy(clients_list[i].hostname, det[j].hostname,TPI_BUFLEN_64-1);
					}
					clients_list[i].l2type =  det[j].flag;
					clients_list[i].time =  det[j].time;
					clients_list[i].interval_time =  det[j].interval_time;
					break;
					
				}
			}
		}	
	}
	
	return ;
}

static void getParentControlClientConfig(struct client_info *clients_list,int client_num)
{
	char *mib_val;
	int i = 0, j = 0, client_mib_count = 0;
	char hostname[TPI_BUFLEN_64] = {0};
	char mark[TPI_BUFLEN_64] = {0};
	char ip[TPI_IP_STRING_LEN] = {0};
	char mac[TPI_MAC_STRING_LEN] = {0};
	char limitenable[8] = {0};

	_SAFE_GET_VALUE(PARENT_CONTROL_CLIENT_NUM, mib_val);
	client_mib_count = atoi(mib_val);

	for(i=0;i<client_mib_count;i++)
	{
		_SAFE_GET_VALUE(PARENT_CONTROL_CLIENT_LIST(i+1), mib_val);//hostname \t ip \t limitEn
		if(strcmp(mib_val,"") == 0)
			continue;
		memset(hostname, 0x0, sizeof(hostname));
		memset(mark, 0x0, sizeof(mark));
		memset(ip, 0x0, sizeof(ip));
		memset(mac, 0x0, sizeof(mac));
		memset(limitenable, 0x0, sizeof(limitenable));
		if(parsePerParentControlClientConfig(mib_val, hostname, mark ,mac, ip, limitenable) ==-1)
			continue;
		
		for (j = 0; j < client_num; j++)
		{
			if(1 == clients_list[j].in_use &&  !strcmp(ip, clients_list[j].ip) &&  !strncasecmp(mac, clients_list[j].mac, strlen(mac)))
			{	
				if(strcmp(limitenable, "true") == 0)
					clients_list[j].limitenable =  1;
				break;
			}
		}	
	}
	return ;
}

int get_all_client_info( struct client_info * clients_list,  int max_client_num)
{
	int i;
	int client_num = 0;
	struct client_info * tmp;

	//init
	tmp = clients_list;
	for(i = 0; i< max_client_num; i++)
	{
		tmp->in_use = 0;
		tmp++;
	}

	client_num = get_arp_clients(clients_list,max_client_num);
	updateParentControlClientInfo(clients_list, client_num);
	getParentControlClientConfig(clients_list, client_num);
	
//	dump_all_client(clients_list,client_num);
	return client_num;

}
#define MAX_CLIENT_NUMBER 		255
/*
urlList = http://www.baidu.com \n  http://www.baidu.com
*/
static int getParentControlUrlFilterList2web(cJSON *array)
{
	char *value;
	if( array ==NULL)
		return 0;
	_SAFE_GET_VALUE(PARENT_CONTROL_URL_FILTER_LIST, value);

	if(strcmp(value, "") == 0)
		return 0;

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf("getParentControlUrlFilterList2web: malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	int num = 0;
	char *sp,*ep;

	num++;
	sp = list;
	while((ep = strchr(sp, '\n'))) {
		*ep++ = '\0';
		cJSON_AddStringToObject(array, "Fuck", sp);
		sp = ep;
		num++;	
	}
	
	/* 只有一条或最后一条*/
	cJSON_AddStringToObject(array, "Fuck", sp);

	free(list);
	list = NULL;
	return num;
}


/*
取值接口：/goform/getParentControl
取值范例：
{
	"onlineList": [{
		"onlineListHostname": "liangia" 			//主机名
		"onlineListIP": "192.168.0.1"				//IP地址
		"onlineListConnectTime": 12345			//连接时间
		"onlineListLimitEn": true | false 				//是否限制上网
	},
	{
		"onlineListHostname": "liangia" 			//主机名
		"onlineListIP": "192.168.0.1"				//IP地址
		"onlineListConnectTime": 12345			//连接时间
		"onlineListLimitEn": true | false 				//是否限制上网
	}],
	"parentCtrl": {
		"parentCtrlOnlineTime: 19:00-11:30, 			//在线时间
		"parentCtrlOnlineDate: "10011010" 				//第一个为每天，第二个为星期一,
		"parentCtrlURLFilterMode: disable | permit | forbid  //关闭 允许 禁止
	},
	urlFilter: {
		urlFilterEn: true|false,
		urlFilterList:[http://www.baidu.com,"http://www.163.com"]
	}
}
*/
static void fromGetParentControl(webs_t wp, char_t *path, char_t *query)
{
	int i = 0, client_num = 0;
	time_t now_time = time(0);
	struct client_info * clients_list ;
	char *out = NULL;
	cJSON *obj = NULL;
	cJSON *root = NULL;
	cJSON *array = NULL;
	
	clients_list = (struct client_info *)malloc(MAX_CLIENT_NUMBER * sizeof(arp_client_info));
	if(clients_list == NULL )
	{
		return ;
	}
	memset(clients_list, 0x0 , sizeof(arp_client_info) * MAX_CLIENT_NUMBER);
	
	client_num = get_all_client_info(clients_list , MAX_CLIENT_NUMBER);

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "onlineList", array = cJSON_CreateArray());
	for(i=0; i< client_num; i++)
	{	
		if(is_backlist(clients_list[i].mac) != 0 )
			continue;
		if(clients_list[i].time == 0)	//tenda_arp 有问题
			continue;
		cJSON_AddItemToArray(array, obj = cJSON_CreateObject());
		if(strlen(clients_list[i].hostname) >0)
			cJSON_AddStringToObject(obj, "onlineListHostname", clients_list[i].hostname);
		else
			cJSON_AddStringToObject(obj, "onlineListHostname", "Unknown");
		cJSON_AddStringToObject(obj, "onlineListRemark", clients_list[i].mark);
		cJSON_AddStringToObject(obj, "onlineListMAC", clients_list[i].mac);		
		cJSON_AddStringToObject(obj, "onlineListIP", clients_list[i].ip);	
		cJSON_AddNumberToObject(obj, "onlineListConnectTime", now_time - clients_list[i].time);
		if(clients_list[i].limitenable == 1)
			cJSON_AddStringToObject(obj, "onlineListLimitEn", "true");
		else
			cJSON_AddStringToObject(obj, "onlineListLimitEn", "false");	
	}

	cJSON_AddItemToObject(root, "parentCtrl", obj = cJSON_CreateObject());
	cJSON_AddStringToObject(obj, "parentCtrlOnlineTime", nvram_safe_get(PARENT_CONTROL_TIME));
	cJSON_AddStringToObject(obj, "parentCtrlOnlineDate", nvram_safe_get(PARENT_CONTROL_DATE));
	cJSON_AddStringToObject(obj, "parentCtrlURLFilterMode", nvram_safe_get(PARENT_CONTROL_URL_FILTER_MODE));

	cJSON_AddItemToObject(root, "urlFilter", obj = cJSON_CreateObject());
	cJSON_AddItemToObject(obj, "urlFilterList", array = cJSON_CreateArray());
	
	getParentControlUrlFilterList2web(array);
	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWriteLongString(wp, out);
	//printf("out:%s\n",out);
	free(out);
	websDone(wp, 200);
	
	free(clients_list);
	clients_list = NULL;
	return ;
}

void firewall_asp_define(void)
{
	websAspDefine(T("getfirewall"), getfirewall);
	websAspDefine(T("mAclGetIP"), aspmAclGetIP);
	websAspDefine(T("mAclGetUrl"), aspmAclGetUrl);
	websAspDefine(T("mAclGetMacFilter"), aspmAclGetMacFilter);
	
	websFormDefine(T("SafeClientFilter"), fromSafeClientFilter);
	websFormDefine(T("SafeUrlFilter"), fromSafeUrlFilter);
	websFormDefine(T("SafeMacFilter"), fromSafeMacFilter);
	websFormDefine(T("SafeWanPing"), fromSafeWanPing);
	websFormDefine(T("SafeWanWebMan"), fromSafeWanWebMan);
	websFormDefine(T("SafeNetAttack"), fromSafeNetAttack);

	/*归一化家长控制设置*/
	websFormDefine(T("setParentControl"), fromSetParentControl);
	websFormDefine(T("getParentControl"), fromGetParentControl);
}

static int getfirewall(int eid, webs_t wp, int argc, char_t **argv)
{
	int retv = 0;
	char *type, *item;
	char_t * v = NULL;
	if (ejArgs(argc, argv, T("%s %s"), &type, &item) < 2) 
	{
		return websWrite(wp, T("Insufficient args\n"));
	}
	if(strcmp(type,"lan") == 0)
	{
		if(strcmp(item,"lanip") == 0)
		{	
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_LAN0_IP, v));
		}
		else if(strcmp(item, "acl_en") == 0)
		{	
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_CLN_EN));
		}
		else if(strcmp(item, "curNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_CLN_CUR_NU, v));
		}
		else if(strcmp(item, "url_en") == 0)
		{
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_URL_EN));
		}
		else if(strcmp(item, "urlNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_URL_CUR_NU, v));
		}
		else if(strcmp(item, "acl_mac") == 0)
		{
			return websWrite(wp, T("%d"),get_filter_mode(_FW_FLT_MAC_EN));
		}
		else if(strcmp(item, "acl_macNum") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_FLT_MAC_CUR_NU, v));
		}
	}
	else if(strcmp(type, "wan") == 0)
	{
		if(strcmp(item, "dos") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_HACKER_ATT, v));
		}
		else if(strcmp(item, "wanweben") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_EN, v));
		}
		else if(strcmp(item, "webport") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_PORT, v));
		}
		else if(strcmp(item, "rmanip") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_SYS_RM_IP, v));
		}
		else if(strcmp(item,"ping") == 0)
		{
			return websWrite(wp, T("%s"),_SAFE_GET_VALUE(_FW_PING_DIS_WAN, v));
		}
	}

	return retv;
}


static int aspmAclGetIP(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_CLIENT(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static int aspmAclGetUrl(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_URL(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static int aspmAclGetMacFilter(int eid, webs_t wp, int argc, char_t **argv)
{
	unsigned int item = 0;
	const char_t *v;
	if(ejArgs(argc, argv, T("%d "),&item) < 1)
	{
		return 0;
	}

	item = item - 1;
	if(item>=0 && item <=9){
		_SAFE_GET_VALUE(_FW_FILTER_MAC(item),v);
		websWrite(wp,T("%s"), v);
	}
	else 
		websWrite(wp,T("%s"), "");
	return 0;
}

static void fromSafeNetAttack(webs_t wp, char_t *path, char_t *query)
{
	char_t *go,*DoSProtect;
	char_t *oldDos;
	go = websGetVar(wp,T("GO"),T(""));
	DoSProtect = websGetVar(wp,T("DoSProtect"),T("0"));
	_SAFE_GET_VALUE(_FW_HACKER_ATT,oldDos);
	
	if(strncmp(oldDos, DoSProtect,1) != 0)
	{
		_SET_VALUE(_FW_HACKER_ATT,DoSProtect);
		_COMMIT();

		sys_restart();

		websRedirect(wp, T("/firewall_disablewan.asp"));
	}
	else
	{
		websRedirect(wp, T("/firewall_disablewan.asp"));
	}
}

static void fromSafeClientFilter(webs_t wp, char_t *path, char_t *query)
{
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;
	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_CLN_EN,aclen);
	if(strcmp(aclen,"disable") == 0)
		goto ip_done;
	
	_SET_VALUE(_FW_FLT_CLN_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_CLIENT(which), stritem);	
	
ip_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_clientfilter.asp"));
}

//filter_url0=192.168.1.7-192.168.1.8:www.baidu.com,on
static void fromSafeUrlFilter(webs_t wp, char_t *path, char_t *query)
{	
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;
	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_URL_EN,aclen);
	if(strcmp(aclen,"disable") == 0)
		goto url_done;
	
	_SET_VALUE(_FW_FLT_URL_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_URL(which), stritem);	

url_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_urlfilter.asp"));

}

static void fromSafeMacFilter(webs_t wp, char_t *path, char_t *query)
{
	char name[VALUEMAXLEN] ={'\0'};
	int which = 0;
	char_t  *aclen,*stritem,*go,*num;//,*saveTag;

	go = websGetVar(wp, T("GO"), T("")); 
	aclen = websGetVar(wp,T("check"),T("disable"));
	num = websGetVar(wp,T("curNum"),T("1"));
	//saveTag = websGetVar(wp,T("saveTag"),T("1"));
	
	_SET_VALUE(_FW_FLT_MAC_EN,aclen);//禁止deny
	if(strcmp(aclen,"disable") == 0)
		goto mac_done;
	
	_SET_VALUE(_FW_FLT_MAC_CUR_NU,num);
	
	which = atoi(num);
	sprintf(name,"CL%d",which);
	stritem = websGetVar(wp,T(name),T(""));
	which = which -1;
	
	if( which>=0 && which<= 9)
			_SET_VALUE(_FW_FILTER_MAC(which), stritem);	
	
	//00:11:22:33:44:55,0-6星期,0-0时间,on是否开启,1诠释
	
	//00:11:22:33:44:55,1-2,11100-29400,on,1
	
mac_done:	
	_COMMIT();

	sys_restart();

	websRedirect(wp, T("/firewall_mac.asp"));

}

static void fromSafeWanPing(webs_t wp, char_t *path, char_t *query)
{
	char_t  *pingwan;//, *go;
	char_t *oldPing;
	pingwan = websGetVar(wp, T("PingWan"), T("0")); 

	_SAFE_GET_VALUE(_FW_PING_DIS_WAN, oldPing);
		
	if(strcmp(oldPing, pingwan) != 0)
	{		
		_SET_VALUE(_FW_PING_DIS_WAN, pingwan);
		_COMMIT();

		sys_restart();
		
		websRedirect(wp, T("/firewall_wanping.asp"));
	}
	else
	{
		websRedirect(wp, T("/firewall_wanping.asp"));
	}
}


///******************************************************************************
//*/远端WEB管理

static void	fromSafeWanWebMan(webs_t wp, char_t *path, char_t *query)
{
	char_t	*wanwebip, *webport, *en, *go;
	//int oldEn;

	wanwebip = websGetVar(wp, T("IP"), T("")); 
	go = websGetVar(wp, T("GO"), T("")); 
	en = websGetVar(wp, T("RMEN"), T("0")); 
	webport = websGetVar(wp, T("port"), T("")); 

	if(atoi(en) == 1)
	{
		_SET_VALUE(_SYS_RM_EN, en);
		_SET_VALUE(_SYS_RM_PORT,webport);
		_SET_VALUE(_SYS_RM_IP,wanwebip);
	}else{
		_SET_VALUE(_SYS_RM_EN, en);
	}

	_COMMIT();
	
	LoadWanListen(atoi(en));

	websRedirect(wp, T("/system_remote.asp"));

}

void LoadWanListen(int load)
{
	char WanIP[24] = {0};
	int port;
	char_t *WanPort,*pHost;

	LoadManagelist();

	if(load){

		webs_Wan_CloseListen();

		_SAFE_GET_VALUE(_SYS_RM_PORT,WanPort);
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
	
	_SAFE_GET_VALUE(_SYS_RM_EN, enable);
	_SAFE_GET_VALUE(_SYS_RM_PORT, remote_port);
	_SAFE_GET_VALUE(_LAN0_IP, lan_ip);
	_SAFE_GET_VALUE(_LAN0_NETMASK, lan_msk);
	
	strcpy(bufTmp,lan_ip);
	inet_aton(bufTmp,&g_localip);

	strcpy(bufTmp,lan_msk);
	inet_aton(bufTmp,&g_localmask);
	g_remote_port = atoi(remote_port);
	
	if(atoi(enable) == 1)
	{//remote
		g_enable_remote_acl = 1;
		
		_SAFE_GET_VALUE(_SYS_RM_IP,rm_ip);
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

