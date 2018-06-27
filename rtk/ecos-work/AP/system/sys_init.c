#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
#include <cyg/hal/romeperf.h>
#endif
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <cyg/io/watchdog.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "sys_utility.h"
#include "hw_settings.h"
#include "net_api.h"
#include "sys_init.h"
#include "apmib.h"
#include "../common/common.h"

// added for traffic control with the fastnat of realtek, by zhuhuan on 2016.03.01
#ifdef CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#define CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#endif

#if ECOS_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
#endif
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
#include <pkgconf/devs_eth_rltk_819x_switch.h>
extern int rtl_vlan_support_enable;
#endif
#ifdef CONFIG_RTL_CUSTOM_PASSTHRU

extern int passThruStatusWlan;
#endif

#if defined(CONFIG_RTL_VAP_SUPPORT)
#define MSSID_INUSE_NUM RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#endif

#ifdef HAVE_SIMPLE_CONFIG
extern int SC_IP_STATUS;
#endif

#if defined(HAVE_TR069)
static int set_start_wan_flag=0;
#endif

#if 0//defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
unsigned int	br0_ip_addr = 0;
unsigned int	br0_ip_mask = 0;
#endif 

static int init_all = 0;
static int skip_wan_link_up=0;

extern void install_exception_handler(void);
extern void bridge_main_fn(void);
extern void bridge_reinit(void);
extern int apply_hw_settings_to_driver(void);
extern void my_wlan_settings(void);
extern void rtl_gpio_init(void);
#ifdef HAVE_WPS
extern void create_wscd(void);
#endif
#ifdef HAVE_PATHSEL
extern void create_pathsel(void);
#endif
#ifdef 	ECOS_DBG_STAT
extern void	dbg_stat_init();
#endif
#ifdef HAVE_MINIIGD
extern void create_miniigd(void);
#endif
extern void create_shell(void);
extern void rtl_gpio_timer(void);
extern void SoftNAT_OP_Mode(int count);

extern void ramfs_init(void);
extern int read_flash_webpage(char *prefix, char *webfile);
extern void create_httpd_thread(void);
extern int set_system_time_flash(void);
#ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
extern void set_multirepeaterFlag(int wlanidx,int isEnable);
#endif
#if defined(HAVE_FIREWALL)
void set_ipfw_rules(char *wan_intf, char *lan_intf);
#endif
#if defined(HAVE_IPV6FIREWALL)
void set_ip6fw_rules(char *wan_intf, char *lan_intf);
#endif
#if defined(CYGPKG_NET_OPENBSD_STACK)
extern int ipforwarding;
#elif defined(CYGPKG_NET_FREEBSD_STACK)
extern int cyg_ipforwarding;
#endif

//unsigned char sys_init_stack[48*1024];
//unsigned char sys_init_stack[32*1024];
//cyg_handle_t sys_init_thread_handle;
//cyg_thread sys_init_thread_obj;

#define STACK_SIZE    1024*16
#define SYS_INIT_NAME   "main"
INIT_STACK_INFO(SYS_INIT_NAME,sys_init,STACK_SIZE)  

cyg_handle_t system_alarm_hdl;
cyg_alarm system_alarm;

unsigned char active_br_wlan_interface[BRIDGE_INTERFACE_MAX_LEN] = {0};
unsigned char last_br_wlan_interface[BRIDGE_INTERFACE_MAX_LEN] = {0};
//dmz related
char dmz_hostaddr[30] = {0};

unsigned char freebsd_Hostmac[6]={0};
#ifdef HAVE_SYSTEM_REINIT
#ifdef HAVE_IWCONTROL 
static int iw_reinit = 0;
#endif
#endif
#ifdef SYSLOG_SUPPORT
extern int do_syslog(int type,char * buf,int len);
extern int syslog_printk(char * buffer);
#define MAX_LOG_BUF 1<<12
void rtl_syslog()
{
	if(do_syslog(SYS_LOG_NUMBER,NULL,0) > 0)
		kick_event(SYS_LOG_EVENT);
}
#endif

#ifdef ECOS_RTL_REPORT_LINK_STATUS
void rtl_link_status_report()
{
	int op_mode,if_type = -1,wispWanId = -1,wlan_mode;
#ifdef HOME_GATEWAY
/*get WAN link status*/
	if_type = get_wan_idx();
	if(if_type < 0)
		goto LAN_LINK_STATUS;
	
	if(get_if_change_status(if_type) >0 ){
		if(get_if_status(if_type) >0){
			kick_event(WAN_LINK_UP_EVENT);
		}
		else if(get_if_status(if_type)== 0){
			kick_event(WAN_LINK_DOWN_EVENT);
		}
		set_if_change_status(if_type,0);
	}
	
LAN_LINK_STATUS:
#endif
/*get LAN link status*/
	return;
	
}
#endif
void system_alarm_func(cyg_handle_t alarm_hdl, cyg_addrword_t data)
{
#ifdef HAVE_WATCHDOG
	watchdog_reset();
#ifdef HAVE_SOFT_WATCHDOG
	externC void softdog_check();
	softdog_check();
#endif
#endif

#ifndef HAVE_NOGPIO
#ifndef CONFIG_RTL_8196CS //no wps button/led, reset button and system led on 96cs
#ifndef HAVE_RLX_PROFILING
	rtl_gpio_timer();
#endif
#endif
#endif

#ifdef ECOS_RTL_REPORT_LINK_STATUS
	rtl_link_status_report();
#endif
#ifdef SYSLOG_SUPPORT
	rtl_syslog();
#endif
}

void sys_reinit(void)
{
	kick_event(REINIT_EVENT);
}

#ifndef HAVE_NOWIFI
#ifdef HAVE_APMIB
void configure_wifi(void)
{
	int i=0, j=0, wlan_disabled=0, wlan_mode=0, op_mode=0;
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
	int wlan_wds_enabled=0, wlan_wds_num=0;
#endif
	char tmpbuff[16],tmpbuff1[16];
	char pWanIP[16],pWanDefIP[16];
	//extern int SetWlan_idx(char * wlan_iface_name);
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
 	int repeater_enabled;
#endif
#ifdef CONFIG_RTK_MESH
 	int mesh_enabled;
#endif

#ifdef HAVE_WATCHDOG
	REG32(BSP_WDTCNR) |= (0xA5000000); //stop watchdog
#endif
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) 
	int wispWanId=0;
#endif

	memset(active_br_wlan_interface, 0x00, sizeof(active_br_wlan_interface));
	/* init wlan interface */
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
	  #if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		if (i>0) {
			break; //don't init wlan1
		}
	  #endif
		memset(tmpbuff,0x00,sizeof(tmpbuff));
		sprintf(tmpbuff, "wlan%d",i);
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
		strcpy(tmpbuff1, tmpbuff);
		strcat(tmpbuff1, "-vxd0");
#endif
		
		if (SetWlan_idx(tmpbuff)) {
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disabled);
			if (wlan_disabled == 1) {
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
				if (is_interface_up(tmpbuff1))
					RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "down", NULL_STR);
#endif									
        			if (is_interface_up(tmpbuff))		
					RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff, "down", NULL_STR);
					
				RunSystemCmd(NULL_FILE, "iwpriv", tmpbuff, "radio_off", NULL_STR);
			}
			else {
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
        			if (is_interface_up(tmpbuff1))
          				RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "down", NULL_STR);
#endif			
				if (is_interface_up(tmpbuff))
          				RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff, "down", NULL_STR);
	
        			RunSystemCmd(NULL_FILE, "flash", "set_mib", tmpbuff, NULL_STR);	
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
       			if (i == 0)
          				apmib_get(MIB_REPEATER_ENABLED1,(void *)&repeater_enabled);
        			else
          				apmib_get(MIB_REPEATER_ENABLED2,(void *)&repeater_enabled);       
        			if (repeater_enabled)
          				RunSystemCmd(NULL_FILE, "flash", "set_mib", tmpbuff1, NULL_STR);
#endif			
				RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff, "up", NULL_STR);
				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				apmib_get(MIB_OP_MODE, (void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
					op_mode=BRIDGE_MODE;
#endif
				if(!(op_mode == WISP_MODE && wlan_mode == CLIENT_MODE)){
					if(active_br_wlan_interface[0] == 0x00)
						sprintf(active_br_wlan_interface, "%s", tmpbuff);
					else
						strcat(active_br_wlan_interface, tmpbuff);
					strcat(active_br_wlan_interface, " ");
				}

#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT

       			if (1==repeater_enabled && (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE))    
       			{
      				RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "up", NULL_STR);  
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
					int wisp_wan_id= -1;
					apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
#endif					
					//we set wlan0-vxd as wan interface when DUT is WISP mode
					if(op_mode !=  WISP_MODE)
					{
						strcat(active_br_wlan_interface, tmpbuff1);
						strcat(active_br_wlan_interface, " ");
					}
#ifdef HOME_GATEWAY
					else
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
					if(i == wisp_wan_id)
#endif
					{
						getWanInfo(pWanIP,NULL,pWanDefIP,NULL);
						/*if wisp mode , add default route.since default route maybe deleted while vxd maybe down/up*/
						RunSystemCmd(NULL_FILE, "route", "add", "-gateway", pWanDefIP, "-interface",tmpbuff1,NULL_STR);
					}
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
					else
					{
						strcat(active_br_wlan_interface, tmpbuff1);
						strcat(active_br_wlan_interface, " ");						
					}
#endif
#endif
       			}
#endif	
#ifdef	CONFIG_RTL_CUSTOM_PASSTHRU

					if(op_mode == WISP_MODE)
					{
						int intValue=0;
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) 

						int repeater_enable1=0;
						int repeater_enable2=0;
						int wisp_wan_id= -1;
						apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
						apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId);
						apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intValue);		
						apmib_get(MIB_REPEATER_ENABLED1,(void *)&repeater_enable1);
						apmib_get(MIB_REPEATER_ENABLED2,(void *)&repeater_enable2);
						if(intValue != 0)
						{
							char tmpStr[16];
							/*should also config wisp wlan index for dual band wireless interface*/
							intValue=((wispWanId&0xF)<<4)|intValue;
							#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT 
							if(repeater_enable1 || repeater_enable2)
								intValue = intValue | 0x8;
							#endif					
							passThruStatusWlan=intValue;	
							//diag_printf("---------wispWanId:%d,repeater_enable1:%d,repeater_enable2:%d,passThruStatusWlan:%x.\n",
							//	wispWanId,repeater_enable1,repeater_enable2,passThruStatusWlan);
							
						}
						else
						{
							passThruStatusWlan=intValue;
						}
						if(i == wisp_wan_id)
#else
		
					apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intValue);
					if(intValue!=0){
						
						#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT 
							if(repeater_enabled && (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE))
								intValue |=0x08;
						#endif		
						passThruStatusWlan=intValue;
					}
					else
					{
						passThruStatusWlan=intValue;
					}					
#endif
					{
#ifdef HOME_GATEWAY
						getWanInfo(pWanIP,NULL,pWanDefIP,NULL);
						/*if wisp mode , add default route.since default route maybe deleted while vxd maybe down/up*/
						RunSystemCmd(NULL_FILE, "route", "add", "-gateway", pWanDefIP, "-interface",tmpbuff1,NULL_STR);
#endif
					}

					//diag_printf("--------------\npassThruStatusWlan:%x,[%s]:[%d].\n",passThruStatusWlan,__FUNCTION__,__LINE__);
					if (passThruStatusWlan == 0)
					{
						if(is_interface_up("pwlan0"))
							interface_down("pwlan0");
						
					}
					else 
					{
						//diag_printf("--------------\npassThruStatusWlan:%x,[%s]:[%d].\n",passThruStatusWlan,__FUNCTION__,__LINE__);
						//config_passThruInfo();
						interface_up("pwlan0");
						if(active_br_wlan_interface[0] == 0x00)
							strcpy(active_br_wlan_interface, "pwlan0");
						else
							strcat(active_br_wlan_interface, "pwlan0");
						strcat(active_br_wlan_interface, " ");
					}
					
				}
#endif
				

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
				apmib_get( MIB_WLAN_WDS_ENABLED, (void *)&wlan_wds_enabled);
				apmib_get( MIB_WLAN_WDS_NUM, (void *)&wlan_wds_num);
				
				if (wlan_wds_num > RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM)
					wlan_wds_num = RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM;
				
				if( (wlan_mode == WDS_MODE || wlan_mode ==AP_WDS_MODE) && (wlan_wds_enabled !=0) &&(wlan_wds_num!=0)) {
					for (j=0; j<wlan_wds_num; j++) {
						sprintf(tmpbuff1, "wlan%d-wds%d", i, j);
						RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "up", NULL_STR);
						strcat(active_br_wlan_interface, tmpbuff1);
						strcat(active_br_wlan_interface, " ");
					}
				}
#endif

#ifdef CONFIG_RTK_MESH
				apmib_get( MIB_WLAN_MESH_ENABLE, (void *)&mesh_enabled);				
				if((wlan_mode ==  AP_MESH_MODE|| wlan_mode == MESH_MODE) && mesh_enabled)
				{
					sprintf(tmpbuff1, "wlan-msh0");
					RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "up", NULL_STR);
					strcat(active_br_wlan_interface, tmpbuff1);
					strcat(active_br_wlan_interface, " ");
				}
#endif

#ifdef CONFIG_RTL_VAP_SUPPORT
				for (j=0; j<RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM; j++) {
					int vap_disable;
					sprintf(tmpbuff1, "%s-va%d", tmpbuff, j);
					SetWlan_idx(tmpbuff1);
					apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&vap_disable);
					if (is_interface_up(tmpbuff1))		
					{
						//down the root interface first, otherwise the vap security will be the same as root from site survey, but the mib is right
						//the root cause is not clear now
						RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "down", NULL_STR);
						if(is_interface_up(tmpbuff))
							RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff, "down", NULL_STR);					
					}

					if( (wlan_disabled == 0) && (vap_disable == 0) && ((wlan_mode == AP_MODE) || (wlan_mode ==AP_WDS_MODE))){
						RunSystemCmd(NULL_FILE, "flash", "set_mib", tmpbuff1, NULL_STR);
						//up all vap interface in set_wlan_bridge() for vap access fail issue
						//RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff, "up", NULL_STR);					
						RunSystemCmd(NULL_FILE, "ifconfig", tmpbuff1, "up", NULL_STR);					
						strcat(active_br_wlan_interface, tmpbuff1);
						strcat(active_br_wlan_interface, " ");
					}	
				}
				if((wlan_disabled == 0) && !is_interface_up(tmpbuff))
				{
					interface_up(tmpbuff);
				}
				SetWlan_idx(tmpbuff);		
            #ifdef CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT
       			if (i == 0)
          		    apmib_get(MIB_REPEATER_ENABLED1,(void *)&repeater_enabled);
        		else
          			apmib_get(MIB_REPEATER_ENABLED2,(void *)&repeater_enabled); 
                /*tell driver: is multirepeater open */
                if(2==repeater_enabled)
                {
                    set_multirepeaterFlag(i,1);
                }else{
                    set_multirepeaterFlag(i,0);
                }
            #endif	
#endif 
#ifdef KLD_ENABLED			//set m2u enabled or not
				int m2uEnabled=0;
				apmib_get(MIB_WLAN_ENHANCE_MODE, (void *)&m2uEnabled); 
				if(m2uEnabled)
				{
					//iwpriv wlan0 set_mib mc2u_disable=0
					RunSystemCmd(NULL_FILE, "iwpriv", tmpbuff,"set_mib","mc2u_disable=0", NULL_STR);
				}
				else
				{
					//iwpriv wlan0 set_mib mc2u_disable=1
					RunSystemCmd(NULL_FILE, "iwpriv", tmpbuff,"set_mib","mc2u_disable=1", NULL_STR);
				}
#endif				
			}
		}

	}

#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
#ifdef HAVE_WATCHDOG
	watchdog_reset();
	watchdog_start();
#endif
}
#ifdef HAVE_WLAN_SCHEDULE
int check_wlSchedule()
{
	int wlschEnable=0;
	int entryNum=0;
	SCHEDULE_T wlan_sched={0};
	int i=0;
	int current=0;
	time_t tm;
	struct tm tm_time;
	time(&tm);
	memcpy(&tm_time, localtime(&tm), sizeof(tm_time));
	current = tm_time.tm_hour * 60 + tm_time.tm_min;

	if(apmib_get(MIB_WLAN_SCHEDULE_ENABLED,(void*)&wlschEnable)==0)
		return -1;
	if(apmib_get(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)==0)
		return -1;

	if(wlschEnable == 0)
		return 1;

	for(i=1; i<entryNum; i++)
	{
		*((char *)&wlan_sched) = (char)i;
		apmib_get(MIB_WLAN_SCHEDULE_TBL, (void *)&wlan_sched);
		if(!wlan_sched.eco)
			continue;
		if(wlan_sched.day==7 || wlan_sched.day==tm_time.tm_wday)
		{//day hit
			if(wlan_sched.fTime<=wlan_sched.tTime)
			{//from<current<to
				if(current>=wlan_sched.fTime && current<=wlan_sched.tTime)
					return 1;
			}else
			{//current<to || current >from
				if(current>=wlan_sched.fTime || current<=wlan_sched.tTime)
					return 1;
			}
		}
	}
	return 0;	
}
void start_wlanSchedule(char onSysInit)
{
	SCHEDULE_T entry[NUM_WLAN_INTERFACE]={0};
	int i=0,enable=0,disableWlanNum=0;
	char buffer[64]={0};
	char tmpBuf[32]={0};
#ifdef CONFIG_WLANIDX_MUTEX
	int s = apmib_save_idx();
#else
	apmib_save_idx();
#endif
	for(i=0;i<NUM_WLAN_INTERFACE;i++)
	{
		apmib_set_wlanidx(i);
		if(!apmib_get(MIB_WLAN_SCHEDULE_ENABLED,(void*)&enable))
		{
			printf("get MIB_WLAN_SCHEDULE_TBL fail!\n");
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif
			return;
		}
		if(!enable) 
		{
			disableWlanNum++;
			continue;
		}
		*((char*)&(entry[i]))=(char)1;
		if(!apmib_get(MIB_WLAN_SCHEDULE_TBL,(void *)&(entry[i])))
		{
			printf("get MIB_WLAN_SCHEDULE_TBL fail!\n");
#ifdef CONFIG_WLANIDX_MUTEX
			apmib_revert_idx(s);
#else
			apmib_revert_idx();
#endif
			return;
		}			
		sprintf(tmpBuf,"wlan%d,%d,%d,%d,",i,entry[i].day,entry[i].fTime,entry[i].tTime);
		strcat(buffer,tmpBuf);
	}
#ifdef CONFIG_WLANIDX_MUTEX
	apmib_revert_idx(s);
#else
	apmib_revert_idx();
#endif
	if(onSysInit)
	{
		if(disableWlanNum==NUM_WLAN_INTERFACE) 
			return;//all wlan schedule disable,nothing todo		
	}else
	{
		if(disableWlanNum!=0)//have wlan schedule be disabled
			configure_wifi();
	// when schedule disbale wlan, then disable schedule, wlan need to reconfig 
		if(disableWlanNum==NUM_WLAN_INTERFACE)
			sprintf(buffer,"disable");
	}
	
	//diag_printf("wlan schedule:%s\n",buffer);
	if(buffer[strlen(buffer)-1]==',')
		buffer[strlen(buffer)-1]='\0';
	//wlanSchedule_startup(tmpBuf);
	//printf("reload %s\n",buffer);
	RunSystemCmd(NULL_FILE, "wlschedule",buffer,NULL_STR);
}
#endif
#endif
#endif

