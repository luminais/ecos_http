#ifdef __CONFIG_TC__
#include <stdio.h>
#include <string.h>
#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"

#ifdef __CONFIG_STREAM_STATISTIC__
#include "../tc/tc.h"
#endif
#ifdef __CONFIG_SUPPORT_GB2312__
#include "wds_encode.h"
#endif


#define TC_RULE_NUMBER_MAX 20
static void fromtc(webs_t wp, char_t *path, char_t *query);
static void fromSpeedControlSave(webs_t wp, char_t *path, char_t *query);
static int get_tc_list(int eid, webs_t wp, int argc, char_t **argv);
static int get_tc_othe(int eid, webs_t wp, int argc, char_t **argv);
int  dhcpd_start(void);
void dhcpd_stop(void);


#define newforeach(word, wordlist, next) \
	for (next = &wordlist[strspn(wordlist, " ")], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ';'); \
	     strlen(word); \
	     next = next ? &next[strspn(next, " ")] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, " ")] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, ';'))


#ifdef __CONFIG_STREAM_STATISTIC__
extern statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];
static void get_stream_stat_list(webs_t wp, char_t *path, char_t *query);
static void formSpeedControlInit(webs_t wp, char_t *path, char_t *query);
static void enable_stream_stat(webs_t wp, char_t *path, char_t *query);
static int get_stream_stat_en(int eid, webs_t wp, int argc, char_t **argv);
#endif/*__CONFIG_STREAM_STATISTIC__*/

void tc_form(void)
{
	websFormDefine(T("trafficForm"), fromtc);//heqiwei 2.4
#ifdef __CONFIG_STREAM_STATISTIC__
	websFormDefine(T("updateIptAccount"), get_stream_stat_list);//heqiwei 2.3
	websFormDefine(T("iptAcount_mng"), enable_stream_stat);
#endif/*__CONFIG_STREAM_STATISTIC__*/
	/**********************hqw add for F307***************************/
	websFormDefine(T("SpeedControlInit"), formSpeedControlInit);//网速控制
	websFormDefine(T("SpeedControlSave"), fromSpeedControlSave);//网速控制
	/***************************************************************/
	return ;
}

void tc_asp(void)
{
	websAspDefine(T("get_tc_list"), get_tc_list);
	websAspDefine(T("get_tc_othe"), get_tc_othe);
#ifdef __CONFIG_STREAM_STATISTIC__
	websAspDefine(T("get_stream_stat_en"), get_stream_stat_en);
#endif

	return ;
}


static void fromtc(webs_t wp, char_t *path, char_t *query)
{
	char_t  *isp_uprate=NULL, *isp_downrate=NULL, *tc_list=NULL,*tc_enable=NULL; 
	int itc;
	char tcname[20] = "tc_list_XXXXXXX";
	char name[20] = "tc_XXXXXXX";
	//char list[128];

	isp_uprate = websGetVar(wp, T("up_Band"), T("12800"));
	isp_downrate = websGetVar(wp, T("down_Band"), T("12800"));
	tc_enable = websGetVar(wp, T("tc_enable"), T("0"));
	

  	_SET_VALUE(TC_ISP_UPRATE, isp_uprate);
	_SET_VALUE(TC_ISP_DOWNRATE, isp_downrate);
	_SET_VALUE(TC_ENABLE, tc_enable);
/*
	diag_printf("set %s=%s\n", TC_ISP_UPRATE, isp_uprate);
	diag_printf("set %s=%s\n", TC_ISP_DOWNRATE, isp_downrate);
	diag_printf("set %s=%s\n", TC_ENABLE, tc_enable);
*/	
	for (itc = 0 ; itc< 10; itc++) {
		sprintf(tcname,"tc_list_%d", (itc+1) );
		tc_list = websGetVar(wp, tcname, T(""));
		
		sprintf(name, "tc_%d", itc);
		if (strlen(tc_list) > 6) {
			diag_printf("set %dth:	%s=%s\n", itc, name, tc_list);
			_SET_VALUE(name, tc_list);

		} else {
			_SET_VALUE(name, "");
			//_DEL_VALUE(name);
		}
	}
  
	
	_COMMIT();
	
	websRedirect(wp, T("net_tc.asp"));
	cyg_thread_delay(100);

	init_stream_control();

	return ;
}

static int get_tc_list(int eid, webs_t wp, int argc, char_t **argv)
{
	int itc=0, retv=0;
	char *value=NULL;
	char tcName[16]="tc_XXXXXX";
	//diag_printf("%s__________%d\n", __FUNCTION__, __LINE__);
	for(itc = 0; itc <10; itc ++) {
		//diag_printf("%s__________%d\n", __FUNCTION__, __LINE__);
		sprintf(tcName,"tc_%d",itc);
		
		_GET_VALUE(tcName, value);
		
		if (strlen(value) > 6) {
			diag_printf("get %dth:	%s=%s\n", itc, tcName, value);
			if (itc == 0) 
				retv = websWrite(wp, T("'%s'"), value);
			else
				retv = websWrite(wp, T(",'%s'"), value);	
		}
	
	}
	
	
	return retv;
}

static int get_tc_othe(int eid, webs_t wp, int argc, char_t **argv)
{
	char_t *name=NULL;
	char_t *value=NULL;
	
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	if(strcmp(name, "isp_uprate") == 0)
	{
		_GET_VALUE(TC_ISP_UPRATE, value);
	}else if(strcmp(name, "isp_downrate") == 0){
	
		_GET_VALUE(TC_ISP_DOWNRATE, value);
	}else if(strcmp(name, "tc_en") == 0){
	
		_GET_VALUE(TC_ENABLE, value);
	}else if(strcmp(name, "lanip") == 0){

		_GET_VALUE(_LAN0_IP, value);
	}

	websWrite(wp, T("%s"), value);
	
	return 0;
}


#ifdef __CONFIG_STREAM_STATISTIC__

