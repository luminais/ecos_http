#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>
#include <string.h>
#include <time.h>

#include <bcmnvram.h>
#include "flash_cgi.h"
#include "alinkgw_api.h"

#define TPSK_SIZE	128
#define TPSK_LIST_SIZE	128

#define ALILINK_WLAN_TPSKLIST_MAX	128
#define TPSK_LEN        64
#define STR_EMPTY_MAC   "00:00:00:00:00:00"
#define STR_MAC_LEN     17
#define STR_MAC_SIZE     18
#define RETURN_OK     	1
#define RETURN_ERR    	0
                                         
#define TPSK_NULL_VAR	{"[]"};
#define TPSK_STR_VAR  "{\"tpsk\":\"%s\",\"mac\":\"%s\",\"duration\":\"%d\"}"
#define HEX_FORM_16		"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x"

#define WIFI_INTER_NAME  					"eth1"
//Usage: wl add_ie <pktflag> length OUI hexdata . 由阿里提供
#define  NETW_ROUTER_ARGC					5
#define  NETW_ROUTER_IE_FLAG 				"3"
#define  NETW_ROUTER_IE_LEN 				"22"
#define  NETW_ROUTER_IE_OUI  			 	"D8:96:E0"
#define  NETW_ROUTER_IE_HEXDATA  			"0101XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX00"
#define  NETW_ROUTER_TMP_TPSK_TIMEOUT	30
#define  NETW_ROUTER_IE_TYPE 				1
#define  NETW_ROUTER_IE_VERSION 			1
#define  NETW_ROUTER_IE_RESERVE 			0

#define TPSK_RANDOM_COUNT					16 
char tpsk_random_str[TPSK_RANDOM_COUNT+1] ;

//extern int wl_vndr_ie(void *wl, const char *command, char **argv);
extern int wl_set_vendor_ie(void *wl, const char *cmd, char **argv);
extern int ali_get_connection_status();
typedef struct tpsk_list
{
    char tpsk[TPSK_LEN];
    char mac[STR_MAC_SIZE];
    int dur;    /* TPSK duration */  //貌似没啥作用
    int index;
    struct tpsk_list *next;
}Alilink_tpsk_list, *pAlilink_tpsk_list;

int gTpskDebug = 0;
int alilink_tpsk_list_count = 0;
Alilink_tpsk_list *alilink_tpsk_list; //tpsklist，可能有多个，全0的为临时tpsk
char alilink_tmp_tpsk[64] = {0};  //临时tpsk，只有一个
int g_needReportTpskList = 0;
int report_alilink_tpsk_state();

void tpsk_debug_level(int level)
{
    if (level > 0)
        gTpskDebug = 1;
    else
        gTpskDebug = 0;

    printf("set tpsk debug level = %d\n", gTpskDebug);
}

