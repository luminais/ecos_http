/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ： tenda_sta_steering.c
Description : 用户迁移
Author ：jack deng
Version ：V1.0
Date ：2017-3-17
Others ：
History ：
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期： 修改者：
   内容：
2）...
 
*****************************************************************************/
//#include <linux/module.h>
//#include <linux/version.h>
//#include <linux/skbuff.h>
//#include <linux/time.h>
//#include <linux/seq_file.h>
//#include <linux/netdevice.h>

#include <cyg/io/eth/rltk/819x/wrapper/sys_support.h>
#include <cyg/io/eth/rltk/819x/wrapper/skbuff.h>
#include <cyg/io/eth/rltk/819x/wrapper/timer.h>
#include <cyg/io/eth/rltk/819x/wrapper/wrapper.h>
#include "./8192cd_cfg.h"
#include "./8192cd.h"
#include "./8192cd_hw.h"
#include "./8192cd_headers.h"
#include "./8192cd_debug.h"


#include "tenda_sta_steering.h"
#include "tenda_wlan_proc.h"
//int tenda_wlan_proc_init(void);
//void tenda_wlan_proc_exit(void);
extern int is_priv_band_steer(void *priv);
extern void set_priv_band_steer(void *priv,int val);
extern unsigned int get_priv_tatal_sta_num(void *p);

#if 0
extern int (*td_sta_steer_check)(unsigned char *mac);
extern int (*td_sta_steer_collect)(unsigned char *mac, int rssi, unsigned char ch);

extern void (*td_steer_bss_update)(struct net_device *dev,
                            unsigned char *ssid,
                            unsigned char key_type,
                            unsigned char *keys,
                            int down);
#endif
extern void (*td_steer_bss_update)(steer_bss_info_t *p, int down);


struct sta_steer g_steer;
static probe_sta_info_t s_sta_steer_list[MAX_STA_STEER_NUM];

//#define DBG_PRINT if(g_steer.debug) printk
#define DBG_PRINT if(g_steer.debug) panic_printk

