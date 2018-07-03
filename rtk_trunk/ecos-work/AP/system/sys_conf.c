#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/kernel/kapi.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"
#endif
#include "sys_utility.h"
#include "net_api.h"
#include "apmib.h"
#include "hw_settings.h"
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
#include "mini_upnp_global.h"
#include "../wsc/src/wsc.h"
#endif
#endif

extern int set_mac_address(const char *interface, char *mac_address);

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#ifdef HAVE_WPS
extern CTX_T wsc_context;

#if	0	//def HAVE_APMIB
int wsc_context_init(void)
{
	int intVal, intVal2, is_client, is_config, is_registrar, is_wep=0, wep_key_type=0, wep_transmit_key=0;
	char tmpbuf[100], tmp1[100];
	CTX_Tp pCtx;


	/*for detial mixed mode info*/ 
	int wlan0_encrypt=0;		
	int wlan0_wpa_cipher=0;
	int wlan0_wpa2_cipher=0;
	/*for detial mixed mode info	mixed issue*/ 
	
	//WPS
	memset(&wsc_context, 0, sizeof(CTX_T));
	pCtx = &wsc_context;
	/*--------------------------------------------------------------------*/
	pCtx->start = 1;
	strcpy(pCtx->wlan_interface_name, "wlan0");
	//strcpy(pCtx->lan_interface_name, "bridge0");
	strcpy(pCtx->lan_interface_name, "eth0");
#ifdef DEBUG
	pCtx->debug = 1;
#endif

	apmib_get(MIB_WLAN_MODE, (void *)&is_client);
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_config);
	apmib_get(MIB_WLAN_WSC_REGISTRAR_ENABLED, (void *)&is_registrar);	

	SC_DEBUG("wlan mode=%d\n",is_client);
	if (is_client == CLIENT_MODE) {
		if (is_registrar) {
			if (!is_config)
				intVal = MODE_CLIENT_UNCONFIG_REGISTRAR;
			else
				intVal = MODE_CLIENT_CONFIG;			
		}
		else
			intVal = MODE_CLIENT_UNCONFIG;
	}
	else {
		if (!is_config)
			intVal = MODE_AP_UNCONFIG;
		else
			intVal = MODE_AP_PROXY_REGISTRAR;
	}
	pCtx->mode = intVal;
	SC_DEBUG("wsc mode=%d\n",pCtx->mode);
	
	if (is_client == CLIENT_MODE)
		intVal = 0;
	else
		apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&intVal);
	pCtx->upnp = intVal;

#if 0	// wps 2.0
	// 9864
	pCtx->config_method = (CONFIG_METHOD_VIRTUAL_PIN |  CONFIG_METHOD_PHYSICAL_PBC | CONFIG_METHOD_VIRTUAL_PBC );
#else	// wps 1.0 
	intVal = 0;
	apmib_get(MIB_WLAN_WSC_METHOD, (void *)&intVal);
	//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	if (intVal == 1) //Pin+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
	else if (intVal == 2) //PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
	if (intVal == 3) //Pin+PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	pCtx->config_method = intVal;
