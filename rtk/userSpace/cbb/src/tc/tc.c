/*
 * Stream Control ecos main entrance.
 *
 * Copyright (C) 2010, Tenda Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Tenda Corporation.
 *	
 * $Id: tc.c,v 1.0 2010/08/20   Exp $
 * $author: stanley 
 * $description: This is support stream control base on  TCP protocol window size,
 *			  but the way have bug to UDP protocol. intelligent stream control base on ternary ( ip client,)
 */
#include <stdio.h>
#include <tc.h>
#include <bcmnvram.h>
#include <stdlib.h>
#include <flash_cgi.h>

#define DEFAULT_WIN  208

#ifdef	__CONFIG_ALINKGW__

struct al_qos
{
	unsigned int expect_up ;
	unsigned int expect_down ;
	unsigned int real_down ;
	unsigned int real_up ;
};

extern struct al_qos al_qos_stream[TC_CLIENT_NUMBER] ;

extern int start_al_qos_flag  ;

#endif

typedef enum{
	PRI_UP_LIMIT = 0,
	PRI_DOWN_LIMIT,
	GUEST_UP_LIMIT,
	GUEST_DOWN_LIMIT,
	NO_LIMIT
}limit_type;

void update_total_stream_statistic(int direction,unsigned int ip_len);
void get_total_stream_statistic(u_long * tx,u_long * rx);

extern int str2arglist(char *buf, int *list, char c, int max);
static unsigned short chsum_fixup_short(unsigned short sum, unsigned short old_data, 
						unsigned short new_data);

unsigned int total_rate_runtime;
tc_ip_index_t ip_index[TC_CLIENT_NUMBER] ;
tc_ip_index_t guest_total_limit;
int g_user_rate_control=0;


void tc_reset_timeout(void){
    	untimeout((timeout_fun*)&tc_tmr_func, 0 );
	timeout((timeout_fun*)&tc_tmr_func, 0, CONFIG_TC_INTERVAL);
	return ;
}

void tc_set_timeout(void){
	timeout((timeout_fun*)&tc_tmr_func, 0, CONFIG_TC_INTERVAL);
	return;
}

void tc_untimeout(void){
	untimeout((timeout_fun*)&tc_tmr_func, 0 );
	return;
}


void tc_tmr_func(void )
{
  	 int i=0;
	 int date = 0;
	//diag_printf("%s__________%d\n", __FUNCTION__, __LINE__);
	for( i=0; i < TC_CLIENT_NUMBER; ++i){

#ifdef __CONFIG_ALINKGW__
		al_qos_stream[i].real_down = 0 ;
		al_qos_stream[i].real_up = 0 ;
#endif

		if(ip_index[i].rule == NULL)
			continue;
		//diag_printf("ip:%d total packets len are clear\n", i);
#ifdef __CONFIG_TC_WINDOWS__	
			ip_index[i].win_up = 0;	
			ip_index[i].win_down = 0;
			ip_index[i].pre_d_up_len = ip_index[i].d_up_len;
			ip_index[i].pre_d_down_len = ip_index[i].d_down_len;
			ip_index[i].d_up_len=0;
			ip_index[i].d_down_len=0;
#endif/*__CONFIG_TC_WINDOWS__*/

			ip_index[i].up_len = 0;	
			date = ip_index[i].down_len -20*1024;
			
			if(date > (int) ip_index[i].expe_down )
			{
				if(ip_index[i].down_limit_len > 0)
				{
					ip_index[i].down_limit_len -= (ip_index[i].expe_down/4);
				}
				ip_index[i].down_len = 0;
			}
			else
			{
				if(ip_index[i].down_limit_len < ip_index[i].expe_down)
				{
					ip_index[i].down_limit_len += (ip_index[i].expe_down/4);
				}
				ip_index[i].down_len = 0;
			}
	}	
#ifdef __CONFIG_GUEST__
	if(guest_total_limit.rule)
		guest_total_limit.down_len = 0;
#endif
	return ;
}