#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
void reconfigure_lan(void)
{
	struct in_addr addr, mask;
	char addrstr[16], maskstr[16];

#ifdef HAVE_APMIB
	unsigned int lan_dhcp_mode;
	
	apmib_get(MIB_DHCP, (void *)&lan_dhcp_mode);
	if(lan_dhcp_mode == DHCP_LAN_SERVER || lan_dhcp_mode == DHCP_LAN_NONE)
	{
		apmib_get(MIB_IP_ADDR,  (void *)&addr);
		apmib_get(MIB_SUBNET_MASK,  (void *)&mask);
		strcpy(addrstr, inet_ntoa(addr));
		strcpy(maskstr, inet_ntoa(mask));
	}
	else//DHCP_LAN_CLIENT
	{
	    #if 0
		strcpy(addrstr, "0.0.0.0");
		strcpy(maskstr, "255.255.255.0");
        #else
        if(!(((getInAddr("eth0", IP_ADDR, &addr))>0) && ((getInAddr("eth0", SUBNET_MASK, &mask))>0) && addr.s_addr>0 && mask.s_addr>0))
        {
            apmib_get(MIB_IP_ADDR,  (void *)&addr);
            apmib_get(MIB_SUBNET_MASK,  (void *)&mask);     
        }
        inet_ntoa_r(addr, addrstr);
        inet_ntoa_r(mask, maskstr);
        #endif
	}
	
	
#else
	strcpy(addrstr, "192.168.1.254");
	strcpy(maskstr, "255.255.255.0");
#endif

//lan ip address
#ifdef HAVE_BRIDGE
#ifdef HAVE_NOETH
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "0.0.0.0", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif	
#else
	//interface_config("eth0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", "0.0.0.0", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif
#else
#ifdef HAVE_NOETH
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "0.0.0.0", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif	
#else
	//interface_config("eth0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", "0.0.0.0", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", addrstr,"netmask" ,maskstr, NULL_STR);	
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", "192.168.0.254", maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "0.0.0.0", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);	
#endif
#endif
#endif

	#if defined(CONFIG_RTL_HARDWARE_NAT)
    rtl_hwNatOnOffByApp();
    #endif
}
#endif

void configure_lan(void)
{
	struct in_addr addr, mask;
	char addrstr[16], maskstr[16];
	int ret1=0, ret2=0,op_mode=0,lan_dhcp=0;

#if 0 //def HAVE_DHCPD
	int lan_type;
	apmib_get(MIB_DHCP,(void*)&lan_type);

	if(lan_type==DHCP_LAN_CLIENT)
		return;
#endif
#ifdef HAVE_APMIB
	apmib_get(MIB_OP_MODE,(void*)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	apmib_get(MIB_DHCP,(void*)&lan_dhcp);

	if((op_mode==BRIDGE_MODE && lan_dhcp==DHCP_LAN_CLIENT)
#ifdef DHCP_AUTO_SUPPORT
		|| (op_mode==BRIDGE_MODE && lan_dhcp==DHCP_AUTO)
#endif
	)
	{
	//printf("%s:%d\n",__FUNCTION__,__LINE__);
		ret1=getInAddr("eth0", IP_ADDR, &addr);
		ret2=getInAddr("eth0", SUBNET_MASK, &mask);
		if(!(ret1>0 && ret2>0 && addr.s_addr>0 && mask.s_addr>0))
		{
			printf("%s:%d\n",__FUNCTION__,__LINE__);
			apmib_get(MIB_IP_ADDR,  (void *)&addr);
			apmib_get(MIB_SUBNET_MASK,  (void *)&mask);		
		}
	}
	else
	{
		//printf("%s:%d\n",__FUNCTION__,__LINE__);
		apmib_get(MIB_IP_ADDR,  (void *)&addr);
		apmib_get(MIB_SUBNET_MASK,  (void *)&mask);	
	}
	inet_ntoa_r(addr, addrstr);
	inet_ntoa_r(mask, maskstr);
	#if 0 //defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	br0_ip_addr = addr.s_addr;
	br0_ip_mask = mask.s_addr;
	#endif
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	extern void rtl_getBrIpAndMask(unsigned long ip, unsigned long mask);
	rtl_getBrIpAndMask(addr.s_addr, mask.s_addr);
	#endif
#else
	strcpy(addrstr, "192.168.1.254");
	strcpy(maskstr, "255.255.255.0");
	#if 0 //defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	br0_ip_addr = 0xc0a801fe;
	br0_ip_mask = 0xffffff00;
	#endif
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
	extern void rtl_getBrIpAndMask(unsigned long ip, unsigned long mask);
	rtl_getBrIpAndMask(0xc0a801fe, 0xffffff00);
	#endif
#endif

//lan ip address
#ifdef HAVE_BRIDGE
#ifdef HAVE_NOETH
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif	
#else
	//interface_config("eth0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif
#else
#ifdef HAVE_NOETH
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);
#endif	
#else
	//interface_config("eth0", addrstr, maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", addrstr,"netmask" ,maskstr, NULL_STR);	
#ifndef HAVE_NOWIFI
	//interface_config("wlan0", "192.168.0.254", maskstr);
	RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", addrstr,"netmask" ,maskstr, NULL_STR);	
#endif
#endif
#endif
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif

}
extern void validate_hw_setings(void);

int rtl_apmib_init()
{
	if (!apmib_init()) {
		printf("apmib_init() failed!\n");
		return FAILED;
	}

	validate_hw_setings();
	return SUCCESS;
}

