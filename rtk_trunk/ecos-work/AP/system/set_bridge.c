#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <stdio.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef CYGINT_NET_BRIDGE_HANDLER
#ifdef CONFIG_NET_STACK_FREEBSD
#include <net/if_var.h>
#else
#include <net/if.h>
#endif
#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <net/if_bridge.h>
#include <sys/errno.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>
#include <ctype.h>
#include "net_api.h"
#ifdef HAVE_APMIB
#include "apmib.h"
#endif
#include "sys_utility.h"
#include "sys_init.h"
#include "../common/common.h"

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
#include <pkgconf/devs_eth_rltk_819x_switch.h>
extern int rtl_vlan_support_enable;
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)&&(!defined(HAVE_NOETH))
void rtl8651_customPassthru_infoSetting(void);

extern int old_passThru_flag ;
extern char passThru_flag[2];
extern int passThruStatusWlan;

void config_passThruInfo(void)
{
	rtl8651_customPassthru_infoSetting();
	
}
#endif

extern unsigned char active_br_wlan_interface[BRIDGE_INTERFACE_MAX_LEN];
extern unsigned char last_br_wlan_interface[BRIDGE_INTERFACE_MAX_LEN];

void up_wlan_interface()
{
	char *token=NULL, *savestr1=NULL;
	unsigned char tmpBuff[BRIDGE_INTERFACE_MAX_LEN]; 

	if(active_br_wlan_interface[0] != 0x00)
	{
		token=NULL;
		savestr1=NULL;
		sprintf(tmpBuff, "%s", active_br_wlan_interface);
		token = strtok_r(tmpBuff," ", &savestr1);
		do{
			if (token == NULL){
				break;
			}else{
				if(!is_interface_up(token))
					interface_up(token);
			}
			token = strtok_r(NULL, " ", &savestr1);
		}while(token !=NULL);
	}
	
	return;
}

void set_wlan_bridge(void)
{
	int sock;
	char brdg[] = "bridge0";
	char *token=NULL, *savestr1=NULL;
	unsigned char tmpBuff[BRIDGE_INTERFACE_MAX_LEN]; 
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock >= 0);

	if(active_br_wlan_interface[0] != 0x00)
	{
		token=NULL;
		savestr1=NULL;
		sprintf(tmpBuff, "%s", active_br_wlan_interface);
		token = strtok_r(tmpBuff," ", &savestr1);
		do{
			if (token == NULL){
				break;
			}else{
				//up wlan interface later, otherwise it will cause 92E multiple vap interface (4 vap) Beacon TX DMA error.
				if(!is_interface_up(token))
					 interface_up(token);
				bridge_add(sock,brdg,token);
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
				bridge_ifsetflag (sock, brdg, token, IFBIF_STP);
#endif
			}
			token = strtok_r(NULL, " ", &savestr1);
		}while(token !=NULL);
	}
	close(sock);
	//up_wlan_interface(); 
}

#ifdef HAVE_SYSTEM_REINIT

/*Delete all bridge interface in bridge0. the down flag control whether need to down the interface*/
void bridge_clean(unsigned int downInf)
{
	int sock;
	int count;
	char brdg[] = "bridge0";
	char *buffer, *token, *savestr1;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock >= 0);

	/*the line below will leads to core dump.  HF*/
	//bridge_clrflag(sock,brdg, IFF_UP);
	
	buffer=malloc((IFNAMSIZ+1)*MAX_BRIDGE_INTF_COUNT);
	if(buffer==NULL){
		printf("No buffer\n");
		return;
	}
	memset(buffer,0x0,(IFNAMSIZ+1)*MAX_BRIDGE_INTF_COUNT);
#ifdef HAVE_BRIDGE
	if(bridge_get_interfaces(sock,brdg,buffer,MAX_BRIDGE_INTF_COUNT)>=0)
	{

		token=NULL;
		savestr1=NULL;
		token = strtok_r(buffer," ", &savestr1);
		do{
			if (token == NULL){
				break;
			}else{
				if(downInf) {
					if(is_interface_up(token)) {
						interface_down(token);
					}
				}	
#ifdef HAVE_BRIDGE
				bridge_delete(sock,brdg,token);
#endif
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
				bridge_ifclrflag (sock, brdg, token, IFBIF_STP);
#endif
			}
			token = strtok_r(NULL, " ", &savestr1);
		}while(token !=NULL);
	}	
#endif
	free(buffer);
	close(sock);
}
#endif

//void main_fn(cyg_addrword_t data)
void bridge_main_fn(void)
{
	int sock;
	char brdg[] = "bridge0";
	unsigned int op_mode,wlan_mode;
	//struct bootp brdg_bootp_data;
	int intValue=0;
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock >= 0);

	//BRIDGE_IF_NAME is created when kernel init, so we don't need to addbr
#ifndef HAVE_NOETH
#ifdef CYGHWR_NET_DRIVER_ETH0
#if defined(CONFIG_RTL_VLAN_SUPPORT) && defined(HAVE_BRIDGE)
	if(rtl_vlan_support_enable)		
		bridge_delete(sock,brdg,"eth0");