static void get_stream_stat_list(webs_t wp, char_t *path, char_t *query)
{
	char_t pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char_t *lanip, *p;
	unsigned int index;
	int up = 0, down = 0;
	int up1=0, down1=0;
	int up2 = 0, down2 = 0;
	int  u_kbs=0, d_kbs=0, u_kbs0=0, d_kbs0=0, u_kbs1=0, d_kbs1=0, u_kbs2 =0, d_kbs2 =0;

	_GET_VALUE(_LAN0_IP, lanip);

	strcpy(pr_ip, lanip);
	p=rindex(pr_ip, '.');
	if(p) *p='\0';

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain\nPragma: no-cache\n\n"));
	
	for(index=0; index< STREAM_CLIENT_NUMBER; ++index){
		if(stream_ip[index].index == NULL)
			continue;	

		sprintf(pre_ip, "%s.%d", pr_ip, index+1);
		(ip_ubs(index)!=0) ? (u_kbs0 = ip_ubs(index)/1000) : (u_kbs0=0);
		(ip_dbs(index)!=0) ? (d_kbs0 = ip_dbs(index)/1000) : (d_kbs0=0);
		
		(ip_ubs(index)!=0) ? (u_kbs = ip_ubs(index)%1000) : (u_kbs=0);
		(ip_dbs(index)!=0) ? (d_kbs = ip_dbs(index)%1000) : (d_kbs=0);
				
		(u_kbs!=0) ? (u_kbs1=u_kbs/100) : (u_kbs1=0);
		(d_kbs!=0) ? (d_kbs1=d_kbs/100) : (d_kbs1=0);
		(u_kbs1!=0) ? (u_kbs2=u_kbs1/10) : (u_kbs2=0);
		(d_kbs1!=0) ? (d_kbs2=d_kbs1/10) : (d_kbs2=0);
		
		(ip_uby(index)!=0) ? (up=ip_uby(index)/1000) : (up=0);
		(ip_dby(index)!=0) ? (down=ip_dby(index)/1000) : (down=0);
		
		(up!=0) ? (up1=up/100) : (up1=0);
		(down!=0) ? (down1=down/100) : (down1=0);
		(up1!=0) ? (up2=up1/10) : (up2=0);
		(down1!=0)?(down2=down1/10) : (down2=0);
#if 0
		diag_printf("%s;%d.%d%d;%d.%d%d;%d;%d.%d%d;%d;%d.%d%d\n",
			pre_ip, 
			(u_kbs0), (u_kbs1), (u_kbs2),
			(d_kbs0), (d_kbs1), (d_kbs2),
			ip_upa(index),ip_uM(index), up1, up2,
			ip_dpa(index),ip_dM(index), down1, down2
		);
#endif
		/*ip;uprate;downrate;sendpacket;sendbytes;recvpackets;recvbytes*/
		websWrite(wp, T("%s;%d.%d%d;%d.%d%d;%d;%d.%d%d;%d;%d.%d%d\n"),
			pre_ip, 
			(u_kbs0), (u_kbs1), (u_kbs2),
			(d_kbs0), (d_kbs1), (d_kbs2),
			ip_upa(index),ip_uM(index), up1, up2,
			ip_dpa(index),ip_dM(index), down1, down2
		);
		
	}
	
	//websWrite(wp, result);
	websDone(wp, 200);	
	return ;
}

void enable_stream_stat(webs_t wp, char_t *path, char_t *query){

	char_t *value= websGetVar(wp, T("enableiptAccountEx"), T("1"));

	_SET_VALUE(TC_STREAM_STAT_EN, value);
	_COMMIT();

	websRedirect(wp, T("sys_iptAccount.asp"));
	
	return ;
}

