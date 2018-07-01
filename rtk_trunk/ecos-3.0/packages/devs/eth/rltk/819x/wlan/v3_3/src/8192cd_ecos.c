/*
 *  Routines to ecos dependent jobs and interfaces
 *
 *  Copyright (c) 2010 Realtek Semiconductor Corp.
 *
 */

/*-----------------------------------------------------------------------------
	This file is for OS related functions.
------------------------------------------------------------------------------*/
#define _8192CD_ECOS_C_

#include "./rtl8192cd/8192cd_cfg.h"

#include <cyg/hal/plf_intr.h>
#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>

#include "./rtl8192cd/8192cd.h"
#include "./rtl8192cd/8192cd_hw.h"
#include "./rtl8192cd/8192cd_headers.h"
#include "./rtl8192cd/8192cd_rx.h"
#include "./rtl8192cd/8192cd_debug.h"

#ifdef PCIE_POWER_SAVING
extern void HostPCIe_Close(void);
#endif

extern void *rtl8192cd_init_one(void *pdev, void *ent, struct _device_info_ *wdev, int vap_idx, int wlan_index);
extern void rtl8192cd_proc_stats_clear(void *data);
extern int rtl8192cd_proc_gpio_ctrl_write(char *command, int gpio_num, char *action, void *data);
extern void rtl8192cd_proc_up_write(char *tmp, void *data);
extern void rtl8192cd_proc_txdesc_idx_write(int txdesc_num, void *data);
extern void rtl8192cd_proc_led(int flag, void *data);
extern void rtl8192cd_proc_debug(struct net_device *dev, char *cmd);

//#ifndef pr_fun
//typedef void pr_fun(char *fmt, ...);
//#endif

pr_fun *ecos_pr_fun = (pr_fun *)diag_printf;

void set_ecos_pr_fun(pr_fun *pr)
{
	ecos_pr_fun = pr;
}

void *rltk_wlan_init(int wlan_idx)
{
	void *ptr = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_idx], -1, wlan_idx);

#ifdef PCIE_POWER_SAVING
	HostPCIe_Close();
#endif

#ifdef _USE_DRAM_
	{
	extern unsigned char *en_cipherstream;
	extern unsigned char *tx_cipherstream;
	extern char *rc4sbox, *rc4kbox;
	extern unsigned char *pTkip_Sbox_Lower, *pTkip_Sbox_Upper;
	extern unsigned char Tkip_Sbox_Lower[256], Tkip_Sbox_Upper[256];

#ifdef CONFIG_RTL8671
	extern void r3k_enable_DRAM(void);    //6/7/04' hrchen, for 8671 DRAM init
	r3k_enable_DRAM();    //6/7/04' hrchen, for 8671 DRAM init
#endif

	en_cipherstream = (unsigned char *)(DRAM_START_ADDR);
	tx_cipherstream = en_cipherstream;

	rc4sbox = (char *)(DRAM_START_ADDR + 2048);
	rc4kbox = (char *)(DRAM_START_ADDR + 2048 + 256);
	pTkip_Sbox_Lower = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*2);
	pTkip_Sbox_Upper = (unsigned char *)(DRAM_START_ADDR + 2048 + 256*3);

	memcpy(pTkip_Sbox_Lower, Tkip_Sbox_Lower, 256);
	memcpy(pTkip_Sbox_Upper, Tkip_Sbox_Upper, 256);
	}
#endif
#if defined(CONFIG_RTL_CUSTOM_PASSTHRU)
	//diag_printf("------wlan passthru init![%s]:[%d].\n",__FUNCTION__,__LINE__);
	rtl_wlan_customPassthru_init();
#endif

#ifdef PERF_DUMP
	rtl8651_romeperfInit();
#endif

#ifdef USB_PKT_RATE_CTRL_SUPPORT
	register_usb_hook = (register_usb_pkt_cnt_fn)(register_usb_pkt_cnt_f);
#endif	

	return ptr;
}


#ifdef UNIVERSAL_REPEATER
void *rltk_wlan_vxd_init(int wlan_idx)
{
	void *ptr = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_idx], -1, wlan_idx);
	return ptr;
}
#endif


#ifdef MBSSID
void *rltk_wlan_vap_init(int wlan_idx, int vap_idx)
{	
	void *ptr = rtl8192cd_init_one(NULL, NULL, &wlan_device[wlan_idx], vap_idx, wlan_idx);
	return ptr;	
}
#endif


