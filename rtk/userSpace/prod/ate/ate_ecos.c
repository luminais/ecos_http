#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>
#include <etioctl.h>
#include <bcmgpio.h>
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>

#include "sys_module.h"
#include "button.h"
#include "systools.h"
#include "ate.h"
#include "sys_timer.h"

#define NVRAM_ACTION_SET_CHECK      0
#define NVRAM_ACTION_JUST_SET       1
#define NVRAM_ACTION_JUST_CHECK     2
 
 
#define MFG_STACK_SIZE (1024*16)
static cyg_uint8 ate_stack[MFG_STACK_SIZE];
static cyg_handle_t ate_thread_handle;
static cyg_thread ate_thread_struct;

static int ate_running = 0;
static PI32 ate_fd = -1;
static struct sockaddr ate_from;

extern int run_clicmd(char *command_buf);
extern void get_address_pool(const char *lan_ip ,const  char *lan_mask,char* start_ip,char* end_ip );
#define foreach_arch(word, wordlist, next,temp,temp1) \
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

//extern int envram_get(int argc, PI8* argv[]);
//extern int envram_set(int argc, PI8* argv[]);
//extern int envram_commit(int argc, PI8* argv[]);
//extern void nvram_default(void);
	
#if defined(__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__) && defined(__CONFIG_ATE__) 
extern unsigned int g_ate_ctl_led;
#endif
extern int envram_commit(int argc, PI8* argv[]);
extern char *__envram_get(const char *name);
extern int __envram_set(const char *name, const char *value);
extern int envram_unset(int argc, char* argv[]);
extern int nvram_unset(const char *name);


static PI8 ate_set_value(const PI8 *name, const PI8 *value);
static PI8 *ate_get_value(PI8 *name);
static PI8 ate_commit(void);
static void ate_get_html_info(PI8 *buf);
static PI8 wl_wps_pin_check(PI8 *pin_string);
static void ate_button_poll(void);
static void ate_change_sys_led_timer_status(DOTIMERINFO action);
static PI32 ate_open_socket(void);
static PI32 ate_recv(PI8 *buf,PIU32 len);
static PI8 ate_action_2(PIU8 argc, PI8 *argv[],PI8 *buf);
static PI8 ate_action_3(PIU8 argc, PI8 *argv[],PI8 *buf);
static PI8 ate_action_4(PIU8 argc, PI8 *argv[],PI8 *buf,int flag);
static PI8 ate_action_6(PIU8 argc, PI8 *argv[],PI8 *buf);
static PI8 ate_handle(PIU8 argc, PI8 *argv[],PI8 *buf,int flag);
static void ate_send(PI8 *buf,PIU32 len);
static void ate_main(void);

int string_ate_to_hwpower(char *string, char *to)
{
    char *ptr, *p_end;
    int i,j = 0;
    int len = strlen(string);
    unsigned int val = 0;
    char *pp = to;
    ptr = string;
    for (i = 0; i < len; i++)
    {
        p_end = string + i;
        if (*p_end == ',')
        {
            *p_end = '\0';
            val = strtoul(ptr,NULL,10);
            sprintf(pp,"%02x",val);
            //printf("pp=%s val=0x%x\n",pp,val);
            pp += 2;
            ptr = p_end + 1;
            j++;
        }
    }
    if (*ptr != ',')
    {
        val = strtoul(ptr,NULL,10);
        sprintf(pp,"%02x",val);
        pp += 2;
        j++;
    }
    //printf("ptr = %p ;p_end = %p\n",ptr,p_end);
    return j;
}
/*lq   nvram get 的值转化为10进制 */
int string_hwpower_to_ate(char *string, char *to)
{
    int i;
    int len = strlen(string);
    int temp = 0;
    char cha_temp;

    char str_temp[10] = {0};
    for (i = 0; i < len; i++)
    {
        cha_temp = *(string + i);
        if(cha_temp>='a'&&cha_temp<='f')
        {
            temp = temp*16 + (cha_temp - 'a') + 10;
        }
        else
            temp = temp*16 + (cha_temp - '0');
        if (i%2 && (i < len - 1))
        {

            memset(str_temp,0,sizeof(str_temp));
            sprintf(str_temp,"%d,",temp);
            strcat(to,str_temp);
            temp = 0;
        }
    }
    memset(str_temp,0,sizeof(str_temp));
    sprintf(str_temp,"%d",temp);
    strcat(to,str_temp);
    return strlen(to);
}

static PI8 ate_unset_value(const PI8 *name)
{
	char *argv[3]= {"envram","unset",WLAN5G_BANDWIDTH};
	if(NULL == name)
	{
		return ERROR;
	}

	if(envram_unset(3,argv) == 0 && nvram_unset(name) == 0)
	{
		return SUCCESS;
	}
	
	return ERROR;
}


static PI8 ate_set_value(const PI8 *name, const PI8 *value)
{
    if( __envram_set(name, value) == 0 && nvram_set(name, value) == 0)
    {
        return SUCCESS;
    }
    return ERROR;
}

static PI8 *ate_get_value(PI8 *name)
{
    return (nvram_get(name) ? : "");
}
static void ate_get_ssid(PI8 *name)
{
	char wlanmac_value[64] = {0};
	char prev_ssid[64] = {0};
	int i = 0,j = 0;
	char *p = NULL;
	if(NULL == name)
	{
		return ;
	}
	if(nvram_get("wl0_ssid")){
		strcpy(prev_ssid, nvram_get("wl0_ssid"));
	}else{
		diag_printf("wl_ssid is empty!!!\n");
		return;
	}
	p = strchr(prev_ssid,'_');
	if(NULL != p)
	{
		*p = '\0'; 
	}
	if(nvram_get("et0macaddr")){
		strcpy(wlanmac_value,nvram_get("et0macaddr"));
	}
	else{
		diag_printf("macaddr is empty!!!\n");
		return;
	}
	for(i=0;  wlanmac_value[i]!='\0' && i<9; ++i){
		if (wlanmac_value[i+9] != ':'){		
				wlanmac_value[j]=wlanmac_value[i+9];
				++j;
			}
	}
	sprintf(name,"%s_%s",prev_ssid,wlanmac_value);
    return;
}

static PI8 *ate_get_envram(PI8 *name)
{
    return (__envram_get(name) ? : "");
}

static PI8 ate_commit(void)
{
    if( envram_commit(0,NULL)==0 && nvram_commit()==0)
    {
        return SUCCESS;
    }
    return ERROR;
}

