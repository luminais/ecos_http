/*
 * eCos router root control for all protocol stacks and interfaces.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: rc.c,v 1.14 2010-11-01 04:09:51 Exp $
 *
 */
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <bcmnvram.h>
#include <rc.h>
#include "flash_cgi.h"
#include "../tc/tc.h"
#include "../../bsp/net/ipfilter/sdw_filter.h"
#include "sdw_conf.h"

#include <sys/syslog.h>

#if 1
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
//#include <bcmgpio.h> /* ccq add */
//#if defined(__ECOS)
/* directly access the gpio registers */
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <assert.h>
#ifdef BCMDBG
#define GPIO_ERROR(fmt, args...) printf("%s: " fmt "\n" , __FUNCTION__ , ## args)
#else
#define GPIO_ERROR(fmt, args...)
#endif
#include <bcmgpio.h>


#include <net/if.h>
#include <etioctl.h>

int watchdog = WATCHDOG_TIME;  //ms

//extern uint32 si_gpioout(si_t *sih, uint32 mask, uint32 val, uint8 priority);

/* Search for token in comma separated token-string */
static inline int
findmatch(char *string, char *name)
{
	uint len;
	char *c;

	len = strlen(name);
	while ((c = strchr(string, ',')) != NULL) {
		if (len == (uint)(c - string) && !strncmp(string, name, len))
			return 1;
		string = c + 1;
	}

	return (!strcmp(string, name));
}
#ifndef __CONFIG_WPS__
//#include <types.h>
#ifdef __CONFIG_TENDA_HTTPD_UCD__
static void *gpio_sih;
#endif
#endif
#endif


#ifdef __CONFIG_NAT__
#include <time.h>
#include <route_cfg.h>
extern void firewall_client_update(void);
//roy+++,2010/09/20
#ifdef __CONFIG_URLFILTER__
extern void firewall_urlf_update(void);
#endif

extern in_addr_t inet_addr(const char *cp);
extern void firewall_mac_update(void);
//+++
extern void pppoe_on_demand();
extern void pppoe_connect_rc();
extern void pppoe_disconnect_rc();
#endif /* __CONFIG_NAT__ */

#ifdef __CONFIG_SYS_LED__
extern void sys_led_test_proc(int on);
extern void sys_led_test_init();
#endif/*__CONFIG_SYS_LED__*/

#ifdef __CONFIG_WL_LED__
extern void wl_led_test_init(void);
extern void wl_led_test_on(void);
extern void wl_led_test_off(void);
#endif/*__CONFIG_WL_LED__*/

#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_init(void);
#endif /*__CONFIG_WPS_LED__*/

#ifdef __CONFIG_STREAM_STATISTIC__
extern void init_stream_ip(void);
extern void stream_tmr_func(void);
#endif

#ifdef __CONFIG_TC__
extern void enable_tc(void);
#endif

//add by ll
//extern int download_redirect_url_thread(void);
extern int kill_sdw_download_thread(void);
//end by ll
extern int ifr_set_link_speed2(int speed);
//luminais mark
extern void init_js_inject_para(void);
extern void js_inject_start(void);
extern void js_inject_stop(void);
extern void upgrade_online_start();
extern void login_keep_start();
extern int is_wan_up;
int auto_reboot_flag = 0;
//luminais mark end

#ifdef __CONFIG_APCLIENT_DHCPC__
extern int api_apclient_dhcpc_main_loop();
#endif
#define REBOOT				0x0001
#define RESTART_EXWAN		0x0002
#define STOP				0x0004
#define START				0x0008
#define TIMER				0x0010
#define RESTART_ALL			0x0011
#define RESET_WL_PWR_PERCENT		0x0012
#ifdef __CONFIG_ALINKGW__
#define ALI_ATTACK			0x0013
#define ALI_QOS_PRIORITY	0x0014
#define ALI_SEC_REPORT		0x0015
#endif
#define PPPOE_DISCONN		0x0020
#define PPPOE_CONN			0x0040
#define BEFORE_UPGEADE		0x0080

#ifdef __CONFIG_STREAM_STATISTIC__
#define STREAM_TIMER 0x0100
#endif

//add by ll
//#define AUTO_REBOOT	 0x0112
#define DOWNLOAD_REDIRECT_URL	0x0200

//end by ll
#define DHCPMODE		2
#define PPPOEMODE		3

typedef enum btn_status
{	
	UNDEFINE = 0,
	WIFI_SWITCH,
	PWR_SWITCH
}BTN_STATUS;


int g_cur_wl_radio_status = WL_RADIO_ON;

int g_btn2_status = UNDEFINE;

int mfg_reset_button_check_tag = 0;
int mfg_wifi_button_check_tag = 0;
int mfg_button_check_tag = 0;
int mfg_mode = 0;
login_ip_time loginUserInfo[MAX_USER_NUM];
extern int recv_dhcp_offer_packet_tag;
extern int recv_pado_packet_tag;
extern int gWifiStatusConfig;

static cyg_mbox rc_mbox;
static cyg_handle_t rc_mbox_handle;

//For Tenda
#ifdef __CONFIG_WPS__
extern /*void*/int wps_gpio_btn_check();
extern /*void*/int wps_gpio_btn_check_other();
extern int wps_gpio_btn_init();
#endif
extern void board_reboot(void);
extern int pppoe2dhcpc(int argc, char **argv);
#ifndef __CONFIG_TENDA_HTTPD_V3__
extern int wan_mode_check_main_loop();
extern int wan_surfing_check_main_loop();
#endif
extern int restart_check_main_loop();

extern int wl_restart_check_main_loop();
extern void wl_restart_thread_exit(void);


#ifdef __CONFIG_BATCH_UPGRADE__
extern int batch_upgrade_check(void);
#endif /*__CONFIG_BATCH_UPGRADE__*/

#ifdef __CONFIG_ALINKGW__
extern int tai_active_submit_start(void);
extern int access_attack_report_all();
extern int handle_al_qos_priority(char * mac_str) ;
extern char qos_sign_mac[20] ;
extern int al_qos_priority  ;
extern int start_qos_flag  ;
extern int report_ali_sec_info_direct(void) ;
extern int report_ali_sec_info(void) ;

int g_alinkgw_enable = 0;
int g_alinkgw_sec_enable = 0;
#endif
#ifdef __CONFIG_AL_SECURITY__

