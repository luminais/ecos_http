
#include <pkgconf/system.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <stdio.h>
#include <network.h>
#include <sys/types.h>
#include <stdlib.h>

#ifdef CONFIG_NET_STACK_FREEBSD
#include <net/if_var.h>
#else
#include <net/if.h>
#endif
#if ECOS_RTL_REPORT_LINK_STATUS
#include <cyg/io/eth/rltk/819x/wrapper/if_status.h>
#endif

#include <net/if_dl.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
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
#include "common.h"

// added for traffic control with the fastnat of realtek, by zhuhuan on 2016.03.01
#ifndef CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#define CONFIG_RTL_QOS_RATE_LIMIT_CHECK
#endif

#if 0
void ppp_dail_on_demand()
{
	/*TODO*/	
	int wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
#ifdef HAVE_PPPOE
	if(wan_type == PPPOE)
		ppp_dail_on_demand(0);
#endif
#ifdef HAVE_L2TP
	if(wan_type== L2TP) {
		//l2tp_dail_on_demand();
	}
#endif
#ifdef HAVE_PPTP
	if(wan_type== PPTP) {
		//pptp_dail_on_demand();
	}
#endif
	
}
#endif
static int backup_wanType = -1;
static int dhcp_connected_flag=0;
static int lan_wan_ip_conflict_flag=0;
static int happen_ip_conflict_flag=0;
static int wanif_flag=0;

#define WAIT_DHCP_TIME 15
#ifdef ECOS_RTL_REPORT_LINK_STATUS

static int wan_idx = -1;
#define MAX_WAN_IDX 32
int set_wan_idx(unsigned int n)
{

	if( n< 0 || n > MAX_WAN_IDX)
		return -1;
	wan_idx = n;
	return 0;
}
int get_wan_idx()
{
	return wan_idx;
}
#endif

void clean_staticIp_info(const char *ifname)
{
	struct ifreq ifr;
	int s = -1;	
	if((s = socket(AF_INET, SOCK_DGRAM, 0))<0)
		return;

	memset(&ifr, 0, sizeof(struct ifreq));
	
	strcpy(ifr.ifr_name, ifname);	
	if (ioctl(s, SIOCGIFADDR, &ifr))
		perror("SIOCGIFADDR 1");

	strcpy(ifr.ifr_name, ifname);
	if (ioctl(s, SIOCDIFADDR, &ifr)) /* delete IF addr */
		perror("SIOCDIFADDR1");
	
	close(s);	
}

int get_ip_conflict_flag()
{	
	return happen_ip_conflict_flag;
}

void set_ip_conflict_flag(int value)
{
	happen_ip_conflict_flag=value;
}

void clear_ip_conflict_flag(void)
{
	happen_ip_conflict_flag=0;
	lan_wan_ip_conflict_flag=0;
}


void ppp_reconnect()
{
	int wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
#ifdef HAVE_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )
    #else
	if(wan_type == PPPOE)
    #endif
		pppoe_reconnect();
#endif

#ifdef HAVE_L2TP
	if(wan_type== L2TP)
		l2tp_reconnect();
#endif

#ifdef HAVE_PPTP
	if(wan_type== PPTP)
		pptp_reconnect();
#endif

}

void ppp_connect()
{
	int wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
#ifdef HAVE_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )
    #else
	if(wan_type == PPPOE)
    #endif
		pppoe_connect();
#endif
	
#ifdef HAVE_L2TP
	if(wan_type== L2TP)
		l2tp_connect();
#endif

#ifdef HAVE_PPTP
	if(wan_type== PPTP)
		pptp_connect();
#endif

}

void _ppp_disconnect(int wan_type)
{	
	unlink("/etc/ppp_link");	
	
#ifdef HAVE_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
		if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )    
    #else
		if(wan_type == PPPOE)
    #endif
			pppoe_disconnect();
#endif
	
#ifdef HAVE_L2TP
		if(wan_type== L2TP)
			l2tp_disconnect();
#endif
	
#ifdef HAVE_PPTP
		if(wan_type== PPTP)
			pptp_disconnect();
#endif
}

void ppp_disconnect()
{
	int wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	_ppp_disconnect(wan_type);
}


#ifdef HAVE_SYSTEM_REINIT
#ifdef HAVE_PPPOE
void pppoe_cleanup(void)
{
	pppoe_disabe();
}
#endif

#ifdef HAVE_L2TP
void l2tp_cleanup(void)
{
}
#endif

#ifdef HAVE_PPTP
void pptp_disable(void);

void pptp_cleanup(void)
{
	pptp_disable();
}
#endif
void  ppp_cleanup(int wan_type)
{
	_ppp_disconnect(wan_type);
#ifdef HAVE_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
		if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )    
    #else
		if(wan_type == PPPOE)
    #endif
			pppoe_cleanup();
#endif
	
#ifdef HAVE_L2TP
		if(wan_type== L2TP)
			l2tp_cleanup();
#endif
	
#ifdef HAVE_PPTP
		if(wan_type== PPTP)
			pptp_cleanup();
#endif	
}
#endif

void ppp_disconnect_handle()
{
	int wan_type;	
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	unlink("/etc/ppp_link");	//remove the ppp link file
	
#ifdef HAVE_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )
    #else
	if(wan_type == PPPOE)
    #endif
		ppppoe_disconnect_handle();
#endif

#ifdef HAVE_L2TP
	if(wan_type== L2TP)
		l2tp_disconnect_handle();
#endif

#ifdef HAVE_PPTP
	if(wan_type== PPTP)
	{
		pptp_disconnect_handle();
	}
#endif
}

#ifdef HAVE_PPPOE
inline void pppoe_connect()
{
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];
	getIntfPPPRunOn(lan_intf,wan_intf);
	/**sys_op & wisp_id  are not used now*/
	set_wanpppoe(0,wan_intf,lan_intf,0);
}
#ifdef PPP_POWEROFF_DISCONNECT
int save_connInfo(u_int16_t Session_ID,u_char*ether_servMac)
{
	char buffer[32]={0};
	if(!ether_servMac)
		return -1;
	sprintf(buffer,"%d",Session_ID);
//	diag_printf("%s:%d Session_ID=%s\n",__FUNCTION__,__LINE__,buffer);
	RunSystemCmd(NULL_FILE, "flash", "set", "PPP_SESSION_NUM",buffer, NULL_STR);
	sprintf(buffer,"%02X%02X%02X%02X%02X%02X",ether_servMac[0],ether_servMac[1],ether_servMac[2],ether_servMac[3],ether_servMac[4],ether_servMac[5]);
//	diag_printf("%s:%d mac=%s\n",__FUNCTION__,__LINE__,buffer);
	RunSystemCmd(NULL_FILE, "flash", "set", "PPP_SERVER_MAC",buffer, NULL_STR);
	return 0;
}

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_REPEATER_MODE
extern struct eth_drv_sc rltk819x_wlan0_vxd_sc0;
#endif
#ifndef HAVE_NOETH
extern struct eth_drv_sc rltk819x_sc1;
extern int rltk819x_send_eth(struct eth_drv_sc *sc, unsigned char *data, int size);
#endif
extern int rltk819x_send_wlan(struct eth_drv_sc *sc, unsigned char *data, int size);