static int ate_set_nvram_restore(void)
{
    char *str;
    str = nvram_get("restore_defaults");
    if (str && (strcmp(str,"1") == 0))
    {
        return SUCCESS;
    }
    else
    {
        if (nvram_set("restore_defaults","1"))
            return ERROR;
        if (nvram_commit())
            return ERROR;
    }
    return SUCCESS;
}
static void ate_get_wifi_power(const char *pre,char *value)
{
	char ate_name[32] = {0};
	char *p_value = NULL;
	if(!pre)
		return;
	if(!value)
		return;
	sprintf(ate_name,"%s%s",pre,"_ctv_power");
	p_value = ate_get_envram(ate_name);

	if(0 == strcmp(p_value,"low"))
	{
		strcpy(value,"0");
	}
	else if(0 == strcmp(p_value,"normal"))
	{
	   strcpy(value,"1");
	}
	else if(0 == strcmp(p_value,"high"))
	{
	   strcpy(value,"2");
	}
	return ;
}
//产测获取功率
static void ate_get_wifi_power_2g_5g(char *value)
{
	char maxpower_2g[32] = {0};
	char maxpower_5g[32] = {0};
	strcpy(maxpower_2g,ate_get_envram(WLAN24G_CURRENT_POWER));
	strcpy(maxpower_5g,ate_get_envram(WLAN5G_CURRENT_POWER));
 	if(0 == strcmp(maxpower_2g, "low") && 0 == strcmp(maxpower_5g, "low"))
	{
		strcpy(value, "0");
	}
	else if(0 == strcmp(maxpower_2g, "normal") && 0 == strcmp(maxpower_5g, "normal"))
	{
		strcpy(value, "1");
	}
	else if(0 == strcmp(maxpower_2g, "high") && 0 == strcmp(maxpower_5g, "high"))
	{
		strcpy(value, "2");
	}
	else
	{
		strcpy(value, "");
		return ;
	}
	return ;
}
//根据pre获取2G 5G支持的最大功率等级
static void ate_get_support_power(const char *pre,char *value)
{
	char ate_name[32] = {0};
	char *p_value = NULL;
	if(!pre)
		return;
	if(!value)
		return;
	sprintf(ate_name,"%s%s",pre,"_country_pwr_power");
	p_value = ate_get_envram(ate_name);
	if(0 == strcmp(p_value,"100"))
	{
		strcpy(value,"0");
	}
	else if(0 == strcmp(p_value,"92,100"))
	{
	   strcpy(value,"1");
	}
	else if(0 == strcmp(p_value,"78,92,100"))
	{
	   strcpy(value,"2");
	}
	return ;
}
static void ate_get_support_power_2g_5g(char *value)
{
	if(!value)
		return;
	char support_power_2g[32] = {0};
	char support_power_5g[32] = {0};
	strcpy(support_power_2g,ate_get_envram(WLAN24G_SUPPOR_POWER_LEVER));
	strcpy(support_power_5g,ate_get_envram(WLAN5G_SUPPOR_POWER_LEVER));
	if(0 == strcmp(support_power_2g, "100") && 0 == strcmp(support_power_5g, "100"))
	{
		strcpy(value, "0");
	}
	else if(0 == strcmp(support_power_2g, "92,100") && 0 == strcmp(support_power_5g, "92,100"))
	{
		strcpy(value, "1");
	}
	else if(0 == strcmp(support_power_2g, "78,92,100") && 0 == strcmp(support_power_5g, "78,92,100"))
	{
		strcpy(value, "2");
	}
	else
	{	
		strcpy(value, "");
		return ;
	}
	return;
}
/* 产测的时候返回产测修改后的IP MAC 国家代码 MTU值 功率等信息  add by 段靖铖 2016年10月27日*/
static void ate_get_html_info(PI8 *buf)
{
    PI8 *lanmac,*wanmac,*pin,*county_code,*akm,*wpa_psk,*serial_number,*mtu_dhcp,*mtu_pppoe = NULL;
    PI8 *mtu_static,*mtu_pptp,*mtu_l2tp,*ip,*login_enpw = NULL;
    PI8 *elink_sn,*ctei_sn = NULL;
	PI8 *access_write = NULL;
    char maxpower_2g[8] = {0};
	char maxpower_5g[8] = {0};
	char power_2g_5g[8] = {0};
	char support_power_2g[8] = {0};
	char support_power_5g[8] = {0};
	char support_power_2g_5g[8] = {0};
    int spower=0;
    char login_depw[64];
	char ssid[64] = {0};
	//以下三个产测防漏测标志位
	PI8 *RFTestFlag = NULL;
	PI8 *ThroughputTestFlag = NULL;
	PI8 *FinishTestFlag = NULL;
    elink_sn = ate_get_envram("elink_sn");
    ctei_sn = ate_get_envram("ctei_sn");
    lanmac = ate_get_envram("et0macaddr");
    wanmac = ate_get_envram("wan0_hwaddr");
    pin = ate_get_envram("wps_device_pin");
    county_code = ate_get_envram("country_code");
    akm = ate_get_value("wl0_akm");
    wpa_psk =ate_get_value("wl0_wpa_psk");
    mtu_dhcp = ate_get_value("dhcp_wan0_mtu");
    mtu_pppoe = ate_get_value("pppoe_wan0_mtu");
    mtu_static = ate_get_value("static_wan0_mtu");
    mtu_pptp = ate_get_value("wan0_pptp_mtu");
    mtu_l2tp = ate_get_value("wan0_l2tp_mtu");
    ip = ate_get_envram("lan_ipaddr");
    login_enpw = ate_get_envram("http_passwd");
	access_write = ate_get_envram("ACCESS_WRITE");
	ate_get_wifi_power("wl0",maxpower_2g);
	ate_get_wifi_power("wl1",maxpower_5g);
	ate_get_wifi_power_2g_5g(power_2g_5g);
	ate_get_support_power("wl0",support_power_2g);
	ate_get_support_power("wl1",support_power_5g);
	ate_get_support_power_2g_5g(support_power_2g_5g);
	RFTestFlag = ate_get_value("RFTestFlag");
	ThroughputTestFlag = ate_get_value("ThroughputTestFlag");
	FinishTestFlag = ate_get_value("FinishTestFlag");
    base64_decode(login_enpw,login_depw,64);
	ate_get_ssid(ssid);


    if(0 == strcmp(akm,""))
    {
        wpa_psk = "";
    }

    serial_number = ate_get_value("serial_number");

    sprintf(buf,"SV=%s_%s;LANMAC=%s;WAN1MAC=%s;WAN2MAC="";SSID=%s;PIN=%s;WPA_PW=%s;SN=%s;COUNTRY=%s;MAXPOWER=%d;MAXPOWER_2G=%s;MAXPOWER_5G=%s;MTU_DHCP=%s;MTU_PPPOE=%s;MTU_STATIC=%s;MTU_PPTP=%s;MTU_L2TP=%s;SUPPORT_POWER=%s;SUPPORT_POWER_2G=%s;SUPPORT_POWER_5G=%s;IP=%s;Login_PW=%s;ACCESS_WRITE=%s;RFTestFlag=%s;ThroughputTestFlag=%s;FinishTestFlag=%s;ELINK_SN=%s;CTEI_SN=%s;",
            W311R_ECOS_SV,__CONFIG_WEB_VERSION__,lanmac,wanmac,ssid,pin,wpa_psk,serial_number,county_code,power_2g_5g,maxpower_2g,maxpower_5g,mtu_dhcp,mtu_pppoe,mtu_static,mtu_pptp,mtu_l2tp,support_power_2g_5g,support_power_2g,support_power_5g,ip,login_depw,access_write,RFTestFlag,ThroughputTestFlag,FinishTestFlag,elink_sn,ctei_sn);
    //PI_PRINTF(ATE,"buf = %s\n",buf);
    return;
}

static void ate_get_lanwan_info(PI8 *buf)
{
    //int wan_num = 1, lan_num = 3;
    //char lanwan_status[4][32] = {0};
    //sprintf(buf,"WanNum=%d;LanNum=%d;Wan1=%s;Wan2=;Lan1=%s;Lan2=%s;Lan3=%s;END",
    //      wan_num, lan_num, lanwan_status[0], lanwan_status[1],lanwan_status[2],lanwan_status[3]);
}


static PI8 wl_wps_pin_check(PI8 *pin_string)
{
    unsigned long pin = strtoul(pin_string, NULL, 10);
    unsigned long int accum = 0;
    unsigned int len = strlen(pin_string);
    if (len != 4 && len != 8)
        return  ERROR;
    if (len == 8)
    {
        accum += 3 * ((pin / 10000000) % 10);
        accum += 1 * ((pin / 1000000) % 10);
        accum += 3 * ((pin / 100000) % 10);
        accum += 1 * ((pin / 10000) % 10);
        accum += 3 * ((pin / 1000) % 10);
        accum += 1 * ((pin / 100) % 10);
        accum += 3 * ((pin / 10) % 10);
        accum += 1 * ((pin / 1) % 10);
        if (0 == (accum % 10))
            return SUCCESS;
    }
    else if (len == 4)
        return SUCCESS;
    return ERROR;
}