int set_mac_addr()
{
	unsigned char mac_addr[16];
#ifdef 	HOME_GATEWAY
	unsigned int op_mode;
#endif

	int i;
	
#ifdef CYGHWR_NET_DRIVER_ETH0
	apmib_get(MIB_ELAN_MAC_ADDR,  (void *)mac_addr);
	if(!memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
		apmib_get(MIB_HW_NIC0_ADDR,  (void *)mac_addr);
	
	if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
		set_mac_address("eth0", mac_addr);
	else
	{
		strcpy(mac_addr,"\x00\xe0\x4c\x81\x96\xc1");
		set_mac_address("eth0", mac_addr);
	}
	for(i=0; i<6;i++)
	{
		freebsd_Hostmac[i] = mac_addr[i];
	}
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	if(rtl_vlan_support_enable){
		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth2", mac_addr);
		else
			set_mac_address("eth2", "\x00\xe0\x4c\x81\x96\xc1");
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH3
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth3", mac_addr);
		else
			set_mac_address("eth3", "\x00\xe0\x4c\x81\x96\xc1");
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth4", mac_addr);
		else
			set_mac_address("eth4", "\x00\xe0\x4c\x81\x96\xc1");
		#endif

        #ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth7", mac_addr);
		else
			set_mac_address("eth7", "\x00\xe0\x4c\x81\x96\xc1");
		#endif
	}
#endif


#ifdef 	HOME_GATEWAY

#ifdef CYGHWR_NET_DRIVER_ETH1
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	if(op_mode == GATEWAY_MODE)
	{
		apmib_get(MIB_WAN_MAC_ADDR, (void *)mac_addr);
		if(!memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))			
			apmib_get(MIB_HW_NIC1_ADDR,  (void *)mac_addr);
		
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth1", mac_addr);
		else
			set_mac_address("eth1", "\x00\xe0\x4c\x81\x96\xc9");
	}
	else if(op_mode == BRIDGE_MODE)
	{
		apmib_get(MIB_ELAN_MAC_ADDR,  (void *)mac_addr);
		if(!memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			apmib_get(MIB_HW_NIC1_ADDR,  (void *)mac_addr);
		
		if(memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
			set_mac_address("eth1", mac_addr);
		else
			set_mac_address("eth1", "\x00\xe0\x4c\x81\x96\xc9");
	}
	else if(op_mode == WISP_MODE)
	{
		int i;
		char tmpbuf[16];
		//unsigned char const_mac[ETH_ALEN]={0x00,0xe0,0x4c,0x81,0x96,0xc9};
		int wisp_wan_id= -1;
		char wlan_name[15],wlan_vxd_name[15];		
		apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
		
		for(i=0;i<NUM_WLAN_INTERFACE;i++)
		{
			memset(mac_addr,'\0',sizeof(mac_addr)); 			
			memset(wlan_name,'\0',sizeof(wlan_name));
			memset(wlan_vxd_name,'\0',sizeof(wlan_vxd_name));			
			if(i == wisp_wan_id)
			{						
				apmib_get(MIB_WAN_MAC_ADDR, (void *)mac_addr);
				if(!memcmp(mac_addr, "\x00\x00\x00\x00\x00\x00", ETH_ALEN))
					apmib_get_ext(MIB_HW_WLAN_ADDR,  (void *)mac_addr,i,0);					
			
			//else
				//apmib_get_ext(MIB_HW_WLAN_ADDR,  (void *)mac_addr,i,0);			
	
			sprintf(wlan_vxd_name,"wlan%d-vxd0",i);
			sprintf(wlan_name,"wlan%d",i);			
			set_mac_address(wlan_name, mac_addr);	
			set_mac_address(wlan_vxd_name, mac_addr);	
			}	
		}
	}
	#endif
#endif
	return SUCCESS;
	
}
#ifdef 	HOME_GATEWAY

void set_wan_netif_mtu(void)
{
	int intValue=0, op_mode=0, wan_type=0;

	#ifndef HAVE_NOETH
	extern int rtl_setWanNetifMtu(int mtu);
	#endif
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
		op_mode=BRIDGE_MODE;
#endif
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	
	if(op_mode==GATEWAY_MODE){
		if(wan_type==STATIC_IP)
			apmib_get(MIB_FIXED_IP_MTU_SIZE, (void *)&intValue);
		#ifdef HAVE_DHCPC
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        else if(wan_type == DHCP_CLIENT || wan_type == DHCP_PLUS)
        #else
		else if(wan_type==DHCP_CLIENT)
        #endif
			apmib_get(MIB_DHCP_MTU_SIZE, (void *)&intValue);
		#endif
	}
	#ifndef HAVE_NOETH
	rtl_setWanNetifMtu(intValue);
	return 1;
	#endif
}

#endif
int set_WPS_pin()
{
	unsigned char tmpBuff[16]={0};
#ifdef KLD_ENABLED
	unsigned char tmpBuff2[16]={0};
#endif
	apmib_get(MIB_HW_WSC_PIN,  (void *)tmpBuff);
	if(!memcmp(tmpBuff, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN))
	{
		generate_pin_code(tmpBuff);
		apmib_set_ext(MIB_HW_WSC_PIN,  (void *)tmpBuff, 0, 0);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		apmib_set_ext(MIB_HW_WSC_PIN,  (void *)tmpBuff, 1, 0);
#endif
		printf("Generated PIN = %s\n", tmpBuff);
		apmib_update(HW_SETTING);
	}
#ifdef KLD_ENABLED
	apmib_get(MIB_WLAN_WSC_PIN,  (void *)tmpBuff2);
	if(!memcmp(tmpBuff2, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN))
	{
		apmib_set_ext(MIB_WLAN_WSC_PIN,  (void *)tmpBuff, 0, 0);
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
		apmib_set_ext(MIB_WLAN_WSC_PIN,  (void *)tmpBuff, 1, 0);
#endif
		printf("Generated PIN = %s\n", tmpBuff);
		apmib_update(CURRENT_SETTING);
	}
#endif
	return SUCCESS;
}

void set_wlan_app()
{
	int lan_type;
	apmib_get(MIB_DHCP,(void*)&lan_type);	

	if(lan_type==DHCP_LAN_CLIENT)
	{
		int retval=0;
		struct in_addr ip_addr;
		retval=getInAddr("eth0", IP_ADDR, &ip_addr);
		if(retval!=1 || ip_addr.s_addr==0)
			return;		
	}

#ifdef HAVE_WPS	
	start_wps();	
#endif
#ifdef HAVE_WLAN_SCHEDULE
	start_wlanSchedule(0);
#endif
}

void start_sys_timer()
{
	cyg_handle_t system_counter_hdl, system_clk_hdl;

	system_clk_hdl = cyg_real_time_clock();
	cyg_clock_to_counter(system_clk_hdl, &system_counter_hdl);
	cyg_alarm_create(system_counter_hdl, system_alarm_func, 0,
				&system_alarm_hdl, &system_alarm);
	cyg_alarm_initialize(system_alarm_hdl, cyg_current_time()+100, 100);
	cyg_alarm_enable(system_alarm_hdl);
}


#if 0
void itostr(int val, unsigned char *tmpstr)
{
	if(!tmpstr)
		return;
	
	int i, j, k;
	i=val;
	k=0;
	do
	{
		j=i%10;
		i=i/10;
		tmpstr[k++]='0'+j;
	}while(i);	
	
	tmpstr[k]=0;
	unsigned char c;
	for(j=0; j<k && k>0; j++, k--)
	{
		c=tmpstr[j];
		tmpstr[j]=tmpstr[k-1];
		tmpstr[k-1]=c;
	}	
}
#endif

void create_dhcpd_configfile(struct in_addr *srvip, struct in_addr *startip, struct in_addr *endip)
{
	if(!srvip || !startip || !endip)
		return;
	
	unsigned char srvipaddr[16], snetmask[16], startaddr[16], endaddr[16], addrip[16], defgway[16];
	struct in_addr snet_mask, *addr, def_gway;
	unsigned char tmpbuf[512];
	unsigned long lease_time;	

	int snet_id, flag=0;
	unsigned char tmpstr[8];

	int val;
	unsigned char str_dns1[16], str_dns2[16], str_dns[64];	
	struct in_addr dns1, dns2;
	inet_ntoa_r(*srvip, srvipaddr);
//	diag_printf("%s:%d	srv_ipaddr=%s\n", __FUNCTION__, __LINE__, srvipaddr);	
	apmib_get(MIB_SUBNET_MASK, (void*)&snet_mask);	

	apmib_get(MIB_DEFAULT_GATEWAY,  (void *)&def_gway);
	if(def_gway.s_addr!=0 && (def_gway.s_addr & snet_mask.s_addr == srvip->s_addr & snet_mask.s_addr))
	{
		inet_ntoa_r(def_gway, defgway);
//		diag_printf("%s:%d	def_gway=%s\n", __FUNCTION__, __LINE__, defgway);
	}
	else
		strcpy(defgway, srvipaddr);
	
	inet_ntoa_r(snet_mask, snetmask);
//	diag_printf("%s:%d	subnet_mask=%s\n", __FUNCTION__, __LINE__, snetmask);

	apmib_get(MIB_DHCP_LEASE_TIME, (void*)&lease_time);
	lease_time *=60;
//	diag_printf("%s:%d	lease_time=%u\n", __FUNCTION__, __LINE__, lease_time);

#ifdef KLD_ENABLED
	apmib_get(MIB_DNSRELAY_ENABLED, (void *)&val);
	if(val==1)		
		snprintf(tmpbuf, sizeof(tmpbuf), "common:snmk=%s:dfll=%u:maxl=%u:dnsv=%s:rout=%s:\n", snetmask, lease_time, lease_time, srvipaddr, defgway);	
	else //disable dns_relay
	{
		apmib_get(MIB_DNS_MODE, (void *)&val);
		if(val==1) //manual
		{
			apmib_get(MIB_DNS1, (void *)&dns1);
			if(dns1.s_addr!=0)
			{
				inet_ntoa_r(dns1, str_dns1);
				strcpy(str_dns, str_dns1);
			}
			
			apmib_get(MIB_DNS2, (void *)&dns2);
			if(dns2.s_addr!=0)
			{
				inet_ntoa_r(dns2, str_dns2);
				if(dns1.s_addr!=0)
				{
					strcat(str_dns, " ");
					strcat(str_dns, str_dns2);
				}
				else
					strcpy(str_dns, str_dns2);
			}	
			
			if(dns1.s_addr==0 && dns2.s_addr==0)				
				snprintf(tmpbuf, sizeof(tmpbuf), "common:snmk=%s:dfll=%u:maxl=%u:rout=%s:\n", snetmask, lease_time, lease_time, defgway);			
			else	
				snprintf(tmpbuf, sizeof(tmpbuf), "common:snmk=%s:dfll=%u:maxl=%u:dnsv=%s:rout=%s:\n", snetmask, lease_time, lease_time, str_dns, defgway);	
		}
		else  //auto 			
			snprintf(tmpbuf, sizeof(tmpbuf), "common:snmk=%s:dfll=%u:maxl=%u:rout=%s:\n", snetmask, lease_time, lease_time, defgway);			
	}	
#else	
	snprintf(tmpbuf, sizeof(tmpbuf), "common:snmk=%s:dfll=%u:maxl=%u:dnsv=%s:rout=%s:\n", snetmask, lease_time, lease_time, srvipaddr, defgway);	
#endif
	write_line_to_file(DHCPD_ADDRPOOL_DB, 1, tmpbuf);

#if 0
	for(addr=startip; addr->s_addr<=endip->s_addr; addr->s_addr++)
	{
		if(addr->s_addr==srvip->s_addr)
			continue;	
		
		snet_id=addr->s_addr & 0x000000ff;
		itostr(snet_id, tmpstr);

		inet_ntoa_r(*addr, addrip);		
		snprintf(tmpbuf, sizeof(tmpbuf), "%s:ipad=%s:tblc=common:\n",tmpstr, addrip);		
		write_line_to_file(DHCPD_ADDRPOOL_DB, 2, tmpbuf);

		flag++;
	}	
#endif

	snprintf(tmpbuf, sizeof(tmpbuf), "server ip:%s\n", srvipaddr);
	write_line_to_file(DHCPD_ADDRPOOL_DB, 2, tmpbuf);
	
	snprintf(tmpbuf, sizeof(tmpbuf), "start ip:%s\n", inet_ntoa(*startip));
	write_line_to_file(DHCPD_ADDRPOOL_DB, 2, tmpbuf);

	snprintf(tmpbuf, sizeof(tmpbuf), "end ip:%s\n", inet_ntoa(*endip));
	write_line_to_file(DHCPD_ADDRPOOL_DB, 2, tmpbuf);

}

void create_staticip_file()
{
	int fh;
	if((fh = open("/etc/static_ip_list", O_RDWR|O_CREAT|O_TRUNC))<0)
		return ;		
	
	int enable, num, i;
	unsigned char tmpbuf[128], ipaddr[16], snetmask[16], subnet[16];
	unsigned char macaddr[6], smac_addr[13];
//	char *pch=NULL;
	DHCPRSVDIP_T staticIPEntry;

	struct in_addr static_ip, net_mask, sub_net;
	
#ifdef KLD_ENABLED
	apmib_get(MIB_IP_MAC_BINDING, (void *)&enable);
#else
	apmib_get(MIB_DHCPRSVDIP_ENABLED, (void *)&enable);
#endif
	
	if(enable)
	{
		apmib_get(MIB_DHCPRSVDIP_TBL_NUM, (void *)&num);
		apmib_get(MIB_SUBNET_MASK, (void*)&net_mask);	
		inet_ntoa_r(net_mask, snetmask);
		
		for(i=1;i<=num;i++)
		{
			*((char *)&staticIPEntry) = (char)i;
			apmib_get(MIB_DHCPRSVDIP_TBL, (void *)&staticIPEntry);
            
#ifdef IPCONFLICT_UPDATE_FIREWALL
            //diag_printf("%s:%d ipAddr=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)staticIPEntry.ipAddr)));
            *((unsigned int*)(staticIPEntry.ipAddr))=get_conflict_ip(*((unsigned int*)(staticIPEntry.ipAddr)));
            //diag_printf("%s:%d ipAddr=%s \n",__FUNCTION__,__LINE__,inet_ntoa(*((struct in_addr*)staticIPEntry.ipAddr)));
#endif
			#ifdef KLD_ENABLED
			if(staticIPEntry.enabled)
			#endif
			{
				memset(&static_ip, 0, sizeof(struct in_addr));
				memcpy(&static_ip, staticIPEntry.ipAddr, sizeof(struct in_addr));		
				inet_ntoa_r(static_ip, ipaddr);
	//			pch=strrchr(ipaddr, '.');
	//			if(pch)
	//				pch++;
				
				memset(macaddr, 0, 6);
				memcpy(macaddr, staticIPEntry.macAddr, 6);
				sprintf(smac_addr, "0x%02x%02x%02x%02x%02x%02x", 
					macaddr[0],macaddr[1],macaddr[2],macaddr[3],macaddr[4],macaddr[5]);
				
	//			sub_net.s_addr=static_ip.s_addr & net_mask.s_addr;
	//			inet_ntoa_r(sub_net, subnet);			
				
	//			sprintf(tmpbuf, "1:%s:%s:1:%s:\"infinity\":%s\n", smac_addr, subnet, smac_addr, pch);
				sprintf(tmpbuf, "%s:%s\n", ipaddr, smac_addr);
				write(fh, tmpbuf, strlen(tmpbuf));
			}
		}
	}
	close(fh);
}

#ifdef ECOS_DBG_STAT
int dbg_dhcpd_index=0;
int dbg_iw_index=0;
#endif

void set_dhcpd()
{
	struct in_addr srv_ipaddr, start_addr, end_addr;
	int fh;
	
	apmib_get(MIB_IP_ADDR, (void*)&srv_ipaddr);	
//	strncpy(srvipaddr, inet_ntoa(srv_ipaddr), 16);
//	diag_printf("%s:%d	srv_ipaddr=%s\n", __FUNCTION__, __LINE__, srvipaddr);	
	
	apmib_get(MIB_DHCP_CLIENT_START, (void*)&start_addr);
//	strncpy(startaddr, inet_ntoa(start_addr), 16);
//	diag_printf("%s:%d	start_addr=%s\n", __FUNCTION__, __LINE__, startaddr);

	apmib_get(MIB_DHCP_CLIENT_END, (void*)&end_addr);
//	strncpy(endaddr, inet_ntoa(end_addr), 16);
//	diag_printf("%s:%d	end_addr=%s\n", __FUNCTION__, __LINE__, endaddr);
	
	create_dhcpd_configfile(&srv_ipaddr, &start_addr, &end_addr);

	create_staticip_file();
	
#if 0
	fh = open(DHCPD_BINDING_DB, O_RDWR|O_CREAT|O_TRUNC);
	if (fh < 0) 
	{
		diag_printf("Create file %s error!\n", DHCPD_BINDING_DB);
		return 0;
	}
	else
	{		
#if 1
		char tmpbuf[512];
		sprintf(tmpbuf, "%s\n", "1:0x00269ef3e983:192.168.1.0:1:0x00269ef3e983:\"infinity\":105");
		write(fh, tmpbuf, strlen(tmpbuf));
		
		sprintf(tmpbuf, "%s\n", "1:0x0021709da92f:192.168.1.0:1:0x0021709da92f:\"infinity\":110");
		write(fh, tmpbuf, strlen(tmpbuf));
#endif
		close(fh);
	}
#endif

#ifdef ECOS_DBG_STAT
	dbg_dhcpd_index=dbg_stat_register("dhcpd");
#endif

	//start dhcpd 
	cyg_uint8 lanDnsFromWanDhcp=0;
#ifdef KLD_ENABLED
	int dnsRelayEnabled;
	int dnsMode;
	apmib_get(MIB_DNSRELAY_ENABLED, (void *)&dnsRelayEnabled);
	apmib_get(MIB_DNS_MODE, (void *)&dnsMode);
	if(dnsRelayEnabled==0 && dnsMode==0)
		lanDnsFromWanDhcp=1;
#endif

	dhcpd_startup(lanDnsFromWanDhcp);
}

#if 0  /*removed by HF*/
void tenda_sys_init(cyg_addrword_t data)
{
	cyg_flag_value_t flag_val;
#ifdef HAVE_APMIB
	unsigned char tmpbuff[16], *ptr;
	int ret, op_mode;
#endif
	extern int set_mac_address(const char *interface, char *mac_address);
	extern int get_thread_info_by_name(char *name, cyg_thread_info *pinfo);
	extern void validate_hw_setings(void);

	install_exception_handler();

	cyg_flag_init(&sys_flag);
	ramfs_init();

#ifndef CONFIG_RTL_8196CS //for 96cs, gpio init should have done at wlan driver
#ifndef HAVE_RLX_PROFILING
	rtl_gpio_init();
#endif
#endif

#ifdef HAVE_APMIB
	ret = rtl_apmib_init();
	if(ret == FAILED)
		return;
	
	set_system_time_flash();
	set_mac_addr();

#ifndef HAVE_NOWIFI
#ifdef HAVE_WPS
	set_WPS_pin();
#endif
	configure_wifi();
	memcpy(last_br_wlan_interface, active_br_wlan_interface, sizeof(last_br_wlan_interface));
#endif

#ifdef HAVE_BRIDGE
	bridge_main_fn();
#endif

	configure_lan();
	
#else
	apply_hw_settings_to_driver();
#ifndef HAVE_NOWIFI
	my_wlan_settings();
#endif
#endif

//extern cyg_sem_t httpd_sem;

	//eth1
#ifndef HAVE_NOETH
	// 1:bridge/WISP, 2:gateway
	// By default, DUT operates as a pure AP, not a gateway.
	// So we add wan port to eth0 vlan, VID[9].
	// As a result, there is no member port in eth1 vlan, VID[8].
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
	if(op_mode == GATEWAY_MODE)
	{
		SoftNAT_OP_Mode(2);
	}
	else if((op_mode == BRIDGE_MODE) || (op_mode == WISP_MODE))
	{
		SoftNAT_OP_Mode(1);
	}
#endif

#if defined(CYGPKG_NET_OPENBSD_STACK)
	ipforwarding = 1;
#elif defined(CYGPKG_NET_FREEBSD_STACK)
	//ipforwarding = 1;
	cyg_ipforwarding = 1;
#endif

#ifdef HAVE_DHCPD
	set_dhcpd();
#endif

#ifndef HAVE_NOWIFI
	set_wlan_app();
#endif

	start_sys_timer();
#ifdef HAVE_WATCHDOG
	watchdog_start();
#endif
    
#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
	rlx_romeperfInit(1);
#endif

	RunSystemCmd(NULL_FILE, "ipfw", "add", "65534", "allow", "ip", "from", "any", "to", "any", NULL_STR);

	if(op_mode == GATEWAY_MODE)
	{
		start_wan();
	}

#ifdef HAVE_HTTPD
	if (read_flash_webpage(".", "") < 0)
		printf("fail to load web pages\n");

	diag_printf("Creating httpd Thread...\n");
	cyg_httpd_start();

	save_cs_to_file();
#endif

#if 1//def HAVE_MROUTED
	
	diag_printf("------------------Creating mrouted Thread...\n");
	create_mrouted("eth1","eth0");

#endif

	
	while (1) {
		flag_val = cyg_flag_wait(&sys_flag, 0xffff, CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
		printf("sys_flag=%x\n", flag_val);
		if (flag_val & RESET_EVENT) {
			do_reset(1);
		}
		else if (flag_val & RELOAD_DEFAULT_EVENT) {
#ifdef HAVE_APMIB
			RunSystemCmd(NULL_FILE, "flash", "reset", NULL_STR);
			printf("reset settings to default\n");
#endif
		}
		else if(flag_val & WLAN_BRIDGE_EVENT)
		{
			
			configure_wifi();
#ifdef HAVE_BRIDGE
			reset_wlan_bridge();
#endif
			configure_lan();
			set_wlan_app();
			//cyg_semaphore_post(&httpd_sem);
			
		}
		else if(flag_val & WLAN_REINIT_EVENT)
		{
			configure_wifi();
		}
		else if (flag_val & REINIT_EVENT) {
#ifdef HAVE_WPS
			int wlan_disable=0, wps_disable=0;
			cyg_thread_info tinfo;
#endif
			extern void wsc_reinit(void);
			extern void wsc_stop_service(void);
			//receive event from wsc
			//printf("system reinit\n");
			//reinit wlan
#ifndef HAVE_NOWIFI
#ifdef HAVE_APMIB
			configure_wifi();
#endif
#endif
#ifdef HAVE_BRIDGE
			reset_wlan_bridge();
			configure_lan();
#endif
#ifndef HAVE_NOWIFI
#ifdef HAVE_WPS
#ifdef HAVE_APMIB
			apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable);
			apmib_get(MIB_WLAN_WSC_DISABLE, (void *)&wps_disable);
#endif
			if (get_thread_info_by_name("wscd", &tinfo)) {
				if (!wlan_disable && !wps_disable)
					wsc_reinit(); //trigger wsc to reinit
				else
					wsc_stop_service(); //trigger wsc to stop service
			}
			else {
				if (!wlan_disable && !wps_disable)
					create_wscd();
			}
#endif
#endif
		}
		else if(flag_val & WLAN_IAPP_EVENT)
		{
			
		}
		else if(flag_val & WLAN_APP_EVENT)
		{
			system("reboot");
		}
		else if(flag_val & WAN_EVENT)
		{
			start_wan();
		}
		else if(flag_val & LAN_EVENT)
		{
			
		}
		else if(flag_val & FIREWARE_EVENT)
		{
			 set_ipfw_rules(wan_intf, lan_intf);
		}
		else {
			printf("invalid sys_flag value: 0x%x\n", flag_val);
		}
	}
}
#endif

#if defined(HAVE_TR069)
void addRouteForACS()
{
	FILE *fp=NULL;
	unsigned char acs_url[CWMP_ACS_URL_LEN]={0};
	unsigned char acs_ip[16]={0};
	char *pch=NULL, *psubch=NULL, *pstart=NULL, *pend=NULL;
	char *pstr=NULL;
	int count=0;	
	apmib_get(MIB_CWMP_ACS_URL, (void *)acs_url);
	//printf("%s:%d ###acs_url=%s\n",__FUNCTION__,__LINE__,acs_url);
	
	if(strlen(acs_url)<1)
		return;
		
	pstr=acs_url;
	while(count<4 && *pstr!='\0')
	{			
		pch=strchr(pstr, '.');
		if(pch==NULL)
			return; 
		for(psubch=pch-1; psubch>=pstr && isdigit(*psubch); psubch--) ;

		psubch++;
		if(psubch<pch)
		{
			if(count==0)
				pstart=psubch;			
			count++;							
		}
		else
		{
			pstart=NULL;
			count=0;
		}				
		if(count==3)
		{
			for(psubch=pch+1; psubch<acs_url+strlen(acs_url) && isdigit(*psubch); psubch++) ;
			psubch--;
			if(psubch>pch)
			{
				pend=psubch;
				break;
			}
			else
			{
				pstart=NULL;
				count=0;
			}
		}
		pstr=pch+1;
	}
	if(pstart && pend && pend>pstart)
		strncpy(acs_ip, pstart, pend-pstart+1);
	else
		return;		
	
	//printf("%s:%d ###acs_ip=%s\n",__FUNCTION__,__LINE__,acs_ip);

	if((fp=fopen("/var/dhcpc_route.conf", "r+"))==NULL)
		return;	
	
	unsigned char routebuf[16];
	unsigned char cmdbuf[128];
	
	fscanf(fp, "%s", routebuf);
	fclose(fp);
	
	//sprintf(cmdbuf, "route add -host %s gw %s dev eth1", acs_ip, routebuf);
	//system(cmdbuf);
	
	RunSystemCmd(NULL_FILE, "route", "add", "-host", acs_ip, "-netmask", "255.255.255.255", "-gateway", routebuf, "-interface", "eth1", NULL_STR);	
}
#endif

void setDefaultGateway(int disc_flag)
{
	int wanip_dynamic=0, wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	int pppoeWithDhcpEnabled = 0;
#ifdef CONFIG_RTL_DHCP_PPPOE	
	apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
#endif
	
	if(wan_type==L2TP)		
		apmib_get(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&wanip_dynamic);
	else if(wan_type==PPTP)
		apmib_get(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&wanip_dynamic);
	else if(wan_type==PPPOE && pppoeWithDhcpEnabled)
		wanip_dynamic=0;
	else		
		return;

	FILE *fp=NULL;
	unsigned char routebuf[16]={0};
	
	if(wanip_dynamic==L2TP_PPTP_STATIC_IP)
	{	
		struct in_addr route_ip;
		if(wan_type==L2TP)
			apmib_get(MIB_L2TP_DEFAULT_GW,  &route_ip);
		else if(wan_type==PPTP)
			apmib_get(MIB_PPTP_DEFAULT_GW,  &route_ip);
		else
			return;	
		inet_ntoa_r(route_ip, routebuf);
	}
	else
	{
		if((fp=fopen("/etc/dhcpc_route.conf", "r+"))!=NULL)
		{
			fscanf(fp, "%s", routebuf);
			fclose(fp);	
		}
	}

	if(disc_flag) 
	{		
		//when l2tp/pptp disconnect, reset default gateway
		RunSystemCmd(NULL_FILE, "route", "add", "-gateway", routebuf, NULL_STR);	
		return;
	}
	
	if(routebuf[0])
		RunSystemCmd(NULL_FILE, "route", "delete", "-gateway", routebuf, NULL_STR);
	
	if((fp=fopen("/etc/wan_info", "r+"))==NULL)
		return;

	char linebuf[64], gw_ip[16]; 
	char *pbuf=NULL;
	while(fgets(linebuf, sizeof(linebuf), fp))
	{			
		for(pbuf=linebuf; *pbuf!='\n' && *pbuf!='\0'; pbuf++) ;
		*pbuf='\0';

		if((pbuf=strchr(linebuf, ':'))==NULL)
			goto ERR_EXIT;
		if(strncmp(linebuf, "gateway", pbuf-linebuf)==0)
		{
			pbuf++;
			strcpy(gw_ip, pbuf);
			break;
		}
	}	
	
	RunSystemCmd(NULL_FILE, "route", "add", "-gateway", gw_ip, NULL_STR);	

ERR_EXIT:
	fclose(fp);	
}
/*Please note . Event is a bitsmask*/
void kick_event(unsigned int event)
{	
#ifdef SYS_INIT_USING_MBOX
	int ret;
	unsigned int *puint=malloc(sizeof(unsigned int));
	if(NULL == puint)
	{
		diag_printf("%s %d malloc failed\n",__FUNCTION__,__LINE__);
		return;
	}
	*puint=event;
	ret=cyg_mbox_tryput(sys_mbox_hdl,puint);
	if(ret <=0)
	{
		diag_printf("sys mbox is full event 0x%x will lost\n",event);
	}
#else
    cyg_flag_setbits(&sys_flag, event);	
#endif
}

#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)
#define SSID_LEN	32

typedef enum _wlan_mac_state {
    STATE_DISABLED=0, STATE_IDLE, STATE_SCANNING, STATE_STARTED, STATE_CONNECTED, STATE_WAITFORKEY
} wlan_mac_state;

typedef struct _bss_info {
    unsigned char state;
    unsigned char channel;
    unsigned char txRate;
    unsigned char bssid[6];
    unsigned char rssi, sq;	// RSSI  and signal strength
    unsigned char ssid[SSID_LEN+1];
} bss_info;

int indicate_wisp_st = 0;

void indicate_wisp_status(void)
{
	bss_info bss;
	char strWanIP[16];

	if (indicate_wisp_st == 0) {
		system_led_off();
		return;
	}

	getWlBssInfo("wlan0-vxd0", &bss);
	getWanInfo(strWanIP,NULL,NULL,NULL);

	if (bss.state == STATE_CONNECTED && strcmp(strWanIP, "0.0.0.0"))
		system_led_on();
	else
		system_led_off();
}
#endif /* #if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)  */

#if defined(HAVE_FIREWALL)
void check_wan_connect(int wantype)
{
    
	struct in_addr addr = {0};
	char wan_intf[MAX_NAME_LEN] = {0};
	char lan_intf[MAX_NAME_LEN] = {0};
    int lan_type = 0;
	
	apmib_get(MIB_DHCP,(void*)&lan_type);
	getInterfaces(lan_intf,wan_intf);
    
    if ((lan_type == DHCP_LAN_CLIENT)&&(getInAddr(wan_intf, IP_ADDR, (void *)&addr)==1) && (addr.s_addr != 0))
    {
        /* wan connected.ip address of lan side may different with the default mib value.
                 so need to reset firewall rules.expecially for soure nat rule. 
             */
        set_ipfw_rules(wan_intf,lan_intf);
        #ifdef HAVE_IPV6FIREWALL
		set_ip6fw_rules(wan_intf,lan_intf);
		#endif
    }

    return;
}
#endif

void backup_file(char *from_file, char *to_file)
{
	if(from_file==NULL || to_file==NULL)
		return ;
	
	FILE *fp=NULL;
	unsigned char dns_buf[32];
	int count=0;
	
	if((fp=fopen(from_file,"r"))==NULL)
		return ;
	
	while(fgets(dns_buf, sizeof(dns_buf), fp))
	{
		if(strstr(dns_buf, "0.0.0.0")!=NULL)
				continue;
		
		dns_buf[strlen(dns_buf)]='\0';
		
		if(count==0)
			write_line_to_file(to_file, 1, dns_buf);
		else
			write_line_to_file(to_file, 2, dns_buf);

		count++;
	}
	fclose(fp);	
}

void enter_loop(void)
{
#ifdef SYS_INIT_USING_MBOX
	unsigned int flag_val, *puint;
#else
	cyg_flag_value_t flag_val;
#endif
	int wan_type, lan_type, op_mode;
#ifdef HOME_GATEWAY	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	apmib_get(MIB_DHCP, (void *)&lan_type);
	int set_firewall_flag=0;	
	int ppp_disconnect_flag=0;
#endif

	int pppoeWithDhcpEnabled = 0;
#ifdef CONFIG_RTL_DHCP_PPPOE	
	apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
#endif

	while (1) {
#ifdef SYS_INIT_USING_MBOX	
		puint=(unsigned int *)cyg_mbox_get(sys_mbox_hdl);
		if(puint) {
			flag_val=*puint;
			/*free the memory*/
			free(puint);
		}	
		else
			continue;
#else
		flag_val = cyg_flag_wait(&sys_flag, 0xffffffff, 
			CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
#endif
//		diag_printf("sys_flag=%x\n", flag_val);
		if (flag_val & RESET_EVENT) {
#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS) 
			indicate_wisp_st = 0;	
#endif			
			do_reset(1);
		}
#ifdef HAVE_SYSTEM_REINIT	
		else if(flag_val & REINIT_SYSTEM_EVENT) {
			unsigned long flag;
			flag=get_reinit_flag();
			sys_reinit_main(flag);				
		}
#endif		
		else if (flag_val & RELOAD_DEFAULT_EVENT) {
#ifdef HAVE_APMIB
			RunSystemCmd(NULL_FILE, "flash", "reset", NULL_STR);
			diag_printf("reset settings to default\n");
#endif
			do_reset(1);
		}

		else if(flag_val & WLAN_BRIDGE_EVENT)
		{
#ifndef HAVE_NOWIFI
			configure_wifi();

			set_wlan_app();
#endif
#ifdef HAVE_BRIDGE
			reset_wlan_bridge();
#endif
			configure_lan();
			//cyg_semaphore_post(&httpd_sem);
			
		}

		else if(flag_val & WLAN_REINIT_EVENT)
		{
#ifndef HAVE_NOWIFI
			configure_wifi();
			set_wlan_app();
#endif
		}
		else if (flag_val & REINIT_EVENT) {
			//receive event from wsc
			//printf("system reinit\n");
			//reinit wlan


#ifndef HAVE_NOWIFI
#ifdef HAVE_APMIB
			configure_wifi();
#endif
#endif
#ifdef HAVE_BRIDGE
			reset_wlan_bridge();
			configure_lan();
#endif
#ifndef HAVE_NOWIFI
#ifdef HAVE_WPS
			
		start_wps();
#endif
#endif
		}
		else if(flag_val & WLAN_IAPP_EVENT)
		{
			
		}
		else if(flag_val & WLAN_APP_EVENT)
		{
			system("reboot");
		}
#ifdef 	HOME_GATEWAY
		else if(flag_val & DHCP_EVENT)
		{
			apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
			
		#ifdef CONFIG_RTL_DHCP_PPPOE	
			apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
		#endif
		
		#if defined(HAVE_TR069)
			if(wan_type==PPPOE && pppoeWithDhcpEnabled)
			{
				addRouteForACS();		

				if(!isFileExist("/etc/ppp_link")) //pppoe not connected
				{
					//diag_printf("%s:%d**********DHCP_EVENT!******\n",__FUNCTION__,__LINE__);
					check_wan_ip();
					syslogAll_printk("start wan applications ...\n");
#if defined(HAVE_SYSTEM_REINIT)
					if(set_start_wan_flag==1)
						kill_wan_app();
#endif

					if(!isFileExist("/etc/resolv.conf"))
						backup_file("/etc/resolv.conf.backup", "/etc/resolv.conf");
					else
						backup_file("/etc/resolv.conf", "/etc/resolv.conf.backup");
						
					start_wan_app();					
					
					set_start_wan_flag=1;				

					setDefaultGateway(1);
					
					#if defined(HAVE_FIREWALL)					
					set_ipfw_rules("eth1","eth0");
					#endif					
				}
			}
		#endif
			
			//diag_printf("%s:%d####DHCP_EVENT!\n",__FUNCTION__,__LINE__);
			
			if(wan_type==DHCP_CLIENT)
			{	
				kick_event(WAN_EVENT);				
			}
			else
			{				
				//dhcp+l2tp/pptp		
//				check_wan_ip();
		#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
				set_firewall_flag=1;
				kick_event(FIREWARE_EVENT);				

		#ifdef HAVE_DNSPROXY
				start_dnsproxy();
		#endif
				
		#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)				
					start_igmpproxy();				
		#endif
				
		#endif
				//set_dhcp_connect_flag(1);
			}			
		}
		else if(flag_val & WAN_EVENT)
		{
			//diag_printf("********WAN_EVENT ******\n");
			/*check ip conflicted
			  *
			  */
			  skip_wan_link_up=1;
			syslogAll_printk("Wan connected,check ip conflicted ...\n");
			check_wan_ip();
			/*Start Wan App
	  		 *  dns
	 		 *  ddns
	  		 *  igmproxy
	 		 *  ...
	 		 */
	 		#ifdef CONFIG_RTL_DHCP_PPPOE	
			apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
			#endif
			apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	 		if(wan_type==L2TP || wan_type==PPTP || (wan_type==PPPOE && pppoeWithDhcpEnabled))
				setDefaultGateway(0);
			syslogAll_printk("start wan applications ...\n");
			#if defined(HAVE_TR069)
#if defined(HAVE_SYSTEM_REINIT)
				if(set_start_wan_flag==1)
					kill_wan_app();
#endif

				if(!isFileExist("/etc/resolv.conf"))
					backup_file("/etc/resolv.conf.backup", "/etc/resolv.conf");
				else
					backup_file("/etc/resolv.conf", "/etc/resolv.conf.backup");

				start_wan_app();	
				
				set_start_wan_flag=1;
			#else
				start_wan_app();
			#endif
			
			set_firewall_flag=0;						
			
			kick_event(FIREWARE_EVENT);	
		}
		else if(flag_val & FIREWARE_EVENT)
		{
	#if defined(HAVE_FIREWALL)
			char wan_intf[MAX_NAME_LEN];
			char lan_intf[MAX_NAME_LEN];
			getInterfaces(lan_intf,wan_intf);			
			syslogAll_printk("set firewall ...\n");
	#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
			if(set_firewall_flag==1)
				strcpy(wan_intf, "eth1");
	#endif
			set_ipfw_rules(wan_intf,lan_intf);
	#endif

			//fix the issue:when LAN and WAN both set static ip, the WAN_EVENT 
			// and LAN_EVENT both are set, it cause the LAN_EVENT miss. 
			apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
			apmib_get(MIB_DHCP, (void *)&lan_type);
			if(wan_type==STATIC_IP && lan_type==DHCP_LAN_NONE)
				kick_event(LAN_EVENT); //set LAN_EVENT
		}
#ifdef HAVE_PPPOE		
		else if(flag_val & WAN_PPP_DISCONNECT_EVENT)
		{
			//diag_printf("%s:%d ###WAN_PPP_DISCONNECT_EVENT\n",__FUNCTION__,__LINE__);
			syslogAll_printk("Wan ppp disconnected...\n");
#ifdef HAVE_SYSTEM_REINIT

			if(get_skip_ppp_disconnect_flag() == 0) {
				ppp_disconnect_handle();
			} else
			{
				/*Do not hanle disconnect event once when in reinit process*/				
				clear_skip_ppp_disconnect_flag();
			}
#else
			ppp_disconnect_handle();
#endif
		}
		else if(flag_val & WAN_PPP_DIAL_EVENT)
		{	
			if(is_interface_up("ppp0"))
				interface_down("ppp0");		
			ppp_connect();
		}
		#if 0
		else if(flag_val & WAN_PPP_DEMAND_DIAL_EVENT)
		{	
			if(is_interface_up("ppp0"))
				interface_down("ppp0");	
			ppp_dail_on_demand();
		}
		#endif
		else if(flag_val & WAN_PPP_LCP_TERMINATE_EVENT)
		{
			
		//	diag_printf("%s:%d \n",__FUNCTION__,__LINE__);
		syslogAll_printk("Wan disconnected...\n");
#ifdef PPP_POWEROFF_DISCONNECT
			extern int save_connInfo(u_int16_t Session_ID,u_char*ether_servMac);
			unsigned char serverMac[6]={0};
			save_connInfo(0,serverMac);
#endif
		}
#endif		
		else if(flag_val & INDICATE_WISP_STATUS_EVENT)
		{
	#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)
			indicate_wisp_status();
	#endif
		}
		
#endif
		else if(flag_val & LAN_EVENT)
		{
//		set_event_flag(&lan_event_flag, 0);
		apmib_get(MIB_DHCP, (void *)&lan_type);	
#ifdef CONFIG_RTL_PHY_POWER_CTRL					
			extern void powerOnOffLanInf();
			extern void powerOnOffWlanInf();
			/*power on or reinit all, init_all will be set to 1, don't down/up lan/wlan interface twice.*/
			if(lan_type==DHCP_LAN_SERVER && init_all==0)
			{
				powerOnOffLanInf();
				powerOnOffWlanInf();
			}
			if(init_all == 1)
				init_all = 0;
#endif
			start_lan_app();
#ifdef HAVE_SYSTEM_REINIT
#ifdef HAVE_IWCONTROL
			iw_reinit = 1;
#endif
#endif
			start_wlan_app();
#ifdef HAVE_SYSTEM_REINIT
#ifdef HAVE_IWCONTROL
			iw_reinit = 0;
#endif
#endif
            #if defined(HAVE_FIREWALL)
            /* fix set dut wan=pppoe,enable web server on wan,set lan=dhcp client,
                       wan PC can't access dut webpage issue */
            #ifdef 	HOME_GATEWAY
            apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	     #endif
            check_wan_connect(wan_type);
            #endif
#ifdef HAVE_SIMPLE_CONFIG
			SC_IP_STATUS = 2;
#endif
#ifdef CONFIG_ECOS_AP_SUPPORT
#ifdef HAVE_NTP
			if(lan_type==DHCP_LAN_CLIENT)
				start_ntp();
#endif
#endif
		}
#ifdef  HOME_GATEWAY
#ifdef ECOS_RTL_REPORT_LINK_STATUS
		else if(flag_val & WAN_LINK_DOWN_EVENT){
			/*dhcp*/
			//diag_printf("********WAN LINK DOWN ******\n");			
#if 0 //def HAVE_L2TP
			if(wan_type==L2TP && is_interface_up("ppp0"))
			{
				l2tp_disconnect_flag=1;
				//diag_printf("%s:%d ### L2TP DISCONNECT!\n",__FUNCTION__,__LINE__);
				ppp_disconnect_handle();
			}			
#endif
#if 0
			/* if exist ppp0 device ,down  ppp0 device!!!! */
			if(is_interface_up("ppp0")){	
				ppp_disconnect_flag=1;
				ppp_disconnect();
				//interface_down("ppp0");
			}
#endif
#ifdef CYGPKG_PPP	
			if((wan_type==L2TP || wan_type==PPTP || wan_type==PPPOE))
			{
				if(is_interface_up("ppp0"))
				{
					cyg_ppp_down(NULL);
				}
			}	
#endif

			#ifdef HAVE_DHCPC
			apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
			if(!(wan_type==L2TP || wan_type==PPTP))
			dhcpc_set_wan_status(1);
			#endif		
		}
		else if(flag_val & WAN_LINK_UP_EVENT ){
			if(skip_wan_link_up==1)
			{
				skip_wan_link_up=0;
				//printf("\n%s:%d skip wan link up event!!!\n",__FUNCTION__,__LINE__);
				continue ;
			}
			apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
			/*dhcp*/
			//diag_printf("********WAN LINK UP ******\n");
			#ifdef HAVE_DHCPC
			if(!(wan_type==L2TP || wan_type==PPTP))
			dhcpc_set_wan_status(2);
			#endif	
#if 0
			if((wan_type==L2TP || wan_type==PPTP || wan_type==PPPOE) && ppp_disconnect_flag){
				//interface_up("ppp0");
				ppp_disconnect_flag=0;
				ppp_disconnect_handle();
			}
#endif
#ifdef CYGPKG_PPP	
			if((wan_type==L2TP || wan_type==PPTP || wan_type==PPPOE))
			{
				if(is_interface_up("ppp0"))
				{
					cyg_ppp_down(NULL);
				}
			}	
#endif

#if 0 //def HAVE_L2TP
			if(wan_type==L2TP && l2tp_disconnect_flag==1)
			{
				sleep(10);
				l2tp_disconnect_flag=0;
				l2tp_connect();
			}
#endif
#if defined(CONFIG_APP_TR069) && defined (CUSTOMIZE_MIDDLE_EAST)
	extern char tr069_enable_notify_wan_connect; 
	if(tr069_enable_notify_wan_connect){
		// send signal to cwmp process to let it know that mib changed
		extern int wan_cable_connect_event;
		wan_cable_connect_event = 1;
		cwmp_handle_notify();
	}
#endif
		}
#endif
#endif
#ifdef SYSLOG_SUPPORT
		else if(flag_val & SYS_LOG_EVENT){
			char *log_buf = NULL;
			if((log_buf = malloc(MAX_LOG_BUF)) != NULL){
				memset(log_buf,0,MAX_LOG_BUF);
				do_syslog(SYS_LOG_READ,log_buf,MAX_LOG_BUF);
				syslog_printk(log_buf);
				free(log_buf);
			}
		}
#endif
#if defined(CONFIG_8881A_UPGRADE_KIT)
		else if(flag_val & AUTODHCP_EVENT){
			int lan_type = DHCP_LAN_SERVER;
			apmib_set(MIB_DHCP,(void*)&lan_type);
			//dhcpc_kill();
			kill_landhcpc();
			configure_lan();
			#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105
			start_lan();
			#endif
			dhcpd_force_restart();
		}
#endif
#if defined( HAVE_L2TP) || defined(HAVE_PPTP) || (defined(HAVE_PPPOE) && defined(CONFIG_RTL_DHCP_PPPOE))
		else if(flag_val & START_DHCP_PPP_EVENT)
		{	
			//diag_printf("%s:%d ##START_DHCP_PPP_EVENT\n",__FUNCTION__,__LINE__);
			start_dhcp_ppp();			
		}
#endif
		else if(flag_val & BRIDGE_WLAN_CONNECT)
		{	
			diag_printf("%s:%d ##BRIDGE_WLAN_CONNECT\n",__FUNCTION__,__LINE__);

#ifdef HAVE_DHCPC
			apmib_get(MIB_OP_MODE, (void *)&op_mode);			
			if(op_mode==BRIDGE_MODE || op_mode==WISP_MODE)	
				dhcpc_reconnect(1);
#endif

			#ifdef DHCP_AUTO_SUPPORT
			extern void set_waitIPCount_value(int value);
			//extern int get_waitIPCount_value();
			//#define WAIT_IP_COUNT_MAX 15
			
			//if(get_waitIPCount_value()>WAIT_IP_COUNT_MAX)
			set_waitIPCount_value(0);
			#endif
		}
		else {
			printf("invalid sys_flag value: 0x%x\n", flag_val);			
		}
	}
}

void gpio_init(void)
{
#ifndef HAVE_NOGPIO
#ifndef CONFIG_RTL_8196CS //for 96cs, gpio init should have done at wlan driver
#ifndef HAVE_RLX_PROFILING
	rtl_gpio_init();
#endif
#endif
#endif
}
void general_mib_check()
{
	unsigned char *ptr;
	//See if flash data is valid
    diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	ptr = apmib_hwconf();
	if (ptr) {
		free(ptr);
	}
	else {
		printf("HW configuration invalid, reset default!\n");
		RunSystemCmd(NULL_FILE, "flash", "default", NULL_STR);
	}

	ptr = apmib_dsconf();
	if (ptr) {
		free(ptr);
	}
	else {
		printf("Default configuration invalid, reset default!\n");
		RunSystemCmd(NULL_FILE, "flash", "default-sw", NULL_STR);
	}
	
	ptr = apmib_csconf();
	if (ptr) {
		free(ptr);
	}
	else {
		printf("Current configuration invalid, reset to default configuration!\n");
		RunSystemCmd(NULL_FILE, "flash", "reset", NULL_STR);
	}

}
void general_mib_init()
{
	if (!apmib_init()) {
		printf("apmib_init() failed!\n");
		return ;
	}

	validate_hw_setings();
}

void general_mib_reinit()
{
	if (!apmib_reinit()) {
		printf("apmib_reinit() failed!\n");
		return ;
	}
	validate_hw_setings();
}

void configure_nic()
{
	set_mac_addr();
#ifdef HOME_GATEWAY
	set_wan_netif_mtu();
#endif
}
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
extern void set_vlanInfo(void);
void configure_vlan()
{
	int i;
	char interFaceName[16]={0};
	
	disable_allVlanInterface();
	enable_vlanInterface("lan0");
	enable_vlanInterface("lan1");
	enable_vlanInterface("lan2");
	enable_vlanInterface("lan3");
	enable_vlanInterface("wan");
	enable_vlanInterface("wlan0");

#if defined(CONFIG_RTL_VAP_SUPPORT)
	for(i=0; i<MSSID_INUSE_NUM; i++)
	{
		#if defined(CONFIG_RTL_92D_SUPPORT)|| defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)	/*For dual band*/
		if(rtl_checkWlanVAEnable(0, i)==0)
		#else
		if(rtl_checkWlanVAEnable(i)==0)
		#endif
		{
			sprintf(interFaceName, "wlan0-va%d", i);
			enable_vlanInterface(interFaceName);
		}
	}
#endif

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	/*For dual band*/
	enable_vlanInterface("wlan1");
#if defined(CONFIG_RTL_VAP_SUPPORT)
	for(i=0; i<MSSID_INUSE_NUM; i++)
	{
		//#if defined(CONFIG_RTL_92D_SUPPORT)	/*For dual band*/
		if(rtl_checkWlanVAEnable(1, i)==0)
		//#else
		//if(rtl_checkWlanVAEnable(i)==0)
		//#endif
		{
			sprintf(interFaceName, "wlan1-va%d", i);
			enable_vlanInterface(interFaceName);
		}
	}
#endif
#endif
	checkIfPvidVlanBind();
	set_vlanInfo();
}
#endif
void configure_opmode()
{
	int op_mode;
#ifndef HAVE_NOETH
	// 1:bridge/WISP, 2:gateway
	// By default, DUT operates as a pure AP, not a gateway.
	// So we add wan port to eth0 vlan, VID[9].
	// As a result, there is no member port in eth1 vlan, VID[8].
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	if(op_mode == GATEWAY_MODE)
	{
		SoftNAT_OP_Mode(2);
	#if defined(CONFIG_RTL_DNS_TRAP)
		enable_dnstrap(0);
	#endif
	}
	else if((op_mode == BRIDGE_MODE) || (op_mode == WISP_MODE))
	{
	#if defined(CONFIG_RTL_DNS_TRAP)
		if (op_mode == BRIDGE_MODE) {
			SoftNAT_OP_Mode(1);
			enable_dnstrap(1);
		}
		else
	#endif
		SoftNAT_OP_Mode(1);
	}
#if defined(CONFIG_RTL_LAYERED_DRIVER_L3)
	/*Add this for setting eth0 ip to asic l3 table, because SoftNAT_OP_Mode will flush all asic tables*/
	reconfigure_lan();	
#endif

#endif

}

void inet_forwaring()
{
#if defined(CYGPKG_NET_OPENBSD_STACK)
	ipforwarding = 1;
#elif defined(CYGPKG_NET_FREEBSD_STACK)
	cyg_ipforwarding = 1;
#endif
}


void configure_bridge()
{
#ifdef HAVE_BRIDGE
	bridge_main_fn();
#endif

#ifdef DHCP_AUTO_SUPPORT
	if(isFileExist(DHCP_AUTO_CHECK_GETIP_FILE))
		unlink(DHCP_AUTO_CHECK_GETIP_FILE);
	
	extern void set_waitIPCount_value(int value);
	set_waitIPCount_value(0);
#endif

	configure_lan();
}
#ifdef HAVE_HTTPD
#ifdef ECOS_DBG_STAT
int dbg_athttpd_index=0;
#endif

void start_httpd()
{
#ifndef JFFS2_SUPPORT
	if (read_flash_webpage("/web", "") < 0)
		printf("fail to load web pages\n");
#endif
	diag_printf("Creating httpd Thread...\n");
#ifdef ECOS_DBG_STAT
	dbg_athttpd_index=dbg_stat_register("athttpd");
#endif

	cyg_httpd_start();

	save_cs_to_file();

}
#endif
#ifdef HAVE_MINIIGD
#ifdef 	ECOS_DBG_STAT
int dbg_igd_index = 0;
int dbg_igd_index1 = 0;
int dbg_igd_index2 = 0;

#endif


void start_miniigd()
{
	int upnp_enable=0;
	apmib_get(MIB_UPNP_ENABLED, (void *)&upnp_enable);
	if (upnp_enable) {
		create_miniigd();
	}
	
}
#endif

#ifdef HAVE_NBSERVER
void start_nbserver()
{
	unsigned char netbios_name[MAX_NAME_LEN];
	apmib_get(MIB_NETBIOS_NAME,(void*)&netbios_name); 
	if(netbios_name[0])
		RunSystemCmd(NULL_FILE, "nmbserver","-i","eth0","-n",netbios_name,"-D",NULL_STR);
}
#endif

#ifdef HAVE_TR069
extern int startCWMP(unsigned char urlChanged);
void start_tr069(void)
{
	int cwmp_flag_in_mib=0;

	int wan_type = 0;
	int pppoeWithDhcpEnabled = 0;
	mib_get(MIB_WAN_DHCP, (void *)&wan_type);
#ifdef CONFIG_RTL_DHCP_PPPOE	
	mib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
#endif
	if(wan_type==PPPOE && pppoeWithDhcpEnabled)
		addRouteForACS();

	mib_get(MIB_CWMP_FLAG, (void *)&cwmp_flag_in_mib);
	if(cwmp_flag_in_mib & CWMP_FLAG_AUTORUN)	
		startCWMP(1);
	return;
}
#endif
void start_dhcpd_daemon()
{
	set_dhcpd();
}

#ifdef HAVE_RADVD
static void start_radvd( void )
{
	extern int getRadvdInfo(radvdCfgParam_t *entry);
	extern int create_RadvdCfgFile(radvdCfgParam_t *radvdcfg);
//	extern int radvd_main(int argc, char *argv[]);
	extern void create_radvd(void);
	
	radvdCfgParam_t entry;
	
	// generate config file for radvd 
	getRadvdInfo( &entry );
	create_RadvdCfgFile( &entry );
	
	// start radvd - pthread 
#ifdef HAVE_SYSTEM_REINIT
	if(entry.enabled == 1)
#endif
	{
		create_radvd();
		//radvd_main( 0, NULL );
	}
}
#endif

#ifdef HAVE_DHCP6S
static void start_dhcp6s( void )
{
	//extern int getRadvdInfo(radvdCfgParam_t *entry);
	//extern int create_RadvdCfgFile(radvdCfgParam_t *radvdcfg);
	//extern int radvd_main(int argc, char *argv[]);
	//
	//radvdCfgParam_t entry;
	//
	//// generate config file for radvd 
	//getRadvdInfo( &entry );
	//create_RadvdCfgFile( &entry );
	//
	//// start radvd - pthread 
	
	extern int getDhcpv6sInfo(dhcp6sCfgParam_t *entry);
	extern int dhcp6s_main(int argc, char *argv[]);
	extern int create_Dhcp6CfgFile(dhcp6sCfgParam_t *dhcp6sCfg);

	dhcp6sCfgParam_t entry;
	getDhcpv6sInfo(&entry);

	if(entry.enabled == 1)
		create_Dhcp6CfgFile(&entry);
	
	char *s0 = "dhcp6s";
	char *s1 = "eth0";
	char *s2 = NULL;
	if(entry.enabled == 1)
		s2 = "enable";
	else
		s2 = "disable";
	char *argv[] = {s0, s1, s2};
	dhcp6s_main( 3, argv );
}
#endif

void set_lan_static_ip(int op_mode)
{
	int ret1, ret2;
	struct in_addr addr, mask, defway;
	char addrstr[16], maskstr[16], defwaystr[16];
	
	ret1=getInAddr("eth0", IP_ADDR, &addr);
	ret2=getInAddr("eth0", SUBNET_MASK, &mask);
	if(!(ret1>0 && ret2>0 && addr.s_addr>0 && mask.s_addr>0))
	{
		apmib_get(MIB_IP_ADDR,  (void *)&addr);
		apmib_get(MIB_SUBNET_MASK,  (void *)&mask);		
	}	
	inet_ntoa_r(addr, addrstr);
	inet_ntoa_r(mask, maskstr);	
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", addrstr,"netmask" ,maskstr, NULL_STR);

//	if(op_mode==BRIDGE_MODE)
	{
		apmib_get(MIB_DEFAULT_GATEWAY,  (void *)&defway);
	
		if(defway.s_addr>0)
		{		
			inet_ntoa_r(defway, defwaystr);
			RunSystemCmd(NULL_FILE, "route", "add", "-gateway" ,defwaystr, "-interface", "eth0", NULL_STR);
		}
//		start_dnsproxy();
	}
	kick_event(LAN_EVENT); //set LAN_EVENT
}
#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105
void start_lan()
{
#ifdef HAVE_HTTPD
		start_httpd();
#endif
	char lan_interface[16], gwbuf[16];
	int lan_type, dns_mode, op_mode;
	unsigned char domain_name[MAX_NAME_LEN];	
	struct in_addr gateway;
	apmib_get(MIB_DHCP,(void*)&lan_type);
	apmib_get(MIB_OP_MODE,(void*)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif

#ifdef HAVE_DHCPD
	if(lan_type==DHCP_LAN_SERVER)
	{		
		apmib_get(MIB_DOMAIN_NAME,(void*)&domain_name);		
		if(domain_name[0])
			write_line_to_file("/etc/domain_name", 1, domain_name);
		
		start_dhcpd_daemon();	
#ifdef HAVE_SIMPLE_CONFIG
		SC_IP_STATUS = 1;
#endif
	}
#endif

#ifdef HAVE_DHCPC
	if(lan_type==DHCP_LAN_CLIENT)
	{
		apmib_get(MIB_DNS_MODE, (void *)&dns_mode);
		sprintf(lan_interface, "%s", "eth0");
		landhcpc_startup(lan_interface, dns_mode);
#ifdef HAVE_SIMPLE_CONFIG
		SC_IP_STATUS = 0;
#endif
	}
#endif

	if(lan_type==DHCP_LAN_NONE)
	{
#if 0
		apmib_get(MIB_DEFAULT_GATEWAY, (void *)&gateway);
		if(gateway.s_addr!=0)
		{			
			inet_ntoa_r(gateway, gwbuf);
			RunSystemCmd(NULL_FILE, "route", "add", "-gateway", gwbuf, NULL_STR);	
		}
#endif
		set_lan_static_ip(op_mode);
#ifdef HAVE_SIMPLE_CONFIG
		SC_IP_STATUS = 1;
#endif

	}

#ifdef DHCP_AUTO_SUPPORT
	if(lan_type==DHCP_AUTO)
		start_dhcpauto();
#endif	
	
#ifdef HAVE_PATHSEL
	start_pathsel();
#endif
}

#ifdef HAVE_TELNET_USERNAME_PASSWORD
void save_username_password_to_file()
{
	//deposit the user name and password in a file
	char userNameInDUT[MAX_NAME_LEN+2]={0}; //deposit router's user name
	char passwordInDUT[MAX_NAME_LEN+2]={0}; //deposit router's password
	int userNameLength=0;
	int passwordLength=0;
	if(!apmib_get(MIB_USER_NAME, (void *)userNameInDUT))
	{		
		printf("Get router's user name failed!\n");
		exit(-1);
	}
	if(!apmib_get(MIB_USER_PASSWORD	, (void *)passwordInDUT))
	{		
		printf("Get router's password failed!\n");
		exit(-1);
	}
	userNameLength=strlen(userNameInDUT);
	passwordLength=strlen(passwordInDUT);
	userNameInDUT[userNameLength]='\n';
	passwordInDUT[passwordLength]='\n';
	write_line_to_file(TELNET_USERNAME_PASSWORD_FILE, 1, userNameInDUT);
	write_line_to_file(TELNET_USERNAME_PASSWORD_FILE, 2, passwordInDUT);
}
#endif
#endif
void start_lan_app()
{
#ifdef HAVE_MINIUPNP
	start_miniupnp();
#endif
#ifdef HAVE_TELNETD
#ifdef HAVE_TELNET_USERNAME_PASSWORD
	save_username_password_to_file();
#endif
	RunSystemCmd(NULL_FILE, "telnetd",NULL_STR);
#endif
#ifdef HAVE_NBSERVER
	start_nbserver();
#endif

#ifdef HAVE_RADVD
	start_radvd();
#endif
#ifdef HAVE_DHCP6S
	start_dhcp6s();
#endif

}

#ifdef HAVE_MP_DAEMON
void start_mp()
{
	int sock;
#ifdef SYS_INIT_USING_MBOX
	unsigned int flag_val, *puint;
#else
	cyg_flag_value_t flag_val;
#endif
	char brdg[] = "bridge0";
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	char tmpbuff[16];
	//assert(sock >= 0);
	tmpbuff[0] = 0x56;
	tmpbuff[1] = 0xaa;
	tmpbuff[2] = 0xa5;
	tmpbuff[3] = 0x5a;
	tmpbuff[4] = 0x7d;
	tmpbuff[5] = 0xe8;
	//RunSystemCmd(NULL_FILE, "flash", "default-sw", NULL_STR);
	set_mac_address("eth0", (void *)tmpbuff);
	//set_mac_address("eth1", (void *)tmpbuff);
	set_mac_address("wlan0", "\x00\xe0\x4c\x81\x96\xc9");
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	set_mac_address("wlan1", "\x00\xe0\x4c\x81\x96\xd9");
#endif
	//BRIDGE_IF_NAME is created when kernel init, so we don't need to addbr
	
	interface_up("eth0");
	bridge_add(sock,brdg,"eth0");

	sleep(1);


	//bridge_ifsetflag (sock, brdg, "eth0", IFBIF_STP);
	//interface_up("eth1");
	
	//bridge_add(sock,brdg,"eth1");

	//bridge_ifsetflag (sock, brdg, "eth1", IFBIF_STP);
	RunSystemCmd(NULL_FILE, "flash", "set_mib", "wlan0", NULL_STR);
	interface_up("wlan0");
	bridge_add(sock,brdg,"wlan0");
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	RunSystemCmd(NULL_FILE, "flash", "set_mib", "wlan1", NULL_STR);

	interface_up("wlan1");
	bridge_add(sock,brdg,"wlan1");
#endif
	RunSystemCmd(NULL_FILE, "ifconfig", "eth0", "192.168.1.6","netmask" ,"255.255.255.0", NULL_STR);
	RunSystemCmd(NULL_FILE,"iwpriv", "wlan0", "set_mib", "mp_specific=1",NULL_STR);
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	RunSystemCmd(NULL_FILE,"iwpriv", "wlan1", "set_mib", "mp_specific=1",NULL_STR);
#endif
	bridge_status(sock,brdg);
	bridge_setflag(sock,brdg, IFF_UP);
	close(sock);
	SoftNAT_OP_Mode(1);
	create_mp_daemon();
	while(1){
#ifdef SYS_INIT_USING_MBOX	
		puint=(unsigned int *)cyg_mbox_get(sys_mbox_hdl);
		if(puint) {
			flag_val=*puint;
			/*free the memory*/
			free(puint);
		}	
		else
			continue;
#else
		flag_val = cyg_flag_wait(&sys_flag, 0xffffffff, 
			CYG_FLAG_WAITMODE_OR | CYG_FLAG_WAITMODE_CLR);
#endif
		//diag_printf("sys_flag=%x\n", flag_val);
		if (flag_val & RESET_EVENT) {
		
			do_reset(1);
		}
		else if (flag_val & RELOAD_DEFAULT_EVENT) {
			do_reset(1);
		}
	}
	

}

#endif
#ifdef HAVE_IWCONTROL
typedef struct _Dot1x_RTLDListener
{
#ifdef __ECOS
	cyg_handle_t mbox_hdl;
#else
	int	WriteFIFO;
#endif
	int	Iffd;
	char	wlanName[16];

}Dot1x_RTLDListener;

#define MAX_WLAN_INTF	10
#define READ_BUF_SIZE	50
#define WLAN_INTF_LEN 	16
#endif

#ifdef ECOS_DBG_STAT
int dbg_wsc_index=0;
#endif

void start_wps()
{
	int wlan_disable=0, wps_disable=0,have_wsc = 0,vxd_disabled = 0, i=0;
	static int inited = 0;
	cyg_thread_info tinfo;
#ifdef HAVE_IWCONTROL
	extern Dot1x_RTLDListener RTLDListenerWscd[MAX_WLAN_INTF];
	extern int link_wscd;
	extern int iw_wlan_num  ;
	extern char iw_wlan_tbl[MAX_WLAN_INTF][20];
#endif
#ifdef HAVE_AUTH
	int auth_enable;
#endif
#ifdef HAVE_IWCONTROL
	if(!inited 
#ifdef HAVE_SYSTEM_REINIT
		|| iw_reinit
#endif
	){
		link_wscd = 0;
	}
#endif
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {

#ifdef HAVE_AUTH
		auth_enable=0;
		apmib_get_ext(MIB_WLAN_ENABLE_1X,(void *)&auth_enable,i,0);
#endif
		apmib_get_ext(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable,i,0);
#ifdef HAVE_AUTH
		if(!auth_enable)
#endif
		{
			if (!wlan_disable) {
				apmib_get_ext(MIB_WLAN_WSC_DISABLE, (void *)&wps_disable,i,0);
					if (!wps_disable) {
#ifdef HAVE_IWCONTROL
					if(!inited 
#ifdef HAVE_SYSTEM_REINIT
						|| iw_reinit
#endif
					){
						sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d",i);
						iw_wlan_num++;
						sprintf(RTLDListenerWscd[link_wscd].wlanName,"wlan%d",i);
						link_wscd++;
						apmib_get_ext(MIB_WLAN_WLAN_DISABLED, (void *)&vxd_disabled,i,NUM_VWLAN_INTERFACE);
						if(!vxd_disabled){
							sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d-vxd0",i);
							iw_wlan_num++;
							sprintf(RTLDListenerWscd[link_wscd].wlanName,"wlan%d-vxd0",i);
							link_wscd++;
						}
					}
#endif				
					have_wsc = 1;

				}
			}
		}
	}
	inited = 1;
	if (get_thread_info_by_name("wscd", &tinfo)) {
		if (have_wsc)
			wsc_reinit(); //trigger wsc to reinit
		else
			wsc_stop_service(); //trigger wsc to stop service
	}
	else{
		if(have_wsc)
		{
#ifdef ECOS_DBG_STAT
			dbg_wsc_index=dbg_stat_register("wsc");
#endif
			create_wscd();
		}
	}
	
}

#ifdef HAVE_PATHSEL
void start_pathsel()
{
#ifdef CONFIG_RTK_MESH
	int mesh_enabled,wlan_mode;
	int i;
	for (i=0; i<NUM_WLAN_INTERFACE; i++)
	{
		apmib_get_ext( MIB_WLAN_MESH_ENABLE, (void *)&mesh_enabled,i,0);
		apmib_get_ext(MIB_WLAN_MODE, (void *)&wlan_mode,i,0);
		if((wlan_mode ==  AP_MESH_MODE|| wlan_mode == MESH_MODE) && mesh_enabled)
		{
			create_pathsel();
			return;
		}
	}
#endif

}
#endif


#ifdef HAVE_IAPP

void start_iapp()
{
	extern int link_iapp;
	extern int iw_wlan_num;
	extern char iw_wlan_tbl[MAX_WLAN_INTF][20];
	int i;
	char parms[150];
	char wlan_interface[16];
#ifdef CONFIG_RTL_VAP_SUPPORT
	int j;
#endif
	link_iapp = 0;

	int wlan_disable = 0,iapp_disable = 0;
#ifdef HAVE_AUTH
	int auth_enable = 0;
#endif
#ifdef HAVE_WPS
	int wps_disable = 0;
#endif
	memset(parms,0,sizeof(parms));
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		apmib_get_ext(MIB_WLAN_WLAN_DISABLED, (void *)&wlan_disable,i,0);
		if(!wlan_disable){
			apmib_get_ext( MIB_WLAN_IAPP_DISABLED, (void *)&iapp_disable,i,0);
#ifdef HAVE_WPS
			apmib_get_ext( MIB_WLAN_WSC_DISABLE, (void *)&wps_disable,i,0);
#endif
#ifdef HAVE_AUTH
			apmib_get_ext( MIB_WLAN_ENABLE_1X, (void *)&auth_enable,i,0);
#endif
			if(!iapp_disable){
				if(parms[0] == 0){
					sprintf(parms,"eth0");
				}
				if(
#ifdef HAVE_AUTH
					(!auth_enable)
#else
					(!iapp_disable)
#endif
					&&
#ifdef HAVE_WPS
					wps_disable
#else
					(!iapp_disable)
#endif
				)
				{
					sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d",i);
					iw_wlan_num++;
				}
					sprintf(wlan_interface," wlan%d",i);
					strcat(parms,wlan_interface);
					link_iapp ++;

			
#ifdef CONFIG_RTL_VAP_SUPPORT
				for (j=0; j<RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM; j++){
					apmib_get_ext(MIB_WLAN_WLAN_DISABLED,&wlan_disable,i,j+1);
					if(wlan_disable)
						continue;
#ifdef HAVE_AUTH
					apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enable,i,j+1);
					if(!auth_enable)
#endif
					{	
						sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d-va%d",i,j);
						iw_wlan_num++;
						
					}
					sprintf(wlan_interface," wlan%d-va%d",i,j);
					strcat(parms,wlan_interface);
					link_iapp++;

				}
#endif
			
			}
		}

	}
	if(parms[0]!= 0){
		extern void creat_iapp(char *arg);
		creat_iapp(parms);
	}

}
#endif
#ifdef HAVE_AUTH

void start_auth()
{
	int auth_enable = 0;
	int wlan_disabled = 0;
	int i,j;
	char arg[300]={0};
	char buf[30];
	

	extern Dot1x_RTLDListener RTLDListenerAuth[MAX_WLAN_INTF];
	extern int link_auth ;
	extern int iw_wlan_num  ;
	link_auth = 0;
	extern char iw_wlan_tbl[MAX_WLAN_INTF][20];
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		apmib_get_ext(MIB_WLAN_WLAN_DISABLED,&wlan_disabled,i,0);
		if(wlan_disabled)
			continue;
		apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enable,i,0);
		if(auth_enable){	
			sprintf(buf,"/wlan%d eth0 auth ",i);
			if(arg[0]==0)
				sprintf(arg,"%s",buf);
			else
				strcat(arg,buf);
			sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d",i);
			iw_wlan_num++;
			sprintf(RTLDListenerAuth[link_auth].wlanName,"wlan%d",i);
			link_auth++;
		}
#ifdef CONFIG_RTL_VAP_SUPPORT
		for (j=0; j<RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM; j++){
			apmib_get_ext(MIB_WLAN_WLAN_DISABLED,&wlan_disabled,i,j+1);
			if(wlan_disabled)
				continue;
			apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enable,i,j+1);
			if(auth_enable){	
				sprintf(buf,"/wlan%d-va%d eth0 auth ",i,j);
				if(arg[0]==0)
					sprintf(arg,"%s",buf);
				else
					strcat(arg,buf);
				sprintf(iw_wlan_tbl[iw_wlan_num],"wlan%d-va%d",i,j);
				iw_wlan_num++;
				sprintf(RTLDListenerAuth[link_auth].wlanName,"wlan%d-va%d",i,j);
				link_auth++;
			}
		}
	}
