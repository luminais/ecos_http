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
#include "tenda_wlan_proc_ecos.h"
//#include <stdio.h>
#include "tenda_dev_probe.h"
//#include "tenda_wlan_proc.h"

extern int is_priv_dev_probe_enable(void *priv);


struct dev_probe_res g_dev_probe_res;
static dev_probe_info_t s_dev_probe_list[MAX_DEV_PROBE_NUM]; /*2.4G探测信息表单*/
static dev_probe_info_t s_dev_probe_list_5g[MAX_DEV_PROBE_NUM];/*5G探测信息表单*/

/*******************************************************************
Function      : dev_probe_list_init
Description    : 函数功能、性能等的描述
                        探针列表信息初始化
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void dev_probe_list_init(void)
{
    int i;
    dev_probe_info_t *p;

    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list[i];
        memset(p, 0, sizeof(dev_probe_info_t));      
        p = &s_dev_probe_list_5g[i];
        memset(p, 0, sizeof(dev_probe_info_t));
    }
}
#if 0
/*----------------------------------------------------------------------------
index: the information element id index, limit is the limit for search
-----------------------------------------------------------------------------*/
/**
 *	@brief	Get Information Element
 *
 *		p (Find ID in limit)		\n
 *	+--- -+------------+-----+---	\n
 *	| ... | element ID | len |...	\n
 *	+--- -+------------+-----+---	\n
 *
 *	@param	pbuf	frame data for search
 *	@param	index	the information element id = index (search target)
 *	@param	limit	limit for search
 *
 *	@retval	p	pointer to element ID
 *	@retval	len	p(IE) len
 */