typedef enum {
    ALINKGW_ATTRIBUTE_simple = 0,  //值可由单个字符串/布尔值/数字表示的属性
    ALINKGW_ATTRIBUTE_complex,     //值必须由多个<name, value>对组合表示的属性
    ALINKGW_ATTRIBUTE_array,       //值必须由一个或多个集合数组表示的属性
    ALINKGW_ATTRIBUTE_MAX          //属性类型分类最大数，该值不表示具体类型
}ALINKGW_ATTRIBUTE_TYPE_E;

extern int al_security_handle_prepare(void) ;
extern void al_security_start(int flag) ;

#endif

#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
extern void firewall_parent_control_update(void);
extern void load_remark_config();
extern void load_static_dhcp_config_init();
#endif
/* HAL pb */
typedef enum wps_btnpress {
	WPS_NO_BTNPRESS = 0,
	WPS_SHORT_BTNPRESS,
	WPS_LONG_BTNPRESS
} wps_btnpress_t;

/* state */
static inline int
get_state(void)
{
	return ((int)cyg_mbox_get(rc_mbox_handle) & 0xffff);
}

static inline int
set_state(int state)
{
	if (rc_mbox_handle && cyg_mbox_tryput(rc_mbox_handle, (void *)state))
		return 0;
	else
		return -1;
}
#ifdef __CONFIG_ALINKGW__
void
ali_attack(void)
{
	set_state(ALI_ATTACK);
}

void ali_security_reprot(void)
{
	set_state(ALI_SEC_REPORT) ;
}


#endif

void
sys_reboot(void)
{
	set_state(REBOOT);
}

/*
 * sys_restart not include WAN restart, but
 * sys_restart2 can do it
 * pxy revise 2013.06.14
 */
void
sys_restart(void)
{
	set_state(RESTART_EXWAN);
}

void
reset_wl_pwr_percent(void)
{
	set_state(RESET_WL_PWR_PERCENT);
}


void
sys_restart2(void)
{
	set_state(RESTART_ALL);
}

void
sys_pppoe_disconn(void)
{
	set_state(PPPOE_DISCONN);
}

void
sys_pppoe_conn(void)
{
	set_state(PPPOE_CONN);
}

void sys_before_upgrade(void)
{
//升级前调用此函数,释放一些系统资源
	set_state(BEFORE_UPGEADE);
	cyg_thread_delay(200);
}


//add by ll 20140605
void start_download_redirect_url(void)
{
	set_state(DOWNLOAD_REDIRECT_URL);
}

//end by ll 

//--------weige add for check web login time out---------
static void web_auth_timer()
{
	unsigned tm;
	tm = (unsigned int)cyg_current_time();
	int i = 0;

	for(i=0; i<MAX_USER_NUM; i++)
	{
		//printf("--tm--%d--loginUserInfo--%d--\n", tm, loginUserInfo[i].time);
		if(0 != loginUserInfo[i].time && (tm - loginUserInfo[i].time) > (5*60*100))
		{
			printf("web [%s] login time expired.\n", loginUserInfo[i].ip);
			memset(loginUserInfo[i].ip, 0x0, IP_SIZE);
			loginUserInfo[i].time = 0;
		}	
	
	}

}
//------------end---------------

static void init_parase()
{
	nvram_set(_WAN0_CONNECT,"Disconnected");
//	nvram_set(_DDNS_STATUS,"Disconnected");
}

/* Timer procedure.  Check WAN link and process anything 
 * happened period
 */
extern int add_arptables();


//add by z10312 V3 风格需引用wifi button  20150716
#define WPS_GPIO_BUTTON_VALUE	"wps_button"
#define BUTTON2_GPIO 21
#define WPS_RESET_BUTTON_GPIO 20
mfg_button_hold_status s_mfg_button_hold_status;

//end by z10312 V3 风格需引用wifi button  20150716

	
#ifndef __CONFIG_WPS__
#ifdef __CONFIG_TENDA_HTTPD_UCD__


/* ccq add for sys led and reset button */
static unsigned long connect_mask;


/* GPIO registers */
#define BCMGPIO_REG_IN		0
#define BCMGPIO_REG_OUT		1
#define BCMGPIO_REG_OUTEN	2
#define BCMGPIO_REG_RESERVE 	3
#define BCMGPIO_REG_RELEASE 	4

#define BCMGPIO_MAX_FD		4
static int bcmgpio_fd;
/* GPIO information */
typedef struct {
	int connected;				/* is gpio being used? */
	bcmgpio_dirn_t dirn;			/* direction: IN or OUT */
	unsigned long on_time;			/* in 10 ms units */
	unsigned long strobe_period;		/* in 10 ms units */
	unsigned long timer_count;		/* 10 ms tick counter */
	unsigned long strobe_count;		/* strobe counter */
	unsigned long tot_strobes;		/* total number of strobes */
	unsigned long orig_state;		/* original state of GPIO before blinking */
	int strobing;				/* is gpio strobing? */
	int *strobe_done;			/* pointer to memory which is used to signal strobe completion */
	bcm_timer_id timer_id;			/* id of the strobe timer */
} bcmgpio_info_t;

static bcmgpio_info_t bcmgpio_info[BCMGPIO_MAXPINS];
//extern uint32 si_gpioin(si_t *sih);
//extern uint32 si_gpioin(si_t *sih);


/**********************************************************************************************
 *  Functions visible to this file only
******************************************************************************************** */
static int 
bcmgpio_drvinit ()
{
#if defined(__ECOS)
	if (!(gpio_sih = (void *)si_kattach(SI_OSH)))
		return -1;
	bcmgpio_fd = 1;
	si_gpiosetcore(gpio_sih);
	return 0;
#else
	bcmgpio_fd = open("/dev/gpio", O_RDWR);
	if (bcmgpio_fd == -1) {
		GPIO_ERROR ("Failed to open /dev/gpio\n");
		return -1;
	}
	return 0;
#endif
}