void send_ppp_termination(char *wan_iface)
{
	unsigned int session_id=0;
	apmib_get(MIB_PPP_SESSION_NUM,(void *)&session_id);
//	diag_printf("%s:%d session_id=%d\n",__FUNCTION__,__LINE__,session_id);
	if(session_id)
	{//need to disconnect		
		//diag_printf("%s:%d session_id=%d\n",__FUNCTION__,__LINE__,session_id);
		send_ppp_msg(session_id,wan_iface);		
		sleep(1);
		send_padt_msg(session_id,wan_iface);		
		sleep(2);	
		{
		//	unsigned char serverMac[6]={0};
		//	save_connInfo(0,serverMac);
		}
	}
}
int send_padt_msg(unsigned int sessid,char *wan_iface)
{
	unsigned char buf[64]={0};
	unsigned char macAddr[MAC_ADDR_LEN]={0};
	unsigned char *p = buf;
	unsigned short sessid_s=sessid;

//	diag_printf("%s:%d session_id=%d wan_iface=%s\n",__FUNCTION__,__LINE__,sessid,wan_iface);

	//ethernet header
	apmib_get(MIB_PPP_SERVER_MAC,(void *)macAddr);
	memcpy(p,macAddr,MAC_ADDR_LEN);
	p+=MAC_ADDR_LEN;

	get_mac_address(wan_iface,macAddr);
	memcpy(p,macAddr,MAC_ADDR_LEN);
	p+=MAC_ADDR_LEN;

	memcpy(p, "\x88\x63", 2);//pppoe session
	p += 2;
	//ethernet header end

	//pppoe header
	memcpy(p, "\x11\xa7", 2);//version type ,session date
	p += 2;
	memcpy(p, &sessid_s, 2);//session id
	p += 2;
	memcpy(p, "\x00\x00", 2);//payload length
	p += 2;
	//pppoe header end	
	if(strcmp(wan_iface,"eth1")==0)
	{
		#ifndef HAVE_NOETH
		return rltk819x_send_eth(&rltk819x_sc1, buf, sizeof(buf));
		#endif
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_REPEATER_MODE
	else if(strcmp(wan_iface,"wlan0-vxd0")==0)
	{
		return rltk819x_send_wlan(&rltk819x_wlan0_vxd_sc0, buf, sizeof(buf));
	}
#endif
	else
		diag_printf("unkonown wan type!");
	return 0;
}

int send_ppp_msg(unsigned int sessid,char *wan_iface)
{
	unsigned char buf[64]={0};
	unsigned char macAddr[MAC_ADDR_LEN]={0};
	unsigned char *p = buf;
	unsigned short sessid_s=sessid;

//	diag_printf("%s:%d session_id=%d wan_iface=%s\n",__FUNCTION__,__LINE__,sessid,wan_iface);

	//ethernet header
	apmib_get(MIB_PPP_SERVER_MAC,(void *)macAddr);
	memcpy(p,macAddr,MAC_ADDR_LEN);
	p+=MAC_ADDR_LEN;
	
	get_mac_address(wan_iface,macAddr);
	memcpy(p,macAddr,MAC_ADDR_LEN);
	p+=MAC_ADDR_LEN;

	memcpy(p, "\x88\x64", 2);//pppoe session
	p += 2;
	//ethernet header end

	//pppoe header
	memcpy(p, "\x11\x00", 2);//version type ,session date
	p += 2;
	memcpy(p, &sessid_s, 2);//session id
	p += 2;
	memcpy(p, "\x00\x12", 2);//payload length
	p += 2;
	//pppoe header end	

	//pppheader
	memcpy(p, "\xc0\x21", 2);
	p += 2;

	//LCP data
	memcpy(p, "\x05", 1);//termination req
	p += 1;
	
	memcpy(p, "\x02", 1);//identifier
	p += 1;
	
	memcpy(p, "\x00\x10", 2);//length
	p += 2;
	
	memcpy(p, "\x55\x73\x65\x72\x20\x72\x65\x71\x75\x65\x73\x74", 12);//User request
	p += 12;
	if(strcmp(wan_iface,"eth1")==0)
	{
		#ifndef HAVE_NOETH
		return rltk819x_send_eth(&rltk819x_sc1, buf, sizeof(buf));
		#endif
	}
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_REPEATER_MODE
	else if(strcmp(wan_iface,"wlan0-vxd0")==0)
	{
		return rltk819x_send_wlan(&rltk819x_wlan0_vxd_sc0, buf, sizeof(buf));
	}
#endif
	else
		diag_printf("unkonown wan type!");
	return 0;
}

#else
int save_connInfo(u_int16_t Session_ID,u_char*ether_servMac)
{
	return 0;
}
#endif

inline void pppoe_disconnect()
{
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];
	
	getIntfPPPRunOn(lan_intf,wan_intf);
	/*clear and restart. wait ppp to clean*/
	pppoe_stop_on_intf(wan_intf,0);
	unlink(WAN_INFO_FILE);
	unlink(DNS_INFO_FILN);
}
inline void pppoe_reconnect()
{
	/*wait pppe clear*/
	sleep(1);
	pppoe_disconnect();
	sleep(1);
	pppoe_connect();
}

void ppppoe_disconnect_handle()
{
	int type;
	apmib_get(MIB_PPP_CONNECT_TYPE,&type);
	
#ifdef CONFIG_RTL_DHCP_PPPOE	
	setDefaultGateway(1);
#if defined(HAVE_FIREWALL)					
	set_ipfw_rules("eth1","eth0");
#endif			
#endif

	if(MANUAL == type ) {
		pppoe_disconnect();
	}
	else if(CONTINUOUS == type)
	{
		pppoe_reconnect();
	}
	else /*CONNECT_ON_DEMAND == type*/
	{	
		
		//extern int pppoe_dail_on_demand(int id);
		char wan_intf[MAX_NAME_LEN];
		char lan_intf[MAX_NAME_LEN];
		getIntfPPPRunOn(lan_intf,wan_intf);
		if(is_interface_up(wan_intf))
			interface_down(wan_intf);
			/*wait pppe clear*/
		sleep(1);
		pppoe_disconnect();
		ppp_dail_on_demand(0);
	}
	return;
}
//int pppoe_running;
void set_wanpppoe(int sys_op, char *wan_iface, char *lan_iface, int wisp_id)
{	
	char service_Name[40];
	char username[40];
	char password[40];
	unsigned int idle_timeout;
	unsigned int connect_type;
	unsigned int mtu;
#ifdef CONFIG_RTL_DHCP_PPPOE
	int pppoeDisabled=0;
	apmib_get(MIB_CWMP_PPPOE_DISABLED, (void *)&pppoeDisabled);
	if(pppoeDisabled==1)
	{
		kick_event(DHCP_EVENT); //set DHCP_EVENT to start wan app and set ipfw
		return ;
	}
#endif
	apmib_get(MIB_PPP_SERVICE_NAME, (void *)service_Name);
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    get_encrypt_pppoe_name(username);
    #else
	apmib_get(MIB_PPP_USER_NAME, (void *)username);
	#endif
	apmib_get(MIB_PPP_PASSWORD, (void *)password);
	apmib_get(MIB_PPP_IDLE_TIME, (void *)&idle_timeout);
	apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&connect_type);
	apmib_get(MIB_PPP_MTU_SIZE, (void *)&mtu);
	diag_printf("%s:%d\n",__FUNCTION__,__LINE__);
	/*ifconfig wan interface up*/			
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "up", NULL_STR);
	
#ifdef PPP_POWEROFF_DISCONNECT
	send_ppp_termination(wan_iface);	
#endif
	pppoe_start_on_intf(wan_iface,0,0,service_Name,NULL,username,password,idle_timeout,connect_type,mtu,save_connInfo);
}
#endif


#ifdef HAVE_L2TP
int L2tp_close(void);
void l2tp_open(char *wan_intname, char *username, char * passwd, struct in_addr  *peer_addr, int mtu, unsigned int connect_type,int idle_timeout, int flag);

void l2tp_connect(void)
{
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];
	getIntfPPPRunOn(lan_intf,wan_intf);
	/**sys_op & wisp_id  are not used now*/

#if 0
	int l2tp_wanip_dynamic=0;
	apmib_get(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&l2tp_wanip_dynamic);
	if(l2tp_wanip_dynamic==L2TP_PPTP_DYNAMIC_IP)
	{
		int wait_sec=0;
		dhcpc_reconnect(1);
		while(dhcp_connected_flag==0)
		{
			sleep(1);
			wait_sec++;
			if(wait_sec>WAIT_DHCP_TIME)
				break;
		}
		dhcp_connected_flag=0;
	}
#endif

#if 0
	while(1)
	{			
		if(getWanLink(wan_intf))	
		{
			set_l2tp(0,wan_intf,lan_intf,0);
			break;
		}
		else
			sleep(1);
	}
#endif	
	//if(getWanLink(wan_intf))	
	set_l2tp(0,wan_intf,lan_intf,0);
	//set_wanl2tp(0,wan_intf,lan_intf,0);
}
void l2tp_disconnect(void)
{
	L2tp_close();
	unlink(WAN_INFO_FILE);
//#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
//	int wan_type;
//	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	setDefaultGateway(1);
//#endif
}
void l2tp_reconnect()
{
	/*wait ppp clear*/
	sleep(1);
	l2tp_disconnect();
	sleep(1);
	l2tp_connect();
}

void l2tp_disconnect_handle()
{
	int type;
	apmib_get(MIB_L2TP_CONNECTION_TYPE,&type);
	if(MANUAL == type ) {
		l2tp_disconnect();
	}
	else if(CONTINUOUS == type)
	{
		l2tp_reconnect();
	}
	else /*CONNECT_ON_DEMAND == type*/
	{
//		extern int pppo_dail_on_demand(int id);
#if 0
		char wan_intf[MAX_NAME_LEN];
		char lan_intf[MAX_NAME_LEN];
		getIntfPPPRunOn(lan_intf,wan_intf);
		if(is_interface_up(wan_intf))
			interface_down(wan_intf);
#endif
		/*wait ppp clear*/
		sleep(1);		
		l2tp_disconnect();		
		sleep(2);
		ppp_dail_on_demand(0);
	}
	return;
}