static unsigned char *get_ie(unsigned char *pbuf, int index, int *len, int limit)
{
	unsigned int tmp,i;
	unsigned char *p;

	if (limit < 1)
		return NULL;

	p = pbuf;
	i = 0;
	*len = 0;
	while(1){
		if (*p == index){
			*len = *(p + 1);
			return (p);
		}
		else{
			tmp = *(p + 1);
			p += (tmp + 2);
			i += (tmp + 2);
		}
		if (i >= limit)
			break;
	}
	return NULL;
}
/*******************************************************************
Function      : is_CCK_rat
Description    : 函数功能、性能等的描述
                        是否是cck的速率
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/

static __inline__ int is_CCK_rate(unsigned char rate)
{
	if ((rate == 2) || (rate == 4) || (rate == 11) || (rate == 22))
		return TRUE;
	else
		return FALSE;
}
/*******************************************************************
Function      : is_OFDM_rate
Description    : 函数功能、性能等的描述
                        是否是ofdm的速率
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/

static __inline__ int is_OFDM_rate(unsigned char rate)
{
	if ((rate == _6M_RATE_) || (rate == _9M_RATE_) || (rate == _12M_RATE_) || (rate == _18M_RATE_) || (rate == _24M_RATE_) || (rate == _36M_RATE_) || (rate == _48M_RATE_) || (rate == _54M_RATE_))
		return TRUE;
	else
		return FALSE;
}
#endif
/*******************************************************************
Function      : dev_probe_get_node_buf
Description    : 函数功能、性能等的描述 
                        2.4G获取没被使用的探针list,用于存储新帧
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static dev_probe_info_t *dev_probe_get_node_buf(int *flag, int *s)
{
    int i = 0;
	dev_probe_info_t *p = NULL;
    *flag = 0;
	if(g_dev_probe_res.dev_num>= MAX_DEV_PROBE_NUM){		
		i = (g_dev_probe_res.ext_num % MAX_DEV_PROBE_NUM);
		g_dev_probe_res.ext_num = i;
		p = &s_dev_probe_list[i];
		g_dev_probe_res.ext_num++;
		*flag = 1;       
	}
	else{
  		for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {	
			p = &s_dev_probe_list[i];
					
		 	if(0 == p->use){				 	
				g_dev_probe_res.dev_num++;
				*flag = 1;
		 		break;
		 	}
  		}
	}   
    *s = i;
    return p;
}

/*******************************************************************
Function      : dev_probe_get_node_buf
Description    : 函数功能、性能等的描述
                        5G获取没被使用的探针list,用于存储新帧
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static dev_probe_info_t *dev_probe_get_node_buf_5g(int *flag, int *s)
{
    int i = 0;
	dev_probe_info_t *p = NULL;
    *flag = 0;
	if(g_dev_probe_res.dev_num_5g>= MAX_DEV_PROBE_NUM){		
		i = (g_dev_probe_res.ext_num_5g % MAX_DEV_PROBE_NUM);
		g_dev_probe_res.ext_num_5g = i;
		p = &s_dev_probe_list_5g[i];
		g_dev_probe_res.ext_num_5g++;
		*flag = 1;       
	}
	else{
  		for (i = 0; i < MAX_DEV_PROBE_NUM; i++){	
			p = &s_dev_probe_list_5g[i];
					
		 	if(0 == p->use){				 	
				g_dev_probe_res.dev_num_5g++;
				*flag = 1;
		 		break;
		 	}
  		}
	}   
    *s = i;
    return p;
}

/*******************************************************************
Function      : dev_probe_find_node
Description    : 函数功能、性能等的描述
                        防止list存储重复mac信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static dev_probe_info_t *dev_probe_find_node(const unsigned char *mac, int *s)
{
    int i;
    dev_probe_info_t *p;

    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list[i];
        if (p->use && !memcmp(p->mac, mac, MACADDRLEN)) {
            *s = i;
            return p;
        }
    }
    return NULL;
}

/*******************************************************************
Function      : dev_probe_find_node
Description    : 函数功能、性能等的描述
                        防止list存储重复5g mac信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static dev_probe_info_t *dev_probe_find_node_5g(const unsigned char *mac, int *s)
{
    int i;
    dev_probe_info_t *p;

    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list_5g[i];
        if (p->use && !memcmp(p->mac, mac, MACADDRLEN)) {
            *s = i;
            return p;
        }
    }
    return NULL;
}

/*******************************************************************
Function      : dev_probe_age_timer_func
Description    : 函数功能、性能等的描述
                        check_time_cnt秒进入一次函数，清掉大于age_timeout秒的设备信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void dev_probe_age_timer_func(unsigned long data)
{
    struct dev_probe_res *probe = (struct dev_probe_res *)(data);
    dev_probe_info_t *p;
    int i, count = 0,count_5g = 0;
    

    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list[i]; 
        if(p->use){
            count++;
        }
        if (p->use && (jiffies - p->rx_time) > (probe->age_timeout * HZ)) {           
            memset(p, 0, sizeof(dev_probe_info_t));          
			probe->dev_num--;
        }    
        p = &s_dev_probe_list_5g[i];        
        if(p->use){
            count_5g++;
        }
        if (p->use && (jiffies - p->rx_time) > (probe->age_timeout * HZ)) {           
            memset(p, 0, sizeof(dev_probe_info_t));          
			probe->dev_num_5g--;
        }
    }
    //TENDA_CREATE_PROC_READ_WRITE_ENTRY("dev_probe", dev_probe_read_proc, dev_probe_write_proc, NULL);
    mod_timer(&probe->age_timer, jiffies + probe->check_time_cnt*HZ);
}
/*******************************************************************
Function      :dev_probe_clean
Description    : 函数功能、性能等的描述
                清除掉2.4G设备的存储信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void dev_probe_clean()//BSPHZX++
{
	int i = 0;
	dev_probe_info_t *p = NULL;
	
	g_dev_probe_res.dev_num = 0;
	g_dev_probe_res.check_time_cnt = 0;
	g_dev_probe_res.ext_num = 0;
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list[i];
        memset(p, 0, sizeof(dev_probe_info_t));
    }
} 
/*******************************************************************
Function      :dev_probe_clean
Description    : 函数功能、性能等的描述
                清除掉5G设备的存储信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void dev_probe_clean_5g()//BSPHZX++
{
	int i = 0;
	dev_probe_info_t *p = NULL;
	
	g_dev_probe_res.dev_num_5g = 0;
	g_dev_probe_res.check_time_cnt = 0;
	g_dev_probe_res.ext_num_5g = 0;
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++) {
        p = &s_dev_probe_list_5g[i];
        memset(p, 0, sizeof(dev_probe_info_t));
    }
} 

/*******************************************************************
Function      :dev_probe_collect_beacon
Description    : 函数功能、性能等的描述
                        收集探针信息beacon和探测响应字段解析
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
static void dev_probe_collect_beacon(dev_probe_info_t *probe_Info,unsigned char dot11channel, unsigned int	pktlen, unsigned int frSubType, unsigned char *pframe, unsigned int ht_cap_elmt_size, unsigned int vht_cap_elmt_size)//BSPHZX++
{
    int i, len;
    unsigned char *p;
    unsigned char * ssid_ptr;
    int ssid_len;
	unsigned char supportedRates[32];
    int supplen=0;

    /* checking SSID */
	if(frSubType == WIFI_PROBERSP){
   		p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_, _SSID_IE_, &len,
    		pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
	}
	else{
    	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _SSID_IE_, &len,
            pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	}
	if ((p == NULL) ||		
         (len == 0) ||		
         (*(p+2) == '\0'))	{
        return;
    }
    
    ssid_ptr = p+2;
    ssid_len = len;
    memcpy(probe_Info->ssid, ssid_ptr, ssid_len);

    probe_Info->ssid[ssid_len] = 0;     
   
     
    /* checking HT_CAP IE */	
	if(frSubType == WIFI_PROBERSP){
   		p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_,  _HT_CAP_, &len,
    		pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
	}
	else{
    	p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _HT_CAP_, &len,
         	pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	}
	if ((p !=  NULL) && (len <= ht_cap_elmt_size)) {
         probe_Info->network |= WIRELESS_11N;
    }
	if(dot11channel > 14){
	    /* checking VHT_CAP IE */	
		if(frSubType == WIFI_PROBERSP){
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_,  EID_VHTCapability, &len,
				pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
		}
		else{
			p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, EID_VHTCapability, &len,
				pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
		}
		if ((p !=  NULL) && (len <= vht_cap_elmt_size)) {
			probe_Info->network |= WIRELESS_11AC;
		}
	}
	
    // look for ERP rate. if no ERP rate existed, thought it is a legacy AP  
	if(frSubType == WIFI_PROBERSP){
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_,
            _SUPPORTEDRATES_IE_, &len,
            pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
	}
	else {
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
            _SUPPORTEDRATES_IE_, &len,
            pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	}
	
	if (p){
		if (len>8)
			len=8;
		memcpy(&supportedRates[supplen], p+2, len);
		supplen += len;
	}

	if(frSubType == WIFI_PROBERSP){
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _PROBERSP_IE_OFFSET_,
			_EXT_SUPPORTEDRATES_IE_, &len,
			pktlen - WLAN_HDR_A3_LEN - _PROBERSP_IE_OFFSET_);
	}
	else{
		p = get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_,
			_EXT_SUPPORTEDRATES_IE_, &len,
			pktlen - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_);
	}
	
	if (p){
		if (len>8)
  			len=8;
		memcpy(&supportedRates[supplen], p+2, len);
		supplen += len;
	}
	  
	for (i=0; i<supplen; i++){
		if (is_CCK_rate(supportedRates[i]&0x7f)){
			probe_Info->network |= WIRELESS_11B;
		}
		if (is_OFDM_rate(supportedRates[i]&0x7f)){
			if(dot11channel > 14){
                probe_Info->network |= WIRELESS_11A;
			}
			else{
                probe_Info->network |= WIRELESS_11G;
			}
		}
	}
    
    //printk("\n ++mac="MACSTR",ssid=%s,network=%d\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],probe_Info.ssid,probe_Info.network);
	return;
}

