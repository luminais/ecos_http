#include <string.h>
#include <stdlib.h>
#include <router_net.h>
#include "flash_cgi.h"
#include "webs.h"
#include "cJSON.h"
#include "cgi_common.h"
#include "sys_module.h"
#include "http.h"
#include "bcmsntp.h"
#include "cgi_lib.h"
/*
	for remote web
int	restart_wan;
int get_restart_wan()
{
	return restart_wan;
}
*/
	struct tz_info
	{
		int index;
		char *time_zone;
		char *tz_str;
		int tz_offset;
	};
	
	static struct tz_info time_zones[] =
	{
		{0, 		"GMT-12:00",	"WPT+12:00" ,-12*3600}, 	   
		{1, 		"GMT-11:00",	"WPT+11:00" ,-11*3600}, 	 
		{2, 		"GMT-10:00",	"WPT+10:00" ,-10*3600}, 	 
		{3, 		"GMT-09:00",	"WPT+09:00" ,-9*3600},		 
		{4, 		"GMT-08:00",	"WPT+08:00" ,-8*3600},		 
		{5, 		"GMT-07:00",	"WPT+07:00" ,-7*3600},		 
		{6, 		"GMT-07:00",	"WPT+07:00" ,-7*3600},		 
		{7, 		"GMT-07:00",	"WPT+07:00" ,-7*3600},		 
		{8, 		"GMT-06:00",	"WPT+06:00" ,-6*3600},		 
		{9, 		"GMT-06:00",	"WPT+06:00" ,-6*3600},		 
		{10,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},	   
		{11,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},	   
		{12,   "GMT-05:00" , "WPT+05:00"  ,-5*3600},	   
		{13,   "GMT-04:30" , "WPT+04:30"  ,-4*3600-30*60},		 
		{14,   "GMT-04:00" , "WPT+04:00"  ,-4*3600},	   
		{15,   "GMT-04:00" , "WPT+04:00"  ,-4*3600},	   
		{16,   "GMT-03:30" , "WPT+03:30"  ,-3*3600-30*60},	  
		{17,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},	   
		{18,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},	   
		{19,   "GMT-03:00" , "WPT+03:00"  ,-3*3600},	   
		{20,   "GMT-02:00" , "WPT+02:00"  ,-2*3600},	   
		{21,   "GMT-01:00" , "WPT+01:00"  ,-1*3600},	   
		{22,   "GMT-01:00" , "WPT+01:00"  ,-1*3600},	   
		{23,   "GMT+00:00" , "WPT-00:00"  ,+0*3600},	   
		{24,   "GMT+00:00" , "WPT-00:00"  ,+0*3600},	   
		{25,   "GMT+01:00" , "WPT-01:00"	,+1*3600},		 
		{26,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},	   
		{27,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},	   
		{28,   "GMT+01:00" , "WPT-01:00"  ,+1*3600},	   
		{29,   "GMT+01:00" , "WPT-01:00"	,+1*3600},		 
		{30,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{31,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{32,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{33,   "GMT+02:00" , "WPT-02:00"	,+2*3600},		 
		{34,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{35,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{36,   "GMT+02:00" , "WPT-02:00"  ,+2*3600},	   
		{37,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},	   
		{38,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},	   
		{39,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},	   
		{40,   "GMT+03:00" , "WPT-03:00"  ,+3*3600},	   
		{41,   "GMT+03:30" , "WPT-03:30"  ,+3*3600+30*60},	  
		{42,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},	   
		{43,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},	   
		{44,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},	   
		{45,   "GMT+04:00" , "WPT-04:00"  ,+4*3600},	   
		{46,   "GMT+04:30" , "WPT-04:30"  ,+4*3600+30*60},	  
		{47,   "GMT+05:00" , "WPT-05:00"  ,+5*3600},	   
		{48,   "GMT+05:00" , "WPT-05:00"  ,+5*3600},	   
		{49,   "GMT+05:30" , "WPT-05:30"  ,+5*3600+30*60},	  
		{50,   "GMT+05:45" , "WPT-05:45"  ,+5*3600+45*60},	  
		{51,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},	   
		{52,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},	   
		{53,   "GMT+06:00" , "WPT-06:00"  ,+6*3600},	   
		{54,   "GMT+06:30" , "WPT-06:30"  ,+6*3600+30*60},	  
		{55,   "GMT+07:00" , "WPT-07:00"  ,+7*3600},	   
		{56,   "GMT+07:00" , "WPT-07:00"  ,+7*3600},	   
		{57,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},	   
		{58,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},	   
		{59,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},	   
		{60,   "GMT+08:00" , "WPT-08:00"  ,+8*3600},	   
		{61,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},	   
		{62,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},	   
		{63,   "GMT+09:00" , "WPT-09:00"  ,+9*3600},	   
		{64,   "GMT+09:30" , "WPT-09:30"  ,+9*3600+30*60},	  
		{65,   "GMT+09:30" , "WPT-09:30"  ,+9*3600+30*60},	  
		{66,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},	   
		{67,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},	   
		{68,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},	   
		{69,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},	   
		{70,   "GMT+10:00" , "WPT-10:00"  ,+10*3600},	   
		{71,   "GMT+11:00" , "WPT-11:00"  ,+11*3600},	   
		{72,   "GMT+12:00" , "WPT-12:00"  ,+12*3600},	   
		{73,   "GMT+12:00" , "WPT-12:00"  ,+12*3600},	   
		{74,   "GMT+13:00" , "WPT-13:00"  ,+13*3600}	   
	};
#define TIME_ZONES_NUMBER 75


RET_INFO cgi_sysmanage_systime_get(webs_t wp, cJSON *root, void *info)
{


	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 

	{
		MODULE_GET_SYSTIME, 
	};
	cJSON_AddItemToObject(root, "sysTime", obj = cJSON_CreateObject());
	
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}

RET_INFO cgi_sysmanage_systime_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info)
{

	

	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
	PIU8 modules[] = 
	
	
	{
		MODULE_SET_SYSTIME,		
	};

	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);
	return RET_SUC;
}

//快速设置里时间的设置，与上面的系统时间设置分开  add duanjingcheng@tenda.com
RET_INFO cgi_wizard_systime_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;	
	PIU8 modules[] = 
	{		
		MODULE_SET_WIZARD_SYSTIME,	
	};
	set_info.wp = wp;	
	set_info.root = NULL;	
	set_info.modules = modules;	
	set_info.module_num = ARRAY_SIZE(modules);	
	cgi_lib_set(set_info,msg,err_code,NULL);
	return(RET_SUC);
}