#endif
	interface_up("eth0");
	eth0_up = 1;
	bridge_add(sock,brdg,"eth0");
#ifdef PPP_POWEROFF_DISCONNECT
	sleep(1);
#endif
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
	bridge_ifsetflag (sock, brdg, "eth0", IFBIF_STP);
#endif
#endif

#ifdef CYGHWR_NET_DRIVER_ETH1
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
#ifdef CONFIG_ECOS_AP_SUPPORT
	op_mode=BRIDGE_MODE;
#endif
	if((op_mode == BRIDGE_MODE) || (op_mode == WISP_MODE))
	{
#if 0
		interface_up("eth1");
		eth1_up = 1;
		bridge_add(sock,brdg,"eth1");
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
		bridge_ifsetflag (sock, brdg, "eth1", IFBIF_STP);
#endif
#endif
	}
#endif

#endif

#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN1
#ifndef HAVE_APMIB
	interface_up("wlan1");
#endif
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if(!(op_mode == WISP_MODE && wlan_mode == CLIENT_MODE)){

	
#ifdef CYGPKG_NET_BRIDGE_STP_CODE
		bridge_add(sock,brdg,"wlan1");
		bridge_ifsetflag (sock, brdg, "wlan1", IFBIF_STP);
#else
		set_wlan_bridge();
#endif
	}
	#ifdef	CONFIG_RTL_CUSTOM_PASSTHRU
	else
	{
		bridge_add(sock,brdg,"pwlan0");
	}
	#endif
#endif
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifndef HAVE_APMIB
	interface_up("wlan0");
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	interface_up("wlan1");
#endif
#endif
	apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
	if(!(op_mode == WISP_MODE && wlan_mode == CLIENT_MODE)){

#ifdef CYGPKG_NET_BRIDGE_STP_CODE
		bridge_add(sock,brdg,"wlan0");
		bridge_ifsetflag (sock, brdg, "wlan0", IFBIF_STP);
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
		bridge_add(sock,brdg,"wlan1");
		bridge_ifsetflag (sock, brdg, "wlan1", IFBIF_STP);
#endif

#else
		set_wlan_bridge();
#endif
	}
	#ifdef	CONFIG_RTL_CUSTOM_PASSTHRU
	else
	{
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
#if 0
		int wisp_wanid=0;
		apmib_get(MIB_WISP_WAN_ID, (void *)&wisp_wanid);
		if(wisp_wanid==0)
			bridge_add(sock,brdg,"wlan1");
		else
			bridge_add(sock,brdg,"wlan0");
#endif
		set_wlan_bridge();
#endif
		//bridge_add(sock,brdg,"pwlan0");
	}
	#endif
#endif

#if defined(CONFIG_RTL_VLAN_SUPPORT)&&(!defined(HAVE_NOETH))
	if(rtl_vlan_support_enable){
		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
			interface_up("eth2");
			bridge_add(sock,brdg,"eth2");
		#ifdef CYGPKG_NET_BRIDGE_STP_CODE
			bridge_ifsetflag (sock, brdg, "eth2", IFBIF_STP);
		#endif
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH3
			interface_up("eth3");
			bridge_add(sock,brdg,"eth3");
		#ifdef CYGPKG_NET_BRIDGE_STP_CODE
			bridge_ifsetflag (sock, brdg, "eth3", IFBIF_STP);
		#endif
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
			interface_up("eth4");
			bridge_add(sock,brdg,"eth4");
		#ifdef CYGPKG_NET_BRIDGE_STP_CODE
			bridge_ifsetflag (sock, brdg, "eth4", IFBIF_STP);
		#endif
		#endif

        #ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7
			interface_up("eth7");
			bridge_add(sock,brdg,"eth7");
		#ifdef CYGPKG_NET_BRIDGE_STP_CODE
			bridge_ifsetflag (sock, brdg, "eth7", IFBIF_STP);
		#endif
		#endif
	}
	else{
		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH2
			if(is_interface_up("eth2")){
#ifdef HAVE_BRIDGE
				bridge_delete(sock,brdg,"eth2");
#endif
				interface_down("eth2");
			}
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH3
			if(is_interface_up("eth3")){
#ifdef HAVE_BRIDGE
				bridge_delete(sock,brdg,"eth3");
#endif
				interface_down("eth3");
			}
		#endif

		#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH4
			if(is_interface_up("eth4")){
#ifdef HAVE_BRIDGE
				bridge_delete(sock,brdg,"eth4");
#endif
				interface_down("eth4");
			}
		#endif

    	#ifdef RTLPKG_DEVS_ETH_RLTK_819X_SWITCH_ETH7
		if(is_interface_up("eth7")){
#ifdef HAVE_BRIDGE
			bridge_delete(sock,brdg,"eth7");
#endif
			interface_down("eth7");
		}
		#endif
	}
#endif