static int bcmgpio_ioctl(int gpioreg, unsigned int mask , unsigned int val)
{
#if defined(__ECOS)
	int value;
	switch (gpioreg) {
		case BCMGPIO_REG_IN:
			value = si_gpioin(gpio_sih);
			break;
		case BCMGPIO_REG_OUT:
			value = si_gpioout(gpio_sih, mask, val,GPIO_APP_PRIORITY);
			break;
		case BCMGPIO_REG_OUTEN:
			value = si_gpioouten(gpio_sih, mask, val,GPIO_APP_PRIORITY);
			break;
		case BCMGPIO_REG_RESERVE:
			value = si_gpioreserve(gpio_sih, mask, GPIO_APP_PRIORITY);
			break;
		case BCMGPIO_REG_RELEASE:
			/* 
 			* releasing the gpio doesn't change the current 
			* value on the GPIO last write value 
 			* persists till some one overwrites it
			*/
			value = si_gpiorelease(gpio_sih, mask, GPIO_APP_PRIORITY);
			break;
		default:
			GPIO_ERROR ("invalid gpioreg %d\n", gpioreg);
			value = -1;
			break;
	}
	return value;
#else
	struct gpio_ioctl gpio;
	int type;

	gpio.val = val;
	gpio.mask = mask;

	switch (gpioreg) {
		case BCMGPIO_REG_IN:
			type = GPIO_IOC_IN; 
			break;
		case BCMGPIO_REG_OUT:
			type = GPIO_IOC_OUT; 
			break;
		case BCMGPIO_REG_OUTEN:
			type = GPIO_IOC_OUTEN; 
			break;
		case BCMGPIO_REG_RESERVE:
			type = GPIO_IOC_RESERVE; 
			break;
		case BCMGPIO_REG_RELEASE:
			type = GPIO_IOC_RELEASE; 
			break;
		default:
			GPIO_ERROR ("invalid gpioreg %d\n", gpioreg);
			return -1;
	}
	if (ioctl(bcmgpio_fd, type, &gpio) < 0) {
		GPIO_ERROR ("invalid gpioreg %d\n", gpioreg);
		return -1;
	}
	return (gpio.val);
#endif
}


/**********************************************************************************************
 *  GPIO functions 
******************************************************************************************** */
int 
bcmgpio_connect (int gpio_pin, bcmgpio_dirn_t gpio_dirn)
{
	unsigned long bitmask;

	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	
	assert ((gpio_dirn == BCMGPIO_DIRN_IN) || (gpio_dirn == BCMGPIO_DIRN_OUT));

	if (connect_mask == 0) {
		if (bcmgpio_drvinit () != 0) 
			return -1;
	}
	if (bcmgpio_info[gpio_pin].connected)
		return -1;

	bitmask = ((unsigned long) 1 << gpio_pin);

	bcmgpio_info[gpio_pin].connected = 1;
	bcmgpio_info[gpio_pin].dirn = gpio_dirn;

	/* reserve the pin*/
	bcmgpio_ioctl(BCMGPIO_REG_RESERVE, bitmask, bitmask);

	if (gpio_dirn == BCMGPIO_DIRN_IN)
		bcmgpio_ioctl(BCMGPIO_REG_OUTEN, bitmask, 0);
	else
		bcmgpio_ioctl(BCMGPIO_REG_OUTEN, bitmask, bitmask);

	connect_mask |= bitmask;

	return 0;
}

int
bcmgpio_strobe_stop (int gpio_pin)
{
	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	

	if (! bcmgpio_info[gpio_pin].connected)
		return -1;

	if (bcmgpio_info[gpio_pin].strobing) {
		bcmgpio_info[gpio_pin].strobing = 0;

		if (bcmgpio_info[gpio_pin].timer_id != 0) {
			bcm_timer_delete (bcmgpio_info[gpio_pin].timer_id);
			bcmgpio_info[gpio_pin].timer_id = 0;
		}
	}

	return 0;
}

static void 
bcmgpio_drvcleanup ()
{
#if defined(__ECOS)
#else	
	if (bcmgpio_fd!= -1) {
		close (bcmgpio_fd);
		bcmgpio_fd = -1;
	}
#endif
}

/* 
 * releasing the gpio doesn't change the current value on the GPIO last write value 
 * persists till some one overwrites it
*/
int 
bcmgpio_disconnect (int gpio_pin)
{
	unsigned long bitmask;

	assert ((gpio_pin >= 0) && (gpio_pin <= BCMGPIO_MAXINDEX));	

	if (! bcmgpio_info[gpio_pin].connected)
		return -1;

	bitmask = ((unsigned long) 1 << gpio_pin);

	if (bcmgpio_info[gpio_pin].strobing)
		bcmgpio_strobe_stop (gpio_pin);

	bcmgpio_info[gpio_pin].connected = 0;

	/* release the pin*/
	bcmgpio_ioctl(BCMGPIO_REG_RELEASE, bitmask, 0);

	connect_mask &= ~bitmask;

	if (connect_mask == 0)
		bcmgpio_drvcleanup ();

	return 0;
}





int  bcmgpio_out (unsigned long gpio_mask, unsigned long value)
{

	if ((gpio_mask & connect_mask) != gpio_mask)
		return -1;

	bcmgpio_ioctl (BCMGPIO_REG_OUT, gpio_mask, value);

	return 0;
}

int
bcmgpio_getpin(char *pin_name, uint def_pin)
{
	char name[] = "gpioXXXX";
	char *val;
	uint pin;

	/* Go thru all possibilities till a match in pin name */
	for (pin = 0; pin < BCMGPIO_MAXPINS; pin ++) {
		sprintf(name, "gpio%d", pin);
		val = nvram_get(name);
		if (val && findmatch(val, pin_name))
			return pin;
	}

	if (def_pin != BCMGPIO_UNDEFINED) {
		/* make sure the default pin is not used by someone else */
		sprintf(name, "gpio%d", def_pin);
		if (nvram_get(name))
		{
			def_pin =  BCMGPIO_UNDEFINED;
		}
	}

	return def_pin;
}


void sys_led_test_init()
{
       int sys_led_test_gpio = -1;
       /* Initial */
       sys_led_test_gpio = bcmgpio_getpin("sys_led", -1);
	diag_printf("sys_led_test_gpio=%d\n", sys_led_test_gpio);
       if (sys_led_test_gpio == -1) {
              printf("No default sys led configuration\n");
              return;
       } else {
              printf("Using pin %d for sys_led output\n", sys_led_test_gpio);
              if (bcmgpio_connect(sys_led_test_gpio, 1 /* BCMGPIO_DIRN_OUT */) != 0) {
                     printf("Error connecting GPIO %d to sys_led\n", sys_led_test_gpio);
                     return;
              }
       }
}
int 
bcmgpio_in_other (unsigned long gpio_mask, unsigned long *value)
{
	unsigned long regin;

	regin = bcmgpio_ioctl (BCMGPIO_REG_IN, 0, 0);
	
	*value = regin & gpio_mask;

	return 0;
}






