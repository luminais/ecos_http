#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <router_net.h>

#include "flash_cgi.h"
#include "webs.h"
#include "cJSON.h"
#include "cgi_common.h"
#include "sys_module.h"
#include "http.h"

#include "cgi_lib.h"
#define PUBLIC_FOLDER "/public"

static int g_user_pass_Flag = 0;
char g_User[64] = {0},g_Pass[64] = {0};
const char cookie_suffix[20][4] = 
{"1qw","ded","ert","fcv","vgf","nrg","xcb","jku","cvd","wdv","2dw","njk","ftb","efv","azx","tuo","cvb","bcx","eee","mfw"};

extern int dns_redirect_disable;
extern int dns_redirect_web;

int quit_redirect_web(webs_t wp,char *url)
{
#ifdef __CONFIG_MOBILE_WEB__
	int phone_flag = isPhone(wp);
#endif
	
	if(NULL == url)
	{
		return -1;
	}
		
	if(1 == nvram_match(SYSCONFIG_QUICK_SET_ENABLE,"1"))
	{
	#ifdef __CONFIG_A9__
		if(0 == strcmp(url,"/goform/getStatusBeforeBridge") || 0 == strcmp(url, "/goform/getRebootStatus"))
		{
			return 0;
		}
	#endif
	
	#ifdef __CONFIG_MOBILE_WEB__
		if(phone_flag== 1)
		{
			if(0 != strcmp(url,"/phone/scan.html") && 0 != strcmp(url,"/goform/getWizard") && 0 != strcmp(url,"/goform/setWizard")
				&& 0 != strcmp(url,"/goform/setWifiRelay")&&0 != strcmp(url,"/phone/login.html")&&0 != strcmp(url,"/login/Auth"))
			{
				websRedirect(wp, T("/phone/scan.html")); 
				return 1;
			}
		}
		else
	#endif
		if(0 != strcmp(url,"/quickset.html") && 0 != strcmp(url,"/goform/getWizard") 
		&& 0 != strcmp(url,"/goform/setWizard") && 0 != strcmp(url,"/login.html")
		&& 0 != strcmp(url,"/login/Auth"))
		{
			websRedirect(wp, T("/quickset.html")); 
			return 1;
		}
	}
	else
	{	
	#ifdef __CONFIG_MOBILE_WEB__
		if(phone_flag== 1)
		{
			if(0 == strcmp(url,"/scan.html"))
			{
				websRedirect(wp, T("/scan.html")); 
				return 1;
			}
		}
		else
	#endif
		if(0 == strcmp(url,"/quickset.html"))
		{
			websRedirect(wp, T("/"));
			return 1;
		}
	}	
	
	return 0;
}

#ifdef __CONFIG_MOBILE_WEB__
/*check web request is from mobile phone*/
int isPhone(webs_t wp)
{		
	if(wp->userAgent == NULL)		
		return -1;	
	//printf("func = %s, wp->userAgent = %s\n", __FUNCTION__, wp->userAgent); 

	if(strstr(wp->userAgent, "Android") || strstr(wp->userAgent, "iPhone") ||		
		strstr(wp->userAgent, "iPad") || strstr(wp->userAgent, "Windows Phone") || 		
		strstr(wp->userAgent, "MQQBrowser") || strstr(wp->userAgent, "iPod") || 		
		strstr(wp->userAgent, "NOKIA") || strstr(wp->userAgent, "Mobile") || 		
		strstr(wp->userAgent, "Symbian")) 	
	{ 
	/*lq 添加手机端英文自适应*/
		if(wp->accLang == NULL)
		{
			return 0;
		}
		if(strncmp(wp->accLang, "zh", 2) != 0)
		{
			return  0;
		}
		return 1; 	
	}		

	return 0;
}
#endif

void fromLoginOut (webs_t wp, char_t *path, char_t *query)
{

	int i = 0;

	if(wp != NULL)
	{
		for(i=0; i<MAX_USER_NUM; i++){
			if(!strcmp(loginUserInfo[i].ip, wp->ipaddr)){
				memset(loginUserInfo[i].ip, 0x0, PI_IP_STRING_LEN);
				loginUserInfo[i].time = 0;
				break;
			}	
		}
	}
	
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n")); 
	websWrite(wp, T("%s"), "{\"errCode\":\"0\"}"); 
	websDone(wp, 200);
	return ;
}