/*******************************************************************
Function      : sta_steer_proc_show
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void sta_steer_proc_show()
{
    int i;
    int cnt = 0;
    probe_sta_info_t *p;
    steer_bss_info_t  *bss = &g_steer.bss[0];

    panic_printk("=== SHOW STEER CFG ===\n");
    panic_printk("ENABLE : %d  DEBUG:0x%x\n",g_steer.enable,g_steer.debug);
    panic_printk("AGETIME: %d\n",g_steer.age_timeout);
    panic_printk("sta_balance_enable: %d\n",g_steer.sta_balance_enable);
    panic_printk("rssi_lmt_2g:%d  rssi_lmt_5g:%d\n",g_steer.rssi_lmt_2g,g_steer.rssi_lmt_5g);

    panic_printk("\n");

    panic_printk("INDEX    DEV     CH        SSID      KEYS   STEER===\n");
    for (i = 0; i < MAX_BSS_NUM; i++) {
        if (bss[i].dev) {
            panic_printk("%2d %12s %3d %16s (%2d: %s)  %d\n",
                cnt++,bss[i].dev->name,bss[i].ch,bss[i].ssid,
                bss[i].key_type,bss[i].keys,bss[i].band_steer_enable);
        }
    }

    cnt = 0;
    panic_printk("=== SHOW STA INFO ===\n");
    for (i = 0; i < MAX_STA_STEER_NUM; i++) {
        p = &s_sta_steer_list[i];
        if (p->use) {
            panic_printk("[%d] "MACSTR" time(%lu) rssi(%d) BAND(0x%x) ass(%d) R(%d)\n",
                cnt++,MAC2STR(p->mac),(jiffies - p->probe_jiffies)/HZ,
                p->rssi,p->bandtype,p->is_assoced,p->roaming_status);
        }
    }  
    
}

/*******************************************************************
Function      : sta_list_init
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void sta_list_init(void)
{
    int i;
    probe_sta_info_t *p;

    for (i = 0; i < MAX_STA_STEER_NUM; i++) {
        p = &s_sta_steer_list[i];
        memset(p, 0, sizeof(probe_sta_info_t));
    }
}

/*******************************************************************
Function      : sta_get_node_buf
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static probe_sta_info_t *sta_get_node_buf(void)
{
    static unsigned int s_sta_node_cnt = 0;
    int i;

    i = s_sta_node_cnt & (MAX_STA_STEER_NUM - 1);
    s_sta_node_cnt++;

    return &s_sta_steer_list[i];
}

/*******************************************************************
Function      : sta_find_node
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static probe_sta_info_t *sta_find_node(const unsigned char *mac)
{
    int i;
    probe_sta_info_t *p;

    for (i = 0; i < MAX_STA_STEER_NUM; i++) {
        p = &s_sta_steer_list[i];
        if (p->use && !memcmp(p->mac, mac, STA_MAC_LEN)) {
            return p;
        }
    }
    return NULL;
}

/*******************************************************************
Function      : sta_steer_set_assoc
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void sta_steer_set_assoc(void *priv, unsigned char *mac, int assoc)
{
    probe_sta_info_t *sta;

    sta = sta_find_node(mac);
    if (!sta) {
        sta = sta_get_node_buf();
        memset(sta, 0, sizeof(probe_sta_info_t));
        memcpy(sta->mac, mac, STA_MAC_LEN);
        sta->use = 1;
    }

    if (assoc) {
        DBG_PRINT("%s: ["MACSTR"] assoc\n",__func__,MAC2STR(mac));
        sta->is_assoced = 1;
        sta->roaming_status = 0;
        sta->auth_cnt = 0;
        sta->priv = priv;
    } else if (priv == sta->priv) {
        DBG_PRINT("%s: ["MACSTR"] disassoc\n",__func__,MAC2STR(mac));
        sta->is_assoced = 0;
    }
    sta->probe_jiffies = jiffies;
}
/*******************************************************************
Function      : sta_steer_age_timer_func
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void sta_steer_age_timer_func(unsigned long data)
{
    struct sta_steer *steer = (struct sta_steer *)(data);
    struct probe_sta_info   *p;
    int i;
    DBG_PRINT("---%s---\n",__func__);

    for (i = 0; i < MAX_STA_STEER_NUM; i++) {
        p = &s_sta_steer_list[i];
        if (p->use && ((jiffies - p->probe_jiffies) > (steer->age_timeout * HZ))){
            p->auth_cnt = 0;
            if (!(p->is_assoced)) {
                memset(p, 0, sizeof(probe_sta_info_t));
            }
        }
    }

    mod_timer(&steer->age_timer, jiffies + steer->age_timeout*HZ);
}

/*******************************************************************
Function      : is_priv_stanum_allow
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static int inline is_priv_stanum_allow(void *priv_2g, void *priv_5g)
{
    unsigned num_2g, num_5g;
    if (g_steer.sta_balance_enable) {
        num_2g = get_priv_tatal_sta_num(priv_2g);
        num_5g = get_priv_tatal_sta_num(priv_5g);

        DBG_PRINT("num_2g(%d) num_5g(%d)\n",num_2g,num_5g);
        /* 5G用户数是2G的 3倍，则不需要用户切换到5G */
        if ((num_5g+1) > (num_2g+1)*STA_NUM_LIMIT_2G_5G) {
            return 0;
        }
    }
    return 1;
}

/*******************************************************************
Function      : sta_steer_assoc_roaming
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int sta_steer_assoc_roaming(void *priv, unsigned char channel, unsigned char *mac, int rssi)
{
    probe_sta_info_t *sta;

    if (!is_priv_band_steer(priv)) {
        return 0;
    }
    
    rssi -= 100;    //for RTK struct
    sta = sta_find_node(mac);
    if (sta && (sta->bandtype & PROBE_STA_BANDTYPE_5G)) {
        if (channel < 14) {
            /* In 2G, check rssi_lmt_2g */
            if (g_steer.rssi_lmt_2g && (rssi > g_steer.rssi_lmt_2g)) {
                sta->probe_jiffies = jiffies;
                sta->roaming_status = STEER_ROAMING_TO_5G;
                DBG_PRINT("STA("MACSTR") RSSI(%d) steer roaming to 5G\n", MAC2STR(sta->mac), rssi);
                return 1;
            }
        } else if (channel > 35) {    //5G
            /* In 5G, check rssi_lmt_5g */
            if (g_steer.rssi_lmt_5g && (rssi < g_steer.rssi_lmt_5g)) {
                sta->probe_jiffies = jiffies;
                sta->roaming_status = STEER_ROAMING_TO_2G;
                DBG_PRINT("STA("MACSTR") RSSI(%d) steer roaming to 2.4G\n", MAC2STR(sta->mac), rssi);
                return 1;
            }
        }
    }
    return 0;
}

