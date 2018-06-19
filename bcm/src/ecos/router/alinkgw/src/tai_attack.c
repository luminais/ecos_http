#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/param.h>
#include <ctype.h>
#include <bcmnvram.h>
#include "flash_cgi.h"
#include "route_cfg.h"
#include "../include/alinkgw_api.h"

struct attack_info{
	char mac[18];
};
#define MAX_ATTACK_NUM		5
#define MACADDR_STR_LEN		17
struct attack_info gAttackInfo[MAX_ATTACK_NUM];

int gAttack_switch = 1;//Ä¬ÈÏ¿ªÆô
unsigned int access_attack_num = 0; 
#define ALILINK_ATTACK_SWITCH			"alilink_attack_switch"
#define	ALI_LOCK()	cyg_scheduler_lock()
#define	ALI_UNLOCK()	cyg_scheduler_unlock()

 extern int ali_get_connection_status();
int set_attack_switch_state(const char *json)
{
	printf("%s JSON:%s\n",__func__, json);
	if(json == NULL)
		return ALINKGW_ERR;
	
	if(strlen(json) != 1)
		return ALINKGW_ERR;
	
	if(strcmp(json, "1") == 0)
		gAttack_switch = 1;
	else if(strcmp(json, "0") == 0)
		gAttack_switch = 0;
	else
		return ALINKGW_ERR;

	_SET_VALUE(ALILINK_ATTACK_SWITCH,  json);
	_COMMIT();
	return ALINKGW_OK;
}

int get_attack_switch_state(char *buf, unsigned int buff_len)
{
	printf("%s %d buf:%s buff_len:%u\n",__func__,__LINE__,buf, buff_len);
	if(buf == NULL)
		return ALINKGW_ERR;
	
	if(buff_len < 2)
		return ALINKGW_BUFFER_INSUFFICENT;
	
	sprintf(buf  , "%d" , gAttack_switch);

	printf("return buf:%s \n",buf);
	return ALINKGW_OK;
}

int access_attack_add_list(const char *mac)
{
	int i = 0;
	if(mac == NULL)
		return 0;
	if(strlen(mac) != MACADDR_STR_LEN)
		return 0;

	for( i = 0 ; i < MAX_ATTACK_NUM; i++)
	{	
		if(strcmp(gAttackInfo[i].mac, mac) == 0)
		{
			return 1;
		}
	}
	for( i = 0 ; i < MAX_ATTACK_NUM; i++)
	{	
		if(strlen(gAttackInfo[i].mac) == 0)
		{
			strncpy(gAttackInfo[i].mac, mac, MACADDR_STR_LEN);
			return 1;
		}
	}
	return 0;
}

int access_attack_report(const char *mac)
{
	printf("%s mac:%s gAttack_switch:%d\n",__func__, mac, gAttack_switch);
	char buff[32] = {0};
	int i;
	if(0 == ali_get_connection_status())
		return 0;
	if(gAttack_switch == 0)
		return 0;
	if( mac == NULL)
		return 0;
	if(strlen(mac) != strlen("00:11:22:33:44:55"))
		return 0;
#if 0
	unsigned           tid;
	tid =  cyg_thread_get_id(cyg_thread_self());
	printf ("#####%s %d   tid=%d#####\n", __FUNCTION__, __LINE__, tid);	
#endif
	access_attack_num++;
	printf("access_attack_num:%u\n",access_attack_num);
	memset(buff, 0, sizeof(buff));
	sprintf(buff, "%u", access_attack_num);
	ALINKGW_report_attr_direct(ALINKGW_ATTR_ACCESS_ATTACK_NUM, ALINKGW_ATTRIBUTE_simple, buff);
	
	memset(buff, 0, sizeof(buff));
	for(i = 0; i < strlen(mac); i++)
       	buff[i] = toupper(mac[i]);
//	strncpy(buff, mac, strlen(mac));
	ALINKGW_report_attr_direct(ALINKGW_ATTR_ACCESS_ATTACKR_INFO, ALINKGW_ATTRIBUTE_simple, buff);

	return 0;
}

int access_attack_report_all()
{
	int i = 0;
	for( i = 0 ; i < MAX_ATTACK_NUM; i++)
	{		
		if(strlen(gAttackInfo[i].mac) == MACADDR_STR_LEN)
		{
			access_attack_report(gAttackInfo[i].mac);
			memset(gAttackInfo[i].mac, 0x0 , sizeof(gAttackInfo[i].mac));
		}
	}
//	memset(gAttackInfo, 0x0 , sizeof(struct attack_info) * MAX_ATTACK_NUM);

	return 0;
	
}

