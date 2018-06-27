#include <stdlib.h>
#include <stdio.h>
 #include <unistd.h>

#include <typedefs.h>
#include <proto/ethernet.h>
#include <bcmparams.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <shutils.h>
#include <wlif_utils.h>
#include <netconf.h>
#include <nvparse.h>
#include <wlutils.h>
//#include <bcmcvar.h>
//#include <ezc.h>
#include <bcmconfig.h>
#include <opencrypto.h>
#include <time.h>
#include <epivers.h>
#include "router_version.h"
#include <proto/802.11.h>
#include <proto/802.1d.h>
#include <security_ipc.h>

#ifdef __CONFIG_WPS__
#include <wps_ui.h>
#endif

//#include <wps_hal.h>

#include "webs.h"
#include "uemf.h"

#ifdef __ECOS
#include <sys/select.h>
#include <sys/time.h>
#define CLOCK_REALTIME	0
#endif

typedef enum wps_blinktype {
	WPS_BLINKTYPE_INPROGRESS = 0,
	WPS_BLINKTYPE_ERROR,
	WPS_BLINKTYPE_OVERLAP,
	WPS_BLINKTYPE_SUCCESS,
	WPS_BLINKTYPE_STOP,
	WPS_BLINKTYPE_STOP_MULTI
} wps_blinktype_t;

#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_on(void);
extern void wps_led_test_off(void);
static int wps_led_on_status=0;
#endif /*__CONFIG_WPS_LED__*/

extern void wps_hal_led_blink(wps_blinktype_t blinktype);
extern void udelay(int delay);
extern void wps_start(void);
extern void wps_stop(void);

#define SLEEP(n)	cyg_thread_delay(n * 100)
#define USLEEP(X)	udelay(X)
#define NVRAM_BUFSIZE	256
static char wps_unit[32];

#ifndef HZ
#define HZ		100
#endif
#define WPS_CHECK_TIME (HZ) 	// 1 SEc
#define WPS_AP_CATCH_CONFIGURED_TIMER WPS_CHECK_TIME
#define WPS_AP_TIMEOUT_SECS (WPS_CHECK_TIME*60*2)	 // 2 Minutes

int g_wps_timer_state=0;//0: stop wps negotiate; 1: start wps negotiate
int wps_init_finish;
char_t uibufbak[256];

static void backup_wps_env(char *uibuf);
static void start_wps_timerother(void);
static void wps_set_env_handler(void);

static void stop_wps_timer(void);
static void start_wps_timer(void);
static void reset_wps_timerall(void);
static void wps_timer_handler(void);
static void start_wps_timerall(void);

static int wps_process(void);
static int is_wps_enabled(void);
static int set_wps_env(char *uibuf);
static int wps_get_lan_idx(void);
static char *make_wl_prefix(char *prefix,int prefix_size, int mode, char *ifname);

 void wps_oob(const char *flag);
 void set_wireless_wps(webs_t wp, char_t *path, char_t *query);

/* This utility routine builds the wl prefixes from wl_unit.
 * Input is expected to be a null terminated string
 *
 * Inputs -prefix: Pointer to prefix buffer
 *	  -prefix_size: Size of buffer
 *        -Mode flag: If set generates unit.subunit output
 *                    if not set generates unit only
 *	  -ifname: Optional interface name string
 *
 *
 * Returns - pointer to prefix, NULL if error.
 *
 *
*/
static char *make_wl_prefix(char *prefix,int prefix_size, int mode, char *ifname)
{
	int unit=-1,subunit=-1;
	char *wl_unit=NULL;

	//assert(prefix);
	//assert(prefix_size);

	if (ifname){
		//assert(*ifname);
		wl_unit=ifname;
	}else{
		wl_unit=nvram_get("wl_unit");

		if (!wl_unit)
			return NULL;
	}

	if (get_ifname_unit(wl_unit,&unit,&subunit) < 0 )
		return NULL;

	if (unit < 0) return NULL;

	if  ((mode) && (subunit > 0 ))
		snprintf(prefix, prefix_size, "wl%d.%d_", unit,subunit);
	else
		snprintf(prefix, prefix_size, "wl%d_", unit);

	return prefix;
}