#endif
	//apmib_get(MIB_WLAN_ENABLE_1X,(void *)&auth_enable);
	if(arg[0]!=0){
		extern void creat_auth(char *arg);
		creat_auth(arg);
	}
}
#endif
#ifdef HAVE_IWCONTROL
void start_iw()
{
	#ifdef ECOS_DBG_STAT
	dbg_iw_index = dbg_stat_register("iw");
	#endif
	extern void creat_iw();
	extern int iw_wlan_num;
	if(iw_wlan_num !=0)
	creat_iw();
}
#endif
#ifdef HAVE_HS2_SUPPORT
void init_hs2()
{
	int auth_enable = 0,hs2_enable = 0,wlan_encrypt = 0;
	int wlan_disabled = 0;
	int i,j,argc = 0;
	char *argv[NUM_WLAN_INTERFACE*RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM*2+3];
	extern Dot1x_RTLDListener RTLDListenerHS2[MAX_WLAN_INTF];
	extern int link_hs2 ;
	link_hs2 = 0;
	for(i = 0;i < NUM_WLAN_INTERFACE*RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM*2+3;i++)
		argv[i] = malloc(30);

	sprintf(argv[argc++],"hs2");
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		apmib_get_ext(MIB_WLAN_WLAN_DISABLED,&wlan_disabled,i,0);
		if(wlan_disabled)
			continue;
		apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enable,i,0);
		apmib_get_ext(MIB_WLAN_HS2_ENABLE,&hs2_enable,i,0);
		apmib_get_ext(MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt,i,0);
		if(auth_enable && hs2_enable &&  wlan_encrypt == 4 ){	
			
			sprintf(RTLDListenerHS2[link_hs2].wlanName,"wlan%d",i);
			link_hs2++;
				sprintf(argv[argc++],"-c");
				sprintf(argv[argc++],"/etc/hs2_wlan%d.conf",i);
				#if 0
				if(arg[0]==0)
					sprintf(arg,"hs2 -c /etc/hs2_wlan%d.conf",i);
				else{
					sprintf(arg," -c /etc/hs2_wlan%d.conf",i);
					strcat(arg,buf);
				}
				#endif
		}