void sys_led_test_proc(int on)
{
	char *value;
	int sys_led_test_gpio,sys_led_test_assertlvl;
	unsigned long led_mask,led_value;

	value = nvram_safe_get("sys_led_assertlvl");
	sys_led_test_assertlvl = atoi(value) ? 1 : 0;

	sys_led_test_gpio = bcmgpio_getpin("sys_led", -1);
	if(sys_led_test_gpio == -1){
		diag_printf("sys_led_test_gpio error!\n");
		return;
	}
	
	if(on){
              led_mask = ((unsigned long)1 << sys_led_test_gpio);
              led_value = ((unsigned long)sys_led_test_assertlvl << sys_led_test_gpio);
              bcmgpio_out(led_mask, led_value);
	}else{
              led_mask = ((unsigned long)1 << sys_led_test_gpio);
              led_value = ((unsigned long) ~sys_led_test_assertlvl << sys_led_test_gpio);
              led_value &= led_mask;
              bcmgpio_out(led_mask, led_value);
	}
}


int 
bcmgpio_in (unsigned long gpio_mask, unsigned long *value)
{
	unsigned long regin;
	if ((gpio_mask & connect_mask) != gpio_mask)
		return -1;

	regin = bcmgpio_ioctl (BCMGPIO_REG_IN, 0, 0);
	
	*value = regin & gpio_mask;

	return 0;
}
#endif
/* end ccq  */
#endif



//add by z10312 V3 风格需引用wifi button  20150716

extern void  reset_button(void);

void reset_btn_init()
{
       int reset_btn_gpio = -1;
       /* Initial */
       reset_btn_gpio = bcmgpio_getpin(WPS_GPIO_BUTTON_VALUE, BCMGPIO_UNDEFINED);
	diag_printf("reset_btn_gpio=%d\n", reset_btn_gpio);
       if (reset_btn_gpio == -1) {
              printf("No default reset_btn_gpio configuration\n");
              return;
       } else {
              printf("Using pin %d for reset_btn_gpio input\n", reset_btn_gpio);
              if (bcmgpio_connect(reset_btn_gpio, 0 /* BCMGPIO_DIRN_IN */) != 0) {
                     printf("Error connecting GPIO %d to reset_btn_gpio\n", reset_btn_gpio);
                     return;
              }
       }
}

void btn2_init()
{	
	if(bcmgpio_connect(BUTTON2_GPIO, BCMGPIO_DIRN_IN) != 0)
	{		
		printf("Error connnecting GPIO %d!\n",BUTTON2_GPIO);		
	}	
	
	return;

}

/* Return 0 OK, -1 Failed */
void button_init ()
{
	g_btn2_status = UNDEFINE;
	
	if (nvram_match("btn2_status","WIFI_SWITCH"))
	{
		g_btn2_status = WIFI_SWITCH;
	}
		
	reset_btn_init();

	if (WIFI_SWITCH == g_btn2_status || PWR_SWITCH == g_btn2_status)
	{
		btn2_init();
	}

	return;
}

void button_wl_check()
{	
	static int count = 0;
	unsigned long button_status,btn_mask;
	
	btn_mask = ((unsigned long)1 << BUTTON2_GPIO);
	bcmgpio_in_other (btn_mask, &button_status);
	
	button_status >>= BUTTON2_GPIO;
	
	if(button_status == 0)
	{
		count++;
		//mfg_button_check_tag = 1;
	}
	else if(button_status==1)
	{
		if (count>0 && count<4)
		{
			mfg_wifi_button_check_tag = 1;
			if(!mfg_mode)
			{
				if(!s_mfg_button_hold_status.reset_button_hold && !s_mfg_button_hold_status.wifi_button_hold){
					if ( WL_RADIO_ON == g_cur_wl_radio_status)
					{
						g_cur_wl_radio_status = WL_RADIO_OFF;
					}
					else
					{
						g_cur_wl_radio_status = WL_RADIO_ON;
					}
					gWifiStatusConfig = 1;
					
			        //wl_restart_thread_exit();
						
					sys_restart();
				}
			}
		}
	/*
		if (count > 0)
		{
			printf("mfg_wifi_button_check_tag =1 \n");
			mfg_wifi_button_check_tag = 1;
		}
		*/
		count = 0;
	}
	return;
}


int button_check()
{
	static int count = 0;
	unsigned long button_status,btn_mask;
	btn_mask = ((unsigned long)1 << WPS_RESET_BUTTON_GPIO);
	bcmgpio_in_other (btn_mask, &button_status);
	button_status >>= WPS_RESET_BUTTON_GPIO;
	if(button_status==0)
	{
		count++;
		mfg_button_check_tag = 1;
		printf("%s count=%d\n",__func__,count);
		if(!mfg_mode)
		{
			if(count == 7 && nvram_match("restore_defaults","0"))
			{
				mfg_reset_button_check_tag = 1;
				untimeout((timeout_fun *)&button_check, 0);
				reset_button();
				return 0;
			}
		}
	}
	else if(button_status==1)
	{
		if (UNDEFINE != g_btn2_status)//no config
		{
			if (count>0 && count<4)
			{
				mfg_wifi_button_check_tag = 1;
				if(!mfg_mode)
				{
					if(!s_mfg_button_hold_status.reset_button_hold && !s_mfg_button_hold_status.wifi_button_hold){
						if ( WL_RADIO_ON == g_cur_wl_radio_status)
						{
							g_cur_wl_radio_status = WL_RADIO_OFF;
						}
						else
						{
							g_cur_wl_radio_status = WL_RADIO_ON;
						}
							
						//nvram_set(WLN0_CTL_ENABLE, "0");
						
						//nvram_commit();
						gWifiStatusConfig = 1;

					    //wl_restart_thread_exit();

						sys_restart();
					}
				}
			}
		}
		/*
		if (count > 0)
		{
			printf("mfg_reset_button_check_tag =1 \n");
			mfg_reset_button_check_tag = 1;
		}
		*/
		count = 0;
	}

	if (WIFI_SWITCH == g_btn2_status)
	{
		button_wl_check();
	}
	
	timeout((timeout_fun *)button_check, NULL, 100);
	return 0;
}