void set_wanl2tp(int sys_op, char *wan_iface, char *lan_iface, int wisp_id)
{	
	char username[32];
	char password[32];
	unsigned int idle_timeout;
	unsigned int connect_type;
	unsigned int mtu;	
	struct in_addr address;
	struct in_addr peer_addr;
	char ip_str[16],mask_str[16];

	apmib_get(MIB_L2TP_USER_NAME, (void *)username);
		
	apmib_get(MIB_L2TP_PASSWORD, (void *)password);
	
	apmib_get(MIB_L2TP_IP_ADDR, (void *)&address);
	inet_ntoa_r(address,ip_str);
	apmib_get(MIB_L2TP_SUBNET_MASK, (void *)&address);	
	inet_ntoa_r(address,mask_str);

	diag_printf("wan_iface(%s) lan_iface(%s)\n",wan_iface,lan_iface);
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "up", NULL_STR);
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, ip_str, mask_str, NULL_STR);
	apmib_get(MIB_L2TP_SERVER_IP_ADDR, (void *)&peer_addr);
	apmib_get(MIB_L2TP_CONNECTION_TYPE, (void *)&connect_type);
	apmib_get(MIB_L2TP_IDLE_TIME, (void *)&idle_timeout);
	apmib_get(MIB_L2TP_MTU_SIZE, (void *)&mtu);
	/*config*/
	diag_printf("peer_addr %x\n",peer_addr.s_addr);
	diag_printf("username(%s) password(%s)  peer_addr(0x%x) mtu(%d), idle_timeout(%d)",
		username,password,peer_addr.s_addr,mtu,idle_timeout);
	l2tp_open(wan_iface,username,password, &peer_addr,mtu,connect_type,idle_timeout, 0);
	diag_printf("%s %d\n",__FUNCTION__,__LINE__);
}

void set_l2tp(int sys_op, char *wan_iface, char *lan_iface, int wisp_id)
{	
	char username[32];
	char password[32];
	unsigned int idle_timeout;
	unsigned int connect_type;
	unsigned int mtu;	
	struct in_addr address;
	struct in_addr peer_addr;
	char ip_str[16],mask_str[16];

#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)	
	int dns_mode;
	apmib_get(MIB_DNS_MODE, (void *)&dns_mode);
	domain2ip(L2TP, dns_mode);		
#endif

	apmib_get(MIB_L2TP_USER_NAME, (void *)username);
		
	apmib_get(MIB_L2TP_PASSWORD, (void *)password);	

	diag_printf("wan_iface(%s) lan_iface(%s)\n",wan_iface,lan_iface);
	
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "up", NULL_STR);
	apmib_get(MIB_L2TP_SERVER_IP_ADDR, (void *)&peer_addr);
	apmib_get(MIB_L2TP_CONNECTION_TYPE, (void *)&connect_type);
	apmib_get(MIB_L2TP_IDLE_TIME, (void *)&idle_timeout);
	apmib_get(MIB_L2TP_MTU_SIZE, (void *)&mtu);	
	
//#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
	addOneRoute(&peer_addr, wan_iface);
//#endif

	/*config*/
	diag_printf("peer_addr %x\n",peer_addr.s_addr);
	diag_printf("username(%s) password(%s)  peer_addr(0x%x) mtu(%d), idle_timeout(%d)",
		username,password,peer_addr.s_addr,mtu,idle_timeout);
		
	l2tp_open(wan_iface,username,password, &peer_addr,mtu,connect_type,idle_timeout, 0);

	//diag_printf("%s %d\n",__FUNCTION__,__LINE__);
}
#endif

#ifdef HAVE_PPTP
void set_wanpptp(int sys_op, char *wan_iface, char *lan_iface, int wisp_id)
{	
	char username[32];
	char password[32];
	unsigned int idle_timeout;
	unsigned int connect_type;
	unsigned int mtu;	
	struct in_addr address;
	struct in_addr local_addr,peer_addr;
	char ip_str[16],mask_str[16];	
	int mppc_flag =0, mppe_flag =0;
	
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)	
	int dns_mode;
	apmib_get(MIB_DNS_MODE, (void *)&dns_mode);
	domain2ip(PPTP, dns_mode);		
#endif

	apmib_get(MIB_PPTP_USER_NAME, (void *)username);
		
	apmib_get(MIB_PPTP_PASSWORD, (void *)password);
	apmib_get(MIB_PPTP_IP_ADDR, (void *)&address);
	inet_ntoa_r(address,ip_str);
	apmib_get(MIB_PPTP_SUBNET_MASK, (void *)&address);	
	inet_ntoa_r(address,mask_str);

	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "up", NULL_STR);
	//RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, ip_str, mask_str, NULL_STR);

	//local ip may eth1/wlan0 ip address(may need redefine)
	//apmib_get(MIB_PPTP_IP_ADDR, (void *)&local_addr);
	if(getInAddr(wan_iface, IP_ADDR, (void *)&local_addr)==0)
		diag_printf("%s,%d,get ip fail\n",__FUNCTION__,__LINE__);
	
	apmib_get(MIB_PPTP_SERVER_IP_ADDR, (void *)&peer_addr);
	apmib_get(MIB_PPTP_CONNECTION_TYPE, (void *)&connect_type);
	apmib_get(MIB_PPTP_IDLE_TIME, (void *)&idle_timeout);
	apmib_get(MIB_PPTP_MTU_SIZE, (void *)&mtu);
	apmib_get( MIB_PPTP_MPPC_ENABLED, (void *)&mppc_flag);
	apmib_get( MIB_PPTP_SECURITY_ENABLED, (void *)&mppe_flag);
	/*config*/

//#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
	addOneRoute(&peer_addr, wan_iface);
//#endif

	diag_printf("username(%s) password(%s)  peer_addr(0x%x) mtu(%d), idle_timeout(%d) mppc_flag(%d) mppe_flag(%d)",
		username,password,peer_addr.s_addr,mtu,idle_timeout,mppc_flag,mppe_flag);
	
	start_pptp(wan_iface,username,password, &local_addr,&peer_addr,mtu,connect_type,idle_timeout, 0 , mppc_flag,mppe_flag);
}


void pptp_connect(void)
{
	char wan_intf[MAX_NAME_LEN];
	char lan_intf[MAX_NAME_LEN];
	getIntfPPPRunOn(lan_intf,wan_intf);
	/**sys_op & wisp_id  are not used now*/
	set_wanpptp(0,wan_intf,lan_intf,0);	
}

void pptp_disconnect(void)
{
	pptp_close();
}
void pptp_reconnect()
{
	sleep(1);
	pptp_disconnect();
	sleep(1);	
	pptp_connect();
}