#ifdef CONFIG_RTL_VAP_SUPPORT
		for (j=0; j<RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM; j++){
			apmib_get_ext(MIB_WLAN_WLAN_DISABLED,&wlan_disabled,i,j+1);
			if(wlan_disabled)
				continue;
			apmib_get_ext(MIB_WLAN_ENABLE_1X,&auth_enable,i,j+1);
			apmib_get_ext(MIB_WLAN_HS2_ENABLE,&hs2_enable,i,j+1);
			apmib_get_ext( MIB_WLAN_ENCRYPT, (void *)&wlan_encrypt,i,j+1);
			if(auth_enable && hs2_enable &&  wlan_encrypt == 4){
				sprintf(argv[argc++],"-c");
				sprintf(argv[argc++],"/etc/hs2_wlan%d-va%d",i,j);
				#if 0
				if(arg[0]==0)
					sprintf(arg,"hs2 -c /etc/hs2_wlan%d-va%d.conf",i,j);
				else{
					sprintf(arg," -c /etc/hs2_wlan%d-va%d.conf",i,j);
					strcat(arg,buf);
				}
				#endif
				sprintf(RTLDListenerHS2[link_hs2].wlanName,"wlan%d-va%d",i,j);
				link_hs2++;
			}
		}
	}
#endif
	//argv[argc++] = 0;

	if(argc > 2){
		hs2_start(argc,argv);
	}
	for(i = 0;i < NUM_WLAN_INTERFACE*RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM*2+3;i++)
		free(argv[i]);
}
#endif
#ifdef HAVE_SIMPLE_CONFIG
void start_simple_config()
{
	int i, intVal;
	int mode, disabled, sc_enabled;
	unsigned char sc_if[32];

	
	intVal = 0;
	for (i=0; i<NUM_WLAN_INTERFACE; i++) {
		
		apmib_get_ext(MIB_WLAN_SC_ENABLED, &sc_enabled, i, 0);
		apmib_get_ext(MIB_WLAN_MODE, &mode, i, 0);
		apmib_get_ext(MIB_WLAN_WLAN_DISABLED, &disabled, i, 0);
		if(disabled ==0)
		{
			if(mode == CLIENT_MODE)
			{
				if(sc_enabled == 1)
				{
					diag_printf("the wlan interface is wlan%d\n", i);
					sprintf(sc_if, "wlan%d", i);
					intVal++;
				}
			}
			else if (mode == AP_MODE)
			{
#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT
				apmib_get_ext(MIB_WLAN_SC_ENABLED, &sc_enabled, i, 5);
				apmib_get_ext(MIB_WLAN_MODE, &mode, i, 0);
				apmib_get_ext(MIB_WLAN_WLAN_DISABLED, &disabled, i, 5);
				if(sc_enabled==1 && mode==AP_MODE && disabled==0)
				{
					diag_printf("the wlan interface is wlan%d-vxd0\n", i);
					sprintf(sc_if, "wlan%d-vxd0", i);
					intVal++;
				}
#endif		
			}
		}
	}

	if(intVal>1)
	{
		diag_printf("More than one interface as RTL Simple Config Client now!\n");
		return -1;
	}
	else if(intVal<1)
	{
		diag_printf("No interface support RTL Simple Config Client now!\n");
		return -1;
	}
	else if(intVal == 1)
	{
		simple_config_start(sc_if);
	}
}
#endif