int add_tpsk_list(Alilink_tpsk_list *node, int save)
{
	
	Alilink_tpsk_list *ptr;
	char count_str[8] = {0};
	char name_str[64] = {0};
	char list_str[128] = {0};
       
	if(NULL == node)
		return RETURN_ERR;

        if (alilink_tpsk_list_count >= ALILINK_WLAN_TPSKLIST_MAX) //add by z10312 
        {
            printf ("add_tpsk_list count  will over  ALILINK_WLAN_TPSKLIST_MAX( 10 ) !!!\n");
            free(node);
            node = NULL;
            return RETURN_ERR;     
        }
        
	for(ptr = alilink_tpsk_list;ptr != NULL; ptr = ptr->next)
	{
		if(strcmp(ptr->mac,node->mac) == 0)
		{//find it,update
			if(gTpskDebug)
				printf("#######add_tpsk_list, find sta mac[%s]  index:%d ######\n",ptr->mac,ptr->index);
			strncpy(ptr->tpsk, node->tpsk, TPSK_LEN);
			ptr->dur = node->dur;
			free(node);
			node = NULL;
			sprintf(list_str, "%s %s %d", ptr->tpsk, ptr->mac, ptr->dur);
			sprintf(name_str, "%s%d",ALILINK_WLAN_TPSK_LIST, ptr->index);
			_SET_VALUE(name_str, list_str);
             
			_COMMIT();
			report_alilink_tpsk_state();
			return RETURN_OK;	
		}
	}
	//add to list
	alilink_tpsk_list_count ++;
	node->index = alilink_tpsk_list_count;
	node->next = alilink_tpsk_list;
	alilink_tpsk_list = node;
	
	if(gTpskDebug)
		printf("alilink_tpsk_list_count:%d \n",alilink_tpsk_list_count);
	
	if(save)
	{
		sprintf(count_str, "%d", alilink_tpsk_list_count);
		_SET_VALUE(ALILINK_WLAN_TPSK_LIST_NUM, count_str);
             
		sprintf(list_str, "%s %s %d", node->tpsk, node->mac, node->dur);
		sprintf(name_str, "%s%d",ALILINK_WLAN_TPSK_LIST, alilink_tpsk_list_count);
		_SET_VALUE(name_str, list_str);
             
		_COMMIT();
		report_alilink_tpsk_state();
	}     
	return RETURN_OK;
}
        
int create_and_add_to_list(char *tpsk, char *mac, int dur, int save)
{
	if(NULL == tpsk || NULL == mac)
		return RETURN_ERR;
	if(strlen(mac) != STR_MAC_LEN)
	{
		if(gTpskDebug)
			printf("mac address error,mac:%s len:%d\n",mac, strlen(mac));
		return RETURN_ERR;
	}
	Alilink_tpsk_list *node;
	node = (pAlilink_tpsk_list)malloc(sizeof(Alilink_tpsk_list));
	if(NULL == node)
	{
		printf("malloc error .\n");
		return RETURN_ERR;
	}
        
	memset(node, 0, sizeof(Alilink_tpsk_list));
	strncpy(node->tpsk, tpsk, TPSK_LEN);
	strncpy(node->mac, mac, STR_MAC_SIZE);
	node->dur = dur;
	
	return add_tpsk_list(node, save);
        
}

int load_tpsk_config()
{
        
	char mib_name[128];
	char *mib_val;
	int i = 0, num = 0;
	char tpsk[64]={0};
	char mac[18]={0};
	int dur = 0;
	    
	    
	alilink_tpsk_list_count = 0;
	_GET_VALUE(ALILINK_WLAN_TPSK_LIST_NUM, mib_val);
	if( atoi(mib_val) > ALILINK_WLAN_TPSKLIST_MAX || atoi(mib_val) == 0)
	{
		printf("tpsklist count[%d] is error.\n",atoi(mib_val));
		return RETURN_OK;	
	}
	num = atoi(mib_val);
	
	for(i = 1; i <= num; i++)
	{
		sprintf(mib_name, "%s%d", ALILINK_WLAN_TPSK_LIST, i);
		_GET_VALUE(mib_name, mib_val);
		sscanf(mib_val, "%s %s %d", tpsk, mac, &dur);
		if(strlen(mac) == STR_MAC_LEN)
			create_and_add_to_list(tpsk, mac, dur, 0);  //dur 先固定写为0，阿里需求
		else
		{
			if(gTpskDebug)
				printf("got error value:%s\n",mib_val);
		}
	}
        		
	return RETURN_OK;
}

int get_tpsk(char *buff, unsigned int buff_len)
{
    

    if(NULL == buff)
        return ALINKGW_ERR;
    
    if(buff_len < strlen(alilink_tmp_tpsk) + 1)
        return ALINKGW_BUFFER_INSUFFICENT;
    
    strncpy(buff, alilink_tmp_tpsk, buff_len);
    if(gTpskDebug)
	printf("%s buf=%s len=%d\n",__func__,buff,strlen(buff));
    return ALINKGW_OK;
}

