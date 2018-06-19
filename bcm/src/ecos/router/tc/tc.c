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
#include <tc.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <stdlib.h>
#include <route_cfg.h>
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



void update_total_stream_statistic(int direction,unsigned int ip_len);
void get_total_stream_statistic(u_long * tx,u_long * rx);

extern int str2arglist(char *buf, int *list, char c, int max);
static unsigned short chsum_fixup_short(unsigned short sum, unsigned short old_data, 
						unsigned short new_data);

unsigned int total_rate_runtime;
tc_ip_index_t ip_index[TC_CLIENT_NUMBER] ;
int g_user_rate_control=0;

#define TC_MALLOC(X)		if(ip_index[X].rule==NULL){	\
								ip_index[X].rule=(tc_ip_rate_t*)malloc(sizeof(tc_ip_rate_t));	\
								if(ip_index[X].rule!=NULL){			\
									bzero((char*)ip_index[X].rule,sizeof(tc_ip_rate_t*));	\
								}else{		\
									diag_printf("init_stream_control	error!\n");	\
									return-1;	\
								}	\
							}	


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
	
	tc_set_timeout();
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


int down_win_stream_control(int nat_dir,ip_t *ip)
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

#ifdef	__CONFIG_ALINKGW__
	if(start_al_qos_flag)
	{
		al_qos_stream[index].real_down += ip_len ;
		if(al_qos_stream[index].real_down >= al_qos_stream[index].expect_down)
		{
			return -1 ;
		}
	}