void pptp_disconnect_handle()
{
	int type;
	int pptp_wanip_dynamic=0;
	apmib_get(MIB_PPTP_CONNECTION_TYPE,&type);
	if(MANUAL == type ) {
		pptp_disconnect();
	}
	else if(CONTINUOUS == type)
	{
		//	diag_printf("[%s],%d** \n",__FUNCTION__,__LINE__);
		apmib_get(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&pptp_wanip_dynamic);
		if(pptp_wanip_dynamic==L2TP_PPTP_STATIC_IP)
			pptp_reconnect();
	}
	else /*CONNECT_ON_DEMAND == type*/
	{
		char wan_intf[MAX_NAME_LEN];
		char lan_intf[MAX_NAME_LEN];
		getIntfPPPRunOn(lan_intf,wan_intf);
		if(is_interface_up(wan_intf))
			interface_down(wan_intf);
		sleep(1);
		pptp_disconnect();
		sleep(2);
		ppp_dail_on_demand(0);		
	}
	return;
}
#endif
void set_staticIP(int sys_op, char *wan_iface, char *lan_iface, int wisp_id)
{
	int intValue=0;
	char tmpBuff[200];
	char tmp_args[16];
	char Ip[32], Mask[32], Gateway[32];
	int wan_type;	
	apmib_get( MIB_WAN_DHCP,  (void *)&wan_type);

	if(wan_type==PPTP)
		apmib_get( MIB_PPTP_IP_ADDR,  (void *)tmpBuff);
	else if(wan_type==L2TP)
		apmib_get( MIB_L2TP_IP_ADDR,  (void *)tmpBuff);	
	else
		apmib_get( MIB_WAN_IP_ADDR,  (void *)tmpBuff);	
	sprintf(Ip, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));

	if(wan_type==PPTP)
		apmib_get( MIB_PPTP_SUBNET_MASK,  (void *)tmpBuff);
	else if(wan_type==L2TP)
		apmib_get( MIB_L2TP_SUBNET_MASK,  (void *)tmpBuff);
	else
		apmib_get( MIB_WAN_SUBNET_MASK,  (void *)tmpBuff);	

	sprintf(Mask, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));

	if(wan_type==PPTP)
		apmib_get(MIB_PPTP_DEFAULT_GW,  (void *)tmpBuff);
	else if(wan_type==L2TP)
		apmib_get(MIB_L2TP_DEFAULT_GW,  (void *)tmpBuff);
	else
		apmib_get(MIB_WAN_DEFAULT_GATEWAY,  (void *)tmpBuff);	
				
	if (!memcmp(tmpBuff, "\x0\x0\x0\x0", 4))
		memset(Gateway, 0x00, 32);
	else
		sprintf(Gateway, "%s", inet_ntoa(*((struct in_addr *)tmpBuff)));

	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, Ip, "netmask", Mask, NULL_STR);
	
	if(Gateway[0]){
		RunSystemCmd(NULL_FILE, "route", "flush", NULL_STR);
		//RunSystemCmd(NULL_FILE, "route", "add", "default", Gateway, NULL_STR);
		RunSystemCmd(NULL_FILE, "route", "add", "-gateway", Gateway, "-interface", wan_iface, NULL_STR);	

		if(wan_type==L2TP || wan_type==PPTP)
			write_line_to_file("/etc/dhcpc_route.conf", 1, Gateway);
	}

	if(wan_type!=PPTP && wan_type!=L2TP)
	{
		apmib_get(MIB_FIXED_IP_MTU_SIZE, (void *)&intValue);
		sprintf(tmp_args, "%d", intValue);
		RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "mtu", tmp_args, NULL_STR);
	}
	
	/*
	//RunSystemCmd(NULL_FILE, "killall", "-9", "dnrd", NULL_STR);
	start_dns_relay();
	start_upnp_igd(DHCP_DISABLED, sys_op, wisp_id, lan_iface);
	setFirewallIptablesRules(0, NULL);
	
	start_ntp();
	start_ddns();
	start_igmpproxy(wan_iface, lan_iface);
#if defined(ROUTE_SUPPORT)
	del_routing();
	start_routing(wan_iface);
#endif
#ifdef SEND_GRATUITOUS_ARP
	//char tmpBuf[128];
	snprintf(tmpBuff, 128, "%s/%s %s", _CONFIG_SCRIPT_PATH, _FIREWALL_SCRIPT_PROG, "Send_GARP");	
	//printf("CMD is : %s \n", tmpBuff);
	system(tmpBuff);
#endif	
	*/

}

void set_dhcp_client(int sys_op, char *wan_iface, char *lan_iface, int wisp_id, int act_source)
{
}

#ifdef HAVE_DHCPC
void set_dhcpWanInfMtu(char *wan_iface)
{
	int intValue = 0;
	char tmp_args[16];
	
	apmib_get(MIB_DHCP_MTU_SIZE, (void *)&intValue);
	sprintf(tmp_args, "%d", intValue);
	RunSystemCmd(NULL_FILE, "ifconfig", wan_iface, "mtu", tmp_args, NULL_STR);
}
#endif

