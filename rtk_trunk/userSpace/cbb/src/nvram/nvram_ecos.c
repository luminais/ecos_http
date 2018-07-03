/*
 * Config data profile utility.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: nvram_ecos.c,v 1.8 2010-10-18 11:48:10 Exp $
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <typedefs.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <osl.h>
///#include <bcmutils.h> //add by z10312 tenda_app_test
///#include <siutils.h>
#include <epivers.h>
#include <router_version.h>
#include <bcmnvram.h>
#include <bcmdevs.h>
#include <bcmendian.h>
#include <flashutl.h>
#include <shutils.h>
#include <netconf.h>
#include <bcmparams.h>
#include <nvparse.h>
#include <wlioctl.h>
#include <wlutils.h>
#include <ezc.h>
#include "wifi.h"
///#include "sys_backupcfg.h" //add by z10312 tenda_app_test


#define cfg_log     diag_printf
#ifdef __ECOS
#define	isdigit(x)	((x) >= '0' && (x) <= '9')
#endif

//roy+++,2010/09/26
#ifndef NVRAM_MAX_STRINGSIZE
#define NVRAM_MAX_STRINGSIZE 256 //from bcmcvar.h
#endif
//+++

#define CFG_INIT_LOCK()
#define CFG_UNLOCK()  /* cyg_scheduler_unlock() */
#define CFG_LOCK()    /* cyg_scheduler_lock() */

#ifdef	CONFIG_NETBOOT
#define RESTORE_DEFAULTS() \
	!nvram_match("restore_defaults", "0")
#else
	#define RESTORE_DEFAULTS() \
		(!nvram_match("restore_defaults", "0") || \
		nvram_invmatch("os_name", "linux") || \
		nvram_invmatch("ecos_name", "ecos"))
#endif//#ifdef	CONFIG_NETBOOT

#define	WL_PREFIX "wl0_"
#define	WLXdotY_PREFIX "wl0.1_"

extern struct nvram_tuple router_defaults[];
//extern void set_pin_code(void);
extern void tapf_board_reboot(void);
extern uint32 wps_gen_pin(char *devPwd, int devPwd_len);


int nvram_erase(void);

/*static*///roy remove,2010/09/26
int
find_nvram(char *var, bool full_match)
{
	struct nvram_tuple *t;
	int len = strlen(var);
	int found = 0;
	int i;
	char *c;
	char tmp[100], prefix[] = "wanXXXXXXXXXX_";

	if (len == 0)
		return -1;

	if (!strncmp(var, "wan_", 4) && strcmp(var,"wan_ifnames") != 0)/*we need wan_ifnames*/
		goto done;

	/* check full name match first */
	for (t = router_defaults; t->name; t++) {
		if (!strcmp(var, t->name)) {
			found = 1;
			break;
		}
	}

//roy +++,2010/11/23
	if(!found){
			if(!strncmp(var,"wl0_",4) ||!strncmp(var,"wl0.1_",6) ){
				found = 1;
			}
	}
//过滤掉一些内容,不让客户看到
	if(found){
		if(/*strstr(var,"ifnames") || strstr(var,"ifname") || */strstr(var,"te_version") || 
			strstr(var,"wme") || strstr(var,"rtylimit") || strstr(var,"radarthrs") )
			found = 0;
	}
//+++

	if (full_match)
		goto done;
//这下面不会执行,因为我们传入的full_match为真
	/* check partial name match */
	if (found == 0) {
		/* Need to check wlX_YYY and wlX.Z_YYY case */
		for (t = router_defaults; t->name; t++) {
			if (!strncmp(t->name, "wl_", 3) && !strncmp(var, "wl", 2)) {
				/* wlX.Z_YYY */
				if (strstr(var, ".")) {
					if (!strcmp(&t->name[3], var+strlen(WLXdotY_PREFIX))) {
						found = 1;
						break;
					}
				}
				/* wlX_YYY */
				else {
					if (!strcmp(&t->name[3], var+strlen(WL_PREFIX))) {
						found = 1;
						break;
					}
				}
			}

			/* We need to take special care of the wan_ variables for multiple
			 * WAN configuration immigrated from the Linux platform.
			 */
			if (!strncmp(t->name, "wan_", 4) && !strncmp(var, "wan", 3)) {
				for (i = 3; i < len; i++) {
					c = var + i;
					if (*c < '0' || *c > '9')
						break;
				}
				if (*(var+i) != '_')
					break;
				strncpy(prefix, var, i + 1);
				prefix[i+1] = 0;
				if (strcmp(var + i + 1, "primary") != 0) {
					if (!nvram_match(strcat_r(prefix, "primary", tmp), "1"))
						break;
					if (!strcmp(var + i + 1, "ipaddr") ||
					    !strcmp(var + i + 1, "netmask") ||
					    !strcmp(var + i + 1, "gateway") ||
					    !strcmp(var + i + 1, "dns") ||
					    !strcmp(var + i + 1, "wins") ||
					    !strcmp(var + i + 1, "domain")) {
						if (nvram_match(strcat_r(prefix, "proto", tmp), "dhcp") ||
						    nvram_match(strcat_r(prefix, "proto", tmp), "pppoe"))
							break;
					}
				}
				else {
					/* We assume the first _primary variable will be set to 0 */
					if (!nvram_get(strcat_r(prefix, "primary", tmp))) {
						break;
					}
				}

				if (!strcmp(&t->name[3], var + i)) {
					found = 1;
					break;
				}
			}
		}
	}
done:
	return found;
}