static int wps_process(void)
{
	char_t *status;
	int	wps_init = 0;
	
	status = nvram_safe_get("wps_proc_status");
	
	switch (atoi(status)) {
	case 1: /* WPS_ASSOCIATED */
		diag_printf("Processing WPS start\n");
		break;
	case 2: /* WPS_OK */
	case 7: /* WPS_MSGDONE */
		diag_printf( "Success\n");
		break;
	case 3: /* WPS_MSG_ERR */
		diag_printf("Fail due to WPS message exange error!\n");
		break;
	case 4: /* WPS_TIMEOUT */
		diag_printf("Fail due to WPS time out!\n");
		break;
	default:
		diag_printf("Init, wps_proc_status = %d\n", atoi(status));
		wps_init = 1;
		break;
	}
	
	if(!wps_init)
		wps_init_finish = 1;
	
	return atoi(status);
}

static int is_wps_enabled()
{
	int i, unit;
	char *ifnames = NULL, *next = NULL;
	char prefix[] = "wlXXXXXXXXXX_";
	char name[IFNAMSIZ] = {0};
	char os_name[IFNAMSIZ] = {0};
	char wl_name[IFNAMSIZ] = {0};
	char lan_name[IFNAMSIZ] = {0};
	char tmp[100] = {0};
	char *wps_mode = NULL;
	char *wl_radio = NULL;
	char *wl_bss_enabled = NULL;

	/* (LAN) */
	for (i = 0; i < 256; i ++) {
		/* Taking care of LAN interface names */
		if (i == 0) {
			strcpy(name, "lan_ifnames");
			strcpy(lan_name, "lan");
		}
		else {
			sprintf(name, "lan%d_ifnames", i);
			sprintf(lan_name, "lan%d", i);
		}

		ifnames = nvram_get(name);
		if (!ifnames)
			continue;

		/* Search for wl_name in ess */
		foreach(name, ifnames, next) {
			if (nvifname_to_osifname(name, os_name, sizeof(os_name)) < 0)
				continue;
			if (wl_probe(os_name) ||
				wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
				continue;

			/* Convert eth name to wl name */
			if (osifname_to_nvifname(name, wl_name, sizeof(wl_name)) != 0)
				continue;

			/* Get configured wireless address */
			snprintf(prefix, sizeof(prefix), "%s_", wl_name);

			/* Ignore radio or bss is disabled */
			snprintf(tmp, sizeof(tmp), "wl%d_radio", unit);
			wl_radio = nvram_safe_get(tmp);
			wl_bss_enabled = nvram_safe_get(strcat_r(prefix, "bss_enabled", tmp));
			if (strcmp(wl_radio, "1") != 0 || strcmp(wl_bss_enabled, "1") != 0)
				continue;
			if (nvram_get(strcat_r(prefix, "hwaddr", tmp)) == NULL)
				continue;

			/* Enabled/ Disabled */
			wps_mode = nvram_get(strcat_r(prefix, "wps_mode", tmp));
			if (!wps_mode ||
				(strcmp(wps_mode, "enabled") != 0 &&
				strcmp(wps_mode, "enr_enabled") != 0)) {
				continue;
			}

			/* got it enabled, wps is running */
			return 1;
		}
	}

	/* (WAN) */
	for (i = 0; i < 256; i ++) {
		/* Taking care of WAN interface names */
		if (i == 0)
			strcpy(name, "wan_ifnames");
		else
			sprintf(name, "wan%d_ifnames", i);

		ifnames = nvram_get(name);
		if (!ifnames)
			continue;

		/* Search for wl_name in it */
		foreach(name, ifnames, next) {
			if (nvifname_to_osifname(name, os_name, sizeof(os_name)) < 0)
				continue;
			if (wl_probe(os_name) ||
				wl_ioctl(os_name, WLC_GET_INSTANCE, &unit, sizeof(unit)))
				continue;

			/* Convert eth name to wl name */
			if (osifname_to_nvifname(name, wl_name, sizeof(wl_name)) != 0)
				continue;

			/* Get configured wireless address */
			snprintf(prefix, sizeof(prefix), "%s_", wl_name);

			/* Ignore radio or bss is disabled */
			snprintf(tmp, sizeof(tmp), "wl%d_radio", unit);
			wl_radio = nvram_safe_get(tmp);
			wl_bss_enabled = nvram_safe_get(strcat_r(prefix, "bss_enabled", tmp));
			if (strcmp(wl_radio, "1") != 0 || strcmp(wl_bss_enabled, "1") != 0)
				continue;
			if (nvram_get(strcat_r(prefix, "hwaddr", tmp)) == NULL)
				continue;

			/* Enabled/ Disabled */
			wps_mode = nvram_get(strcat_r(prefix, "wps_mode", tmp));
			if (!wps_mode || strcmp(wps_mode, "enr_enabled") != 0)
				continue;

			/* got it enabled, wps is running */
			return 1;
		}
	}

	return 0;
}


static int set_wps_env(char *uibuf)
{
	int wps_fd = -1;
	struct sockaddr_in to;
	int sentBytes = 0;
	uint32 uilen = strlen(uibuf);
	
	if ((wps_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		goto exit;
	}
	
	/* send to WPS */
	to.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	to.sin_family = AF_INET;
	to.sin_port = htons(WPS_UI_PORT);

	sentBytes = sendto(wps_fd, uibuf, uilen, 0, (struct sockaddr *) &to,
		sizeof(struct sockaddr_in));

	if (sentBytes != uilen) {
		goto exit;
	}

	/* pxy rm, it seems to have no effect */
#if 0
	/* Sleep 120 s to make sure
	   WPS have received socket */
	USLEEP(120*1000);
	diag_printf("USLEEP(120*1000) OVER: return\n");
#endif	
	close(wps_fd);
	return 0;

exit:
	if (wps_fd >= 0)
		close(wps_fd);

	/* Show error message ?  */
	return -1;
}

void stop_wps_when_failed(void)
{
	char uibuf[256] = "SET ";
	int uilen = 4;
	
	uilen += sprintf(uibuf + uilen, "wps_config_command=%d ", WPS_UI_CMD_STOP);
	uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_NONE);
	set_wps_env(uibuf);
	diag_printf("%s\n", uibuf);
	
 	reset_wps_timerall();
}

  void stop_wireless_wps()
{
	  char_t uibuf[256] = "SET ";
	  int uilen = 4; 
	 uilen += sprintf(uibuf + uilen, "wps_config_command=%d ", WPS_UI_CMD_STOP);
	 uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_NONE);
	 set_wps_env(uibuf);
	 diag_printf("%s\n", uibuf);
	 wps_hal_led_blink(12);
	 reset_wps_timerall();
	  return ;
}
  
 void set_wireless_wps(webs_t wp, char_t *path, char_t *query)
{
	char nvifname[IFNAMSIZ], osifname[IFNAMSIZ];		
	char_t pin[9]={0};
	char_t uibuf[256] = "SET ";
	int uilen = 4;
	char_t *mode,  *enable, *oldpin;
	
	enable = websGetVar(wp, T("wpsEn"), T("enabled"));
	mode = websGetVar(wp, T("wpsmethod"), T("pbc"));
	oldpin = websGetVar(wp, T("PIN"), T("0"));	

	if(strcmp(enable,"true") == 0)
		enable = "enabled";
	else if(strcmp(enable,"false") == 0)
		enable = "disabled";
	
	//diag_printf("WPS enable:%s\n", enable);
	//diag_printf("WPS wpsmode:%s\n", mode);
	//diag_printf("WPS oldpin:%s\n", oldpin);
	
	if(0 == strcmp(enable, "enabled"))		//enabled wps
	{
		strcpy(pin, oldpin);
		diag_printf("enable wps...\n");
		
		if (0 == strcmp(mode, "pbc") || (0 == strcmp(mode, "pin") && 
			/*0 != strcmp(pin, "0") &&*/ 0 != strcmp(pin, "00000000"))) 						//add enrollee
		{		
			if(0 == strcmp(mode, "pin")){
				/*input station pin code on the router wps web page.*/
				nvram_set(WLN0_WPS_METHOD, "pin");
				uilen += sprintf(uibuf + uilen, "wps_method=%d ", WPS_UI_METHOD_PIN);
				nvram_set(WLN0_WPS_PIN, pin);
			}else if(0 == strcmp(mode, "pbc")){
				nvram_set(WLN0_WPS_METHOD, "pbc");
				nvram_set(WLN0_WPS_PIN, "");
				strncpy(pin, "00000000", 8);
				uilen += sprintf(uibuf + uilen, "wps_method=%d ", WPS_UI_METHOD_PBC);		
			}
			uilen += sprintf(uibuf + uilen, "wps_sta_pin=%s ", pin);
			uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_ADDENROLLEE);
		}
		else if (0 == strcmp(mode, "pin") &&
			(0 == strcmp(pin, "0") || 0 == strcmp(pin, "00000000"))) 		// config AP
		{	/*input router pin code on the station UI*/
			uilen += sprintf(uibuf + uilen, "wps_method=%d ", WPS_UI_METHOD_PIN);
			uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_CONFIGAP);
			nvram_set(WLN0_WPS_PIN, "");
		}
		
		nvram_set("wps_proc_status", "0");
		strcpy(wps_unit, nvram_safe_get("wl_unit"));
		uilen += sprintf(uibuf + uilen, "wps_config_command=%d ", WPS_UI_CMD_START);
		sprintf(nvifname, "wl%s", wps_unit);
		
		diag_printf("nvifname:%s\n", nvifname);
		
		nvifname_to_osifname(nvifname, osifname, sizeof(osifname));

		uilen += sprintf(uibuf + uilen, "wps_pbc_method=%d ", WPS_UI_PBC_SW);
		uilen += sprintf(uibuf + uilen, "wps_ifname=%s ", osifname);
		diag_printf("[set_wireless_wps]:: uibuf:%s\n", uibuf);

		set_wps_env(uibuf);
		start_wps_timerall();

		return ;
		
	}
	else if(0 == strcmp(enable, "disabled"))		//disabled wps
	{		
		char uibuf[256] = "SET ";
		int uilen = 4;
		
		nvram_set(WLN0_WPS_ENABLE, "disabled");

		uilen += sprintf(uibuf + uilen, "wps_config_command=%d ", WPS_UI_CMD_STOP);
		uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_NONE);
		set_wps_env(uibuf);
		diag_printf("%s\n", uibuf);
		wps_hal_led_blink(12);
		
	 	reset_wps_timerall();
		return ;	
	}
	
	return ;
}

