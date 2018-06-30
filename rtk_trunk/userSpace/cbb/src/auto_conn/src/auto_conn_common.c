#include <stdlib.h>
#include <sys/param.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <iflib.h>
#include <ecos_oslib.h>
#include  "auto_conn_common.h"
#include  "router_net.h"
#ifdef __CONFIG_AUTO_CONN_CLIENT__
int auto_conn_extend_status = -1; 
//record auto-conn status
#endif
int connect_24g_flag = 1;//2.4G connect flag
int device_name_24G = 0; 
int device_name_5G = 0;


#ifdef __CONFIG_AUTO_CONN_SERVER__
int tenda_auto_conn_vif_extender = -1; 			//record auto-conn status
#endif
char auto_conn_extend_prev_mac[24];				//record prev connect AP MAC
int auto_conn_extend_rssi;							//record legal rssi
//int auto_conn_finish = 0;					//record auto-conn is done, restart system and disable dhcpd
int AutoConnDebugLevel = AUTO_CONN_DEBUG_OFF;	//record auto-conn debug level,test
//int dhcpc_br0_restart = 0;
//int auto_conn_getting_ip = 0;				//record auto-conn is getting ip

static int led_blink_start = 0;					//record led start blink
static int bridge_led_on_status = 0;			//used for led blink
extern int wl_link_status();

void auto_conn_debug_level(int level)
{
	if (level > 0)
	{
		AutoConnDebugLevel = AUTO_CONN_DEBUG_ERR;
	}
	else
	{
		AutoConnDebugLevel = AUTO_CONN_DEBUG_OFF;
	}

	//diag_printf("auto_conn debug level = %d\n", AutoConnDebugLevel);
	printf("auto_conn debug level = %d\n", AutoConnDebugLevel);
}
#ifdef __CONFIG_AUTO_CONN_CLIENT__
int extend_get_status()
{
	return auto_conn_extend_status;
}