void ate_set_all_led_on( void )
{
#if 0
    PI32 s, gpio_pin;
    PI32 led_mask=0,led_value = 0,value =0 ;
    PI32 vecarg[2];
    struct ifreq ifr;

    /* Light all the other led except lan&wan, e.g. wl/sys/wps */
    for(gpio_pin = 5; gpio_pin <= 7; gpio_pin++)
    {
        /*enable the wl gpio value input*/
        bcmgpio_connect(gpio_pin, BCMGPIO_DIRN_OUT);
        led_mask = ((unsigned long)1 << gpio_pin);
        led_value = (~value << gpio_pin);
        led_value &= led_mask;
        bcmgpio_out(led_mask, led_value);
        bcmgpio_disconnect(gpio_pin);
    }

    /* Light lan&wan led */
    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("open socket failed\n");
        return;
    }

    vecarg[0] = 0x0 << 16;
    vecarg[0] |= (0x12 & 0xffff);
    vecarg[1] = 0x155;
    ifr.ifr_data = (caddr_t) vecarg;
    strncpy(ifr.ifr_name, "eth0", sizeof (ifr.ifr_name));
    if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0)
    {
        printf("Error: etcrobowr\n");
        close(s);
        return ;
    }

    close(s);
#endif

#if defined(__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__) && defined(__CONFIG_ATE__) 
		g_ate_ctl_led = 1;
#endif

    tenda_all_led_on_off(LED_ON);
    /*lq 接口冲突*/
#ifdef __CONFIG_EXTEND_LED__
    extend_led_on_off(LED_ON);
#endif
}
void ate_set_all_led_off( void )
{
#if 0
    PI32 s, gpio_pin;
    PI32 led_mask=0,led_value = 0,value =0 ;
    PI32 vecarg[2];
    struct ifreq ifr;

    /* Light all the other led except lan&wan, e.g. wl/sys/wps*/
    for(gpio_pin = 5; gpio_pin <= 7; gpio_pin++)
    {
        /*enable the wl gpio value input*/
        bcmgpio_connect(gpio_pin, BCMGPIO_DIRN_OUT);
        led_mask = ((unsigned long)1 << gpio_pin);
        led_value = (value << gpio_pin);
        //led_value &= led_mask;
        bcmgpio_out(led_mask, led_value);
        bcmgpio_disconnect(gpio_pin);
    }

    /* Light lan&wan led */
    if((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("open socket failed\n");
        return;
    }

    vecarg[0] = 0x0 << 16;
    vecarg[0] |= (0x12 & 0xffff);
    vecarg[1] = 0x0;
    ifr.ifr_data = (caddr_t) vecarg;
    strncpy(ifr.ifr_name, "eth0", sizeof (ifr.ifr_name));
    if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0)
    {
        printf("Error: etcrobowr\n");
        close(s);
        return ;
    }

    close(s);
#endif

#if defined(__CONFIG_SUPPORT_SWITCH_LED_BY_GPIO__) && defined(__CONFIG_ATE__) 
		g_ate_ctl_led = 1;
#endif

	tenda_all_led_on_off(LED_OFF);
   

#ifdef __CONFIG_EXTEND_LED__
    extend_led_on_off(LED_OFF);
#endif
}

static void ate_change_sys_led_timer_status(DOTIMERINFO action)
{
    sys_do_timer_action(action,SYSLED_TIMER);
    return;
}

#ifdef __CONFIG_EXTEND_LED__
void extend_led_stop();
void extend_led_start();
static void ate_change_extend_led_timer_status(DOTIMERINFO action)
{
    if(action)
    {
        extend_led_start();
    }
    else
    {
        extend_led_stop();
    }
    return;
}
#endif


static void ate_button_poll(void)
{
	PI8 utimes = 0;


    for(utimes = 0; utimes < 2; utimes++)
    {
        if(1 == sys_button_get_button_result())
        {
            PI_PRINTF(ATE,"Button pressed!!!\n\n");

            PI_PRINTF(ATE,"ledtest  off \n");
            ate_set_all_led_off();

            cyg_thread_delay(50); // 0.5 s

            PI_PRINTF(ATE,"ledtest  on \n");
            ate_set_all_led_on();

            break;
        }
        //cyg_thread_delay(10); // 0.1 s
    }

    return;
}

static PI32 ate_open_socket(void)
{
    PI32 fd = -1;
    struct sockaddr_in local;
    PI8 lan_ip[PI_IP_STRING_LEN] = {0};

    strcpy(lan_ip,nvram_safe_get("lan_ipaddr"));

    memset( &local, 0, sizeof(local) );
    local.sin_family = AF_INET;
    local.sin_port = htons(7329);
    local.sin_addr.s_addr = inet_addr(lan_ip);

    fd = socket( AF_INET, SOCK_DGRAM, 0 );

    if ( fd < 0 )
    {
        PI_PRINTF(ATE,"ate socket error.\n");
        return -1;
    }

    if (bind(fd, (struct sockaddr *)&local, sizeof(local)) < 0)
    {
        PI_PRINTF(ATE,"ate bind error.\n");
        close(fd);
        return -1;
    }

    ate_fd = fd;

    return fd;
}

static PI32 ate_recv(PI8 *buf,PIU32 len)
{
    PI32 ret = 0;
    fd_set rfds;
    PI32 ilen;
    PI32 retval;
    struct timeval tv = {2,0};

    FD_ZERO(&rfds);
    FD_SET(ate_fd, &rfds);

    tv.tv_sec = 0;
    tv.tv_usec = 200 *1000; //0.2 second

    retval = select(ate_fd + 1, &rfds, NULL, NULL, &tv);

    if (retval <= 0 || !FD_ISSET(ate_fd, &rfds))
        return 0;

    ilen = sizeof(ate_from);
    ret = recvfrom(ate_fd, buf, len, 0, &ate_from, &ilen);

    if (ret == len )
        buf[len - 1] = '\0';

    return ret;
}

static PI8 ate_action_2(PIU8 argc, PI8 *argv[],PI8 *buf)
{
    if(2 != argc)
        return ERROR;

    if ( strcmp(argv[0], "Tenda_mfg")==0 )
    {
        if( strcmp(argv[1], "reboot")==0 )
        {
            strcpy(buf,"success");
            ate_send(buf,strlen(buf));
            cyg_thread_delay(100);
            run_clicmd(argv[1]);
            return SUCCESS;
        }
        else if( strcmp(argv[1], "default")==0 )
        {
            nvram_set("restore_defaults", "1");

            if(SUCCESS == nvram_commit())
            {
                strcpy(buf,"success");
                ate_send(buf,strlen(buf));
                cyg_thread_delay(100);
                msg_send(MODULE_RC, RC_SYSTOOLS_MODULE, "string_info=restore");
                return SUCCESS;
            }
            else
            {
                strcpy(buf,"error");
                return ERROR;
            }
        }
        else if ( strcmp(argv[1], "htmlVersionInfo")==0 )
        {
            ate_get_html_info(buf);
            return INFO;
        }
        else if(!strcmp(argv[1], "LanWanInfo"))
        {

            ate_get_lanwan_info(buf);
            return INFO;
        }
	else 
	{
	   	PIU8 gpio = 0,i = 0;
            if(0 == strcmp(argv[1], "ResetButton"))
            {
                gpio = RESET_BUTTON_GPIO;
            }
            else if(0 == strcmp(argv[1], "WiFiButton"))
            {
                gpio = WIFI_BUTTON_GPIO;
            }
#ifdef __CONFIG_WPS_RTK__
            else if(0 == strcmp(argv[1], "WPSButton"))
            {
                gpio = WPS_BUTTON_GPIO;
            }
#endif
            else
            {
                return ERROR;
            }

            for(i = 0; i < 15 ; i++)
            {
                if (1 == sys_button_get_ate_result(gpio))
                {
                    return SUCCESS;
                }
                cyg_thread_delay(50);
            }
	}
    }
    //start: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
    else if(strcmp(argv[0], "ledtest")==0)
    {
        if(strcmp(argv[1], "on")==0)
        {
            ate_set_all_led_on();
            return SUCCESS;
        }
        else if(strcmp(argv[1], "off")==0)
        {
            ate_set_all_led_off();
            return SUCCESS;
        }
    }
    //end: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
    return ERROR;
}