void tenda_stop_wps_timer(void)
{
//call by reset oob
	if(g_wps_timer_state ==1)
		reset_wps_timerall();
	
#ifdef __CONFIG_WPS_LED__
//some times the led is turn off after reset oob, so turn on it here
	if(wps_led_on_status==0){
		wps_led_on_status=1;
		wps_led_test_on();
		
	}
#endif
	return;
}

/* Check lan index according to wl_unit */
static int wps_get_lan_idx(void)
{
	int i;
	char prefix[] = "wlXXXXXXXXXX_";
	char *wl_ifname, *lan_ifnames;
	char tmp[NVRAM_BUFSIZE];

	if (!make_wl_prefix(prefix,sizeof(prefix), 1, NULL))
		return -1; /* Error */

	wl_ifname = nvram_safe_get(strcat_r(prefix, "ifname", tmp));

	/* ldf revise */
#if 0
	/* find wlx_ifname in lan_ifnames */
	for (i = 0; i< MAX_NVPARSE; i++) {
	//for (i = MAX_NVPARSE; i> -1; i--) {
		if (i == 0)
			snprintf(tmp, sizeof(tmp), "lan_ifnames");
		else
			snprintf(tmp, sizeof(tmp), "lan%d_ifnames", i);

		lan_ifnames = nvram_get(tmp);
		if (lan_ifnames && find_in_list(lan_ifnames, wl_ifname))
			break;
	}

	if (i == MAX_NVPARSE)
		return -1;
#else
	if(strcmp(wl_ifname, "eth1") == 0)
		return 0;
	else if(strcmp(wl_ifname, "wl0.1") == 0)
		return 1;
	else
		return -1;
#endif

	/* ldf rm */
	//return i;
}

