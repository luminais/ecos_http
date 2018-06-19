#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>

#include "webs.h"
#include "flash_cgi.h"
#include "route_cfg.h"
#include "wsIntrn.h"

extern int wl_restart_check_main_loop();
extern char* get_product_pwr_info();


void changeWeekForm(char *in_week, char *out_week)
{
	char *ptr;
	if(in_week == NULL ||out_week == NULL )
		return;
	ptr = in_week; 
	while( *ptr!= '\0')
	{
		switch(*ptr)
		{	
			case ',':
				break;
			case '-':
				break;
			case '8':
			case '9':
			case '0':
				printf("illegal character %c\n",*ptr);
				return 0;	
			default:
				if(*ptr >= '1' && *ptr <= '7')//星期天到星期六
				{
					out_week[atoi(ptr) - 1] = '1';
				}
				break;
		}
		ptr++;
	}
	return;
}


void formWifiControlInit(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	int zone_hh,zone_mm;
	int sec,min,hour;
	char week[16]={0};
	int on_time = 0, off_time = 0;
	char week_tmp[8] ={"0000000"}; 

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));

	websWrite(wp, "{");
	websWrite(wp, "\"wifi-power\":\"%s\",", get_product_pwr_info());
	websWrite(wp, "\"enable\":\"%s\",", nvram_safe_get(WLN0_CTL_ENABLE));
	
	_GET_VALUE("alilink_wlan_ontime_list1", tmp_value);
	if(tmp_value)
	{
		if(strchr(tmp_value,'+'))
		{
			sscanf(tmp_value,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
			on_time = hour*60*60 + min*60 + sec;
		}
		
	}

	_GET_VALUE("alilink_wlan_offtime_list1", tmp_value);
	if(tmp_value)
	{
		if(strchr(tmp_value,'+'))
		{
			sscanf(tmp_value,"UTC+%d:%d %d %d %d ? * %s",&zone_hh,&zone_mm,&sec,&min,&hour,week);
			off_time = hour*60*60 + min*60 + sec;
		}
		
	}
	if(on_time >= off_time)
	{
		websWrite(wp, "\"time-interval\":\"%d,%d\",",off_time,on_time);
	}
	else
	{
		websWrite(wp, "\"time-interval\":\"%d,86400;0,%d\",",off_time,on_time);
	}
	changeWeekForm(week , week_tmp);
	websWrite(wp, "\"time-round\":\"%s\",",week_tmp);
	websWrite(wp, "\"wanMode\":\"%s\"", nvram_safe_get("wl0_mode"));
	websWrite(wp, "}");
	
	websDone(wp, 200);

	return;
}

#if 0
void formWifiControlInit(webs_t wp, char_t *path, char_t *query)
{
	char *tmp_value;
	char result[2048] = {0};

	strncat(result, "{", 1);

	//add by ll
	string_cat(result,"wifi-power", get_product_pwr_info());
	strncat(result, ",", 1);
	//end by ll

	_GET_VALUE(WLN0_CTL_ENABLE, tmp_value);

	string_cat(result, "enable", tmp_value);
	strncat(result, ",", 1);	
	
	_GET_VALUE(WLN0_CTL_DAY_ROUND, tmp_value);

	string_cat(result, "time-round", tmp_value);
	strncat(result, ",", 1);	

	_GET_VALUE(WLN0_CTL_TIME_INTERVAL, tmp_value);

	string_cat(result, "time-interval", tmp_value);
	
	//add liuchengchi 2014-11-12
	strncat(result, ",", 1);
	//?T???￡ê?
	
	//?￡ê?
	string_cat(result,"wanMode",nvram_safe_get("wl0_mode"));
	//endadd
	
	strncat(result, "}", 1);

	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	
	websWrite(wp, result);	
	
	websDone(wp, 200);

	return;
}
#endif


void changeWeekForm2(char *in_week, char *out_week)
{
	char *ptr;
	if(in_week == NULL ||out_week == NULL )
		return;
	int n = 0;
	ptr = in_week; 
	while( *ptr!= '\0')
	{
		switch(*ptr)
		{	
			case ',':
				n ++;
				break;
			case '-':
				break;
			case '8':
			case '9':
			case '0':
				printf("illegal character %c\n",*ptr);
				return 0;
			case '7':
				out_week[n] = '1';
				n ++;
			default:
				if(*ptr >= '1' && *ptr <= '6')//星期天到星期六
				{
				
					out_week[n] = *ptr + 1;
					n ++;
				}
				
				break;
		}
		ptr++;
	}	

	int i,j;
	char temp; 

	for(i=0; i<strlen(out_week) ;  i+=2) 
	{
		for(j=i+2; j<strlen(out_week);  j+=2)
		{
			if(out_week[i]>out_week[j]) 
			{ 
				temp=out_week[i]; 
				out_week[i]=out_week[j]; 
				out_week[j]=temp; 
			} 
		}
	}
	return;
}