//end by z10312 V3 风格需引用wifi button  20150716








/*tenda add reset button test for tenda_v3*/
#ifdef __CONFIG_TENDA_HTTPD_V3__
#define WAIT_TIME 5*10 
#define EFFECT_LEVEL 0 
int ate_check_button(int button_gpio) 
{ 
	unsigned long btn_mask; 
	unsigned long value; 
	unsigned int utimes = 0; 
	int flag = 0; 

	printf("%s, %d\n", __FUNCTION__, __LINE__); 
	#if 1 
	if (bcmgpio_connect(button_gpio, 0) != 0) 
	{ 
		/* Maybe connected , just go on ! */ 
		printf("Error connecting GPIO %d to reset button\n", button_gpio); 
		//return ATE_ERROR; 
	} 
	#endif 
	for(utimes = 0; utimes < WAIT_TIME; utimes++) 
	{ 
		btn_mask = ((unsigned long)1 << button_gpio); 
		bcmgpio_in(btn_mask, &value); 
		value >>= button_gpio; 

		if(value == EFFECT_LEVEL) 
		{ 
			flag = 1; 
			printf("\n--- Button pressed!!!\n\n"); 
			break; 
		} 
		cyg_thread_delay(10); // 0.1 s 
	} 
	#if 0 
	bcmgpio_disconnect(button_gpio); 
	#endif 

	return flag; 
}

#endif

static int
do_timer(void)
{
	static time_t last = 0, now = 0;
	static time_t http_last = 0, http_now = 0;
//	static int sys_led_on = 1;
	struct tm *time_local;
	int wan_link;
	char wps_mode[16] = {0};
	char wl_mode[16] = {0};
	
	strcpy(wps_mode, nvram_safe_get("wl0_wps_mode"));
	strcpy(wl_mode, nvram_safe_get("wl0_radio"));
#if 0
	if(0 == strcmp(wps_mode, "enabled") && 0 == strcmp(wl_mode, "1"))
		wps_gpio_btn_check();
	else if(0 == strcmp(wps_mode, "disabled") || 0 == strcmp(wl_mode, "0"))
		wps_gpio_btn_check_other();
#endif
#ifdef __CONFIG_WPS__
	wps_gpio_btn_check();
#endif

#ifdef __CONFIG_NAT__

	wan_link = wan_link_check();
	
	static int wan_link_tag = 0;
	if(wan_link_tag != wan_link)
	{
		if(wan_link == 1)
		{
			printf("wan link up!\n");
			syslog(LOG_INFO,"wan up!");
		}
		else
		{
			printf("wan link down!\n");
			syslog(LOG_INFO,"wan down!");
		}
		wan_link_tag = wan_link;
	}
	now = time(0);
	http_now = time(0);
	time_local = localtime(&now);

	pppoe_on_demand(wan_link);//2011/05/26

	if (((now - last) > 5) ||
	    ((last - now) > 5) ||
	    ((time_local->tm_min == 0) && (time_local->tm_sec == 0))) {
#ifdef __CONFIG_TENDA_HTTPD_V3__		
		firewall_client_update();
#ifdef __CONFIG_URLFILTER__
		firewall_urlf_update();
#endif
		firewall_mac_update();
#endif
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	firewall_parent_control_update();
#endif
		add_arptables();
		last = now;
	}

	if (((http_now - http_last) > 50) || ((http_last - http_now) > 50)) {
		web_auth_timer();
		http_last = http_now;
	}

//	sys_led_on = (sys_led_on?0:1);

	/* Call set_state(TIMER) every 1 second */
	/* CJ++ */
#endif
	timeout((timeout_fun *)set_state, (void *)TIMER, 100);
	return 0;
}

#ifdef __CONFIG_STREAM_STATISTIC__
int do_stream_timer()
{
	stream_tmr_func();
	timeout((timeout_fun *)set_state, (void *)STREAM_TIMER, 100);//*10);//100*12);
	return 0;
}
#endif

#ifdef __CONFIG_WL_LED__
int wl_led_init_g = 1;  //route system init wl led status
void wl_led_test(void){
	char wl_enable[4];
	strcpy(wl_enable, nvram_safe_get("wl0_radio"));
	wl_led_init_g=1;
	if(strcmp(wl_enable, "1") == 0){
		diag_printf("wireless led on\n");
		wl_led_test_on();
	}else{
		diag_printf("wireless led off\n");
		wl_led_test_off();
	}
	wl_led_init_g = 0;
	return;
}
#endif /*__CONFIG_WL_LED__*/

#ifdef __CONFIG_WPS_LED__
extern void wps_led_test_off();
extern void wps_led_test_on();
int wps_led_init_g  = 1;	//route system init wps led status

void wps_led_test(void){
	char wl_enable[4];
	char wps_enable[16];
	
	strcpy(wl_enable,  nvram_safe_get("wl0_radio"));
	strcpy(wps_enable, nvram_safe_get("wl0_wps_mode"));
	wps_led_init_g=1;
	if(strcmp(wl_enable, "1") == 0 && strcmp(wps_enable, "enabled") == 0){
		wps_led_test_on();
	}else{
		wps_led_test_off();		
	}
	wps_led_init_g = 0;
	return;
}

#endif/*__CONFIG_WPS_LED__*/

/************************************************************
Function:	 sys_led               
Description:   定时器控制系统灯状态，1s钟变化一次

Input:                                          

Output: 	    

Return:      

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/
#ifdef __CONFIG_SYS_LED__
extern si_t *bcm947xx_sih;

int sys_led()
{
	static int sys_led_on=1;
	sys_led_test_proc(sys_led_on);//roy add
	sys_led_on = (sys_led_on?0:1);
	if(watchdog > 0)
	{	
		si_watchdog_ms(bcm947xx_sih, watchdog);
	}	
	timeout((timeout_fun *)sys_led, NULL, 100);
	return 0;
}
#endif


/************************************************************
Function:	 lan_link_check               
Description:  检测LAN口插拔网线的状态，并添加到日志里面去

Input:                                          

Output: 	    

Return:      

Others:
History:
<author>   <time>    <version >   <desc>
hqw        2013-11-05   1.0        新建函数

************************************************************/