#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)&&(!defined(HAVE_NOETH))
	apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intValue);
	//diag_printf("--------\nintValue:%d,[%s]:[%d].\n",intValue,__FUNCTION__,__LINE__);
	
	if(op_mode == GATEWAY_MODE )
	{
		
		//old_passThru_flag=passThru_flag[0];
		if(intValue & 0x1)
		{
			if(intValue & 0x2)
				passThru_flag[0]=0x3;
			else
				passThru_flag[0]=0x1;
		}
		else
		{
			if(intValue & 0x2)
				passThru_flag[0]=0x2;
			else
				passThru_flag[0]=0;
		}	
		//diag_printf("--------\npassThru_flag[0]:%d,[%s]:[%d].\n",passThru_flag[0],__FUNCTION__,__LINE__);
		if (passThru_flag[0] == 0)
		{
			if(is_interface_up("peth0"))
				interface_down("peth0");
#ifdef HAVE_BRIDGE
			bridge_delete(sock,brdg,"peth0");
#endif
		}
		else
		{
			
			config_passThruInfo();
			interface_up("peth0");
			bridge_add(sock,brdg,"peth0");
			
			
		}
	}
#endif

	bridge_status(sock,brdg);
	bridge_setflag(sock,brdg, IFF_UP);

	/*
	build_bootp_record(&brdg_bootp_data,
                           "eth0",
                           "192.168.1.254",
                           "255.255.255.0",
                           "192.168.1.255",
                           "192.168.1.101",
                           "192.168.1.55");
	//show_bootp("eth0", &brdg_bootp_data);
  	if (!init_net("eth0", &brdg_bootp_data)) {
            printf("Network initialization failed for %s\n", brdg);
	}*/

	/*
	while(1) {
		cyg_kmem_print_stats();
		printf("------------------------------------------------\n");
		bridge_show_all(sock);
		printf("------------------------------------------------\n");
		cyg_thread_delay(500);
	}
	*/
	close(sock);
}

void reset_wlan_bridge(void)
{
	int sock;
	char brdg[] = "bridge0";
	char *token=NULL, *savestr1=NULL;
	unsigned char tmpBuff[80], cmdBuff[80]; 
	
	
	sock = socket(AF_INET, SOCK_DGRAM, 0);
	assert(sock >= 0);

	//use active_br_wlan_interface for virtual AP, vxd and dual band support

	if(last_br_wlan_interface[0] != 0x00)
	{
		token=NULL;
		savestr1=NULL;
		sprintf(tmpBuff, "%s", last_br_wlan_interface);
		token = strtok_r(tmpBuff," ", &savestr1);
		do{
			if (token == NULL){
				break;
			}else{
#ifdef HAVE_BRIDGE
				bridge_delete(sock, brdg, token);
#endif
			}
			token = strtok_r(NULL, " ", &savestr1);
		}while(token !=NULL);
	}
	close(sock);
	
	memcpy(last_br_wlan_interface, active_br_wlan_interface, sizeof(last_br_wlan_interface));
	set_wlan_bridge();

}
#endif

void set_lan_dhcpd(char *interface)
{
	char tmpBuff1[32]={0}, tmpBuff2[32]={0};
	int intValue=0, dns_mode=0;
	char line_buffer[100]={0};
	char tmp1[64]={0};
	char tmp2[64]={0};
	char *strtmp=NULL, *strtmp1=NULL;
	/*
	DHCPRSVDIP_T entry;
	int i, entry_Num=0;
	*/
	
	sprintf(line_buffer,"interface %s\n",interface);
	write_line_to_file(DHCPD_CONF_FILE, 1, line_buffer);

	apmib_get(MIB_DHCP_CLIENT_START,  (void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"start %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_DHCP_CLIENT_END,	(void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"end %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_SUBNET_MASK,	(void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"opt subnet %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_DHCP_LEASE_TIME, (void *)&intValue);
	intValue *= 60;
	sprintf(line_buffer,"opt lease %ld\n",intValue);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	apmib_get(MIB_IP_ADDR,	(void *)tmp1);
	strtmp= inet_ntoa(*((struct in_addr *)tmp1));
	sprintf(line_buffer,"opt router %s\n",strtmp);
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	sprintf(line_buffer,"opt dns %s\n",strtmp); /*now strtmp is ip address value */
	write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

	memset(tmp1, 0x00, 64);
	apmib_get( MIB_DOMAIN_NAME, (void *)&tmp1);
	if(tmp1[0]){
		sprintf(line_buffer,"opt domain %s\n",tmp1);
		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	}

	dhcpd_startup();	
}

void set_lan_dhcp(char *interface)
{
	unsigned int lan_dhcp_mode;
	
	apmib_get(MIB_DHCP, (void *)&lan_dhcp_mode);
	if(lan_dhcp_mode == DHCP_LAN_SERVER)
	{
		set_lan_dhcpd(interface);
	}
	else if(lan_dhcp_mode == DHCP_LAN_CLIENT)
	{
		set_dhcpc(interface);
	}
	else if(lan_dhcp_mode == DHCP_LAN_NONE)
	{
		
	}
	
}