void wps_oob(const char *flag)
{
	int lan_idx;
	char tmp[NVRAM_BUFSIZE]={'\0'};

	lan_idx = wps_get_lan_idx();
	if (lan_idx == -1){
		diag_printf("In wps_oob function: wps_get_lan_idx error\n");
		return ;
	}

	/* found, get lanx_wps_oob */
	if (lan_idx == 0)
		snprintf(tmp, sizeof(tmp), "lan_wps_oob");
	else
		snprintf(tmp, sizeof(tmp), "lan%d_wps_oob", lan_idx);

	/*
	 * OOB: enabled
	 * Configured: disabled
	 */
	 if(0 == strcmp(flag, "unconfigured") )//OOB: enabled
	 {
	 	
		if (nvram_match(tmp, "disabled")){	
			nvram_set(tmp, "enabled");	/* OOB (Unconfigued), wps fail or reset OOB*/
		}
		//diag_printf("Configured, wps fail .  lan_wps_oob=enabled\n");
#ifdef __CONFIG_WPS_LED__
		wps_led_test_on();		//add by stanley 2010/10/25
#endif /*__CONFIG_WPS_LED__*/
		//diag_printf("In %s wps led blink...\n", __FUNCTION__);
	 }else if(0 == strcmp(flag, "configured")){//Configured: disabled
	 	char_t *wps_ssid;
		if(nvram_match(tmp, "enabled"))
			nvram_set(tmp, "disabled");	/* Configured, wps success */

/*liuke modify @ 2011/11/25*/
#if 0		
		wps_ssid = nvram_get(WLN0_WPS_SSID);
		nvram_set(WLN0_SSID0, wps_ssid);

	//printf("\n\n----------------Return to default factory WPS SSID-----------------\n\n\n");	//MM
#else
		//wps_ssid = nvram_get(WLN0_SSID0);
		//nvram_set(WLN0_WPS_SSID, wps_ssid);
#endif
/*liuke modify end*/

		//diag_printf("Configured, wps success .  lan_wps_oob=disable\n");
#ifdef __CONFIG_WPS_LED__
		wps_led_test_on();		//add by staley  2010/10/25
#endif/*__CONFIG_WPS_LED__*/	
	 }
	return ;
}