void start_wlan_app()
{
#ifdef HAVE_IWCONTROL
	extern int iw_wlan_num;
	iw_wlan_num = 0;
#endif
#ifdef HAVE_WPS
	start_wps();
#endif
#ifdef HAVE_WLAN_SCHEDULE
	start_wlanSchedule(1);
#endif
#ifdef HAVE_AUTH
	start_auth();
#endif

#ifdef HAVE_IAPP
	start_iapp();
#endif

#ifdef HAVE_HS2_SUPPORT
	init_hs2();
#endif

#ifdef HAVE_IWCONTROL
	start_iw();
#endif

#ifdef HAVE_SIMPLE_CONFIG
	start_simple_config();
#endif
}

#ifdef 	ECOS_DBG_STAT
int dbg_napt_index=0;
#endif
#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
void config_wan(unsigned int flag)
#else
void config_wan()
#endif
{
	int op_mode;
	RunSystemCmd(NULL_FILE, "ipfw", "add", "65534", "allow", "ip", "from", "any", "to", "any", NULL_STR);
	diag_printf("%s,%d, start config wan \n",__FUNCTION__,__LINE__);

#if defined(HAVE_TR069)
	set_start_wan_flag=0;
#endif

#ifdef HAVE_PPPOE
	/*pppoe init only should be called once*/
	pppoe_init(0);
#endif

#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	rtl_initNapt();
#if HAVE_NAT_ALG   //lynn enable Punch hole in firewall
#ifndef NO_FW_PUNCH
        char temp[32] = {0};
		extern void rtl_uninit_punchFW(void);
		rtl_uninit_punchFW();

        sprintf(temp, "%u:%u", IPFW_INDEX_ALG, IPFW_SECTOR_SIZE-1);
        extern void rtl_parseNaptOption(const char* option, const char* parms);
        rtl_parseNaptOption("punch_fw", temp);
#endif
#endif
#endif
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
	if((op_mode == GATEWAY_MODE) || (op_mode == WISP_MODE))
	{
		diag_printf("%s,%d, start config wan \n",__FUNCTION__,__LINE__);	

#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105		
#ifdef CONFIG_RTL_DHCP_PPPOE
		start_wan(flag);
#else
		start_wan();
#endif		
#endif

	}
}
#endif
#if defined(HAVE_DNSPROXY)

#ifdef ECOS_DBG_STAT
int dbg_dns_index=0;
extern int dbg_stat_add(int dbg_type, int module_type, int action_type, unsigned int size);
extern int dbg_stat_register(char *name);
#endif
void start_dnsproxy()
{
	int mode;
	unsigned char tmpbuf[32];
	struct in_addr dns1_addr, dns2_addr;
	apmib_get(MIB_DNS_MODE, (void *)&mode);
	
#ifdef	ECOS_DBG_STAT
	dbg_dns_index=dbg_stat_register("dns");
#endif
	if(mode==DNS_MANUAL)
	{
		apmib_get(MIB_DNS1, (void *)&dns1_addr);
		apmib_get(MIB_DNS2, (void *)&dns2_addr);
		if(dns1_addr.s_addr!=0)
		{
			sprintf(tmpbuf,"nameserver %s\n",inet_ntoa(dns1_addr));
			write_line_to_file("/etc/resolv.conf", 1, tmpbuf);
		}
		if(dns2_addr.s_addr!=0)
		{
			sprintf(tmpbuf,"nameserver %s\n",inet_ntoa(dns2_addr));
			if(dns1_addr.s_addr!=0)
				write_line_to_file("/etc/resolv.conf", 2, tmpbuf);	
			else
				write_line_to_file("/etc/resolv.conf", 1, tmpbuf);
		}
	}		
	
	if(!isFileExist("/etc/resolv.conf"))
		return;
//#ifndef ECOS_DOMAIN_NAME_QUERY_SUPPORT
//	cyg_dns_proxy_start();	
//#else
//	restart_dns_proxy(1);
//#endif
	if(get_dnsproxy_status())
		restart_dns_proxy(1);
	else
		cyg_dns_proxy_start();
}
#endif

#ifdef HAVE_RTLDD
int init_rtldd(void);
void start_rtldd()
{
	init_rtldd();
}
#endif

#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)	

extern void create_mrouted(char * wan_if, char * lan_if);
#elif defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)

extern int 	ip_mroute_kernel_mode_init(char * wanintfname,char * lanintfname );			
#endif
#ifdef 	ECOS_DBG_STAT
int dbg_igmpproxy_index=0;
#endif

void start_igmpproxy()
{

	char lanintfname[32],wanintfname[32];
	int disabled=0;
	//diag_printf("start_igmpproxy[%s]:[%d].\n",__FUNCTION__,__LINE__);
	if ( !apmib_get( MIB_IGMP_PROXY_DISABLED, (void *)&disabled) )
	{
		diag_printf("get MIB_IGMP_PROXY_DISABLED fail\n");
		return;
	}
	
	if(disabled){
		//diag_printf("IGMPPROXY DISABEL RETURN[%s]:[%d].\n",__FUNCTION__,__LINE__);
		return;
	}	
	
	RunSystemCmd(NULL_FILE, "route", "add", "-net", "224.0.0.0","-gateway", "192.168.1.0", "-netmask","240.0.0.0", "-interface", "eth0", NULL_STR);
	//route add -net 224.0.0.0 -gateway 192.168.1.0 -netmask 240.0.0.0 -interface eth0 
	getInterfaces(lanintfname,wanintfname);
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
	strcpy(wanintfname,"eth1");
#endif
	
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)	
	//diag_printf("start_igmpproxy[%s]:[%d].\n",__FUNCTION__,__LINE__);
	create_mrouted(wanintfname,"eth0");

#elif defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)

	ip_mroute_kernel_mode_init(wanintfname,lanintfname );	

#endif
}

#if defined(ROUTE_SUPPORT)
extern void addStaticRoute(STATICROUTE_Tp pEntry);
extern int  run_clicmd(char *command_buf);
//extern unsigned int sleep( unsigned int seconds );
void init_static_route(void)
{
	int enable=0,entryNum=0,i=0;
	STATICROUTE_T entry={{0}};	

	if(!apmib_get(MIB_STATICROUTE_ENABLED, (void *)&enable))
		return;
	
	if(enable)
	{
		if(!apmib_get(MIB_STATICROUTE_TBL_NUM,(void *)&entryNum))
			return;
		for(i=1;i<=entryNum;i++)
		{
			*((char *)&entry) = (char)i;
			if(!apmib_get(MIB_STATICROUTE_TBL, (void *)&entry))
				return;	
			#ifdef KLD_ENABLED
			if(entry.enabled)
			#endif	
			addStaticRoute(&entry);
		}
	}
}

void init_rip_route(void)
{
	int ret;
	int dynamic_route_enable=0, nat_enable=0, riplan_enable=0;
	char shellcmd[32]; //it's safe to store shell command to a middle variable
	
	ret = apmib_get(MIB_DYNAMICROUTE_ENABLED, (void*)&dynamic_route_enable);
	//diag_printf("dynamic_route_enable = %d\n", dynamic_route_enable);
	if(!ret)
		return;

	ret = apmib_get(MIB_NAT_ENABLED, (void*)&nat_enable);
	//diag_printf("nat_enable = %d\n", nat_enable);
	if(!ret)
		return;

	ret = apmib_get(MIB_RIP_ENABLED, (void*)&riplan_enable);
	//diag_printf("riplan_enable = %d\n", riplan_enable);
	if(!ret)
		return;

	if(dynamic_route_enable && !nat_enable && riplan_enable) {
		//run_clicmd("route add -net 224.0.0.0 -gateway 192.168.1.0 "
		//       "-netmask 240.0.0.0 -metric 1 -interface eth0"); //set multicast route
		RunSystemCmd(NULL_FILE, "route", "add", "-net", "224.0.0.0","-gateway", \
				"192.168.1.0", "-netmask","240.0.0.0", "-interface", "eth0", NULL_STR);
		sleep(1);
		memset(shellcmd, 0, sizeof(shellcmd));
		if(riplan_enable == 1) {
			//cyg_routed_start();
			strcpy(shellcmd, "routed -s");
			run_clicmd(shellcmd);
			//RunSystemCmd(NULL_FILE, "routed", "-s", NULL_STR);
		} else if(riplan_enable == 2) {
			strcpy(shellcmd, "routed -s -P ripv2");
			run_clicmd(shellcmd);
			//RunSystemCmd(NULL_FILE, "routed", "-s", "-P", "ripv2", NULL_STR);
		}
	}
}

#if defined(ROUTE6D_SUPPORT)
void init_rip6_route(void)
{
	int ret;
	int dynamic_route_enable=0, nat_enable=0, rip6_enable=0;
	
	sleep(5);
	ret = apmib_get(MIB_DYNAMICROUTE_ENABLED, (void*)&dynamic_route_enable);
	//printf("dynamic_route_enable = %d\n", dynamic_route_enable);
	if(!ret)
		return;

	ret = apmib_get(MIB_NAT_ENABLED, (void*)&nat_enable);
	//printf("nat_enable = %d\n", nat_enable);
	if(!ret)
		return;

	ret = apmib_get(MIB_RIP6_ENABLED, (void*)&rip6_enable);
	//printf("rip6_enable = %d\n", rip6_enable);
	if(!ret)
		return;

	if(dynamic_route_enable && !nat_enable && rip6_enable) {
		//cyg_route6d_start();
		run_clicmd("route6d -N wlan0");
	}
}
#endif
#endif

#ifdef HAVE_NTP
void start_ntp()
{	
//	extern void init_sntpd();
	//extern int start_ntpclient();
	int enabled=0;
	if(!apmib_get(MIB_NTP_ENABLED,(void*)&enabled))
	{
		diag_printf("get MIB_NTP_ENABLED fail!\n");
		return;
	}
	if(enabled)
	{
		start_ntpclient();
//		init_sntpd();
	}
}
#endif

#ifdef CYGPKG_NS_DNS
int  get_dns_from_resolv_file(struct in_addr *pdns_ip)
{
	int i;
	FILE *fp=NULL;
	unsigned char dns_buf[32];
		
	fp=fopen("/etc/resolv.conf","r");	

	if(fp!=NULL)
	{
		unsigned char *pchar=NULL;
		while(fgets(dns_buf, sizeof(dns_buf), fp))
		{	
			for(pchar=dns_buf; *pchar!='\n' && *pchar!='\0'; pchar++) ;
			*pchar='\0';
			if((pchar=strchr(dns_buf, ' '))!=NULL)
			{
				pchar++;
				inet_aton(pchar, pdns_ip);
				break;			
			}
		}
		fclose(fp);
	}
	else
		return -1;	
	return 0;
}

void dns_res_start()
{
	/*Get dns from resolv.conf*/
	struct in_addr ip;
	get_dns_from_resolv_file(&ip);
	cyg_dns_res_start(inet_ntoa(ip));
}
#endif


#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
extern int rtl_setAliasAddrByInfName(char *name);
void start_napt(void)
{
	int wan_type=0;
	char lan_intf[MAX_NAME_LEN];
	char wan_intf[MAX_NAME_LEN];
	getInterfaces(lan_intf,wan_intf);	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
    
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == DHCP_CLIENT || wan_type == DHCP_PLUS)
    #else
	if(wan_type == DHCP_CLIENT)	/*wait for dhcp set wan ip to in_ifaddrhead*/
    #endif
		sleep(1);
	rtl_setAliasAddrByInfName(wan_intf);
}	

inline void tenda_start_napt(char *ifname)
{
	rtl_setAliasAddrByInfName(ifname);
}	
	
#endif
#ifdef 	HOME_GATEWAY

void start_wan_app()
{
	unsigned int connect_type;
	int wan_type=0 ;
	int ppp_dial_on_demand = 0;
	apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	if((wan_type == PPPOE || wan_type == L2TP || wan_type == PPTP) && connect_type ==1)
		ppp_dial_on_demand = 1;
		
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
	int wan_type;
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
#endif
#if defined(HAVE_DNSPROXY) && !defined(HAVE_RTL_DNSMASQ)
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER	
	if(wan_type==L2TP || wan_type==PPTP)
		restart_dns_proxy(1);
	else
		start_dnsproxy();
#else
	if(ppp_dial_on_demand)
		restart_dns_proxy(1);
	else
		start_dnsproxy();
#endif
#endif

#ifdef CYGPKG_NS_DNS
	dns_res_start();
#endif


#ifdef HAVE_NTP
	syslogAll_printk("Init system time ...\n");
	set_time();
	start_ntp();
#endif

#ifdef HAVE_RTLDD
	start_rtldd();
#endif

#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE) || defined (CONFIG_RTL_IGMP_PROXY_KERNEL_MODE)
#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER		
#else
	start_igmpproxy();
#endif
#endif

#if defined(ROUTE_SUPPORT)
	init_static_route();
	init_rip_route();
#endif

#ifdef ROUTE6D_SUPPORT
	init_rip6_route();
#endif

#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	start_napt();
#endif
#ifdef HAVE_MINIIGD
	start_miniigd();
#endif

}
#endif

#ifdef HOME_GATEWAY
#if defined(HAVE_SYSTEM_REINIT)
void kill_wan_app()
{
#ifdef HAVE_NTP
	clean_ntpclient();
#endif

#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	/*then clearn up NAPT*/
	rtl_UninitNapt();	
#endif

#ifdef HAVE_MINIIGD
	kill_miniigd();
#endif

#if defined(HAVE_DNSPROXY)
	kill_dnsproxy();
#endif

#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)	
	clean_mrouted();
#endif
	
#ifdef HAVE_RTLDD
	kill_rtldd();
#endif
}
#endif
#endif

#ifdef DAYLIGHT_SAVING_TIME
void set_timezone()
{
	_set_timezone();
}
#else
void set_timezone()
{
	char tmpStr[32]={0};
	char *ptr_std, *ptr_dst;
	int daylight_save;
	int stdOffset=0, dstOffset=0;
	
	apmib_get(MIB_NTP_TIMEZONE, (void *)tmpStr);
	apmib_get(MIB_DAYLIGHT_SAVE,(void *)&daylight_save);
//	diag_printf("%s:%d tmpStr(%s) daylight_save(%d)\n",__FILE__,__LINE__,tmpStr,daylight_save);
	if (tmpStr[0] == '\0') //sanity check
		strcpy(tmpStr, "-8 4");
	/**/
	ptr_std=strtok(tmpStr," ");
	ptr_dst=strtok(NULL," ");

	stdOffset = -3600*atoi(ptr_std);
	
	//diag_printf("%s %d ptr_std(%s) ptr_dst(%s)\n",__FUNCTION__,__LINE__,ptr_std,ptr_dst);
	if(daylight_save == 1)
		cyg_libc_time_setdst(CYG_LIBC_TIME_DSTON);
	else
		cyg_libc_time_setdst(CYG_LIBC_TIME_DSTOFF);

	//diag_printf("%s:%d std=%s dst=%s\n",__FILE__,__LINE__,ptr_std,ptr_dst);
	dstOffset=stdOffset-3600*atoi(ptr_dst);
//	diag_printf("%s:%d stdOffset=%d dstOffset=%d\n",__FILE__,__LINE__,stdOffset,dstOffset);
	cyg_libc_time_setzoneoffsets(stdOffset, (daylight_save ? dstOffset : 0));
}
#endif

void set_time()
{
	set_timezone();

	set_system_time_flash();
	
}


#ifdef SYSLOG_SUPPORT
void set_log()
{
	int intValue=0;
	if(!apmib_get(MIB_SCRLOG_ENABLED, (void*)&intValue))
	{
		fprintf(stderr,"get MIB_SCRLOG_ENABLED fail!\n");
		return;
	}
	extern void setSyslogType(int syslog_type);
	setSyslogType(intValue);
#ifdef REMOTELOG_SUPPORT
		if(!apmib_get(MIB_REMOTELOG_ENABLED, (void*)&intValue)){
			fprintf(stderr,"get MIB_REMOTELOG_ENABLED fail!\n");
			return;
		}
		struct in_addr remote_svr_ip;
		if(!apmib_get(MIB_REMOTELOG_SERVER, (void*)&remote_svr_ip)){
			fprintf(stderr,"get MIB_REMOTELOG_SERVER fail!\n");
			return;
		}
		//printf("%s:%d remotelog_enable=%d remote_svr_ip=%s\n",__FUNCTION__,__LINE__,intValue,inet_ntoa(remote_svr_ip));
		if(intValue){
			extern void setRemotelogEnable(int remotelog_enable,void * remotelog_ip);
			setRemotelogEnable(intValue,(void *)(&remote_svr_ip));
		}	
#endif
}
#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
#if defined(CONFIG_8881A_UPGRADE_KIT)
int auto_dhcp = 0;
#endif
void auto_opmode(int port)
{
	int val, val2;


#ifdef BRIDGE_REPEATER
		val2=1;
		apmib_set( MIB_REPEATER_ENABLED1, (void *)&val2);

		apmib_get(MIB_OP_MODE,(void *)&val);
		if(val==BRIDGE_MODE)
		{
			
			printf(" ==> bridge mode\n");
		}
		else
		{
#endif
#if defined(CONFIG_CMJ_SALES20131014)
			printf(" ==> router mode\n");
			val = GATEWAY_MODE;
			apmib_set( MIB_OP_MODE, (void *)&val);
			val = 0;
			apmib_set( MIB_REPEATER_ENABLED1, (void *)&val);
			apmib_set( MIB_REPEATER_ENABLED2, (void *)&val);
			apmib_set( MIB_WLAN_CHANNEL, (void *)&val);
#ifdef BRIDGE_REPEATER
		}
#endif
#if defined(CONFIG_RTL_8881A_SELECTIVE)
			val=3;
#else
			val=2;
#endif
	apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);