/***********web login check**********/
int TWL300NWebsSecurityByCookieHandler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						char_t *url, char_t *path, char_t *query)
{
	char *userid = NULL, *password = NULL;
	char *pfiletype = NULL;
	char *purlbuf = NULL;
	char *pcookie = NULL;
	int i = 0, k = 0;
	
	char *p_user_cookie,*p_cookie_tmp, *value;
	char urlbuf[256] = {0};
	char lan_ip[40]={0};
	char wan_ip[40]={0};
	char default_Pwd[64];

	char lan_wlpwd[64]={0}, wan_wlpwd[64]={0};	
	char lan_index[64]={0}, wan_index[64]={0};

	char lan_quickset[64]={0};
	char lan_redirect[64]={0};

	char lan_login[64]={0};
	char wan_login[64]={0}; 
	char lan_login_error[64]={0};
	char wan_login_error[64]={0};
	char login_succes[64]={0}, login_error[64]={0}; 

	char http_true_passwd[256] = {0};

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path);

	if(strncmp(url,PUBLIC_FOLDER,strlen(PUBLIC_FOLDER))==0
		|| strcmp(url,"/favicon.ico")==0 || wp->host_str == NULL) 
		return 0;

	if(g_user_pass_Flag == 0)
	{
		memset(g_User,0,sizeof(g_User));
		memset(g_Pass,0,sizeof(g_Pass));

		strncpy(g_User,_SAFE_GET_VALUE(LAN_HTTP_LOGIN_USERNAME,value),sizeof(g_User));
		strncpy(g_Pass,_SAFE_GET_VALUE(LAN_HTTP_LOGIN_PASSWD,value),sizeof(g_Pass));

		g_user_pass_Flag = 1;
	}
	strncpy(default_Pwd,_SAFE_GET_VALUE(LAN_HTTP_FAC_PW,value),sizeof(default_Pwd));
	
	memcpy(urlbuf, url, 255);	

	if ((purlbuf = strchr(urlbuf, '?')) != NULL)
		*purlbuf = '\0';

	if(wp->cookie)
	{
		p_user_cookie = wp->cookie;
		while ((p_cookie_tmp = strstr(p_user_cookie,"ecos_pw="))!=NULL)
		{
			pcookie = p_cookie_tmp + strlen("ecos_pw=");
			p_user_cookie = pcookie;
		}
	}

	if (strlen(urlbuf)>3)
	{
		pfiletype = strrchr(urlbuf, '.');
		if (pfiletype)
		{
			pfiletype++;
			if (!memcmp(pfiletype, "gif", 3)||
				!memcmp(pfiletype, "js", 2)||
				!memcmp(pfiletype, "css", 3)||
				!memcmp(pfiletype, "png", 3)||
				!memcmp(pfiletype, "jpg", 3)||
				!memcmp(pfiletype, "xml", 3)||
				!memcmp(pfiletype, "ico", 3)||
				//以下四种类型ttf、woff、eot、svg为字体文件后缀
				!memcmp(pfiletype, "ttf", 3)||
				!memcmp(pfiletype, "woff", 4)||
				!memcmp(pfiletype, "eot", 3)||
				!memcmp(pfiletype, "svg", 3)
				)
				return 0;
		}
	}

	//根据coverity分析结果修改，原来为无效的判断:g_Pass!=NULL(已去掉)  2017/1/11 F9项目修改
	if ((strcmp(g_Pass,default_Pwd) == 0) && wp->url!=NULL && (strstr(wp->url,"/goform/ate")!=NULL) )
		 return 0;

	strncpy(lan_ip,wp->host_str,strlen(wp->host_str));

#ifdef __CONFIG_MOBILE_WEB__
	int phone_flag = isPhone(wp);
	if(phone_flag== 1)
	{
		snprintf(lan_login,sizeof(lan_login),"http://%s/phone/login.html",lan_ip); 
		snprintf(wan_login,sizeof(wan_login),"http://%s/phone/login.html",wp->host_str);
		
		snprintf(lan_login_error,sizeof(lan_login_error),"http://%s/phone/login.html?0",lan_ip);
		snprintf(wan_login_error,sizeof(wan_login_error),"http://%s/phone/login.html?0",wp->host_str);
					
		snprintf(lan_wlpwd,sizeof(lan_wlpwd),"http://%s/redirect_set_wlsec.asp",lan_ip);
		snprintf(lan_index,sizeof(lan_index),"http://%s/phone/scan.html",lan_ip);

		snprintf(lan_quickset,sizeof(lan_quickset),"http://%s/phone/scan.html",lan_ip);
		snprintf(lan_redirect,sizeof(lan_quickset),"http://%s/phone/scan.html",lan_ip);

		snprintf(wan_wlpwd,sizeof(wan_wlpwd),"http://%s/phone/scan.html",wp->host_str);
		snprintf(wan_index,sizeof(wan_index),"http://%s/phone/scan.html",wp->host_str);
		
		snprintf(login_succes,sizeof(login_succes),"http://%s/loginsuccess.html",lan_ip);
		snprintf(login_error,sizeof(login_error),"http://%s/loginerror.html",lan_ip);
	}
	else