#ifdef __CONFIG_TC_WINDOWS__
#define calculate_windows(the_win,rel_len,expe_len,cur_win,wmrd) 	 do{\
		if((rel_len)==0)\
			(rel_len)=1;\
		if((the_win)==0)\
			(the_win)=((cur_win)>>(wmrd));\
		else if(expe_len == 0)\
			(the_win)= 0;\
		else\
			(the_win)=(the_win)-(((the_win)*(rel_len))/(expe_len));\
		if(((the_win)<DEFAULT_WIN)&&((DEFAULT_WIN)<(cur_win)))\
			(the_win)=DEFAULT_WIN;\
		else	if(((the_win)<DEFAULT_WIN)&&((DEFAULT_WIN)>(cur_win)))\
			(the_win)=cur_win;\
		else if(the_win>cur_win)\
			(the_win)=cur_win;\
	}while(0)


#define calcuate_radix(expe_len,curwin,wmrd,drop)		do{\
	wmrd=(((curwin)>>14)+2+drop)-((expe_len)>>14);\
	wmrd=((wmrd)<(0))?(0):(wmrd);\
	}while(0)
	
#define calcuate_drop_radix(expe_len,drop_len,drop_rad)	 	do{\
	if((drop_len)>(expe_len))\
		drop_rad=(drop_len)>>11;\
	else\
		drop_rad=(drop_len)>>13;\
	}while(0)	


int down_win_stream_control(int nat_dir,ip_t *ip,int type)
{
	unsigned long index = 0;
	unsigned short ip_len = 0;
	unsigned short cur_win = 0;
	unsigned short the_win = 0;
	unsigned int drop_radix = 0;
	unsigned int hlen = 0;
	int wmrd =0;
	tcphdr_t *tcp = NULL;
	
	if((ip == NULL) || (nat_dir != NAT_INBOUND)){
		diag_printf("down_win_stream_control error!\n");
		return -1; //drop packets
	}
	hlen = ip->ip_hl << 2;
	ip_len =  ntohs(ip->ip_len);
	
	index =  (ntohl(ip->ip_dst.s_addr)&0xff) - 1;

	/*lq 防止内存泄漏*/
	if(index >= TC_CLIENT_NUMBER || index < 0)
		return 1;
#ifdef __CONFIG_GUEST__
	if(type == GUEST_DOWN_LIMIT)
	{	
		if((guest_total_limit.rule != NULL) && (guest_total_limit.down_flag != 0) ){
			if((guest_total_limit.down_len)  < (guest_total_limit.down_limit_len)){
				guest_total_limit.down_len += ip_len;
				return 1;

			}else{
				guest_total_limit.d_down_len += ip_len;
				return -1;
			}
		}
	}
#endif
	//diag_printf("index=%d\n",index);	
	if((ip_index[index].rule != NULL) && (ip_index[index].down_flag != 0) ){
		//if(ip_index[index].down_len < (ip_index[index].expe_down)){
		if((ip_index[index].down_len)  < (ip_index[index].down_limit_len)){
				ip_index[index].down_len += ip_len;

		}else{
			ip_index[index].d_down_len += ip_len;

			return -1;
		}
	}


		/*modify recv packets will means control up stream
		 *so you have to sure enable up stream control
		 */
	 if((ip_index[index].rule != NULL)&& (ip_index[index].up_flag != 0) && (ip->ip_p == IPPROTO_TCP)){
		tcp = (tcphdr_t *)(tcphdr_t *)((char *)ip + hlen);
		cur_win = ntohs(tcp->th_win);
		
		calcuate_drop_radix(ip_index[index].expe_down,
					ip_index[index].pre_d_down_len, drop_radix);
		calcuate_radix(ip_index[index].expe_up,cur_win,wmrd,drop_radix);
		calculate_windows(ip_index[index].win_up,ip_index[index].up_len,
		 				ip_index[index].expe_up,cur_win,wmrd);
		
		the_win=ip_index[index].win_up;

		if((cur_win!= the_win) && (cur_win<DEFAULT_WIN)){
			tcp->th_sum = chsum_fixup_short(tcp->th_sum, tcp->th_win, htons(the_win));
			tcp->th_win = htons(the_win);
		}	
	}
	return 1;
}
		