int get_stream_stat_en(int eid, webs_t wp, int argc, char_t **argv){
	char_t *name=NULL;
	char_t *value=NULL;
	if (ejArgs(argc, argv, T("%s"), &name) < 1) {
		return websWrite(wp, T("Insufficient args\n"));
	}

	if(strcmp(name, "stream_stat_en") == 0){
		_SAFE_GET_VALUE(TC_STREAM_STAT_EN, value);
		
	}
	//diag_printf("TC_STREAM_STAT_EN=%s\n", value)
	websWrite(wp, T("%d"), value == NULL ? 0 : atoi(value));

	return 0;
}
/*************************hqw add for F307***********************/
/************************************************************
Function:	 d_list_to_web               
Description:   网速控制的后台接口，将之前写入的限速规则，
			备注规则，过滤规则加入表中

Input:                                          

Output: 

Return:         是否向web传输数据

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

int is_gb2312_code(char * str)
{
	unsigned one_byte = 0X00; //binary 00000000

	int i = 0;

    unsigned char k = 0;
    unsigned char c = 0;
	
	if(str == NULL)
	{
		return -1;
	}
	
    for (i=0; (unsigned char)str[i] != '\0' ;)
    {
        c = (unsigned char)str[i];
        if (c>>7 == one_byte) 
		{
            i++;
            continue;
        } 
		else if(c >= 0XA1 && c <= 0XF7)
		{
            k = (unsigned char)str[i+1];
            if (k >= 0XA1 && k <= 0XFE)
			{				
				return 1;
            }       
        }
        i += 2; 
    }

    return 0; 
}


int d_list_to_web(char *name,webs_t wp)
{
	int i,j,mac_temp,first_temp=0;
	char arglists[12][128]={0};
	char *nvram_loc_value;
	char nvram_loc[128]={0},middle_vlaue[25]={0},vlaue[256]={0};
	char lowermac[18]={0};
	int mactoip[6]={0};
	char result[1024]={0};
	char ip_part[4][5]={0};
	int ip_num = 0;
	struct in_addr a;
	char mib_value[256] = {0};
	
	for(i=0;i<20;i++)
	{
		sprintf(nvram_loc,"%s%d",name,i);
		_GET_VALUE(nvram_loc,nvram_loc_value);
		memset(mib_value , 0 ,sizeof(mib_value));
		if(strlen(nvram_loc_value) < sizeof(mib_value))
			strncpy(mib_value, nvram_loc_value, strlen(nvram_loc_value));
		else
			strncpy(mib_value, nvram_loc_value, sizeof(mib_value) -1);
		
		if(strcmp(mib_value,"") == 0)
			continue;
		
		if(rindex(mib_value,',') == NULL)
			continue;
		
		bzero(arglists,sizeof(arglists));
		if(strcmp(name,"tc_") == 0)
		{
			sscanf(mib_value,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
				arglists[0],arglists[1],arglists[2],arglists[3],arglists[4],arglists[5],arglists[6],arglists[7],arglists[8],arglists[10],arglists[9]);
			strncpy(arglists[9],rindex(mib_value,',')+1,strlen(rindex(mib_value,',')+1));
		}
		else if(strcmp(name,"remark_") == 0)
		{
			sscanf(mib_value,"%[^,],%s,",
				arglists[0],arglists[1]);
			strncpy(arglists[1],rindex(mib_value,',')+1,strlen(rindex(mib_value,',')+1));
		}
		else
		{
			sscanf(mib_value,"%[^,],%[^,],%[^,],%[^,],%s,",
				arglists[0],arglists[1],arglists[2],arglists[3],arglists[4]);			
			strncpy(arglists[4],rindex(mib_value,',')+1,strlen(rindex(mib_value,',')+1));
		}
		strncpy(lowermac,arglists[0],17);
		for(j=0;j<strlen(arglists[0]);j++)
		{
			if(lowermac[j] >= 'A'&&lowermac[j] <= 'Z')
				lowermac[j] +=32;
		}
		//根据mac找IP
		sscanf(lowermac,"%02x:%02x:%02x:%02x:%02x:%02x",mactoip,mactoip+1,mactoip+2,mactoip+3,mactoip+4,mactoip+5);
		a.s_addr = lookip(mactoip);
		if(strcmp(inet_ntoa(a),"0.0.0.0") == 0)
			continue;
		sscanf(inet_ntoa(a),"%[^.].%[^.].%[^.].%s",ip_part[0],ip_part[1],ip_part[2],ip_part[3]);
		ip_num = atoi(ip_part[3]);
		if(stream__ip_iist[ip_num].index == NULL)
			stream__ip_iist[ip_num].index = (stream_list_t*)malloc(sizeof(stream_list_t));
		
		strcpy(stream__ip_iist[ip_num].index->hostname," ");
		stream__ip_iist[ip_num].index->ip = lookip(mactoip);
		strcpy(stream__ip_iist[ip_num].index->mac,arglists[0]) ;
		//stream__ip_iist[ip_num].index->type = 0;
		if(strcmp(name,"tc_") == 0)
		{
			stream__ip_iist[ip_num].index->down_byte_pers = 0;
			stream__ip_iist[ip_num].index->set_pers = (atoi(arglists[6]));//-1为无限制，0表示无法上网
			
			if(strlen(arglists[10]) == 0)
				stream__ip_iist[ip_num].index->limit = 301;
			else
				stream__ip_iist[ip_num].index->limit = (atof(arglists[10]));
			strcpy(stream__ip_iist[ip_num].index->remark,arglists[9]) ;
		}
		else if(strcmp(name,"remark_") == 0)
		{
			stream__ip_iist[ip_num].index->down_byte_pers = 0;
			stream__ip_iist[ip_num].index->set_pers = 301*128;//-1为无限制，0表示无法上网
			stream__ip_iist[ip_num].index->limit = 301;
			strcpy(stream__ip_iist[ip_num].index->remark,arglists[1]) ;
		}
		else
		{
			stream__ip_iist[ip_num].index->down_byte_pers = 0;
			stream__ip_iist[ip_num].index->set_pers = 0;//-1为无限制，0表示无法上网
			stream__ip_iist[ip_num].index->limit = 0;
			strcpy(stream__ip_iist[ip_num].index->remark,arglists[4]) ;	
		}
	}
	return first_temp;
}
#ifdef __CONFIG_ALINKGW__
static int getBlacklist(char *result)
{
	char *value ;
	if(result == NULL )
		return 0;
	
	if (!nvram_match(WLN0_MAC_MODE, "deny")) 
	{
		return 0;
	}
	_GET_VALUE(WLN0_MAC_LIST, value);

	if(strcmp(value, "") == 0)
		return 0;

	char *list = (char *)malloc(strlen(value)+1);
	if(list == NULL){
		printf(" malloc failed\n");
		return -1;
	}
	
	memset(list,0,strlen(value)+1);

	strcpy(list, value);
	
	int num = 0;
	char *sp,*ep;

	num++;
	sp = list;
	while((ep = strchr(sp, ' '))) {
		if(num >= 2)
			strncat(result, ",", 1);	
		*ep++ = '\0';
		strncat(result, "{", 1);
		string_cat(result,"macval",sp);
		strncat(result, "}", 1);
		sp = ep;
		num++;	
	}
	
	if(num >= 2)
		strncat(result, ",", 1);	
	/* 只有一条或最后一条*/
	strncat(result, "{", 1);
	string_cat(result,"macval",sp);
	strncat(result, "}", 1);
	free(list);
	list = NULL;
	return num;
}
#endif
/************************************************************
Function:	 formSpeedControlInit               
Description:  网速控制的前台接口

Input:                                          

Output: 

Return:         返回给前台客户端的信息

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

extern char* get_product_pwr_info();

static void formSpeedControlInit(webs_t wp, char_t *path, char_t *query)
{
	char_t pre_ip[32]={'\0'}, pr_ip[32]={'\0'};
	char_t *lanip, *p, *buf;
	unsigned int index;
	int up = 0, down = 0;
	int up1=0, down1=0;
	int up2 = 0, down2 = 0;
	int  u_kbs=0, d_kbs=0, u_kbs0=0, d_kbs0=0, u_kbs1=0, d_kbs1=0, u_kbs2 =0, d_kbs2 =0;
	int i=0,j=0,temp=0,first_temp=0, mac_temp=0,old_temp[30]={-1};
	char result[2048] = {0};
	char  middle_vlaue[20]={0};
	char  middle_vlaue2[20]={0};
	char  vlaue[32]={0};
	char hostname[64] = {'\0'};
	struct in_addr a;
	unsigned char *mac_look_list;
	unsigned char mac_look_list_down[20]={0};
	int k=0;
	#ifdef __CONFIG_ALINKGW__
	char *blacklist = NULL;
	#endif
	_GET_VALUE(_LAN0_IP, lanip);

	strcpy(pr_ip, lanip);
	p=rindex(pr_ip, '.');
	if(p) *p='\0';


	strncat(result, "{", 1);
	//add by ll
	string_cat(result,"wifi-power", get_product_pwr_info());
	strncat(result, ",", 1);
	//end by ll
//是否开启流量控制
	string_cat(result,"enablecontrol",nvram_safe_get("tc_enable"));
	strncat(result, ",", 1);	

	string_cat(result,"localip",wp->ipaddr);
	strncat(result, ",", 1);	
	
	string_cat(result,"error-info",nvram_safe_get("err_check"));
	strncat(result, ",", 1);	

	
//流量监测表controllist
	strncat(result, "\"controllist\"", strlen("\"controllist\""));
	strncat(result, ":", 1);
	strncat(result, "[", 1);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	websWrite(wp, result); 

	first_temp=d_list_to_web("tc_",wp);
	first_temp=d_list_to_web("remark_",wp);
	first_temp=d_list_to_web("filter_mac",wp);
#ifdef __CONFIG_ALINKGW__	
	if (nvram_match(WLN0_MAC_MODE, "deny")) 
	{
		_GET_VALUE(WLN0_MAC_LIST, blacklist);
	}
#endif
	for(index=0; index< STREAM_CLIENT_NUMBER; ++index)
	{
		sprintf(pre_ip, "%s.%d", pr_ip, index+1);
		temp=0;
		if(!strcmp(pre_ip , lanip))
		{
			continue ;
		}
		mac_look_list = lookmac(inet_addr(pre_ip));
		//从tenda_arp表中根据IP查找MAC
		if(mac_look_list != NULL)
		{
			//mac_look_list_down存放的是查找到的MAC地址
			sprintf(mac_look_list_down,"%02x:%02x:%02x:%02x:%02x:%02x",mac_look_list[0],mac_look_list[1],mac_look_list[2],mac_look_list[3],mac_look_list[4],mac_look_list[5]);
		}
		else
		{
			//没有找到则说明没有该用户
			strcpy(mac_look_list_down,"00:00:00:00:00:00");
			continue;
		}
		#if 1
		if(ismulticast(mac_look_list))
			continue;
		#endif

		#if 0
		if(memcmp(mac_look_list_down,"00:00:00:00:00:00",17) == 0)//如果查找到的MAC为全0是不合法的
			continue;
		#endif
		
		for(k=0;k<strlen(mac_look_list_down);k++)
		{
			//将MAC地址转换成大写
			if(mac_look_list_down[k] >= 'a' && mac_look_list_down[k] <= 'z')
				mac_look_list_down[k] -= 32;
		}
		if(iplookflag(inet_addr(pre_ip)) == 1)
		{
			//通过ioctl查询是否在线			
			if(isWifiClient(mac_look_list_down,pre_ip) ==0)
				continue;
		}
#ifdef __CONFIG_ALINKGW__
		if( blacklist && strstr(blacklist, mac_look_list_down))
		{
			continue;
		}
#endif
		for(j = 0;j<STREAM_CLIENT_NUMBER;j++)
		{
			//查找原来表中是否有记录包括原来就有
			if(stream__ip_iist[j].index == NULL)
				continue;
			if(strncmp(stream__ip_iist[j].index->mac,mac_look_list_down , strlen(stream__ip_iist[j].index->mac)) == 0)
			{
				if(index+1 != j)
				{
					stream__ip_iist[index+1].index = stream__ip_iist[j].index;
					stream__ip_iist[j].index = NULL;
				}
				break;
			}
		}

		if(j == STREAM_CLIENT_NUMBER)//没有相关的记录
		{
			stream__ip_iist[index+1].index = (stream_list_t*)malloc(sizeof(stream_list_t));
			stream__ip_iist[index+1].index->set_pers = 301*128;//301*128为无限制，0表示无法上网
			stream__ip_iist[index+1].index->limit = 301;
			strcpy(stream__ip_iist[index+1].index->remark,"") ;
			strcpy(stream__ip_iist[index+1].index->hostname,"") ;
		}		
		strcpy(hostname,mac_look_hostname(mac_look_list_down));
		if(strcmp(hostname,"") != 0 && strcmp(hostname," ") != 0)
		{				
			strcpy(stream__ip_iist[index+1].index->hostname,hostname) ;
		}
		stream__ip_iist[index+1].index->ip = inet_addr(pre_ip);
		strcpy(stream__ip_iist[index+1].index->mac,mac_look_list_down) ;
		//stream__ip_iist[index+1].index->type = lookflag(mac_look_list);
		stream__ip_iist[index+1].index->type = iplookflag(inet_addr(pre_ip));
		stream__ip_iist[index+1].index->down_byte_pers = 0;

		if(stream_ip[index].index == NULL)
			goto go_web;	

		(ip_ubs(index)!=0) ? (u_kbs0 = ip_ubs(index)/1000) : (u_kbs0=0);
		(ip_dbs(index)!=0) ? (d_kbs0 = ip_dbs(index)/1000) : (d_kbs0=0);
		
		(ip_ubs(index)!=0) ? (u_kbs = ip_ubs(index)%1000) : (u_kbs=0);
		(ip_dbs(index)!=0) ? (d_kbs = ip_dbs(index)%1000) : (d_kbs=0);
				
		(u_kbs!=0) ? (u_kbs1=u_kbs/100) : (u_kbs1=0);
		(d_kbs!=0) ? (d_kbs1=d_kbs/100) : (d_kbs1=0);
		(u_kbs1!=0) ? (u_kbs2=u_kbs1/10) : (u_kbs2=0);
		(d_kbs1!=0) ? (d_kbs2=d_kbs1/10) : (d_kbs2=0);
		
		(ip_uby(index)!=0) ? (up=ip_uby(index)/1000) : (up=0);
		(ip_dby(index)!=0) ? (down=ip_dby(index)/1000) : (down=0);
		
		(up!=0) ? (up1=up/100) : (up1=0);
		(down!=0) ? (down1=down/100) : (down1=0);
		(up1!=0) ? (up2=up1/10) : (up2=0);
		(down1!=0)?(down2=down1/10) : (down2=0);
#if 0
		diag_printf("%s;%d.%d%d;%d.%d%d;%d;%d.%d%d;%d;%d.%d%d\n",
			pre_ip, 
			(u_kbs0), (u_kbs1), (u_kbs2),
			(d_kbs0), (d_kbs1), (d_kbs2),
			ip_upa(index),ip_uM(index), up1, up2,
			ip_dpa(index),ip_dM(index), down1, down2
		);
#endif
		/*ip;uprate;downrate;sendpacket;sendbytes;recvpackets;recvbytes*/

		//加载速率
		sprintf(vlaue,"%d.%d%d",(d_kbs0), (d_kbs1), (d_kbs2));
		#if 0
		if(atof(vlaue) !=0 )
		{
			srand(time((time_t *)NULL));
			int a = rand()%12 +92;
			if(stream__ip_iist[index+1].index->set_pers != 0 && stream__ip_iist[index+1].index->set_pers != 301*128)
			{
				if((int)(atof(vlaue)) < stream__ip_iist[index+1].index->set_pers*0.92)
					stream__ip_iist[index+1].index->down_byte_pers = stream__ip_iist[index+1].index->set_pers*a/100;
				else if((int)(atof(vlaue)) > stream__ip_iist[index+1].index->set_pers*1.08)
					stream__ip_iist[index+1].index->down_byte_pers = stream__ip_iist[index+1].index->set_pers*a/100;
				else
					stream__ip_iist[index+1].index->down_byte_pers = (int)(atof(vlaue));
			}
			else
			{
				stream__ip_iist[index+1].index->down_byte_pers = (int)(atof(vlaue));
			}
		}
		else
		{
			stream__ip_iist[index+1].index->down_byte_pers = 0;
		}
		#endif
		
		stream__ip_iist[index+1].index->down_byte_pers = (int)(atof(vlaue));