void clean_tmp_tpsk(void)
{
	if(strlen(tpsk_random_str) == 0)
	{
		if(gTpskDebug)
			printf("Once before tmp tpsk already del... , return .\n");
		return;
	}
    
	char * add_ie_argv[NETW_ROUTER_ARGC];
	add_ie_argv[0] = "";
	add_ie_argv[1] = NETW_ROUTER_IE_FLAG;
	add_ie_argv[2] = NETW_ROUTER_IE_LEN;
	add_ie_argv[3] = NETW_ROUTER_IE_OUI;
	add_ie_argv[4] = NETW_ROUTER_IE_HEXDATA;
	sprintf(add_ie_argv[4], "%02x%02x"HEX_FORM_16"%02x", NETW_ROUTER_IE_TYPE, NETW_ROUTER_IE_VERSION,
		tpsk_random_str[0], tpsk_random_str[1], tpsk_random_str[2], tpsk_random_str[3], tpsk_random_str[4], 
		tpsk_random_str[5], tpsk_random_str[6], tpsk_random_str[7], tpsk_random_str[8], tpsk_random_str[9], 
		tpsk_random_str[10], tpsk_random_str[11], tpsk_random_str[12], tpsk_random_str[13], tpsk_random_str[14], 
		tpsk_random_str[15], NETW_ROUTER_IE_RESERVE);

	memset(tpsk_random_str, 0x0, sizeof(tpsk_random_str));
	memset(alilink_tmp_tpsk,0x0 ,sizeof(alilink_tmp_tpsk));

	printf("go to del ie....\n");
	
	if(gTpskDebug){
		printf("del add_ie_argv[4] len:%d  %s \n",strlen(add_ie_argv[4]),add_ie_argv[4]);
	}
	// 关闭智能设备厂商IE
	int ret = wl_set_vendor_ie(WIFI_INTER_NAME, "del", add_ie_argv);
	printf("del ie ,return ret :%d \n",ret);
    
}

/*16位随机字串生成函数 */
void set_tpsk_random_string()
{
	const char *random_string = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int n;

	srand((int)time(0));

	for(n=0; n<TPSK_RANDOM_COUNT; n++)
	{
		tpsk_random_str[n] = random_string[rand()%strlen(random_string)];
	}
	if(gTpskDebug)
		printf("random len:%d %s\n",strlen(tpsk_random_str),tpsk_random_str);
	return;
}

int set_tpsk(const char *json_in)
{
    
	char * add_ie_argv[NETW_ROUTER_ARGC];
	add_ie_argv[0] = "";
	add_ie_argv[1] = NETW_ROUTER_IE_FLAG;
	add_ie_argv[2] = NETW_ROUTER_IE_LEN;
	add_ie_argv[3] = NETW_ROUTER_IE_OUI;
	add_ie_argv[4] = NETW_ROUTER_IE_HEXDATA;

	if(NULL == json_in)
	    return ALINKGW_ERR;
	    
	if(strlen(json_in) + 1 > sizeof(alilink_tmp_tpsk) -TPSK_RANDOM_COUNT)
	    return ALINKGW_ERR;
	
	untimeout((timeout_fun *)clean_tmp_tpsk,NULL);
	clean_tmp_tpsk(); //先清除上一次未完成的配网
	
	memset(tpsk_random_str, 0x0, sizeof(tpsk_random_str));
	if(gTpskDebug)
		printf("***************************************\n");
	set_tpsk_random_string();
	strncpy(alilink_tmp_tpsk, json_in, sizeof(alilink_tmp_tpsk));
	if(gTpskDebug)
		printf("tpsk len:%d %s\n",strlen(alilink_tmp_tpsk), alilink_tmp_tpsk);
    	strcat(alilink_tmp_tpsk, tpsk_random_str);
	if(gTpskDebug)
	{
		printf("new tpsk len:%d %s\n",strlen(alilink_tmp_tpsk), alilink_tmp_tpsk);
   		printf("***************************************\n");
	}
	sprintf(add_ie_argv[4], "%02x%02x"HEX_FORM_16"%02x", NETW_ROUTER_IE_TYPE, NETW_ROUTER_IE_VERSION,
		tpsk_random_str[0], tpsk_random_str[1], tpsk_random_str[2], tpsk_random_str[3], tpsk_random_str[4], 
		tpsk_random_str[5], tpsk_random_str[6], tpsk_random_str[7], tpsk_random_str[8], tpsk_random_str[9], 
		tpsk_random_str[10], tpsk_random_str[11], tpsk_random_str[12], tpsk_random_str[13], tpsk_random_str[14], 
		tpsk_random_str[15], NETW_ROUTER_IE_RESERVE);

	if(gTpskDebug){
		printf("add_ie_argv[4] len:%d  %s \n",strlen(add_ie_argv[4]),add_ie_argv[4]);
		printf("add ie, timeout is %d\n",NETW_ROUTER_TMP_TPSK_TIMEOUT);
	}
	//扩展物联路由厂商IE, 使其智联设备可以识别物理路由

	wl_set_vendor_ie(WIFI_INTER_NAME, "add", add_ie_argv);
	
	timeout((timeout_fun *)clean_tmp_tpsk, NULL, NETW_ROUTER_TMP_TPSK_TIMEOUT*100); 

	return ALINKGW_OK;
}

