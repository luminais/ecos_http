/*
* mtenda.c for bcm ecos, 2010/08/25 roy
*/

/********************************* Includes ***********************************/

#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include <net/if_var.h>

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/param.h>
#include <net/if_dl.h>
#include <net/radix.h>
#include <net/route.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <sys/sockio.h>
#include <autoconf.h>

#include  "uemf.h"
#include  "wsIntrn.h"
#include  "route_cfg.h"
#include  "flash_cgi.h"
#include  "../ddns/include/ddns.h"
#include  "chlan.h"
#include  "rc.h"
#include  "cJSON.h"
#ifdef __CONFIG_IPV6__
#include  <network6.h>
#endif
#include  "arp_clients.h"

#include "systools.h"
#include "http.h"
#include "sys_module.h"
#include "bcmupnp.h"
#include "dhcp_server.h"
#include "wan.h"
#include "pi_common.h"
#include "bcmsntp.h"
#include "wifi.h"
#include "firewall.h"
#include "reboot_check.h"
#include "wan_mode_check.h"
#include "bcmddns.h"

#ifdef __CONFIG_IPV6__
static void formGetIPV6Info(webs_t wp, char_t *path, char_t *query)
{
	char value[256] = {0};
	char *tmp_value = NULL;
	int con_type = 0;
	websWrite(wp, T("HTTP/1.1 200 OK\nContent-type: text/plain; charset=utf-8\nPragma: no-cache\nCache-Control: no-cache\n\n"));
	websWrite(wp, "{");

	_GET_VALUE("ipv6_enable", tmp_value);
	websWrite(wp, "\"ipv6En\":\"%s\",", atoi(tmp_value)? "true" : "false");

	/* WAN */
	websWrite(wp, "\"ipv6Wan\":{");
	_GET_VALUE("wan6_con_type", tmp_value);
	con_type = atoi(tmp_value);
	switch(con_type)
	{
		case 0:
			websWrite(wp, "\"ipv6WanType\":\"%s\",", "dhcp");
			break;
		case 1:
			websWrite(wp, "\"ipv6WanType\":\"%s\",", "static");
			break;
		case 2:
			websWrite(wp, "\"ipv6WanType\":\"%s\",", "pppoe");
			break;
		default:
			websWrite(wp, "\"ipv6WanType\":\"%s\",", "");
			break;
	}


	memset(value, 0x0, sizeof(value));
	websWrite(wp, "\"ipv6WanPPPoEUser\":\"%s\",", changeStr2CJSON(nvram_safe_get("wan6_pppoe_username"), value));

	memset(value, 0x0, sizeof(value));
	websWrite(wp, "\"ipv6WanPPPoEPwd\":\"%s\",", changeStr2CJSON(nvram_safe_get("wan6_pppoe_passwd"), value));

	websWrite(wp, "\"ipv6WanIP\":\"%s\",", nvram_safe_get("wan6_static_addr"));
	websWrite(wp, "\"ipv6WanGateway\":\"%s\",", nvram_safe_get("wan6_static_gateway"));
	websWrite(wp, "\"ipv6WanDns1\":\"%s\",", nvram_safe_get("wan6_static_pri_dns"));
	websWrite(wp, "\"ipv6WanDns2\":\"%s\",", nvram_safe_get("wan6_static_sec_dns"));

	if(0 == con_type) //dhcp
	{
		_GET_VALUE("wan6_dhcp_delegation", tmp_value);
		websWrite(wp, "\"ipv6WayDelegation\":\"%s\",", atoi(tmp_value)? "true" : "false");

		_GET_VALUE("wan6_dhcp_nontemporary", tmp_value);
		websWrite(wp, "\"ipv6WayTemporary\":\"%s\"", atoi(tmp_value)? "true" : "false");

	}
	else if(2 == con_type)//pppoe
	{
		_GET_VALUE("wan6_pppoe_delegation", tmp_value);
		websWrite(wp, "\"ipv6WayDelegation\":\"%s\",", atoi(tmp_value)? "true" : "false");

		_GET_VALUE("wan6_pppoe_nontemporary", tmp_value);
		websWrite(wp, "\"ipv6WayTemporary\":\"%s\"", atoi(tmp_value)? "true" : "false");
	}
	else
	{
		websWrite(wp, "\"ipv6WayDelegation\":\"%s\",", "true");
		websWrite(wp, "\"ipv6WayTemporary\":\"%s\"", "false");
	}
	websWrite(wp, "},");

	/* LAN */
	websWrite(wp, "\"ipv6Lan\":{");
	_GET_VALUE("lan6_type", tmp_value);
	websWrite(wp, "\"ipv6LanType\":\"%s\",", atoi(tmp_value)? "manual" : "auto");

	_GET_VALUE("lan6_dhcp_prefix_mode", tmp_value);
	websWrite(wp, "\"ipv6LanDhcpPrefixType\":\"%s\",", atoi(tmp_value)? "manual" : "auto");

	websWrite(wp, "\"ipv6LanDhcpPrefix\":\"%s\",", nvram_safe_get("lan6_dhcp_prefix"));

	_GET_VALUE("lan6_dhcp_enable", tmp_value);
	websWrite(wp, "\"ipv6LanDhcpEn\":\"%s\",", atoi(tmp_value)? "true" : "false");

	_GET_VALUE("lan6_dhcp_ifid_mode", tmp_value);
	websWrite(wp, "\"ipv6LanDhcpType\":\"%s\",", atoi(tmp_value)? "manual" : "auto");

	websWrite(wp, "\"ipv6LanDhcpStartID\":\"%s\",", nvram_safe_get("lan6_dhcp_start_id"));
	websWrite(wp, "\"ipv6LanDhcpEndID\":\"%s\",", nvram_safe_get("lan6_dhcp_end_id"));

	_GET_VALUE("lan6_dhcp_dns_mode", tmp_value);
	websWrite(wp, "\"ipv6LanDnsType\":\"%s\",", atoi(tmp_value)? "manual" : "auto");

	websWrite(wp, "\"ipv6LanDns1\":\"%s\",", nvram_safe_get("lan6_dhcp_pri_dns"));
	websWrite(wp, "\"ipv6LanDns2\":\"%s\",", nvram_safe_get("lan6_dhcp_sec_dns"));
	websWrite(wp, "\"ipv6LanIP\":\"%s\"", nvram_safe_get("lan6_ipaddr"));
	websWrite(wp, "},");

	/* STATUS */
	websWrite(wp, "\"statusIPv6\":{");
	DHCP6C_STATUS *ipv6_wan_status = NULL;
	ipv6_wan_status = (DHCP6C_STATUS *)malloc(sizeof(DHCP6C_STATUS));
	if(ipv6_wan_status == NULL){
		printf("malloc DHCP6C_STATUS failed!\n");
		return ;
	}
	memset(ipv6_wan_status, 0, sizeof(DHCP6C_STATUS));
	get_status_v6(ipv6_wan_status);
	websWrite(wp, "\"statusIPv6\":\"%s\",", strlen(ipv6_wan_status->ipv6_addr)? "true" : "false");
	switch(atoi(nvram_safe_get("wan6_con_type")))
	{
		case 0:
			websWrite(wp, "\"statusIPv6WanType\":\"%s\",", "dhcp");
			break;
		case 1:
			websWrite(wp, "\"statusIPv6WanType\":\"%s\",", "static");
			break;
		case 2:
			websWrite(wp, "\"statusIPv6WanType\":\"%s\",", "pppoe");
			break;
		default:
			break;
	}
	websWrite(wp, "\"statusIPv6WanIP\":\"%s\",", ipv6_wan_status->ipv6_addr);
	websWrite(wp, "\"statusIPv6WanGateway\":\"%s\",", ipv6_wan_status->gateway_v6);
	websWrite(wp, "\"statusIPv6LanIP\":\"%s\",", ipv6_wan_status->lan6_addr);
	websWrite(wp, "\"statusIPv6Dns1\":\"%s\",",  ipv6_wan_status->pri_dns);
	websWrite(wp, "\"statusIPv6Dns2\":\"%s\"",  ipv6_wan_status->sec_dns);
	websWrite(wp, "}");

	free(ipv6_wan_status);
	websWrite(wp, "}\n");
	websDone(wp, 200);

	return ;
}