#endif
	{
		snprintf(lan_login,sizeof(lan_login),"http://%s/login.html",lan_ip); 
		snprintf(wan_login,sizeof(wan_login),"http://%s/login.html",wp->host_str);
		
		snprintf(lan_login_error,sizeof(lan_login_error),"http://%s/login.html?1",lan_ip);
		snprintf(wan_login_error,sizeof(wan_login_error),"http://%s/login.html?0",wp->host_str);
		
		snprintf(lan_wlpwd,sizeof(lan_wlpwd),"http://%s/redirect_set_wlsec.asp",lan_ip);
		snprintf(lan_index,sizeof(lan_index),"http://%s/index.html",lan_ip);

		snprintf(lan_quickset,sizeof(lan_quickset),"http://%s/quickset.html",lan_ip);
		snprintf(lan_redirect,sizeof(lan_quickset),"http://%s/network-diagnose.html",lan_ip);

		snprintf(wan_wlpwd,sizeof(wan_wlpwd),"http://%s/redirect_set_wlsec.asp",wp->host_str);
		snprintf(wan_index,sizeof(wan_index),"http://%s/index.html",wp->host_str);
		
		snprintf(login_succes,sizeof(login_succes),"http://%s/loginsuccess.html",lan_ip);
		snprintf(login_error,sizeof(login_error),"http://%s/loginerror.html",lan_ip);
	}

	if(1 == quit_redirect_web(wp,urlbuf))
	{
		return 0;
	}
	if(wp != NULL)
	{
		for (i=0; i<MAX_USER_NUM; i++)
		{
			if(!strcmp(loginUserInfo[i].ip, wp->ipaddr)) 
			{
				break;
			}
		}
	}
	if(i<MAX_USER_NUM)	
	{
		/*neither lan nor wan*/
		if (strncmp(wp->host_str,lan_ip,strlen(lan_ip))!=0 && strncmp(wp->host_str,wan_ip,strlen(wan_ip))!=0)
		{
			if(dns_redirect_web == 1){
				websResponse(wp,302,NULL,T(lan_wlpwd));
				dns_redirect_web = -1;
			}else{
				websResponse(wp,302,NULL,T(lan_login));
			}
			return 0;
		}
		
		if(strcmp(g_Pass, "") == 0){
			if((!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/login/Auth", 11))
			#ifdef __CONFIG_MOBILE_WEB__
				|| (!memcmp(urlbuf, "/phone/login.html", 17))
			#endif
			)
			{
				goto LOGINOK;
			} 
			else
			{
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}
		}
		//根据coverity分析结果修改，原来为无效的判断:if( wp->ipaddr!=NULL)  2017/1/11 F9项目修改
		if ( wp!=NULL)
		{
			sprintf(http_true_passwd,"%s%s",g_Pass,cookie_suffix[((unsigned int)inet_addr(wp->ipaddr))%20]);
		}
		else
		{
			sprintf(http_true_passwd,"%s",g_Pass);
		}

		if(pcookie && !strncmp(pcookie, http_true_passwd,strlen(http_true_passwd))){

			
			if(!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1 && urlbuf[0]=='/') || !memcmp(urlbuf, "/login/Auth", 11)
			#ifdef __CONFIG_MOBILE_WEB__
				|| (!memcmp(urlbuf, "/phone/login.html", 17))
			#endif
			)
			{
				goto LOGINOK;
			}
			else if((!memcmp(urlbuf, "/goform/NetWorkSetupInit", strlen("/goform/NetWorkSetupInit")))
						||(!memcmp(urlbuf, "/goform/SpeedControlInit", strlen("/goform/SpeedControlInit")))
							||(!memcmp(urlbuf, "/goform/WirelessRepeatInit", strlen("/goform/WirelessRepeatInit")))
								||(!memcmp(urlbuf, "/goform/WirelessRepeatApInit", strlen("/goform/WirelessRepeatApInit")))
									||(!memcmp(urlbuf, "/goform/SystemManageInit", strlen("/goform/SystemManageInit")))
										||(!memcmp(urlbuf, "/goform/getStatus", strlen("/goform/getStatus")))
										||(!memcmp(urlbuf, "/goform/getQos", strlen("/goform/getQos")))
										||(!memcmp(urlbuf, "/goform/getWAN", strlen("/goform/getWAN")))
										||(!memcmp(urlbuf, "/goform/getWifi", strlen("/goform/getWifi")))
										||(!memcmp(urlbuf, "/goform/getSysTools", strlen("/goform/getSysTools"))))
			{
				return 0;
			}			
			else
			{
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}	
		}
		else{
			if(strstr(wp->url,"login")!=NULL || !memcmp(urlbuf, "/login/Auth", 11) 
				|| !memcmp(urlbuf, "/redirect_set_wlsec", 18)|| !memcmp(urlbuf, "/goform/redirectSetwlSecurity", 29))
			{
				goto RELOGIN;
			}
			else{
				websResponse(wp,302,NULL,T(lan_login));
				loginUserInfo[i].time = (unsigned int)cyg_current_time();
				return 0;
			}
		}
	} 
	else
	{
		
		/*neither lan nor wan*/
		if ( strncmp(wp->host_str,lan_ip,strlen(lan_ip))!=0 && strncmp(wp->host_str,wan_ip,strlen(wan_ip))!=0 )
		{
			if(dns_redirect_web == 1){
				websResponse(wp,302,NULL,T(lan_wlpwd));
				dns_redirect_web = -1;
			}else{
				websResponse(wp,302,NULL,T(lan_login));
			}
			return 0;
		}
		
		if(strcmp(g_Pass, "") == 0
		#ifdef __CONFIG_MOBILE_WEB__
			|| (phone_flag == 1 && nvram_match("http_defaultpwd", g_Pass))
		#endif
		)
		{
			
			goto NOPASSWORD;
		}		

RELOGIN:
		if (strlen(urlbuf)==1&&urlbuf[0]=='/')
		{
		#ifdef __CONFIG_MOBILE_WEB__
			if(strcmp(g_Pass, "") == 0 ||
				(phone_flag == 1 && nvram_match("http_defaultpwd", g_Pass)))
				goto NOPASSWORD;
			else
		#endif
			{
				goto ERROREXIT;
			}
		}
		else if (!memcmp(urlbuf, "/login.html", 11))
			return 0;
	#ifdef __CONFIG_MOBILE_WEB__
		else if(!memcmp(urlbuf, "/phone/login.html", 17)) //hong add phone html)
			return 0;
	#endif
		else if  (!memcmp(urlbuf, "/loginerror.html", 16))
			return 0;
		else if (!memcmp(urlbuf, "/login/Auth", 11))
		{
			password = websGetVar(wp, T("password"), T(""));
			//userid = websGetVar(wp, T("Username"), T("admin"));	根据coverity分析结果修改，原来为无效的判断:if( wp->ipaddr!=NULL)  2017/1/11 F9项目修改
			
			if ( !strcmp(password, g_Pass))
			{
				for(i=0; i<MAX_USER_NUM; i++)
				{
					if (strlen(loginUserInfo[i].ip) == 0 || !strcmp(loginUserInfo[i].ip, wp->ipaddr))
					{
						memcpy(loginUserInfo[i].ip , wp->ipaddr, PI_IP_STRING_LEN); 
						
						loginUserInfo[i].time = (unsigned int)cyg_current_time();
						goto LOGINOK;
						break;
						
					}
				}
				if(i == MAX_USER_NUM) 
				{
					websRedirect(wp, T(strcat(lan_login,"?2")));//有密码时用户数达上限
					return 0;
				}
			}
			else
			{
				goto LOGINERR;
			}		
		}
		else{
			goto ERROREXIT;
		}
	}
	return 0;
	
