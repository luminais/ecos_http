/***********************************************************
 Copyright (C), 1998-2015, Tenda Tech. Co., Ltd.

 FileName : wlconf_rltk.c
 Description : 无线配置模块，读取nvram参数，将参数同步到驱动内核空间
 Author : fh
 Version : V1.0
 Date :2015-12-30

 Function List:
 	1.wlconf	----------配置无线参数
 	2.wlconf_down	------关闭无线接口
 	3.wlconf_start	------打开无线接口
 Others :


 History :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期    ：2016-01-08    修改者：fh
   内容     ：增加wisp 和 ap+client。将之前临时新增nvram规范化，并存放到envram中
   Version ：V1.1
   Other:
   修改nvram 包含增加envram
envram set HW_WLAN0_RF_TYPE=10
envram set HW_WLAN0_REG_DOMAIN=3
envram set HW_WLAN0_LED_TYPE=0
envram set HW_BOARD_VER=1
envram set HW_WLAN0_11N_XCAP=45
envram set HW_WLAN0_11N_THER=38

	增加nvram 和envram
envram set HW_WLAN0_TX_POWER_CCK_A=2b2b2b2b2b2b2b2b2b2b2b2b2b2b
envram set HW_WLAN0_TX_POWER_CCK_B=2b2b2b2b2b2b2b2b2b2b2b2b2b2b
envram set HW_WLAN0_TX_POWER_HT40_1S_A=2c2c2c2c2c2c2c2c2c2c2c2c2c2c
envram set HW_WLAN0_TX_POWER_HT40_1S_B=2c2c2c2c2c2c2c2c2c2c2c2c2c2c
envram set HW_WLAN0_TX_POWER_DIFF_HT40_2S=0000000000000000000000000000
envram set HW_WLAN0_TX_POWER_DIFF_HT20=ffffffffffffffffffffffffffff
envram set HW_WLAN0_TX_POWER_DIFF_OFDM=ffffffffffffffffffffffffffff

	没有改动的nvram
nvram set rltk_wisp_wlan_id=0
nvram set rltk_wlan_macclone_enable=0
nvram set rltk_wlan_network_type=0
nvram set rltk_wlan_tx_beamforming=0
nvram set rltk_wlan_iapp_disable=0
nvram set rltk_wlan_protection_disable=1
nvram set rltk_wlan_short_gi=1
nvram set rltk_wlan_aggregation=1
nvram set rltk_wlan_wmm_enable=1

 History :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期    ：2016-02-25    修改者：fh
   内容     ：完善前期忽略的参数配置
   Version ：V1.1
   Other:
	新增envram和nvram
HW_WLAN0_11N_TSSI1=0
HW_WLAN0_11N_TSSI2=0
HW_WLAN0_11N_TRSWITCH=0
HW_WLAN0_11N_TRSWPAPE_C9=0
HW_WLAN0_11N_TRSWPAPE_CC=0
	新增nvram
rltk_wlan_ldpc_enable=1
rltk_wlan_ack_timeout=0
rltk_wlan_sta_num=0
rltk_wlan_block_relay=0
rltk_wlan_wpa_group_rekey_time=86400
rltk_wlan_stbc_enable=1
rltk_wlan_preamble=0

History :
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期    ：2016-03-1    修改者：dxd
   内容     ：
   Version ：V1.1
   Other:
    将HW_WLAN0_XXX的获取方式统一修改成16进制读取(atoi -> strtoul)

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 History :
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 1) 日期	：2016-03-1    修改者：fh
	内容	 ：切换中继模式下的主副接口配置
	Version ：V1.2
	Other:关联修改
	1、wlrelay.h中的宏WL_CLIENT_IFNAME
	2、wireless.c中new_set_wireless_wisp函数nvram参数lan、wan接口名

 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


 *******************************************************************/


/* System include files */
#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/kernel/kapi.h>
#include <cyg/io/flash.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <ctype.h>
//#include <regex.h>
#include <time.h>
#include <netinet/rtl_vlan.h>

/* no System include files */
#include<nvram.h>
#include<shutils.h>
#include<sys_utility.h>

/*no System include files */
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"

#include "wifi.h"