go_web:		
		if(first_temp)
		{
			websWrite(wp, ","); 
		}
		strcpy(result,"");
		strncat(result, "{", 1);
		//表
		char tremark[128]={0};
		//hqw add for gbl2312 to utf-8
		char *value = NULL;
		value = stream__ip_iist[index+1].index->hostname;
#ifdef __CONFIG_SUPPORT_GB2312__
		if (1 == is_cn_encode(value))
		{
			char_t mib_value[49] = {0};
			char_t tmp_ssid[49] = {0};
			char_t *tmp_value = tmp_ssid;
			strcpy(mib_value, value);
			set_cn_ssid_encode("utf-8", mib_value, tmp_ssid);
			//printf("Decoded(to %s) ssid:%s\n","utf-8", tmp_ssid);
			value = tmp_value;
		}
#else
		if(1 == is_gb2312_code(value))
		{				
			strcpy(value,"") ;
		}
#endif
		//end
		string_cat(result,"hostname",value);
		strncat(result, ",", 1);

		string_cat(result,"remark",encodeSSID(stream__ip_iist[index+1].index->remark,tremark));
		strncat(result, ",", 1);

		a.s_addr = stream__ip_iist[index+1].index->ip;
		string_cat(result,"ipval",inet_ntoa(a));		
		strncat(result, ",", 1);

		sprintf(vlaue,"%d",stream__ip_iist[index+1].index->type);
		string_cat(result,"linktype",vlaue);
		strncat(result, ",", 1);

		string_cat(result,"macval",stream__ip_iist[index+1].index->mac);		
		strncat(result, ",", 1);

		sprintf(vlaue,"%.02f",((float)stream__ip_iist[index+1].index->down_byte_pers)/128);
		string_cat(result,"downloadspd",vlaue);
		strncat(result, ",", 2048);

		sprintf(vlaue,"%.02f",stream__ip_iist[index+1].index->limit);
		string_cat(result,"spdlimit",vlaue);
		
		strncat(result, "}", 1);
		websWrite(wp, result);  
		first_temp=1;
	}