#ifdef ECOS_DBG_STAT
int dbg_dhcpc_index=0;
//int dbg_stat_add(int dbg_type, int module_type, int action_type, unsigned int size);
//int dbg_stat_register(char *name);
#endif
#ifdef KLD_ENABLED
void set_wanspeed(unsigned char *wan_interface, int op_mode)
{
    int intValue = 0;
    unsigned int    ret;
    unsigned int    args[0];

    if (wan_interface == NULL)
        return;
    
	if (op_mode != GATEWAY_MODE) 
	    return;
    
	apmib_get(MIB_WAN_FORCE_SPEED,  (void *)&intValue);
    ret = (unsigned int)intValue;
    re865xIoctl(wan_interface, RTL8651_IOCTL_SETWANLINKSPEED, (unsigned int)(args), 0, (unsigned int)&ret);

    return;
}
#endif
#if defined(CONFIG_GET_SERVER_IP_BY_DOMAIN)
int translate_domain_to_ip(unsigned char *server_domain, struct in_addr *server_ip)
{
	unsigned char tmp_server_ip[32];	
	unsigned char str[32], tmp_cmd[128];
	char   **pptr;
	struct hostent *hptr;
	int count=0;
		
	while(count<=3)
	{
		if((hptr = gethostbyname(server_domain)) != NULL)
		{
			sprintf(tmp_server_ip, "%s", inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
			inet_aton(tmp_server_ip, (void *)server_ip);
			return 0;		
		}
		else
		{
			printf(" gethostbyname error for host:%s try again!\n", server_domain);
			count++;
		}
	}
	return -1;
}

void domain2ip(int wan_type, int dns_mode)
{	
	unsigned char server_domain[33];
	struct in_addr server_ip;
	int enable_server_domain=0;
	struct in_addr dns1, dns2;
	unsigned char tmpbuf[32];
		
	if(wan_type!=PPTP && wan_type!=L2TP)
		return;
	
	if(wan_type==PPTP)
		apmib_get(MIB_PPTP_GET_SERV_BY_DOMAIN,(void*)&enable_server_domain);
	else if(wan_type==L2TP)
		apmib_get(MIB_L2TP_GET_SERV_BY_DOMAIN,(void*)&enable_server_domain);
	
	if(enable_server_domain)
	{
		if(dns_mode==DNS_MANUAL)
		{
			apmib_get(MIB_DNS1, (void *)&dns1);
			apmib_get(MIB_DNS2, (void *)&dns2);
			if(dns1.s_addr!=0)
			{
				sprintf(tmpbuf,"nameserver %s\n",inet_ntoa(dns1));
				write_line_to_file("/etc/resolv.conf", 1, tmpbuf);
			}
			if(dns2.s_addr!=0)
			{
				sprintf(tmpbuf,"nameserver %s\n",inet_ntoa(dns2));
				if(dns1.s_addr!=0)
					write_line_to_file("/etc/resolv.conf", 2, tmpbuf);	
				else
					write_line_to_file("/etc/resolv.conf", 1, tmpbuf);
			}			
		}
	#ifdef CYGPKG_NS_DNS
		dns_res_start();
	#endif
		
		if(wan_type==PPTP)
			apmib_get(MIB_PPTP_SERVER_DOMAIN, server_domain);
		else if(wan_type==L2TP)
			apmib_get(MIB_L2TP_SERVER_DOMAIN, server_domain);
		
		if(translate_domain_to_ip(server_domain, &server_ip) == 0)
		{			
			if(wan_type==PPTP)
				apmib_set(MIB_PPTP_SERVER_IP_ADDR, (void *)&server_ip);
			else if(wan_type==L2TP)
				apmib_set(MIB_L2TP_SERVER_IP_ADDR, (void *)&server_ip);
			
			apmib_update(CURRENT_SETTING);
		}
		else
		{
			diag_printf("can't get ServerDomain:%s 's IP",server_domain);
			return 0;
		}
	}
}
#endif

//#ifdef SUPPORT_FOR_RUSSIA_CUSTOMER
void addOneRoute(struct in_addr *server_addr, char *wan_iface)
{
	int wan_type = -1;
	FILE *fp=NULL;
	struct in_addr mask_val,gw_addr;	
	unsigned char routebuf[16];
	
	if(server_addr==NULL)
		return;

	apmib_get( MIB_WAN_DHCP,  (void *)&wan_type);

	/*Get mask address value!!!*/
	if(wan_type==PPTP)
		apmib_get( MIB_PPTP_SUBNET_MASK,  (void *)&mask_val);
	else if(wan_type==L2TP)
		apmib_get( MIB_L2TP_SUBNET_MASK,  (void *)&mask_val);
	else
		apmib_get( MIB_WAN_SUBNET_MASK,  (void *)&mask_val);	

	/*Get gw address!!!*/
	if((fp=fopen("/etc/dhcpc_route.conf", "r+"))==NULL)
		return;
	fscanf(fp, "%s", routebuf);
	fclose(fp);	

	if ( !inet_aton(routebuf, &gw_addr) ) {
		diag_printf("\r\n Invalid IP-address value!__[%s-%u]\r\n",__FILE__,__LINE__);
		return;
	}

	/*check weather the default gw address  in the same subnet with l2tp/pptp server ip*/
	if((server_addr->s_addr & mask_val.s_addr) != (gw_addr.s_addr & mask_val.s_addr))
	{
		char server_ip[16];
		memset(server_ip, 0, sizeof(server_ip));
		inet_ntoa_r(*server_addr, server_ip);
		RunSystemCmd(NULL_FILE, "route", "add", "-host", server_ip, "-netmask", "255.255.255.255", "-gateway", routebuf, "-interface", wan_iface, NULL_STR);	
	}
	else
	{			
		//diag_printf("%s,%d.In the same subnet !!!!\n",__FUNCTION__,__LINE__);
	}
}
//#endif
#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS
unsigned char alias2_wan_interface[16] = {0};
void rtl_checkdoubleAlias(int wan_type, unsigned char *waninterface)
{
    if (!waninterface) return;
    
    if ((wan_type == PPTP || wan_type == L2TP))
    { 
        
        struct in_addr address;
        
        rtl_setDoubleAlias(1);
        
        if ((getInAddr(waninterface, IP_ADDR, (void *)&address)==0) || (address.s_addr == 0))
        {
            //failed 
            //diag_printf("%s %d cannot get alias ip address!\n", __FUNCTION__, __LINE__);
            rtl_setDoubleAliasIp(0);
        }   
        else
        {
            rtl_setDoubleAliasIp(address.s_addr);
        }
    }
    else
    {
        rtl_setDoubleAlias(0);
        rtl_setDoubleAliasIp(0);
    }
    memcpy(alias2_wan_interface, waninterface, sizeof(alias2_wan_interface));
    //diag_printf("%s %d wan_interface=%s \n", __FUNCTION__, __LINE__, alias2_wan_interface);
    
    return ;
}
#endif




#if defined( HAVE_L2TP) || defined(HAVE_PPTP) || (defined(HAVE_PPPOE) && defined(CONFIG_RTL_DHCP_PPPOE))
struct dhcp_ppp_arg 
{
	int wan_type;
	int dns_mode;
	int connect_type;
	int op_mode;
	int wisp_wan_id;
	char wan_interface[16];
	char br_interface[16];
};

struct dhcp_ppp_arg DhcpPppArg;
void start_dhcp_ppp()
{	
	int wan_type;
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	
	//if(wan_type!=L2TP && wan_type!=PPTP && wan_type!=PPPOE)
	if(wan_type!=L2TP && wan_type!=PPTP)
		return;
	
	check_wan_ip();
	
#if 1
	if(lan_wan_ip_conflict_flag)
	{
		lan_wan_ip_conflict_flag=0;	
		RunSystemCmd(NULL_FILE, "ifconfig", DhcpPppArg.wan_interface, "down", NULL_STR);
		
		RunSystemCmd(NULL_FILE, "ifconfig", DhcpPppArg.wan_interface, "up", NULL_STR);
		return;
	}
#endif	

	if(DhcpPppArg.connect_type == CONNECT_ON_DEMAND)	//demand mode
		ppp_dail_on_demand(0);
	else if(DhcpPppArg.connect_type == CONTINUOUS) //continue mode
	{
		if(DhcpPppArg.wan_type == L2TP )
		{
			#ifdef HAVE_L2TP
			l2tp_disconnect();
			sleep(3);
			set_l2tp(DhcpPppArg.op_mode, DhcpPppArg.wan_interface, DhcpPppArg.br_interface, DhcpPppArg.wisp_wan_id);
			#endif
		}
		else if(DhcpPppArg.wan_type == PPTP)			
		{		
			#ifdef HAVE_PPTP
			pptp_disconnect();
			sleep(3);
			set_wanpptp(DhcpPppArg.op_mode, DhcpPppArg.wan_interface, DhcpPppArg.br_interface, DhcpPppArg.wisp_wan_id);
			#endif
		}		
		#if 0
		else if(DhcpPppArg.wan_type == PPPOE)			
		{		
			#if defined(HAVE_PPPOE) && defined(CONFIG_RTL_DHCP_PPPOE)
			pppoe_disconnect();
			sleep(3);
			set_wanpppoe(DhcpPppArg.op_mode, DhcpPppArg.wan_interface, DhcpPppArg.br_interface, DhcpPppArg.wisp_wan_id);
			#endif
		}		
		#endif
	}
	else if(DhcpPppArg.connect_type == MANUAL) //manual mode		 
		;
	
#ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS    
    rtl_checkdoubleAlias(DhcpPppArg.wan_type, DhcpPppArg.wan_interface);
#endif
}

#endif

#ifdef HAVE_SYSTEM_REINIT
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
extern int old_passThru_flag ;
extern char passThru_flag[2];
extern int passThruStatusWlan;

void start_custom_passthru_configure(void)
{
	int intValue=0;
	int sock;
	int op_mode=0;
	int repeater_enable1=0;
	int repeater_enable2=0;
	int wispWanId= -1;
	int wlan_mode;
	char brdg[] = "bridge0";
	
	apmib_get(MIB_OP_MODE, (void *)&op_mode);
	if((op_mode == GATEWAY_MODE )||(op_mode == WISP_MODE ))
	{
		apmib_get(MIB_CUSTOM_PASSTHRU_ENABLED, (void *)&intValue);
		//diag_printf("--------\nintValue:%d,[%s]:[%d].\n",intValue,__FUNCTION__,__LINE__);
		sock = socket(AF_INET, SOCK_DGRAM, 0);
		if(sock < 0){
			diag_printf("%s:socket failed!\n",__FUNCTION__);
			return ;
		}
		if(op_mode == GATEWAY_MODE )
		{
			if(is_interface_up("pwlan0"))
				interface_down("pwlan0");
			
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
#ifdef HAVE_BRIDGE
				bridge_add(sock,brdg,"peth0");
#endif
			}
		}
#ifndef HAVE_NOWIFI 
		else if(op_mode == WISP_MODE)
		{
			apmib_get(MIB_WISP_WAN_ID, (void *)&wispWanId);	
			apmib_get(MIB_REPEATER_ENABLED1,(void *)&repeater_enable1);
			apmib_get(MIB_REPEATER_ENABLED2,(void *)&repeater_enable2);
			apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
			
			if(is_interface_up("peth0"))
				interface_down("peth0");
			
#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) 
			if(intValue != 0)
			{
				char tmpStr[16];
				/*should also config wisp wlan index for dual band wireless interface*/
				intValue=((wispWanId&0xF)<<4)|intValue;
			#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT 
				if(repeater_enable1 || repeater_enable2)
				{
					intValue = intValue | 0x8;
				}
			#endif					
				passThruStatusWlan=intValue;	
				//diag_printf("---------wispWanId:%d,repeater_enable1:%d,repeater_enable2:%d,passThruStatusWlan:%x.\n",
				//	wispWanId,repeater_enable1,repeater_enable2,passThruStatusWlan);
				
			}
			else
			{
				passThruStatusWlan=intValue;
			}
#else
			if(intValue!=0){
			#ifdef CONFIG_RTL_REPEATER_MODE_SUPPORT 
				if((repeater_enable1||repeater_enable2)
					&& (wlan_mode == AP_MODE || wlan_mode == AP_WDS_MODE)){
					intValue |=0x08;
				}	
			#endif		
				passThruStatusWlan=intValue;
			}
			else
			{
				passThruStatusWlan=intValue;
			}					
#endif
			//diag_printf("--------------\npassThruStatusWlan:%x,[%s]:[%d].\n",passThruStatusWlan,__FUNCTION__,__LINE__);
			if (passThruStatusWlan == 0)
			{
				if(is_interface_up("pwlan0"))
					interface_down("pwlan0");
#ifdef HAVE_BRIDGE
				bridge_delete(sock,brdg,"pwlan0");
#endif
			}
			else 
			{
				//diag_printf("--------------\npassThruStatusWlan:%x,[%s]:[%d].\n",passThruStatusWlan,__FUNCTION__,__LINE__);
				//config_passThruInfo();
				interface_up("pwlan0");
#ifdef HAVE_BRIDGE
				bridge_add(sock,brdg,"pwlan0");
#endif
			}
		
		}
#endif
		close(sock);
	}
	
}
#endif
#endif