/*根据传入接口和MAC地址计算相应SSID，例如：tenda_123456*/
void set_ssid_mac(char* wl_prefix){
	char wlanmac_key[64]={'\0'};
	char wlanmac_value[64]={'\0'};
	char wlanssid_key[64]={'\0'};
	char prev_ssid[64]={'\0'};
	char ssid_mac[64]={'\0'};
	
	int i = 0, l = 0;

	if(NULL == wl_prefix)
	{
		printf("wl_prefix is NULL!\n");
		return ;
	}
	/*规格规定：2.4G Tenda_MAC（eth0）后6位*/
	if(0 == strncmp(wl_prefix,WL_24G,strlen(WL_24G)))
	{
		strcpy(wlanmac_key,"et0macaddr");
	}
	else
	{
		strcpy(wlanmac_key,wl_prefix);
		strcat(wlanmac_key,"_hwaddr");
		
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
		if(1 == atoi(nvram_safe_get("wl_doubleBandUn_enable")))
		{
			/*双频优选默认开启的情况下，恢复出厂设置才将5G的ssid设置与2.4G的一样*/
			memset(wlanmac_key,0x0,sizeof(wlanmac_key));
			strcpy(wlanmac_key,"et0macaddr");
		}
#endif
	}
	
	if(nvram_get(wlanmac_key)){
		strcpy(wlanmac_value,nvram_get(wlanmac_key));
	}
	else{
		cfg_log("macaddr is empty!!!\n");
		return;
	}

	strcpy(wlanssid_key,wl_prefix);
	strcat(wlanssid_key,"_ssid");
	if(nvram_get(wlanssid_key)){
		strcpy(prev_ssid, nvram_get(wlanssid_key));
	}else{
		cfg_log("wl_ssid is empty!!!\n");
		return;
	}
	for(i=0;  wlanmac_value[i]!='\0' && i<9; ++i){
		if (wlanmac_value[i+9] != ':'){		
				wlanmac_value[l]=wlanmac_value[i+9];
				++l;
			}
	}
	wlanmac_value[6]='\0';
	if(0 == strncmp(wl_prefix,WL_5G,strlen(WL_5G)))
	{
		snprintf(ssid_mac, sizeof(ssid_mac),"%s_%s_5G", prev_ssid, wlanmac_value);
		
#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
		/*add by lrl 2018/4/9 双频优选功能 2.4G 和 5G ssid一样*/		
		if(1 == atoi(nvram_safe_get("wl_doubleBandUn_enable")))
		{
			memset(ssid_mac,0x0,sizeof(ssid_mac));
			snprintf(ssid_mac, sizeof(ssid_mac),"%s_%s", prev_ssid, wlanmac_value);
		}	
#endif
	}
	else
	{
		snprintf(ssid_mac, sizeof(ssid_mac),"%s_%s", prev_ssid, wlanmac_value);
	}
	printf("====wlanssid_key:%s====ssid_mac:%s=======%s [%d]\n",wlanssid_key,ssid_mac, __FUNCTION__, __LINE__);
	nvram_set(wlanssid_key, ssid_mac);
	
	return;
}