#else
	apmib_get( MIB_WAN_DETECT, (void *)&val);

	if (val == 1) {
		apmib_get( MIB_UPGRADE_KIT, (void *)&val);
		apmib_get( MIB_OP_MODE, (void *)&val2);
	
		sleep(2);
		if (val == 1 && val2 == BRIDGE_MODE) { //upgrade kit
			printf(" ==> upgrade kit mode, ");
			val = wan_link_status(port);
			if (val) {
				printf(" wan link, enable DHCPC\n");
				val2 = DHCP_CLIENT;
				#if defined(CONFIG_8881A_UPGRADE_KIT)
				auto_dhcp = 1;
				#endif
			}
			else {
				printf(" wan unlink, disable DHCPC, enable DHCPD\n");
				val2 = DHCP_SERVER;
			}

			apmib_set( MIB_DHCP, (void *)&val2);
		}
		else { // CMJ
			val2 = DHCP_SERVER;
			apmib_set( MIB_DHCP, (void *)&val2);

			val = wan_link_status(port);
			//val = 1;
			if (val) {
				printf(" ==> router mode\n");
				val = GATEWAY_MODE;
				apmib_set( MIB_OP_MODE, (void *)&val);
				val = 0;
				apmib_set( MIB_REPEATER_ENABLED1, (void *)&val);
				apmib_set( MIB_REPEATER_ENABLED2, (void *)&val);
			#if defined(CONFIG_CMJ_WAN_DETECT)
				apmib_set( MIB_WLAN_CHANNEL, (void *)&val);
			#endif				
			}
			else {
				printf(" ==> WISP mode\n");
				val = WISP_MODE;
				apmib_set( MIB_OP_MODE, (void *)&val);
				val = 1;
				apmib_set( MIB_REPEATER_ENABLED1, (void *)&val);
				apmib_set( MIB_REPEATER_ENABLED2, (void *)&val);
			#if defined(CONFIG_CMJ_WAN_DETECT)
				//change to wlan0-vxd
				apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				apmib_get( MIB_WLAN_CHANNEL, (void *)&val);
				//change to wlan0
				apmib_set_vwlanidx(0);
				apmib_set( MIB_WLAN_CHANNEL, (void *)&val);
			#endif
			}
#if defined(CONFIG_RTL_8881A_SELECTIVE)
			val=3;
#else
			val=2;
#endif
			apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
		}
	}
#if defined(CONFIG_CMJ_INDICATE_WISP_STATUS)
	apmib_get( MIB_OP_MODE, (void *)&val);
	if (val == 2)
		indicate_wisp_st = 1;
#endif

#endif /* #if defined(CONFIG_CMJ_SALES20131014) */
}
#endif
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) && defined(CONFIG_CUTE_MAHJONG_RTK_UI)
void cutemj_check_opmode()
{
	int op_mode,wlan_mode,val;
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
	apmib_get(MIB_WLAN_MODE,(void *)&wlan_mode);
	if(op_mode == WISP_MODE && wlan_mode == AP_MODE){
		val = 1;
		apmib_set( MIB_REPEATER_ENABLED1, (void *)&val);
		apmib_set( MIB_REPEATER_ENABLED2, (void *)&val);	
	}
}
#endif
#ifdef ULINK_DHCP_AUTO

void ulink_dhcpauto_genDhcpdConfigFile()
{
	struct in_addr srv_ipaddr={0}, start_addr={0}, end_addr={0};				

	apmib_get(MIB_IP_ADDR, (void*)&srv_ipaddr); 
	//	strncpy(srvipaddr, inet_ntoa(srv_ipaddr), 16);
	//	diag_printf("%s:%d	srv_ipaddr=%s\n", __FUNCTION__, __LINE__, srvipaddr);	

	apmib_get(MIB_DHCP_CLIENT_START, (void*)&start_addr);
	//	strncpy(startaddr, inet_ntoa(start_addr), 16);
	//	diag_printf("%s:%d	start_addr=%s\n", __FUNCTION__, __LINE__, startaddr);

	apmib_get(MIB_DHCP_CLIENT_END, (void*)&end_addr);
	//	strncpy(endaddr, inet_ntoa(end_addr), 16);
	//	diag_printf("%s:%d	end_addr=%s\n", __FUNCTION__, __LINE__, endaddr);

	create_dhcpd_configfile(&srv_ipaddr, &start_addr, &end_addr);

	create_staticip_file();
}

void start_ulink_dhcpauto()
{
	int op_mode=0,lan_dhcp=0;
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
	
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	apmib_get(MIB_DHCP,(void *)&lan_dhcp);
	if(op_mode!=BRIDGE_MODE)
		return;
	switch(lan_dhcp)
	{
		case DHCP_LAN_SERVER:			
			break;
		case DHCP_LAN_CLIENT:
			ulink_dhcpauto_genDhcpdConfigFile();			
			break;
		case DHCP_LAN_NONE:
			ulink_dhcpauto_genDhcpdConfigFile();			
			break;
		default:
			diag_printf("invalid lan dhcp type, not support %d for dhcp auto!",lan_dhcp);
			return;
	}
	
	RunSystemCmd(NULL_FILE, "ulink_dhcp_auto",NULL_STR);
}
#endif
#ifdef DHCP_AUTO_SUPPORT
void dhcpauto_genDhcpdConfigFile()
{
	struct in_addr srv_ipaddr={0}, start_addr={0}, end_addr={0};				

	apmib_get(MIB_IP_ADDR, (void*)&srv_ipaddr); 
	//	strncpy(srvipaddr, inet_ntoa(srv_ipaddr), 16);
	//	diag_printf("%s:%d	srv_ipaddr=%s\n", __FUNCTION__, __LINE__, srvipaddr);	

	apmib_get(MIB_DHCP_CLIENT_START, (void*)&start_addr);
	//	strncpy(startaddr, inet_ntoa(start_addr), 16);
	//	diag_printf("%s:%d	start_addr=%s\n", __FUNCTION__, __LINE__, startaddr);

	apmib_get(MIB_DHCP_CLIENT_END, (void*)&end_addr);
	//	strncpy(endaddr, inet_ntoa(end_addr), 16);
	//	diag_printf("%s:%d	end_addr=%s\n", __FUNCTION__, __LINE__, endaddr);

	create_dhcpd_configfile(&srv_ipaddr, &start_addr, &end_addr);

	create_staticip_file();
}


void get_wlan_mode(int *wlan_mode)
{
	SetWlan_idx("wlan0");
	apmib_get(MIB_WLAN_MODE, (void *)wlan_mode);	
}

void start_dhcpauto()
{
	int op_mode=0,lan_dhcp=0;
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	apmib_get(MIB_DHCP,(void *)&lan_dhcp);
	if(op_mode!=BRIDGE_MODE)
		return;

#if 0
	switch(lan_dhcp)
	{
		case DHCP_LAN_SERVER:			
			break;
		case DHCP_LAN_CLIENT:
			dhcpauto_genDhcpdConfigFile();			
			break;
		case DHCP_LAN_NONE:
			dhcpauto_genDhcpdConfigFile();			
			break;
		default:
			diag_printf("invalid lan dhcp type, not support %d for dhcp auto!",lan_dhcp);
			return;
	}
#endif

	dhcpauto_genDhcpdConfigFile();	

#ifdef CONFIG_ECOS_AP_SUPPORT
	RunSystemCmd(NULL_FILE, "dhcp_auto",NULL_STR);
#else
	//for CMJ bridge mode only have repeater 
	//RunSystemCmd(NULL_FILE, "dhcp_auto","wlan0-vxd0",NULL_STR);
	RunSystemCmd(NULL_FILE, "dhcp_auto",NULL_STR);
#endif
	//or
	//RunSystemCmd(NULL_FILE, "dhcp_auto","eth1",NULL_STR);
	//RunSystemCmd(NULL_FILE, "dhcp_auto","wlan0-vxd0",NULL_STR);
}


#endif


#ifdef HAVE_SYSTEM_REINIT
/*
  * cleanup  wan connection info,connection configuration
  * kill the wan dial up app such as dhcpc ,pppoe, l2tp,pptp
  * and other works
  */
  
static cyg_mutex_t system_reinit_mutex;
static int cleanup_http_flag;//to avoid deadlock
#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
extern int rtl_UninitNapt(void); 
#endif
//extern void dhcpc_stop(void);
#ifdef HAVE_DHCPC
extern void kill_wandhcpc();
#endif

#ifdef HAVE_IWCONTROL
extern void kill_iw();
#endif
#ifdef HAVE_IAPP
extern void kill_iapp();
#endif

#ifdef HAVE_WPS
extern void kill_wscd();
#endif

#ifdef HAVE_MINIIGD
extern void kill_miniigd();
#endif

#ifdef HAVE_SIMPLE_CONFIG
extern void kill_simple_config();
#endif

#ifdef HAVE_TR069
extern int tr069_exit();
#endif

#ifdef ROUTE_SUPPORT
	extern void cyg_routed_exit();
#endif

#ifdef ROUTE6D_SUPPORT
	extern void cyg_route6d_exit();
#endif

#ifdef HAVE_RADVD
	extern void kill_radvd();
#endif

#ifdef HAVE_RTLDD
	extern void kill_rtldd();
#endif

/*mutex is to prevent Module/Event lost
  * for example , handle WAN event will clear WAN_M flag, the mutex
  * will make sure another WAN_M event will not be cleared during previous handling
  */
void system_reinit_mutex_init()
{
	cyg_mutex_init(&system_reinit_mutex);
}

int system_reinit_mutex_lock()
{
	if (cyg_mutex_lock(&system_reinit_mutex))
		return 0;
	return -1;
}

int system_reinit_mutex_unlock()
{
	cyg_mutex_unlock(&system_reinit_mutex);
	return 0;
}

#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
void sys_cleanup_wan(unsigned int flag)
#else
void sys_cleanup_wan()
#endif
{
	int opmode,wanid;
	unsigned char ifname[16];
	DHCP_T old_wan_type;
	DHCP_T cur_wan_type;
	
	apmib_get(MIB_WAN_DHCP,(void *)&cur_wan_type);
	apmib_get(MIB_WAN_OLD_DHCP, (void *)&old_wan_type);

	
#ifdef CONFIG_RTL_DHCP_PPPOE
	int dhcpDisabled=0;
	apmib_get(MIB_CWMP_DHCP_DISABLED, (void *)&dhcpDisabled);	
#endif

	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);

	switch (old_wan_type) {			
		case STATIC_IP:
			break;

		case DHCP_CLIENT:			
			//dhcpc_stop();
			//diag_printf("%s:%d####begin call kill_wandhcpc()\n",__FUNCTION__,__LINE__);
			//kill_wandhcpc();			
			//diag_printf("%s:%d####end call kill_wandhcpc()\n",__FUNCTION__,__LINE__);
			break;

		case PPPOE:
		case PPTP:
		case L2TP:
			ppp_cleanup(old_wan_type);
			/*cleanup will leads to a ppp disconnect event. 
			  *since congfig_wan will be called later, the disconnect event
			  *handle may interrupt the config_wan sometimes and leads to
			  *repeatly connect/disconnect
			  *just set skip flag,  the disconnect event will be ingored once
			  * in disconnect handle
			  */
			diag_printf("%s %d\n",__FUNCTION__,__LINE__);
			  
			set_skip_ppp_disconnect_flag(1);
			break;
			
		default:
			break;
	}
	/*jwj: move delete dhcp and eth1 after ppp clean, because l2tp will send
	stopCNN, it need eth1 router.*/
#ifdef HAVE_DHCPC
#ifdef CONFIG_RTL_DHCP_PPPOE
	if(!(flag&SYS_WAN_PPPOE_M) || dhcpDisabled==1)
	{
		kill_wandhcpc();
		clean_staticIp_info("eth1");
	}
#else
	kill_wandhcpc();
	clean_staticIp_info("eth1");
#endif
#endif	

#ifdef HAVE_NTP
	clean_ntpclient();
#endif
	/*Cleanup default routing*/
	delDefaultRoute();

	/*Cleanup ipfw*/
	flushIpfw();
	
#if defined(HAVE_NAPT)&&!defined(HAVE_NATD)
	/*then clearn up NAPT*/
	rtl_UninitNapt();	
#endif
	
	/*kill wan app */
#ifdef HAVE_MINIIGD
	kill_miniigd();
#endif

#if defined(HAVE_DNSPROXY)
	kill_dnsproxy();
#endif


#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)	
	clean_mrouted();
#endif

#ifdef HAVE_RTLDD
	kill_rtldd();
#endif
	apmib_set(MIB_WAN_OLD_DHCP, (void *)&cur_wan_type);
#ifdef HAVE_HTTPD
	apmib_update_web(CURRENT_SETTING);
#else
	apmib_update(CURRENT_SETTING);
#endif

	apmib_get(MIB_OP_MODE, (void *)&opmode);
	if(opmode == WISP_MODE)
	{
		apmib_get(MIB_WISP_WAN_ID, (void *)&wanid);
		sprintf(ifname, "wlan%d-vxd0", wanid);
		if(is_interface_up(ifname)) {
			interface_down(ifname);
		}
		if(is_interface_up("pwlan0"))
			interface_down("pwlan0");
	}

	diag_printf("%s %d\n",__FUNCTION__,__LINE__);
}
#endif
/* cleanup the lan app, such as dhcpd, mini_upnp, mini_igd ...
  *cleanup the lan configurations
  *and other works related to lan
  */
void sys_cleanup_lan()
{
	/*lan ip & lan mac should be changed. but nothing need to clean*/
	
	/*kill the lan app as we can*/
#ifdef HAVE_DHCPD
	kill_dhcpd();
#endif
#ifdef HAVE_DHCPC
	kill_landhcpc();
#endif
#ifdef HAVE_HTTPD
	clean_httpd();
#endif
#ifdef HAVE_TELNETD
	clean_telnetd();
#endif
#ifdef HAVE_RADVD
	kill_radvd();
#endif
#ifdef DHCP_AUTO_SUPPORT
	clean_dhcp_auto();
#endif
#ifdef ULINK_DHCP_AUTO
	clean_ulink_dhcp_auto();
#endif
#if defined (CONFIG_IPV6) && defined (HAVE_RTL_DNSMASQ)	
	kill_dnsmasq();
#endif
}

/*
 *cleanup the wifi setting ,down the wifi interface ..
 *
 */
void sys_cleanup_wifi()
{

}

/*
 *cleanup the wifi app such as wscd, auth,iwcontrol ...
 */
void sys_cleanup_wifi_app()
{
	/*It should cleanup iwcontrol first 
	*	Because of should be Stopping put message into mail box 
	*	before clean mail box*/
	
#ifdef HAVE_IWCONTROL
	kill_iw();
#endif

#ifdef HAVE_IAPP
	kill_iapp();
#endif

#ifdef HAVE_WPS
	kill_wscd();
#endif

#ifdef HAVE_AUTH
	kill_auth();
#endif

#ifdef HAVE_SIMPLE_CONFIG
	kill_simple_config();
#endif
#ifdef HAVE_WLAN_SCHEDULE
	clean_wlachedule();
#endif
}


/*cleanup the bridge such as delete bridge
  *
  */
void sys_cleanup_bridge()
{
	bridge_clean(1);
}


void sys_cleanup_napt()
{
}

void sys_cleanup_misc()
{
}

void sys_cleanup_rip()
{
	/* kill the RIPv1/v2 daemon routed */
#ifdef ROUTE_SUPPORT
	cyg_routed_exit();
#endif

	/* kill the RIPng daemon route6d */
#ifdef ROUTE6D_SUPPORT
	cyg_route6d_exit();
#endif
}

void sys_cleanup_tr069()
{
	/* kill the route6d daemon */
#ifdef HAVE_TR069
	tr069_exit();
#endif
}

void sys_cleanup_dispatch(unsigned int flag)
{
	/*clean the application which can be killed*/	
	
	#if 0
	if(!flag)
		flag=SYS_ALL_M;
	#endif
	if(flag & SYS_REINIT_ALL || flag & SYS_LAN_M)
		cleanup_http_flag=1;//to avoid deadlock, when cleanup dispatch cleanning httpd waiting for httpd exit, at the same time httpd receive post and try to kick_reinit_m, deadlock occur
	system_reinit_mutex_lock();
	if(flag & SYS_REINIT_ALL)
	{
		clear_reinit_flag(SYS_REINIT_ALL);
		/*TO clean all*/
		flag = SYS_ALL_M;
	}

	/*clean up from high layer to low layer*/
	if(flag & SYS_MISC_M) {
		
		clear_reinit_flag(SYS_MISC_M);
		sys_cleanup_misc();
	}	

	if(flag & SYS_RIP_M) {
		clear_reinit_flag(SYS_RIP_M);
		sys_cleanup_rip();
	}	

	if(flag & SYS_TR069_M) {
		clear_reinit_flag(SYS_TR069_M);
		sys_cleanup_tr069();
	}	

	if(flag & SYS_NAPT_M) {		
		clear_reinit_flag(SYS_NAPT_M);
		sys_cleanup_napt();
	}	
	
#ifdef HOME_GATEWAY	
#ifdef CONFIG_RTL_DHCP_PPPOE
	if(flag & (SYS_WAN_M|SYS_WAN_PPPOE_M)) {
		clear_reinit_flag(SYS_WAN_M|SYS_WAN_PPPOE_M);
		sys_cleanup_wan(flag);
	}
#else
	if(flag & SYS_WAN_M) {
		clear_reinit_flag(SYS_WAN_M);
		sys_cleanup_wan();
	}
#endif
#endif
	if(flag & SYS_LAN_M) {
		clear_reinit_flag(SYS_LAN_M);
		sys_cleanup_lan();
	}

	if(flag & SYS_WIFI_M) {		
		clear_reinit_flag(SYS_WIFI_M);
		sys_cleanup_wifi();
	}	

	if(flag & SYS_BRIDGE_M) {		
		clear_reinit_flag(SYS_BRIDGE_M);
		sys_cleanup_bridge();
	}

	if(flag & SYS_WIFI_APP_M) {	
		clear_reinit_flag(SYS_WIFI_APP_M);
		sys_cleanup_wifi_app();
	}

	if(flag & SYS_DOS_M)
	{
		
	}
#ifdef CONFIG_ECOS_AP_SUPPORT
	if(flag & SYS_AP_LAN_M)
	{
#ifdef HAVE_NTP
		clear_reinit_flag(SYS_AP_LAN_M);
		clean_ntpclient();		
#endif
	}
#endif
	
	system_reinit_mutex_unlock();
	cleanup_http_flag=0;
	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
}

void sys_reinit_all()
{	
	/*reinit start- most code is same as sys init*/
	init_all = 1;
	
	/*Set net forwarding*/
	inet_forwaring();

	/*check if mib validate*/	
	syslogAll_printk("Reinit Check if mib validate ..\n");
	general_mib_check();
	
	/* init apmib*/
	syslogAll_printk("ReInit mib ...\n");
	general_mib_reinit();	
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) 
#if defined(CONFIG_CUTE_MAHJONG_RTK_UI)
	{
		int val=2;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
		cutemj_check_opmode();
	}	
#else
	auto_opmode(1);
#endif
#endif
#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER) && (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) || defined(CONFIG_RTL_ULINKER))
	int op_mode;
	unsigned char buffer[32];
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif

	if(op_mode == BRIDGE_MODE){
		sprintf(buffer, "wlan0-vxd led 4");
		run_clicmd(buffer);//set wlan led
		create_twinkle_rssi();
	}
	else{
		sprintf(buffer, "wlan0-vxd led 3");
		run_clicmd(buffer);//set wlan led
	}
#endif

#if defined(CONFIG_RTL_XDG)
	{
		int val=3;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
	}	
#endif



	/*set day time. Time Zone and flash time*/
	syslogAll_printk("Init system time ...\n");
	set_time();
#ifdef HAVE_MP_DAEMON
	start_mp();
#else
#ifdef SYSLOG_SUPPORT
	/*init log setting*/
	set_log();
#endif
#ifdef HAVE_WPS
	set_WPS_pin();
#endif

	/*Set up WIFI MIB*/
#ifndef HAVE_NOWIFI
	syslogAll_printk("Init wifi mib ...\n");
	configure_wifi();
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	syslogAll_printk("config vlan ...\n");
	configure_vlan();
#endif

	/*Set up NIC MIB or vlan*/
	syslogAll_printk("config nic ...\n");
	configure_nic();
	


	/*configure bridge used to setup the bridge and assign ip address*/
	/*and bring the interface up*/
	syslogAll_printk("config bridge ...\n");
	configure_bridge();

	/*configure opmode, this must after configure_bridge
	   since it will deal with NIC asic*/
	syslogAll_printk("config opmode ...\n");
	configure_opmode();
	
#ifdef HAVE_CERT_MNG
	/*
 	 * Start certificate management system
 	 */
	diag_printf("Certificate management ...\n");
	extern int cert_mng_init(int reinit);
	int cert_init_ret=cert_mng_init(0);
	if(cert_init_ret!=1)
		diag_printf("Certificate Management init failed %d!!!\n", cert_init_ret);
	