#endif	

	/*assigned auth support list 2.0*/
	pCtx->auth_type_flags = (WSC_AUTH_OPEN | WSC_AUTH_WPAPSK |  WSC_AUTH_WPA2PSK);
	SC_DEBUG("auth_type_flags:0x%02X\n",pCtx->auth_type_flags);			

	/*assigned Encry support list 2.0*/
	pCtx->encrypt_type_flags = 
		(WSC_ENCRYPT_NONE | WSC_ENCRYPT_TKIP | WSC_ENCRYPT_AES );
	SC_DEBUG("encrypt_type_flags:0x%02X\n",pCtx->encrypt_type_flags);				
	
	apmib_get(MIB_WLAN_WSC_AUTH, (void *)&intVal2);
	pCtx->auth_type_flash = intVal2;

	apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
	pCtx->encrypt_type_flash = intVal;
	if (intVal == WSC_ENCRYPT_WEP)
		is_wep = 1;

	if (is_client) {
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
		if (intVal == 0)
			intVal = 1;
		else
			intVal = 2;
	}
	else
		intVal = 1;
	pCtx->connect_type = intVal;

	apmib_get(MIB_WLAN_WSC_MANUAL_ENABLED, (void *)&intVal);
	pCtx->manual_config = intVal;

	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&intVal);
		if (intVal != ENCRYPT_WEP) {
			printf("WEP mismatched between WPS and host system\n");
			return -1;
		}
		apmib_get(MIB_WLAN_WEP, (void *)&intVal);
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			return -1;
		}
		apmib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_transmit_key++;
		pCtx->wep_transmit_key = wep_transmit_key;
		if (intVal == WEP64) {
			apmib_get(MIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}
			strcpy(pCtx->network_key, tmp1);

			apmib_get(MIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}
			strcpy((char *)pCtx->wep_key2, tmp1);

			apmib_get(MIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}
			strcpy((char *)pCtx->wep_key3, tmp1);

			apmib_get(MIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}
			strcpy((char *)pCtx->wep_key4, tmp1);
		}
		else {
			apmib_get(MIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			strcpy(pCtx->network_key, tmp1);

			apmib_get(MIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			strcpy((char *)pCtx->wep_key2, tmp1);

			apmib_get(MIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			strcpy((char *)pCtx->wep_key3, tmp1);

			apmib_get(MIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str((unsigned char *)tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			strcpy((char *)pCtx->wep_key4, tmp1);
		}
	}
	else {
		apmib_get(MIB_WLAN_WPA_PSK, (void *)&tmp1);
		strcpy(pCtx->network_key, tmp1);
	}
	pCtx->network_key_len = strlen(pCtx->network_key);
	
	apmib_get(MIB_WLAN_SSID, (void *)&tmp1);	
	strcpy(pCtx->SSID, tmp1);

	apmib_get(MIB_HW_WSC_PIN, (void *)&tmp1);
	strcpy(pCtx->pin_code, tmp1);

	apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	pCtx->rf_band = intVal;

	/*
	apmib_get(MIB_HW_MODEL_NUM, (void *)&tmp1);
	strcpy(pCtx->model_num, tmp1);

	apmib_get(MIB_HW_SERIAL_NUM, (void *)&tmp1);
	strcpy(pCtx->serial_num, tmp1);
	*/
	apmib_get(MIB_DEVICE_NAME, (void *)&tmp1);
	strcpy(pCtx->device_name, tmp1);

	apmib_get(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&intVal);
	pCtx->config_by_ext_reg = intVal;


#if 0
	/*for detial mixed mode info*/ 
	//  mixed issue	
	apmib_get(MIB_WLAN_ENCRYPT, (void *)&wlan0_encrypt);
	apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&wlan0_wpa_cipher);
	apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&wlan0_wpa2_cipher);

	intVal=0;	
	if(wlan0_encrypt==6){	// mixed mode
		if(wlan0_wpa_cipher==1){
			intVal |= WSC_WPA_TKIP;
		}else if(wlan0_wpa_cipher==2){
			intVal |= WSC_WPA_AES;		
		}else if(wlan0_wpa_cipher==3){
			intVal |= (WSC_WPA_TKIP | WSC_WPA_AES);		
		}
		if(wlan0_wpa2_cipher==1){
			intVal |= WSC_WPA2_TKIP;
		}else if(wlan0_wpa2_cipher==2){
			intVal |= WSC_WPA2_AES;		
		}else if(wlan0_wpa2_cipher==3){
			intVal |= (WSC_WPA2_TKIP | WSC_WPA2_AES);		
		}		
	}
	pCtx->mixedmode= intVal;	
	/*for detial mixed mode info*/ 
#endif
	
	return 0;
}
#endif
#endif

#ifndef HAVE_APMIB
void my_wlan_settings(void)
{
	char *ifname = "wlan0";
#ifdef HAVE_WPS
	CTX_Tp pCtx;
#endif
	
	//RunSystemCmd(NULL_FILE, "ifconfig", ifname, "down", NULL_STR);
	
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "opmode=16", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "ssid=ecos-ap", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "channel=11", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "band=11", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "ampdu=1", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "use40M=1", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "2ndchoffset=1", NULL_STR); // 1:below, 2:above
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "shortGI20M=1", NULL_STR); 
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "shortGI40M=1", NULL_STR);
	
#if 0
	// No security
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "encmode=0", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "psk_enable=0", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "802_1x=0", NULL_STR);
#endif
#if 0
	// WPA-PSK, TKIP
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "encmode=2", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "psk_enable=1", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wpa_cipher=2", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "passphrase=1234567890", NULL_STR);
#endif
#if 0
	// WPA2-PSK, AES
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "encmode=4", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "psk_enable=2", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wpa2_cipher=8", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "passphrase=1234567890", NULL_STR);
#endif
#if 0
	// WEP40
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "encmode=1", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "authtype=2", NULL_STR);	// auth: auto
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "802_1x=0", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepdkeyid=0", NULL_STR);	
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey1=1111111111", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey2=2222222222", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey3=3333333333", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey4=4444444444", NULL_STR);	
#endif
#if 0
	// WEP128
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "encmode=5", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "authtype=2", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "802_1x=0", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepdkeyid=0", NULL_STR);		
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey1=11111111111111111111111111", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey2=22222222222222222222222222", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey3=33333333333333333333333333", NULL_STR);
	RunSystemCmd(NULL_FILE, "iwpriv", ifname, "set_mib", "wepkey4=44444444444444444444444444", NULL_STR);		
#endif
	//RunSystemCmd(NULL_FILE, "ifconfig", ifname, "up", NULL_STR);

#ifdef HAVE_WPS
	//WPS
	memset(&wsc_context, 0, sizeof(CTX_T));
	pCtx = &wsc_context;
	//--------------------------------------------------------------------
	pCtx->start = 1;
	strcpy(pCtx->wlan_interface_name, "wlan0");
	//strcpy(pCtx->lan_interface_name, "bridge0");
	strcpy(pCtx->lan_interface_name, "eth0");
#ifdef DEBUG
	pCtx->debug = 1;