static PI8 ate_action_3(PIU8 argc, PI8 *argv[],PI8 *buf)
{

    if(3 != argc)
        return ERROR;

    if (strcmp(argv[0], "nvram") == 0 && strcmp(argv[1], "get") == 0)
    {
        char val_buf[1024] = {0};
        char *str = NULL;
        int change = 0;
        if (strncmp(argv[2],"HW_WLAN",7) == 0)
        {
            change = 1;
        }

        str = ate_get_value(argv[2]);

        if (change)
        {
            if (string_hwpower_to_ate(str,val_buf))
            {
                str = val_buf;
            }
            else
            {
                return ERROR;
            }
        }
        sprintf(buf,"success:%s",str);
        return INFO;
    }
    return ERROR;

}

typedef struct ate_set_info{
	char *key;
	int  (*action) (char *value,int flag);
}ATE_SET_INFO;

//PIN
static int ate_set_pin(char *value,int flag)
{
	char ate_name[64] = {0};
	char ate_value[64] = {0};
	PI8 wps_pin[16] = {0};
	if(!value)
	{
		return ERROR;
	}
	strcpy(ate_name,"wps_device_pin");
	strcpy(ate_value,value);

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),ate_value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (wl_wps_pin_check(ate_value) == 0)
	{
		if (ate_set_value(ate_name, ate_value) == ERROR)
		{
			return ERROR;
		}
	}
	else
	{
		return ERROR;
	}
}
//end PIN

//MAC
static int ate_set_mac(char *value,int flag)
{
	char ate_name[64] = {0};
	struct ether_addr *hwaddr = NULL;
	PI8 macaddr[32] = {0};
	unsigned int lan_mac = 0;
	unsigned int mac_24 = 0;
	unsigned int mac_5 = 0;
	char macaddr_5G[32] = {0};
	char macaddr_24[32] = {0};
	if(NULL == value)
	{
		return ERROR;
	}
	hwaddr = ether_aton(value);
	if(hwaddr)
	{
		memset(macaddr, 0, sizeof(macaddr));
		snprintf(macaddr, sizeof(macaddr), "%02X:%02X:%02X:%02X:%02X:%02X",
				 hwaddr->octet[0] & 0XFF,
				 hwaddr->octet[1] & 0XFF,
				 hwaddr->octet[2] & 0XFF,
				 hwaddr->octet[3] & 0XFF,
				 hwaddr->octet[4] & 0XFF,
				 hwaddr->octet[5] & 0XFF);

	   mac_add(macaddr,macaddr_24,1);
	   mac_add(macaddr_24,macaddr_5G,4);
	   memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"et0macaddr");
		if (flag == NVRAM_ACTION_JUST_CHECK)
		{
			if (strcmp(ate_get_value(ate_name),macaddr) == 0)
				return SUCCESS;
			else
				return ERROR;
		}

		if (ate_set_value(ate_name, macaddr) == ERROR)
		{
			return ERROR;
		}


		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wl1_hwaddr");
		if (ate_set_value(ate_name, macaddr_5G) == ERROR)
		{
			return ERROR;
		}
		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wl0_hwaddr");
		if (ate_set_value(ate_name, macaddr_24) == ERROR)
		{
			return ERROR;
		}
		
		set_ssid_mac("wl0");
		set_ssid_mac("wl1");
		ate_set_nvram_restore();
	}//end hwaddr
	else
	{
		printf("func:%s line:%d\n",__func__,__LINE__);
		return ERROR;
	}
}
//end MAC

//SN
static int ate_set_sn(char *value,int flag)
{
	char ate_name[64] = {0};
	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"serial_number");

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}
	
	return SUCCESS;
}

//end SN

static int ate_set_elink_sn(char *value,int flag)
{
	char ate_name[64] = {0};
	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"elink_sn");

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}
	
	return SUCCESS;
}

static int ate_set_ctei_sn(char *value,int flag)
{
	char ate_name[64] = {0};
	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"ctei_sn");

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}
	
	return SUCCESS;
}

//MTU_DHCP
static int ate_set_dhcp_mtu(char *value,int flag)
{
	char ate_name[64] = {0};
	if ((NULL != value) && (strcmp(value,"")) != 0)/*若写入的值为空,或者为NULL，则不做操作*/
	{
		strcpy(ate_name,"dhcp_wan0_mtu");
		if(flag == NVRAM_ACTION_JUST_CHECK)
		{
			if(strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
			else
			return ERROR;
		}

		if(ate_set_value(ate_name,value) == ERROR)
		return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_mtu");
		if(ate_set_value(ate_name,value) == ERROR)
		return ERROR;
	}

	return SUCCESS;
}
//end MTU_DHCP

//MTU_PPPOE
static int ate_set_pppoe_mtu(char *value,int flag)
{
	char ate_name[64] = {0};
	if ((NULL != value) && ((strcmp(value,"")) != 0))/*若写入的值为空,或者为NULL，则不做操作*/
	{
		strcpy(ate_name,"pppoe_wan0_mtu");
		
		if(flag == NVRAM_ACTION_JUST_CHECK)
		{
			if(strcmp(ate_get_value(ate_name),value) == 0)
				return SUCCESS;
			else
				return ERROR;
		}

		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_pppoe_mtu");
		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_pppoe_mru");
		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;
	}

	return SUCCESS;
}
//end MTU_PPPOE

//MTU_STATIC
static int ate_set_static_mtu(char *value,int flag)
{
	char ate_name[64] = {0};
	
	if ((NULL != value) && (strcmp(value,"") != 0))/*若写入的值为空,或者为NULL，则不做操作*/
	{
		strcpy(ate_name,"static_wan0_mtu");
		
		if(flag == NVRAM_ACTION_JUST_CHECK)
		{
			if(strcmp(ate_get_value(ate_name),value) == 0)
				return SUCCESS;
			else
				return ERROR;
		}

		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_mtu");
		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;
	}

	return SUCCESS;
}

//end MTU_STATIC

//MTU_PPTP
static int ate_set_pptp_mtu(char *value,int flag)
{
	char ate_name[64] = {0};
	if((NULL != value) && (strcmp(value,"") != 0))/*若写入的值为空,或者为NULL，则不做操作*/
	{
		strcpy(ate_name,"wan0_pptp_mtu");
		
		if(flag == NVRAM_ACTION_JUST_CHECK)
		{
			if(strcmp(ate_get_value(ate_name),value) == 0)
				return SUCCESS;
			else
				return ERROR;
		}

		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_pptp_mru");
		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

	}

	return SUCCESS;
}

//end MTU_PPTP

//MTU_L2TP
static int ate_set_l2tp_mtu(char *value,int flag)
{
	char ate_name[64] = {0};
	if ((NULL != value) && (strcmp(value,"") != 0))/*若写入的值为空,或者为NULL，则不做操作*/
	{
		strcpy(ate_name,"wan0_l2tp_mtu");
	
		if(flag == NVRAM_ACTION_JUST_CHECK)
		{
			if(strcmp(ate_get_value(ate_name),value) == 0)
				return SUCCESS;
			else
				return ERROR;
		}

		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;

		memset(ate_name,0,sizeof(ate_name));
		strcpy(ate_name,"wan0_l2tp_mru");
		if(ate_set_value(ate_name,value) == ERROR)
			return ERROR;
	}

	return SUCCESS;
}

//end MTU_L2TP

//WPA_PW
static int ate_set_wpa_pw(char *value,int flag)
{
	char ate_name_2g[64] = {0};
	char ate_name_5g[64] = {0};

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name_2g,WLAN24G_PASSWD);
	strcpy(ate_name_5g,WLAN5G_PASSWD);
	
	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name_2g),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}
	//2g
	if(ate_set_value(WLAN24G_AKM, "psk psk2"))
	{
		return ERROR;
	}

	if (ate_set_value(WLAN24G_CRYPTO, "aes") == ERROR)
	{
		return ERROR;
	}
	if (ate_set_value(ate_name_2g, value) == ERROR)
	{
		return ERROR;
	}
	//end 2g

	//5g
	if(ate_set_value(WLAN5G_AKM, "psk psk2"))
	{
		return ERROR;
	}

	if (ate_set_value(WLAN5G_CRYPTO, "aes") == ERROR)
	{
		return ERROR;
	}
	if (ate_set_value(ate_name_5g, value) == ERROR)
	{
		return ERROR;
	}
	//end 5g
	
	return SUCCESS;
}