int extend_is_undo_status()
{
	if (auto_conn_extend_status == AUTO_CONN_VIF_EXTEND_UNDO)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int extend_is_setdef_status()
{
	if (auto_conn_extend_status == AUTO_CONN_VIF_EXTEND_SETDEF)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int extend_is_doing_status()
{
	if (auto_conn_extend_status == AUTO_CONN_VIF_EXTEND_DOING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int extend_is_done_status()
{
	if (auto_conn_extend_status == AUTO_CONN_VIF_EXTEND_DONE)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

extern int no_exist_count;
void extend_set_undo_status()
{
	auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_UNDO;
	// è®¾ç½®ä¸ºundoçŠ¶æ€åŽï¼Œä¸å­˜åœ¨æœåŠ¡å™¨çš„è®¡æ•°ä¹Ÿé‡æ–°å¼€å§?	no_exist_count = 0;
}

void extend_set_setdef_status()
{
	auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_SETDEF;
}

void extend_set_doing_status()
{
	auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_DOING;
}

void extend_set_done_status()
{
	auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_DONE;
}
#endif
extern struct ip_set primary_lan_ip_set;

void auto_conn_led_blink_handler()
{
	if (bridge_led_on_status == 0)
	{
		bridge_led_on_status = 1;
		//set_bridge_led(1);	//need fix

	}
	else
	{
		bridge_led_on_status = 0;
		//set_bridge_led(0);	//need fix
	}

	if (extend_is_done_status() && primary_lan_ip_set.up == 1)
	{
		led_blink_start = 0;
		return;
	}

	timeout((timeout_fun *)&auto_conn_led_blink_handler, 0, 50);
}

int is_led_blink()
{
	return led_blink_start;
}
#ifdef __CONFIG_AUTO_CONN_CLIENT__
void auto_conn_extend_status_init()
{
	char *rssi = NULL;

	if (nvram_match("product_finish", "1"))
	{
		if (strcmp(nvram_recv_safe_get("wl0_ssid"), "Tenda_Extender_1") == 0)
		{
			auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_UNDO;
			printf("###func=%s, switch status to AUTO_START_VIF_AP_UNDO---\n", __FUNCTION__);
		}
		else
		{
			auto_conn_extend_status = AUTO_CONN_VIF_EXTEND_DONE;
			printf("###func=%s, switch status to AUTO_START_VIF_AP_DONE---\n", __FUNCTION__);
		}
	}

	rssi = nvram_recv_safe_get("auto_conn_extend_rssi");
	auto_conn_extend_rssi = atoi(rssi);

	if (auto_conn_extend_rssi <= 0 || auto_conn_extend_rssi > 50)
	{
		auto_conn_extend_rssi = -20;
	}
	else
	{
		auto_conn_extend_rssi = -auto_conn_extend_rssi;
	}

	printf("###func=%s, set auto-conn rssi to %d\n", __FUNCTION__, auto_conn_extend_rssi);

}
#endif

void
tenda_prepare_key(uint8 *key_data, int key_data_len, rc4_ks_t *ks)
{
	unsigned int counter, index1 = 0, index2 = 0;
	uint8 key_byte, temp;
	uint8 *key_state = ks->state;

	for (counter = 0; counter < RC4_STATE_NBYTES; counter++)
	{
		key_state[counter] = (uint8) counter;
	}

	for (counter = 0; counter < RC4_STATE_NBYTES; counter++)
	{
		key_byte = key_data[index1];
		index2 = (key_byte + key_state[counter] + index2) % RC4_STATE_NBYTES;
		temp = key_state[counter];
		key_state[counter] = key_state[index2];
		key_state[index2] = temp;
		index1 = (index1 + 1) % key_data_len;
	}

	ks->x = 0;
	ks->y = 0;
}

/* encrypt or decrypt using RC4 */
void
tenda_rc4(uint8 *buf, int data_len, rc4_ks_t *ks)
{
	uint8 tmp;
	uint8 xor_ind, x = ks->x, y = ks->y, *key_state = ks->state;
	int i;

	for (i = 0; i < data_len; i++)
	{
		y += key_state[++x]; /* mod RC4_STATE_NBYTES */
		tmp = key_state[x];
		key_state[x] = key_state[y];
		key_state[y] = tmp;
		xor_ind = key_state[x] + key_state[y];
		buf[i] ^= key_state[xor_ind];
	}

	ks->x = x;
	ks->y = y;
}

int crypt_decrypt_rc4(unsigned char *inbuf, int len, struct ether_header *eh)
{
	rc4_ks_t key;
	unsigned char *outbuf = NULL;
	char key_str[KEY_STR_LEN + 1] = {0};

	if (!inbuf || len < 0)
	{
		printf("%s : Bad parameter.\n", __FUNCTION__);
		return -1;
	}

	outbuf = (char *)malloc(len + 1);

	if (NULL == outbuf )
	{
		printf("%s : No memory.\n", __FUNCTION__);
		return -1;
	}

#if 1
	memcpy(outbuf, inbuf, len);
	snprintf(key_str, KEY_STR_LEN, "Tenda_%02x%02x%02x%02x%02x%02x",
	         eh->ether_dhost[0] & 0xff, eh->ether_dhost[1] & 0xff, eh->ether_dhost[2] & 0xff,
	         eh->ether_dhost[3] & 0xff, eh->ether_dhost[4] & 0xff, eh->ether_dhost[5] & 0xff);
#else
	memcpy(outbuf, inbuf, len);
	memcpy(key_str, "123456789012", KEY_STR_LEN);
#endif

	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###key_str=%s, before encrypt:%02x\n", key_str, *inbuf);
	tenda_prepare_key(key_str, KEY_STR_LEN, &key);
	tenda_rc4(outbuf, len, &key);

	memcpy(inbuf, outbuf, len);
	AUTO_CONN_DBGPRINT(AUTO_CONN_DEBUG_ERR, "###after encrypt:%02x\n", *inbuf);

	free(outbuf);
	return len;
}
void set_entype_none(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "");
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G) 
	{
		nvram_set(WLAN5G_REPEATER_WEP, "disabled");
		nvram_set(WLAN5G_REPEATER_AKM, "");
		nvram_set(WLAN5G_REPEATER_CRYPTO, "");
	}

}
void set_entype_wpa_aes(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "aes");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN5G_REPEATER_WEP, "disabled");
		nvram_set(WLAN5G_REPEATER_AKM, "psk");
		nvram_set(WLAN5G_REPEATER_CRYPTO, "aes");
		nvram_set(WLAN5G_REPEATER_PASSWD, config_5G_info->passwd);
	}
}
void set_entype_wpa_tkip(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN5G_REPEATER_WEP, "disabled");
		nvram_set(WLAN5G_REPEATER_AKM, "psk");
		nvram_set(WLAN5G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN5G_REPEATER_PASSWD, config_5G_info->passwd);
	}
}
void set_entype_wpa2_aes(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN5G_REPEATER_WEP, "disabled");
		nvram_set(WLAN5G_REPEATER_AKM, "psk");
		nvram_set(WLAN5G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN5G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN5G_REPEATER_WEP, "disabled");
		nvram_set(WLAN5G_REPEATER_AKM, "psk2");
		nvram_set(WLAN5G_REPEATER_CRYPTO, "aes");
		nvram_set(WLAN5G_REPEATER_PASSWD, config_5G_info->passwd);
	}
}
void set_entype_wpa2_tkip(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_5G_info->passwd);
	}
}
void set_entype_wpa_wpa2_aes(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "aes");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "aes");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_5G_info->passwd);
	}
}
void set_entype_wpa_wpa2_tkip(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)	
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
		
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_5G_info->passwd);
	
	}
}
void set_entype_wpa_wpa2_aes_tkip(struct transmit_wifi_info *copy_wifi_info)
{
	if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)
	{
		struct transmit_wifi_info *config_24G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip+aes");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_24G_info->passwd);
	}
	if(device_name_5G == AUTO_CONN_VIF_5G && device_name_24G != AUTO_CONN_VIF_24G)
	{
		struct transmit_wifi_info *config_5G_info = copy_wifi_info;
		nvram_set(WLAN24G_REPEATER_WEP, "disabled");
		nvram_set(WLAN24G_REPEATER_AKM, "psk2");
		nvram_set(WLAN24G_REPEATER_CRYPTO, "tkip+aes");
		nvram_set(WLAN24G_REPEATER_PASSWD, config_5G_info->passwd);
	
	}
}
PASSWORDTYPE set_passtype[] =
{
	{NONE, set_entype_none},
	{WPA_AES, set_entype_wpa_aes},
	{WPA_TKIP, set_entype_wpa_tkip},
	{WPA2_AES, set_entype_wpa2_aes},
	{WPA2_TKIP, set_entype_wpa2_tkip},
	{WPA_WPA2_AES, set_entype_wpa_wpa2_aes},
	{WPA_WPA2_TKIP, set_entype_wpa_wpa2_tkip},
	{WPA_WPA2_AES_TKIP, set_entype_wpa_wpa2_aes_tkip},
};