int up_win_stream_control(int nat_dir, ip_t *ip)
{
	unsigned long index = 0;
	unsigned short ip_len = 0;
	unsigned short cur_win = 0;
	unsigned short the_win = 0;
	unsigned int hlen = 0;
	unsigned int drop_radix = 0;
	int wmrd = 0; 
	tcphdr_t *tcp = NULL;
	
	if((ip == NULL) || (nat_dir != NAT_OUTBOUND)){
		diag_printf("up_win_stream_control error!\n");
		return -1; //drop packets
	}
	hlen = ip->ip_hl << 2;
	ip_len =  ntohs(ip->ip_len);
	index =  (ntohl(ip->ip_src.s_addr)&0xff) - 1;
	//根据coverity分析结果修改，原来为无效的判断:if(index >= TC_CLIENT_NUMBER || index < 0)  2017/1/11 F9项目修改
	if(index >= TC_CLIENT_NUMBER)
		return 1;
	//diag_printf("index=%d\n",index);
	if((ip_index[index].rule != NULL) && (ip_index[index].up_flag != 0) ){
		ip_index[index].up_len += ip_len;
		if(ip_index[index].up_len < (ip_index[index].expe_up)){
			;
#ifdef __CONFIG_TC_RANDOM_DROP__ 
			
#endif
		}else{
			ip_index[index].d_up_len += ip_len;

			return -1;
		}
	}
	
		/*modify send packets windows size will means control down stream
		 *so you have to sure enable down stream control
		 */	
	if((ip_index[index].rule != NULL) && (ip_index[index].down_flag != 0) && (ip->ip_p == IPPROTO_TCP)){
		tcp = (tcphdr_t *)(tcphdr_t *)((char *)ip + hlen);
		cur_win = ntohs(tcp->th_win);

		calcuate_drop_radix(ip_index[index].expe_up,
					ip_index[index].pre_d_up_len, drop_radix);
		calcuate_radix(ip_index[index].expe_down,cur_win,wmrd,drop_radix);
		calculate_windows(ip_index[index].win_down, ip_index[index].down_len,
		 				ip_index[index].expe_down, cur_win,wmrd);
		
		the_win=ip_index[index].win_down;
		
		if((cur_win!= the_win) && (cur_win<DEFAULT_WIN)){
			tcp->th_sum = chsum_fixup_short(tcp->th_sum, tcp->th_win, htons(the_win));
			tcp->th_win = htons(the_win);
		}	
	}
	
	
	return 1;
}

#endif /*__CONFIG_TC_WINDOWS__*/






void init_ip_index_array(void){
	int i;
	for(i=0; i< TC_CLIENT_NUMBER; ++i){
		ip_index[i].rule= NULL;	
	}
#ifdef __CONFIG_GUEST__
	guest_total_limit.rule = NULL;
#endif
	return;
}



/*****************************************************************************
 函 数 名  : tc_rule_fflush
 功能描述  : 清空规则列表
 输入参数  : void
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年9月13日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
void tc_rule_fflush( void )
{
	int i = 0;
	for(i = 0; i < TC_CLIENT_NUMBER; ++i)
	{
		if(ip_index[i].rule == NULL)
			continue;
		free(ip_index[i].rule);
		diag_printf("===========ip:%d free\n", i);
		ip_index[i].rule = NULL;
	}
#ifdef __CONFIG_GUEST__
	if(guest_total_limit.rule)
	{
		free(guest_total_limit.rule);
		guest_total_limit.rule = NULL;
	}
#endif
}

/*****************************************************************************
 函 数 名  : set_tc_enable_rule
 功能描述  : 初始化设置需要添加规则的IP
 输入参数  : int index
             int maxrate
             int minrate
             int direction
 输出参数  : 无
 返 回 值  : void

 修改历史      :
  1.日    期   : 2016年9月13日
    作    者   : liquan
    修改内容   : 新生成函数index

*****************************************************************************/
void set_tc_enable_rule( tc_ip_index_t *index, int maxrate, int minrate, int direction )
{
	if(index->rule == NULL)
	{
		if(index->rule==NULL){	
			index->rule=(tc_ip_rate_t*)malloc(sizeof(tc_ip_rate_t));	
			if(index->rule!=NULL){			
				bzero((char*)index->rule,sizeof(tc_ip_rate_t));	
			}else{		
				diag_printf("init_stream_control	error!\n");	
				return-1;	
			}	
		}
		index->d_down_len = 0;
		index->pre_d_down_len = 0;
		index->down_flag = 0;
		index->max_down = 0;
		index->min_down = 0;
		index->expe_down = 0;
		index->down_limit_len = 0;
		index->up_flag = 0;
		index->max_up = 0;
		index->min_up = 0;
		index->expe_up = 0;
		index->d_up_len = 0;
		index->pre_d_up_len = 0;
	}

	if( direction == 0)
	{
		index->up_flag = 1;
		index->max_up = maxrate * 1024;
		index->min_up = minrate * 1024;
		index->expe_up = maxrate * 1024;
		index->d_up_len = 0;
		index->pre_d_up_len = 0;
	}        
	else     
	{        
		index->d_down_len = 0;
		index->pre_d_down_len = 0;
		index->down_flag = 1;
		index->max_down = maxrate * 1024;
		index->min_down = minrate * 1024;
		index->expe_down = maxrate * 1024;
		index->down_limit_len = index->expe_down;
	}
}