void interrupt_dsr(struct net_device *dev)
{
	struct rtl8192cd_priv *priv;
#ifdef MBSSID
	int i;
#endif

	priv = (struct rtl8192cd_priv *)dev->priv;

#ifdef DFS
	if (priv->pshare->has_triggered_dfs_switch_channel) {
		DFS_SwitchChannel(priv);
		priv->pshare->has_triggered_dfs_switch_channel = 0;
	}
#endif
	
	if (priv->pshare->has_triggered_process_mcast_dzqueue) {
		process_mcast_dzqueue(priv);
		priv->pkt_in_dtimQ = 0;
		priv->pshare->has_triggered_process_mcast_dzqueue = 0;
	}

#ifdef MBSSID
	for (i=0; i<RTL8192CD_NUM_VWLAN; i++) {
		if (priv->pshare->has_triggered_vap_process_mcast_dzqueue[i]) {
			process_mcast_dzqueue(priv->pvap_priv[i]);
			priv->pvap_priv[i]->pkt_in_dtimQ = 0;
			priv->pshare->has_triggered_vap_process_mcast_dzqueue[i] = 0;
		}
	}
#endif

#if defined(TXREPORT)
	if (priv->pshare->has_triggered_C2H_isr) {
#ifdef  CONFIG_WLAN_HAL
//		C2H_isr_88XX(priv);
		GET_HAL_INTERFACE(priv)->C2HHandler(priv);
#elif (defined(CONFIG_RTL_92C_SUPPORT) || defined(CONFIG_RTL_92D_SUPPORT))
		C2H_isr(priv);
#endif
		priv->pshare->has_triggered_C2H_isr = 0;
	}
#endif

#ifndef ISR_DIRECT
	if (priv->pshare->rx_tasklet && priv->pshare->has_triggered_rx_tasklet) {
		rtl8192cd_rx_tkl_isr((unsigned long)priv);
		//priv->pshare->has_triggered_rx_tasklet = 0;
	}
#endif

#ifndef TX_ISR_DIRECT
	if (priv->pshare->tx_tasklet && priv->pshare->has_triggered_tx_tasklet) {
		rtl8192cd_tx_dsr((unsigned long)priv);
		priv->pshare->has_triggered_tx_tasklet = 0;
	}
#endif

#ifdef SW_TX_QUEUE  /*2015 0910 move sw_tx_queue to dsr*/
	if (priv->pshare->has_triggered_sw_tx_Q_tasklet) {
        rtl8192cd_swq_timeout((unsigned long) priv);        
		priv->pshare->has_triggered_sw_tx_Q_tasklet = 0;
	}
#endif


}

int can_xmit(struct net_device *dev)
{
	return 1;
}

#ifdef TX_SCATTER
struct sk_buff *copy_skb(struct sk_buff *skb)
{
	struct sk_buff *new_skb;
	int i;
	
	new_skb = dev_alloc_skb(skb->total_len);	
	if (new_skb) {
		for (i = 0; i<skb->list_num; i++) {
			memcpy(new_skb->tail, skb->list_buf[i].buf, skb->list_buf[i].len);
			skb_put(new_skb, skb->list_buf[i].len);		
		}	
		new_skb->dev = skb->dev;
		new_skb->cb[1] = skb->cb[1];
	}
	
	return new_skb;
}
#endif


/*
 * Following routines are added for shell debug command
 */
#ifdef WDS
struct net_device *rltk_get_wds_net_device(int wlan_idx, int wds_num)
{
	struct rtl8192cd_priv *priv = wlan_device[wlan_idx].priv;
	if (priv)
		return priv->wds_dev[wds_num];
	else
		return NULL;
}
#endif

#ifdef CONFIG_RTK_MESH
struct net_device *rltk_get_mesh_net_device(int wlan_idx, int mesh_num)
{
	struct rtl8192cd_priv *priv = wlan_device[wlan_idx].priv;
	if (priv)
		return priv->mesh_dev;
	else
		return NULL;
}
#endif

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
extern int rtl_wlan_customPassthru_init(void);

void passthru_wlan_show()
{
	diag_printf("\npassthru_wlan:%d	 ", passThruStatusWlan);
	if (passThruStatusWlan&IP6_PASSTHRU_MASK){
		diag_printf("ipv6 passthru support");
		#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if(passThruStatusWlan&PPPOE_PASSTHRU_MASK)
			diag_printf("& PPPOE passthru support");
		#endif
		
	}	
	else
	{
		#if	defined(CONFIG_RTL_CUSTOM_PASSTHRU_PPPOE)
		if(passThruStatusWlan&PPPOE_PASSTHRU_MASK)
			diag_printf("PPPOE passthru support");
		#endif
	}
	diag_printf("\n");
}