void copy_config_24G(unsigned char *recv_data)
{
	struct transmit_wifi_info *config_24G_info;
	config_24G_info = (struct transmit_wifi_info *)recv_data;
	nvram_set(WLAN24G_REPEATER_SSID, config_24G_info->ssid);
	int type_index = 0;
	for(type_index = 0;type_index < 8;type_index++)
	{
		if(ntohl(config_24G_info->enTpye) == set_passtype[type_index].pass_index)
		{
			SELECT_TYPE(set_passtype[type_index])(config_24G_info);
		}
	}
}
void copy_config_5G(unsigned char *recv_data)
{
	struct transmit_wifi_info *config_5G_info;
	config_5G_info = (struct transmit_wifi_info *)recv_data;
	nvram_set(WLAN5G_REPEATER_SSID, config_5G_info->ssid);
	int type_index = 0;
	for(type_index = 0;type_index < 8;type_index++)
	{
		if(ntohl(config_5G_info->enTpye) == set_passtype[type_index].pass_index)
		{
			SELECT_TYPE(set_passtype[type_index])(config_5G_info);
		}
	}
	
}


#ifdef __CONFIG_AUTO_CONN_SERVER__
int get_auto_status()
{
	return tenda_auto_conn_vif_extender;
}

int router_is_undo_status()
{
	if (tenda_auto_conn_vif_extender == AUTO_CONN_VIF_ROUTER_UNDO)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int router_is_init_status()
{
	if (tenda_auto_conn_vif_extender == AUTO_CONN_VIF_ROUTER_INIT)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int router_is_doing_status()
{
	if (tenda_auto_conn_vif_extender == AUTO_CONN_VIF_ROUTER_DOING)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int router_set_auto_status(int status)
{
	if (status < AUTO_CONN_VIF_ROUTER_UNDO || status > AUTO_CONN_VIF_ROUTER_DOING)
	{
		printf("set status fail!\n");
		return 0;
	}

	tenda_auto_conn_vif_extender = status;
	return 1;
}

void router_set_undo_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_ROUTER_UNDO;
}

void router_set_init_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_ROUTER_INIT;
}

void router_set_doing_status()
{
	tenda_auto_conn_vif_extender = AUTO_CONN_VIF_ROUTER_DOING;
}

void unset_va1_default()
{
	nvram_unset(WLAN24G_VIRTUAL_ENABLE);
	nvram_unset(WLAN24G_VIRTUAL_SSID);
	nvram_unset(WLAN24G_VIRTUAL_WEP);
	nvram_unset(WLAN24G_VIRTUAL_AKM);
	nvram_unset(WLAN24G_VIRTUAL_CRYPTO);
	nvram_unset(WLAN24G_VIRTUAL_PASSWD);
	nvram_unset(WLAN24G_VIRTUAL_HIDE_SSID);

	nvram_unset(WLAN5G_VIRTUAL_ENABLE);
	nvram_unset(WLAN5G_VIRTUAL_SSID);
	nvram_unset(WLAN5G_VIRTUAL_WEP);
	nvram_unset(WLAN5G_VIRTUAL_AKM);
	nvram_unset(WLAN5G_VIRTUAL_CRYPTO);
	nvram_unset(WLAN5G_VIRTUAL_PASSWD);
	nvram_unset(WLAN5G_VIRTUAL_HIDE_SSID);
	device_name_24G = 0;
	device_name_5G = 0;
	
}
void auto_conn_vif_control(unsigned int on)
{
	if (on)
	{
		char msg_tmp[PI_BUFLEN_64] = {0};
		if(device_name_24G == AUTO_CONN_VIF_24G && device_name_5G != AUTO_CONN_VIF_5G)
		{
			nvram_set(WLAN24G_VIRTUAL_ENABLE, "1");
			nvram_set(WLAN24G_VIRTUAL_SSID, "Tenda_Extender_1");
			nvram_set(WLAN24G_VIRTUAL_WEP, "disabled");
			nvram_set(WLAN24G_VIRTUAL_AKM, "psk2");
			nvram_set(WLAN24G_VIRTUAL_CRYPTO, "aes");
			nvram_set(WLAN24G_VIRTUAL_PASSWD, "Extender123456");
			nvram_set(WLAN24G_VIRTUAL_HIDE_SSID, "1"); 
			sprintf(msg_tmp,"op=%d,wlan_ifname=%s",OP_RESTART,TENDA_WLAN24_VIRTUAL_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
		}
		if(device_name_5G == WLAN_RATE_5G && device_name_24G != AUTO_CONN_VIF_24G)
		{
			nvram_set(WLAN5G_VIRTUAL_ENABLE, "1");
			nvram_set(WLAN5G_VIRTUAL_SSID, "Tenda_Extender_1");
			nvram_set(WLAN5G_VIRTUAL_WEP, "disabled");
			nvram_set(WLAN5G_VIRTUAL_AKM, "psk2");
			nvram_set(WLAN5G_VIRTUAL_CRYPTO, "aes");
			nvram_set(WLAN5G_VIRTUAL_PASSWD, "Extender123456");
			nvram_set(WLAN5G_VIRTUAL_HIDE_SSID, "1"); 
			sprintf(msg_tmp,"op=%d,wlan_ifname=%s",OP_RESTART,TENDA_WLAN5_VIRTUAL_IFNAME);
			msg_send(MODULE_RC,RC_WIFI_MODULE,msg_tmp);
		}
	}
	else
	{
		tenda_ifconfig(TENDA_WLAN5_VIRTUAL_IFNAME, 0, NULL, NULL);
		tenda_ifconfig(TENDA_WLAN24_VIRTUAL_IFNAME, 0, NULL, NULL);
		unset_va1_default();
	}
}

#endif