/*****************************************************************************
 函 数 名  : get_rule_mib_value
 功能描述  : 获取每一个IP对应的配置参数
 输入参数  : stream_list_t* cur_rule
 			      int start_index,
 			      int end_index,
 			      int i
 输出参数  : 无
 返 回 值  : int

 修改历史      :
  1.日    期   : 2016年9月13日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int get_rule_mib_value( tc_rule_p cur_rule, int start_index, int end_index, int i)
{
	char value[128] = "\0";
	char *tmp;
	char arglists[10][128] ;
	struct in_addr a;
	in_addr_t  cur_ip;
	int ip_part[4][4] = {0};


	value[0] = '\0';
	_GET_VALUE(ADVANCE_TC_RULE(i), tmp);
	strcpy(value, tmp);


	if(strcmp(value, "") == 0)
		return -1;
	bzero(arglists, sizeof(arglists));
	sscanf(value, "%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
	       arglists[0], arglists[1], arglists[2], arglists[3], arglists[4], arglists[5], arglists[6], arglists[7], arglists[8], arglists[9]);

	if(start_index < 0 || start_index > 255)
	{
		cur_ip = tenda_arp_mac_to_ip(arglists[0]);
		if(cur_ip == 0)
			return -1;
		a.s_addr = cur_ip;
		bzero(ip_part, sizeof(ip_part));
		sscanf(inet_ntoa(a), "%[^.].%[^.].%[^.].%s", ip_part[0], ip_part[1], ip_part[2], ip_part[3]);
	}

	cur_rule->port = atoi(arglists[1]);

	if(start_index >= 0 && start_index < 256)
		cur_rule->startip = start_index;
	else
		cur_rule->startip = atoi(ip_part[3]) - 1;

	if(end_index >= 0 && end_index < 256)
		cur_rule->endip = end_index;
	else
		cur_rule->endip = atoi(ip_part[3]) - 1;
	cur_rule->direction = atoi(arglists[4]);
	cur_rule->minrate = atoi(arglists[5]);
	cur_rule->maxrate = atoi(arglists[6]);
	cur_rule->enable = atoi(arglists[7]);
	cur_rule->protocol = atoi(arglists[8]);
	strcpy(cur_rule->mac , arglists[0]);

	return 1;
}
	
	
	
#ifdef CONFIG_RTL_HARDWARE_NAT
extern int rtl865x_delArp(unsigned long ip);
/*****************************************************************************
 函 数 名  : tc_mac_check
 功能描述  : 判断HW MAC 是否匹配TC MAC,  使其不走硬加速
 输入参数  : 
 			      const char * mac,
 输出参数  : 0: 匹配失败1:匹配成功
 
 返 回 值  : int

 修改历史      :
  1.日    期   : 2016年12月29日
    作    者   : 林玲珑
    修改内容   : 新生成函数

*****************************************************************************/
int tc_mac_check(const char * mac)
{
	
	int i;
	char *tmp;
	int flag = 0;
	unsigned long index = 0;
	char value[128] = {0};
	unsigned char mac6byte[ETHER_ADDR_LEN];
	for(i = 0; i < TC_RULE_NUMBER_MAX * 2; ++i)
	{
		
		_GET_VALUE(ADVANCE_TC_RULE(i), tmp);
		strcpy(value, tmp);
		
		if (strlen(value) == 0)
			continue;
		
		sscanf(value, "%s[^,]", value);	
		ether_atoe(value, mac6byte);
		if (!memcmp(mac, mac6byte, ETHER_ADDR_LEN))
		{
			return 1;
		}
	}
#ifdef  __CONFIG_GUEST__
	flag = tenda_arp_is_wireless_client(mac);
	if(flag != 0)
		return 1;
#endif
	return 0;	
}