#ifdef __CONFIG_AUTO_CONN__
int
mac_add(char *macaddr, char *result, unsigned int add_num)
{
	int i = 0;
	unsigned char mac_ocet[6] = {0};
	unsigned int mac_value = 0;

	if(macaddr == NULL || result == NULL)
		return 0;
	
	for (;;) {
		mac_ocet[i++] = (char) strtoul(macaddr, &macaddr, 16);
		if (!*macaddr++ || i == 6)
			break;
	}

	if(i == 6)
	{
		mac_value = (mac_ocet[3]&0XFF);
		mac_value = (mac_value<<8) + (mac_ocet[4]&0XFF);
		mac_value = (mac_value<<8) + (mac_ocet[5]&0XFF);

		mac_value += add_num;
		sprintf(result, "%02X:%02X:%02X:%02X:%02X:%02X", mac_ocet[0], mac_ocet[1], mac_ocet[2],
			(mac_value>>16)&0XFF, (mac_value>>8)&0XFF, (mac_value)&0XFF);

		return 1;
	}

	return 0;
}
#endif
/*lq 根据主接口的地址修改此接口的地址*/
void set_minor_mac(char*major_prefix,char* minor_sprefix,unsigned int add_num)
{
	char major_mac[64]={0};
	char minor_mac[64]={0};
	char major_key[64]={'\0'};
	char major_value[64]={'\0'};
	char minor_key[64]={'\0'};

	if(NULL == major_prefix || NULL == minor_sprefix)
	{
		printf("wl_major_prefix or wl_minor_sprefix is NULL\n");
		return ;	
	}
	
	strcpy(major_key,major_prefix);
	strcat(major_key,"_hwaddr");
	
	if(nvram_get(major_key)){
		strcpy(major_mac,nvram_get(major_key));
		mac_add(major_mac, minor_mac, add_num);
		strcpy(minor_key,minor_sprefix);
		strcat(minor_key,"_hwaddr");
		nvram_set(minor_key, minor_mac);
	}
	else{
		cfg_log("macaddr is empty!!!\n");
		return;
	}
	
}
void set_wan0_mac(void)
{
	char vlan2mac[64]={0};
	
	if(nvram_get("et0macaddr")){
		strcpy(vlan2mac,nvram_get("et0macaddr"));
	}
	else{
		cfg_log("macaddr is empty!!!\n");
		return;
	}

	#ifdef __CONFIG_AUTO_CONN__
	char new_mac[64] = {0};
	mac_add(vlan2mac, new_mac, 4);
	strcpy(vlan2mac, new_mac);
	#endif
	
	nvram_set("wan0_hwaddr", vlan2mac);
	
	return;
}

#ifdef __CONFIG_ALINKGW__
#define ALILINK_WLAN_TPSKLIST_MAX	128
#endif

#ifdef __CONFIG_NAT__
#endif /* __CONFIG_NAT__ */


/* Get all configuration */
int
CFG_read_prof(char * file, int max)
{
	char *p;
	int len;
	int num = 0;
	nvram_getall(file, max,1);
	for (p = file; *p; p += len+1) {
		len = strlen(p);
		num++;
	}

	return (int)(p-file) + num;
}


/* Write all configuration to NV */
int
CFG_write_prof(char * file, int size)
{
	char line[2*NVRAM_MAX_STRINGSIZE], *sp, *ep;
	char *var, *val;
	int rc = 0;
	int len = 0;
	int num = 0;
	int crc = 0;
	int cal_num = 0;
	char* temp =NULL;	
	int count = 0;
	int i = 0;
	
	for (sp = ep = file; (*sp && sp < file+size); sp++) {

		if(*sp == '\r' && *(sp+1) == '\n' )
		{
			*sp='\0';
			sp++;
		}
		else
			continue;
		
		
		len = strlen(ep);
		if (len > (2*NVRAM_MAX_STRINGSIZE -1) || len <3)
			break;
		
		if(*ep == '\n'){//*sp=='\r'
			ep++;
			len--;
			sp++;//now *sp='\n'
		}
		
		memcpy(line, ep/*sp*/, len);
		line[len] = 0;

		sp++;
		if(!*sp)
			break;
		
		/*new line start*/
		ep = sp;

		var = line;
		/* skip comment */
		if (*var == '#')
			continue;
		/*lq 获取校验值*/
		if (*var == '@')
		{
			crc = atoi((var+1));
			printf("crc:%d\n",crc);
			continue;
		}
		
		temp= line;
		count = 0;
		i = 0;
		while(i < strlen(line))
		{
			cal_num += *(temp+i);
			i++;
		}

		/* found the '=' */
		val = strchr((const char*)var, '=');
		if(!val)
			break;
		/* cut the var name */
		*val = 0;
		val++;
		
		//if (find_nvram(var, true)) {
			nvram_set(var, val);
			num++;
		//}
	}
	/*lq 如果配置文件被修改过，则不进行导入操作*/
	
	if(cal_num == crc)
	{	
		nvram_commit();
	}
	else
	{
		rc = -1;
	}
	
	if (rc)
		cfg_log("CFG_write_prof err %x\n", rc);
	else
		cfg_log("CFG_write_prof %d items\n", num);

	return rc;
}