void formLanIpv6Set(webs_t wp, char_t *path, char_t *query)
{
	char *dhcp6s_enable,*dhcp6s_mode,*d6ns_mode,*pri_dns,*sec_dns,*start_id,*end_id,
		 *lan_ip6_addr,*ipv6_dhcp_prefix_mode,*ipv6_dhcp_prefix,*ipv6_lan_type;

	dhcp6s_enable = websGetVar(wp, T("ipv6LanDhcpEn"), T("0"));
	ipv6_lan_type = websGetVar(wp, T("ipv6LanType"), T("0"));
	lan_ip6_addr = websGetVar(wp, T("ipv6LanIP"), T(""));
	ipv6_dhcp_prefix_mode = websGetVar(wp, T("ipv6LanDhcpPrefixType"), T(""));
	ipv6_dhcp_prefix = websGetVar(wp, T("ipv6LanDhcpPrefix"), T(""));
	dhcp6s_mode = websGetVar(wp, T("ipv6LanDhcpType"), T("0"));
	d6ns_mode = websGetVar(wp, T("ipv6LanDnsType"), T(""));
	pri_dns = websGetVar(wp, T("ipv6LanDns1"), T(""));
	sec_dns = websGetVar(wp, T("ipv6LanDns2"), T(""));
	start_id = websGetVar(wp, T("ipv6LanDhcpStartID"), T(""));
	end_id = websGetVar(wp, T("ipv6LanDhcpEndID"), T(""));

	_SET_VALUE("lan6_dhcp_enable", !strcmp(dhcp6s_enable, "true") ? "1" : "0");
	_SET_VALUE("lan6_type", !strcmp(ipv6_lan_type, "manual") ? "1" : "0");
	_SET_VALUE("lan6_ipaddr", lan_ip6_addr);
	_SET_VALUE("lan6_dhcp_ifid_mode", !strcmp(dhcp6s_mode, "manual") ? "1" : "0");
	_SET_VALUE("lan6_dhcp_prefix_mode", !strcmp(ipv6_dhcp_prefix_mode, "manual") ? "1" : "0");
	if(strlen(ipv6_dhcp_prefix) > 2)// ::
	{
		_SET_VALUE("lan6_dhcp_prefix", ipv6_dhcp_prefix);
		ipv6_dhcp_prefix[strlen(ipv6_dhcp_prefix) - 2] = '\0';
	}
	_SET_VALUE("lan6_dhcp_dns_mode", !strcmp(d6ns_mode, "manual") ? "1" : "0");
	_SET_VALUE("lan6_dhcp_pri_dns", pri_dns);
	_SET_VALUE("lan6_dhcp_sec_dns", sec_dns);

	_SET_VALUE("lan6_dhcp_start_id", start_id);
	_SET_VALUE("lan6_dhcp_end_id", end_id);

	return;
}