/*******************************************************************
Function      : sta_steer_check
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int sta_steer_check(void *priv,unsigned char *mac,unsigned char ch)
{
    probe_sta_info_t *sta;
    int ret = 0;

    if (!g_steer.enable) {
        return 0;
    }
    
    if (is_priv_band_steer(priv) && g_steer.priv_5g) {
        sta = sta_find_node(mac);
        if (sta && (sta->bandtype & PROBE_STA_BANDTYPE_5G)) {
            sta->probe_jiffies = jiffies;
            if (ch > 35) {
                /* handle 5g sta */
                if (sta->roaming_status == STEER_ROAMING_TO_2G && sta->auth_cnt++ < g_steer.auth_reject_limit) {
                    ret = 1;
                    DBG_PRINT(" STA("MACSTR") auth(%d) In 5G,need Roaming to 2G!\n",MAC2STR(sta->mac),sta->auth_cnt);
                }else {
                    sta->auth_cnt = 0;
                }
            } else {
                /* handle 2.4g sta */
                if (is_priv_stanum_allow(priv,g_steer.priv_5g)
                    && (sta->rssi > g_steer.rssi_lmt_5g)
                    && (sta->roaming_status != STEER_ROAMING_TO_2G)) {
					if (sta->auth_cnt++ < g_steer.auth_reject_limit) {
                        ret = 1;
                        DBG_PRINT(" STA("MACSTR") auth cnt(%d) Reject\n",MAC2STR(sta->mac),sta->auth_cnt);
                    }else {
                        sta->auth_cnt = 0;
                        DBG_PRINT(" STA("MACSTR") auth cnt > 3,so accept\n",MAC2STR(sta->mac));
                    }
                }
            }
        }
#if 0
        if (sta && sta->rssi > g_steer.rssi_lmt_5g
                 && (sta->roaming_status != STEER_ROAMING_TO_2G)) {
            if (sta->auth_cnt++ < STA_AUTH_REJECT_LIMIT) {
                ret = 1;
                DBG_PRINT(" STA("MACSTR") auth cnt(%d) Reject\n",MAC2STR(sta->mac),sta->auth_cnt);
            }else {
                sta->auth_cnt = 0;
                DBG_PRINT(" STA("MACSTR") auth cnt > 3,so accept\n",MAC2STR(sta->mac));
            }
        }
#endif
    }

    return ret;
	
}

int sta_steer_get_auth_status()
{
	return g_steer.auth_status;
}

int sta_steer_get_assoc_status()
{
	return g_steer.assoc_status;
}

/*******************************************************************
Function      : sta_steer_collect_probe
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int sta_steer_collect_probe(void *priv, unsigned char *mac, int rssi, unsigned char ch)
{
    probe_sta_info_t *p;
    int ret = 0;
    
    if (!g_steer.enable) {
        return ret;
    }
#if 0
    if (ch < 14) {
        return 1;
    }
#endif
    p = sta_find_node(mac);

    if (!p) {
        p = sta_get_node_buf();
        memset(p, 0, sizeof(probe_sta_info_t));
        memcpy(p->mac, mac, STA_MAC_LEN);
        p->use = 1;
    }
    else
    {
        if ((ch < 14)
            && (p->bandtype & PROBE_STA_BANDTYPE_5G)
            && is_priv_band_steer(priv)){
            ret = 1;
        }
    }

    p->probe_jiffies = jiffies;
    p->rssi = rssi - 100;
    if (ch < 14) {
        p->bandtype |= PROBE_STA_BANDTYPE_2G;
    } else {
        p->bandtype |= PROBE_STA_BANDTYPE_5G;
    }
    return ret;
}

/*******************************************************************
Function      : is_same_bss_in_steer
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static int is_same_bss_in_steer(steer_bss_info_t *p)
{
    int i;
    steer_bss_info_t  *bss;

    for (i = 0; i < MAX_BSS_NUM; i++) {
        bss = &g_steer.bss[i];
        if (bss == p || bss->dev == NULL || bss->dev == p->dev) {
            continue;
        }

        if ((bss->key_type == p->key_type)
            && (strcmp(bss->ssid, p->ssid) == 0)
            && (strcmp(bss->keys, p->keys) == 0)) {
            return 1;
        }
    }

    return 0;
}

/*******************************************************************
Function      : steer_update_bss_ability
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void steer_update_bss_ability(void)
{
    int i;
    steer_bss_info_t  *bss;

    for (i = 0; i < MAX_BSS_NUM; i++) {
        bss = &g_steer.bss[i];
        if (bss->dev) {
            bss->band_steer_enable = is_same_bss_in_steer(bss);
            set_priv_band_steer(bss->priv,bss->band_steer_enable);
        }
    }
}

/*******************************************************************
Function      : steer_update_bss_info
Description    : 函数功能、性能等的描述
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void steer_update_bss_info(steer_bss_info_t *p, int down)
{
    int i;
    steer_bss_info_t  *bss;
    panic_printk("%s: isdown(%d)\n",__func__, down);

    /* 已经存在，更新 */
    for (i = 0; i < MAX_BSS_NUM; i++) {
        bss = &g_steer.bss[i];
        if (bss->dev == p->dev) {
            if (down) {
                memset(bss, 0, sizeof(steer_bss_info_t));
            } else {
                memcpy(bss, p, sizeof(steer_bss_info_t));
            }
            goto up_ability;
        }
    }

    /* new, update bss info */
    for (i = 0; !down && i < MAX_BSS_NUM; i++) {
        bss = &g_steer.bss[i];
        if (bss->dev == NULL) {
            memcpy(bss, p, sizeof(steer_bss_info_t));
            goto up_ability;
        }
    }