#endif
	/*start all lan app here
	  * lan app: 
	  *	  web server
	  *     dhcpd
	  *     miniigd
	  *     miniupnp
	  *     ...
	  */
	
	syslogAll_printk("start lan applications ...\n");
#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105
	start_lan();
#endif

	
#ifdef HAVE_SIMPLE_CONFIG
	start_simple_config();
#endif

#ifdef ULINK_DHCP_AUTO
	start_ulink_dhcpauto();
#endif

//#ifdef DHCP_AUTO_SUPPORT //move to start_lan()
//	start_dhcpauto();
//#endif

//	start_lan_app();

	/*start all wlan app here
	  * wlan app:
	  *       wps
	  *       iwcontrol
	  *       iapp
	  *       ...
	  */
////	syslogAll_printk("start wlan applications ...\n");

	/*Doing Wan configuration. if gateway mode
	  * init according to different wan type
	  * configure nat
	  * configure firewall
	  *
	  */
	syslogAll_printk("Config wan ...\n");
#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
	config_wan(SYS_WAN_M);
#else
	config_wan();
#endif
#endif
diag_printf("%s:%d\n",__FUNCTION__,__LINE__);

#ifdef CONFIG_IPV6
	extern void set_ipv6();
	set_ipv6();
#endif
	/*we will enter a while loop, do according to the coming flag*/

#if defined(HAVE_DNSPROXY) && !defined(HAVE_RTL_DNSMASQ)
#if defined(ECOS_DOMAIN_NAME_QUERY_SUPPORT)
	cyg_dns_proxy_start();
#endif
#endif
#ifdef DAYLIGHT_SAVING_TIME
	diag_printf("start dst loop...\n");
	start_dst_time_loop();
#endif

	// no need to enter loop if reinit
	//enter_loop();
#endif
}

#if defined(HAVE_FIREWALL)
void reset_fw_rule()
{
	struct in_addr addr = {0};
	char wan_intf[MAX_NAME_LEN] = {0};
	char lan_intf[MAX_NAME_LEN] = {0};
	
	getInterfaces(lan_intf,wan_intf);
    
    if ((getInAddr(wan_intf, IP_ADDR, (void *)&addr)==1) && (addr.s_addr != 0))
    {
        //when reinit lan, lan ip may be changed, so need to reset firewall rules.expecially for soure nat rule. 
            
        set_ipfw_rules(wan_intf,lan_intf);
        #ifdef HAVE_IPV6FIREWALL
		set_ip6fw_rules(wan_intf,lan_intf);
	#endif
    }
}
#endif


/*Change lan Setting. eg lan ip,lan mac. 
   note: bridge will not be reinit*/
void sys_reinit_lan()
{
	/*Set up NIC MIB or vlan*/
	syslogAll_printk("config nic ...\n");
	configure_nic();

	/*reinit wlan for MAC Clone should change WLAN MAC*/
#ifndef HAVE_NOWIFI
	syslogAll_printk("config wlan ...\n");
	configure_wifi();
#endif
	/*assign ip address*/
	syslogAll_printk("config lan ...\n");
	configure_lan();
	
	syslogAll_printk("config bridge app ...\n");
	#if 0  //add by z10312  临时解决编译问题, 使得系统可以跑起来, 后续开发按需打开 -0105
	/*reinit lan app*/
	start_lan();
	#endif
#ifdef ULINK_DHCP_AUTO
	start_ulink_dhcpauto();
#endif
//#ifdef DHCP_AUTO_SUPPORT //move to start_lan()
//	start_dhcpauto();
//#endif
#if defined (CONFIG_IPV6) && defined (HAVE_RTL_DNSMASQ)	
	set_dnsv6();
#endif

#if defined(HAVE_FIREWALL)
	reset_fw_rule();
#endif

}

#ifdef HOME_GATEWAY
/*Change Wan Setting. eg Wan type, Wan mac address*/
#ifdef CONFIG_RTL_DHCP_PPPOE
void sys_reinit_wan(unsigned int flag)
#else
void sys_reinit_wan()
#endif
{
	int op_mode;
	/*Set up NIC MIB or vlan*/
	syslogAll_printk("config nic ...\n");
	configure_nic();

	syslogAll_printk("Config wan ...\n");
#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
	config_wan(flag);
#else	
	config_wan();
#endif
#endif

	syslogAll_printk("config wan app ...\n");
}
#endif

void sys_reinit_wifi()
{
	syslogAll_printk("Init wifi mib ...\n");
#ifndef HAVE_NOWIFI
	configure_wifi();
#endif
}


void sys_reinit_wifi_app()
{
	syslogAll_printk("Reinit wifi app ...\n");
#ifdef HAVE_IWCONTROL
	iw_reinit = 1;
#endif
	start_wlan_app();
#ifdef HAVE_IWCONTROL
	iw_reinit = 0;
#endif

	
}

void sys_reinit_bridge()
{
	/*configure bridge used to setup the bridge and assign ip address*/
#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	syslogAll_printk("config vlan ...\n");
	configure_vlan();
#endif
	
	/*and bring the interface up*/
	syslogAll_printk("config bridge ...\n");
	configure_bridge();

	/*OPMODE should be set after config_bridge to handle eth0 and eth1. HF*/
	/*configure opmode, this must after configure_bridge
	   since it will deal with NIC asic*/
	syslogAll_printk("config opmode ...\n");
	configure_opmode();
}

#ifdef HAVE_TR069
void sys_reinit_tr069()
{
	start_tr069();
}
#endif

void sys_reinit_napt()
{
}

void sys_reinit_misc()
{
}

void sys_reinit_rip()
{
	/* start the RIPv1/v2 daemon routed */
#ifdef ROUTE_SUPPORT
	init_rip_route();
#endif

	/* start the RIPng daemon route6d */
#ifdef ROUTE6D_SUPPORT
	init_rip6_route();
#endif
}

void sys_reinit_common(void)
{
	/*Set net forwarding*/
	inet_forwaring();

	/*check if mib validate*/	
	syslogAll_printk("Reinit Check if mib validate ..\n");
	general_mib_check();
	
	/* init apmib*/
	syslogAll_printk("ReInit mib ...\n");
	general_mib_reinit();	
}

/*As init_all  config_nic->bridge->wan->lan*/
void sys_reinit_dispatch(unsigned int flag)
{
	/* flag should not be 0. if it's 0 means all module handled.*/
	if(flag & SYS_REINIT_ALL){
		sys_reinit_all();
		return;
	}

	if(flag & SYS_WIFI_M)
		sys_reinit_wifi();
	
	if(flag & SYS_BRIDGE_M)
	{		
		sys_reinit_bridge();
	}

#ifdef HOME_GATEWAY  
#ifdef CONFIG_RTL_DHCP_PPPOE
	 if(flag & (SYS_WAN_M | SYS_WAN_PPPOE_M))
	 	sys_reinit_wan(flag);
#else
	 if(flag & SYS_WAN_M)
	 	sys_reinit_wan();
	  
#endif
#endif  

	if(flag & SYS_LAN_M)
		sys_reinit_lan();

	if((!(flag &SYS_LAN_M))&&(flag & SYS_WIFI_APP_M))
		sys_reinit_wifi_app();

#ifdef HAVE_TR069
	if(flag & SYS_TR069_M) {
		sys_reinit_tr069();
	}
#endif

	if(flag & SYS_NAPT_M)
		sys_reinit_napt();

	if(flag & SYS_MISC_M)
		sys_reinit_misc();

	if(flag & SYS_RIP_M)
		sys_reinit_rip();

	if(flag & SYS_DOS_M)
	{
#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)
#if defined(DOS_SUPPORT)
		set_dos();
#endif
#endif
	}
#ifdef CONFIG_ECOS_AP_SUPPORT
	if(flag & SYS_AP_LAN_M)
	{
#ifdef HAVE_NTP
		start_ntp();
#endif
	}
#endif
	//when reinit, check whether existed lan/wan ip conflict
	//check_wan_ip();
}

static unsigned long reinit_flag_m=0;
static unsigned long skip_ppp_disconnect_flag=0;
/*Entry of reinit*/
void  sys_reinit_main(unsigned int flag)
{
	diag_printf("%s %d flag 0x%x\n",__FUNCTION__,__LINE__,flag);

#ifdef HOME_GATEWAY
	//if last time happens  lan/wan ip conflict, then next reinit  should reinit all
	if(get_ip_conflict_flag())
	{
		//diag_printf("%s %d setting SYS_REINIT_ALL!!!\n",__FUNCTION__,__LINE__);
		clear_ip_conflict_flag();
		flag=set_reinit_flag(SYS_REINIT_ALL);
	}
#endif
	
	/*clean up*/	
	sys_cleanup_dispatch(flag);
	
	/*reinit Common. such as Apmib*/	
	sys_reinit_common();

	/*reinit*/
	
	sys_reinit_dispatch(flag);
}

unsigned long get_reinit_flag()
{
	return reinit_flag_m;
}
unsigned long clear_reinit_flag(unsigned long value)
{
	reinit_flag_m &= ~value;
	return reinit_flag_m;
}

unsigned long set_reinit_flag(unsigned long value)
{
	reinit_flag_m |=value;
	return reinit_flag_m;
}


unsigned long get_skip_ppp_disconnect_flag(void)
{
	return skip_ppp_disconnect_flag;
}


unsigned long set_skip_ppp_disconnect_flag(unsigned long value)
{
	skip_ppp_disconnect_flag =value;
	return skip_ppp_disconnect_flag;
}

unsigned long clear_skip_ppp_disconnect_flag(void)
{
	return set_skip_ppp_disconnect_flag(0);
}

void kick_reinit_m(unsigned long module)
{
	if(cleanup_http_flag)//to avoid deadlock, this flag mean httpd is going to be clean up, needn't to kick event
		return;
	system_reinit_mutex_lock();
	set_reinit_flag(module);
	system_reinit_mutex_unlock();
	kick_event(REINIT_SYSTEM_EVENT);
}

#endif

#if defined(CONFIG_RTK_PWM)
void pwm_init(void)
{
	//97FN QA board pin MUX
	REG32(0xb8000808)  =   (REG32(0xb8000808)  & (~(0xF << 16))) |  (0x9 << 16); //set RXC
	REG32(0xb8000800)  =   (REG32(0xb8000800)  & (~(0xFFF << 16))) |  (0x754 << 16); //set TXD1, TXD2, TXD3
	REG32(0xb8000010)  =   (REG32(0xb8000010)  & (~0xFFFFFFFF )) |  (0x80003800);
	REG32(0xb8000014)  =   (REG32(0xb8000014)  & (~(0x1F << 4))) |  (0x13 << 4); //enable timer

	REG32(0xb8148008)  =   (REG32(0xb8148008)  & (~(0x3)))|0x3;//enable DW timer 0
}
#endif

void sys_init_main(cyg_addrword_t data)
{
	init_all = 1;
	/*exception handler*/
	install_exception_handler();

#ifdef SYS_INIT_USING_MBOX
	cyg_mbox_create(&sys_mbox_hdl,&sys_mbox);
#else
	/*init event flag*/
	cyg_flag_init(&sys_flag);
#endif

	/*mount ramfs*/
	ramfs_init();
	syslogAll_printk("Mount ramfs ..\n");
	/*init GPIO if needed*/
	gpio_init();

	/*Start Timer*/
	start_sys_timer();
	
#if defined(CONFIG_RTK_PWM)
	pwm_init();
#endif

#ifdef HAVE_WATCHDOG
	watchdog_start();
#endif

#ifdef HAVE_SYSTEM_REINIT
	system_reinit_mutex_init();
#endif

#ifdef CYGPKG_HAL_ROMEPERF_SUPPORT
	rlx_romeperfInit(1);
#endif

#ifdef 	ECOS_DBG_STAT
	dbg_stat_init();
#endif

	/*Set net forwarding*/
	inet_forwaring();
#ifdef CONFIG_RTL_8197F
	//TBD, temporarily return
	//return;
#endif

	/*check if mib validate*/	
	syslogAll_printk("Check if mib validate ..\n");
	general_mib_check();
	
	/* init apmib*/
	syslogAll_printk("Init mib ...\n");
	general_mib_init();	
#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE) 
#if defined(CONFIG_CUTE_MAHJONG_RTK_UI)
	{
		int val=2;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
		cutemj_check_opmode();
	}	
#else
	auto_opmode(1);
#endif
#endif
#if defined(HAVE_TWINKLE_RSSI) && defined(BRIDGE_REPEATER) && (defined(CONFIG_CUTE_MAHJONG_SELECTABLE) || defined(CONFIG_RTL_ULINKER))
	int op_mode;
	unsigned char buffer[32];
	apmib_get(MIB_OP_MODE,(void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif

	if(op_mode == BRIDGE_MODE){
		sprintf(buffer, "wlan0-vxd led 4");
		run_clicmd(buffer);//set wlan led
		create_twinkle_rssi();
	}
	else{
		sprintf(buffer, "wlan0-vxd led 3");
		run_clicmd(buffer);//set wlan led
	}
#endif

#if defined(CONFIG_RTL_XDG)
	{
		int val=3;
		apmib_set(MIB_WLAN_BAND2G5G_SELECT,(void *)&val);
	}	
#endif



	/*set day time. Time Zone and flash time*/
	syslogAll_printk("Init system time ...\n");
	set_time();
#ifdef HAVE_MP_DAEMON
	start_mp();
#else
#ifdef SYSLOG_SUPPORT
	/*init log setting*/
	set_log();
#endif
#ifdef HAVE_WPS
	set_WPS_pin();
#endif

	/*Set up WIFI MIB*/
#ifndef HAVE_NOWIFI
	syslogAll_printk("Init wifi mib ...\n");
	configure_wifi();
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	syslogAll_printk("config vlan ...\n");
	configure_vlan();
#endif

	/*Set up NIC MIB or vlan*/
	syslogAll_printk("config nic ...\n");
    diag_printf("[%s:%d] \n", __FUNCTION__ ,__LINE__);
	configure_nic();
	


	/*configure bridge used to setup the bridge and assign ip address*/
	/*and bring the interface up*/
	syslogAll_printk("config bridge ...\n");
	configure_bridge();

	/*configure opmode, this must after configure_bridge
	   since it will deal with NIC asic*/
	syslogAll_printk("config opmode ...\n");
	configure_opmode();
	
#ifdef HAVE_CERT_MNG
	/*
 	 * Start certificate management system
 	 */
	diag_printf("Certificate management ...\n");
	extern int cert_mng_init(int reinit);
	int cert_init_ret=cert_mng_init(0);
	if(cert_init_ret!=1)
		diag_printf("Certificate Management init failed %d!!!\n", cert_init_ret);
	
#endif
	/*start all lan app here
	  * lan app: 
	  *	  web server
	  *     dhcpd
	  *     miniigd
	  *     miniupnp
	  *     ...
	  */
	
	syslogAll_printk("start lan applications ...\n");
	start_lan();

	
#ifdef HAVE_SIMPLE_CONFIG
	start_simple_config();
#endif

//	start_lan_app();

	/*start all wlan app here
	  * wlan app:
	  *       wps
	  *       iwcontrol
	  *       iapp
	  *       ...
	  */
////	syslogAll_printk("start wlan applications ...\n");
////	start_wlan_app();

	/*Doing Wan configuration. if gateway mode
	  * init according to different wan type
	  * configure nat
	  * configure firewall
	  *
	  */
	syslogAll_printk("Config wan ...\n");
#ifdef HOME_GATEWAY
#ifdef CONFIG_RTL_DHCP_PPPOE
	config_wan(SYS_WAN_M);
#else
	config_wan();
#endif
#endif

#ifdef CONFIG_IPV6
	extern void set_ipv6();
	set_ipv6();
#endif
#ifdef HAVE_TR069
	start_tr069();
#endif
#ifdef ULINK_DHCP_AUTO
	start_ulink_dhcpauto();
#endif

//#ifdef DHCP_AUTO_SUPPORT
//	start_dhcpauto();
//#endif
	/*we will enter a while loop, do according to the coming flag*/



#if defined(HAVE_DNSPROXY) && !defined(HAVE_RTL_DNSMASQ)
#if defined(ECOS_DOMAIN_NAME_QUERY_SUPPORT)
	cyg_dns_proxy_start();
#endif
#endif
#ifdef DAYLIGHT_SAVING_TIME
	diag_printf("start dst loop...\n");
	start_dst_time_loop();
#endif
	enter_loop();
#endif
}

extern void tapf_sys_init(cyg_addrword_t data);
void create_sys_init_thread(void)
{
	/* Create the thread */
	cyg_thread_create(8,
		      tapf_sys_init,
		      0,
                     SYS_INIT_NAME,
                     GET_THREAD_STACK(sys_init),
                     STACK_SIZE,
                     &GET_THREAD_HANDLE_T(sys_init),
                     &GET_THREAD_DATA(sys_init));
	/* Let the thread run when the scheduler starts */
	cyg_thread_resume(GET_THREAD_HANDLE_T(sys_init));
}
#ifdef HAVE_SYSTEM_REINIT
int reinit_test(unsigned int argc, unsigned char *argv[])
{
	if(argc==0)
	{
		kick_reinit_m(SYS_ALL_M);
		return 0;
	}
#ifdef HAVE_HTTPD
	if(strcmp(argv[0],"httpd")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_httpd();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			cyg_httpd_start();
		}
	}else
#endif
#ifdef HAVE_TELNETD
	if(strcmp(argv[0],"telnetd")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_telnetd();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			RunSystemCmd(NULL_FILE, "telnetd",NULL_STR);
		}
	}
#endif
#ifdef HAVE_NTP
	if(strcmp(argv[0],"ntpclient")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_ntpclient();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_ntpclient();
		}
	}
#endif
#ifdef HAVE_WLAN_SCHEDULE
	
	if(strcmp(argv[0],"wlschedule")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_wlachedule();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_wlanSchedule(1);
		}
	}
#endif
#ifdef DHCP_AUTO_SUPPORT
	
	if(strcmp(argv[0],"dhcp_auto")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_dhcp_auto();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_dhcpauto();
		}
	}
#endif
#ifdef ULINK_DHCP_AUTO
	
	if(strcmp(argv[0],"ulink_dhcp_auto")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_ulink_dhcp_auto();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_ulink_dhcpauto();
		}
	}
#endif
#if defined (CONFIG_RTL_IGMP_PROXY_USER_MODE)	
	if(strcmp(argv[0],"mrouted")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			clean_mrouted( );
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_igmpproxy("eth1","eth0");
		}
	}
#endif

#ifdef HAVE_NBSERVER
	
	if(strcmp(argv[0],"nbserver")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"reinit")==0)
		{			
			start_nbserver();	
		}
	}
#endif

#ifdef HAVE_TR069
	if(strcmp(argv[0],"tr069")==0)
	{
		if(argc<2)
		{
			return 0;
		}
		if(strcmp(argv[1],"clean")==0)
		{
			sys_cleanup_tr069();
		}else
		if(strcmp(argv[1],"reinit")==0)
		{
			start_tr069();
		}
		
	}
#endif
	if(strcmp(argv[0],"all")==0)
	{
		kick_reinit_m(SYS_ALL_M);
	}
	if(strcmp(argv[0],"wan")==0)
	{
		kick_reinit_m(SYS_WAN_M);
	}
	if(strcmp(argv[0],"lan")==0)
	{
		kick_reinit_m(SYS_LAN_M);
	}
	if(strcmp(argv[0],"wifi")==0)
	{
		kick_reinit_m(SYS_WIFI_M);
	}
	if(strcmp(argv[0],"bridge")==0)
	{
		kick_reinit_m(SYS_BRIDGE_M);
	}
	if(strcmp(argv[0],"napt")==0)
	{
		kick_reinit_m(SYS_NAPT_M);
	}
	if(strcmp(argv[0],"misc")==0)
	{
		kick_reinit_m(SYS_MISC_M);
	}
	if(strcmp(argv[0],"rip")==0)
	{
		kick_reinit_m(SYS_RIP_M);
	}
#ifdef CONFIG_RTL_DHCP_PPPOE
	if(strcmp(argv[0],"wan2")==0)
	{
		kick_reinit_m(SYS_WAN_PPPOE_M);
	}
#endif
}
#endif