typedef enum { GATEWAY_MODE=0, BRIDGE_MODE=1, WISP_MODE=2 } OPMODE_T;
typedef enum { AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3 } WLAN_MODE_T;
#define MAX_2G_CHANNEL_NUM_MIB		14
#define MAX_5G_CHANNEL_NUM_MIB		196
static inline int
nvram_match(char *name, char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

#define nvram_safe_get(name) (nvram_get(name) ? : "")

#define IFUP	(IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)//用tenda_ifconfig替换RunSystemCmd(NULL_FILE, "ifconfig", name,....

#define WLDBG  //调试打印开关

//虚拟AP功能需要开启，但由于ecos-work目录中的宏无法引到userspace目录故在此定义MBSSID
#define MBSSID
#define B1_G1	40
#define B1_G2	48

#define B2_G1	56
#define B2_G2	64

#define B3_G1	104
#define B3_G2	112
#define B3_G3	120
#define B3_G4	128
#define B3_G5	136
#define B3_G6	144

#define B4_G1	153
#define B4_G2	161
#define B4_G3	169
#define B4_G4	177

void assign_diff_AC(unsigned char* pMib, unsigned char* pVal)
{
	memset((pMib+35), pVal[0], (B1_G1-35));
	memset((pMib+B1_G1), pVal[1], (B1_G2-B1_G1));
	memset((pMib+B1_G2), pVal[2], (B2_G1-B1_G2));
	memset((pMib+B2_G1), pVal[3], (B2_G2-B2_G1));
	memset((pMib+B2_G2), pVal[4], (B3_G1-B2_G2));
	memset((pMib+B3_G1), pVal[5], (B3_G2-B3_G1));
	memset((pMib+B3_G2), pVal[6], (B3_G3-B3_G2));
	memset((pMib+B3_G3), pVal[7], (B3_G4-B3_G3));
	memset((pMib+B3_G4), pVal[8], (B3_G5-B3_G4));
	memset((pMib+B3_G5), pVal[9], (B3_G6-B3_G5));
	memset((pMib+B3_G6), pVal[10], (B4_G1-B3_G6));
	memset((pMib+B4_G1), pVal[11], (B4_G2-B4_G1));
	memset((pMib+B4_G2), pVal[12], (B4_G3-B4_G2));
	memset((pMib+B4_G3), pVal[13], (B4_G4-B4_G3));

}

static inline char* get_HWparameters_fromNvram(char* ifname,char* name)
{
	char temp[PI_BUFLEN_512]= {0};

	if(NULL == ifname || NULL == name)
		return "";

	if(0 == strncmp(ifname,TENDA_WLAN5_AP_IFNAME,5))
	{
		strcat_r("HW_WLAN0_",name,temp);
	}
	if(0 == strncmp(ifname,TENDA_WLAN24_AP_IFNAME,5))
	{
		strcat_r("HW_WLAN1_",name,temp);
	}

	return nvram_safe_get(temp);
}
/*lq 将配置射频参数的提取出来，放在外面*/
void config_hw_var(struct wifi_mib *pmib,char*ifname)
{
	int i, intVal, intVal2, encrypt, enable1x, wep, mode, enable1xVxd,opmode;
	unsigned char buf1[1024], buf2[1024], mac[6];
	char * nvramval = NULL;
			// set RF parameters
		//apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"RF_TYPE"),NULL,16);
	//没有看到什么地方设置该flash值,硬件参数值为:10
	//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_RF_TYPE:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);
	pmib->dot11RFEntry.dot11RFType = intVal;


	//apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
	intVal =atoi(nvram_safe_get("HW_BOARD_VER"));
	//没有看到什么地方设置该flash值，硬件参数天线:1
	//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_BOARD_VER:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);
	if (intVal == 1)
		pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
	else if(intVal == 2)
		pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
	else
		pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R

	if(get_wlan_id(ifname))
	{
		//默认功率
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_CCK_A");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_CCK_A, (void *)buf1);
		//硬件参数，flash all中为14个16进制2b，字符+
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_A, buf1, MAX_2G_CHANNEL_NUM_MIB);
		//WLDBG("\033[32m[%s]->[%s]->[%d]pmib->dot11RFEntry.pwrlevelCCK_A:%s\033[0m\n",__FILE__,__func__,__LINE__,pmib->dot11RFEntry.pwrlevelCCK_A);


		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_CCK_B");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_CCK_B, (void *)buf1);
		//硬件参数，flash all中为14个16进制2b，字符+
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
		//WLDBG("\033[32m[%s]->[%s]->[%d]pmib->dot11RFEntry.pwrlevelCCK_B:%s\033[0m\n",__FILE__,__func__,__LINE__,pmib->dot11RFEntry.pwrlevelCCK_B);

		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_HT40_1S_A");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)buf1);
		//硬件参数，flash all中为14个16进制2c，字符,
		//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_TX_POWER_HT40_1S_A:%s\033[0m\n",__FILE__,__func__,__LINE__,buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);

		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_HT40_1S_B");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)buf1);
		//硬件参数，flash all中为14个16进制2c，字符,
		//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_TX_POWER_HT40_1S_B:%s\033[0m\n",__FILE__,__func__,__LINE__,buf1);
		//WLDBG("\033[32m[%s]->[%s]->[%d]pmib->dot11RFEntry.pwrlevelHT40_1S_B:%s\033[0m\n",__FILE__,__func__,__LINE__,pmib->dot11RFEntry.pwrlevelHT40_1S_B);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);

		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_HT40_2S");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_DIFF_HT40_2S, (void *)buf1);
		//14个字节全0
		memcpy(pmib->dot11RFEntry.pwrdiffHT40_2S, buf1, MAX_2G_CHANNEL_NUM_MIB);

		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_HT20");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_DIFF_HT20, (void *)buf1);
		//14个字节全1
		memcpy(pmib->dot11RFEntry.pwrdiffHT20, buf1, MAX_2G_CHANNEL_NUM_MIB);


		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_OFDM");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		//apmib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)buf1);
		//14个字节全1
		memcpy(pmib->dot11RFEntry.pwrdiffOFDM, buf1, MAX_2G_CHANNEL_NUM_MIB);
	}
	else{

#if defined(CONFIG_RTL_92D_SUPPORT)|| defined(CONFIG_RTL_8812_SUPPORT)
			//apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_5G_HT40_1S_A");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_5G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A, buf1, MAX_5G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_5G_HT40_1S_B");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_5G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B, buf1, MAX_5G_CHANNEL_NUM_MIB);
#if 0
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT40_2S, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_HT40_2S");
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_5G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff5GHT40_2S, buf1, MAX_5G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT20, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_HT20");
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_5G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff5GHT20, buf1, MAX_5G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_OFDM, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_OFDM");
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_5G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff5GOFDM, buf1, MAX_5G_CHANNEL_NUM_MIB)
#endif
#endif

//#if defined(CONFIG_RTL_8812_SUPPORT)
		// 5G
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_20BW1S_OFDM1T_A");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A,buf2);
		
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_40BW2S_20BW2S_A");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A,buf2);
#if 1		
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_80BW1S_160BW1S_A");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A,buf2);


		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_80BW2S_160BW2S_A");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A,buf2);
#endif
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_20BW1S_OFDM1T_B");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B,buf2);

		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_40BW2S_20BW2S_B");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B,buf2);
#if 1		
		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_80BW1S_160BW1S_B");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B,buf2);


		//apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_5G_80BW2S_160BW2S_B");
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B,buf2);
#endif
	}
	if(get_wlan_id(ifname))
	{
		// 2G
		//apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_20BW1S_OFDM1T_A");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A, buf1, MAX_2G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_40BW2S_20BW2S_A");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_20BW1S_OFDM1T_B");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B, buf1, MAX_2G_CHANNEL_NUM_MIB);

		//apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, (void *)buf1);
		nvramval=get_HWparameters_fromNvram(ifname,"TX_POWER_DIFF_40BW2S_20BW2S_B");
		memset(buf1,0x0,1024);
		memset(buf2,0x0,1024);
		string_to_hex(nvramval, buf2, strlen(nvramval));
		memcpy(buf1, buf2, MAX_2G_CHANNEL_NUM_MIB);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	}