/*******************************************************************
Function      : dev_probe_collect
Description    : 函数功能、性能等的描述
                        收集探针信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void dev_probe_collect(unsigned char *mac, int rssi,unsigned char dot11channel, unsigned int pktlen, unsigned int frSubType, unsigned char *pframe, unsigned int ht_cap_elmt_size, unsigned int vht_cap_elmt_size)
{
    int flag = 0, s = 0;
    dev_probe_info_t *p = NULL;
    if(dot11channel > 14){
        p = dev_probe_find_node_5g(mac, &s);
    }
    else{
        p = dev_probe_find_node(mac, &s);
    }
    if(NULL == p){       
        if(dot11channel > 14){
            p = dev_probe_get_node_buf_5g(&flag, &s);            
            g_dev_probe_res.dot11channel_5g = dot11channel;           
        }
        else{
            p = dev_probe_get_node_buf(&flag, &s);          
            g_dev_probe_res.dot11channel = dot11channel;
        }
        if(1 == flag){
           memcpy(p->mac, mac, MACADDRLEN);
           p->rssi = rssi - 100;
           p->use = 1;
           p->isAP= 0;
	    memset(p->ssid,0x0,WLAN_SSID_MAXLEN+1);
           if((frSubType == WIFI_PROBERSP)||(frSubType == WIFI_BEACON)){
               p->isAP= 1;           
               dev_probe_collect_beacon(p, dot11channel, pktlen, frSubType, pframe, ht_cap_elmt_size, vht_cap_elmt_size);
           }   
           p->rx_time = jiffies;
	   //  diag_printf("\n ++mac=%02x:%02x:%02x:%02x:%02x:%02x,dot11channel:%d,rssi=%d, deviceNum=%d curtime=%llu,isAP=%d,network=%d,ext_num=%d i=%d,ssid=%s\n",\
	//	 	p->mac[0],p->mac[1],p->mac[2],p->mac[3],p->mac[4],\
	//	 	p->mac[5],dot11channel,p->rssi,g_dev_probe_res.dev_num-1,jiffies/HZ,p->isAP,p->network,\
	//	 	g_dev_probe_res.ext_num,s,p->ssid);

        }     
    }
    else{       
        p->rx_time = jiffies;        
    }
}

/*******************************************************************
Function      : dev_probe_ioctl
Description    : 函数功能、性能等的描述
                        收集探针信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int tenda_dev_probe_ioctl(unsigned char dot11channel, web_tenda_dev_probe_ioctl p[])
{
    int i, count = 0;
    dev_probe_info_t *probe;

    /* 已经存在，更新 */
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++){ 
        if(dot11channel > 14){
            probe = &s_dev_probe_list_5g[i];
        }
        else{
            probe = &s_dev_probe_list[i];
        }
        if(probe->use){
           memcpy(p[count].mac, probe->mac, MACADDRLEN);
           p[count].isAP = probe->isAP;        
           p[count].rssi = probe->rssi;        
           p[count].network = probe->network;          
           memcpy(p[count].ssid, probe->ssid, WLAN_SSID_MAXLEN+1);
           count++;
        }
    }
    return count;
}