up_ability:
    steer_update_bss_ability();
    if (p->ch > 14)
        g_steer.priv_5g = p->priv;
}

#if 0
int sta_steer_init(void)
{
    
    struct sta_steer  *steer = &g_steer;
    struct timer_list *timer;

    memset(steer, 0, sizeof(struct sta_steer));

    sta_list_init();

    steer->age_timeout = STA_STEER_AGETIME;
    timer = &steer->age_timer;
    init_timer(timer);
    timer->function = sta_steer_age_timer_func;
    timer->data = (unsigned long)steer;
	mod_timer(timer, jiffies + EXPIRE_TO*HZ);

    tenda_wlan_proc_init();

    //td_sta_steer_collect = sta_steer_collect_probe;
    //td_steer_bss_update = steer_update_bss_info;

    steer->enable = DEFAULT_STA_STEER_ENABLE;

    steer->sta_balance_enable = 0;
    steer->rssi_lmt_2g = DEF_RSSI_ROAMING_2G;
    steer->rssi_lmt_5g = DEF_RSSI_ROAMING_5G;
	steer->auth_reject_limit = STA_AUTH_REJECT_LIMIT;
	steer->auth_status = 17;      //_STATS_UNABLE_HANDLE_STA_;
	steer->assoc_status = 1;   //_RSON_UNSPECIFIED_;
    panic_printk("--- STA STEERING INIT ---\n");
    return 0;
}
#endif

int steer_timer_up = 0;
int sta_steer_init(void)
{
    
    struct sta_steer  *steer = &g_steer;
    struct timer_list *timer;

    memset(steer, 0, sizeof(struct sta_steer));

    sta_list_init();

    steer->age_timeout = STA_STEER_AGETIME;
    timer = &steer->age_timer;
    init_timer(timer);
    timer->function = sta_steer_age_timer_func;
    timer->data = (unsigned long)steer;
	mod_timer(timer, jiffies + steer->age_timeout*HZ);
    steer_timer_up = 1;
    //tenda_wlan_proc_init();

    //td_sta_steer_collect = sta_steer_collect_probe;
    //td_steer_bss_update = steer_update_bss_info;

    steer->enable = DEFAULT_STA_STEER_ENABLE;

    steer->sta_balance_enable = 0;
    steer->rssi_lmt_2g = DEF_RSSI_ROAMING_2G;
    steer->rssi_lmt_5g = DEF_RSSI_ROAMING_5G;
	steer->auth_reject_limit = STA_AUTH_REJECT_LIMIT;
	steer->auth_status = 17;      //_STATS_UNABLE_HANDLE_STA_;
	steer->assoc_status = 1;   //_RSON_UNSPECIFIED_;
    panic_printk("--- STA STEERING INIT ---\n");
    return 0;
}


void sta_steer_exit(void)
{
    
    struct sta_steer  *steer = &g_steer;
	if (timer_pending(&steer->age_timer))
	{	
		panic_printk("test %s(%d)\n",__func__,__LINE__);
		del_timer_sync(&steer->age_timer);
        steer_timer_up = 0;
	}
    //td_sta_steer_collect = NULL;
    //td_steer_bss_update = NULL;
    panic_printk("--- STA STEERING EXIT ---\n");
}