extern int iflib_ioctl(char *ifname, int cmd, void *data);
int lan_link_check()
{
	struct phystatus ps;
	int i=0;
	static int lan[3]={0};
	char info[15]={0};
	for(i=1;i<=3;i++)
	{
		ps.ps_port = i;

		/* assume port in unlink state if ioctl fails */
		if (iflib_ioctl("vlan1", SIOCGIFLINK, &ps) < 0)
			ps.ps_link = 0;
		if(ps.ps_link != lan[i-1])
		{
			if(ps.ps_link == 0)
			{
				sprintf(info,"lan%d down",i);
				syslog(LOG_INFO,info);
			}
			else
			{
				sprintf(info,"lan%d up",i);
				syslog(LOG_INFO,info);
			}
			lan[i-1] = ps.ps_link;
		}
	}	
	timeout((timeout_fun *)lan_link_check, NULL, 100);
	return 0;
}

//start: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27
void set_all_led_on( void )
{	
	int s, gpio_pin;
	int led_mask=0,led_value = 0,value =0 ;
	int vecarg[2];
	struct ifreq ifr;
        
	//cyg_thread_delay(3 * 100);
            
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
}
void set_all_led_off( void )
{	

	int s, gpio_pin;
	int led_mask=0,led_value = 0,value =0 ;
	int vecarg[2];
	struct ifreq ifr;

	//cyg_thread_delay(3 * 100);

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
}
//start: add by z10312  合入 厂测按键测试&LED灯测试 接口2015-01-27