//end WPA_PW

//PLC_PW
static int ate_set_plc_pw(char *value,int flag)
{
	return SUCCESS;
}
//end PLC_PW

//WAN_MODE
static int ate_set_wan_mode(char *value,int flag)
{
	return SUCCESS;
}
//end WAN_MODE

//SSID
static int ate_set_ssid(char *value,int flag)
{
	char ate_name[64] = {0};

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"wl0_ssid");
	
	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"wl_ssid");
	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	return SUCCESS;
}
//end SSID

//WPS
static int ate_set_wps(char *value,int flag)
{
	if(NULL == value)
	{
		return ERROR;
	}
	if (strcmp(value,"off") == 0)
	{
		ate_set_value("lan_wps_oob", "enabled");
		ate_set_value("wl_wps_mode", "disabled");
		ate_set_value("wps_mode", "disabled");
		ate_set_value("wl_wps_method", "");
		if (ate_commit() != SUCCESS)
		{
			return ERROR;
		}


		if (strcmp(ate_get_value("wps_mode"),"disabled"))
		{
			return ERROR;
		}
	}
	return SUCCESS;
}
//end WPS

//Channel
static int ate_set_channel(char *value,int flag)
{
	char ate_name[64] = {0};

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"wl0_channel");

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"wl_channel");
	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	if(atoi(value) <= 5 && atoi(value) >= 1)
	{
		if (ate_set_value("wl0_nctrlsb", "lower") == ERROR)
		{
			return ERROR;
		}

		if (ate_set_value("wl_nctrlsb", "lower") == ERROR)
		{
			return ERROR;
		}
	}
	else if(atoi(value) <= 13 && atoi(value) >= 8)
	{
		if (ate_set_value("wl0_nctrlsb", "upper") == ERROR)
		{
			return ERROR;
		}

		if (ate_set_value("wl_nctrlsb", "upper") == ERROR)
		{
			return ERROR;
		}
	}
	
	return SUCCESS;
}

//end Channel

//Login_PW
static int ate_set_login_pw(char *value,int flag)
{
	char ate_name[64] = {0};
	char user[64], auth[64];

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name,"http_passwd");
	
	base64_encode(value,strlen(value),auth,64);

	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(envram_get(ate_name),auth) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (__envram_set(ate_name, auth) == ERROR)
	{
		return ERROR;
	}

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"http_defaultpwd1");
	if (__envram_set(ate_name, "1") == ERROR)
	{
		return ERROR;
	}

	return SUCCESS;

}
//end Login_PW