/*******************************************************************
Function      : dev_probe_num_ioctl
Description    : 函数功能、性能等的描述
                        收集探针信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int tenda_dev_probe_num_ioctl(unsigned char dot11channel)
{
    int i, count = 0;
    dev_probe_info_t *probe;

    /* 已经存在，更新 */
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++){ 
        if(dot11channel > 14){
            probe = &s_dev_probe_list_5g[i];
        }
        else{
            probe = &s_dev_probe_list[i];
        }
        if(probe->use){
           count++;
        }
    }
    return count;
}

/*******************************************************************
Function      : dev_probe_proc_show
Description    : 函数功能、性能等的描述
				显示探针信息
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void dev_probe_proc_show(void *buff)
{
    int i;
    int count = 0;
    dev_probe_info_t *p;
    struct seq_file *s = buff;

    seq_printf(s,"=== SHOW DEVICE PROBE CFG ===\n");
    seq_printf(s,"ENABLE : %d \n",g_dev_probe_res.enable);
    seq_printf(s,"AGETIME: %d \n",g_dev_probe_res.age_timeout);
    seq_printf(s,"\n");
	seq_printf(s,"\n2.4G\n mac                 rssi     isAP    index   channel   devNum    count   network  ssid\n");
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++){        
        p = &s_dev_probe_list[i];
        if(p->use){
            count++;
            seq_printf(s,"\n"MACSTR"    %d       %d       %d        %d        %d        %d       %d   %s\n",p->mac[0],p->mac[1],p->mac[2],p->mac[3],p->mac[4],p->mac[5],p->rssi,p->isAP,i,g_dev_probe_res.dot11channel,g_dev_probe_res.dev_num,count,p->network,p->ssid);
        }
    }
    
    count = 0;
    seq_printf(s,"\n5G\n mac                 rssi     isAP    index   channel   devNum    count   network  ssid\n");
    for (i = 0; i < MAX_DEV_PROBE_NUM; i++){        
        p = &s_dev_probe_list_5g[i];
        if(p->use){          
            count++;
            seq_printf(s,"\n"MACSTR"    %d       %d       %d        %d        %d        %d       %d   %s\n",p->mac[0],p->mac[1],p->mac[2],p->mac[3],p->mac[4],p->mac[5],p->rssi,p->isAP,i,g_dev_probe_res.dot11channel_5g,g_dev_probe_res.dev_num_5g,count,p->network,p->ssid);
        }
    }    
 
}
/*******************************************************************
Function      : dev_probe_init
Description    : 函数功能、性能等的描述
				探针信息初始化接口
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
int dev_probe_init(void)
{
    
    struct dev_probe_res *probe = &g_dev_probe_res;
    struct timer_list *timer;

    memset(probe, 0, sizeof(struct dev_probe_res));

    dev_probe_list_init();

    probe->check_time_cnt = DEV_PROBE_CHECKTIMECNT;  
    probe->age_timeout = DEV_PROBE_AGETIME;
    timer = &probe->age_timer;
    init_timer(timer);
	
    timer->function = dev_probe_age_timer_func;
    timer->data = (unsigned long)probe;
   // timer->expires = jiffies + probe->check_time_cnt*HZ;
   // add_timer(timer);
    mod_timer(timer, jiffies + probe->check_time_cnt*HZ);
    //tenda_wlan_proc_init();

    probe->enable = DEFAULT_DEV_PROBE_ENABLE;

    printk("---DEV PROBE INIT ---\n");
    return 0;
}
/*******************************************************************
Function      : dev_probe_exit
Description    : 函数功能、性能等的描述
				探针信息退出接口
Input         : 输入参数说明，包括每个参数的作
Output        : 对输出参数的说明
Return        : 函数返回值的说明
Others        : 其他说明
*******************************************************************/
void dev_probe_exit(void)
{
    
    struct dev_probe_res *probe = &g_dev_probe_res;
    del_timer_sync(&probe->age_timer);   
    //tenda_wlan_proc_exit();
    //td_sta_steer_collect = NULL;
    //td_steer_bss_update = NULL;
    printk("---DEV PROBE EXIT ---\n");
}