/*****************************************************************************
 函 数 名  : rtl865x_hw_tc_mac_delete
 功能描述  : 根据TC MAC 删除HW MAC ，使其不走硬加速
 输入参数  : 
 			      const char * mac,
 输出参数  : 无
 
 返 回 值  : 

 修改历史      :
  1.日    期   : 2016年12月29日
    作    者   : 林玲珑
    修改内容   : 新生成函数

*****************************************************************************/
void rtl865x_hw_tc_mac_delete (const char *mac)
{	
	
	in_addr_t tempip;
	tempip = tenda_arp_mac_to_ip(mac);
	
	if (tempip)
	{
		rtl865x_delArp(tempip);
	}
}
#endif


/************************************************************
Function:	 new_init_stream_control
Description: 将限速规则写入限速规则表中

Input:

Output:

Return:

Others:
History:
<author>   <time>	 <version >   <desc>
hqw 	   2013-10-30	1.0 	   新建函数

************************************************************/

int new_init_stream_control(void)
{
	int i = 0;
	struct tc_rule cur_rule;
	char *tc_on = NULL;
	int guest_limit = 0;
	g_user_rate_control = 0;
	/*lq 如果是AP 模式，则不开启流控*/

	if(nvram_match(SYSCONFIG_WORKMODE, "bridge") 
		|| nvram_match(SYSCONFIG_WORKMODE, "client+ap"))
		return 0;
	tc_on =  nvram_safe_get("tc_enable");	//tc_enable为是否开启控制
	if(strcmp(tc_on, "1") == 0)
	{
	
		tc_rule_fflush();
		for(i = 0; i < TC_RULE_NUMBER_MAX * 2; ++i)
		{
		
			if(get_rule_mib_value(&cur_rule, -1, -1, i) == -1)
				continue;
			if(cur_rule.enable == 1)
			{
				set_tc_enable_rule(&(ip_index[cur_rule.startip]), cur_rule.maxrate, cur_rule.minrate, cur_rule.direction);
				strcpy(ip_index[cur_rule.startip].ip_mac , cur_rule.mac);
				#ifdef CONFIG_RTL_HARDWARE_NAT
				rtl865x_hw_tc_mac_delete(cur_rule.mac);
				#endif
			}
			g_user_rate_control = 1;
		}
	}
	else
	{
		tc_rule_fflush();
	}
	
#ifdef __CONFIG_GUEST__
	guest_limit = atoi(nvram_safe_get("wl_guest_down_speed_limit"));
	if(guest_limit)
	{
		set_tc_enable_rule(&guest_total_limit,guest_limit,guest_limit, 1);
		g_user_rate_control = 1;
	}
#endif
	return 0;

}