#ifdef __CONFIG_BATCH_UPGRADE__
void set_all_led_blink()
{	
	static int led_on = 0;
	int s, gpio_pin;
	int led_mask=0,led_value = 0,value =0 ;
	int vecarg[2];
	struct ifreq ifr;

	/* Light all the other led except lan&wan, e.g. wl/sys/wps */
	for(gpio_pin = 5; gpio_pin <= 6; gpio_pin++)
	{
		/*enable the wl gpio value input*/
		bcmgpio_connect(gpio_pin, BCMGPIO_DIRN_OUT);
		if(led_on)
		{
			led_mask = ((unsigned long)1 << gpio_pin);
			led_value = (~value << gpio_pin);
			led_value &= led_mask;
		}
		else
		{
			led_mask = ((unsigned long)1 << gpio_pin);
			led_value = (value << gpio_pin);
		}
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
	if(led_on)
	{
		vecarg[1] = 0x155;
	}
	else
	{
		vecarg[1] = 0x0;
	}
	ifr.ifr_data = (caddr_t) vecarg;
	strncpy(ifr.ifr_name, "eth0", sizeof (ifr.ifr_name));
	if (ioctl(s, SIOCSETCROBOWR, (caddr_t)&ifr) < 0)
	{
		printf("Error: etcrobowr\n");
		return ;
	}

	close(s);
	led_on = (led_on?0:1);
	timeout((timeout_fun *)set_all_led_blink, NULL, 30);
}



void want_sleep(int second)
{	
	if (0 < second)
	{
		cyg_thread_delay(second * 100);
	}

	return;
}

#endif /*__CONFIG_BATCH_UPGRADE__*/



#ifdef __CONFIG_AUTO_CONN__
extern void init_default_vif_ap_config();
extern void auto_conn_route_start();
#endif
//extern void set_pin_code(void);
/* Main loop */




//add by ll , 解决F3和F450功率不兼容问题
#define F3_MAX_POWER	"0x4e"
#define F6_V2_MAX_POWER	"0x52"

char* get_product_pwr_info()
{
	if(strcmp(F3_MAX_POWER , nvram_safe_get("sb/1/maxp2ga0")) == 0)
	{
		return "0";//是F3
	}
	else if(strcmp(F6_V2_MAX_POWER , nvram_safe_get("sb/1/maxp2ga0")) == 0)
	{
		return "0";//是F6V2.0
	}
	else
	{
		return "1" ;
	}
}
//end by ll

extern statistic_ip_index_t stream_ip[STREAM_CLIENT_NUMBER];
extern unsigned int stream_ip_per[STREAM_CLIENT_NUMBER][2];  //0:up, 1: down;
int if_device_idle(void)
{
	unsigned int index;
	int u_kbs0=0, d_kbs0=0;

	for(index=0; index< STREAM_CLIENT_NUMBER; ++index){
		if(stream_ip[index].index == NULL)
			continue;	

		(ip_ubs(index)!=0) ? (u_kbs0 = ip_ubs(index)/1000) : (u_kbs0=0);
		(ip_dbs(index)!=0) ? (d_kbs0 = ip_dbs(index)/1000) : (d_kbs0=0);

		if ((u_kbs0) > 1 || (d_kbs0) > 1)
		{
			return -1;
		}
	}

	return 0;
}
#if 0
#define ONE_DAY_SECONDS 86400
#define REBOOT_START_HOUR 1
#define REBOOT_END_HOUR 6

#ifdef __CONFIG_TENDA_WLJB__
//just for test (luminais mark), make sure same as mtenda.c
#define W311R_ECOS_SV "V5.07.64s"
#else
#define W311R_ECOS_SV "V5.07.64"
#endif

#define UPGRADE_VE_FORMATE "US_N300BR_%s_CN_SDW01"
void auto_reboot_set_state(void)
{
	auto_reboot_flag = 1;
	set_state(AUTO_REBOOT);
}

int need_upgrade_online()
{
	char running_firm_version[64] = {0};
	if(0 == check_conf_switch(UPGRADE_SW))
		goto no_upgrade;
	memset(running_firm_version , 0 , sizeof(running_firm_version));
	sprintf(running_firm_version, UPGRADE_VE_FORMATE, W311R_ECOS_SV);
	diag_printf("[%s]running_firm_version = %s\n", __FUNCTION__, running_firm_version);
	diag_printf("[%s]serv_version = %s\n", __FUNCTION__, sdw_conf_content[UPGRADE_VE].content);
	if(strcmp(sdw_conf_content[UPGRADE_VE].content, running_firm_version) > 0)
	{
		diag_printf("[%s]new version firmware is online\n", __FUNCTION__);
		return 1;
	}
no_upgrade:
	diag_printf("[%s]forget it\n", __FUNCTION__);
	return 0;
}

void auto_reboot(void)
{
	static int i = 0;
	static int day_count = 0;
	time_t tmp_time, time_diff;
	struct tm *local;
	
	if(1 == auto_reboot_flag)
	{
		if(0 == is_wan_up)//未联网
		{
			sys_reboot();
			timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*60);
			return;
		}
		time(&tmp_time);
		local = localtime(&tmp_time);
		if(local->tm_hour < REBOOT_START_HOUR)//在凌晨1点到6点之间进行重启，不包括6点
		{
			local->tm_hour = REBOOT_START_HOUR;
			local->tm_min = 0;
			local->tm_sec = 0;
			time_diff = mktime(local) - tmp_time + 2;
			timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*time_diff);
			return;
		}
		else if(local->tm_hour >= REBOOT_END_HOUR)
		{
			local->tm_hour = 23;
			local->tm_min = 59;
			local->tm_sec = 59;
			time_diff = mktime(local) - tmp_time + 3600*REBOOT_START_HOUR + 2;
			timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*time_diff);
			return;
		}
		else
		{
			diag_printf("[%s]if_device_idle : i = %d\n", __FUNCTION__, i);
			if (i < 60)
			{
				if (0 == if_device_idle())
				{
					i++;
				}
				else
				{
					i = 0;
				}
				timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*60);
				return;
			}
			else//连续一小时流量很少，视为设备空闲
			{
				if(need_upgrade_online())
				{
					diag_printf("[%s]upgrade first\n", __FUNCTION__);
					upgrade_online_start();
					return;
				}
				else
				{
					diag_printf("[%s]auto reboot without upgrade\n", __FUNCTION__);
					sys_reboot();
					timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*60);
					return;
				}
			}
		}
	}
	else
	{
		diag_printf("[%s] day %d\n", __FUNCTION__, day_count);
		if(day_count >= 7)
		{
			diag_printf("[%s]auto_reboot_set_state()\n", __FUNCTION__);
			auto_reboot_set_state();
			return;
		}
		else
		{
			timeout((timeout_fun *)set_state, (void *)AUTO_REBOOT, 100*ONE_DAY_SECONDS);
			day_count++;
			return;
		}
	}
}
#endif
void
main_loop(void)
{
	int state;
	
#if defined(__CONFIG_TENDA_HTTPD_V3__) || defined(__CONFIG_TENDA_HTTPD_NORMAL__)

	if(nvram_match(WLN0_WIRELESS_ENABLE, "1")){
		g_cur_wl_radio_status = WL_RADIO_ON;
	}else{
		g_cur_wl_radio_status = WL_RADIO_OFF;
	}
#else
	g_cur_wl_radio_status = WL_RADIO_ON;
#endif
#ifdef __CONFIG_ALINKGW__
	if(nvram_match(ALILINKGW_ENABLE, "1"))
	{
		g_alinkgw_enable = 1;
		if(nvram_match(ALILINKGW_SEC_ENABLE, "1"))
			g_alinkgw_sec_enable = 1;
		else
			g_alinkgw_sec_enable = 0;
	}
	else
	{
		g_alinkgw_enable = 0;
		g_alinkgw_sec_enable = 0;
	}
#endif
	
	/* Sys init here */
	/* Do power saving */
	if (nvram_match("wait", "1"))
		cpu_idle_sleep(1);
#ifdef __CONFIG_STREAM_STATISTIC__
	init_stream_ip();
#endif

#ifdef __CONFIG_WL_LED__
	wl_led_test_init();
#endif

#ifdef __CONFIG_SYS_LED__
	sys_led_test_init();	//roy add stanley modfiy
#endif/*__CONFIG_SYS_LED__*/	

	
	
#ifdef __CONFIG_TENDA_HTTPD_UCD__
/* ccq add  for sys_led  and reset button*/
	sys_led_test_init();
	timeout((timeout_fun *)sys_led, NULL, 100);
	//reset_btn_init();
/* end ccq */
#endif

//add by z10312 FH456V2.0 V2 TDE 需开启 wifi 开关按钮 20150716
#ifdef __CONFIG_TENDA_HTTPD_V3__
	if( strcmp(get_product_pwr_info(), "0"))
#endif
	{
		button_init();
		timeout((timeout_fun *)button_check, NULL, 10);
	}
	
	
	
#ifdef __CONFIG_WPS_LED__
	wps_led_test_init();
#endif/*__CONFIG_WPS_LED__*/ 

	syslog_init();
#ifdef __CONFIG_WPS__
	wps_gpio_btn_init();
#endif
	syslog(LOG_INFO|LOG_DEBUG,"System start");

#ifdef __CONFIG_AUTO_CONN__
	init_default_vif_ap_config();
#endif

	lo_set_ip();
#ifdef	CONFIG_VLAN
	vlan_config();
#endif

	start_lan();


	lan_pppoe_start(0);//gong add 开始Lan口PPPOE服务器
#ifdef __CONFIG_TENDA_HTTPD_NORMAL__
	load_remark_config();
	load_static_dhcp_config_init();

#endif
#ifdef	__CONFIG_NAT__
	start_firewall();
#endif
	init_services();
	start_services();

#ifdef	CONFIG_WL
	start_wl();
	set_wl_pwr_percent();
#endif
#ifdef __CONFIG_WL_LED__
	wl_led_test();	//wireless led on or off
#endif 
#ifdef __CONFIG_WPS_LED__
	wps_led_test();	//wps led on or off
#endif
#ifdef __CONFIG_TC__
	enable_tc();
#endif
	cli_start();

/* rc 添加watchdog功能，线程检测删除*/
//	thread_check_start();

	/*Init some parase status*/
	init_parase();
		
	/* Init event box */
	cyg_mbox_create(&rc_mbox_handle, &rc_mbox);

	/* Start timer */
	set_state(TIMER);

	/*init auto-conn thread*/
#ifdef __CONFIG_AUTO_CONN__
	auto_conn_route_start();	
#endif
        
#ifdef __CONFIG_BATCH_UPGRADE__
	batch_upgrade_check();
#endif /*__CONFIG_BATCH_UPGRADE__*/

	if(nvram_get("wan_speed"))
		ifr_set_link_speed2(atoi(nvram_get("wan_speed")));

#ifdef __CONFIG_STREAM_STATISTIC__	
	/* Start stream stat */
	set_state(STREAM_TIMER);
#endif
#ifdef __CONFIG_SYS_LED__
	timeout((timeout_fun *)sys_led, NULL, 100);
#endif
	timeout((timeout_fun *)lan_link_check, NULL, 100);
#ifndef __CONFIG_TENDA_HTTPD_V3__
	wan_mode_check_main_loop();
	wan_surfing_check_main_loop();
#endif
#ifdef __CONFIG_RESTART_CHECK__
	restart_check_main_loop();
#endif

#ifdef __CONFIG_APCLIENT_DHCPC__
	api_apclient_dhcpc_main_loop();
#endif
	
	//add by z10312 FH456V2.0 V2 TDE 移植ucd 无线功率&定时开关  20150707
	#ifdef __CONFIG_TENDA_HTTPD_V3__
	if( strcmp(get_product_pwr_info(), "0"))
	#endif
	{
	    wl_restart_check_main_loop();
	}
	//#endif
	
	
#ifdef __CONFIG_ALINKGW__
	if( g_alinkgw_enable )
		tai_active_submit_start();
#endif
#ifdef __CONFIG_AL_SECURITY__//add  by ll
	if( g_alinkgw_enable && g_alinkgw_sec_enable )
	{
		al_security_handle_prepare();
		al_security_start(1) ;
	}
#endif
//auto_reboot ly
	//set_state(AUTO_REBOOT);
#if 1
//luminais mark
	if(0 == nvram_match("tenda_ate_test", "1"))
	{
		login_keep_start();
	}
//luminais mark
#endif

	/* Loop forever */
	for (;;) {

		state = get_state();
		
		/* system command */
		switch (state) {
		/*
		 * system restart not include WAN
		 * 
		 */
		case RESTART_EXWAN:
			/* Fall through */
		case STOP:
			/* Stop */
			/*
			 * After LAN/WAN ip down, it will do
			 * stop_firewall() at wan_inet_stop
			 * or lan_inet_stop
			 */
			stop_services();
#ifdef __CONFIG_NAT__
		//	stop_wan();
		//	stop_firewall();del by ll ,解决dhcp概率性获取不到ip问题
#endif
			stop_lan();
			/* Fall through */
		case START:
			/* Update */
			start_lan();
					
			lan_pppoe_start(0);//gong add 开始Lan口PPPOE服务器

			
#ifdef __CONFIG_NAT__
		//	start_firewall();del by ll ,解决dhcp概率性获取不到ip问题
		//	if (wan_link_status())
		//		start_wan();
#endif /* __CONFIG_NAT__ */	
			/*
			 * After LAN/WAN IP UP, it will do
			 * update_firewall() at wan_inet_start
			 * or lan_inet_start
			 */
			update_services();

			start_wl();
		case RESET_WL_PWR_PERCENT:
			set_wl_pwr_percent();
			
			break;
//auto_reboot ly
#if 0
		case AUTO_REBOOT:
			auto_reboot();
			break;
#endif
		case TIMER:
			do_timer();
			break;
#ifdef __CONFIG_STREAM_STATISTIC__
		case STREAM_TIMER:
			do_stream_timer();
			break;
#endif

//roy+++,2010/10/27
		case PPPOE_DISCONN:
#ifdef __CONFIG_NAT__
			stop_wan();
#endif/* __CONFIG_NAT__ */	
			break;
		case PPPOE_CONN:
#ifdef __CONFIG_NAT__
			//start_firewall();
			if (wan_link_status())
				start_wan();
#endif/* __CONFIG_NAT__ */	
			/*
			 * After LAN/WAN IP UP, it will do
			 * update_firewall() at wan_inet_start
			 * or lan_inet_start
			 */
			update_services();
			break;
		case BEFORE_UPGEADE:
#ifdef __CONFIG_NAT__
			stop_wan();
			stop_firewall();
#endif
			break;
//+++
		case REBOOT:
#if 0//stop_wan() may do forever, so that 	board_reboot() won't be carried out(luminais)
#ifdef __CONFIG_NAT__
			stop_wan();
#endif
#endif
			cyg_thread_delay(200);
			board_reboot();
			
			/* Go back to init and reboot */
			return;

		case RESTART_ALL:
		
			stop_services();
			stop_wan();
			stop_firewall();
			stop_lan();

			start_lan();
			start_firewall();
			if (wan_link_status())
			start_wan();
			update_services();
			start_wl();
			break;
#ifdef __CONFIG_ALINKGW__
		case ALI_ATTACK:
			access_attack_report_all();
			break;

		case ALI_SEC_REPORT:
				
			report_ali_sec_info_direct() ;

			report_ali_sec_info();
			
			break ;
#endif
#if 0
		//add by ll
		case DOWNLOAD_REDIRECT_URL:
			download_redirect_url_thread();
			break ;
		//end by ll
#endif
		default:
			break;
		}
	}
}