#ifdef __CONFIG_ALINKGW__
	strcpy(result,"");
	websWrite(wp, "],");
	strncat(result, "\"blacklist\"", strlen("\"blacklist\""));
	strncat(result, ":", 1);
	strncat(result, "[", 1);

	getBlacklist(result);
	strncat(result, "]", 1);
	websWrite(wp, result);  

	websWrite(wp, "}\n");
#else
	websWrite(wp, "]}\n");
#endif
	websDone(wp, 200);
	return ;
}

unsigned int list_ip_loc=0;
unsigned int list_mac_loc=0;
unsigned int list_remark_loc=0;
unsigned int list_ip_num=0;
unsigned int list_mac_num=0;
unsigned int list_remark_num=0;
unsigned int list_lock = 1;

#define foreach(word, wordlist, next,temp,temp1) \
	for (next = &wordlist[strspn(wordlist, temp)], \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, temp)] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, temp1); \
	     strlen(word); \
	     next = next ? &next[strspn(next, temp)] : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[strcspn(word, temp)] = '\0', \
	     word[sizeof(word) - 1] = '\0', \
	     next = strchr(next, temp1))

/*如果替换表的话需要同时清除全局表*/
void unset_list(char *filter_name,int num)
{
	int i = 0;
	char mac[20] = {0},name[20] = {0};

	if(NULL == filter_name || num < 0 || num > 19)
		return;

	sprintf(name,"%s%d",filter_name,num);

	if(strlen(nvram_safe_get(name)) < strlen("00:00:00:00:00:00"))
		return;
	
	strncpy(mac,nvram_safe_get(name),strlen("00:00:00:00:00:00"));

	if(0 == strcmp(mac,"") || strlen(mac) < strlen("00:00:00:00:00:00"))
		return;
	
	for(i = 1;i < STREAM_CLIENT_NUMBER;i++)
	{
		if(stream__ip_iist[i].index == NULL)
			continue;
		
		if(0 == strcmp(stream__ip_iist[i].index->mac,mac))
		{
			strcpy(stream__ip_iist[i].index->remark,"");
			stream__ip_iist[i].index->set_pers = 301*128;
			stream__ip_iist[i].index->limit = 301;
			break;
		}
	}
	
	return;
}

/************************************************************
Function:	 replace			   
Description: 替换掉name表中的相同MAC地址的数据

Input:	   name:表名			   mac:表中的标志MAC地址   
		   ip:IP地址第四段	   remark:备注 			
		   speed:设置的速率 ip_list:IP地址

Output: 

Return: 	   

Others:
History:
<author>   <time>	 <version >   <desc>
hqw 	   2013-10-30	1.0 	   新建函数

************************************************************/

void replace(char *name,char *mac,char *ip,char *remark,int speed,char *ip_list ,char *limit,webs_t wp)
{
	char value[256]={0};
	char *nvram_loc_value;
	char nvram_loc[128]={0};
	int i=0;
	char dhcp_list[50]={0};
	for(i=0;i<20;i++)
	{
		strcmp(value,"");
		sprintf(nvram_loc,"%s%d",name,i);
		_GET_VALUE(nvram_loc,nvram_loc_value);
		if(strncmp(nvram_loc_value,mac,17) == 0)
		{
			if(strcmp(name,"filter_mac") == 0)
				sprintf(value,"%s,0-6,0-0,on,%s",mac,remark);
			else if(strcmp(name,"tc_") == 0)
			{
				sprintf(value,"%s,80,%s,%s,1,%d,%d,1,0,%s,%s",mac,ip,ip,speed,speed,limit,remark);
				sprintf(dhcp_list," ;%s;%s;1;60",ip_list,mac);
				_SET_VALUE(LAN0_DHCP_SATIC(i),dhcp_list);
			}
			else
				sprintf(value,"%s,%s",mac,remark);
			_SET_VALUE(nvram_loc,value);
			return;
		}
	}
	//如果在表中没有找到，则添加进去
	//2015-11-19,dbug by yp,传入最后三个参数,解决闲限制而20个以上的客户端王网速是异常重启问题
	add(name,mac,ip,remark,speed,ip_list,wp,limit);
}