//#endif

	//apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_TSSI1"),NULL,16);
	//值为0
	//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_11N_TSSI1:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);
	pmib->dot11RFEntry.tssi1 = intVal;

	//apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_TSSI2"),NULL,16);
	//值为0
	//WLDBG("\033[32m[%s]->[%s]->[%d]MIB_HW_11N_TSSI2:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);
	pmib->dot11RFEntry.tssi2 = intVal;


	//apmib_get(MIB_HW_11N_THER, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_THER"),NULL,16);
	//值为:38
	pmib->dot11RFEntry.ther = intVal;
	//apmib_get(MIB_HW_11N_TRSWITCH, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_TRSWITCH"),NULL,16);
	//值为0
	pmib->dot11RFEntry.trswitch = intVal;

	//apmib_get(MIB_HW_11N_TRSWPAPE_C9, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_TRSWPAPE_C9"),NULL,16);
	//值为0
	//WLDBG("\033[32m[%s]->[%s]->[%d]dot11RFEntry.trsw_pape_C9:%d\033[0m\n",__FILE__,__func__,__LINE__,pmib->dot11RFEntry.trsw_pape_C9);
	pmib->dot11RFEntry.trsw_pape_C9 = intVal;

	//apmib_get(MIB_HW_11N_TRSWPAPE_CC, (void *)&intVal);
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_TRSWPAPE_CC"),NULL,16);
	//值为0
	pmib->dot11RFEntry.trsw_pape_CC = intVal;

	//apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
	//intVal =atoi(nvram_safe_get("HW_WLAN0_11N_XCAP"));
	intVal =strtoul(get_HWparameters_fromNvram(ifname,"11N_XCAP"),NULL,16);

	//值为45
	pmib->dot11RFEntry.xcap = intVal;

}

/*
lq  功能描述  : 0  5   1不是很清楚几个参数的含义，参考SDK配置渲的
*/
int get_vwlan_id(char *ifname)
{	
	if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME)
		|| !strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
	{
		return 0;
	}
#ifdef __CONFIG_GUEST__
	else if(!strcmp(ifname,TENDA_WLAN24_GUEST_IFNAME)
		|| !strcmp(ifname,TENDA_WLAN5_GUEST_IFNAME))
	{
		return 1;
	}
#endif
	else if(!strcmp(ifname,TENDA_WLAN24_VIRTUAL_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN5_VIRTUAL_IFNAME))
	{
		return 2;
	}
	else if(!strcmp(ifname,TENDA_WLAN24_REPEATER_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN5_REPEATER_IFNAME))
	{
		return 5;
	}
	
}

/*
 lq 功能描述  : 该函数是获取24G和5G索引的，用于wlconfg
*/
int get_wlan_id(char *ifname)
{	
	if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN24_REPEATER_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN24_GUEST_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN24_VIRTUAL_IFNAME))
	{
		return 1;
	}
	else if(!strcmp(ifname,TENDA_WLAN5_AP_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN5_REPEATER_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN5_GUEST_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN5_VIRTUAL_IFNAME))
	{
		return 0;
	}
}
/*
 lq 功能描述  : 根据工作模式，转换成wlconfg的判断问
*/
OPMODE_T get_wlan_opmode()
{
		//设置opmode和mode
	if(nvram_match(SYSCONFIG_WORKMODE,"wisp"))
	{
		return WISP_MODE;
	}
	else if(nvram_match(SYSCONFIG_WORKMODE,"client+ap"))
	{
		return BRIDGE_MODE;
	}
	else
	{
		return GATEWAY_MODE;
	}
}
/*
 lq 功能描述  : 桥接接口就是client，ap接口就是ap
*/
static WLAN_MODE_T get_wlan_mode(char* ifname)
{
		//设置opmode和mode
	if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN5_AP_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN5_GUEST_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN24_GUEST_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN24_VIRTUAL_IFNAME)
	   || !strcmp(ifname,TENDA_WLAN5_VIRTUAL_IFNAME))
	{
		return AP_MODE;
	}
	else if(!strcmp(ifname,TENDA_WLAN5_REPEATER_IFNAME)
	        || !strcmp(ifname,TENDA_WLAN24_REPEATER_IFNAME))
	{
		return CLIENT_MODE;
	}
}