void passthru_wlan_write()
{
	
}

struct net_device *rltk_get_pwlan_net_device(int dev_num, int device_num)
{
	#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	struct rtl8192cd_priv *priv = wlan_device[dev_num].priv;
	//diag_printf("dev_num:%d,device_num:%d,[%s]:[%d].\n",dev_num,device_num,__FUNCTION__,__LINE__);
	if (priv){
		
		if (!priv->pWlanDev){
			
			diag_printf("no pwlan dev! need to init[%s]:[%d].\n",__FUNCTION__,__LINE__);
			rtl_wlan_customPassthru_init();
			
		}
		
		return priv->pWlanDev;
	}
	else{
		return NULL;
	}	
	#else
		return NULL;
	#endif
}
#endif

void rtl8192cd_wlan_cmd_dispatch(char *name, int argc, char *argv[])
{
	extern struct rtl8192cd_priv *wlan_search_priv(char *name);
	struct rtl8192cd_priv *priv = wlan_search_priv(name);
	struct net_device *dev;
	extern int _atoi(char * s, int base);

	if (priv == NULL) {
		ecos_pr_fun("search driver name [%s] failed!\n", name);
		return;
	}
	dev = priv->dev;

	// handle special commands first
	if (strcmp(argv[0], "stats")==0) {
		if (argc > 1) {
			rtl8192cd_proc_stats_clear(dev);
			return;
		}
	}
#ifdef RTLWIFINIC_GPIO_CONTROL
	else if (strcmp(argv[0], "gpio_ctrl")==0) {
		if (argc >= 4) {
			rtl8192cd_proc_gpio_ctrl_write(argv[1], _atoi(argv[2], 10), argv[3], dev);
			return;
		}
	}
#endif
#ifdef CLIENT_MODE
	else if (strcmp(argv[0], "up_flag")==0) {
		if (argc > 1) {
			rtl8192cd_proc_up_write(argv[1], dev);
			return;
		}
	}
#endif
	else if (strcmp(argv[0], "txdesc")==0) {
		if (argc > 1) {
			rtl8192cd_proc_txdesc_idx_write(_atoi(argv[1], 10), dev);
			return;
		}
	}
	else if (strcmp(argv[0], "led")==0) {
		if (argc > 1) {
			rtl8192cd_proc_led(_atoi(argv[1], 10), dev);
		}
		return;
	}

#ifdef CONFIG_RTL_CUSTOM_PASSTHRU
	else if(strcmp(argv[0], "passthru_wlan")==0)
	{
		int mask_value=0;
		if (argc >= 2)
		{
			if (strcmp(argv[1], "read")==0){
				passthru_wlan_show();
				
			}	
			else if ((strcmp(argv[1], "write")==0))
			{
				if(argc > 2){
					mask_value=strtol(argv[2], NULL, 0);
					passThruStatusWlan=mask_value;
					passthru_wlan_write();
				}	
				
			}
		}	
		else{
			passthru_wlan_show();
		}
	}
#endif
	// others
	rtl8192cd_proc_debug(dev, argv[0]);
}

#ifdef CONFIG_RTL_ALP
#define STATUS_FILE "/var/run/wifi_status.xml"
#include <stdio.h>

//the len of the *dest must longer than 32 X 6
int encode_ssid_str(const char *source,char *dest )
{
	int i;
	char *tmp = source;
	if(source ==NULL || dest ==NULL )return -1;
	for(i=0;i<32;i++)
	{
		switch(*tmp){
		case '&':
			memcpy(dest,"&amp;",5);
			dest = dest + 5;
			 tmp++;
			 break;
		case '\"':
			memcpy(dest,"&quot;",6);
			dest = dest +6 ;
			 tmp++;
			 break;
		case '<':
			memcpy(dest,"&lt;",4);
			dest = dest + 4;
			 tmp++;
			 break;
		case '>':
			memcpy(dest,"&gt;",4);
			dest = dest +4;
			 tmp++;
			 break;
		default:
			*dest++ = *tmp++;
			break;
		}
	}
return 1;
}