ERROREXIT:
	websRedirect(wp, T(lan_login));
	return 0;

LOGINOK:
	if(dns_redirect_web == 1){
		websResponseWithCookie(wp,302,NULL,T(lan_wlpwd), 1);
		dns_redirect_web = -1;
	}
	else{
		if(strncmp(wp->host_str,wan_ip,strlen(wan_ip))==0){
			/*remote access router*/
			websResponseWithCookie(wp,302,NULL,T(wan_index), 1);
		}
		else{
			/*local access router*/ 	

			char *restore_quick_set;
			_GET_VALUE(SYSCONFIG_QUICK_SET_ENABLE,restore_quick_set);
			
			if(atoi(restore_quick_set) == 1){
				websResponseWithCookie(wp,302,NULL,T(lan_quickset), 1);
			}
			else{
				websResponseWithCookie(wp,302,NULL,T(lan_index), 1);
			}			
		}
	}
	return 0;
	
LOGINERR:
	websRedirect(wp, T(lan_login_error));
	printf("Login ERROR !!!\n");
	return 0;
	
NOPASSWORD:
	if(strcmp(g_Pass, "") == 0
	#ifdef __CONFIG_MOBILE_WEB__
		|| (phone_flag == 1 && nvram_match("http_defaultpwd", g_Pass))
	#endif
	)
	{
		for(i=0; i<MAX_USER_NUM; i++)
		{
			if (strlen(loginUserInfo[i].ip) == 0)
			{
				memcpy(loginUserInfo[i].ip , wp->ipaddr, PI_IP_STRING_LEN);
				loginUserInfo[i].time = (unsigned int)cyg_current_time();

				if((!memcmp(urlbuf, "/login.html", 11) || (strlen(urlbuf)==1&&urlbuf[0]=='/')) || (!memcmp(urlbuf, "/login/Auth", 11))
				#ifdef __CONFIG_MOBILE_WEB__
					|| (!memcmp(urlbuf, "/phone/login.html", 17))
				#endif
				)
				{
					//printf("NOPASSWORD: goto LOGINOK\n");
					goto LOGINOK;
				} 
				else
				{
					//printf("NOPASSWORD: return\n");
					loginUserInfo[i].time = (unsigned int)cyg_current_time();
					return 0;
				}	

				break;
				
			}	
		}

		if(i == MAX_USER_NUM) 
		{
			if ((!memcmp(urlbuf, "/login.html", 11)) 
			#ifdef __CONFIG_MOBILE_WEB__
				|| (!memcmp(urlbuf, "/phone/login.html", 17))
			#endif
				)	
			{
				return 0;
			}
			websRedirect(wp, T(strcat(lan_login,"?3")));//无密码时用户数达上限
		}
	}
	return 0;

}