/*****************************************************************************
 函 数 名  : update_stream_control
 功能描述  : 在有新的客户端接入时，刷新改客户端IP对应的规则
 输入参数  : in_addr_t cur_ip
             char* cur_mac
 输出参数  : 无
 返 回 值  :

 修改历史      :
  1.日    期   : 2016年9月14日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int update_stream_control(in_addr_t cur_ip, char* cur_mac)
{
	int i = 0;
	struct in_addr a;
	struct tc_rule cur_rule;
	int ip_part[4][4] = {0};
	char *tc_on = NULL;
	int rule_index = 0;
	int client_type = 0;

	if(cur_mac == NULL)
		return -1;

	client_type = tenda_arp_is_wireless_client(cur_mac);
	//如果是访客网络的客户端,直接返回
	if((3 == client_type) || (4 == client_type))
	{
		return -1;
	}

	tc_on =  nvram_safe_get("tc_enable");

	a.s_addr = cur_ip;
	bzero(ip_part, sizeof(ip_part));
	sscanf(inet_ntoa(a), "%[^.].%[^.].%[^.].%s", ip_part[0], ip_part[1], ip_part[2], ip_part[3]);
	rule_index = atoi(ip_part[3]) - 1;
	if(ip_index[rule_index].rule != NULL)
	{
		free(ip_index[rule_index].rule);
		diag_printf("ip:%d free\n", rule_index);
		ip_index[rule_index].rule = NULL;
	}

	if(strcmp(tc_on, "1") == 0)
	{
		for(i = 0; i < TC_RULE_NUMBER_MAX * 2; ++i)
		{
			if(get_rule_mib_value(&cur_rule, rule_index, rule_index, i) == -1)
				continue;
			if(strncasecmp(cur_rule.mac, ether_ntoa((struct ether_addr *)cur_mac), strlen(cur_rule.mac)))
				continue;


			if(cur_rule.enable == 1)
			{
				set_tc_enable_rule(&(ip_index[cur_rule.startip]), cur_rule.maxrate, cur_rule.minrate, cur_rule.direction);
				strcpy(ip_index[cur_rule.startip].ip_mac , cur_rule.mac);
			}
			g_user_rate_control = 1;
		}
	}
	return 0;
}



void del_stream_control()
{
	int i = 0;
	g_user_rate_control = 0;
	for(i=0; i< TC_CLIENT_NUMBER; ++i)
	{
		if(ip_index[i].rule== NULL)
			continue;
		free(ip_index[i].rule);
		diag_printf("ip:%d free\n", i);
		ip_index[i].rule= NULL;
	
	}
	return;
}

void enable_tc(void){
	char *tc_enable;
	_GET_VALUE(ADVANCE_TC_ENABLE, tc_enable);
	init_ip_index_array();
	if(strcmp(tc_enable, "1")==0)
	{
		new_init_stream_control();
	}
	return;
}
static unsigned short chsum_fixup_short(unsigned short sum, unsigned short old_data, unsigned short new_data)
{
	 unsigned long sum_new;
	sum_new = sum + old_data - new_data;
	sum_new = (sum_new >> 16) + (sum_new & 0xFFFF);
	return (unsigned short)sum_new;
}

#ifdef __CONFIG_STREAM_STATISTIC__

statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
#ifdef __CONFIG_GUEST__
statistic_ip_index_t guest_stream_ip[STREAM_CLIENT_NUMBER];
unsigned int guest_stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;
#endif
unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;

static total_stream_statistic_t g_total_stream_statistics;

static total_stream_statistic_t g_total_bridge_stream_statistics;

static total_stream_statistic_t g_total_bridge_stream_speed;

#define MAX_TC_STREAM_b  (95 * 1024 * 1024 / 8)//95Mbps

void stream_tmr_func(void)
{
	int i=0;
	unsigned int up_stream = 0,down_stream = 0;
	float up_stream_percent = 1.0,down_stream_percent = 1.0;
	
#ifdef __CONFIG_SPEEDTEST_IMPROVE__
	extern int g_speedtest_web_update_tc_timeout;
	if (g_speedtest_web_update_tc_timeout  > 0)
		g_speedtest_web_update_tc_timeout--;
//add by z10312, 打开网页时,需关闭hw_nat,保持实时速率统计
#ifdef CONFIG_RTL_HARDWARE_NAT
#if defined(CONFIG_RTL_HARDWARE_NAT)
	extern int gHwNatEnabled;
	if (!g_speedtest_web_update_tc_timeout && (!strcmp(nvram_safe_get("lan_netmask"), "255.255.255.0")))
	{
		rtl_hwNatOnOff(1);
		
	}
	else 
	{
		if (gHwNatEnabled) 
		{
			rtl_hwNatOnOff(0);
		}
	}
#endif
#endif
#endif
	
    /* 把tc定时器去除，相应的处理函数加到流量统计定时器函数中，只有开启限速时，
     tc的定时器函数才执行，added by zhuhuan on 2016.07.07 */
    if(1 == g_user_rate_control)
    {
        tc_tmr_func();
    }

	for(i = 0; i < STREAM_CLIENT_NUMBER; ++i){
		if(stream_ip[i].index)
		{
			up_stream += stream_ip[i].index->up_byte_pers;
			down_stream += stream_ip[i].index->down_byte_pers;

		}

#ifdef __CONFIG_GUEST__		
		if(guest_stream_ip[i].index)
		{
			up_stream += guest_stream_ip[i].index->up_byte_pers;
			down_stream += guest_stream_ip[i].index->down_byte_pers;
		}
#endif
	}