void formWanStatic6Set(webs_t wp, char_t *path, char_t *query)
{
	char *wan_ip6_addr, *wan_ipv6_gateway, *wan_pri_dns, *wan_sec_dns;

	wan_ip6_addr = websGetVar(wp, T("ipv6WanIP"), T(""));
	wan_ipv6_gateway = websGetVar(wp, T("ipv6WanGateway"), T(""));
	wan_pri_dns = websGetVar(wp, T("ipv6WanDns1"), T(""));
	wan_sec_dns = websGetVar(wp, T("ipv6WanDns2"), T(""));

	_SET_VALUE("wan6_con_type", "1"); //IPv6 连接方式  0:DHCP  1:static  2: pppoe
	_SET_VALUE("wan6_static_addr", wan_ip6_addr);
	_SET_VALUE("wan6_static_gateway", wan_ipv6_gateway);
	_SET_VALUE("wan6_static_pri_dns", wan_pri_dns);
	_SET_VALUE("wan6_static_sec_dns", wan_sec_dns);

	return;
}

void formWanDHCP6Set(webs_t wp, char_t *path, char_t *query)
{
	char *delegation, *temporary;

	delegation = websGetVar(wp, T("ipv6WayDelegation"), T(""));
	temporary = websGetVar(wp, T("ipv6WayTemporary"), T(""));

	_SET_VALUE("wan6_con_type", "0"); //IPv6 连接方式  0:DHCP  1:static  2: pppoe
	_SET_VALUE("wan6_dhcp_delegation", !strcmp(delegation, "true") ? "1" : "0");
	_SET_VALUE("wan6_dhcp_nontemporary", !strcmp(temporary, "true") ? "1" : "0");

	return;
}

void formWanPPPoE6Set(webs_t wp, char_t *path, char_t *query)
{
	char *ppoe_user,*pppoe_passwd, *delegation, *temporary;
	ppoe_user = websGetVar(wp, T("ipv6WanPPPoEUser"), T(""));
	pppoe_passwd = websGetVar(wp, T("ipv6WanPPPoEPwd"), T(""));
	delegation = websGetVar(wp, T("ipv6WayDelegation"), T(""));
	temporary = websGetVar(wp, T("ipv6WayTemporary"), T(""));

	_SET_VALUE("wan6_con_type", "2"); //IPv6 连接方式  0:DHCP  1:static  2: pppoe
	_SET_VALUE("wan6_pppoe_username",ppoe_user);
	_SET_VALUE("wan6_pppoe_passwd",pppoe_passwd);
	_SET_VALUE("wan6_pppoe_delegation", !strcmp(delegation, "true") ? "1" : "0");
	_SET_VALUE("wan6_pppoe_nontemporary", !strcmp(temporary, "true") ? "1" : "0");

	/*ipv4和ipv6 pppoe用户名密码保持一致，巴西MTL02定制*/
	_SET_VALUE(_WAN0_PPPOE_USERNAME,ppoe_user);
	_SET_VALUE(_WAN0_PPPOE_PASSWD,pppoe_passwd);

	return;
}

static void formSetIpv6Info(webs_t wp, char_t *path, char_t *query)
{
	char *ipv6_enable,*ipv6_con_type;

	ipv6_enable = websGetVar(wp, T("ipv6En"), T(""));
	ipv6_con_type = websGetVar(wp, T("ipv6WanType"), T(""));

	if(!strcmp(ipv6_enable, "true"))
	{
		_SET_VALUE("ipv6_enable", "1");
		formLanIpv6Set(wp, path, query);

		if(!strcmp(ipv6_con_type, "static"))
			formWanStatic6Set(wp, path, query);
		else if(!strcmp(ipv6_con_type, "dhcp"))
			formWanDHCP6Set(wp, path, query);
		else if(!strcmp(ipv6_con_type, "pppoe"))
			formWanPPPoE6Set(wp, path, query);
	}
	else
		_SET_VALUE("ipv6_enable", "0");

	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	websWrite(wp, T("%s"), "{\"errCode\":\"100\"}");
	websDone(wp, 200);

	_COMMIT();
	sys_reboot();
}
#endif