//IP
static ate_set_ip(char *value,int flag)
{
	char ate_name[64] = {0};
	char dhcp_ip_start[20],dhcp_ip_end[20];
	char* lan_mask = "255.255.255.0";
	strcpy(ate_name,"lan_ipaddr");
	
	if(NULL == value)
	{
		return ERROR;
	}
	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(envram_get(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (__envram_set(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"lan_dns");
	if (__envram_set(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	get_address_pool( value , lan_mask,dhcp_ip_start,dhcp_ip_end );

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"dhcp_start");
	if (__envram_set(ate_name, dhcp_ip_start) == ERROR)
	{
		return ERROR;
	}

	memset(ate_name,0,sizeof(ate_name));
	strcpy(ate_name,"dhcp_end");
	if (__envram_set(ate_name, dhcp_ip_end) == ERROR)
	{
		return ERROR;
	}

	return SUCCESS;
}

//end IP
//SUPPORT_POWER
static int ate_set_support_power_2g_5g(char *value,int flag)
{
	char ate_name_2g[64] = {0};
	char ate_name_5g[64] = {0};
	char *p_ate_val = NULL;
	char *str_power[] = {"100","92,100","78,92,100"};
	
	if(NULL == value)
	{
		return ERROR;
	}

	strcpy(ate_name_2g,"wl0_country_pwr_power");
	strcpy(ate_name_5g,"wl1_country_pwr_power");
	if(0 == strcmp(value,"0"))
    {
        p_ate_val = str_power[0];
    }
    else if(0 == strcmp(value,"1"))
    {
        p_ate_val = str_power[1];
    }
    else
    {
        p_ate_val = str_power[2];
    }
    if(flag == NVRAM_ACTION_JUST_CHECK)
    {
    	if((strcmp(ate_get_value(ate_name_2g),p_ate_val) != 0) || (strcmp(ate_get_value(ate_name_5g),p_ate_val) != 0))
    	{
			return ERROR;
		}
		return SUCCESS;
    }
	if((ate_set_value(ate_name_2g,p_ate_val) == ERROR) || (ate_set_value(ate_name_5g,p_ate_val) == ERROR))
	{
		 return ERROR;
	}

	return SUCCESS;
}
//end SUPPORT_POWER
//SUPPORT_POWER_2G
static int ate_set_support_power_2g(char *value,int flag)
{
	char *p_ate_val = NULL;
	char ate_name_2g[64] = {0};
	char *str_power[] = {"100","92,100","78,92,100"};

	if(NULL == value)
	{
		return ERROR;
	}
	if(0 == strcmp(value,"0"))
    {
        p_ate_val = str_power[0];
    }
    else if(0 == strcmp(value,"1"))
    {
        p_ate_val = str_power[1];
    }
    else
    {
        p_ate_val = str_power[2];
    }
	strcpy(ate_name_2g,"wl0_country_pwr_power");
	if(flag == NVRAM_ACTION_JUST_CHECK)
    {
    	if((strcmp(ate_get_value(ate_name_2g),p_ate_val) != 0))
    	{
			return ERROR;
		}
		return SUCCESS;
    }
	if((ate_set_value(ate_name_2g,p_ate_val) == ERROR))
	{
		 return ERROR;
	}

	return SUCCESS;
}
//end SUPPORT_POWER_2G

//SUPPORT_POWER_5G
static int ate_set_support_power_5g(char *value,int flag)
{
	char *p_ate_val = NULL;
	char ate_name_5g[64] = {0};
	char *str_power[] = {"100","92,100","78,92,100"};

	if(NULL == value)
	{
		return ERROR;
	}
	if(0 == strcmp(value,"0"))
	{
		p_ate_val = str_power[0];
	}
	else if(0 == strcmp(value,"1"))
	{
		p_ate_val = str_power[1];
	}
	else
	{
		p_ate_val = str_power[2];
	}
	strcpy(ate_name_5g,"wl1_country_pwr_power");
	if(flag == NVRAM_ACTION_JUST_CHECK)
	{
		if((strcmp(ate_get_value(ate_name_5g),p_ate_val) != 0))
		{
			return ERROR;
		}
		return SUCCESS;
	}
	if((ate_set_value(ate_name_5g,p_ate_val) == ERROR))
	{
		 return ERROR;
	}

	return SUCCESS;

}
//end SUPPORT_POWER_5G

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
//MAXPOWER
static int ate_set_max_power_2g_5g(char *value,int flag)
{
    char *p_ate_val = NULL;
	char ate_name_2g[64] = {0};
	char ate_name_5g[64] = {0};
	char *str_power[] = {"low","normal","high"};

	if(NULL == value)
	{
		return ERROR;
	}
	//strcpy(ate_name,"country_power");
	strcpy(ate_name_2g,"wl0_ctv_power");
	strcpy(ate_name_5g,"wl1_ctv_power");

	if(0 == strcmp(value,"0"))
	{
		p_ate_val = str_power[0];
	}
	else if(0 == strcmp(value,"1"))
	{
		p_ate_val = str_power[1];
	}
	else
	{
		p_ate_val = str_power[2];
	}

	if(flag == NVRAM_ACTION_JUST_CHECK)
	{
		if((strcmp(ate_get_value(ate_name_2g),p_ate_val) == 0) && (strcmp(ate_get_value(ate_name_5g),p_ate_val) == 0))
		{
			return SUCCESS;
		}
		return ERROR;
	}
	if((ate_set_value(ate_name_2g,p_ate_val) == ERROR) || (ate_set_value(ate_name_5g,p_ate_val) == ERROR))
    {
    	return ERROR;
	}

	return SUCCESS;
}
//end MAXPOWER

//MAXPOWER_2G
static int ate_set_max_power_2g(char *value,int flag)
{
	char *p_ate_val = NULL;
	char ate_name_2g[64] = {0};
	char *str_power[] = {"low","normal","high"};

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name_2g,"wl0_ctv_power");

	if(0 == strcmp(value,"0"))
	{
		p_ate_val = str_power[0];
	}
	else if(0 == strcmp(value,"1"))
	{
		p_ate_val = str_power[1];
	}
	else
	{
		p_ate_val = str_power[2];
	}

	if(flag == NVRAM_ACTION_JUST_CHECK)
	{
		if((strcmp(ate_get_value(ate_name_2g),p_ate_val) == 0))
		{
			return SUCCESS;
		}
		return ERROR;
	}
	if((ate_set_value(ate_name_2g,p_ate_val) == ERROR))
	{
		return ERROR;
	}

	return SUCCESS;

}
//#end MAXPOWER_2G

//MAXPOWER_5G
static int ate_set_max_power_5g(char * value,int flag)
{
	char *p_ate_val = NULL;
	char ate_name_5g[64] = {0};
	char *str_power[] = {"low","normal","high"};

	if(NULL == value)
	{
		return ERROR;
	}
	strcpy(ate_name_5g,"wl1_ctv_power");

	if(0 == strcmp(value,"0"))
	{
		p_ate_val = str_power[0];
	}
	else if(0 == strcmp(value,"1"))
	{
		p_ate_val = str_power[1];
	}
	else
	{
		p_ate_val = str_power[2];
	}

	if(flag == NVRAM_ACTION_JUST_CHECK)
	{
		if((strcmp(ate_get_value(ate_name_5g),p_ate_val) == 0))
		{
			return SUCCESS;
		}
		return ERROR;
	}
	if((ate_set_value(ate_name_5g,p_ate_val) == ERROR))
	{
		return ERROR;
	}

	return SUCCESS;
}
//enf MAXPOWER_5G

//COUNTRY
static int ate_set_country(char *value,int flag)
{
	char ate_name[64] = {0};

	if(NULL == value)
	{
		return ERROR;
	}
	
	strcpy(ate_name,"country_code");
	
	if (flag == NVRAM_ACTION_JUST_CHECK)
	{
		if (strcmp(ate_get_value(ate_name),value) == 0)
			return SUCCESS;
		else
			return ERROR;
	}

	if (ate_set_value(ate_name, value) == ERROR)
	{
		return ERROR;
	}

	/* Indonesia has no 40MHz and 80MHz bandwidth in 5G band */
	if(0 == strcmp(value, "ID"))
	{
		ate_set_value(WLAN5G_BANDWIDTH, "20");
	}
	else
	{
		ate_unset_value(WLAN5G_BANDWIDTH);
	}
	
	return SUCCESS;
}

// end COUNTRY

#endif

static int ate_set_radio_parameter(PIU8 argc, PI8 *argv[],int flag)
{
	char ate_name[64] = {0};
	char *p_ate_val = NULL;

    char val_buf[1024] = {0};
	int access_write = 1;
	if(strncmp(argv[2],"ACCESS_WRITE",12)
		&& strcmp("off",ate_get_envram("ACCESS_WRITE")) == 0)
	{
		access_write = 0;
	}
    if (strncmp(argv[2],"HW_WLAN",7) == 0)
    {
        if (string_ate_to_hwpower(argv[3],val_buf))
        {
            argv[3] = val_buf;
        }
        else
        {
            return ERROR;
        }
    }

    strcpy(ate_name,argv[2]);
    p_ate_val = argv[3];
    if (flag == NVRAM_ACTION_JUST_CHECK)
    {
        if (strcmp(ate_get_value(ate_name),p_ate_val) == 0)
            return SUCCESS;
        else
            return ERROR;
    }

	if (access_write == 0 || ate_set_value(ate_name, p_ate_val) == ERROR)
    {
        return ERROR;
    }

	return SUCCESS;
}	

ATE_SET_INFO ate_set_module[] = 
{
	{"PIN",ate_set_pin},
	{"MAC",ate_set_mac},	
	{"SN",ate_set_sn},
	{"MTU_DHCP",ate_set_dhcp_mtu},
	{"MTU_PPPOE",ate_set_pppoe_mtu},
	{"MTU_STATIC",ate_set_static_mtu},
	{"MTU_PPTP",ate_set_pptp_mtu},
	{"MTU_L2TP",ate_set_l2tp_mtu},
	{"WPA_PW",ate_set_wpa_pw},
	{"PLC_PW",ate_set_plc_pw},
	{"WAN_MODE",ate_set_wan_mode},
	{"SSID",ate_set_ssid},
	{"WPS",ate_set_wps},
	{"Channel",ate_set_channel},
	{"Login_PW",ate_set_login_pw},
	{"IP",ate_set_ip},
	{"SUPPORT_POWER",ate_set_support_power_2g_5g},
	{"SUPPORT_POWER_2G",ate_set_support_power_2g},
	{"SUPPORT_POWER_5G",ate_set_support_power_5g},
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	{"MAXPOWER",ate_set_max_power_2g_5g},
	{"MAXPOWER_2G",ate_set_max_power_2g},
	{"MAXPOWER_5G",ate_set_max_power_5g},
	{"COUNTRY",ate_set_country},
#endif
	{"ELINK_SN",ate_set_elink_sn},
	{"CTEI_SN",ate_set_ctei_sn},

};

static PI8 ate_action_4(PIU8 argc, PI8 *argv[],PI8 *buf,int flag)
{
    char ate_name[64] = {0};
	int ret = ERROR;

	
	#if 1
	int i = 0;
	strcpy(ate_name,argv[2]);

	
	if (strcmp(argv[0], "nvram") == 0 && strcmp(argv[1], "set") == 0)
    {
		for(i = 0; i < ARRAY_SIZE(ate_set_module); i++)
		{
			if(0 == stricmp(ate_name,ate_set_module[i].key))
			{
				ret = ate_set_module[i].action(argv[3],flag);
				//printf("func:%s line:%d ret:%d\n",__func__,__LINE__,ret);
				break;
			}
		}
		if(i == ARRAY_SIZE(ate_set_module))
		{
			ret = ate_set_radio_parameter(argc,argv,flag);
		}
		
		if (flag == NVRAM_ACTION_JUST_CHECK)
		{
			return ret;
		}
		
		if (flag == NVRAM_ACTION_SET_CHECK)
	        {
	            if (ate_commit() != SUCCESS)
	            {
	                return ERROR;
	            }
	        }
		//printf("func:%s line:%d ret:%d\n",__func__,__LINE__,ret);
		return ret;
	}
	else if (strcmp(argv[0], "Tenda_mfg") == 0 && strcmp(argv[1], "check" )== 0 && strcmp(argv[2], "USB") == 0)
    {
    }
	#endif
    return ERROR;
}

static PI8 ate_action_6(PIU8 argc, PI8 *argv[],PI8 *buf)
{
#if 0
    if(strcmp(argv[0],"wlctrl")==0&&strcmp(argv[1],"set")==0&&strcmp(argv[2],"wl")==0)
    {
        PI8 *wl_cmd=NULL,*tmp=NULL;
        PI8 wl_buf[256]= {0},wl_buf2[256]= {0};
        PI32 wl_err = -1,nsetl = -1,ngetl1 = -1;
        PIU8 argc_wl;
        PI8 *argv_wl[32];
        tmp=argv[5];

        wl_set(argv[4], WLC_DOWN, &nsetl, sizeof(nsetl));
        wl_get(argv[4], WLC_GET_UP, &ngetl1, sizeof(ngetl1));

        if (ngetl1 != 0)
        {
            PI_PRINTF(ATE,"wl status is not [down] status,return false!!!\n");
            return ERROR;
        }

        while(tmp)
        {
            wl_cmd=strchr(tmp,',');
            if(wl_cmd!=NULL)
            {
                *wl_cmd='\0';
            }
            strcpy(wl_buf,tmp);
            sprintf(wl_buf2,"wl -i %s %s",argv[4],wl_buf);
            memset(argv_wl, 0, sizeof(argv_wl));
            argc_wl = get_argv(wl_buf2, argv_wl);
            wl_err=wl(argc_wl,argv_wl);

            if(wl_err!=0)
            {
                sprintf(buf,"%s ,excute error!\n",wl_buf);
                return ERROR;
            }

            if(!strcmp(wl_buf,"counters"))
            {
                wl_get_buf(buf);
            }

            if(wl_cmd!=NULL)
            {
                tmp=wl_cmd+1;
            }
            else
            {
                tmp=NULL;
            }
        }

        if(wl_err==0)
        {
            return SUCCESS;
        }
        else
        {
            return ERROR;
        }

        wl_set(argv[4], WLC_UP, &nsetl, sizeof(nsetl));
        wl_get(argv[4], WLC_GET_UP, &ngetl1, sizeof(ngetl1));
        if ( ngetl1 != 1)
        {
            PI_PRINTF(ATE,"wl status is not [up] status,return false!!!\n");
            return ERROR;
        }
    }
#endif
    return ERROR;
}

static PI8 ate_handle(PIU8 argc, PI8 *argv[],PI8 *buf,int flag)
{
	PI8 result = ERROR;

    switch (argc)
    {
        case 2://Tenda_mfg xx or ledtest xx
            result = ate_action_2(argc,argv,buf);
            break;
        case 3://wlctrl
            result = ate_action_3(argc,argv,buf);
            break;
        case 4://nvram set && Tenda_mfg check USB
            result = ate_action_4(argc,argv,buf,flag);
            break;
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
        case 6: //wlctrl set wl -i eth1 -- interactive,mpc,ap,ALL
            result = ate_action_6(argc,argv,buf);
            break;
#endif
        default:
            result = ERROR;
    }
    return result;
}

static void ate_send(PI8 *buf,PIU32 len)
{
    PI_PRINTF(ATE,"ate send [%s]!\n",buf);

    if(sendto(ate_fd, buf, len, 0, &ate_from, sizeof(ate_from)) <= 0)
        PI_ERROR(ATE,"send error!\n");

    return;
}

/*
 * Wrapper to extract some Wireless Parameter out of the driver
 */
#define MP_QUERY_STATS  0x8B6D
#define MP_QUERY_TSSI   0x8B6F
#define MP_QUERY_THER 0x8B77

static int iw_get_ext(int skfd,    /* Socket to the kernel */
                      char *ifname,           /* Device name */
                      int request,                /* WE ID */
                      struct iwreq *pwrq)    /* Fixed part of the request */
{
    strncpy(pwrq->ifr_name, ifname, IFNAMSIZ);  /* Set device name */
    return(ioctl(skfd, request, pwrq));         /* Do the request */
}

static int MP_get_ext(char *ifname, char *buf, unsigned int ext_num)
{
    int skfd;
    struct iwreq wrq;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    wrq.u.data.pointer = (caddr_t)buf;
    wrq.u.data.length = strlen(buf);

    if (iw_get_ext(skfd, ifname, ext_num, &wrq) < 0)
    {
        printf("MP_get_ext failed\n");
        return -1;
    }

    close(skfd);
    return 0;
}

void dump_argv(int argc, char **argv)
{
    int i = 0;
    while(argv--)
    {
        printf("%s ",argv[i++]);
    }
    printf("\n");
}

static void ate_main(void)
{
    PI8 recvbuf[4096], sendbuf[2048],buf[1024], tem_buf[1024];
    char cmd_buf[20][64] = {0};
    PIU8 argc;
    PI8 *argv[32];
	PI8 result;
    PI8 *next = NULL;
    int is_first_cmd; // 当同时有几条命令一起提交时，用来确定标记第一条命令
	char wlan_index[16] = {0};
    PI8 cmd_type[2][16] = {"iwpriv", "nvram set",};
    char mp_query_buf[48] = {0};
	int tx_ok, tx_fail, rx_ok, rx_fail;

    int line = 0;
    /*
    *第一步:创建套接字
    *第二步:关闭按键相应的功能
    *第三步:接收数据,并同时监听按键触发所有LED全亮或者全灭
    *第四步:针对数据进行响应
    *第五步:把相应的结果发送数据出去
    */

    if(-1 == ate_open_socket())
    {
        return;
    }

    PI_PRINTF(ATE,"ate start loop.\n");

    sys_button_change_ate_mode(ATE_RUN);

    ate_change_sys_led_timer_status(DO_TIMER_OFF);
#ifdef __CONFIG_EXTEND_LED__
    ate_change_extend_led_timer_status(DO_TIMER_OFF);//关闭扩展LED灯定时器
#endif

    while(1)
    {
		tx_ok = 0;
		tx_fail = 0; 
		rx_ok = 0; 
		rx_fail = 0;
		
        ate_button_poll();

        memset(recvbuf,0x0,sizeof(recvbuf));
        if(ate_recv(recvbuf,sizeof(recvbuf)) <= 0)
            continue;

        PI_PRINTF(ATE,"ate recv [%s]\n",recvbuf);
        memset(buf,0x0,sizeof(buf));
        memset(sendbuf,0x0,sizeof(sendbuf));

		if(!strncmp(recvbuf, cmd_type[0], strlen(cmd_type[0]))) //iwpriv
		{
			if(!strncmp(recvbuf,"iwpriv wlan0", strlen("iwpriv wlan0")))
			{
				strncpy(wlan_index,"wlan0",strlen("wlan0"));
			}else
			{
				strncpy(wlan_index,"wlan1",strlen("wlan1"));
			}
		}
		
        // 处理无线命令，格式不同
        if(NULL != strchr(recvbuf, ';'))
        {
            if(!strncmp(recvbuf, cmd_type[1], strlen(cmd_type[1]))) // nvram
            {
                is_first_cmd = 1;
                result = SUCCESS;
                /* 第一步:修改内存中的nvram */
                foreach_arch(buf, recvbuf, next, ";", ';')
                {
                    memset(tem_buf,0x0,sizeof(tem_buf));
                    if('=' == buf[(int)strlen(buf) - 1])
                        continue;
                    memset(argv, 0, sizeof(argv));
                    if(1 == is_first_cmd)
                    {
                        is_first_cmd = 0;
                        argc = get_argv(buf, argv);
                    }
                    else
                    {
                        snprintf(tem_buf, sizeof(tem_buf), "%s %s", cmd_type[1],buf);
                        argc = get_argv(tem_buf, argv);
                    }
                    //dump_argv(argc,argv);
                    if (argc == 3)
                    {
                        char *p = strchr(argv[2],'=');
                        if (p)
                        {
                            *p = '\0';
                            if (*(p+1) == '\0')
                            {
                                printf("argv=%s continue\n",argv[2]);
                                continue;
                            }

                            argv[3] = p+1;
                            argc = 4;
                        }
                        else
                        {
                            result = ERROR;
                        }
                    }
                    if (ate_handle(argc,argv,buf,NVRAM_ACTION_JUST_SET) == ERROR)
                    {
                        result = ERROR;
                        break;
                    }
                }

                /* 第二步:将nvram保存到flash中 */
                result = ate_commit();
                /*
                if (result == SUCCESS) {
                    result = ate_set_nvram_restore();
                }
                */
                /* 第三步:检查nvram是否设置正确 */
                memset(buf,0x0,sizeof(buf));
                is_first_cmd = 1;
                foreach_arch(buf, recvbuf, next, ";", ';')
                {
                    memset(tem_buf,0x0,sizeof(tem_buf));
                    if('=' == buf[(int)strlen(buf) - 1])
                        continue;
                    memset(argv, 0, sizeof(argv));
                    if(1 == is_first_cmd)
                    {
                        is_first_cmd = 0;
                        argc = get_argv(buf, argv);
                    }
                    else
                    {
                        snprintf(tem_buf, sizeof(tem_buf), "%s %s", cmd_type[1],buf);
                        argc = get_argv(tem_buf, argv);
                    }
                    if (argc == 3)
                    {
                        char *p = strchr(argv[2],'=');
                        if (p)
                        {
                            *p = '\0';
                            if (*(p+1) == '\0')
                            {
                                printf("argv=%s continue\n",argv[2]);
                                continue;
                            }

                            argv[3] = p+1;
                            argc = 4;
                        }
                        else
                        {
                            result = ERROR;
                        }
                    }
                    if (ate_handle(argc,argv,buf,NVRAM_ACTION_JUST_CHECK) == ERROR)
                    {
                        result = ERROR;
                        break;
                    }
                }

            }
            else if(!strncmp(recvbuf, cmd_type[0], strlen(cmd_type[0]))) //iwpriv
            {
                is_first_cmd = 1;			
                foreach_arch(buf, recvbuf, next, ";", ';')
                {
                    memset(tem_buf,0x0,sizeof(tem_buf));
                    if(1 == is_first_cmd)
                    {
                        is_first_cmd = 0;
                        strncpy(tem_buf, buf, sizeof(tem_buf));
                    }
                    else
                        snprintf(tem_buf, sizeof(tem_buf), "%s %s %s", cmd_type[0],wlan_index,buf);
                    printf("run_clicmd=%s\n",tem_buf);
                    if (strcmp(buf,"mp_query") == 0)
                    {
                        MP_get_ext(wlan_index, tem_buf, MP_QUERY_STATS);
                        printf("---mp_query=%s\n",tem_buf);
                        //sscanf(tem_buf,"Tx OK:%d, Tx Fail:%d, Rx OK:%d, CRC error:%d",&a,&a,&rx_ok,&a);
                        //printf("---rx_ok=%d\n",rx_ok);
                        snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
                    }
                    else if (strcmp(buf,"mp_ther") == 0)
                    {
                        MP_get_ext(wlan_index, tem_buf, MP_QUERY_THER);
                        printf("---mp_ther=%s\n",tem_buf);
                        snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
                    }
                    else if (strcmp(buf,"mp_tssi") == 0)
                    {
                        MP_get_ext(wlan_index, tem_buf, MP_QUERY_TSSI);
                        printf("---mp_tssi=%s\n",tem_buf);
                        snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
                    }
                    else
                    {
                        if(NULL != strstr(recvbuf,"mp_reset_rx"))
                            strcpy(cmd_buf[line++], tem_buf);
                        run_clicmd(tem_buf);
                    }
                }
	            result = SUCCESS;
	            //sprintf(buf,"%d",rx_ok);
	            sprintf(buf,"%s",mp_query_buf);
			}		
        }
		//多条结束
        else if (!strncmp(recvbuf, cmd_type[0], strlen(cmd_type[0])))
        {

            if (strstr(recvbuf,"mp_query"))
            {
                memset(tem_buf,0x0,sizeof(tem_buf));
                MP_get_ext(wlan_index, tem_buf, MP_QUERY_STATS);
				sscanf(tem_buf, "Tx OK:%d, Tx Fail:%d, Rx OK:%d, CRC error:%d", &tx_ok, &tx_fail, &rx_ok, &rx_fail);
               
				printf("---1mp_query=%s, rx_ok = %d\n",tem_buf, rx_ok);
				
				if(rx_ok == 0)
				{
					printf("jamtest rx_ok = %d\n", rx_ok);
					int singleLine;

					for (singleLine = 0; singleLine < line; singleLine++)
					{
						printf("jamtest %s\n", cmd_buf[singleLine]);
						run_clicmd(cmd_buf[singleLine]);
					}
					line = 0;
					printf("mp_reset_rx OK!!recvbuf=%s!!\n", recvbuf);

				}

		
                snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
            }
            else if (strstr(recvbuf,"mp_ther"))
            {
                memset(tem_buf,0x0,sizeof(tem_buf));
                MP_get_ext(wlan_index, tem_buf, MP_QUERY_THER);
                printf("---mp_ther=%s\n",tem_buf);
                snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
            }
            else if (strstr(recvbuf,"mp_tssi"))
            {
                memset(tem_buf,0x0,sizeof(tem_buf));
                MP_get_ext(wlan_index, tem_buf, MP_QUERY_TSSI);
                printf("---mp_tssi=%s\n",tem_buf);
                snprintf(mp_query_buf,sizeof(mp_query_buf),"%s",tem_buf);
            }
            else
            {
                printf("run_clicmd=%s\n",recvbuf);
                run_clicmd(recvbuf);
            }
            result = SUCCESS;
            //sprintf(buf,"%d",rx_ok);
            sprintf(buf,"%s",mp_query_buf);
        }
		
        else if (!strncmp(recvbuf, "ifconfig", 8))
        {
            printf("run_clicmd=%s\n",recvbuf);
            run_clicmd(recvbuf);
            result = SUCCESS;
        }
        else
        {
            memset(argv, 0, sizeof(argv));
            argc = get_argv(recvbuf, argv);

            result = ate_handle(argc,argv,buf,NVRAM_ACTION_SET_CHECK);
        }

        if (result == SUCCESS)
        {
            strcpy(sendbuf, "success");
            if(strlen(buf))
            {
                strcat(sendbuf,":");
                strcat(sendbuf,buf);
            }
        }
        else if(result == INFO)
        {
            strcpy(sendbuf, buf);
        }
        else
        {
            strcpy(sendbuf, "error");
            if(strlen(buf))
            {
                strcat(sendbuf,":");
                strcat(sendbuf,buf);
            }
        }
                
        ate_send(sendbuf,strlen(sendbuf));
    }
    ate_running = 0;
    sys_button_change_ate_mode(ATE_EXIT);
    ate_change_sys_led_timer_status(DO_TIMER_ON);
#ifdef __CONFIG_EXTEND_LED__
    ate_change_extend_led_timer_status(DO_TIMER_ON);//打开扩展器LED灯定时器
#endif
    return;
}
#ifdef CONFIG_TENDA_ATE_REALTEK
extern void create_realtek_mp_daemon(void);
#endif

void ate_start(void)
{
    if(ate_running == 0)
    {
        cyg_thread_create(
            7,
            (cyg_thread_entry_t *)ate_main,
            0,
            "ate_daemon",
            ate_stack,
            sizeof(ate_stack),
            &ate_thread_handle,
            &ate_thread_struct);

        ate_running = 1;

        cyg_thread_resume(ate_thread_handle);
    }
#ifdef CONFIG_TENDA_ATE_REALTEK
    create_realtek_mp_daemon();
#endif
}