#endif

	
	//diag_printf("index=%d\n",index);	
	if((ip_index[index].rule != NULL) && (ip_index[index].down_flag != 0) ){
		ip_index[index].down_len += ip_len;
		//if(ip_index[index].down_len < (ip_index[index].expe_down)){
		if((ip_index[index].down_len)  < (ip_index[index].down_limit_len)){
		
#ifdef __CONFIG_TC_RANDOM_DROP__ 
			
#endif
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
#ifdef __CONFIG_STREAM_STATISTIC__
void stream_statistic2(int index, int ip_len, int direction);
#endif

int tc_control_forward(struct ip *ip, unsigned long lan_addr ,unsigned long lan_mask)
{
	unsigned long index = 0;
	unsigned short ip_len = 0;
	unsigned long src_ip, dst_ip;
	unsigned char dir = -1;

	src_ip = ntohl(ip->ip_src.s_addr);
	dst_ip = ntohl(ip->ip_dst.s_addr);
	ip_len =  ip->ip_len;

	//diag_printf("2===========src %lx dst %lx ",src_ip,dst_ip);

	if(ip_len < 100 || (src_ip & lan_mask) == (dst_ip & lan_mask)){
		// is it impossible go to here?
		return 0;
	}

	if((src_ip & lan_mask) == (lan_addr & lan_mask)){
		dir = 0;// up stream
	}
	else if( (dst_ip & lan_mask) == (lan_addr & lan_mask)){
		dir = 1;// down stream
	}
	else{
		dir = -1;
	}

	//diag_printf("dir=%d ip_len=%d\n",dir,ip_len);
	
	switch(dir)
	{
		case 0:
			index =  (src_ip&0xff) - 1;
			if((ip_index[index].rule != NULL) && (ip_index[index].up_flag != 0) ){
				ip_index[index].up_len += ip_len;
				if(ip_index[index].up_len < (ip_index[index].expe_up)){
					;
				}else{
					ip_index[index].d_up_len += ip_len;

					return -1;
				}
			}
		       break;
		case 1:
			index =  (dst_ip&0xff) - 1;
			if((ip_index[index].rule != NULL) && (ip_index[index].down_flag != 0) ){
				ip_index[index].down_len += ip_len;
				if(ip_index[index].down_len < (ip_index[index].expe_down)){
						;				
				}else{
					ip_index[index].d_down_len += ip_len;

					return -1;
				}
			}
			break;
		default:
			break;
	}

#ifdef __CONFIG_STREAM_STATISTIC__
	if(index != 0){
		stream_statistic2(index, ip_len,dir);
	}
#endif

	return 0;
}
#endif /*__CONFIG_TC_WINDOWS__*/


void init_ip_index_array(void){
	int i;
	for(i=0; i< TC_CLIENT_NUMBER; ++i){
		ip_index[i].rule= NULL;	
	}
	return;
}

int init_stream_control(void)
{
//tcrulename: 0port,1sip,2eip,3(0:up,1:down),4minrate,5maxrate,6(1:enable,0:disable),7(0:TCP&UDP,1:tcp,2:udp)
	int i = 0, argc = 0;
	unsigned int startip=0, endip=0;
	unsigned int minrate=0, maxrate=0;
	unsigned int direction = 0;
	unsigned int enable = 0;
	unsigned int port = 0,  protocol = 0;
	//static int tc_init[TC_RULE_NUMBER_MAX];
	//char tc_name[8] = "tc_XXXX";
	char *tc_on=NULL;
	char value[128]="\0";
	char *tmp;
	char *arglists[8] ;

	//this is bad way, because of user repate save TC rule, cause system burthen
	// repate malloc and free real isn't good idea.
	 
	 if(g_user_rate_control == 1 ){
		tc_untimeout();
		g_user_rate_control =0;
	 }
	bzero(arglists, sizeof(arglists));
	tc_on =  nvram_safe_get("tc_enable");	
	if(strcmp(tc_on, "1") == 0){		//enable stream control					
		for(i=0; i< TC_CLIENT_NUMBER; ++i){
			if(ip_index[i].rule== NULL)
				continue;
			//KFREE(ip_index[i].rule);
			free(ip_index[i].rule);
			diag_printf("ip:%d free\n", i);
			ip_index[i].rule= NULL;	
		}	//add by stanley for inused rule collison by 2010/11/11
		
		for(i = 0; i < TC_RULE_NUMBER_MAX; ++i){	//read TC config parameter
			value[0]='\0';
			_GET_VALUE(TC_RULE_(i), tmp);
			strcpy(value, tmp);
	
			if(value == NULL)
				continue;
			
			argc = str2arglist(value, (int*) arglists, ',', 8);	//modify by stanley 2010/10/19

			if(argc < 8)
				continue;
			
			port = atoi(arglists[0]);//端口
			startip = atoi(arglists[1])-1;//起始地址
			endip = atoi(arglists[2])-1;//终止地址
			direction = atoi(arglists[3]);		//0:up上传; 1:down下载
			minrate = atoi(arglists[4]);//最小速率
			maxrate = atoi(arglists[5]);//最大速率
			enable = atoi(arglists[6]);		//0:disable; 1:enable
			protocol = atoi(arglists[7]);		//0:TCP&UDP; 1:TCP; 2:UDP

			if(0 == direction && enable == 1){		//up
				for(; startip <= endip; ++startip){
					if(ip_index[startip].rule == NULL){
						TC_MALLOC(startip);
						ip_index[startip].down_flag = 0;
						ip_index[startip].max_down = 0;
						ip_index[startip].min_down = 0 ;
						ip_index[startip].expe_down = 0 ;
						ip_index[startip].d_down_len = 0;
						ip_index[startip].d_up_len = 0;
						ip_index[startip].pre_d_down_len = 0;
						ip_index[startip].pre_d_up_len = 0;
						
						//diag_printf("up ip: %d malloc\n", startip);
					}
					//diag_printf("ip:%d  %s=%d\n", startip, __FUNCTION__, __LINE__);
					ip_index[startip].up_flag = 1; 
					ip_index[startip].max_up = maxrate * 1024;
					ip_index[startip].min_up = minrate * 1024;
					ip_index[startip].expe_up = maxrate * 1024;	
				}
			}else if( 1 == direction && enable == 1){//down
				for(; startip <= endip && enable ==1 ; ++startip){
					if(ip_index[startip].rule== NULL){
						TC_MALLOC(startip);
						ip_index[startip].up_flag = 0; 
						ip_index[startip].max_up = 0;
						ip_index[startip].min_up = 0;
						ip_index[startip].expe_up = 0;
						ip_index[startip].d_down_len = 0;
						ip_index[startip].d_up_len = 0;
						ip_index[startip].pre_d_down_len = 0;
						ip_index[startip].pre_d_up_len = 0;
						
						//diag_printf("down ip:%d  malloc\n", startip);
					}
					//diag_printf("ip:%d  %s=%d\n", startip, __FUNCTION__, __LINE__);
					ip_index[startip].down_flag = 1;
					ip_index[startip].max_down = maxrate * 1024;
					ip_index[startip].min_down = minrate * 1024;
					ip_index[startip].expe_down =maxrate * 1024;
					ip_index[startip].down_limit_len = ip_index[startip].expe_down;
				}
			}
		} //for: i < TC_RULE_NUMBER_MAX		
		
		if (g_user_rate_control == 0){
			tc_set_timeout();
			g_user_rate_control = 1;
		}	//add by stanley for resolve repeat set timeout by 2010/11/11
		
	}else{	//Disable stream control		
	
		if (g_user_rate_control ==1){
			tc_untimeout();
			g_user_rate_control = 0;
		}
		for(i=0; i< TC_CLIENT_NUMBER; ++i){
			if(ip_index[i].rule== NULL)
				continue;
			//KFREE(ip_index[i].rule);
			free(ip_index[i].rule);
			diag_printf("ip:%d free\n", i);
			ip_index[i].rule= NULL;
			
		}
	}
	return 0;
}


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
	//tcrulename: 0port,1sip,2eip,3(0:up,1:down),4minrate,5maxrate,6(1:enable,0:disable),7(0:TCP&UDP,1:tcp,2:udp)
		int i = 0,j = 0,argc = 0;
		unsigned int startip=0, endip=0;
		unsigned int minrate=0, maxrate=0;
		unsigned int direction = 0;
		unsigned int enable = 0;
		unsigned int port = 0,	protocol = 0;
		//static int tc_init[TC_RULE_NUMBER_MAX];
		//char tc_name[8] = "tc_XXXX";
		char *tc_on=NULL;
		char value[128]="\0";
		char *tmp;
		char arglists[10][128] ;
		char c[5]={0};
		char c1[4][4]={0};
		struct in_addr a;
		
	
		//this is bad way, because of user repate save TC rule, cause system burthen
		 //repate malloc and free real isn't good idea.
		 
		 if(g_user_rate_control == 1 )
		 {
			tc_untimeout();
			g_user_rate_control =0;
		 }
		
		tc_on =  nvram_safe_get("tc_enable");	//tc_enable为是否开启控制		
		if(strcmp(tc_on, "1") == 0)
		{
			for(i=0; i< TC_CLIENT_NUMBER; ++i)
			{
				if(ip_index[i].rule== NULL)
					continue;
				free(ip_index[i].rule);
				diag_printf("ip:%d free\n", i);
				ip_index[i].rule= NULL; 
			}	
		
			for(i = 0; i < TC_RULE_NUMBER_MAX; ++i)
			{	//read TC config parameter*
				value[0]='\0';
				_GET_VALUE(TC_RULE_(i), tmp);
				strcpy(value, tmp);
				
		
				if(strcmp(value,"") == 0)
					continue;
				bzero(arglists, sizeof(arglists));
				sscanf(value,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
					arglists[0],arglists[1],arglists[2],arglists[3],arglists[4],arglists[5],arglists[6],arglists[7],arglists[8],arglists[9]);
				
				port = atoi(arglists[1]);//端口
				startip = atoi(arglists[2])-1;//起始地址
				endip = atoi(arglists[3])-1;//终止地址
				direction = atoi(arglists[4]);		//0:up上传; 1:down下载
				minrate = atoi(arglists[5]);//最小速率
				maxrate = atoi(arglists[6]);//最大速率
				enable = atoi(arglists[7]); 	//0:disable; 1:enable
				protocol = atoi(arglists[8]);		//0:TCP&UDP; 1:TCP; 2:UDP
		
				if( 1 == direction && enable == 1)
				{//down
						if(ip_index[startip].rule== NULL)
						{
							TC_MALLOC(startip);
						}
						if(maxrate >0 && maxrate <= 128 * 8)
						{
							ip_index[startip].up_flag = 1; 
							ip_index[startip].max_up = 32 * 1024;
							ip_index[startip].min_up = 32 * 1024;
							ip_index[startip].expe_up = 32 * 1024;
						}
						else
						{
							ip_index[startip].up_flag = 0; 
							ip_index[startip].max_up = 0;
							ip_index[startip].min_up = 0;
							ip_index[startip].expe_up = 0;
						}
						ip_index[startip].d_down_len = 0;
						ip_index[startip].d_up_len = 0;
						ip_index[startip].pre_d_down_len = 0;
						ip_index[startip].pre_d_up_len = 0;
						ip_index[startip].down_flag = 1;
						ip_index[startip].max_down = maxrate * 1024;
						ip_index[startip].min_down = minrate * 1024;
						ip_index[startip].expe_down =maxrate * 1024;
						ip_index[startip].down_limit_len = ip_index[startip].expe_down;
						strcpy(ip_index[startip].ip_mac , arglists[0]);
				}
			} //for: i < TC_RULE_NUMBER_MAX 		
			
			if (g_user_rate_control == 0)
			{
				tc_set_timeout();
				g_user_rate_control = 1;
			}	//add by stanley for resolve repeat set timeout by 2010/11/11
		}
		else
		{
			if (g_user_rate_control ==1){
				tc_untimeout();
				g_user_rate_control = 0;
			}
			for(i=0; i< TC_CLIENT_NUMBER; ++i){
				if(ip_index[i].rule== NULL)
					continue;
				free(ip_index[i].rule);
				diag_printf("ip:%d free\n", i);
				ip_index[i].rule= NULL;
			
			}
		}
		return 0;

}


void enable_tc(void){
	char *tc_enable;
	_GET_VALUE(TC_ENABLE, tc_enable);
	init_ip_index_array();
	if(strcmp(tc_enable, "1")==0)
	{
		#ifdef __CONFIG_TENDA_HTTPD_V3__
		init_stream_control();
		#else
		new_init_stream_control();
		#endif
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
unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;

static total_stream_statistic_t g_total_stream_statistics;

void stream_tmr_func(void)
{
	int i=0;
	
	for(i = 0; i < STREAM_CLIENT_NUMBER; ++i){
		if(stream_ip[i].index == NULL)
			continue;

		stream_ip_per[i][0]=stream_ip[i].index->up_byte_pers ;
		stream_ip_per[i][1]=stream_ip[i].index->down_byte_pers ;
		stream_ip[i].index->up_byte_pers = 0;
		stream_ip[i].index->down_byte_pers = 0;
	}
	return ;
}

void init_stream_ip(void){
	int i=0;
	for(i = 0; i < STREAM_CLIENT_NUMBER; ++i){
		stream_ip[i].index=NULL;
	}
 	return;
}
void stream_statistic(ip_t *ip, int direction){
	unsigned int index=0;
	unsigned int ip_len=0;

	if(direction ==INBOUND ){			/*wan--->lan*/
		index =  (ntohl(ip->ip_dst.s_addr)&0xff) - 1;
	}else if(direction == OUTBOUND){		/*lan--->wan*/
		index =  (ntohl(ip->ip_src.s_addr)&0xff) - 1;
	}else {
		diag_printf("In stream_statistic error!\n");
		return ;
	}

	if(stream_ip[index].index== NULL){
		stream_ip[index].index = (stream_statistic_t*)malloc(sizeof(stream_statistic_t));
		//bzero((char*)stream_ip[index].index, sizeof(stream_statistic_t));
		if(stream_ip[index].index != NULL ){
			stream_ip[index].index->down_bytes = 0;
			stream_ip[index].index->down_packets = 0;
			stream_ip[index].index->up_bytes = 0;
			stream_ip[index].index->up_packets = 0;
			stream_ip[index].index->up_Mbytes= 0;
			stream_ip[index].index->down_Mbytes= 0;
			stream_ip[index].index->up_byte_pers = 0;
			stream_ip[index].index->down_byte_pers = 0;
		}else{
			diag_printf("stream statistic malloc fial!\n");
			return ;
		}
	}

	ip_len =  ntohs(ip->ip_len);

	update_total_stream_statistic(direction,ip_len);
	
	if(direction == INBOUND){ //down
		stream_ip[index].index->down_packets++;
		//hqw add for 统计网速的时候减去ip头部以及tcp头部
		if(ip_len >= 40)
		{
			stream_ip[index].index->down_bytes += ip_len;
			stream_ip[index].index->down_bytes -= 40;
		}
		//end
		//stream_ip[index].index->down_bytes += ip_len;
		stream_ip[index].index->down_byte_pers += ip_len;

		if(stream_ip[index].index->down_bytes >= 1000*1000 ){
			stream_ip[index].index->down_Mbytes++;
			stream_ip[index].index->down_bytes=0;
		}
	}else if(direction == OUTBOUND){	//up
		stream_ip[index].index->up_packets++;
		stream_ip[index].index->up_bytes += ip_len;
		stream_ip[index].index->up_byte_pers += ip_len;
		
		if(stream_ip[index].index->up_bytes >= 1000*1000){
			stream_ip[index].index->up_Mbytes++;
			stream_ip[index].index->up_bytes=0;
		}
	}

	return;
}
void stream_statistic2(int index, int ip_len, int direction)
{
	if(stream_ip[index].index== NULL){
		stream_ip[index].index = (stream_statistic_t*)malloc(sizeof(stream_statistic_t));
		//bzero((char*)stream_ip[index].index, sizeof(stream_statistic_t));
		if(stream_ip[index].index != NULL ){
			stream_ip[index].index->down_bytes = 0;
			stream_ip[index].index->down_packets = 0;
			stream_ip[index].index->up_bytes = 0;
			stream_ip[index].index->up_packets = 0;
			stream_ip[index].index->up_Mbytes= 0;
			stream_ip[index].index->down_Mbytes= 0;
			stream_ip[index].index->up_byte_pers = 0;
			stream_ip[index].index->down_byte_pers = 0;
		}else{
			diag_printf("stream statistic malloc fial!\n");
			return ;
		}
	}

	update_total_stream_statistic(direction,ip_len);
	
	if(direction == 1){ //down
		stream_ip[index].index->down_packets++;
		stream_ip[index].index->down_bytes += ip_len;
		stream_ip[index].index->down_byte_pers += ip_len;

		if(stream_ip[index].index->down_bytes >= 1000*1000 ){
			stream_ip[index].index->down_Mbytes++;
			stream_ip[index].index->down_bytes=0;
		}
	}else if(direction == 0){	//up
		stream_ip[index].index->up_packets++;
		stream_ip[index].index->up_bytes += ip_len;
		stream_ip[index].index->up_byte_pers += ip_len;
		
		if(stream_ip[index].index->up_bytes >= 1000*1000){
			stream_ip[index].index->up_Mbytes++;
			stream_ip[index].index->up_bytes=0;
		}
	}

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