void silence_stainfo(void)
{
        struct rtl8192cd_priv *priv = wlan_device[0].priv;
        struct net_device *dev = priv->dev;
        int wifimode=0, i;
        unsigned char wifimode_str[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};
        struct list_head *phead, *plist;
        struct stat_info *pstat;
        FILE *fptr;
	char decode_ssid[200]={0}; //must longer than 32 X 6

        fptr=fopen(STATUS_FILE,"w+");
        if (fptr==NULL){
                diag_printf("%s: Fail to create %s\n", __FUNCTION__, STATUS_FILE);
                return;
        }
	
        fprintf(fptr,"<XML_ROOT><runtime><wifi_tmpnode><media>");
	 encode_ssid_str(priv->pmib->dot11StationConfigEntry.dot11DesiredSSID,decode_ssid);
	 fprintf(fptr,"<ssid>%s</ssid>", decode_ssid);
        //fprintf(fptr,"<ssid>%s</ssid>", priv->pmib->dot11StationConfigEntry.dot11DesiredSSID);
        fprintf(fptr,"<channel>%d</channel>", priv->pmib->dot11RFEntry.dot11channel);
        if ((priv->pmib->dot11OperationEntry.opmode & WIFI_AP_STATE) == WIFI_AP_STATE){
                fprintf(fptr,"<clients>");
                phead = &priv->asoc_list;
                if (!netif_running(dev) || list_empty(phead)){
                        if (fptr!=NULL){fclose(fptr);}
                        return;
                }
                plist = phead->next;
                while (plist != phead) {
                        pstat = list_entry(plist, struct stat_info, asoc_list);
                        if (pstat->expire_to != 0)
                        {
                                fprintf(fptr,"<entry><macaddr>%02x:%02x:%02x:%02x:%02x:%02x</macaddr>",
                                                pstat->hwaddr[0], pstat->hwaddr[1], pstat->hwaddr[2],
                                                pstat->hwaddr[3], pstat->hwaddr[4], pstat->hwaddr[5]);
                               if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11A){
                                        wifimode = WIRELESS_11A;
                                }else if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11G){
                                        if (!isErpSta(pstat))
                                                wifimode = WIRELESS_11B;
                                        else {
                                                wifimode = WIRELESS_11G;
                                                for (i=0; i<STAT_OPRATE_LEN; i++) {
                                                        if (is_CCK_rate(STAT_OPRATE[i])) {
                                                                wifimode |= WIRELESS_11B;
                                                                break;
                                                        }
                                                }
                                        }
                                }else{ // 11B only
                                        wifimode = WIRELESS_11B;
                                }
                                if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11N) {
                                        if (pstat->ht_cap_len) {
                                                if (!should_restrict_Nrate(priv, pstat)) {
                                                        wifimode |= WIRELESS_11N;
                                                }
                                        }
                                }
				    if (priv->pmib->dot11BssType.net_work_type & WIRELESS_11AC)
				    {
					     if (pstat->vht_cap_len) {
                                                if (!should_restrict_Nrate(priv, pstat)) {
                                                        wifimode |= WIRELESS_11AC;
                                                }
                                        }
				    }
                                if (wifimode & WIRELESS_11AC)
                                        memcpy(wifimode_str, "11AC", 4);
                                else if (wifimode & WIRELESS_11N)
                                        memcpy(wifimode_str, "11N", 3);
				    else if (wifimode & WIRELESS_11A)
                                        memcpy(wifimode_str, "11A", 3);
                                else if (wifimode & WIRELESS_11G)
                                        memcpy(wifimode_str, "11G", 3);
                                else if (wifimode & WIRELESS_11B)
                                        memcpy(wifimode_str, "11B", 3);
                                else
                                        memcpy(wifimode_str, "unknown", 7);
                                fprintf(fptr,"<band>%s</band></entry>", wifimode_str);
                        }
                        plist = plist->next;
                }
                fprintf(fptr,"</clients>");
        }else if ((priv->pmib->dot11OperationEntry.opmode & WIFI_STATION_STATE) == WIFI_STATION_STATE){
                if ((priv->pmib->dot11OperationEntry.opmode & WIFI_ASOC_STATE) == WIFI_ASOC_STATE)
                        fprintf(fptr,"<connection>CONNECTED</connection>");
                else if (priv->pmib->dot11OperationEntry.opmode & WIFI_AUTH_NULL & WIFI_AUTH_STATE1 & WIFI_AUTH_SUCCESS)
                        fprintf(fptr,"<connection>CONNECTED</connection>");
                else
                        fprintf(fptr,"<connection>DISCONNECTED</connection>");
        }
        fprintf(fptr,"</media></wifi_tmpnode></runtime></XML_ROOT>");
        fclose(fptr);
        return;
}
#endif