#endif
	//--------------------------------------------------------------------
	pCtx->mode = MODE_AP_UNCONFIG;
	//pCtx->mode = MODE_AP_PROXY_REGISTRAR;
	pCtx->upnp = 1;	
	// Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	pCtx->config_method = CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC;
	pCtx->connect_type = CONNECT_TYPE_BSS;
	pCtx->manual_config = 0;
	
	//auth_type
	pCtx->auth_type_flash = WSC_AUTH_OPEN;
	//pCtx->auth_type_flash = WSC_AUTH_WPAPSK;
	//pCtx->auth_type_flash = WSC_AUTH_WPA2PSK;
	//pCtx->auth_type_flash = WSC_AUTH_WPA2PSKMIXED;
	
	//encrypt_type
	pCtx->encrypt_type_flash = WSC_ENCRYPT_NONE;
	//pCtx->encrypt_type_flash = WSC_ENCRYPT_WEP;
	//pCtx->encrypt_type_flash = WSC_ENCRYPT_TKIP;
	//pCtx->encrypt_type_flash = WSC_ENCRYPT_AES;
	//pCtx->encrypt_type_flash = WSC_ENCRYPT_TKIPAES;
	
	strcpy(pCtx->network_key, "");
	//strcpy(pCtx->network_key, "1234567890");
	//pCtx->wep_transmit_key = 1;
	//WEP64
	//strcpy(pCtx->network_key, "1111111111");
	//strcpy(pCtx->wep_key2, "2222222222");
	//strcpy(pCtx->wep_key3, "3333333333");
	//strcpy(pCtx->wep_key4, "4444444444");
	//WEP128
	//strcpy(pCtx->network_key, "11111111111111111111111111");
	//strcpy(pCtx->wep_key2, "22222222222222222222222222");
	//strcpy(pCtx->wep_key3, "33333333333333333333333333");
	//strcpy(pCtx->wep_key4, "44444444444444444444444444");
	pCtx->network_key_len = strlen(pCtx->network_key);
	strcpy(pCtx->SSID, "ecos-ap");
	pCtx->rf_band = 1;
	//pCtx->rf_band = 2; //if channel > 14
	strcpy(pCtx->device_name, "Realtek Wireless AP");
	pCtx->config_by_ext_reg = 0;
	//--------------------------------------------------------------------
#endif
}
#endif

#endif

/*
void init_lan_address(void)
{
	struct sockaddr_in *addrp;
	struct ifreq ifr;
#ifdef HAVE_APMIB
	struct in_addr addr, mask;
#endif
	struct in_addr gwaddr;
	struct ecos_rtentry route;
	char addrstr[16], maskstr[16];
	int s=-1;

#ifdef HAVE_APMIB
	apmib_get(MIB_IP_ADDR,  (void *)&addr);
	strcpy(addrstr, inet_ntoa(addr));
	apmib_get(MIB_SUBNET_MASK,  (void *)&mask);
	strcpy(maskstr, inet_ntoa(mask));
	apmib_get(MIB_DEFAULT_GATEWAY,  (void *)&gwaddr);
#else
	strcpy(addrstr, "192.168.1.254");
	strcpy(maskstr, "255.255.255.0");
	gwaddr.s_addr = 0;
#endif

	interface_config("eth0", addrstr, maskstr);
	
	s = socket(AF_INET, SOCK_DGRAM, 0);
	if (s < 0) {
		perror("socket");
		goto out;
	}
	strcpy(ifr.ifr_name, "eth0");
	addrp = (struct sockaddr_in *) &ifr.ifr_addr;
	memset(addrp, 0, sizeof(*addrp));
	memset(&route, 0, sizeof(route));
	addrp->sin_family = AF_INET;
	addrp->sin_port = 0;
	addrp->sin_len = sizeof(*addrp);
	addrp->sin_addr.s_addr = 0; // Use 0,0,GATEWAY for the default route
	memcpy(&route.rt_dst, addrp, sizeof(*addrp));
	addrp->sin_addr.s_addr = 0;
	memcpy(&route.rt_genmask, addrp, sizeof(*addrp));
	addrp->sin_addr = gwaddr;
	memcpy(&route.rt_gateway, addrp, sizeof(*addrp));

	route.rt_dev = ifr.ifr_name;
	route.rt_flags = RTF_UP|RTF_GATEWAY;
	route.rt_metric = 0;
	//delete default route first
	ioctl(s, SIOCDELRT, &route);

	if ( 0 != gwaddr.s_addr ) {
	    //add default route now
            if (ioctl(s, SIOCADDRT, &route)) {
		printf("Route - dst: %s",
		inet_ntoa(((struct sockaddr_in *)&route.rt_dst)->sin_addr));
		printf(", mask: %s",
		inet_ntoa(((struct sockaddr_in *)&route.rt_genmask)->sin_addr));
		printf(", gateway: %s\n",
		inet_ntoa(((struct sockaddr_in *)&route.rt_gateway)->sin_addr));
                if (errno != EEXIST) {
                    perror("SIOCADDRT 3");
                }
            }
	}
out:
	if (s != -1)
		close(s);
}
*/