static inline char * wl_strcat(const char *s1, const char *s2, char *buf)
{
	strcpy(buf, s1);
	strcat(buf, s2);
	return buf;
}
/*****************************************************************************
 函 数 名  : generate_wlan_configuration
 功能描述  : 在配置无线接口的时候，
 				都是将参数拷贝到wl_conf里面进行配置的，如
             			果后期需要添加参数，则需要在过滤数组里
             			面添加需要的配置
 输入参数  : char* ifname  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2017年10月27日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
char *wlan_config[] = 
{
	"ssid","hwaddr","radio","encode","closed","akm","wep","wpa_psk",
	"crypto","nettype","channel","bandside","bandwidth","ctv_power",
	"country_pwr_power","offset_power","mode","ifname",
	"",
};

static void  generate_wlan_configuration(char* ifname)
{
	int index = 0;
	char*value = NULL;
	char key[512] = {0};
	char src_prefix[32]={0};
	char dst_prefix[32]="wl_conf_";
	
	if(!strcmp(ifname,TENDA_WLAN24_AP_IFNAME))
	{
		strcpy(src_prefix,"wl0_");
	}
	else if(!strcmp(ifname,TENDA_WLAN5_AP_IFNAME))
	{
		strcpy(src_prefix,"wl1_");
	}
	else if(!strcmp(ifname,TENDA_WLAN24_REPEATER_IFNAME))
	{
		strcpy(src_prefix,"wl0.1_");
	}
	else if(!strcmp(ifname,TENDA_WLAN5_REPEATER_IFNAME))
	{
		strcpy(src_prefix,"wl1.1_");
	}
	else if(!strcmp(ifname,TENDA_WLAN24_GUEST_IFNAME))
	{
		strcpy(src_prefix,"wl0.2_");
	}
	else if(!strcmp(ifname,TENDA_WLAN5_GUEST_IFNAME))
	{
		strcpy(src_prefix,"wl1.2_");
	}	
	else if(!strcmp(ifname,TENDA_WLAN24_VIRTUAL_IFNAME))
	{
		strcpy(src_prefix,"wl0.3_");
	}
	else if(!strcmp(ifname,TENDA_WLAN5_VIRTUAL_IFNAME))
	{
		strcpy(src_prefix,"wl1.3_");
	}

	for(index = 0;strcmp(wlan_config[index],"");index++)
	{
		value = nvram_get(wl_strcat(src_prefix,wlan_config[index],key));
		if(value != NULL)
		{
			nvram_set(wl_strcat(dst_prefix,wlan_config[index],key),value);
		}
	}

}
/************************************************************
Function:	 wlconf
Description: 配置无线参数
Input:
	接口名称(如wlan0)
Output:
	无
Return:
	0	配置成功
	-1	初始化失败，配置失败
Others:
************************************************************/
int wlconf(char *ifname)
{
	struct wifi_mib *pmib;
	int i, intVal, intVal2, encrypt, enable1x, wep, mode, enable1xVxd,opmode;
	unsigned char buf1[1024], buf2[1024], mac[6];
	int skfd;
	struct iwreq wrq, wrq_root;
	int wlan_band=0, channel_bound=0, aggregation=0;
	struct wdsEntry *wds_Entry=NULL;
	int vwlan_idx=0;
	char tmp[100];
	char * nvramval;
#ifdef MBSSID
	int v_previous=0;
#ifdef CONFIG_RTL_819X
	int vap_enable=0, intVal4=0;
#endif
#endif
	if (0 != nvram_init())
	{
		WLDBG("Initialize nvram failed!\n");
		goto InitError;
	}
	
	WLDBG("wlconf(%s) \n",ifname);
	/*需要配置的接口参数放在wl_conf_   里面*/
	generate_wlan_configuration(ifname);

	vwlan_idx = get_vwlan_id(ifname);

	//设置opmode和mode

	opmode = get_wlan_opmode();
	mode = get_wlan_mode(ifname);

	/****************************lq begin wl   config******************************/
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0)
	{
		WLDBG("socket() fail\n");
		return -1;
	}
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0)
	{
		WLDBG("Interface %s open failed!\n", ifname);
		close(skfd);
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL)
	{
		WLDBG("MIB buffer allocation failed!\n");
		close(skfd);
		return -1;
	}

	// Disable WLAN MAC driver and shutdown interface first

	WLDBG("\033[32m[%s]->[%s]->[%d]Disable WLAN MAC driver and shutdown interface first\033[0m\n",__FILE__,__func__,__LINE__);

	if (is_interface_up(ifname))
	{
		RunSystemCmd(NULL_FILE, "ifconfig", ifname, "down", NULL_STR);
	}
	/* clean up tmp */
	memset(tmp, 0, sizeof(tmp));
	//vxd 或者va
	if (vwlan_idx != 0)
	{
		// virtual interface
		sprintf((char *)buf1, "wlan%d", get_wlan_id(ifname));
		strncpy(wrq_root.ifr_name, (char *)buf1, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0)
		{
			WLDBG("Root Interface %s open failed!\n", buf1);
			goto InitError;
		}
	}
	if (vwlan_idx == 0)
	{
		intVal =strtoul(get_HWparameters_fromNvram(ifname,"RF_TYPE"),NULL,16);

		if (intVal == 0)
		{
			WLDBG("RF type is NULL!\n");
			goto InitError;
		}
	}

	//该接口是否启用
	intVal = atoi(nvram_safe_get("wl_conf_radio"));

	if(intVal == 0)
		goto InitError;
	
	//无线是否启用，默认启用,值为0

	WLDBG("\033[32m[%s]->[%s]->[%d]MIB_WLAN_WLAN_DISABLED:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);


	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (vwlan_idx == 0)
	{
		if (ioctl(skfd, 0x8B42, &wrq) < 0)
		{
			WLDBG("Get WLAN MIB failed!\n");
			goto InitError;
		}
	}
	else
	{
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, 0x8B42, &wrq_root) < 0)
		{
			WLDBG("Get WLAN MIB failed!\n");
			goto InitError;
		}
	}

	// check mib version
	if (pmib->mib_version != MIB_VERSION)
	{
		WLDBG("WLAN MIB version mismatch!\n");
		goto InitError;
	}

	if (vwlan_idx > 0)
	{
		//if not root interface, clone root mib to virtual interface
		wrq.u.data.pointer = (caddr_t)pmib;
		wrq.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, 0x8B43, &wrq) < 0)
		{
			WLDBG("Set WLAN MIB failed!\n");
			goto InitError;
		}
		pmib->miscEntry.func_off = 0;
	}

	// Set parameters to driver
	if (vwlan_idx == 0)
	{
		intVal =strtoul(get_HWparameters_fromNvram(ifname,"REG_DOMAIN"),NULL,16);
		//值来自无线基本设置,属于硬件参数，值为:3，页面找不到相应的控件，regdomain0=3
		pmib->dot11StationConfigEntry.dot11RegDomain = intVal;
	}

	memset(mac,0,6);
	ether_atoe(nvram_safe_get("wl_conf_hwaddr"),mac);//中继模式下，主副接口使用同一个mac地址

	//虚拟接口的MAC地址我们自己计算
	//flash all出来的值为全0
	if (!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6))
	{
		printf("========MAC is NULL====%s [%d]\n", __FUNCTION__, __LINE__);
	}

	nvramval=nvram_safe_get("wl_conf_ssid");
	memset(buf1,0,1024);//没有清除会导致ssid超出32字节系统会异常重启
	
	memcpy(buf1,nvramval,strlen(nvramval));
	intVal2 = strlen((char *)buf1);

	WLDBG("\033[32m[%s]->[%s]->[%d]ssid:%s\033[0m\n",__FILE__,__func__,__LINE__,buf1);

	pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
	memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
	memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

	if ((pmib->dot11StationConfigEntry.dot11DesiredSSIDLen == 3) &&
	    ((pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'A') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'a')) &&
	    ((pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'N') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'n')) &&
	    ((pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'Y') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'y')))
	{
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	}
	else
	{
		if(intVal2 > 32)
		{
			WLDBG("The SSID of %s is too long!\n",ifname);
			goto InitError;
		}
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = intVal2;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, buf1, intVal2);
	}


	set_mac_address(ifname, (char *)mac);
	memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), mac, 6);


#ifdef BR_SHORTCUT
	if (opmode == WISP_MODE
#ifdef MBSSID
	    && vwlan_idx == 0
#endif
	   )
		pmib->dot11OperationEntry.disable_brsc = 1;