static void stop_wps_timer(void)
{
	untimeout((timeout_fun *)&wps_timer_handler,0);
	return;
}

static void start_wps_timer(void)
{
	timeout((timeout_fun *)&wps_timer_handler, 0, (WPS_CHECK_TIME));
	return;
}

/*
 * pxy add, 2013.12.03
 */
#if 1
static void start_wps_timerother(void)
{
	untimeout((timeout_fun *)&wps_set_env_handler, 0);
	timeout((timeout_fun *)&wps_set_env_handler, 0, (WPS_CHECK_TIME * 5));
	
	return;
}
#endif

static void reset_wps_timerall(void)
{
	stop_wps_timer();
	g_wps_timer_state = 0;
	/* pxy add, 2013.12.03, reset wps init status */
	wps_init_finish = 0;
	
	return ;
}

static void start_wps_timerall(void)
{
	reset_wps_timerall();
	start_wps_timer();
	return;
}

void start_wps_led_button_timerall(void){
	start_wps_timerall();
	return;
}

static void wps_timer_handler(void)
{
	int wps_status = 0;
	static int wsc_timeout_counter = 0;

	wps_status = wps_process();

	if(wps_status==2 || wps_status ==7){
			wps_oob("configured");	/*wps success*/
			diag_printf("wps_oob(configured)\n");				
#ifdef __CONFIG_WPS_LED__
			if(wps_led_on_status == 0){
				wps_led_on_status=1;
				wps_led_test_on();
			}
#endif
			untimeout((timeout_fun *)&wps_led_test_off, 0);
			timeout((timeout_fun *)wps_led_test_off, NULL, 300 * 100);//300ÃëÖ®ºó¹Ø±ÕwpsµÆ
			reset_wps_timerall();
			return ;
	}
	

	if(wps_status ==  1 && g_wps_timer_state == 0){
		/*In wps process, wps led will blink*/
	//	diag_printf("wps led blink...\n");
#ifdef __CONFIG_WPS_LED__
		wps_led_test_on();
		wps_led_on_status=1;
#else
		wps_hal_led_blink(WPS_BLINKTYPE_INPROGRESS);//ok
#endif
		g_wps_timer_state = 1;
	}

#ifdef __CONFIG_WPS_LED__
	if(wps_led_on_status==0){
		wps_led_on_status=1;
		wps_led_test_on();
		
	}
	else{
		wps_led_on_status=0;
		wps_led_test_off();
	}
#endif
	wsc_timeout_counter += WPS_AP_CATCH_CONFIGURED_TIMER;
	if(wsc_timeout_counter > WPS_AP_TIMEOUT_SECS/* || wps_status != 1 wps process*/){	//wps process time out, when timer over 2m
		wsc_timeout_counter = 0;
		
		if((wps_status ==  2 || wps_status == 7) && g_wps_timer_state == 1){
			/*when wps successed, wps led will off.*/
#ifdef __CONFIG_WPS_LED__
			if(wps_led_on_status == 0){
				wps_led_on_status=1;
				wps_led_test_on();
			}
#endif			
		}else{
#if 0
			/*when wps timeout, wps led will  light off. */
			if (wps_is_enable())	//add by stanley 2010/12/30 wait test
			{	
				wps_oob("unconfigured");	/*wps fial*/
				sys_restart();//add by stanley 12/6
			}
			cyg_thread_delay(200);	//add by stanley 12/6
#ifdef __CONFIG_WPS_LED__
			if(wps_led_on_status == 0)
			{
				wps_led_on_status=1;
				wps_led_test_on();
			}
#else
			wps_hal_led_blink(WPS_BLINKTYPE_STOP);
#endif
#else
//roy modify,20110113
			stop_wps_when_failed();
			cyg_thread_delay(200);	
#ifdef __CONFIG_WPS_LED__
			if(wps_led_on_status == 1)
			{
				wps_led_on_status=0;
				wps_led_test_off();
			}
#else
			wps_hal_led_blink(WPS_BLINKTYPE_STOP);
#endif	
			return;
#endif			
		}
		/* pxy add, 2013.12.03, reset wsc_timeout_counter */
		wsc_timeout_counter = 0;
		reset_wps_timerall();
		return ;
	}

	start_wps_timer();
	
	return;
}