#if 0

PIU8 set_modules_loginAuth[] = 
{
	MODULE_SET_LOGIN_PWD,
	MODULE_SET_END,
};

PIU8 get_modules_loginAuth[] = 
{
	MODULE_GET_LOGIN_PWD,
	MODULE_GET_END,
};
#endif
RET_INFO cgi_sysmanage_loginpwd_set(webs_t wp, CGI_MSG_MODULE * msg, char * err_code, void *info)
{

	cJSON *obj = NULL;
	CGI_LIB_INFO set_info;
		
	PIU8 modules[] = 
	{
		MODULE_SET_LOGIN_PWD,		
	};
	
	set_info.wp = wp;
	set_info.root = NULL;
	set_info.modules = modules;
	set_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_set(set_info,msg,err_code,NULL);

	
	char_t *new_password,*old_password;
	new_password = cgi_lib_get_var(wp,NULL, T("newPwd"), "");
	old_password = cgi_lib_get_var(wp,NULL, T("oldPwd"), "");

	cJSON *cj_set	= NULL;
	cJSON *module	= NULL;
	cJSON *cj_query 	= NULL;
	int uc_ret = 100;
	cj_query = cJSON_CreateObject();
	cj_set = cJSON_CreateObject();
	cJSON_AddItemToObject(cj_query, "module", module = cJSON_CreateArray());
	cJSON_AddItemToArray(module,cJSON_CreateString("uc_set_login_sta_update"));
	cJSON_AddStringToObject(cj_set, "new_passwd",new_password);
	cJSON_AddStringToObject(cj_set, "old_passwd",old_password);
#ifdef __CONFIG_TENDA_APP__
	uc_cgi_set_module_param(cj_query,cj_set);
#endif
	cJSON_Delete(cj_query);
	cJSON_Delete(cj_set);

	
	return RET_SUC;
}


RET_INFO cgi_sysmanage_loginpwd_get(webs_t wp, cJSON *root, void *info)
{
	cJSON *obj = NULL;
	CGI_LIB_INFO get_info;
	PIU8 modules[] = 
	{
		MODULE_GET_LOGIN_PWD,	
	};

	cJSON_AddItemToObject(root, "loginAuth", obj = cJSON_CreateObject());
	
	get_info.wp = wp;
	get_info.root = obj;
	get_info.modules = modules;
	get_info.module_num = ARRAY_SIZE(modules);
	cgi_lib_get(get_info,NULL);
	return RET_SUC;
}