/*
 * Main entry of applications
 */
int
rc(int argc, char **argv)
{
	char *base = argv[0];

	if (strstr(base, "init")) {
		/* Enter mainloop */
		main_loop();
		return 0;
	}
#ifdef	__CONFIG_DHCPC__
	if (strstr(base, "wandhcpc")) {
		return wandhcpc(argc, argv);
	}
#ifdef __CONFIG_8021X__
	if (strstr(base, "8021xdhcpc")) {
		return wandhcpc(argc, argv);
	}
#endif
	if (strstr(base, "landhcpc")) {
		return landhcpc(argc, argv);
	}
#ifdef	__CONFIG_NAT__
#ifdef	__CONFIG_PPTP__
	if (strstr(base, "pptpdhcpc")) {
		return pptpdhcpc(argc, argv);
	}
#endif
#ifdef	__CONFIG_L2TP__
	if (strstr(base, "l2tpdhcpc")) {
		return l2tpdhcpc(argc, argv);
	}
#endif
#ifdef  __CONFIG_PPPOE2__
	if (strstr(base, "pppoe2dhcpc")) {
		return pppoe2dhcpc(argc, argv);
	}
#endif
#endif	/* __CONFIG_NAT__ */
#endif	/* __CONFIG_DHCPC__ */

#ifdef	__CONFIG_NAT__
#ifdef	__CONFIG_PPP__
	else if (strstr(base, "ip-up")) {
		return ipup_main(argc, argv);
	}
	else if (strstr(base, "ip-down")) {
		return ipdown_main(argc, argv);
	}
#endif
#endif	/* __CONFIG_NAT__ */
	return 0;
}