/*
 * pxy add, 2013.12.03
 */
#if 1
static void wps_set_env_handler(void)
{
	if(!strncmp(uibufbak,"SET ", 4) && !wps_init_finish){
		set_wps_env(uibufbak);
		timeout((timeout_fun *)&wps_set_env_handler, 0, (WPS_CHECK_TIME * 5));
	}
	else{
		untimeout((timeout_fun *)&wps_set_env_handler, 0);
	}

	return;
}

static void backup_wps_env(char *uibuf)
{
	memset(uibufbak, 0, sizeof(uibufbak));

	strcpy(uibufbak, uibuf);
}
#endif

 int ComputeChecksum(
	unsigned int PIN)
{
	int digit_s;
    unsigned int accum = 0;

	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 
	accum += 1 * ((PIN / 1000000) % 10); 
	accum += 3 * ((PIN / 100000) % 10); 
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10); 

	digit_s = (accum % 10);
	return ((10 - digit_s) % 10);
} // ComputeChecksum


void set_pin_code(void){
	char devPwd[10] ;
	unsigned int iPin=0, checksum=0;

	unsigned char *ptr=NULL;
	char if_hw[20];
	struct ether_addr *eth0_hwaddr;
	if(nvram_get("et0macaddr")){
		strcpy(if_hw,nvram_get("et0macaddr"));
		eth0_hwaddr = ether_aton(if_hw);
		if(eth0_hwaddr == NULL){
			diag_printf("%s: failed!!!\n",__FUNCTION__);
			return;
		}	
		ptr = eth0_hwaddr->octet;
	}
	else{
		diag_printf("%s: macaddr is empty!!!\n",__FUNCTION__);
		return;
	}

	iPin = ptr[3] * 256 * 256 + ptr[4] * 256 + ptr[5];
	//printf("[%c][%c][%c]\n", ptr[3] , ptr[4] ,ptr[5] );
	iPin = iPin % 10000000;
	//printf("pin=%u\n", iPin);
	checksum = ComputeChecksum( iPin );
	iPin = iPin*10 + checksum;

	sprintf(devPwd, "%08u", iPin);
	printf("eth0 mac: %s\n", if_hw);
	printf("Generate new WPS PIN = %s, oldpin= %s\n", devPwd, nvram_get("wps_device_pin"));
	nvram_set("wps_device_pin", devPwd);
	
	return;
}