#if 1
	/*由于定时器过快，间隔时间不是1s实际是800ms-1050ms，这里取真正时间*/
	static cyg_tick_count_t now_time = 0,before_time = 0;
	float time_real = 1.0;
	before_time = now_time;
	now_time = cyg_current_time();

	time_real = ((float)(now_time - before_time))/100;
#endif
	up_stream_percent = (up_stream > MAX_TC_STREAM_b)?(MAX_TC_STREAM_b / (float)up_stream):1.0;
	down_stream_percent = (down_stream > MAX_TC_STREAM_b)?(MAX_TC_STREAM_b / (float)down_stream):1.0;
	
	for(i = 0; i < STREAM_CLIENT_NUMBER; ++i){
		if(stream_ip[i].index)
		{
#if 1
			stream_ip_per[i][0]=(unsigned int)((float)(stream_ip[i].index->up_byte_pers) / time_real);
			stream_ip_per[i][1]=(unsigned int)((float)(stream_ip[i].index->down_byte_pers) / time_real);
#endif
			stream_ip_per[i][0]=(unsigned int)((float)stream_ip[i].index->up_byte_pers * up_stream_percent);
			stream_ip_per[i][1]=(unsigned int)((float)stream_ip[i].index->down_byte_pers * down_stream_percent);
			stream_ip[i].index->up_byte_pers = 0;
			stream_ip[i].index->down_byte_pers = 0;

		}
#ifdef __CONFIG_GUEST__	
		if(guest_stream_ip[i].index)
		{
			guest_stream_ip_per[i][0]=(unsigned int)((float)(guest_stream_ip[i].index->up_byte_pers) / time_real);
			guest_stream_ip_per[i][1]=(unsigned int)((float)(guest_stream_ip[i].index->down_byte_pers) / time_real);
			guest_stream_ip_per[i][0]=(unsigned int)((float)guest_stream_ip[i].index->up_byte_pers * up_stream_percent);
			guest_stream_ip_per[i][1]=(unsigned int)((float)guest_stream_ip[i].index->down_byte_pers * down_stream_percent);
			guest_stream_ip[i].index->up_byte_pers = 0;
			guest_stream_ip[i].index->down_byte_pers = 0;
		}
#endif
	}
	return ;
}

void init_stream_ip(void){
	int i=0;
	for(i = 0; i < STREAM_CLIENT_NUMBER; ++i){
		stream_ip[i].index=NULL;
#ifdef __CONFIG_GUEST__		
		guest_stream_ip[i].index=NULL;
#endif
	}
 	return;
}
void stream_statistic(ip_t *ip, int direction,int type){
	unsigned int index=0;
	unsigned int ip_len=0;
	statistic_ip_index_t *stream_tmp = stream_ip;
	
	
	
	#ifdef __CONFIG_SPEEDTEST_IMPROVE__
	extern int g_speedtest_web_update_tc_timeout;
	extern int g_speedtest_reboot_check;
	
	if (g_speedtest_web_update_tc_timeout  == 0 &&  g_speedtest_reboot_check == 0)
		return ; 
	#endif

	
	if(direction ==INBOUND ){			/*wan--->lan*/
		index =  (ntohl(ip->ip_dst.s_addr)&0xff) - 1;
	}else if(direction == OUTBOUND){		/*lan--->wan*/
		index =  (ntohl(ip->ip_src.s_addr)&0xff) - 1;
	}else {
		diag_printf("In stream_statistic error!\n");
		return ;
	}
#ifdef __CONFIG_GUEST__
	if(type == GUEST_UP_LIMIT || type == GUEST_DOWN_LIMIT)
	{
		stream_tmp = guest_stream_ip;
	}
#endif
	//根据coverity分析结果修改，原来为无效的判断:if(index >= TC_CLIENT_NUMBER || index < 0)  2017/1/11 F9项目修改
	if(index >= TC_CLIENT_NUMBER)
		return;
	if(stream_tmp[index].index== NULL){
		stream_tmp[index].index = (stream_statistic_t*)malloc(sizeof(stream_statistic_t));
		//bzero((char*)stream_ip[index].index, sizeof(stream_statistic_t));
		if(stream_tmp[index].index != NULL ){
			stream_tmp[index].index->down_bytes = 0;
			stream_tmp[index].index->down_packets = 0;
			stream_tmp[index].index->up_bytes = 0;
			stream_tmp[index].index->up_packets = 0;
			stream_tmp[index].index->up_Mbytes= 0;
			stream_tmp[index].index->down_Mbytes= 0;
			stream_tmp[index].index->up_byte_pers = 0;
			stream_tmp[index].index->down_byte_pers = 0;
		}else{
			diag_printf("stream statistic malloc fial!\n");
			return ;
		}
	}

	ip_len =  ntohs(ip->ip_len);

	update_total_stream_statistic(direction,ip_len);
	
	if(direction == INBOUND){ //down
		stream_tmp[index].index->down_packets++;
		//hqw add for 统计网速的时候减去ip头部以及tcp头部
		if(ip_len >= 40)
		{
			stream_tmp[index].index->down_bytes += ip_len;
			stream_tmp[index].index->down_bytes -= 40;
		}
		//end
		//stream_ip[index].index->down_bytes += ip_len;
 		stream_tmp[index].index->down_byte_pers += ip_len;

		if(stream_tmp[index].index->down_bytes >= 1000*1000 ){
			stream_tmp[index].index->down_Mbytes++;
			stream_tmp[index].index->down_bytes=0;
		}
	}else if(direction == OUTBOUND){	//up
		stream_tmp[index].index->up_packets++;
		stream_tmp[index].index->up_bytes += ip_len;
		stream_tmp[index].index->up_byte_pers += ip_len;
		
		if(stream_tmp[index].index->up_bytes >= 1000*1000){
			stream_tmp[index].index->up_Mbytes++;
			stream_tmp[index].index->up_bytes=0;
		}
	}

	return;
}