/************************************************************
Function:	 add               
Description:  添加name相对应的数据

Input:     name:表名   		       mac:表中的标志MAC地址   
		 ip:IP地址第四段    remark:备注 			
		 speed:设置的速率ip_list:IP地址

Output: 

Return:        返回给网页表是否满

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

void add(char *name,char *mac,char *ip,char *remark,int speed,char *ip_list,webs_t wp, char *limit)
{
	char value[256]={0};
	char nvram_loc[128]={0};
	char loc[5]={0};
	char num[5]={0};
	int i=0;
	char dhcp_list[50]={0};
	//如果是mac过滤表
	if(strcmp(name,"filter_mac") == 0)
	{
		sprintf(nvram_loc,"%s%d",name,list_mac_loc);
		sprintf(loc,"%d",list_mac_loc);
		_SET_VALUE(_FW_FLT_MAC_CUR_NU,loc+1);
		sprintf(value,"%s,0-6,0-0,on,%s",mac,remark);
		i = list_mac_loc;
		if(20 == list_mac_num)
		{	
			unset_list("filter_mac",list_mac_loc);
		}
		list_mac_loc++;
		list_mac_num++;
		if(list_mac_num == 21)
		{
			list_mac_num--;
			websWrite(wp, "mac_overflow ");
		}
		sprintf(num,"%d",list_mac_num);
		_SET_VALUE("filter_mac_num",num);
		list_mac_loc %= 20;
		sprintf(loc,"%d",list_mac_loc);
		_SET_VALUE("filter_mac_loc",loc);
	}
	//如果是限速表
	else if(strcmp(name,"tc_") == 0)
	{
		sprintf(nvram_loc,"%s%d",name,list_ip_loc);
		sprintf(value,"%s,80,%s,%s,1,%d,%d,1,0,%s,%s",mac,ip,ip,speed,speed,limit,remark);
		sprintf(dhcp_list," ;%s;%s;1;60",ip_list,mac);
		_SET_VALUE(LAN0_DHCP_SATIC(list_ip_loc),dhcp_list);
		i = list_ip_loc;
		if(20 == list_ip_num)
		{	
			unset_list("tc_",list_ip_loc);
		}
		list_ip_loc++;
		list_ip_num++;
		if(list_ip_num == 21)
		{
			list_ip_num--;
			websWrite(wp, "tc_overflow ");
		}
		sprintf(num,"%d",list_ip_num);
		_SET_VALUE("tc_num",num);
		list_ip_loc %= 20;
		sprintf(loc,"%d",list_ip_loc);
		_SET_VALUE("tc_loc",loc);
	}
	//如果是备注表
	else
	{
		sprintf(nvram_loc,"%s%d",name,list_remark_loc);
		sprintf(value,"%s,%s",mac,remark);
		i = list_remark_loc;
		if(20 == list_remark_num)
		{	
			unset_list("remark_",list_remark_loc);
		}
		list_remark_loc++;
		list_remark_num++;
		if(list_remark_num == 21)
		{
			list_remark_num--;
			websWrite(wp, "remark_overflow ");
		}
		sprintf(num,"%d",list_remark_num);
		_SET_VALUE("remark_num",num);
		list_remark_loc %= 20;
		sprintf(loc,"%d",list_remark_loc);
		_SET_VALUE("remark_loc",loc);
	}
	printf("add in %s%d:%s\n",name,i,value);
	_SET_VALUE(nvram_loc,value);
}


/************************************************************
Function:	 del               
Description:  删除name相对应的表中相同mac地址的数据

Input:     name:表名   mac:表中的标志MAC地址                                     

Output: 

Return:        

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

void del(char *name,char *mac)
{
	char arglists[12][128]={0};
	char *nvram_loc_value,*nvram_max_value;
	char nvram_loc[128]={0},nvram_max[128]={0};
	int i = 0;
	char loc[5]={0};
	for(i=0;i<20;i++)
	{	
		sprintf(nvram_loc,"%s%d",name,i);
		_GET_VALUE(nvram_loc,nvram_loc_value);
		bzero(arglists, sizeof(arglists));
		if(strncmp(nvram_loc_value,mac,17) == 0)
		{ 
			if(strcmp(name,"tc_") == 0)//arglists[9]为备注，arglists[6]为限速
			{
				sscanf(nvram_loc_value,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
					arglists[0],arglists[1],arglists[2],arglists[3],arglists[4],arglists[5],arglists[6],arglists[7],arglists[8],arglists[10],arglists[9]);				
				strncpy(arglists[9],rindex(nvram_loc_value,',')+1,strlen(rindex(nvram_loc_value,',')+1));
			}
			else if(strcmp(name,"remark_") == 0)
			{
				sscanf(nvram_loc_value,"%[^,],%s,",
					arglists[0],arglists[1]);				
				strncpy(arglists[1],rindex(nvram_loc_value,',')+1,strlen(rindex(nvram_loc_value,',')+1));
			}
			else//arglists[4]为备注
			{
				sscanf(nvram_loc_value,"%[^,],%[^,],%[^,],%[^,],%s,",
					arglists[0],arglists[1],arglists[2],arglists[3],arglists[4]);				
				strncpy(arglists[4],rindex(nvram_loc_value,',')+1,strlen(rindex(nvram_loc_value,',')+1));
			}
			//如果是mac过滤表
			if(strcmp(name,"filter_mac") == 0)
			{
				sprintf(nvram_max,"filter_mac%d",(list_mac_loc >= 1)?list_mac_loc-1:19);
				_GET_VALUE(nvram_max,nvram_max_value);
				_SET_VALUE(nvram_loc,nvram_max_value);
				_SET_VALUE(nvram_max,"");			
				if(list_mac_loc == 0)
					list_mac_loc = 19;
				else
					list_mac_loc--;
				if(list_mac_num != 0)
					list_mac_num--;
				sprintf(loc,"%d",list_mac_loc);
				_SET_VALUE("filter_mac_loc",loc);
				sprintf(loc,"%d",list_mac_num);
				_SET_VALUE("filter_mac_num",loc);
			}
			//如果是限速表
			else if(strcmp(name,"tc_") == 0)
			{
				sprintf(nvram_max,"tc_%d",(list_ip_loc >= 1)?list_ip_loc-1:19);
				_GET_VALUE(nvram_max,nvram_max_value);
				_SET_VALUE(nvram_loc,nvram_max_value);
				_SET_VALUE(nvram_max,"");
				sprintf(nvram_loc,"dhcp_static_lease%d",i);
				sprintf(nvram_max,"dhcp_static_lease%d",list_ip_loc-1);
				_GET_VALUE(nvram_max,nvram_max_value);
				_SET_VALUE(nvram_loc,nvram_max_value);
				_SET_VALUE(nvram_max,"");
				if(list_ip_loc == 0)
					list_ip_loc = 19;
				else
					list_ip_loc--;
				if(list_ip_num != 0)
					list_ip_num--;
				sprintf(loc,"%d",list_ip_loc);
				_SET_VALUE("tc_loc",loc);
				sprintf(loc,"%d",list_ip_num);
				_SET_VALUE("tc_num",loc);
			}
			//如果是备注表
			else
			{				
				sprintf(nvram_max,"remark_%d",(list_remark_loc >= 1)?list_remark_loc-1:19);
				_GET_VALUE(nvram_max,nvram_max_value);
				_SET_VALUE(nvram_loc,nvram_max_value);
				_SET_VALUE(nvram_max,"");
				if(list_remark_loc == 0)
					list_remark_loc = 19;
				else
					list_remark_loc--;
				if(list_remark_num != 0)
					list_remark_num--;
				sprintf(loc,"%d",list_remark_loc);
				_SET_VALUE("remark_loc",loc);
				sprintf(loc,"%d",list_remark_num);
				_SET_VALUE("remark_num",loc);
			}	
			return;
		}
	}
}

/************************************************************
Function:	 shut_down               
Description:  关闭网速控制功能

Input:                                          

Output: 

Return:        

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/

void shut_down()
{
	char value[256]={0};
	char nvram_mac[128]={0};
	char nvram_mac_value[128]={0};
	char nvram_tc[128]={0};
	char nvram_tc_value[128]={0};
	int i=0;
	char arglists[12][128]={0};
	for(i=0;i<20;i++)
	{
		strcpy(nvram_mac,"");
		strcpy(nvram_mac_value,"");
		strcpy(nvram_tc,"");
		strcpy(nvram_tc_value,"");
		strcpy(value,"");
		bzero(arglists,sizeof(arglists));
		sprintf(nvram_mac,"%s%d","filter_mac",i);
		sprintf(nvram_tc,"%s%d","tc_",i);
		sprintf(nvram_mac_value,"%s",nvram_safe_get(nvram_mac));
		sprintf(nvram_tc_value,"%s",nvram_safe_get(nvram_tc));
		if(strcmp(nvram_mac_value,"") != 0)
		{	
			sscanf(nvram_mac_value,"%[^,],%[^,],%[^,],%[^,],%s,",
				arglists[0],arglists[1],arglists[2],arglists[3],arglists[4]);			
			strncpy(arglists[4],rindex(nvram_mac_value,',')+1,strlen(rindex(nvram_mac_value,',')+1));
			sprintf(value,"%s,0-6,0-0,off,%s",arglists[0],arglists[4]);
			_SET_VALUE(nvram_mac,value);
		}
		if(strcmp(nvram_tc_value,"") != 0)
		{
			sscanf(nvram_tc_value,"%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%s",
				arglists[0],arglists[1],arglists[2],arglists[3],arglists[4],arglists[5],arglists[6],arglists[7],arglists[8],arglists[10],arglists[9]);			
			strncpy(arglists[9],rindex(nvram_tc_value,',')+1,strlen(rindex(nvram_tc_value,',')+1));
			sprintf(value,"%s,%s,%s,%s,%s,%s,%s,0,%s,%s,%s",
				arglists[0],arglists[1],arglists[2],arglists[3],arglists[4],arglists[5],arglists[6],arglists[8],arglists[10],arglists[9]);
			_SET_VALUE(nvram_tc,value);
		}
	}
}
#ifdef __CONFIG_ALINKGW__
extern void wl_blacklist();
extern int wl_deauth_sta(char *wl, char *mac);
static int  saveBlacklist(char *blacklist)
{
	char_t *oldblacklist, *wlunit;
	if(blacklist == NULL)
		return 0;
	_GET_VALUE(WLN0_MAC_LIST,oldblacklist);
	if(strcmp(oldblacklist, blacklist) == 0)
		return 0;
	
	if(strlen(blacklist) < strlen("00:11:22:33:44:55"))
	{
		_SET_VALUE(WLN0_MAC_MODE, "disabled");
		_SET_VALUE(WLN0_MAC_LIST, "");
	}
	else
	{
		_SET_VALUE(WLN0_MAC_MODE, "deny");
		_SET_VALUE(WLN0_MAC_LIST, blacklist);
	}
	_GET_VALUE(WLN0_UNIT,wlunit);
	wl_unit(wlunit,1);
	return 1;
}

static int deauth_blacklist(char *blacklist)
{
	char value[32]={0};
	char *next;
	int wl_is_ap = 1;
	
	if(blacklist == NULL)
		return 0;

	if(0 != strcmp(nvram_safe_get("wl0_mode"),"ap"))
	{
		wl_is_ap = 0;
	}
	
	foreach(value, blacklist, next, " ", ' ')
	{
		//如果是AP模式，就要断开eth1接口的连接，但是如果是wisp或者apclient模式下则需要断开的是wl0.1接口的，否则就会导致WISP或者APCLIENT掉线
		if (wl_is_ap)
		{
			wl_deauth_sta("eth1", value);
		}
		else
		{
			wl_deauth_sta("wl0.1", value);
		}
	}
	return 1;
}
#endif
/************************************************************
Function:	 fromSpeedControlSave               
Description:   网速控制的后台接口，设置限速，备注以及MAC地址过滤

Input:                                          

Output: 

Return:         返回给前台是否处理完

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-10-30   1.0        新建函数

************************************************************/
static void fromSpeedControlSave(webs_t wp, char_t *path, char_t *query)
{
	char_t  *isp_uprate=NULL, *isp_downrate=NULL, *tc_list=NULL,*tc_enable=NULL; 
	int j=0,sys_reboot_ = 0;
	int set_speed = 0;
	int try_num = 0;

	char value[256]={0};
	char value1[25]={0};
	char *list,*next;
	char list_part[3][128]={0};
	char ip_part[4][4]={0};
	struct in_addr a;
	
	int sys_restart_tag = 0, tc_reboot = 0;

	list = websGetVar(wp, T("list"), T(""));
	isp_uprate = websGetVar(wp, T("up_Band"), T("12800"));
	isp_downrate = websGetVar(wp, T("down_Band"), T("12800"));
	tc_enable = websGetVar(wp, T("enablecontrol"), T("0"));//是否开启
#ifdef __CONFIG_ALINKGW__
	char_t  *blacklist = websGetVar(wp, T("blacklist"), T(""));
	char_t  *moveBlackList = websGetVar(wp, T("moveBlackList"), T(""));
	int wl_restart = saveBlacklist(blacklist);	
#endif
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	
	
	
	_SET_VALUE(TC_ISP_UPRATE, isp_uprate);
	_SET_VALUE(TC_ISP_DOWNRATE, isp_downrate);
	_SET_VALUE(TC_ENABLE, tc_enable);
	if(strcmp(tc_enable,"0") == 0)
		_SET_VALUE(_FW_FLT_MAC_EN,"disable");
	else
		_SET_VALUE(_FW_FLT_MAC_EN,"deny");
	if(strcmp(tc_enable,"0") == 0)
	{
		shut_down();
		new_init_stream_control();
	}
#ifdef __CONFIG_ALINKGW__	
	foreach(value, moveBlackList, next, " ", ' ')
	{
		del("filter_mac",value);
		wl_restart = 1;
	}
#endif

	/*最多等待5秒钟，如果5秒钟代码还未执行完，则强行退出不保存*/
	while(0 == list_lock && try_num <= 50)
	{
		cyg_thread_delay(10);
		try_num++;
	}
	
	if(list_lock)
	{
		list_lock=0;

		/*获取到锁之后才去取全局变量，避免数据错乱*/
		list_ip_loc = atoi(nvram_safe_get("tc_loc"))%20;
		list_mac_loc = atoi(nvram_safe_get("filter_mac_loc"))%20;
		list_remark_loc = atoi(nvram_safe_get("remark_loc"))%20;
		list_ip_num = atoi(nvram_safe_get("tc_num"))%20;
		list_mac_num = atoi(nvram_safe_get("filter_mac_num"))%20;
		list_remark_num = atoi(nvram_safe_get("remark_num"))%20;
		
		foreach(value,list,next,";",';')
		{	
			bzero(list_part, sizeof(list_part));
			sscanf(value,"%[^,],%[^,],%s",list_part[0],list_part[1],list_part[2]);
			//第一个为MAC地址	
			//第二个为限速
			//第三个为备注
			strncpy(list_part[2],rindex(value,',')+1,strlen(rindex(value,',')+1));
			set_speed = (atof(list_part[1])*128);
			if(set_speed < (atof(list_part[1])*128))
			{
				set_speed++;
			}
			for(j = 0;j<STREAM_CLIENT_NUMBER;j++)
			{
				if(stream__ip_iist[j].index == NULL)
					continue;
				sprintf(value1,"%s",stream__ip_iist[j].index->mac);
				if(strcmp(value1,list_part[0]) != 0)
					continue;
				a.s_addr = stream__ip_iist[j].index->ip;
				bzero(ip_part, sizeof(ip_part));
				sscanf(inet_ntoa(a),"%[^.].%[^.].%[^.].%s",ip_part[0],ip_part[1],ip_part[2],ip_part[3]);
				if(atof(list_part[1]) == 301)
				{
					//开始本身就是不限速
					if(stream__ip_iist[j].index->set_pers == 301*128)
					{	
						if(strcmp(list_part[2],"") == 0)
						{
							//如果是删除备注，则从备注表中删除
							del("remark_",list_part[0]);
						}
						else
						{
							//如果是更新备注的话进这里
							replace("remark_",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a), list_part[1],wp);
						}
					}
					else
					{
						//如果之前是MAC地址过滤则将MAC地址过滤规则链中删除
						if(stream__ip_iist[j].index->set_pers == 0)
						{			
							del("filter_mac",list_part[0]);
							sys_restart_tag = 1;
						}
						//如果之前是限速则在限速中删除，并且将调用更新限速规则的标志位置1
						if(stream__ip_iist[j].index->set_pers > 0 && stream__ip_iist[j].index->set_pers <= 300*128)
						{
							del("tc_",list_part[0]);
							sys_reboot_ = 1;
							tc_reboot = 1;
							sys_restart_tag = 1;
						}
						//添加到备注表中
						if(strcmp(list_part[2],"") != 0)
							add("remark_",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a),wp,list_part[1]);
					}
					strcpy(stream__ip_iist[j].index->remark,list_part[2]) ;	
					stream__ip_iist[j].index->set_pers = 301*128;
					stream__ip_iist[j].index->limit = 301;
				}
				//如果设置为禁止上网
				else if(atof(list_part[1]) == 0)
				{
					//如果本身就是禁止上网，则替换mac过滤表中的数据
					if(stream__ip_iist[j].index->set_pers == 0)
					{	
						replace("filter_mac",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a), list_part[1],wp);
					}
					else
					{
						//如果开始是无限制则删除该表项中的数据
						if(stream__ip_iist[j].index->set_pers == 301*128)
						{
							del("remark_",list_part[0]);
						}
						//如果开始是限速则删除该表项中的数据
						if(stream__ip_iist[j].index->set_pers > 0 && stream__ip_iist[j].index->set_pers <= 300*128)
						{
							del("tc_",list_part[0]);
							sys_reboot_ = 1;
							tc_reboot = 1;
						}
						//添加到mac地址过滤中
						add("filter_mac",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a),wp,list_part[1]);
					}
				
					//00:11:22:33:44:55,0-6星期,0-0时间,on是否开启,诠释
					
					//00:11:22:33:44:55,1-2,11100-29400,on,1	
					sys_restart_tag = 1;
					strcpy(stream__ip_iist[j].index->remark,list_part[2]) ;	
					stream__ip_iist[j].index->set_pers = 0;
					stream__ip_iist[j].index->limit = 0;
				}
				//如果设置为限速
				else
				{
					//如果之前本身就是限速，则替换限速表中的数据
					if(stream__ip_iist[j].index->set_pers != 301*128 && stream__ip_iist[j].index->set_pers != 0)
					{	
						replace("tc_",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a), list_part[1],wp);
					}
					else
					{
						//如果之前本身是禁止上网，则删除mac过滤表项中的数据
						if(stream__ip_iist[j].index->set_pers == 0)
						{
							del("filter_mac",list_part[0]);
						}
						//如果之前本身是无限速，则删除mac过滤表项中的数据
						if(stream__ip_iist[j].index->set_pers == 301*128)
						{
							del("remark_",list_part[0]);
						}
						//添加到限速表中
						add("tc_",list_part[0],ip_part[3],list_part[2],set_speed,inet_ntoa(a),wp,list_part[1]);
					}
					tc_reboot = 1;
					sys_reboot_ = 1;
					sys_restart_tag = 1;
					strcpy(stream__ip_iist[j].index->remark,list_part[2]) ;	
					stream__ip_iist[j].index->set_pers = (int)(atof(list_part[1])*128);
					stream__ip_iist[j].index->limit = atof(list_part[1]);
				}
			}
		}

		if(sys_reboot_ == 1)
		{
			new_init_stream_control();
		}
		if(tc_reboot == 1)
		{
			printf("restart dhcp!\n");
			dhcpd_stop();
			dhcpd_start();
		}
		
		list_lock = 1;
		_COMMIT();
		#if 0
		if(sys_restart_tag == 1)
		{
			_RESTART_ALL();
		}
		#endif
		#if 1
		update_firewall();
		#endif
#ifdef __CONFIG_ALINKGW__
		if( wl_restart )
		{
			wl_blacklist();
			deauth_blacklist(blacklist);
		}
#endif			
		websWrite(wp, "1");
		websDone(wp, 200);	
	}
	return ;
}

/**************************************************************/
#endif/*__CONFIG_STREAM_STATISTIC__*/
#endif /*__CONFIG_TC__*/