int get_tpsk_list(char *buff, unsigned int buff_len)
{
	int buff_size = 0;
	int n = 0;
	int flag = 0;
	char *buff_tmp;
	Alilink_tpsk_list *ptr;
	
	if(NULL == buff)
		return ALINKGW_ERR;

	buff_size = ALILINK_WLAN_TPSKLIST_MAX*alilink_tpsk_list_count;
	buff_tmp = (char *)malloc(buff_size);
	if(buff_tmp == NULL){
		printf("malloc failed\n");
		return ALINKGW_ERR;
	}
	memset(buff_tmp,0,buff_size);
	n = sprintf(buff_tmp, "[");
	
	for(ptr = alilink_tpsk_list; ptr != NULL; ptr = ptr->next)
	{
		if(flag == 0)
		{
			n += sprintf(buff_tmp+n, TPSK_STR_VAR, ptr->tpsk, ptr->mac, ptr->dur);
			flag = 1;
		}
		else
			n += sprintf(buff_tmp+n, ","TPSK_STR_VAR, ptr->tpsk, ptr->mac, ptr->dur);
		
	}
	n += sprintf(buff_tmp+n, " ]");
	buff_tmp[n] = '\0';
        
	if(buff_len < strlen(buff_tmp) + 1)
	{
		free(buff_tmp);
   		buff_tmp = NULL;
        	return ALINKGW_BUFFER_INSUFFICENT;
	}
    strncpy(buff, buff_tmp, strlen(buff_tmp));
	
   free(buff_tmp);
   buff_tmp = NULL;
    return ALINKGW_OK;
}

int set_tpsk_list(const char *json_in)
{//空接口，后续用到再实现@20150601
	if(NULL == json_in)
		return ALINKGW_ERR;

	return ALINKGW_OK;
}

int report_alilink_tpsk_state()
{
	if(0 == ali_get_connection_status())
		return ALINKGW_OK;
	
	int ret = ALINKGW_report_attr(ALINKGW_ATTR_TPSK_LIST);
        
   	if(ret != ALINKGW_OK)
   	{
   	   if(gTpskDebug)
	 	  printf("[sample] [%s], report attibute(%s) failed\n", __func__, ALINKGW_ATTR_TPSK_LIST);
	   return ret;
   	}
  	return ALINKGW_OK;
}

int check_alilink_tpsk_state()
{
	if(g_needReportTpskList)
	{
		g_needReportTpskList = 0;
		return 1;
	}
	return 0;
}

