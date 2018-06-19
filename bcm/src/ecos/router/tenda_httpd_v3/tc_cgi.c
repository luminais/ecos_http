#ifdef __CONFIG_TC__
#include <stdio.h>
#include "route_cfg.h"
#include "flash_cgi.h"
#include "webs.h"
#include "uemf.h"

#ifdef __CONFIG_STREAM_STATISTIC__
#include "../tc/tc.h"
#endif

#define TC_RULE_NUMBER_MAX 10 
static void fromtc(webs_t wp, char_t *path, char_t *query);
static int get_tc_list(int eid, webs_t wp, int argc, char_t **argv);
static int get_tc_othe(int eid, webs_t wp, int argc, char_t **argv);

#ifdef __CONFIG_STREAM_STATISTIC__
int g_user_stream_statistic=0;
extern statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];
static void get_stream_stat_list(webs_t wp, char_t *path, char_t *query);
static void enable_stream_stat(webs_t wp, char_t *path, char_t *query);
static int get_stream_stat_en(int eid, webs_t wp, int argc, char_t **argv);
#endif/*__CONFIG_STREAM_STATISTIC__*/

void tc_form(void)
{
	websFormDefine(T("trafficForm"), fromtc);
#ifdef __CONFIG_STREAM_STATISTIC__
	websFormDefine(T("updateIptAccount"), get_stream_stat_list);
	websFormDefine(T("iptAcount_mng"), enable_stream_stat);
#endif/*__CONFIG_STREAM_STATISTIC__*/
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
	websWrite(wp, T("%d"), atoi(value));

	return 0;
}
#endif/*__CONFIG_STREAM_STATISTIC__*/
#endif /*__CONFIG_TC__*/