#ifdef CONFIG_RTL_DHCP_PPPOE
void start_wan_back(unsigned int flag)
#else
void start_wan_back()  //start_wan 转变为start_wan_back  ,//add by z10312  临时解决编译问题, 跟tenda app 里start_wan重名, 后续开发按需打开 -0105
#endif
{
	char *nat_interface;
	int op_mode=0, lan_type=0, wan_type=0, wisp_wan_id=0;
	unsigned char wan_interface[16], br_interface[16]="bridge0";
	unsigned char hosts[MAX_NAME_LEN];
	unsigned char eth_ifname[8]="eth1";
	int dns_mode;
	unsigned int pppoe_connect_type,l2tp_connect_type,pptp_connect_type;
	unsigned int connect_type;
	int wlan_mode;
	int l2tp_wanip_dynamic,pptp_wanip_dynamic, enable_server_domain;
	apmib_get(MIB_PPP_CONNECT_TYPE, (void *)&pppoe_connect_type);
	apmib_get(MIB_L2TP_CONNECTION_TYPE, (void *)&l2tp_connect_type);
	apmib_get(MIB_PPTP_CONNECTION_TYPE, (void *)&pptp_connect_type);
	
	printf("Init WAN Interface...\n");

	if(isFileExist("/etc/wan_info"))
		unlink("/etc/wan_info");


	apmib_get(MIB_OP_MODE, (void *)&op_mode);
	apmib_get(MIB_WAN_DHCP, (void *)&wan_type);
	apmib_get(MIB_WISP_WAN_ID,(void *)&wisp_wan_id);
	apmib_get(MIB_DNS_MODE, (void *)&dns_mode);

	#ifdef CONFIG_RTL_DHCP_PPPOE
	int pppoeWithDhcpEnabled = 0, pppoeDisabled=0, dhcpDisabled=0;
	apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
	//apmib_get(MIB_CWMP_PPPOE_DISABLED, (void *)&pppoeDisabled);
	apmib_get(MIB_CWMP_DHCP_DISABLED, (void *)&dhcpDisabled);	
	#endif

	backup_wanType = wan_type;
	#if defined(CONFIG_RTL_FREEBSD_FAST_PATH)	
	//open the fast skb forward!!!
	set_fast_skb_fw(1);	
	#endif
	
	#if defined(CONFIG_RTL_HARDWARE_NAT)
	rtl_hwNatOnOffByApp();
	#endif
	
	RunSystemCmd(NULL_FILE, "ipfw", "add", "10", "allow", "ip", "from", "any", "to", "any", NULL_STR);
	if(op_mode == GATEWAY_MODE){
		//sprintf(wan_interface, "%s", "eth1");	
		strcpy(wan_interface, eth_ifname);

#if ECOS_RTL_REPORT_LINK_STATUS
	int wan_port=4;	
#if defined(CONFIG_RTL_8881A) 
#if defined(CONFIG_RTL_8881AM) || defined(CONFIG_CUTE_MAHJONG)
	wan_port=get_cmj_xdg_wan_port();  
#endif
#endif
	//diag_printf("%s:%d ####wan_port=%d\n",__FUNCTION__,__LINE__,wan_port);
	set_wan_idx(wan_port); //WAN_PORT
#endif
	}
	else if(op_mode == WISP_MODE)
	{
		//for smart repeater
#ifdef CONFIG_WLANIDX_MUTEX
		int s = apmib_save_idx();
#else
		apmib_save_idx();
#endif
		apmib_set_wlanidx(wisp_wan_id);
		
		apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
		
		if(wlan_mode == CLIENT_MODE){
			sprintf(wan_interface, "wlan%d", wisp_wan_id);
#if ECOS_RTL_REPORT_LINK_STATUS
			if(wisp_wan_id)
				set_wan_idx(WLAN1_PORT);
			else
				set_wan_idx(WLAN0_PORT);
#endif
		}
		else{
			sprintf(wan_interface, "wlan%d-vxd0", wisp_wan_id);
#if ECOS_RTL_REPORT_LINK_STATUS
			if(wisp_wan_id)
				set_wan_idx(WLAN1_VXD_PORT);
			else
				set_wan_idx(WLAN0_VXD_PORT);
#endif
		}
#ifdef CONFIG_WLANIDX_MUTEX
		apmib_revert_idx(s);
#else
		apmib_revert_idx();
#endif
	}
	
#ifdef CONFIG_RTL_WANIP_SAME_WITH_GWIP_SUPPORT
	extern void rtl_set_is_dhcp_wantype(unsigned int value);
    if(wan_type == DHCP_CLIENT ||wan_type == STATIC_IP)
	{
		rtl_set_is_dhcp_wantype(1);
	}else{
		rtl_set_is_dhcp_wantype(0);
	}
#endif

	if(wan_type == STATIC_IP){
		set_staticIP(op_mode, wan_interface, br_interface, wisp_wan_id);		
	} 

	#if 0 //defined(CONFIG_RTL_QOS_RATE_LIMIT_CHECK)
	rtl_set_wan_if_name(op_mode, wlan_mode, wisp_wan_id);
	#endif
	
    #if defined(CONFIG_RTL_NETSNIPER_SUPPORT) //|| defined(CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT)
    rtl_get_wan_interface(op_mode, wlan_mode, wisp_wan_id);
    #endif
#ifdef HAVE_DHCPC
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == DHCP_CLIENT || wan_type == DHCP_PLUS)
    #else
    if(wan_type == DHCP_CLIENT)
    #endif
	{
		if ( apmib_get( MIB_HOST_NAME, (void *)&hosts)){
			if(hosts[0])
				write_line_to_file("/etc/hosts.conf", 1, hosts);
		}
		
#ifdef ECOS_DBG_STAT
		dbg_dhcpc_index=dbg_stat_register("dhcpc");
#endif
		
        #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
        if (wan_type == DHCP_CLIENT)            
		    wandhcpc_startup(wan_interface, dns_mode);
        else if (wan_type == DHCP_PLUS)
            handle_dhcpplus(wan_interface, dns_mode);
        #else		
		wandhcpc_startup(wan_interface, dns_mode);
        #endif
		set_dhcpWanInfMtu(wan_interface);
	}
#endif

	
#ifdef HAVE_PPPOE
#ifdef CONFIG_RTL_FAST_PPPOE
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 )
    #else
	if(wan_type == PPPOE)
    #endif
	{
		set_fast_pppoe_fw(1);	
	} else
	{
		set_fast_pppoe_fw(0);
	}
#endif

	if(wan_type == PPPOE)
	{	
#ifdef CONFIG_RTL_DHCP_PPPOE
	if(pppoeWithDhcpEnabled)
	{		
		cyg_thread_info tinfo;	
		#ifdef HAVE_DHCPC
		if(dhcpDisabled==0 && (!(flag&SYS_WAN_PPPOE_M) || get_thread_info_by_name("dhcpc eth1", &tinfo)==0))
			wandhcpc_startup(wan_interface, dns_mode);
		#endif				
	}
#endif


    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if((wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2) && pppoe_connect_type == 1)
    #else
	if(wan_type == PPPOE && pppoe_connect_type == 1)	//demand mode
    #endif
	{
		ppp_dail_on_demand(0);
	}
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    else if((wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2) && pppoe_connect_type == 0)
    #else
	else if(wan_type == PPPOE && pppoe_connect_type == 0) //continuous mode
    #endif
		set_wanpppoe(op_mode, wan_interface, br_interface, wisp_wan_id);
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    else if((wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2) && pppoe_connect_type == 2)
    #else
	else if(wan_type == PPPOE && pppoe_connect_type == 2) //manual mode
    #endif
		 ;	
	}
#endif

#ifdef HAVE_L2TP
	if(wan_type == L2TP)
	{	
#ifdef FAST_L2TP
		set_fast_l2tp_fw(1);
#endif

#if defined(CONFIG_DYNAMIC_WAN_IP)
		apmib_get(MIB_L2TP_WAN_IP_DYNAMIC, (void *)&l2tp_wanip_dynamic);
		if(l2tp_wanip_dynamic==L2TP_PPTP_STATIC_IP)
		{
			set_staticIP(op_mode, wan_interface, br_interface, wisp_wan_id);
			
			if(l2tp_connect_type == CONNECT_ON_DEMAND)	//demand mode
				ppp_dail_on_demand(0);
			else if(l2tp_connect_type == CONTINUOUS) //continue mode		
				set_l2tp(op_mode, wan_interface, br_interface, wisp_wan_id);
			else if(l2tp_connect_type == MANUAL) //manual mode
				;	
		}
		else //dhcp+l2tp
		{		
			DhcpPppArg.wan_type=wan_type;
			DhcpPppArg.dns_mode=dns_mode;
			DhcpPppArg.connect_type=l2tp_connect_type;
			DhcpPppArg.op_mode=op_mode;
			strcpy(DhcpPppArg.wan_interface, wan_interface);
			strcpy(DhcpPppArg.br_interface, br_interface);
			DhcpPppArg.wisp_wan_id=wisp_wan_id;
			
			#ifdef HAVE_DHCPC
			wandhcpc_startup(wan_interface, dns_mode);
			#endif			
		}		
#endif
	} else {
		#ifdef FAST_L2TP
			set_fast_l2tp_fw(0);
		#endif
	}
#endif


#ifdef HAVE_PPTP
	//default now ,disable pptp fast forward!!!
	if(wan_type == PPTP)
	{	
#ifdef FAST_PPTP
		set_fast_pptp_fw(1);
#endif

#if defined(CONFIG_DYNAMIC_WAN_IP)
		apmib_get(MIB_PPTP_WAN_IP_DYNAMIC, (void *)&pptp_wanip_dynamic);
		if(pptp_wanip_dynamic==L2TP_PPTP_STATIC_IP)
		{
			set_staticIP(op_mode, wan_interface, br_interface, wisp_wan_id);
			
			if(pptp_connect_type == CONNECT_ON_DEMAND)	//demand mode
				ppp_dail_on_demand(0);
			else if(pptp_connect_type == CONTINUOUS) //continue mode
				set_wanpptp(op_mode, wan_interface, br_interface, wisp_wan_id);	
			else if(pptp_connect_type == MANUAL) //manual mode
				 ;
		}
		else //dhcp+pptp
		{			
			DhcpPppArg.wan_type=wan_type;
			DhcpPppArg.dns_mode=dns_mode;
			DhcpPppArg.connect_type=pptp_connect_type;
			DhcpPppArg.op_mode=op_mode;
			strcpy(DhcpPppArg.wan_interface, wan_interface);
			strcpy(DhcpPppArg.br_interface, br_interface);
			DhcpPppArg.wisp_wan_id=wisp_wan_id;
			
		#ifdef HAVE_DHCPC
			wandhcpc_startup(wan_interface, dns_mode);
		#endif				
		}	
#endif	
	} else {
		#ifdef FAST_PPTP
			set_fast_pptp_fw(0);
		#endif
	}