/*
 * pxy add, 2013.12.03
 */
#ifdef __CONFIG_WPS__

void wps_service_start()
{
	wps_led_test_on();
	nasd_stop();
	eapd_stop();
#ifdef __CONFIG_IGD__
	igd_stop();
#endif
	eapd_start();
	nasd_start();
#ifdef __CONFIG_IGD__
	igd_start();
#endif
	wps_start();
}
	
int start_wps_init()
{
	char *value = NULL;
	char buf[64] = {0}, prefix[32] = {0};

	if((value = nvram_get("wl_unit")) == NULL)
		return -1;
	
	if(atoi(value) != 0 && atoi(value) != 0.1)
		return -1;
	
	sprintf(prefix,"wl%s_",value);
	
	nvram_set(strcat_r(prefix, "wps_mode", buf), "enabled");
	nvram_set(strcat_r(prefix, "wps_method", buf), "pbc");
	nvram_set(strcat_r(prefix, "wps_pin", buf), "00000000");

	wps_service_start();
	return 0;
}

void start_wps_negotiate()
{
	char_t nvifname[IFNAMSIZ], osifname[IFNAMSIZ];		
	char_t uibuf[256] = "SET ";
	char_t *status;
	char_t *pin = "00000000", *method = "pbc";	
	int uilen = 4, sendnum = 0;
	char_t *wl_mode;
	wl_mode = nvram_safe_get("wl0_mode");
	if(strcmp(wl_mode, "sta") == 0 || strcmp(wl_mode, "wet") == 0
		|| nvram_match("wps_mode", "disabled") && nvram_match("wl0.1_wps_mode", "enabled"))
	{
		/* pxy add, 2014.7.30, for Tenda wps standard edited by wws */
		copy_wl_index_to_unindex("0.1");
	}
	if(start_wps_init() < 0){
		printf("[pxydebug]start_wps_init failed!");
		return;
	}
	
	strcpy(wps_unit, nvram_safe_get("wl_unit"));
	sprintf(nvifname, "wl%s", wps_unit);
	nvifname_to_osifname(nvifname, osifname, sizeof(osifname));
	
	uilen += sprintf(uibuf + uilen, "wps_method=%d ", WPS_UI_METHOD_PBC);		
	uilen += sprintf(uibuf + uilen, "wps_sta_pin=%s ", pin);
	uilen += sprintf(uibuf + uilen, "wps_action=%d ", WPS_UI_ACT_ADDENROLLEE);
	uilen += sprintf(uibuf + uilen, "wps_config_command=%d ", WPS_UI_CMD_START);
	uilen += sprintf(uibuf + uilen, "wps_pbc_method=%d ", WPS_UI_PBC_SW);
	uilen += sprintf(uibuf + uilen, "wps_ifname=%s ", osifname);


	start_wps_timerall();

	set_wps_env(uibuf);

	backup_wps_env(uibuf);

	start_wps_timerother();
		

	return ;
}
#endif