extern int gWifiStatusConfig;
extern int g_cur_wl_radio_status;
#define WL_RADIO_ON		1
#define WL_RADIO_OFF		0

void formWifiControlSet(webs_t wp, char_t *path, char_t *query)
{
	char *wlctl_enable, *time_round, *time_week, *time_interval,*sche_cnt;
	char time[4][16] = {0};
	int off_time,off_sec,off_min,off_hour;
	int on_time,on_sec,on_min,on_hour;
	char week_new[16] = {0};;
	char time_str[64] = {0};
	int flag = 0;
	wlctl_enable = websGetVar(wp, T("enable"), T("0"));
	time_round = websGetVar(wp, T("time-round"), T("1111111"));//everyday defaultly
	time_interval = websGetVar(wp, T("time-interval"), T(""));
	time_week = websGetVar(wp, T("time-str"), T("1,2,3,4,5,6,7"));
	
	_SET_VALUE(WLN0_CTL_ENABLE, wlctl_enable);
	
	_GET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, sche_cnt);
	if (atoi(sche_cnt) == 0)
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "1");
	}
	
	if (atoi(wlctl_enable))
	{
		_SET_VALUE(WLN0_CTL_DAY_ROUND, time_round);
		_SET_VALUE(WLN0_CTL_TIME_INTERVAL, time_interval);
		if(!strstr(time_interval, ";"))
		{
			sscanf(time_interval, "%[^,],%s", time[0], time[1]);
			off_time = atoi( time[0]);
			off_hour = off_time /3600;
			off_min = (off_time - 3600*off_hour)/60;
			off_sec = 0;

			on_time = atoi( time[1]);
			on_hour = on_time /3600;
			on_min = (on_time - 3600*on_hour)/60;
			on_sec = 0;
			
			
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);

			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,time_week);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
			
		}
		else
		{
			sscanf(time_interval, "%[^,],%[^;];%[^,],%s", time[0], time[1], time[2], time[3]);
			off_time = atoi( time[0]);
			off_hour = off_time /3600;
			off_min = (off_time - 3600*off_hour)/60;
			off_sec = 0;

			on_time = atoi( time[3]);
			on_hour = on_time /3600;
			on_min = (on_time - 3600*on_hour)/60;
			on_sec = 0;
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", off_sec, off_min, off_hour,time_week);
			_SET_VALUE("alilink_wlan_offtime_list1", time_str);

			strcpy(week_new, time_week);
			changeWeekForm2(time_week, week_new);
			memset(time_str , 0 ,sizeof(time_str));
			sprintf(time_str, "UTC+08:00 %d %d %d ? * %s", on_sec, on_min, on_hour,week_new);
			_SET_VALUE("alilink_wlan_ontime_list1", time_str);
		}
		
	}
	else
	{
		_SET_VALUE(ALILINK_WLAN_SCHE_LIST_NUM, "0");
	#if 0
		_GET_VALUE(WLN0_WIRELESS_ENABLE, enable_val);	
		if(atoi(enable_val) == 0)
		{
			_SET_VALUE(WLN0_WIRELESS_ENABLE, "1");
		}
	#endif	
	}
	_COMMIT();	
	
	websWrite(wp, "1");
	
	websDone(wp, 200);	
	
	gWifiStatusConfig = 0;
	wl_restart_check_main_loop();

}

#if 0
void formWifiControlSet(webs_t wp, char_t *path, char_t *query)
{
	char *wlctl_enable, *time_round, *time_interval;

	wlctl_enable = websGetVar(wp, T("enable"), T("0"));
	time_round = websGetVar(wp, T("time-round"), T("1111111"));//everyday defaultly
	time_interval = websGetVar(wp, T("time-interval"), T(""));

	_SET_VALUE(WLN0_CTL_ENABLE, wlctl_enable);

	if (atoi(wlctl_enable))
	{
		_SET_VALUE(WLN0_CTL_DAY_ROUND, time_round);
		_SET_VALUE(WLN0_CTL_TIME_INTERVAL, time_interval);
	}
	
	_COMMIT();	

	websWrite(wp, "1");

	websDone(wp, 200);

	wl_restart_check_main_loop();
}
#endif
void wirelessCtlAspDefine()
{

}

void wirelessCtlGoformDefine()
{
	websFormDefine(T("GetWifiControl"),formWifiControlInit);	
	websFormDefine(T("SetWifiControl"), formWifiControlSet);
}

void wirelessCtlAspGoformDefine()
{
	wirelessCtlAspDefine();
	
	wirelessCtlGoformDefine();
}