#endif

    #ifdef CONFIG_RTL_SUPPORT_DOUBLE_ALIAS    
    rtl_checkdoubleAlias(wan_type, wan_interface);
    #endif
	/*
	else if(wan_type == DHCP_CLIENT)
		set_dhcp_client(sys_op, wan_iface, lan_iface, wisp_id, act_source);
	else if(wan_type == PPPOE){
		int sessid = 0;
		char cmdBuf[50],tmpBuff[30];
		memset(tmpBuff,0, sizeof(tmpBuff));
		apmib_get(MIB_PPP_SESSION_NUM, (void *)&sessid);
		apmib_get(MIB_PPP_SERVER_MAC,  (void *)tmpBuff);

		sprintf(cmdBuf,"flash clearppp %d %02x%02x%02x%02x%02x%02x",sessid,(unsigned char)tmpBuff[0],(unsigned char)tmpBuff[1],(unsigned char)tmpBuff[2],(unsigned char)tmpBuff[3],(unsigned char)tmpBuff[4],(unsigned char)tmpBuff[5]);
		system(cmdBuf);
		sleep(2);	// Wait util pppoe server reply PADT, then start pppoe dialing, otherwise pppoe server will reply PADS with PPPoE tags: Generic-Error.
		
		set_pppoe(sys_op, wan_iface, lan_iface, wisp_id, act_source);
	}
	apmib_get(MIB_DHCP,(void*)&lan_type);
	if(lan_type == DHCP_CLIENT)
	{//when set lan dhcp client,default route should get from lan dhcp server.
	//otherwise,DHCP offer pocket from dhcp server would be routed to wan(default gw),and client can't complete dhcp
		RunSystemCmd(NULL_FILE, "route", "del", "default", wan_iface, NULL_STR);
	}
	*/

//dhcp_getting_ip:

#if defined(HAVE_PPPOE) || defined(HAVE_L2TP)
    #ifdef CONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
    if(wan_type == PPPOE || wan_type == PPPOE_HENAN || wan_type == PPPOE_NANCHANG || wan_type == PPPOE_OTHER1 || wan_type == PPPOE_OTHER2 || wan_type== L2TP || wan_type== PPTP)
    #else
	if(wan_type == PPPOE  || wan_type== L2TP || wan_type== PPTP) 
    #endif
		nat_interface="ppp0";
	else
#endif	
		nat_interface=wan_interface;

#if HAVE_NATD
	create_natd_thread(nat_interface);	
#endif

#ifdef KLD_ENABLED
	/*Set wan port speed*/
    set_wanspeed(wan_interface, op_mode);
#endif

#ifdef HAVE_SYSTEM_REINIT
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	start_custom_passthru_configure( );
#endif
#endif
	
if(wan_type == STATIC_IP)
	kick_event(WAN_EVENT);
}

void ppp_shutdown()
{	
//	diag_printf("%s %d backup_wanType(%d)#######\n",__FUNCTION__,__LINE__,backup_wanType);
	switch(backup_wanType)
	{
#ifdef	HAVE_PPPOE
		case PPPOE:			
			pppoe_stop_link(0);
			break;
#endif
#ifdef HAVE_L2TP
		case L2TP:
			l2tp_stop_link();
			break;
#endif
#ifdef HAVE_PPTP
		case PPTP:
			pptp_stop_link();
			break;
#endif			
		default:
			break;
	}
}

#if 0 //def CONFIG_RTL_PHY_POWER_CTRL
void powerOnOffLanInf()
{
	int op_mode;
	apmib_get(MIB_OP_MODE,(void*)&op_mode);
	
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "off", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "off", NULL_STR);

#ifndef KLD_ENABLED
	int wlan_idx, wlan_mode;
	char wlan_ifname[16];
#endif

	if(op_mode!=WISP_MODE)
	{
#ifndef KLD_ENABLED
		if(op_mode==WISP_MODE)
		{		
			apmib_save_idx();
			for(wlan_idx=0; wlan_idx<NUM_WLAN_INTERFACE; wlan_idx++)
			{			
				apmib_set_wlanidx(wlan_idx);			
				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				if(wlan_mode!=CLIENT_MODE)
				{
					sprintf(wlan_ifname, "wlan%d", wlan_idx);
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_ifname, "down", NULL_STR);
				}
			}
			apmib_revert_idx();
		}	
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "down", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1", "down", NULL_STR);
		}
	}
	if(op_mode==BRIDGE_MODE)
		RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "off", NULL_STR);
	
	sleep(3);
	
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "0", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "1", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "2", "on", NULL_STR);
	RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "3", "on", NULL_STR);

	if(op_mode!=WISP_MODE)
	{
#ifndef KLD_ENABLED
		if(op_mode==WISP_MODE)
		{		
			apmib_save_idx();
			for(wlan_idx=0; wlan_idx<NUM_WLAN_INTERFACE; wlan_idx++)
			{			
				apmib_set_wlanidx(wlan_idx);			
				apmib_get(MIB_WLAN_MODE, (void *)&wlan_mode);
				if(wlan_mode!=CLIENT_MODE)
				{
					sprintf(wlan_ifname, "wlan%d", wlan_idx);
					RunSystemCmd(NULL_FILE, "ifconfig", wlan_ifname, "up", NULL_STR);
				}
			}
			apmib_revert_idx();
		}	
		else
#endif
		{
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan0", "up", NULL_STR);
			RunSystemCmd(NULL_FILE, "ifconfig", "wlan1", "up", NULL_STR);
		}
	}
	
	if(op_mode==BRIDGE_MODE)
		RunSystemCmd(NULL_FILE, "eth", "phypower", "port", "4", "on", NULL_STR);
}
#endif