void bridge_stream_statistic(ip_t *ip, int direction){
	unsigned int index=0;
	unsigned int ip_len=0;

	ip_len =  ntohs(ip->ip_len);

	if (0 == direction)//UP_DIRECTION
	{
		g_total_bridge_stream_statistics.up_bytes += ip_len;
	}
	else //DOWN_DIRECTION
	{
		//printf("========%lu=======%s [%d]\n",g_total_bridge_stream_statistics.down_bytes, __FUNCTION__, __LINE__);
		g_total_bridge_stream_statistics.down_bytes += ip_len;
	}
	return;
}


void bridge_stream_tmr_func(void)
{
	static unsigned long last_up_bytes = 0;
	static unsigned long last_down_bytes = 0;
	unsigned long up_bytes = 0;
	unsigned long down_bytes = 0;

	up_bytes= g_total_bridge_stream_statistics.up_bytes - last_up_bytes;
	down_bytes = g_total_bridge_stream_statistics.down_bytes - last_down_bytes;
	last_up_bytes = g_total_bridge_stream_statistics.up_bytes ;
	last_down_bytes = g_total_bridge_stream_statistics.down_bytes ;
	g_total_bridge_stream_speed.up_bytes = (up_bytes < MAX_TC_STREAM_b)? up_bytes : MAX_TC_STREAM_b;
	g_total_bridge_stream_speed.down_bytes = (down_bytes < MAX_TC_STREAM_b)? down_bytes : MAX_TC_STREAM_b;

	return ;
}

void get_bridge_stream_speed(unsigned long * tx,unsigned long * rx)
{
	(*tx) = g_total_bridge_stream_speed.up_bytes;
	(*rx) = g_total_bridge_stream_speed.down_bytes;

	return;
}

#define UP_DIRECTION		0
#define DOWN_DIRECTION		1
/*
	up_bytes、down_bytes类型为unsigned long
	不用处理超过0XFFFFFF的情况
	如超过0XFFFFFF会自动从0开始计数
*/
void update_total_stream_statistic(int direction,unsigned int ip_len)
{	
	if (UP_DIRECTION == direction)//UP_DIRECTION
	{
		g_total_stream_statistics.up_bytes += ip_len;
	}
	else //DOWN_DIRECTION
	{
		g_total_stream_statistics.down_bytes += ip_len;
	}

	return;
}
	
void get_total_stream_statistic(u_long * tx,u_long * rx)
{
	(*tx) = g_total_stream_statistics.up_bytes;
	(*rx) = g_total_stream_statistics.down_bytes;

	return;
}
#endif/*__CONFIG_STREAM_STATISTIC__*/