#endif

	intVal =strtoul(get_HWparameters_fromNvram(ifname,"LED_TYPE"),NULL,16);
	pmib->dot11OperationEntry.ledtype = intVal;



	WLDBG("\033[32m[%s]->[%s]->[%d]mode:%d\033[0m\n",__FILE__,__func__,__LINE__,mode);


	//AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3, AP_MESH_MODE=4, MESH_MODE=5
	//lq 有待更新
	if (mode == CLIENT_MODE)
	{
		intVal2 =atoi(nvram_safe_get("rltk_wlan_network_type"));

		WLDBG("\033[32m[%s]->[%s]->[%d]MIB_WLAN_NETWORK_TYPE:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal2);

		//无线站点模式或点对点模式，一般只用站点模式:0
		//pmib->dot11OperationEntry.opmode;16 –AP, 8 –Infrastructure client,32 - Ad-hoc client
		if (intVal2 == 0)
		{
			pmib->dot11OperationEntry.opmode = 8;
		}
		else
		{
		}
	}
	else
		pmib->dot11OperationEntry.opmode = 16;

	if (mode == WDS_MODE)	// WDS only
		pmib->dot11WdsInfo.wdsPure = 1;
	else
		pmib->dot11WdsInfo.wdsPure = 0;

	if (vwlan_idx == 0)
	{
		//射频参数配置全部在该函数内部完成
		config_hw_var(pmib,ifname);

		if (pmib->dot11RFEntry.dot11RFType == 10)
		{

			intVal =atoi(nvram_safe_get("wl_conf_offset_power"));//功率偏移值

			if (intVal)
			{
				for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				{
					if(pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevelCCK_A[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelCCK_A[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
					}
					if(pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevelCCK_B[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelCCK_B[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
					}

					if(pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
					}
					if(pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
					}

				}
#if defined(CONFIG_RTL_92D_SUPPORT)	|| defined(CONFIG_RTL_8812_SUPPORT)
				for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				{
					if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;
					}
					if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0)
					{
						if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
							pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= intVal;
						else
							pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
					}
				}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)						
			}
		}

		intVal =atoi(nvram_safe_get("wl_conf_channel"));

		WLDBG("\033[32m[%s]->[%s]->[%d]channel:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);
		if(opmode == GATEWAY_MODE)
		{
			//信道，0为自动信道，sdk中默认11
			pmib->dot11RFEntry.dot11channel = intVal;
        		pmib->dot11nConfigEntry.td_acs_enable = intVal ? 0 : 1;
		}else
		{
			//桥接模式下设置为固定信道
			if(get_wlan_id(ifname))
			{
				//信道，0为自动信道，sdk中默认11
				pmib->dot11RFEntry.dot11channel = 6;
        			pmib->dot11nConfigEntry.td_acs_enable = 0;
			}else
			{
				//信道，0为自动信道，sdk中默认11
				if(nvram_match("country_code","ID"))
				{
					pmib->dot11RFEntry.dot11channel = 149;
        				pmib->dot11nConfigEntry.td_acs_enable = 0;
				}else
				{
					pmib->dot11RFEntry.dot11channel = 36;
        				pmib->dot11nConfigEntry.td_acs_enable = 0;
				}
			}
		}

		intVal =atoi(nvram_safe_get("rltk_wlan_preamble"));

		//默认值为0
		pmib->dot11RFEntry.shortpreamble = intVal;

		intVal =atoi(nvram_safe_get("rltk_wlan_stbc_enable"));
		//默认值1，Space-time block coding空时块编码
		pmib->dot11nConfigEntry.dot11nSTBC = intVal;

		intVal =atoi(nvram_safe_get("rltk_wlan_ldpc_enable"));
		//默认值为1。LDPC码即低密度奇偶校验码（Low Density Parity Check Code，LDPC）

		WLDBG("\033[32m[%s]->[%s]->[%d]%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);

		pmib->dot11nConfigEntry.dot11nLDPC = intVal;

		intVal =atoi(nvram_safe_get("rltk_wlan_ack_timeout"));

		//默认值为0，高级设置cgi中有设置函数，但是页面没有看到控件
		pmib->miscEntry.ack_timeout = intVal;

		intVal =atoi(nvram_safe_get("rltk_wlan_tx_beamforming"));

		//默认值为0，Beamforming即波束成形，是通用信号处理技术，用于控制传播的方向和射频信号的接收
		pmib->dot11RFEntry.txbf = intVal;

		intVal =atoi(nvram_safe_get("rltk_wlan_iapp_disable"));
		//默认值为0
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;

		//max station num
		intVal =atoi(nvram_safe_get("rltk_wlan_sta_num"));
		//默认值0,	 范围0-32. 0 –disable (not limit).
		pmib->dot11StationConfigEntry.supportedStaNum = intVal;

		// set 11g protection mode
		intVal =atoi(nvram_safe_get("rltk_wlan_protection_disable"));
		//默认值为1
		pmib->dot11StationConfigEntry.protectionDisabled = intVal;

		// set block relay
		intVal =atoi(nvram_safe_get("rltk_wlan_block_relay"));
		//默认值为0
		pmib->dot11OperationEntry.block_relay = intVal;


		pmib->dot11WdsInfo.wdsEnabled = 0;
		pmib->dot11WdsInfo.wdsNum = 0;


		pmib->dot11StationConfigEntry.dot11AclNum = 0;

#ifdef __CONFIG_WL_MAC_FILTER__
		char *next = NULL, var[64] = {0};
		char wl_filter_mode[16] = {0};
		unsigned char mac_addr[6] = {0};
		i = 0;
		/*lq无线访问控制只需要在配置主接口的时候进行配置，次接口的时候不需要配置*/
		if(strcmp(ifname,TENDA_WLAN24_AP_IFNAME) == 0 || strcmp(ifname,TENDA_WLAN5_AP_IFNAME) == 0)
		{
			strcpy(wl_filter_mode, nvram_safe_get(ADVANCE_MACFILTER_MODE));

			if(0 == strcmp(wl_filter_mode, "disabled"))
			{
				pmib->dot11StationConfigEntry.dot11AclMode = 0;
			}
			else
			{
				if(0 == strcmp(wl_filter_mode, "deny")) 		/*仅禁止*/
				{
					pmib->dot11StationConfigEntry.dot11AclMode = 2;
					
				}
				else if(0 == strcmp(wl_filter_mode, "pass")) 	/*仅允许*/
				{
					pmib->dot11StationConfigEntry.dot11AclMode = 1;
				}
				else
				{
					pmib->dot11StationConfigEntry.dot11AclMode = 0;
				}
				foreach(var, nvram_safe_get(ADVICE_MAC_LIST), next) 
				{
					if (ether_atoe(var, &mac_addr)) 
					{
						memcpy(&(pmib->dot11StationConfigEntry.dot11AclAddr[i][0]), &(mac_addr[0]), 6);
						pmib->dot11StationConfigEntry.dot11AclNum++;
						i++;
					}
				}

			}

		}

#endif

		// set nat2.5 disable when client and mac clone is set
		intVal =atoi(nvram_safe_get("rltk_wlan_macclone_enable"));
		//默认值为0
		if ((intVal == 1) && (mode == CLIENT_MODE))
		{
			//AP_MODE=0, CLIENT_MODE=1, WDS_MODE=2, AP_WDS_MODE=3, AP_MESH_MODE=4, MESH_MODE=5
			pmib->ethBrExtInfo.macclone_enable = 1;
		}
		else
		{
			pmib->ethBrExtInfo.macclone_enable = 0;
		}

		WLDBG("\033[32m[%s]->[%s]->[%d]opmode:%d\033[0m\n",__FILE__,__func__,__LINE__,opmode);

		if (opmode == WISP_MODE)
		{
			pmib->ethBrExtInfo.nat25_disable = 0;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}

#ifdef WIFI_SIMPLE_CONFIG
		pmib->wscEntry.wsc_enable = 0;
#endif

		if(nvram_match("wl_conf_bandwidth","auto"))
		{

			if(get_wlan_id(ifname))
				channel_bound =1;
			else
			{
				channel_bound =2;
				//俄罗斯国家不支持80M
				if(0 == strcmp(nvram_safe_get("country_code"),"RU"))
					channel_bound =1;
			}
			if(opmode == GATEWAY_MODE)
			{
				pmib->dot11nConfigEntry.td_width_enable = 1;
			}
			else
			{
			    //桥接模式下关闭自动频宽功能
				pmib->dot11nConfigEntry.td_width_enable = 0;
			}
		}else
		{
			channel_bound =atoi(nvram_safe_get("wl_conf_bandwidth")) / 40;
			pmib->dot11nConfigEntry.td_width_enable = 0;
		}

		pmib->dot11nConfigEntry.dot11nUse40M = channel_bound;

		//apmib_get(MIB_WLAN_CONTROL_SIDEBAND, &intVal);
		//默认值为1 应该是向下扩展
		nvramval = nvram_safe_get("wl_conf_bandside");
		if(channel_bound ==0)
		{
			pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
		}
		else
		{
			// 扩展信道，向中间信道扩展，fromSetWifi中设置，Realtek驱动中设置如下
			if(!strcmp(nvramval,"lower"))
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;//向高信道扩展
			if(!strcmp(nvramval,"upper"))
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;//向低信道扩展
		}
    
        //国家代码传入驱动 
        nvramval = nvram_safe_get("country_code");
        memset(buf1, 0, 1024);
        memcpy(buf1, nvramval, strlen(nvramval));
        
        memset(pmib->dot11dCountry.dot11CountryString, 0, 3);
        memcpy(pmib->dot11dCountry.dot11CountryString, buf1, 3);

		intVal =atoi(nvram_safe_get("rltk_wlan_short_gi"));
		//默认值为1，保护间隔（Short Guard Interval)：是OFDM符号中用于减少符号间干扰的时间
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor80M = intVal;

		aggregation =atoi(nvram_safe_get("rltk_wlan_aggregation"));
		//默认值为1，应该是Frame Aggregation多个帧聚合成一个来发
		if(aggregation ==0)
		{
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}
		else if(aggregation ==1)
		{
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}
		else if(aggregation ==2)
		{
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}
		else if(aggregation ==3)
		{
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}

#if defined(CONFIG_RTL_819X) && defined(MBSSID)
		if(pmib->dot11OperationEntry.opmode & 0x00000010)
		{
			// AP mode  。16 –AP, 8 – Infrastructure client,32 – Ad-hoc client
			for (vwlan_idx = 1; vwlan_idx < RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM+1; vwlan_idx++)
			{
				//apmib_set_vwlanidx(vwlan_idx);
				//apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal4);

				if(1==atoi(nvram_safe_get(strcat_r("wl0.2_", "radio", tmp)))
					|| 1==atoi(nvram_safe_get(strcat_r("wl1.2_", "radio", tmp))))
				{
					intVal4 = 0;
				}

				WLDBG("\033[32m[%s]->[%s]->[%d]MIB_WLAN_WLAN_DISABLED:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal4);

				if (intVal4 == 0)
					vap_enable++;
				intVal4=0;
			}

			vwlan_idx = 0;
		}
		if (vap_enable && (mode == AP_MODE || mode == AP_WDS_MODE))
			pmib->miscEntry.vap_enable=1;
		else
			pmib->miscEntry.vap_enable=0;

		WLDBG("\033[32m[%s]->[%s]->[%d]vap_enable = %d(mib:%d)\033[0m\n",__FILE__,__func__,__LINE__,vap_enable,pmib->miscEntry.vap_enable);

#endif
	}
	//lq end vlan_idx == 0

	if (vwlan_idx != 5/*NUM_VWLAN_INTERFACE*/)
	{
		// set band
		nvramval = nvram_safe_get("wl_conf_nettype");

		if(0 == strcmp(nvramval, "bgn"))
		{
			intVal=11;//bgn
		}
		else if(0 == strcmp(nvramval, "gn"))
		{
			intVal=10;
		}
		else if(0 == strcmp(nvramval, "bg"))
		{
			intVal=3;//bg
		}
		else if(0 == strcmp(nvramval, "b"))
		{
			intVal=1;//b
		}
		else if(0 == strcmp(nvramval, "g"))
		{
			intVal=2;//g
		}
		else if(0 == strcmp(nvramval, "n_only"))
		{
			intVal=11;	// 按照AC6V1.0规格，2.4G n only与bgn做成一样。
		}
		else if(0 == strcmp(nvramval, "ac"))
		{
			// A+N+AC
			intVal=72;
		}
		else if(0 == strcmp(nvramval, "ac_only"))
		{
			// AC
			//intVal=64;
			intVal=72;	// 按照AC6V1.0规格，5G ac only与an+ac做成一样。
		}
		else if(0 == strcmp(nvramval, "a"))
		{
			// A
			intVal=4;
		}
		else if(0 == strcmp(nvramval, "an"))
		{
			// A+N
			intVal=8;
		}

		wlan_band = intVal;

		WLDBG("\033[32m[%s]->[%s]->[%d]wlan_band:%d\033[0m\n",__FILE__,__func__,__LINE__,wlan_band);

		if ((mode != 1) && (pmib->dot11OperationEntry.wifi_specific == 1) && (wlan_band == 2))
			wlan_band = 3;

		if (wlan_band == 8)
		{
			// pure-11n
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
			//lrl 临时解决编译问题 PHYBAND_5G=2 PHYBAND_2G = 1
			if(pmib->dot11RFEntry.phyBandSelect == 2)
			{
				wlan_band += 4; // a+n
				pmib->dot11StationConfigEntry.legacySTADeny = 4;
			}
			else if (pmib->dot11RFEntry.phyBandSelect == 1)
#endif
			{
				wlan_band += 3; // b+g+n
				pmib->dot11StationConfigEntry.legacySTADeny = 3;
			}
		}
		else if (wlan_band == 2)
		{
			// pure-11g
			//wlan_band += 1; // b+g
			pmib->dot11StationConfigEntry.legacySTADeny = 0;
		}
		else if (wlan_band == 10)
		{
			// g+n
			wlan_band += 1; // b+g+n
			pmib->dot11StationConfigEntry.legacySTADeny = 1;
		}
		else if (wlan_band == 64)
		{
			// pure-11ac
			wlan_band += 12; // a+n
			pmib->dot11StationConfigEntry.legacySTADeny = 12;
		}
		else if (wlan_band == 72)
		{
			//ac+n
			wlan_band += 4; //a
			pmib->dot11StationConfigEntry.legacySTADeny = 0;
		}
		else
			pmib->dot11StationConfigEntry.legacySTADeny = 0;

		pmib->dot11BssType.net_work_type = wlan_band;

		WLDBG("\033[32m[%s]->[%s]->[%d]net_work_type:%d\033[0m\n",__FILE__,__func__,__LINE__,wlan_band);

		intVal =atoi(nvram_safe_get("rltk_wlan_wmm_enable"));
		//设置wmm默认值为1
		pmib->dot11QosEntry.dot11QosEnable = intVal;
	}



	intVal =atoi(nvram_safe_get("wl_conf_closed"));
	//默认值为0 Hidden AP enable/disable	0 – disabled, 1 – enabled

	WLDBG("\033[32m[%s]->[%s]->[%d]hiddenap:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);

	pmib->dot11OperationEntry.hiddenAP = intVal;


	intVal =2;

	nvramval = nvram_safe_get("wl_conf_akm");
	if(!strcmp(nvramval,"psk"))
	{
		encrypt = 2;
	}
	else if(!strcmp(nvramval,"psk2"))
	{
		encrypt = 4;
	}
	else if(!strcmp(nvramval,"psk psk2"))
	{
		encrypt = 6;
	}
	else
	{
		encrypt = 0;
	}
	nvramval = nvram_safe_get("wl_conf_wep");
	if(!strcmp(nvramval,"enabled"))
	{
		encrypt = 1;
	}

	WLDBG("\033[32m[%s]->[%s]->[%d]encrypt:%d\033[0m\n",__FILE__,__func__,__LINE__,encrypt);

	//默认值为0，不加密; 4----wpa2; 	 1----wep;	   6---wpa mixed

	if ((intVal == 1) && (encrypt != 1))
	{
		intVal = 0;
	}
	//0 – open system, 1 – shared key,2 – auto
	pmib->dot1180211AuthEntry.dot11AuthAlgrthm = intVal;

	if (encrypt == 0)
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	else if (encrypt == 1)
	{

	}
	else
	{
		// WPA mode
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
	}

#ifndef CONFIG_RTL8196B_TLD
#ifdef MBSSID
	if (vwlan_idx > 0 && pmib->dot11OperationEntry.guest_access)
		pmib->dot11OperationEntry.block_relay = 1;
#endif
#endif
		//不支持802.1x默认设置为0
	pmib->dot118021xAuthEntry.dot118021xAlgrthm = 0;

	intVal = 0;
	pmib->dot118021xAuthEntry.acct_enabled = intVal;
	// When using driver base WPA, set wpa setting to driver
#if 1

	if (encrypt >= 2 )

	{
		if (encrypt == 2)
			intVal = 1;
		else if (encrypt == 4)
			intVal = 2;
		else if (encrypt == 6)
			intVal = 3;
		else
		{
			WLDBG("invalid ENCRYPT value!\n");
			goto InitError;
		}
		pmib->dot1180211AuthEntry.dot11EnablePSK = intVal;

		nvramval =nvram_safe_get("wl_conf_wpa_psk");
		memset(buf1,0,1024);
		memcpy(buf1,nvramval,strlen(nvramval));
		//默认值为空字符串,存放wpa psk密码

		WLDBG("\033[32m[%s]->[%s]->[%d]wpa_psk:%s\033[0m\n",__FILE__,__func__,__LINE__,buf1);

		strcpy((char *)pmib->dot1180211AuthEntry.dot11PassPhrase, (char *)buf1);

		//apmib_get(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
		intVal =atoi(nvram_safe_get("rltk_wlan_wpa_group_rekey_time"));
		//默认值为86400

		WLDBG("\033[32m[%s]->[%s]->[%d]%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);

		pmib->dot1180211AuthEntry.dot11GKRekeyTime = intVal;
	}
	else
		pmib->dot1180211AuthEntry.dot11EnablePSK = 0;

	if (/*intVal3 != 0 && */encrypt >= 2
	   )

	{
		if (encrypt == 2 || encrypt == 6) //6 --wpa 混合
		{
			//默认值为1
			nvramval = nvram_safe_get("wl_conf_crypto");
			if(!strcmp(nvramval,"tkip"))
			{
				intVal = 2;
			}
			else if(!strcmp(nvramval,"aes"))
			{
				intVal = 8;
			}
			else if(!strcmp(nvramval,"tkip+aes"))
			{
				intVal = 10;
			}
			else
			{
				WLDBG("invalid WPA2_CIPHER_SUITE value!\n");
				goto InitError;
			}


			WLDBG("\033[32m[%s]->[%s]->[%d]crypto:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);

			pmib->dot1180211AuthEntry.dot11WPACipher = intVal;
		}

		if (encrypt == 4 || encrypt == 6)// 4 -- wpa2;	  6 --wpa 混合
		{
			nvramval = nvram_safe_get("wl_conf_crypto");
			if(!strcmp(nvramval,"tkip"))
			{
				intVal = 2;
			}
			else if(!strcmp(nvramval,"aes"))
			{
				intVal = 8;
			}
			else if(!strcmp(nvramval,"tkip+aes"))
			{
				intVal = 10;
			}
			else
			{
				WLDBG("invalid WPA2_CIPHER_SUITE value!\n");
				goto InitError;
			}

	
			WLDBG("\033[32m[%s]->[%s]->[%d]crypto:%d\033[0m\n",__FILE__,__func__,__LINE__,intVal);

			pmib->dot1180211AuthEntry.dot11WPA2Cipher = intVal;
		}

        if (encrypt == 6){
			pmib->dot1180211AuthEntry.dot11WPACipher = 10;			
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = 10;
		}
	}
#endif


	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B43, &wrq) < 0)
	{
		WLDBG("Set WLAN MIB failed!\n");
		goto InitError;
	}


	close(skfd);
	if(pmib != NULL)
		free(pmib);
	return 0;
InitError:
	if(pmib != NULL)
		free(pmib);
	close(skfd);
	return -1;
}




/************************************************************
Function:	 wlconf_down
Description: 关闭接口
Input:
	接口名称(如wlan0)
Output:
	无
Return:
Others:
************************************************************/

int wlconf_down(char *name)
{

	WLDBG("\033[32m[%s]->[%s]->[%d]\033[0mname:%s\n",__FILE__,__func__,__LINE__,name);

	//RunSystemCmd(NULL_FILE, "ifconfig", name, "down", NULL_STR);
	tenda_ifconfig(name, 0, NULL, NULL);
	return 0;
}


/************************************************************
Function:	 wlconf_start
Description: 关闭接口
Input:
	接口名称(如wlan0)
Output:
	无
Return:
Others:
************************************************************/
int wlconf_start(char *name)
{

	WLDBG("\033[32m[%s]->[%s]->[%d]\033[0mname:%s\n",__FILE__,__func__,__LINE__,name);

	//RunSystemCmd(NULL_FILE, "ifconfig", name, "up", NULL_STR);


//	#ifdef __CONFIG_A9__
	//增加Intel网卡性能兼容性动态修改adaptivity和dig值
	char buf[128] = {0};
	char *nvramval=NULL;
//	sprintf((char *)buf, "td_adaptivity_mode=%d", atoi(nvram_safe_get("td_adaptivity_mode")));
//	RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);

	//BSPLJF++ 增加对8814AE 在复杂环境下，提升TX的配置。
	memset(buf, 0, sizeof(buf));
	nvramval = nvram_safe_get("adaptivity_enable");
	if(strcmp(nvramval, ""))
	{	
		sprintf((char *)buf, "adaptivity_enable=%d", atoi(nvramval));
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
	}
	
	//BSPLJF++ 增加对8814AE 连上一个设备后的抗干扰接收灵敏度调整的范围控制，td_dig_sel为十进制，对应驱动中的dig调整的上限。
	//及添加自适应
	memset(buf, 0x0, sizeof(buf));
	if (!strcmp(nvram_safe_get("td_auto_antiInterferance"), "1"))
	{
	
		WLDBG ("%s %d \n", __FUNCTION__, __LINE__);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib","td_auto_antiInterferance=1", NULL_STR);

        memset(buf, 0x0, sizeof(buf));
        nvramval = nvram_safe_get("td_FA_UpperBound");
        if(strcmp(nvramval, ""))
    	{	
    	    sprintf((char *)buf, "td_FA_UpperBound=%d", atoi(nvramval));
    		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
    	}

        memset(buf, 0x0, sizeof(buf));
        nvramval = nvram_safe_get("td_FA_LowerBound");
        if(strcmp(nvramval, ""))
    	{	
    	    sprintf((char *)buf, "td_FA_LowerBound=%d", atoi(nvramval));
    		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
    	}

        memset(buf, 0x0, sizeof(buf));
        nvramval = nvram_safe_get("td_FA_UpperBound_20M");
        if(strcmp(nvramval, ""))
    	{	
    	    sprintf((char *)buf, "td_FA_UpperBound_20M=%d", atoi(nvramval));
    		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
    	}

        memset(buf, 0x0, sizeof(buf));
        nvramval = nvram_safe_get("td_FA_LowerBound_20M");
        if(strcmp(nvramval, ""))
    	{	
    	    sprintf((char *)buf, "td_FA_LowerBound_20M=%d", atoi(nvramval));
    		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
    	}
        
	}
	else
	{
		WLDBG ("%s %d \n", __FUNCTION__, __LINE__);
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib","td_auto_antiInterferance=0", NULL_STR);
		sprintf((char *)buf, "td_dig_sel=%d", atoi(nvram_safe_get("td_dig_sel")));
		RunSystemCmd(NULL_FILE, "iwpriv", "wlan1", "set_mib",buf, NULL_STR);
	}
	
	tenda_ifconfig(name, IFUP, NULL, NULL);

#ifdef __CONFIG_WIFI_DOUBLEBAND_UNITY__
	if(0 == strcmp(name,TENDA_WLAN5_AP_IFNAME))
	{
		//双频优选开关
		memset(buf, 0x0, sizeof(buf));
		sprintf((char *)buf,"wlan0 steer write %d",atoi(nvram_safe_get("wl_doubleBandUn_enable")));
		run_clicmd(buf);
		
		//双频优选信号强度
		nvramval = nvram_safe_get("wl_doubleBandUn_lmt_rssi_2g");
		if(0 != strcmp(nvramval,""))
		{
			memset(buf, 0x0, sizeof(buf));
			sprintf((char *)buf,"wlan0 steer write 2,rssi_lmt_2g,%s",nvramval);
			run_clicmd(buf);
		}
		
		nvramval = nvram_safe_get("wl_doubleBandUn_lmt_rssi_5g");
		if(0 != strcmp(nvramval,""))
		{
			memset(buf, 0x0, sizeof(buf));
			sprintf((char *)buf,"wlan0 steer write 2,rssi_lmt_5g,%s",nvramval);
			run_clicmd(buf);
		}
		
		//双频优选是否开启设备平衡(5G连接数量和2.4G数量保持相对平衡)
		memset(buf, 0x0, sizeof(buf));
		sprintf((char *)buf,"wlan0 steer write 2,sta_balance_enable,%d",atoi(nvram_safe_get("wl_doubleBandUn_sta_balance_en")));
		run_clicmd(buf);
	}
#endif

    return 0;
}