int avoid_confliction_ip(char *wanIp, char *wanMask)
{	
	char line_buffer[100]={0};
	char *strtmp=NULL;
	char tmp1[64]={0};
	unsigned int tmp1Val;
	struct in_addr inIp, inMask, inGateway;
	struct in_addr myIp, myMask, mask;
	unsigned int inIpVal, inMaskVal, myIpVal, myMaskVal, maskVal;
	char tmpBufIP[64]={0}, tmpBufMask[64]={0};
	DHCP_T dhcp;
	DHCP_T wan_dhcp_type;
	int opmode = 0;
	apmib_get( MIB_DHCP, (void *)&dhcp);
	apmib_get( MIB_WAN_DHCP, (void *)&wan_dhcp_type);

	int pppoeWithDhcpEnabled = 0;
#ifdef CONFIG_RTL_DHCP_PPPOE	
	apmib_get(MIB_PPPOE_DHCP_ENABLED, (void *)&pppoeWithDhcpEnabled);
#endif

#ifdef HAVE_DHCPD	
	if(get_dhcpd_running_state() || dhcp == DHCP_SERVER){

	}else
#endif	
	{
		return 0; //no dhcpd or dhcp server is disable
	}

	//diag_printf("wanip %s  wanmask %s\n",wanIp,wanMask);	
	
	if ( !inet_aton(wanIp, &inIp) ) {
		diag_printf("\r\n Invalid IP-address value!__[%s-%u]\r\n",__FILE__,__LINE__);
		return 0;
	}
	
	if ( !inet_aton(wanMask, &inMask) ) {
		diag_printf("\r\n Invalid IP-address value!__[%s-%u]\r\n",__FILE__,__LINE__);
		return 0;
	}

	//wan ip and netmask
	memcpy(&inIpVal, &inIp, 4);
	memcpy(&inMaskVal, &inMask, 4);

	getInAddr("eth0", IP_ADDR, (void *)&myIp );	
	getInAddr("eth0", SUBNET_MASK, (void *)&myMask );
		
	//lan ip and netmask
	memcpy(&myIpVal, &myIp, 4);
	memcpy(&myMaskVal, &myMask, 4);

	//diag_printf("\r\n inIpVal=[0x%x],__[%s-%u]\r\n",inIpVal,__FILE__,__LINE__);
	//diag_printf("\r\n inMaskVal=[0x%x],__[%s-%u]\r\n",inMaskVal,__FILE__,__LINE__);
	//diag_printf("\r\n myIpVal=[0x%x],__[%s-%u]\r\n",myIpVal,__FILE__,__LINE__);
	//diag_printf("\r\n myMaskVal=[0x%x],__[%s-%u]\r\n",myMaskVal,__FILE__,__LINE__);

	memcpy(&maskVal,htonl(myMaskVal)>htonl(inMaskVal)?&inMaskVal:&myMaskVal,4);
	
	//diag_printf("\r\n maskVal=[0x%x],__[%s-%u]\r\n",maskVal,__FILE__,__LINE__);
	
	if((inIpVal & maskVal) == (myIpVal & maskVal)) //wan ip conflict lan ip 
	{
		lan_wan_ip_conflict_flag=1;
		happen_ip_conflict_flag=1;
		int i=0, j=0;
		diag_printf("\r\n wan ip conflict lan ip!,__[%s-%u]\r\n",__FILE__,__LINE__);	
		for(i=0; i<32; i++)
		{
			//if((maskVal & htonl(1<<i)) != 0)
			if((maskVal & htonl(1<<i)) != 0)
				break;
		}
		
		if((myIpVal & htonl(1<<i)) == 0)
		{
			myIpVal = myIpVal | htonl(1<<i);
		}
		else
		{
			myIpVal = myIpVal & htonl(~(1<<i));
		}
		
		memcpy(&myIp, &myIpVal, 4);
				
						
		for(j=0; j<32; j++)
		{
			if((myMaskVal & htonl(1<<j)) != 0)
				break;
		}

#ifdef IPCONFLICT_UPDATE_FIREWALL
		set_conflict_ip_mask(myIpVal,myMaskVal);
#endif

#if 1
			struct in_addr start_addr, end_addr;

////		system("killall -9 udhcpd 2> /dev/null");
////		system("rm -f /var/run/udhcpd.pid 2> /dev/null");
////		system("rm -f /var/udhcpd.conf");
		
////		sprintf(line_buffer,"interface %s\n","br0");
////		write_line_to_file(DHCPD_CONF_FILE, 1, line_buffer);
	
		apmib_get(MIB_DHCP_CLIENT_START,  (void *)tmp1);		
		if((*(unsigned int*)tmp1 & myMaskVal) != (myIpVal & myMaskVal))
			*(unsigned int*)tmp1=(myIpVal & myMaskVal) | (*(unsigned int*)tmp1 & (~(myMaskVal)));
		
	//	memcpy(tmp1, &myIpVal,  j);
////		*(unsigned int*)tmp1 ^= (1<<(j));
////		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
////		diag_printf("\r\n start ip=[%s],__[%s-%u]\r\n",strtmp,__FILE__,__LINE__);		
////		sprintf(line_buffer,"start %s\n",strtmp);
////		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		memcpy(&start_addr.s_addr, (unsigned int*)tmp1, 4);
//		diag_printf("%s:%d startip=%s\n",__FUNCTION__,__LINE__,inet_ntoa(start_addr));
		
		apmib_get(MIB_DHCP_CLIENT_END,  (void *)tmp1);		
		if((*(unsigned int*)tmp1 & myMaskVal) != (myIpVal & myMaskVal))
			*(unsigned int*)tmp1=(myIpVal & myMaskVal) | (*(unsigned int*)tmp1 & (~(myMaskVal)));
		
		//memcpy(tmp1, &myIpVal,  j);
////		*(unsigned int*)tmp1 ^= (1<<(j));
		memcpy(&end_addr.s_addr, (unsigned int*)tmp1, 4);		
//		diag_printf("%s:%d endip=%s\n",__FUNCTION__,__LINE__,inet_ntoa(end_addr));

////		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
////		diag_printf("\r\n end ip=[%s],__[%s-%u]\r\n",strtmp,__FILE__,__LINE__);		
////		sprintf(line_buffer,"end %s\n",strtmp);
////		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
	
////		diag_printf("\r\n subnet mask=[%s],__[%s-%u]\r\n",inet_ntoa(myMask),__FILE__,__LINE__);			
////		sprintf(line_buffer,"opt subnet %s\n",inet_ntoa(myMask));
////		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

////		diag_printf("\r\n gateway ip=[%s],__[%s-%u]\r\n",inet_ntoa(myIp),__FILE__,__LINE__);					
////		sprintf(line_buffer,"opt router %s\n",inet_ntoa(myIp));
////		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

////		diag_printf("\r\n dns ip=[%s],__[%s-%u]\r\n",inet_ntoa(myIp),__FILE__,__LINE__);							
////		sprintf(line_buffer,"opt dns %s\n",inet_ntoa(myIp)); /*now strtmp is ip address value */
////		write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);
		
////		memset(tmp1,0x00,sizeof(tmp1));
////		apmib_get( MIB_DOMAIN_NAME, (void *)&tmp1);
////		if(tmp1[0]){
////			sprintf(line_buffer,"opt domain %s\n",tmp1);
////			write_line_to_file(DHCPD_CONF_FILE, 2, line_buffer);

			create_dhcpd_configfile((struct in_addr*)&myIpVal, &start_addr, &end_addr);
						
//		}
#endif
		memset(tmp1,0x00,sizeof(tmp1));
		memcpy(tmp1, &myIpVal,  4);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(tmpBufIP,"%s",strtmp);
		//diag_printf("\r\n tmpBufIP=[%s],__[%s-%u]\r\n",tmpBufIP,__FILE__,__LINE__);

		memset(tmp1,0x00,sizeof(tmp1));
		memcpy(tmp1, &myMaskVal,  4);
		strtmp= inet_ntoa(*((struct in_addr *)tmp1));
		sprintf(tmpBufMask,"%s",strtmp);
		//diag_printf("\r\n tmpBufMask=[%s],__[%s-%u]\r\n",tmpBufMask,__FILE__,__LINE__);

		/* avoid wisp-vxd connect remote ap, and ip conflict */		
		apmib_get(MIB_OP_MODE,(void *)&opmode);
		if(opmode==WISP_MODE)
			sleep(5);		

		RunSystemCmd(NULL_FILE, "ifconfig", "eth0", tmpBufIP, "netmask", tmpBufMask, NULL_STR);
		//diag_printf("\r\n line_buffer=[%s],__[%s-%u]\r\n",line_buffer,__FILE__,__LINE__);									

#ifdef IPCONFLICT_UPDATE_FIREWALL
        create_staticip_file();
#endif
//		setDefaultGateway(0);

#ifdef HAVE_DHCPD
		dhcpd_restart(2);	
#endif

#if 0
#ifdef CONFIG_RTL_PHY_POWER_CTRL
		extern void powerOnOffLanInf();
		extern void powerOnOffWlanInf();
		powerOnOffLanInf();
		powerOnOffWlanInf();
#else
		sleep(2);
#endif
#endif

#ifdef HAVE_DHCPC
		if(!(wan_dhcp_type==L2TP || wan_dhcp_type==PPTP || (wan_dhcp_type==PPPOE && pppoeWithDhcpEnabled)))
			dhcpc_reconnect(1);
		else
		{
			if(wanif_flag==0)
				kick_event(START_DHCP_PPP_EVENT);
		}
#endif

#if 0
		sprintf(line_buffer, "udhcpd %s", DHCPD_CONF_FILE);
		system(line_buffer);
		//start_dnrd();
#endif		
////		return 1;
	}
#ifdef IPCONFLICT_UPDATE_FIREWALL
	else        
	set_conflict_ip_mask(myIpVal,myMaskVal);
#endif

	return 0;
}


/*Called from wan connect*/
void check_wan_ip()
{
	char intfname[32];
	u_int32_t ip,mask,gateway,dns1,dns2;
	unsigned char tmpIP[16];
	unsigned char tmpMask[16];	
	
	if(!isFileExist("/etc/wan_info"))
		return;	
	lan_wan_ip_conflict_flag=0;
	
	memset(intfname,0,sizeof(intfname));
	/*Get Wan info*/
	get_WanInfo("/etc/wan_info",intfname,&ip, &mask, &gateway, &dns1, &dns2);
	/*avoid conflict ip*/

	if(strncmp(intfname, "ppp", 3)==0)
		wanif_flag=1;
	
	//diag_printf("%s %d intfname %s ip(0x%x) mask(0x%x) gateway(0x%x) dns1(0x%x) dns2(0x%x)\n",__FUNCTION__,__LINE__,intfname,ip,mask,gateway,dns1,dns2);
	inet_ntoa_r(*(struct in_addr *)&ip,tmpIP);
	inet_ntoa_r(*(struct in_addr *)&mask,tmpMask);
	avoid_confliction_ip(tmpIP,tmpMask);
	return;
}

