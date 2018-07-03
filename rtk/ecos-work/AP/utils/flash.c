/*
 *      Operation routines for FLASH MIB access
 *
 *      Authors: David Hsu	<davidhsu@realtek.com.tw>
 *
 *      $Id: flash.c,v 1.93 2009/09/15 02:12:32 bradhuang Exp $
 *
 */


/* System include files */
#include <network.h>
#include <pkgconf/devs_eth_rltk_819x_wlan.h>
#include <cyg/kernel/kapi.h>
#ifdef CYGPKG_IO_FLASH
#include <cyg/io/flash.h>
#endif
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
#if defined(CONFIG_RTL_VLAN_SUPPORT)
#include <netinet/rtl_vlan.h>
#define VLAN_LAN_PVID_DEF 9
#define VLAN_WAN_PVID_DEF 8
#endif
#if defined(CONFIG_RTL_VAP_SUPPORT)
#ifndef DEF_MSSID_NUM
#define DEF_MSSID_NUM RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM
#else
#define DEF_MSSID_NUM 0

#endif
#endif

#define noPARSE_TXT_FILE

#define WLAN_FAST_INIT
#define BR_SHORTCUT	

/* Local include files */
#include "apmib.h"
#include "mibtbl.h"
#include "sys_utility.h"
#include "net_api.h"

extern int rtk_flash_read(char *buf, int offset, int len);
extern int rtk_flash_write(char *buf, int offset, int len);
extern char *apmib_load_csconf(void);
extern char *apmib_load_dsconf(void);
extern char *apmib_load_hwconf(void);
extern int set_mac_address(const char *interface, char *mac_address);

#ifdef CONFIG_RTL_COMAPI_CFGFILE
extern int comapi_initWlan(char *ifname);
extern int dumpCfgFile(char *ifname, struct wifi_mib *pmib);
#endif

extern int file_gz_uncompress(char *infile, char *outfile);
extern int Encode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);
extern int Decode(unsigned char *ucInput, unsigned int inLen, unsigned char *ucOutput);
extern unsigned int mib_get_setting_len(CONFIG_DATA_T type);
extern unsigned int mib_tlv_save(CONFIG_DATA_T type, void *mib_data, unsigned char *mib_tlvfile, unsigned int *tlv_content_len);
extern unsigned int mib_get_real_len(CONFIG_DATA_T type);
extern unsigned int mib_get_flash_offset(CONFIG_DATA_T type);
extern time_t get_epoch_build_time(void);

/* Constand definitions */
#define DEC_FORMAT	("%d")
#define UDEC_FORMAT ("%u")
#define BYTE5_FORMAT	("%02x%02x%02x%02x%02x")
#define BYTE6_FORMAT	("%02x%02x%02x%02x%02x%02x")
#define BYTE13_FORMAT	("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x")
#define STR_FORMAT	("%s")
#define HEX_FORMAT	("%02x")
#define SCHEDULE_FORMAT	("%d,%d,%d,%d")
#ifdef HOME_GATEWAY
#define PORTFW_FORMAT	("%s, %d, %d, %d")
#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
//InstanceNum RootIdx VWlanIdx IsConfigured RfBandAvailable
#define CWMP_WLANCONF_FMT	("%d, %d, %d, %d, %d")
#endif
#define PORTFILTER_FORMAT ("%d, %d, %d")
#define IPFILTER_FORMAT	("%s, %d")
#define TRIGGERPORT_FORMAT ("%d, %d, %d, %d, %d, %d")
#endif
#define MACFILTER_FORMAT ("%02x%02x%02x%02x%02x%02x")
#define MACFILTER_COLON_FORMAT ("%02x:%02x:%02x:%02x:%02x:%02x")
#define WDS_FORMAT	("%02x%02x%02x%02x%02x%02x,%u")

#ifdef KLD_ENABLED
#define DHCPRSVDIP_FORMAT ("%02x%02x%02x%02x%02x%02x,%s,%s,%d")
#else
#define DHCPRSVDIP_FORMAT ("%02x%02x%02x%02x%02x%02x,%s,%s")
#endif

#define VLANCONFIG_FORMAT ("%s,%d,%d,%d,%d,%d,%d")
#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
//#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d, %d, %d, %d, %d, %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s")
#define IPSECTUNNEL_FORMAT ("%d, %d, %s, %d, %s, %d, %d, %s , %d, %s, %d, %d,  %d, %d,  %s, %d, %d, %d, %lu, %lu, %d, %s, %s, %s, %d, %s, %s, %d, %d, %s")
#endif
#ifdef CONFIG_IPV6
#define RADVD_FORMAT ("%u, %s, %u, %u, %u, %u, %u, %u, %u, %u ,%u, %u, %s, %u, %u, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %u, %u, %u, %u, %u, %u, %s, %u, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %u, %u, %u, %u, %u, %u, %s, %d")
#define DNSV6_FORMAT ("%d, %s")
#define DHCPV6S_FORMAT ("%d, %s, %s, %s, %s")
/*enable, ifname, pd's len, pd's id, pd's ifname*/
#define DHCPV6C_FORMAT ("%d, %s, %d, %d, %s")

#define ADDR6_FORMAT ("%d, %d, %d, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x")
#define ADDRV6_FORMAT ("%d, %04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x")

#endif
#endif

#ifdef CONFIG_RTL_WAPI_SUPPORT
#define CA_CERT "/var/myca/CA.cert"
#endif

#define SPACE	(' ')
#define EOL	('\n')

#define LOCAL_ADMIN_BIT 0x02

#ifdef CONFIG_RTL_SIMPLE_CONFIG
unsigned char sc_default_pin[]="57289961";
#endif


typedef enum { 
	HW_MIB_AREA=1, 
	HW_MIB_WLAN_AREA,
	DEF_MIB_AREA,
	DEF_MIB_WLAN_AREA,
	MIB_AREA,
	MIB_WLAN_AREA
} CONFIG_AREA_T;

static CONFIG_AREA_T config_area;
#if 0
/* Macro definition */
static int _is_hex(char c)
{
    return (((c >= '0') && (c <= '9')) ||
            ((c >= 'A') && (c <= 'F')) ||
            ((c >= 'a') && (c <= 'f')));
}

static int string_to_hex(char *string, unsigned char *key, int len)
{
	char tmpBuf[4];
	int idx, ii=0;
	for (idx=0; idx<len; idx+=2) {
		tmpBuf[0] = string[idx];
		tmpBuf[1] = string[idx+1];
		tmpBuf[2] = 0;
		if ( !_is_hex(tmpBuf[0]) || !_is_hex(tmpBuf[1]))
			return 0;

		key[ii++] = (unsigned char) strtol(tmpBuf, (char**)NULL, 16);
	}
	return 1;
}
#endif
static void convert_lower(char *str)
{	int i;
	int len = strlen(str);
	for (i=0; i<len; i++)
		str[i] = tolower(str[i]);
}

static int APMIB_GET(int id, void *val)
{
	if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA)
		return apmib_getDef(id, val);
	else
		return apmib_get(id, val);
}

static int APMIB_SET(int id, void *val)
{
	if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA)
		return apmib_setDef(id, val);
	else
		return apmib_set(id, val);
}

/* Local declarations routines */
//static int flash_read(char *buf, int offset, int len);
static int writeDefault(int isAll);
static int searchMIB(char *token);
static void getMIB(char *name, int id, TYPE_T type, int num, int array_separate, char **val);
static void setMIB(char *name, int id, TYPE_T type, int len, int valnum, char **val);
static void dumpAll(void);
static void dumpAllHW(void);
static void showHelp(void);
static void showAllMibName(void);
static void showAllHWMibName(void);
static void showSetACHelp(void);

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
static void showSetMeshACLHelp(void);
#endif
//static void showSetVlanConfigHelp(void);
static void showSetWdsHelp(void);
#ifdef TLS_CLIENT
static int read_flash_cert(char *prefix, char *certfile);
#endif
#ifdef VPN_SUPPORT
static int read_flash_rsa(char *prefix);
#endif
#ifdef HOME_GATEWAY
static void showSetPortFwHelp(void);
static void showSetPortFilterHelp(void);
static void showSetIpFilterHelp(void);
static void showSetMacFilterHelp(void);
static void showSetUrlFilterHelp(void);
static void showSetTriggerPortHelp(void);
#ifdef GW_QOS_ENGINE
//static void showSetQosHelp(void);
#endif
#ifdef ROUTE_SUPPORT
static void showSetStaticRouteHelp(void);
#endif

#ifdef VPN_SUPPORT
static void showSetIpsecTunnelHelp(void);
#endif
static int generatePPPConf(int is_pppoe, char *option_file, char *pap_file, char *chap_file);
#endif

#ifdef TLS_CLIENT
static void showSetCertRootHelp(void);
static void showSetCertUserHelp(void);
#endif

static void generateWpaConf(char *outputFile, int isWds);

#ifdef WLAN_FAST_INIT
static int initWlan(char *ifname);
#endif

#ifdef WIFI_SIMPLE_CONFIG
//static int updateWscConf(char *in, char *out, int genpin);
#endif

#ifdef COMPRESS_MIB_SETTING
int flash_mib_checksum_ok(int offset);
int flash_mib_compress_write(CONFIG_DATA_T type, char *data, TLV_PARAM_HEADER_T *pheader, unsigned char *pchecksum);
#endif

#ifdef MIB_TLV
//int mib_write_to_raw(const mib_table_entry_T *mib_tbl, void *data, char *pfile, unsigned int *idx);
#endif

#if defined(CONFIG_RTK_MESH)
static int set_mesh_mac_first = 1;
#endif

#ifdef PARSE_TXT_FILE
static int parseTxtConfig(char *filename, APMIB_Tp pConfig);
static int getToken(char *line, char *value);
static int set_mib(APMIB_Tp pMib, int id, void *value);
static void getVal2(char *value, char **p1, char **p2);
#ifdef HOME_GATEWAY
static void getVal3(char *value, char **p1, char **p2, char **p3);
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4);
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5);
#endif

static int acNum;

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
static int meshAclNum;
#endif

static int wdsNum;

#ifdef HOME_GATEWAY
static int macFilterNum, portFilterNum, ipFilterNum, portFwNum, triggerPortNum, staticRouteNum;
static int urlFilterNum;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
static int qosRuleNum;
#endif
#endif
static int dhcpRsvdIpNum;
#ifdef TLS_CLIENT
static int certRootNum, certUserNum ;
#endif
#if defined(VLAN_CONFIG_SUPPORTED)
static int vlanConfigNum=MAX_IFACE_VLAN_CONFIG;	
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
static int vlanNum=MAX_VLAN_NUM;
static int netIfNum=MAX_NETIFACE_NUM;
static int vlan_netIf_bindNum=MAX_VLAN_NETIF_BIND_NUM;

#endif
static is_wlan_mib=0;

/////////////////////////////////////////////////////////////////////////////////////////
static char __inline__ *getVal(char *value, char **p)
{
	int len=0;

	while (*value == ' ' ) value++;

	*p = value;

	while (*value && *value!=',') {
		value++;
		len++;
	}

	if ( !len ) {
		*p = NULL;
		return NULL;
	}

	if ( *value == 0)
		return NULL;

	*value = 0;
	value++;

	return value;
}
#endif  // PARSE_TXT_FILE


void dumpMallInfo()
{
#if 1
            struct mallinfo info;
            info = mallinfo();
            diag_printf("--------------------------------------\n");
            diag_printf("arena: %d\n", info.arena);
            diag_printf("ordblks: %d\n", info.ordblks);
            diag_printf("smblks: %d\n", info.smblks);
            diag_printf("hblks: %d\n", info.hblks);
            diag_printf("hblkhd: %d\n", info.hblkhd);
            diag_printf("usmblks: %d\n", info.usmblks);
            diag_printf("fsmblks: %d\n", info.fsmblks);
            diag_printf("uordblks: %d\n", info.uordblks);
            diag_printf("fordblks: %d\n", info.fordblks);
            diag_printf("keepcost: %d\n", info.keepcost);
            diag_printf("maxfree: %d\n", info.maxfree);
            diag_printf("--------------------------------------\n");

            //sleep(3);
#endif
}

int set_system_time_flash(void)
{
	int cur_year=0;
	//int time_mode=0;
	struct tm tm_time;
	time_t tm;
	extern time_t sys_settime;
	
	tm = get_epoch_build_time();
	if (cyg_libc_time_settime(tm) != 0)
		fprintf(stderr, "set system Time Error");
	sys_settime = tm;
	//apmib_get( MIB_NTP_ENABLED, (void *)&time_mode);
	//if(time_mode == 0)
	{		
		apmib_get( MIB_SYSTIME_YEAR, (void *)&cur_year);

		if (cur_year != 0) {
			tm_time.tm_year = cur_year - 1900;
			apmib_get( MIB_SYSTIME_MON, (void *)&(tm_time.tm_mon));
			apmib_get( MIB_SYSTIME_DAY, (void *)&(tm_time.tm_mday));
			apmib_get( MIB_SYSTIME_HOUR, (void *)&(tm_time.tm_hour));
			apmib_get( MIB_SYSTIME_MIN, (void *)&(tm_time.tm_min));
			apmib_get( MIB_SYSTIME_SEC, (void *)&(tm_time.tm_sec));
			tm = mktime_rewrite(&tm_time);
			if (tm < 0)
				fprintf(stderr, "make Time Error!\n");
			if (cyg_libc_time_settime(tm) != 0)
				fprintf(stderr, "set system Time Error2\n");
			sys_settime = tm;
		}
	}
	return 0;
}

int update_system_time_flash(void)
{
	time_t tm={0};
	struct tm tm_time={0};
	
	time(&tm);
	//diag_printf("%s:%d time=%s\n",__FUNCTION__,__LINE__,ctime(&tm));
	tm_time=*localtime(&tm);
	
	//diag_printf("%s:%d time=%s\n",__FUNCTION__,__LINE__,asctime(&tm_time));
	tm_time.tm_year+=1900;
	apmib_set( MIB_SYSTIME_YEAR, (void *)&(tm_time.tm_year));
	apmib_set( MIB_SYSTIME_MON, (void *)&(tm_time.tm_mon));
	apmib_set( MIB_SYSTIME_DAY, (void *)&(tm_time.tm_mday));
	apmib_set( MIB_SYSTIME_HOUR, (void *)&(tm_time.tm_hour));
	apmib_set( MIB_SYSTIME_MIN, (void *)&(tm_time.tm_min));
	apmib_set( MIB_SYSTIME_SEC, (void *)&(tm_time.tm_sec));
	
}

//////////////////////////////////////////////////////////////////////
static void readFileSetParam(char *file)
{
	FILE *fp;
	char line[200], token[40], value[100], *ptr;
	int idx, hw_setting_found=0, ds_setting_found=0, cs_setting_found=0;
	char *arrayval[2];
	mib_table_entry_T *pTbl;

	
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return;
	}

	fp = fopen(file, "r");
	if (fp == NULL) {
		printf("read file [%s] failed!\n", file);
		return;
	}

	arrayval[0] = value;

	while ( fgets(line, 200, fp) ) {
		ptr = get_token(line, token);
		if (ptr == NULL)
			continue;
		if (get_value(ptr, value)==0)
			continue;

		idx = searchMIB(token);
		if ( idx == -1 ) {
			printf("invalid param [%s]!\n", token);
			fclose(fp);
			return;
		}
		if (config_area == HW_MIB_AREA || config_area == HW_MIB_WLAN_AREA) {
			hw_setting_found = 1;
			if (config_area == HW_MIB_AREA)
				pTbl = hwmib_table;
			else
				pTbl = hwmib_wlan_table;
		}
		else if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA) {
			ds_setting_found = 1;
			if (config_area == DEF_MIB_AREA)
				pTbl = mib_table;
			else
				pTbl = mib_wlan_table;
		}
		else {
			cs_setting_found = 1;
			if (config_area == MIB_AREA)
				pTbl = mib_table;
			else
				pTbl = mib_wlan_table;
		}
		config_area = 0;
		setMIB(token, pTbl[idx].id, pTbl[idx].type, pTbl[idx].size, 1, arrayval);
	}
	fclose(fp);

	if (hw_setting_found)
		apmib_update(HW_SETTING);
	if (ds_setting_found)
		apmib_update(DEFAULT_SETTING);
	if (cs_setting_found)
		apmib_update(CURRENT_SETTING);
}

//////////////////////////////////////////////////////////////////////
static int resetDefault(void)
{
	char *defMib;

	apmib_sem_lock();
	if (pMibDef)
		free(pMibDef);
	pMibDef = NULL;
	if ((defMib=apmib_dsconf()) == NULL) {
		printf("Default configuration invalid!\n");
		apmib_sem_unlock();
		return -1;
	}
	pMibDef = (APMIB_Tp)defMib;
	apmib_sem_unlock();
	
	if (!apmib_updateDef()) {
		printf("Write DS to CS failed!\n");
		free(pMibDef);
		pMibDef = NULL;
		return -1;
	}
	apmib_reinit();
	return 0;
}

#if defined(CONFIG_RTL_8812_SUPPORT)

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
	int x=0, y=0;

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
#endif
//////////////////////////////////////////////////////////////////////
int flash_main(unsigned int argc, unsigned char *argv[])
{
	int ret;
	int argNum=0, action=0, idx, num, valNum=0;
	char mib[64]={0};
	#if defined(CONFIG_RTL_8881A) || defined(CONFIG_SLOT_1_8812) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	#define max_index 200
	int index;
	//char valueArray[200][64], *value[200];
	char *value[max_index];
	char *valueArray[max_index];
	#else
	char valueArray[16][64], *value[16]= {0};
	#endif
	//char *ptr;
	
	#if defined(CONFIG_RTL_8881A) || defined(CONFIG_SLOT_1_8812)|| defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	index = max_index;
	while(index)
	{
		if((valueArray[index-1] = malloc(64)) == NULL)
		{
			while(index < max_index)
			{
				free(valueArray[index]);
				++index;
			}
			diag_printf("%s,%d,malloc error\n",__FUNCTION__,__LINE__);
			return 0;
		}
		--index;
	}
	#endif
#ifdef PARSE_TXT_FILE
	char filename[100]={0};
	APMIB_T apmib;
#endif
	mib_table_entry_T *pTbl=NULL;
/*
if(0)
{
	int kk=0;
	for(kk=0; kk < argc ; kk++)
	{
		printf("\r\n argv[%d]=[%s],__[%s-%u]\r\n",kk, argv[kk],__FILE__,__LINE__);

	}
}
*/
    if ( argc > 0 ) {
#ifdef PARSE_TXT_FILE
		if ( !strcmp(argv[argNum], "-f") ) {
			if (++argNum < argc)
				sscanf(argv[argNum++], "%s", filename);
		}
#endif
		if ( !strcmp((char *)argv[argNum], "get") ) {
			action = 1;
			if (++argNum < argc) {
				if (argc > 2 && !memcmp(argv[argNum], "wlan", 4)) {
					int idx;
#ifdef MBSSID
					if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						argv[argNum][6] == 'v' && argv[argNum][7] == 'a') {
						idx = atoi((char *)&argv[argNum][8]);
						if (idx >= NUM_VWLAN_INTERFACE) {
							printf("invalid virtual wlan interface index number!\n");
							ret = 0 ;
							goto final_return;
						}
						apmib_set_vwlanidx(idx+1);
					}
#ifdef UNIVERSAL_REPEATER
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						!memcmp(&argv[argNum][6], "vxd0", 4)) {
					apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				}
#endif									
#endif
					idx = atoi((char *)&argv[argNum++][4]);
					if (idx >= NUM_WLAN_INTERFACE) {
						printf("invalid wlan interface index number!\n");
						goto normal_return;
					}
					apmib_set_wlanidx(idx);
				}
				sscanf((char *)argv[argNum], "%s", mib);
				while (++argNum < argc) {
					sscanf((char *)argv[argNum], "%s", valueArray[valNum]);
					value[valNum] = valueArray[valNum];
					valNum++;
				}
				value[valNum]= NULL;
			}
		}
		else if (!strcmp((char *)argv[argNum], "set")) {
			action = 2;
			if (++argNum < argc) {
				if (argc > 3 && !memcmp(argv[argNum], "wlan", 4)) {
					int idx;
#ifdef MBSSID
					if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						argv[argNum][6] == 'v' && argv[argNum][7] == 'a') {
						idx = atoi((char *)&argv[argNum][8]);
						if (idx >= NUM_VWLAN_INTERFACE) {
							printf("invalid virtual wlan interface index number!\n");
							ret = 0 ;
							goto final_return;
						}
						apmib_set_vwlanidx(idx+1);
					}
#ifdef UNIVERSAL_REPEATER
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						!memcmp(&argv[argNum][6], "vxd0", 4)) {
					apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				}
#endif									
#endif
					idx = atoi((char *)&argv[argNum++][4]);
					if (idx >= NUM_WLAN_INTERFACE) {
						printf("invalid wlan interface index number!\n");
						goto normal_return;
					}
					apmib_set_wlanidx(idx);
				}
				sscanf((char *)argv[argNum], "%s", mib);
				int SettingSSID = 0;
				if( !strcmp(mib , "SSID") ||
					!strcmp(mib , "WLAN0_SSID") ||
					!strcmp(mib , "WLAN0_WSC_SSID") ||
					!strcmp(mib , "WLAN0_VAP0_SSID") ||
					!strcmp(mib , "WLAN0_VAP1_SSID") ||
					!strcmp(mib , "WLAN0_VAP2_SSID") ||
					!strcmp(mib , "WLAN0_VAP3_SSID") ||	
					!strcmp(mib , "WLAN0_VAP4_SSID") ||	
					!strcmp(mib , "REPEATER_SSID1") ||	
					!strcmp(mib , "REPEATER_SSID2") ||
					!strcmp(mib , "NTP_TIMEZONE") ||
					!strcmp(mib , "SC_DEVICE_NAME") 
					){

					SettingSSID = 1;
				}
				
				while (++argNum < argc) {
					if(SettingSSID == 1){						

						//memcpy(valueArray[valNum] , argv[argNum] , strlen(argv[argNum]));
						strcpy(valueArray[valNum] , (char *)argv[argNum] );
						value[valNum] = valueArray[valNum];
						if((argNum+1) < argc){//space in SSID
							int i,len;
							for(i=1;i<argc-argNum;i++){
								len = strlen(valueArray[valNum]);
								if(valueArray[valNum][len-1] == '\\'){
									valueArray[valNum][len-1] = ' ';
									strcpy(&valueArray[valNum][len] , (char *)argv[argNum+i]) ;
								}
								else
									break;
							}
						}
						valNum++;
						break;
					}					
					sscanf((char *)argv[argNum], "%s", valueArray[valNum]);
					value[valNum] = valueArray[valNum];
					valNum++;	
				}
				value[valNum]= NULL;
			}
		}
		else if ( !strcmp((char *)argv[argNum], "all") ) {
            action = 3;
		}
		else if ( !strcmp((char *)argv[argNum], "default") ) {
            ret = writeDefault(1);
			goto final_return;
			//return writeDefault(1);
		}
		else if ( !strcmp((char *)argv[argNum], "default-sw") ) {
			ret = writeDefault(0);
			goto final_return;			
			//return writeDefault(0);
		}
		else if ( !strcmp((char *)argv[argNum], "reset") ) {
			ret = resetDefault();
			goto final_return;				
			//return resetDefault();
		}
		/*
		else if ( !strcmp((char *)argv[argNum], "test-hwconf") ) {
			apmib_sem_lock();
			if ((ptr=apmib_hwconf()) == NULL) {
				apmib_sem_unlock();
				return -1;
			}
			free(ptr);
			apmib_sem_unlock();
			return 0;
		}
		else if ( !strcmp((char *)argv[argNum], "test-dsconf") ) {
			apmib_sem_lock();
			if ((ptr=apmib_dsconf()) == NULL) {
				apmib_sem_unlock();
				return -1;
			}
			free(ptr);
			apmib_sem_unlock();
			return 0;
		}
		else if ( !strcmp((char *)argv[argNum], "test-csconf") ) {
			apmib_sem_lock();
			if ((ptr=apmib_csconf()) == NULL) {
				apmib_sem_unlock();
				return -1;
			}
			free(ptr);
			apmib_sem_unlock();
			return 0;
		}
		*/
		else if ( !strcmp((char *)argv[argNum], "wpa") ) {
			int isWds = 0;
			if ((argNum+2) < argc) {
				if (memcmp(argv[++argNum], "wlan", 4)) {
					printf("Miss wlan_interface argument!\n");
					ret = 0;
					goto final_return;						
				}
#ifdef MBSSID
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						argv[argNum][6] == 'v' && argv[argNum][7] == 'a') {
					idx = atoi((char *)&argv[argNum][8]);
					if (idx >= NUM_VWLAN_INTERFACE) {
						printf("invalid virtual wlan interface index number!\n");
						ret = 0;
						goto final_return;
					}
					apmib_set_vwlanidx(idx+1);
				}
#ifdef UNIVERSAL_REPEATER
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						!memcmp(&argv[argNum][6], "vxd0", 4)) {
					apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				}
#endif				
#endif
				apmib_set_wlanidx(atoi((char *)&argv[argNum][4]));
				if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE) {
					printf("invalid wlan interface index number!\n");
					goto normal_return;
				}

#ifdef CONFIG_RTK_MESH
				if ((argNum+2) < argc)
				{
					if( !strcmp((char *)argv[argNum+2], "wds")) 
						isWds = 1;
					else if(!strcmp((char *)argv[argNum+2], "msh"))
						isWds = 7;
				}
#else
				if (((argc-1) > (argNum+1)) && !strcmp((char *)argv[argNum+2], "wds")) 
					isWds = 1;
#endif // CONFIG_RTK_MESH

				generateWpaConf((char *)argv[argNum+1], isWds);
				goto normal_return;
			}
			else {
				printf("Miss arguments [wlan_interface config_filename]!\n");
				ret = 0;
				goto final_return;
			}
		}
		// set flash parameters by reading from file
		else if ( !strcmp((char *)argv[argNum], "-param_file") ) {
			if ((argNum+2) < argc) {
				if (memcmp(argv[++argNum], "wlan", 4)) {
					printf("Miss wlan_interface argument!\n");
					ret = 0;
					goto final_return;
				}
#ifdef MBSSID
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						argv[argNum][6] == 'v' && argv[argNum][7] == 'a') {
					idx = atoi((char *)&argv[argNum][8]);
					if (idx >= NUM_VWLAN_INTERFACE) {
						printf("invalid virtual wlan interface index number!\n");
						ret = 0;
						goto final_return;
					}
					apmib_set_vwlanidx(idx+1);
				}
#ifdef UNIVERSAL_REPEATER
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						!memcmp(&argv[argNum][6], "vxd0", 4)) {
					apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				}
#endif				
#endif
				apmib_set_wlanidx(atoi((char *)&argv[argNum][4]));
				if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE) {
					printf("invalid wlan interface index number!\n");
					goto normal_return;
				}
				readFileSetParam((char *)argv[argNum+1]);
			}
			else
				printf("Miss arguments [wlan_interface param_file]!\n");
			ret = 0;
			goto final_return;
		}
		else if (strcmp(argv[argNum], "tblchk") == 0) {
			// do mib tbl check
			//return mibtbl_check();
			ret = mibtbl_check();
			goto final_return;			
		}
#ifdef CONFIG_RTL_WAPI_SUPPORT
		else if(0==strcmp((char *)argv[argNum],"wapi-check"))
		{
			/*if WAPI ,check need to init CA*/
			 struct stat status;
			int init;
			if ( !apmib_init()) {
				printf("Initialize AP MIB failed!\n");
				ret = -1;
				goto final_return;
			}
			apmib_get(MIB_WLAN_WAPI_CA_INIT,(void *)&init);
			if(init)
			{
				ret = 0;
				goto final_return;	
			}
			/*check if CA.cert exists. since the defauts maybe load*/
			if (stat(CA_CERT, &status) < 0)
			{
				system("initCAFiles.sh");
				init=1;
				if(!apmib_set(MIB_WLAN_WAPI_CA_INIT,(void *)&init))
					printf("set MIB_WLAN_WAPI_CA_INIT error\n");
				apmib_update(CURRENT_SETTING);
			}			
			ret = 0;
			goto final_return;	
		}
#else	
	else if(0==strcmp((char *)argv[argNum],"wapi-check"))
	{
		//printf("WAPI Support Not Enabled\n");
		ret = 0;
		goto final_return;
	}
	else if(0==strcmp((char *)argv[argNum],"mib_size"))
	{	
		printf("header size %d\n",sizeof(struct param_header));
		printf("hwmib size %d\n",sizeof(HW_SETTING_T));
		printf("hw wlan mib size %d\n",sizeof(HW_WLAN_SETTING_T));
		printf("apmib size %d\n",sizeof(APMIB_T));	
		printf("hwmib size %d\n",sizeof(HW_WLAN_SETTING_T));
		ret = 0;
		goto final_return;
	}
#endif
#ifdef WLAN_FAST_INIT
		else if ( !strcmp((char *)argv[argNum], "set_mib") ) {
			if ((argNum+1) < argc) {
				if (memcmp(argv[++argNum], "wlan", 4)) {
					printf("Miss wlan_interface argument!\n");
					ret = -1;
					goto final_return;
				}
#ifdef MBSSID
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						argv[argNum][6] == 'v' && argv[argNum][7] == 'a') {
					idx = atoi((char *)&argv[argNum][8]);
					if (idx >= NUM_VWLAN_INTERFACE) {
						printf("invalid virtual wlan interface index number!\n");
						ret = 0;
						goto final_return;
					}
					apmib_set_vwlanidx(idx+1);
				}
#ifdef UNIVERSAL_REPEATER
				if (strlen((char *)argv[argNum]) >= 9 && argv[argNum][5] == '-' &&
						!memcmp(&argv[argNum][6], "vxd0", 4)) {
					apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				}
#endif
#endif
				apmib_set_wlanidx(atoi((char *)&argv[argNum][4]));
				if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE) {
					printf("invalid wlan interface index number!\n");
					goto error_return;
				}
				
#ifdef MBSSID
  #ifdef CONFIG_RTL_COMAPI_CFGFILE
				ret = comapi_initWlan(argv[argNum]);
  #else
				ret = initWlan((char *)argv[argNum]);
  #endif 
				apmib_set_vwlanidx(0);
				goto final_return;  
//				return ret;
#else
				ret =  initWlan(argv[argNum]);
				goto final_return;	
#endif

			}
			else
				printf("Miss arguments [wlan_interface]!\n");
			ret =  -1;
			goto final_return;	
		}
#endif

#if 0
#ifdef WIFI_SIMPLE_CONFIG
		else if ( !strcmp((char *)argv[argNum], "upd-wsc-conf") ) {
			ret = updateWscConf(argv[argNum+1], argv[argNum+2], 0);
			goto final_return;			
		}
#ifdef CONFIG_RTL_COMAPI_CFGFILE
        else if ( !strcmp((char *)argv[argNum], "def-wsc-conf") ) {
			ret =  defaultWscConf(argv[argNum+1], argv[argNum+2], 0);	
			goto final_return;	
		}
#endif
		else if ( !strcmp((char *)argv[argNum], "gen-pin") ) {
			ret =  updateWscConf(0, 0, 1);		
			goto final_return;	
		}		
#endif // WIFI_SIMPLE_CONFIG
#endif
	else if ( !strcmp((char *)argv[argNum], "gethw") ) {
			action = 4;
			if (++argNum < argc) {
				if (argc > 3 && !memcmp(argv[argNum], "wlan", 4)) {
					int idx;
					idx = atoi((char *)&argv[argNum++][4]);
					if (idx >= NUM_WLAN_INTERFACE) {
						printf("invalid wlan interface index number!\n");
						ret = 0;
						goto final_return;	
					}
					apmib_set_wlanidx(idx);
				}
				sscanf((char *)argv[argNum], "%s", mib);
				while (++argNum < argc) {
					sscanf((char *)argv[argNum], "%s", valueArray[valNum]);
					value[valNum] = valueArray[valNum];
					valNum++;
				}
				value[valNum]= NULL;
			}
		}else if ( !strcmp((char *)argv[argNum], "sethw") ) {
			action = 5;
			if (++argNum < argc) {
				if (argc > 4 && !memcmp(argv[argNum], "wlan", 4)) {
					int idx;
					idx = atoi((char *)&argv[argNum++][4]);
					if (idx >= NUM_WLAN_INTERFACE) {
						printf("invalid wlan interface index number!\n");
						ret = 0;
						goto final_return;	
					}
					apmib_set_wlanidx(idx);
				}
				sscanf((char *)argv[argNum], "%s", mib);
				while (++argNum < argc) {
					sscanf((char *)argv[argNum], "%s", valueArray[valNum]);
					value[valNum] = valueArray[valNum];
					valNum++;
				}
				value[valNum]= NULL;
			}
		}else if ( !strcmp((char *)argv[argNum], "allhw") ) {
			action = 6;
		}
	}

//    diag_printf("[%s:%d]action=%d\n", __FUNCTION__, __LINE__, action);
	if ( action == 0) {
		showHelp();
		goto error_return;
	}
	if ( (action==1 && !mib[0]) ||
	     (action==2 && !mib[0]) ) {
		showAllMibName();
		goto error_return;
	}

	if ( action==2 && (!strcmp(mib, "MACAC_ADDR") || !strcmp(mib, "DEF_MACAC_ADDR"))) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetACHelp();
			goto error_return;
		}
		if ( (!strcmp(value[0], "del") && !value[1]) || (!strcmp(value[0], "add") && !value[1]) ) {
			showSetACHelp();
			goto error_return;
		}
	}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	if ( action==2 && (!strcmp(mib, "MESH_ACL_ADDR") || !strcmp(mib, "DEF_MESH_ACL_ADDR"))) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetMeshACLHelp();
			goto error_return;
		}
		if ( (!strcmp(value[0], "del") && !value[1]) || (!strcmp(value[0], "add") && !value[1]) ) {
			showSetMeshACLHelp();
			goto error_return;
		}
	}
#endif
#if defined(VLAN_CONFIG_SUPPORTED)

	if ( action==2 && (!strcmp(mib, "VLANCONFIG_TBL") || !strcmp(mib, "DEF_VLANCONFIG_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetVlanConfigHelp();
			goto error_return;
		}
	}
#endif	
	

	if ( action==2 && (!strcmp(mib, "WDS") || !strcmp(mib, "DEF_WDS"))) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetWdsHelp();
			goto error_return;
		}
		if ( (!strcmp(value[0], "del") && !value[1]) || (!strcmp(value[0], "add") && !value[1]) ) {
			showSetWdsHelp();
			goto error_return;
		}
	}

#ifdef HOME_GATEWAY
	if ( action==2 && (!strcmp(mib, "PORTFW_TBL") || !strcmp(mib, "DEF_PORTFW_TBL"))) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetPortFwHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "PORTFILTER_TBL") || !strcmp(mib, "DEF_PORTFILTER_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetPortFilterHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "IPFILTER_TBL") || !strcmp(mib, "DEF_IPFILTER_TBL"))) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetIpFilterHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "MACFILTER_TBL") || !strcmp(mib, "DEF_MACFILTER_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetMacFilterHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "URLFILTER_TBL") || !strcmp(mib, "DEF_URLFILTER_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetUrlFilterHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "TRIGGERPORT_TBL") || !strcmp(mib, "DEF_TRIGGERPORT_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetTriggerPortHelp();
			goto error_return;
		}
	}
#ifdef ROUTE_SUPPORT
	if ( action==2 && (!strcmp(mib, "STATICROUTE_TBL") || !strcmp(mib, "DEF_STATICROUTE_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetStaticRouteHelp();
			goto error_return;
		}
	}
#endif //ROUTE
#endif
#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	if ( action==2 && (!strcmp(mib, "IPSECTUNNEL_TBL") || !strcmp(mib, "DEF_IPSECTUNNEL_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetIpsecTunnelHelp();
			goto error_return;
		}
	}
#endif
#endif
#ifdef TLS_CLIENT
	if ( action==2 && (!strcmp(mib, "CERTROOT_TBL") || !strcmp(mib, "DEF_CERTROOT_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetCertRootHelp();
			goto error_return;
		}
	}
	if ( action==2 && (!strcmp(mib, "CERTUSER_TBL") || !strcmp(mib, "DEF_CERTUSER_TBL")) ) {
		if (!valNum || (strcmp(value[0], "add") && strcmp(value[0], "del") && strcmp(value[0], "delall"))) {
			showSetCertUserHelp();
			goto error_return;
		}
	}	
#endif

	switch (action) {
	case 1: // get

#ifdef PARSE_TXT_FILE
		if ( filename[0] ) {
			if ( parseTxtConfig(filename, &apmib) < 0) {
				printf("Parse text file error!\n");
				goto error_return;
			}

			if ( !apmib_init(&apmib)) {
				printf("Initialize AP MIB failed!\n");
				goto error_return;
			}
		}
		else

#endif
		if ( !apmib_init()) {
			printf("Initialize AP MIB failed!\n");
			goto error_return;
		}

		idx = searchMIB(mib);

		if ( idx == -1 ) {
			showHelp();
			showAllMibName();
			goto error_return;
		}
		num = 1;

		if (config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA) { // wlan default or current
			if (mib_wlan_table[idx].id == MIB_WLAN_MACAC_ADDR)
				APMIB_GET(MIB_WLAN_MACAC_NUM, (void *)&num);
			else if (mib_wlan_table[idx].id == MIB_WLAN_WDS)
				APMIB_GET(MIB_WLAN_WDS_NUM, (void *)&num);
			else if(mib_wlan_table[idx].id == MIB_WLAN_SCHEDULE_TBL)
				APMIB_GET(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&num);
		}
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
#if 0
		else if (config_area == MIB_AREA) {	// mib_table
			if (mib_table[idx].id == MIB_WLAN_MESH_ACL_ADDR)
			{				
				APMIB_GET(MIB_WLAN_MESH_ACL_NUM, (void *)&num);
			}
			
		}
#else
		else if (mib_table[idx].id == MIB_WLAN_MESH_ACL_ADDR)
			APMIB_GET(MIB_WLAN_MESH_ACL_NUM, (void *)&num);		
#endif

#endif
#ifdef HOME_GATEWAY
		else if (!strcmp(mib, "PORTFW_TBL"))
			APMIB_GET(MIB_PORTFW_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "PORTFILTER_TBL"))
			APMIB_GET(MIB_PORTFILTER_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "IPFILTER_TBL"))
			APMIB_GET(MIB_IPFILTER_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "MACFILTER_TBL"))
			APMIB_GET(MIB_MACFILTER_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "URLFILTER_TBL"))
			APMIB_GET(MIB_URLFILTER_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "TRIGGERPORT_TBL"))
			APMIB_GET(MIB_TRIGGERPORT_TBL_NUM, (void *)&num);

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
		else if (!strcmp(mib, "QOS_RULE_TBL"))
			APMIB_GET(MIB_QOS_RULE_TBL_NUM, (void *)&num);
#endif

#ifdef ROUTE_SUPPORT
		else if (!strcmp(mib, "STATICROUTE_TBL"))
			APMIB_GET(MIB_STATICROUTE_TBL_NUM, (void *)&num);
#endif //ROUTE
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
		else if (!strcmp(mib, "IPSECTUNNEL_TBL"))
			APMIB_GET(MIB_IPSECTUNNEL_TBL_NUM, (void *)&num);
#endif
#endif

#ifdef TLS_CLIENT
		else if (!strcmp(mib, "CERTROOT_TBL"))
			APMIB_GET(MIB_CERTROOT_TBL_NUM, (void *)&num);
		else if (!strcmp(mib, "CERTUSER_TBL"))
			APMIB_GET(MIB_CERTUSER_TBL_NUM, (void *)&num);			
#endif
		else if (!strcmp(mib, "DHCPRSVDIP_TBL"))
			APMIB_GET(MIB_DHCPRSVDIP_TBL_NUM, (void *)&num);
#if defined(VLAN_CONFIG_SUPPORTED)
		else if (!strcmp(mib, "VLANCONFIG_TBL"))
		{
			APMIB_GET(MIB_VLANCONFIG_TBL_NUM, (void *)&num);
		}
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
		else if (!strcmp(mib, "VLAN_TBL"))
		{
			APMIB_GET(MIB_VLAN_TBL_NUM, (void *)&num);
		}
		else if (!strcmp(mib, "NETIFACE_TBL"))
		{
			APMIB_GET(MIB_NETIFACE_TBL_NUM, (void *)&num);
		}
		else if (!strcmp(mib, "VLAN_NETIF_BIND_TBL"))
		{
			APMIB_GET(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&num);
		}
#endif
#ifdef WLAN_PROFILE
		else if (!strcmp(mib, "PROFILE_TBL1"))
			APMIB_GET(MIB_PROFILE_NUM1, (void *)&num);
		else if (!strcmp(mib, "PROFILE_TBL2"))
			APMIB_GET(MIB_PROFILE_NUM2, (void *)&num);		
#endif

		if (config_area == HW_MIB_AREA)
			pTbl = hwmib_table;
		else if (config_area == HW_MIB_WLAN_AREA)
			pTbl = hwmib_wlan_table;
		else if (config_area == DEF_MIB_AREA || config_area == MIB_AREA)
			pTbl = mib_table;
		else
			pTbl = mib_wlan_table;

		if(NULL == pTbl) 
		{
			ret = 0;
			goto final_return;
		}
		getMIB(mib, pTbl[idx].id, pTbl[idx].type, num, 1 ,value);
		break;

	case 2: // set
		if ( !apmib_init()) {
			printf("Initialize AP MIB failed!\n");
			goto error_return;
		}
		idx = searchMIB(mib);
		if ( idx == -1 ) {
			showHelp();
			showAllMibName();
			goto error_return;
		}
		if ( valNum < 1) {
			showHelp();
			goto error_return;
		}
		if (config_area == HW_MIB_AREA)
			pTbl = hwmib_table;
		else if (config_area == HW_MIB_WLAN_AREA)
			pTbl = hwmib_wlan_table;
		else if (config_area ==DEF_MIB_AREA  || config_area == MIB_AREA)
			pTbl = mib_table;
		else
			pTbl = mib_wlan_table;

		if(NULL == pTbl)
		{
			ret = -1;
			goto final_return;	
		}
		
		setMIB(mib, pTbl[idx].id, pTbl[idx].type, pTbl[idx].size, valNum, value);
		break;

	case 3: // all
        dumpAll();
		break;
	case 4: // gethw
        if ( !apmib_init_HW()) {
			printf("Initialize AP HW MIB failed!\n");
			ret = -1;
			goto final_return;
		}
		idx = searchMIB(mib);
		if ( idx == -1 ) {
			showHelp();
			showAllHWMibName();
			ret = -1;
			goto final_return;
		}
		num = 1;
		if (config_area == HW_MIB_AREA)
			pTbl = hwmib_table;
		else if (config_area == HW_MIB_WLAN_AREA)
			pTbl = hwmib_wlan_table;

		if(NULL == pTbl)
		{
			ret = -1;
			goto final_return;
		}
		getMIB(mib, pTbl[idx].id, pTbl[idx].type, num, 1 ,value);
		break;
		
		case 5: // sethw
		if ( !apmib_init_HW()) {
			printf("Initialize AP MIB failed!\n");
			ret = -1;
			goto final_return;
		}
		idx = searchMIB(mib);
		if ( idx == -1 ) {
			showHelp();
			showAllHWMibName();
			ret = -1;
			goto final_return;
		}
		if ( valNum < 1) {
			showHelp();
			ret = -1;
			goto final_return;
		}
		if (config_area == HW_MIB_AREA)
			pTbl = hwmib_table;
		else if (config_area == HW_MIB_WLAN_AREA)
			pTbl = hwmib_wlan_table;
		
		if(NULL == pTbl)
		{
			ret = -1;
			goto final_return;
		}
		setMIB(mib, pTbl[idx].id, pTbl[idx].type, pTbl[idx].size, valNum, value);
		break;
		case 6: // allhw
		dumpAllHW();
		break;	
	}

normal_return:
	apmib_set_vwlanidx(0);
	
	ret= 0;

	goto final_return;
		
error_return:
	apmib_set_vwlanidx(0);
	
	ret= -1;
	
final_return:
#if defined(CONFIG_RTL_8881A) || defined(CONFIG_SLOT_1_8812) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	index = max_index ;
	while(index)
	{
		free(valueArray[index-1]);
		--index;
	}
#endif
	return ret;	
}

//////////////////////////////////////////////////////////////////////////////////

#if defined(CONFIG_RTL_92D_SUPPORT)||defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN) || defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
PHYBAND_TYPE_T wlanPhyBandDef[] = {PHYBAND_5G, PHYBAND_2G}; /* phybandcheck */
#else
PHYBAND_TYPE_T wlanPhyBandDef[] = {PHYBAND_2G, PHYBAND_2G}; /* phybandcheck */
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
void addVlanItem(APMIB_Tp pMib,int i,char* devName,char vlanId)
{
	sprintf(pMib->netIfaceArray[i].netifName,devName);
	pMib->netIfaceArray[i].netifPvid=vlanId;
	pMib->netIfaceNum++;
	pMib->vlanNetifBindArray[i].vlanId=vlanId;
	pMib->vlanNetifBindArray[i].netifId=i;
	pMib->vlanNetifBindNum++;
}
#endif

#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
void Init_WlanConf(APMIB_Tp AP_Mib)
{
	int i;
	AP_Mib->cwmp_WlanConf_Enabled =1;
	AP_Mib->cwmp_WlanConf_EntryNum=MAX_CWMP_WLANCONF_NUM;
	for(i=0;i<MAX_CWMP_WLANCONF_NUM;i++){
		if(i==0){
			#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			AP_Mib->cwmp_WlanConfArray[i].InstanceNum=1;
			AP_Mib->cwmp_WlanConfArray[i].RootIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].IsConfigured=1;
			AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_5G;
			#else
			AP_Mib->cwmp_WlanConfArray[i].InstanceNum=1;
			AP_Mib->cwmp_WlanConfArray[i].RootIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].IsConfigured=1;
			AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
			#endif
		}else if(i ==1){			
		#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			AP_Mib->cwmp_WlanConfArray[i].InstanceNum=2;
			AP_Mib->cwmp_WlanConfArray[i].RootIdx=1;
			AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].IsConfigured=1;
			AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
		#else
			AP_Mib->cwmp_WlanConfArray[i].InstanceNum=0;
			AP_Mib->cwmp_WlanConfArray[i].RootIdx=0;
			AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=1;
			AP_Mib->cwmp_WlanConfArray[i].IsConfigured=0;
			AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
		#endif
		
		}else{
			#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if( i > (NUM_VWLAN_INTERFACE+1) ){
				AP_Mib->cwmp_WlanConfArray[i].InstanceNum=0;
				AP_Mib->cwmp_WlanConfArray[i].RootIdx=1;
				AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=i-(NUM_VWLAN_INTERFACE+1);
				AP_Mib->cwmp_WlanConfArray[i].IsConfigured=0;
				AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
				
			}else{
				AP_Mib->cwmp_WlanConfArray[i].InstanceNum=0;
				AP_Mib->cwmp_WlanConfArray[i].RootIdx=0;
				AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=i-1;
				AP_Mib->cwmp_WlanConfArray[i].IsConfigured=0;
				AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_5G;
			}
			#else
			if( i > (NUM_VWLAN_INTERFACE) ){
				AP_Mib->cwmp_WlanConfArray[i].InstanceNum=0;
				AP_Mib->cwmp_WlanConfArray[i].RootIdx=1;
				AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=i-(NUM_VWLAN_INTERFACE);
				AP_Mib->cwmp_WlanConfArray[i].IsConfigured=0;
				AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
				
			}else{
				AP_Mib->cwmp_WlanConfArray[i].InstanceNum=0;
				AP_Mib->cwmp_WlanConfArray[i].RootIdx=0;
				AP_Mib->cwmp_WlanConfArray[i].VWlanIdx=i;
				AP_Mib->cwmp_WlanConfArray[i].IsConfigured=0;
				AP_Mib->cwmp_WlanConfArray[i].RfBandAvailable=PHYBAND_2G;
			}
			#endif
		}
	}
}
#endif

static int writeDefault(int isAll)
{	
    PARAM_HEADER_T header;
	//APMIB_T mib;
	APMIB_Tp pMib=(APMIB_Tp)malloc(sizeof(APMIB_T));
	int retVal=0;
	HW_SETTING_Tp pHwmib=(HW_SETTING_Tp)malloc(sizeof(HW_SETTING_T));
	char *data;
	int status, offset, i,j, idx;
	unsigned char checksum;
//	unsigned char buff[sizeof(APMIB_T)+sizeof(checksum)+1];
	unsigned char *buff;
#ifdef VLAN_CONFIG_SUPPORTED
	int vlan_entry=0;
#endif
/*
#ifdef COMPRESS_MIB_SETTING
	unsigned char* pContent = NULL;
	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = header.len+sizeof(PARAM_HEADER_T);
	unsigned int compLen;
	unsigned int real_size = 0;
	int zipRate=0;
#endif // #ifdef COMPRESS_MIB_SETTING
*/
#ifdef MIB_TLV
	TLV_PARAM_HEADER_T tlvheader;
	unsigned char *pfile = NULL;
	unsigned char *mib_tlv_data = NULL;
	unsigned int tlv_content_len = 0;
	unsigned int mib_tlv_max_len = 0;
#endif
	int stat;
#ifdef CYGPKG_IO_FLASH
	cyg_flashaddr_t err_addr;
#endif
	
	buff=calloc(1, 0x6000);
	if ( buff == NULL || !pMib ||!pHwmib) {
		printf("Allocate buffer failed!\n");
		retVal=-1;
		goto FLASH_DEFAULT_END;
	}else
	{
		bzero(buff,0x6000);
		bzero(pMib,sizeof(APMIB_T));
		bzero(pHwmib,sizeof(HW_SETTING_T));
	}

    if (isAll) {
		int num_wlan_interface = NUM_WLAN_INTERFACE;

	#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
		num_wlan_interface = 1;
	#endif
		
		// write hw setting
        //diag_printf("[%s:%d]write hw setting\n", __FUNCTION__, __LINE__);
        sprintf((char *)header.signature, "%s%02d", HW_SETTING_HEADER_TAG, HW_SETTING_VER);
		header.len = (sizeof(HW_SETTING_T) + sizeof(checksum));
		//diag_printf("[%s:%d]header.len=%x\n", __FUNCTION__, __LINE__, header.len);

		memset((char *)pHwmib, '\0', sizeof(HW_SETTING_T));
		pHwmib->boardVer = 1;
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
	        memcpy(pHwmib->nic0Addr, "\x0\xe0\x4c\x81\x96\xc1", 6);
		memcpy(pHwmib->nic1Addr, "\x0\xe0\x4c\x81\x96", 6);
		pHwmib->nic1Addr[5] = 0xc1 + NUM_WLAN_MULTIPLE_SSID;
#endif	
		// set RF parameters
		for (idx=0; idx<num_wlan_interface; idx++) {
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
			memcpy(pHwmib->wlan[idx].macAddr, "\x0\xe0\x4c\x81\x96", 5);
#ifdef  CONFIG_SAME_LAN_MAC
			pHwmib->wlan[idx].macAddr[5] = 0xc1 + (idx*16) + 0;
#else
            pHwmib->wlan[idx].macAddr[5] = 0xc1 + (idx*16) + 1;
#endif
			memcpy(pHwmib->wlan[idx].macAddr1, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr1[5] = pHwmib->wlan[idx].macAddr[5] + 1;

			memcpy(pHwmib->wlan[idx].macAddr2, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr2[5] = pHwmib->wlan[idx].macAddr[5] + 2;

			memcpy(pHwmib->wlan[idx].macAddr3, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr3[5] = pHwmib->wlan[idx].macAddr[5] + 3;

			memcpy(pHwmib->wlan[idx].macAddr4, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr4[5] = pHwmib->wlan[idx].macAddr[5] + 4;

			memcpy(pHwmib->wlan[idx].macAddr5, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr5[5] = pHwmib->wlan[idx].macAddr[5] + 5;

			memcpy(pHwmib->wlan[idx].macAddr6, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr6[5] = pHwmib->wlan[idx].macAddr[5] + 6;

			memcpy(pHwmib->wlan[idx].macAddr7, "\x0\xe0\x4c\x81\x96", 5);
			pHwmib->wlan[idx].macAddr7[5] = pHwmib->wlan[idx].macAddr[5] + 7;
			pHwmib->wlan[idx].regDomain = FCC;
			pHwmib->wlan[idx].rfType = 10;
			pHwmib->wlan[idx].xCap = 0;
			pHwmib->wlan[idx].Ther = 0;
#ifdef	CONFIG_RTL_8881A_SELECTIVE
			pHwmib->wlan[idx].xCap2 = 0;
			pHwmib->wlan[idx].Ther2 = 0;
						
#endif				
/*
#if defined(CONFIG_RTL_8196C)
                        hwmib.wlan[idx].trswitch = 0;
#elif defined(CONFIG_RTL_8198)
                        hwmib.wlan[idx].trswitch = 1;
#endif
*/
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevelCCK_A[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevelCCK_B[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevelHT40_1S_A[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevelHT40_1S_B[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiffHT40_2S[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiffHT20[i] = 0;

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiffOFDM[i] = 0;

			pHwmib->wlan[idx].TSSI1 = 0;
			pHwmib->wlan[idx].TSSI2 = 0;

			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevel5GHT40_1S_A[i] = 0;
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrlevel5GHT40_1S_B[i] = 0;
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff5GHT40_2S[i] = 0;
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff5GHT20[i] = 0;
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff5GOFDM[i] = 0;

#if defined(CONFIG_RTL_8812_SUPPORT) 
			// 5G

			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_20BW1S_OFDM1T_A[i] = 0;
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_40BW2S_20BW2S_A[i] = 0;
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_80BW1S_160BW1S_A[i] = 0;		
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_80BW2S_160BW2S_A[i] = 0;

			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_20BW1S_OFDM1T_B[i] = 0;
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_40BW2S_20BW2S_B[i] = 0;
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_80BW1S_160BW1S_B[i] = 0;		
			for (i=0; i<MAX_5G_DIFF_NUM; i++)
				pHwmib->wlan[idx].pwrdiff_5G_80BW2S_160BW2S_B[i] = 0;

			// 2G

			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff_20BW1S_OFDM1T_A[i] = 0;
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff_40BW2S_20BW2S_A[i] = 0;
			
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff_20BW1S_OFDM1T_B[i] = 0;
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++)
				pHwmib->wlan[idx].pwrdiff_40BW2S_20BW2S_B[i] = 0;
#endif
			pHwmib->wlan[idx].trswpape_C9 = 0;//0xAA;
			pHwmib->wlan[idx].trswpape_CC = 0;//0xAF;

			pHwmib->wlan[idx].target_pwr = 0;

#endif
			}
#ifdef _LITTLE_ENDIAN_
		swap_mib_value(pHwmib,HW_SETTING);
#endif
		data = (char *)pHwmib;
		checksum = CHECKSUM((unsigned char *)data, header.len-1);
		header.len = (sizeof(HW_SETTING_T) + sizeof(checksum));
		header.len = WORD_SWAP(sizeof(HW_SETTING_T) + sizeof(checksum));
		//diag_printf("[%s:%d] header.len=%x, checksum=%x\n", __FUNCTION__, __LINE__, header.len, checksum);
			//erase
#if 0
			if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+HW_SETTING_OFFSET), HW_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
	            		printf("FLASH: erase HS failed: %s\n", flash_errmsg(stat));
						retVal=-1;
						goto FLASH_DEFAULT_END;
			}
#endif
			//program
			memcpy(buff, &header, sizeof(header));
			memcpy(buff+sizeof(header), pHwmib, sizeof(HW_SETTING_T));
			memcpy(buff+sizeof(header)+sizeof(HW_SETTING_T), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
			if ((stat = cyg_flash_program((FLASH_BASE_ADDR+HW_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(HW_SETTING_T)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
				printf("write HS failed!\n");
				retVal=-1;
				goto FLASH_DEFAULT_END;
			}else{
				//diag_printf("[%s:%d]write HS succeed\n", __FUNCTION__, __LINE__);
			}
#endif
#if 0
#ifdef COMPRESS_MIB_SETTING
		}
#endif	

#ifdef MIB_TLV
		if(mib_tlv_data != NULL)
			free(mib_tlv_data);
#endif
#endif
	}
	//return 0;
	// write default & current setting
    //diag_printf("[%s:%d]write default & current setting\n", __FUNCTION__, __LINE__);
	memset(pMib, '\0', sizeof(APMIB_T));

	// give a initial value for testing purpose
#if defined(CONFIG_RTL_8196B)	
	strcpy((char *)pMib->deviceName, "RTL8196b");
#elif defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198)
	strcpy((char *)pMib->deviceName, "RTL8196c");
#elif defined(CONFIG_RTL_819XD)
	strcpy((char *)pMib->deviceName, "RTL819XD");
#elif defined(CONFIG_RTL_8196E)
	strcpy((char *)pMib->deviceName, "RTL8196E");
#elif defined(CONFIG_RTL_8197F)
	strcpy((char *)pMib->deviceName, "RTL8197F");
#else
	strcpy((char *)pMib->deviceName, "RTL865x");
#endif

	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		for (i=0; i<(NUM_VWLAN_INTERFACE+1); i++) {
#if defined(CONFIG_RTL_92D_SUPPORT) && !defined(CONFIG_RTL_92D_DMDP) && !defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
			if (idx!=0)
				pMib->wlan[idx][i].wlanDisabled =1;
#endif
#ifdef MBSSID
                       if ((i > 0) && (i<=4))
                       {
                       	    if(0 == idx)
                               	sprintf((char *)pMib->wlan[idx][i].ssid, "RTK 11n AP VAP%d", i);
				    else
					sprintf((char *)pMib->wlan[idx][i].ssid, "RTK 11n AP %d VAP%d", idx,i);
                       }				   
                       else if(i == 0)
#endif			
			  {
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
					if(0 == idx)
                       	strcpy(pMib->wlan[idx][i].ssid, "RTK 11n AP 5G" );
					else if(1 == idx)
                       	strcpy(pMib->wlan[idx][i].ssid, "RTK 11n AP 2.4G" );
					else
						sprintf(pMib->wlan[idx][i].ssid, "RTK 11n AP %d",idx);
#else
					if(0 == idx)
                               	strcpy((char *)pMib->wlan[idx][i].ssid, "RTK 11n AP" );
				    else
					sprintf((char *)pMib->wlan[idx][i].ssid, "RTK 11n AP %d",idx);
#endif
			  }				   

#ifdef UNIVERSAL_REPEATER
			if(i == NUM_VWLAN_INTERFACE) //rpt interface
			{
				pMib->wlan[idx][i].wlanMode = CLIENT_MODE; //assume root interface is AP
				sprintf((char *)pMib->wlan[idx][i].ssid, "RTK 11n AP RPT%d",idx);				
				if(idx == 0)
					sprintf((char *)pMib->repeaterSSID1, "RTK 11n AP RPT%d",idx);
				else
					sprintf((char *)pMib->repeaterSSID2, "RTK 11n AP RPT%d",idx);
#if defined(CONFIG_RTL_SIMPLE_CONFIG) && defined(HAVE_TWINKLE_RSSI)
				pMib->wlan[idx][i].ScEnabled = 1; 
				pMib->wlan[idx][i].ScSyncProfile = 1; 
#endif
			}
#endif

			if (i == 0)
			{	
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
				if(wlanPhyBandDef[idx] == PHYBAND_2G){
					pMib->wlan[idx][i].channel = 11;
#ifdef KLD_ENABLED
					pMib->wlan[idx][i].chanNum = 11;
#endif
				}
				else if(wlanPhyBandDef[idx] == PHYBAND_5G){
					pMib->wlan[idx][i].channel = 44;
#ifdef KLD_ENABLED
					pMib->wlan[idx][i].chanNum = 44;
#endif
				}
				else
#endif
					pMib->wlan[idx][i].channel = 11;

				pMib->wlan[idx][i].wep = WEP_DISABLED;
				pMib->wlan[idx][i].beaconInterval = 100;
				pMib->wlan[idx][i].basicRates = TX_RATE_1M|TX_RATE_2M|TX_RATE_5M|TX_RATE_11M;				
				pMib->wlan[idx][i].supportedRates = TX_RATE_1M|TX_RATE_2M|TX_RATE_5M |TX_RATE_11M|
					TX_RATE_6M|TX_RATE_9M|TX_RATE_12M|TX_RATE_18M|TX_RATE_24M|
						TX_RATE_36M|TX_RATE_48M|TX_RATE_54M;
				pMib->wlan[idx][i].wepKeyType=1;
				pMib->wlan[idx][i].protectionDisabled=1;
				strcpy((char *)pMib->wlan[idx][i].wdsWepKey, "0000000000" );
#ifdef KLD_ENABLED
				pMib->wlan[idx][i].rtsThreshold = 2346;
#else
				pMib->wlan[idx][i].rtsThreshold = 2347;
#endif
#ifdef HAVE_TWINKLE_RSSI
				pMib->wlan[idx][i].ScGoodRssiThreshold = 50;
				pMib->wlan[idx][i].ScNormalRssiThreshold = 35;
				pMib->wlan[idx][i].ScPoorRssiThreshold = 22;
#endif
			}
			if(i <= 4)
			{
				pMib->wlan[idx][i].rateAdaptiveEnabled = 1;

#ifdef CONFIG_RTL_92D_SUPPORT				
				if(wlanPhyBandDef[idx] == PHYBAND_2G)
					pMib->wlan[idx][i].wlanBand = BAND_11BG | BAND_11N;
				else if(wlanPhyBandDef[idx] == PHYBAND_5G)
					pMib->wlan[idx][i].wlanBand = BAND_11A | BAND_11N;
		        else
#endif
        			pMib->wlan[idx][i].wlanBand = BAND_11BG | BAND_11N;

				pMib->wlan[idx][i].wmmEnabled = 1;
				pMib->wlan[idx][i].wpa2Cipher = (unsigned char)WPA_CIPHER_AES; 
				pMib->wlan[idx][i].wpaCipher = (unsigned char)WPA_CIPHER_TKIP;
			}
			pMib->wlan[idx][i].preambleType = LONG_PREAMBLE;

			//pMib->wlan[idx][i].fragThreshold = 2346;
			//pMib->wlan[idx][i].authType = AUTH_BOTH;
			//pMib->wlan[idx][i].inactivityTime = 30000; // 300 sec
			//pMib->wlan[idx][i].dtimPeriod = 1;
			if(i>4)
			{
				pMib->wlan[idx][i].wpaGroupRekeyTime = 86400;
				pMib->wlan[idx][i].wpaAuth = (unsigned char)WPA_AUTH_PSK;
				pMib->wlan[idx][i].wpaCipher = (unsigned char)WPA_CIPHER_TKIP; //Keith
				pMib->wlan[idx][i].wpa2Cipher = (unsigned char)WPA_CIPHER_AES; //Keith
			}
			pMib->wlan[idx][i].rsPort = 1812;
			pMib->wlan[idx][i].accountRsPort = 0;
			pMib->wlan[idx][i].accountRsUpdateDelay = 0;
			pMib->wlan[idx][i].rsMaxRetry = 3;
			pMib->wlan[idx][i].rsIntervalTime = 5;
			pMib->wlan[idx][i].accountRsMaxRetry = 0;
			pMib->wlan[idx][i].accountRsIntervalTime = 0;

#ifdef BRIDGE_REPEATER
			if(i == NUM_VWLAN_INTERFACE)
				pMib->wlan[idx][i].wlanDisabled = 0;
			else
#endif
			if (i > 0)
				pMib->wlan[idx][i].wlanDisabled = 1;


#ifdef WLAN_EASY_CONFIG
			pMib->wlan[idx][i].acfMode = MODE_BUTTON;
			pMib->wlan[idx][i].acfAlgReq = ACF_ALGORITHM_WPA2_AES;
			pMib->wlan[idx][i].acfAlgSupp = ACF_ALGORITHM_WPA_TKIP  | ACF_ALGORITHM_WPA2_AES;
			strcpy(pMib->wlan[idx][i].acfScanSSID, "REALTEK_EASY_CONFIG");
#endif

#ifdef WIFI_SIMPLE_CONFIG

			//strcpy(pMib->wlan[idx][i].wscSsid, pMib->wlan[idx][i].ssid ); //must be the same as pMib->wlan[idx].ssid
			pMib->wlan[idx][i].wscDisable = 0;
#endif
			// for 11n
			if (i == 0)
			{
				pMib->wlan[idx][i].aggregation = 1;
#if defined(CONFIG_RTL_8812_SUPPORT)				
				if(wlanPhyBandDef[idx] == PHYBAND_5G)
					pMib->wlan[idx][i].channelbonding = 2;
		        else
#endif
				pMib->wlan[idx][i].channelbonding = 1;
				pMib->wlan[idx][i].shortgiEnabled = 1;
				pMib->wlan[idx][i].fragThreshold=2346;
				pMib->wlan[idx][i].authType = AUTH_BOTH;
				pMib->wlan[idx][i].inactivityTime = 30000; // 300 sec
				pMib->wlan[idx][i].dtimPeriod = 1;
				pMib->wlan[idx][i].wpaGroupRekeyTime = 86400;
				pMib->wlan[idx][i].wpaAuth = (unsigned char)WPA_AUTH_PSK;
				pMib->wlan[idx][i].wscMethod = 3;
				//strcpy(pMib->wlan[idx].wscPin, "12345670"); //move to hw setting
				pMib->wlan[idx][i].wscAuth = WSC_AUTH_OPEN; //open
				pMib->wlan[idx][i].wscEnc = WSC_ENCRYPT_NONE; //open
				pMib->wlan[idx][i].wscUpnpEnabled = 1;
				pMib->wlan[idx][i].wscRegistrarEnabled = 1;
				pMib->wlan[idx][i].scheduleRuleNum = MAX_SCHEDULE_NUM;
				for(j=0;j<MAX_SCHEDULE_NUM;j++)
				{
					sprintf(pMib->wlan[idx][i].scheduleRuleArray[j].text,"rule_%d",j+1);
				}
				pMib->wlan[idx][i].phyBandSelect = wlanPhyBandDef[idx];
				pMib->wlan[idx][i].macPhyMode = DMACDPHY;
			}			
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL8192E) || defined(CONFIG_RTL_8812_SUPPORT)
			pMib->wlan[idx][i].TxBeamforming = 1; 
#endif
			pMib->wlan[idx][i].LDPCEnabled = 1;
			pMib->wlan[idx][i].STBCEnabled = 1;
			pMib->wlan[idx][i].CoexistEnabled= 0;
			pMib->wlan[idx][i].dtimPeriod= 1;			
		}
	}
#ifdef CONFIG_RTL_DUAL_PCIESLOT_BIWLAN
	pMib->wlan[0][0].wlanBand=BAND_11A | BAND_11N | BAND_11AC;
	for (idx=0; idx<1; idx++) {//vap
			for (i=0; i<=4; i++)
			{
				pMib->wlan[idx][i].wlanBand=BAND_11A | BAND_11N | BAND_11AC;
			}
	}

#endif

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	pMib->wlan[1][0].CoexistEnabled = 0; // 2G enable coexist by default

	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		pMib->wlan[idx][NUM_VWLAN_INTERFACE].wlanDisabled = 0;

	#if defined(CONFIG_CMJ_SALES20131014)
		strcpy(pMib->wlan[idx][0].ssid, idx?"Realtek_CMJ_AP":"Realtek_CMJ_AP");
	#else
		/* set security */
		pMib->wlan[idx][0].encrypt = 6;
		pMib->wlan[idx][0].wpaCipher = 1;
		strcpy(pMib->wlan[idx][0].wpaPSK, "123456789");

		strcpy(pMib->wlan[idx][0].ssid, idx?"RTK_123456789":"RTK_123456789");
	#endif
		pMib->wlan[idx][0].channel = 0;
	#if defined(CONFIG_CMJ_WAN_DETECT)
		pMib->wlan[idx][NUM_VWLAN_INTERFACE].channel = (idx?11:44);
	#endif
		pMib->wlan[idx][0].phyBandSelect  = (idx?PHYBAND_2G:PHYBAND_5G);
		pMib->wlan[idx][0].channelbonding = (idx?1:2);
		pMib->wlan[idx][0].macPhyMode = SMACSPHY;
		pMib->wlan[idx][0].wlanBand = (idx?(BAND_11BG|BAND_11N):(BAND_11A|BAND_11N|BAND_11AC));
	}
	strcpy(pMib->domainName,"123456789");
#ifdef HAVE_NBSERVER
	strcpy(pMib->netbiosName,"");
#endif


#endif

#if defined(CONFIG_RTL_819X)
	pMib->wifiSpecific = 2; //Brad modify for 11n wifi test
#endif
	pMib->ipAddr[0] = 192;
	pMib->ipAddr[1] = 168;
	pMib->ipAddr[2] = 1;
	pMib->ipAddr[3] = 254;

	pMib->subnetMask[0] = 255;
	pMib->subnetMask[1] = 255;
	pMib->subnetMask[2] = 255;
	pMib->subnetMask[3] = 0;

	pMib->dhcpClientStart[0] = 192;
	pMib->dhcpClientStart[1] = 168;
	pMib->dhcpClientStart[2] = 1;
	pMib->dhcpClientStart[3] = 100;

	pMib->dhcpClientEnd[0] = 192;
	pMib->dhcpClientEnd[1] = 168;
	pMib->dhcpClientEnd[2] = 1;
	pMib->dhcpClientEnd[3] = 200;
	pMib->dhcp = DHCP_SERVER;
	pMib->dhcpLeaseTime=480;
#ifdef KLD_ENABLED
	pMib->dnsRelayEnabled=1;
#endif
	//pMib->dhcp = STATIC_IP;

	strcpy((char *)pMib->superName, "super");
	strcpy((char *)pMib->superPassword, "super");
	
#ifdef HOME_GATEWAY

	pMib->vpnPassthruIPsecEnabled=1;
	pMib->vpnPassthruPPTPEnabled=1;
	pMib->vpnPassthruL2TPEnabled=1;

#if defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	pMib->cusPassThru=3;
	pMib->upnpEnabled=1;
#endif

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
		pMib->qosAutoUplinkSpeed=1;
		pMib->qosManualUplinkSpeed=512;
		pMib->qosAutoDownLinkSpeed=1;
		pMib->qosManualDownLinkSpeed=512;
#endif

	pMib->wanIpAddr[0] = 172;
	pMib->wanIpAddr[1] = 1;
	pMib->wanIpAddr[2] = 1;
	pMib->wanIpAddr[3] = 1;

	pMib->wanSubnetMask[0] = 255;
	pMib->wanSubnetMask[1] = 255;
	pMib->wanSubnetMask[2] = 255;
	pMib->wanSubnetMask[3] = 0;

	pMib->wanDefaultGateway[0] = 172;
	pMib->wanDefaultGateway[1] = 1;
	pMib->wanDefaultGateway[2] = 1;
	pMib->wanDefaultGateway[3] = 254;

	pMib->wanDhcp = DHCP_CLIENT;
	pMib->dnsMode = DNS_AUTO;
	pMib->pppIdleTime = 300;
	pMib->pptpIdleTime = 300;
	pMib->l2tpIdleTime = 300;
	pMib->pppMtuSize = 1492;
	pMib->pptpIpAddr[0]=172;
	pMib->pptpIpAddr[1]=1;
	pMib->pptpIpAddr[2]=1;
	pMib->pptpIpAddr[3]=2;
	pMib->pptpSubnetMask[0]=255;
	pMib->pptpSubnetMask[1]=255;
	pMib->pptpSubnetMask[2]=255;
	pMib->pptpSubnetMask[3]=0;
	pMib->pptpServerIpAddr[0]=172;
	pMib->pptpServerIpAddr[1]=1;
	pMib->pptpServerIpAddr[2]=1;
	pMib->pptpServerIpAddr[3]=1;
	pMib->pptpMtuSize = 1460;
	pMib->l2tpIpAddr[0]=172;
	pMib->l2tpIpAddr[1]=1;
	pMib->l2tpIpAddr[2]=1;
	pMib->l2tpIpAddr[3]=2;
	pMib->l2tpSubnetMask[0]=255;
	pMib->l2tpSubnetMask[1]=255;
	pMib->l2tpSubnetMask[2]=255;
	pMib->l2tpSubnetMask[3]=0;
	pMib->l2tpServerIpAddr[0]=172;
	pMib->l2tpServerIpAddr[1]=1;
	pMib->l2tpServerIpAddr[2]=1;
	pMib->l2tpServerIpAddr[3]=1;
	pMib->l2tpMtuSize = 1460; /* keith: add l2tp support. 20080515 */
	pMib->L2tpwanIPMode = 1; /* keith: add l2tp support. 20080515 */
	pMib->fixedIpMtuSize = 1500;
	pMib->dhcpMtuSize = 1500;
	pMib->ntpEnabled=0;
	pMib->daylightsaveEnabled=0;
	strcpy(pMib->ntpTimeZone,"-8 4");
	pMib->ntpServerIp1[0]=0;
	pMib->ntpServerIp1[1]=0;
	pMib->ntpServerIp1[2]=0;
	pMib->ntpServerIp1[3]=0;
	pMib->ntpServerIp2[0]=0;
	pMib->ntpServerIp2[1]=0;
	pMib->ntpServerIp2[2]=0;
	pMib->ntpServerIp2[3]=0;

	pMib->wan_detect = 1;
	pMib->upgrade_kit = 0;

	pMib->ddnsEnabled=0;
	pMib->ddnsType=0;
	strcpy(pMib->ddnsDomainName,"host.dyndns.org");

#ifdef CONFIG_RTL_DHCP_PPPOE
	pMib->pppoeWithDhcpEnabled=1;
#endif

#ifdef CONFIG_IPV6
	pMib->radvdCfgParam.enabled =0;
	strcpy(pMib->radvdCfgParam.interface.Name,"eth0");
	pMib->radvdCfgParam.interface.MaxRtrAdvInterval=600;
	pMib->radvdCfgParam.interface.MinRtrAdvInterval=600*0.33;
	pMib->radvdCfgParam.interface.MinDelayBetweenRAs=3;
	pMib->radvdCfgParam.interface.AdvManagedFlag=0;
	pMib->radvdCfgParam.interface.AdvOtherConfigFlag=0;
	pMib->radvdCfgParam.interface.AdvLinkMTU=1500;
	pMib->radvdCfgParam.interface.AdvReachableTime=0;
	pMib->radvdCfgParam.interface.AdvRetransTimer=0;
	pMib->radvdCfgParam.interface.AdvCurHopLimit=64;
	pMib->radvdCfgParam.interface.AdvDefaultLifetime=600*3;
	strcpy(pMib->radvdCfgParam.interface.AdvDefaultPreference,"medium");
	pMib->radvdCfgParam.interface.AdvSourceLLAddress=1;
	pMib->radvdCfgParam.interface.UnicastOnly=0;
				/*prefix 1*/
	pMib->radvdCfgParam.interface.prefix[0].Prefix[0]=0x2001;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[1]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[2]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[3]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[4]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[5]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[6]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].Prefix[7]=0x0;
	pMib->radvdCfgParam.interface.prefix[0].PrefixLen=64;
	pMib->radvdCfgParam.interface.prefix[0].AdvOnLinkFlag=1;
	pMib->radvdCfgParam.interface.prefix[0].AdvAutonomousFlag=1;
	pMib->radvdCfgParam.interface.prefix[0].AdvValidLifetime=2592000;
	pMib->radvdCfgParam.interface.prefix[0].AdvPreferredLifetime=604800;
	pMib->radvdCfgParam.interface.prefix[0].AdvRouterAddr=0;
	strcpy(pMib->radvdCfgParam.interface.prefix[0].if6to4,"eth1");
	pMib->radvdCfgParam.interface.prefix[0].enabled=1;
				/*prefix 2*/
	pMib->radvdCfgParam.interface.prefix[1].Prefix[0]=0x2002;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[1]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[2]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[3]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[4]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[5]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[6]=0;
	pMib->radvdCfgParam.interface.prefix[1].Prefix[7]=0;
	pMib->radvdCfgParam.interface.prefix[1].PrefixLen=64;
	pMib->radvdCfgParam.interface.prefix[1].AdvOnLinkFlag=1;
	pMib->radvdCfgParam.interface.prefix[1].AdvAutonomousFlag=1;
	pMib->radvdCfgParam.interface.prefix[1].AdvValidLifetime=2592000;
	pMib->radvdCfgParam.interface.prefix[1].AdvPreferredLifetime=604800;
	pMib->radvdCfgParam.interface.prefix[1].AdvRouterAddr=0;
//	pMib->radvdCfgParam.interface.prefix[1].if6to4="";
	pMib->radvdCfgParam.interface.prefix[1].enabled=1;
#ifdef CONFIG_IPV6_WAN_RA
	pMib->radvdCfgParam_wan.enabled =0;
	strcpy(pMib->radvdCfgParam_wan.interface.Name,"eth1");
	pMib->radvdCfgParam_wan.interface.MaxRtrAdvInterval=600;
	pMib->radvdCfgParam_wan.interface.MinRtrAdvInterval=600*0.33;
	pMib->radvdCfgParam_wan.interface.MinDelayBetweenRAs=3;
	pMib->radvdCfgParam_wan.interface.AdvManagedFlag=0;
	pMib->radvdCfgParam_wan.interface.AdvOtherConfigFlag=0;
	pMib->radvdCfgParam_wan.interface.AdvLinkMTU=1500;
	pMib->radvdCfgParam_wan.interface.AdvReachableTime=0;
	pMib->radvdCfgParam_wan.interface.AdvRetransTimer=0;
	pMib->radvdCfgParam_wan.interface.AdvCurHopLimit=64;
	pMib->radvdCfgParam_wan.interface.AdvDefaultLifetime=600*3;
	strcpy(pMib->radvdCfgParam_wan.interface.AdvDefaultPreference,"medium");
	pMib->radvdCfgParam_wan.interface.AdvSourceLLAddress=1;
	pMib->radvdCfgParam_wan.interface.UnicastOnly=0;
				/*prefix 1*/
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[0]=0x2003;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[1]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[2]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[3]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[4]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[5]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[6]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].Prefix[7]=0x0;
	pMib->radvdCfgParam_wan.interface.prefix[0].PrefixLen=64;
	pMib->radvdCfgParam_wan.interface.prefix[0].AdvOnLinkFlag=1;
	pMib->radvdCfgParam_wan.interface.prefix[0].AdvAutonomousFlag=1;
	pMib->radvdCfgParam_wan.interface.prefix[0].AdvValidLifetime=2592000;
	pMib->radvdCfgParam_wan.interface.prefix[0].AdvPreferredLifetime=604800;
	pMib->radvdCfgParam_wan.interface.prefix[0].AdvRouterAddr=0;
	//strcpy(pMib->radvdCfgParam_wan.interface.prefix[0].if6to4,"");
	pMib->radvdCfgParam_wan.interface.prefix[0].enabled=1;
				/*prefix 2*/
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[0]=0x2004;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[1]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[2]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[3]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[4]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[5]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[6]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].Prefix[7]=0;
	pMib->radvdCfgParam_wan.interface.prefix[1].PrefixLen=64;
	pMib->radvdCfgParam_wan.interface.prefix[1].AdvOnLinkFlag=1;
	pMib->radvdCfgParam_wan.interface.prefix[1].AdvAutonomousFlag=1;
	pMib->radvdCfgParam_wan.interface.prefix[1].AdvValidLifetime=2592000;
	pMib->radvdCfgParam_wan.interface.prefix[1].AdvPreferredLifetime=604800;
	pMib->radvdCfgParam_wan.interface.prefix[1].AdvRouterAddr=0;
//	pMib->radvdCfgParam_wan.interface.prefix[1].if6to4="";
	pMib->radvdCfgParam_wan.interface.prefix[1].enabled=1;

#endif
//dnsv6
	pMib->dnsCfgParam.enabled=0;
	strcpy(pMib->dnsCfgParam.routerName,"router.my");
//dhcp6
	pMib->dhcp6sCfgParam.enabled=0;
	strcpy(pMib->dhcp6sCfgParam.DNSaddr6,"2001:db8::35");
	strcpy(pMib->dhcp6sCfgParam.addr6PoolS,"2001:db8:1:2::1000");
	strcpy(pMib->dhcp6sCfgParam.addr6PoolE,"2001:db8:1:2::2000");
	strcpy(pMib->dhcp6sCfgParam.interfaceNameds,"br0");
//ipv6 addr
	pMib->addrIPv6CfgParam.enabled=0;
	pMib->addrIPv6CfgParam.prefix_len[0]=0;
	pMib->addrIPv6CfgParam.prefix_len[1]=0;
	{
		int itemp;
		for(itemp=0;itemp<8;itemp++)
		{
			pMib->addrIPv6CfgParam.addrIPv6[0][itemp]=0;
			pMib->addrIPv6CfgParam.addrIPv6[1][itemp]=0;
		}
	}
//6to4 
	pMib->tunnelCfgParam.enabled=0;
#endif
#endif

#ifdef KLD_ENABLED
	pMib->webLoginTimeout=5;
	strcpy(pMib->userName,"admin");
	bzero(pMib->userPassword,sizeof(pMib->userPassword));
    pMib->rtspAlgEnable = 1;    
    pMib->sipAlgEnable = 1;
    pMib->ftpAlgEnable = 1;
    pMib->tftpAlgEnable = 1;
    pMib->pptpAlgEnable = 1;
    pMib->l2tpAlgEnable = 1;
    pMib->h323AlgEnable = 1;
    pMib->spiFirewall = 0;
    pMib->antiSpoofing = 0;
	pMib->dhcpRsvdIpNum=MAX_DHCP_RSVD_IP_NUM;
	for(i=0;i<MAX_DHCP_RSVD_IP_NUM;i++)
	{
		pMib->dhcpRsvdIpArray[i].index=i;
	}
#endif
// added by rock /////////////////////////////////////////
#ifdef VOIP_SUPPORT
	flash_voip_default(&pMib->voipCfgParam);
#endif

	// SNMP, Forrest added, 2007.10.25.
#ifdef CONFIG_SNMP
	pMib->snmpEnabled = 1;
	sprintf(pMib->snmpName, "%s", "Realtek");
	sprintf(pMib->snmpLocation, "%s", "AP");
	sprintf(pMib->snmpContact, "%s", "Router");
	sprintf(pMib->snmpRWCommunity, "%s", "private");
	sprintf(pMib->snmpROCommunity, "%s", "public");
	pMib->snmpTrapReceiver1[0] = 0;
	pMib->snmpTrapReceiver1[1] = 0;
	pMib->snmpTrapReceiver1[2] = 0;
	pMib->snmpTrapReceiver1[3] = 0;
	pMib->snmpTrapReceiver2[0] = 0;
	pMib->snmpTrapReceiver2[1] = 0;
	pMib->snmpTrapReceiver2[2] = 0;
	pMib->snmpTrapReceiver2[3] = 0;
	pMib->snmpTrapReceiver3[0] = 0;
	pMib->snmpTrapReceiver3[1] = 0;
	pMib->snmpTrapReceiver3[2] = 0;
	pMib->snmpTrapReceiver3[3] = 0;
#endif

#ifdef CONFIG_RTK_MESH
	for (idx=0; idx<NUM_WLAN_INTERFACE; idx++) {
		pMib->wlan[idx][0].meshEnabled = 0;
#ifdef CONFIG_NEW_MESH_UI
		pMib->wlan[idx][0].meshRootEnabled = 0;
#else
		pMib->wlan[idx][0].meshRootEnabled = 0;		// if meshEnabled default value "1", Here "1" also
#endif
		pMib->wlan[idx][0].meshMaxNumOfNeighbors = 32;
		strcpy(pMib->wlan[idx][0].meshID, "RTK-mesh");
	}

#ifdef 	_11s_TEST_MODE_
	pMib->meshTestParam6 = 43627 ;
	pMib->meshTestParam7 = 48636 ;
	pMib->meshTestParam8 = 42090 ;
	pMib->meshTestParam9 = 43627 ;
	pMib->meshTestParama = 42606 ;
	pMib->meshTestParamb = 47016 ;
	pMib->meshTestParamc = 57710 ;
	pMib->meshTestParamd = 46323 ;
	pMib->meshTestParame = 47016 ;
	pMib->meshTestParamf = 47811 ;
#endif

#endif // CONFIG_RTK_MESH
#if defined(HOME_GATEWAY) && defined(ROUTE_SUPPORT)
	pMib->natEnabled = 1;
#endif

#ifdef HOME_GATEWAY
#ifdef CONFIG_APP_SAMBA
	pMib->sambaEnabled = 1;
#else
	pMib->sambaEnabled = 0;
#endif
#endif

#ifdef SNMP_SUPPORT
	strcpy(pMib->snmpROcommunity,"public");
	strcpy(pMib->snmpRWcommunity,"private");
#endif
#if defined(VLAN_CONFIG_SUPPORTED)
for(vlan_entry=0;vlan_entry<MAX_IFACE_VLAN_CONFIG;vlan_entry++){
	pMib->VlanConfigArray[vlan_entry].enabled=0;
	pMib->VlanConfigArray[vlan_entry].vlanId = 1;
	pMib->VlanConfigArray[vlan_entry].cfi = 1;
}	
pMib->VlanConfigEnabled=0;
pMib->VlanConfigNum=MAX_IFACE_VLAN_CONFIG;
sprintf(pMib->VlanConfigArray[0].netIface, "%s", "eth0");
sprintf(pMib->VlanConfigArray[1].netIface, "%s", "eth2");
sprintf(pMib->VlanConfigArray[2].netIface, "%s", "eth3");
sprintf(pMib->VlanConfigArray[3].netIface, "%s", "eth4");
sprintf(pMib->VlanConfigArray[4].netIface, "%s", "wlan0");
sprintf(pMib->VlanConfigArray[5].netIface, "%s", "wlan0-va0");
sprintf(pMib->VlanConfigArray[6].netIface, "%s", "wlan0-va1");
sprintf(pMib->VlanConfigArray[7].netIface, "%s", "wlan0-va2");
sprintf(pMib->VlanConfigArray[8].netIface, "%s", "wlan0-va3");
sprintf(pMib->VlanConfigArray[9].netIface, "%s", "eth1");

pMib->VlanConfigArray[0].vlanId = 3022;
pMib->VlanConfigArray[0].priority = 7;
pMib->VlanConfigArray[1].vlanId = 3030;
pMib->VlanConfigArray[1].priority = 0;
pMib->VlanConfigArray[2].vlanId = 500;
pMib->VlanConfigArray[2].priority = 3;
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
//lan vlan and wan vlan setting
pMib->vlanNum=2;
pMib->vlanArray[0].vlanId=VLAN_LAN_PVID_DEF;
pMib->vlanArray[1].vlanId=VLAN_WAN_PVID_DEF;
#ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
pMib->vlanArray[0].ForwardRule=LAN_WAN_NAPT;
pMib->vlanArray[1].ForwardRule=LAN_WAN_NAPT;
#endif

for(i=0;i<MAX_NETIFACE_NUM;i++)
{
	pMib->netIfaceArray[i].netifId=i;
}
pMib->netIfaceNum=0;
pMib->vlanNetifBindNum=0;
i=0;

addVlanItem(pMib,i++,"lan0",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"lan1",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"lan2",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"lan3",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wan",VLAN_WAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan0",VLAN_LAN_PVID_DEF);
#if defined(CONFIG_RTL_VAP_SUPPORT)
for(j=0;j<DEF_MSSID_NUM;j++)
{
	char vapNamBuf[16]={0};
	sprintf(vapNamBuf,"wlan0-va%d",j);
	addVlanItem(pMib,i++,vapNamBuf,VLAN_LAN_PVID_DEF);
}
#endif
#ifdef UNIVERSAL_REPEATER
addVlanItem(pMib,i++,"wlan0-vxd0",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan0-vxd1",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan0-vxd2",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan0-vxd3",VLAN_LAN_PVID_DEF);
#endif
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
addVlanItem(pMib,i++,"wlan1",VLAN_LAN_PVID_DEF);
#if defined(CONFIG_RTL_VAP_SUPPORT)
for(j=0;j<DEF_MSSID_NUM;j++)
{
	char vapNamBuf[16]={0};
	sprintf(vapNamBuf,"wlan1-va%d",j);
	addVlanItem(pMib,i++,vapNamBuf,VLAN_LAN_PVID_DEF);
}
#endif

#ifdef UNIVERSAL_REPEATER
addVlanItem(pMib,i++,"wlan1-vxd0",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan1-vxd1",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan1-vxd2",VLAN_LAN_PVID_DEF);
addVlanItem(pMib,i++,"wlan1-vxd3",VLAN_LAN_PVID_DEF);
#endif
#endif
#endif

#if defined(CONFIG_RTL_WEB_WAN_ACCESS_PORT)
pMib->wanAccessPort = 8080;
#endif

#ifdef HOME_GATEWAY
#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
pMib->qosManualUplinkSpeed = 512;
pMib->qosManualDownLinkSpeed = 512;
#endif
#endif

#if defined(CONFIG_RTL_DUAL_PCIESLOT_BIWLAN)
	pMib->wlanBand2G5GSelect = BANDMODEBOTH;
#elif defined(CONFIG_WLAN_HAL_8814AE)&&!defined(CONFIG_RTL_92D_SUPPORT)
	pMib->wlanBand2G5GSelect = BANDMODE2G;
#elif defined(CONFIG_RTL_8881A_SELECTIVE)
	pMib->wlanBand2G5GSelect = BANDMODESINGLE;
#else
	pMib->wlanBand2G5GSelect = BANDMODE2G;
#endif


#if defined(WLAN_PROFILE)
	pMib->wlan_profile_enable1 = 0;
	pMib->wlan_profile_num1 = 0;
	pMib->wlan_profile_enable2 = 0;
	pMib->wlan_profile_num2 = 0;
#endif //#if defined(WLAN_PROFILE)

#ifdef CONFIG_ECOS_AP_SUPPORT
	pMib->opMode=BRIDGE_MODE;
#endif
#ifdef BRIDGE_REPEATER
	pMib->opMode=BRIDGE_MODE;
	pMib->repeaterEnabled1=1;
#endif

#if defined(CONFIG_APP_TR069)
#if defined(WLAN_SUPPORT)
		//The Setting is "MUST"
		Init_WlanConf(pMib);
#endif
#if defined (_PRMT_USERINTERFACE_)
		sprintf((char *)pMib->UIF_Cur_Lang, "%s", "en-us");	
#ifdef _ALPHA_DUAL_WAN_SUPPORT_
		/* default enable TR069 */
		pMib->cwmp_Flag |= CWMP_FLAG_AUTORUN;
		pMib->cwmp_enabled = 1;
#endif	
#endif
#if defined (CUSTOMIZE_MIDDLE_EAST)
	pMib->cwmp_ConnReqPort = 51005;	
	sprintf(pMib->cwmp_ConnReqPath,"/%02x%02x%02x%02x%02x%02x",pHwmib->nic0Addr[0],pHwmib->nic0Addr[1],pHwmib->nic0Addr[2],pHwmib->nic0Addr[3],pHwmib->nic0Addr[4],pHwmib->nic0Addr[5]);
#endif
#endif

   
	data = (char *)pMib;
#ifdef _LITTLE_ENDIAN_
	swap_mib_value(pMib,DEFAULT_SETTING);
#endif

	// write default setting
    //diag_printf("[%s:%d]write default setting\n", __FUNCTION__, __LINE__);

#ifdef MIB_TLV

//mib_display_data_content(DEFAULT_SETTING, pMib, sizeof(APMIB_T));

	mib_tlv_max_len = mib_get_setting_len(DEFAULT_SETTING)*4;

	tlv_content_len = 0;

	pfile = malloc(mib_tlv_max_len);
    memset(pfile, 0x00, mib_tlv_max_len);
	//diag_printf("%s:%d mib_tlv_max_len=%d\n",__FUNCTION__,__LINE__,mib_tlv_max_len);

   if( pfile != NULL && mib_tlv_save(DEFAULT_SETTING, (void*)pMib, pfile, &tlv_content_len) == 1)
	{
        mib_tlv_data = malloc(tlv_content_len+1); // 1:checksum
		if(mib_tlv_data != NULL)
		{
			memcpy(mib_tlv_data, pfile, tlv_content_len);
		}else
            diag_printf("[%s:%d]malloc %d ERROR!\n", __FUNCTION__, __LINE__, tlv_content_len+1);

		free(pfile);
		
	}

    if(mib_tlv_data != NULL)
	{

        sprintf((char *)tlvheader.signature, "%s%02d", DEFAULT_SETTING_HEADER_TAG, DEFAULT_SETTING_VER);
		tlvheader.len = tlv_content_len+1;
		data = (char *)mib_tlv_data;
		checksum = CHECKSUM((unsigned char *)data, tlvheader.len-1);
		tlvheader.len = DWORD_SWAP(tlv_content_len+1);
		data[tlv_content_len] = checksum;
//mib_display_tlv_content(DEFAULT_SETTING, data, header.len);

	}
	else
	{
#endif //#ifdef MIB_TLV
		data = (char *)pMib;
		sprintf((char *)header.signature, "%s%02d", DEFAULT_SETTING_HEADER_TAG, DEFAULT_SETTING_VER);
		checksum = CHECKSUM((unsigned char *)data, header.len-1);
		header.len = WORD_SWAP(sizeof(APMIB_T) + sizeof(checksum));
		//diag_printf("%s:%d header.len=%d sizeof(APMIB_T)=%d\n",__FUNCTION__,__LINE__,header.len,sizeof(APMIB_T));

#ifdef MIB_TLV
	}
#endif

#ifdef COMPRESS_MIB_SETTING

//fprintf(stderr,"\r\n header.len=%u, __[%s-%u]",header.len,__FILE__,__LINE__);

    //diag_printf("[%s:%d]tlvheader.len=%x\n", __FUNCTION__, __LINE__,tlvheader.len);
	if(flash_mib_compress_write(DEFAULT_SETTING, data, &tlvheader, &checksum) == 1)
	{
		COMP_TRACE(stderr,"\r\n flash_mib_compress_write DEFAULT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
    }
	else
	{
        //diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);

#endif //#ifdef COMPRESS_MIB_SETTING

		//erase
		#if 0
		if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+DEFAULT_SETTING_OFFSET), DEFAULT_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
	    		printf("FLASH: erase DS failed: %s\n", flash_errmsg(stat));
				retVal=-1;
				goto FLASH_DEFAULT_END;
		}
		#endif
		//program
        memcpy(buff, &header, sizeof(header));
		memcpy(buff+sizeof(header), pMib, sizeof(APMIB_T));
		memcpy(buff+sizeof(header)+sizeof(APMIB_T), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
		if ((stat = cyg_flash_program((FLASH_BASE_ADDR+DEFAULT_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(APMIB_T)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
			printf("write DS failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
#endif
#ifdef COMPRESS_MIB_SETTING
	}
#endif
	//return 0;

	// write current setting
    //diag_printf("[%s:%d]write current setting\n", __FUNCTION__, __LINE__);
#ifdef MIB_TLV
	if(mib_tlv_data != NULL)
	{
		sprintf((char *)tlvheader.signature, "%s%02d", CURRENT_SETTING_HEADER_TAG, CURRENT_SETTING_VER);
		tlvheader.len =tlv_content_len+1;
		checksum = CHECKSUM((unsigned char *)data, tlvheader.len-1);
		tlvheader.len = DWORD_SWAP(tlvheader.len);
		//diag_printf("%s:%d tlv_content_len=%d header.len=%d\n",__FUNCTION__,__LINE__,tlv_content_len,header.len);
//mib_display_tlv_content(CURRENT_SETTING, data, header.len);
	}
	else
	{
#endif
        sprintf((char *)header.signature, "%s%02d", CURRENT_SETTING_HEADER_TAG, CURRENT_SETTING_VER);
        header.len = (sizeof(APMIB_T) + sizeof(checksum));
        //diag_printf("%s:%d header.len=%d sizeof(APMIB_T)=%d\n",__FUNCTION__,__LINE__,header.len,sizeof(APMIB_T));
		header.len = WORD_SWAP(header.len);
#ifdef MIB_TLV
	}
#endif

#ifdef COMPRESS_MIB_SETTING

	if(flash_mib_compress_write(CURRENT_SETTING, data, &tlvheader, &checksum) == 1)
	{
		COMP_TRACE(stderr,"\r\n flash_mib_compress_write CURRENT_SETTING DONE, __[%s-%u]", __FILE__,__LINE__);			
	}
	else
	{

#endif //#ifdef COMPRESS_MIB_SETTING
		
		//erase
		#if 0
		if ((stat = cyg_flash_erase((FLASH_BASE_ADDR+CURRENT_SETTING_OFFSET), CURRENT_SETTING_SECTOR_LEN, &err_addr)) != CYG_FLASH_ERR_OK) {
	    		printf("FLASH: erase CS failed: %s\n", flash_errmsg(stat));
				retVal=-1;
				goto FLASH_DEFAULT_END;
		}
		#endif
		//program
		memcpy(buff, &header, sizeof(header));
		memcpy(buff+sizeof(header), pMib, sizeof(APMIB_T));
		memcpy(buff+sizeof(header)+sizeof(APMIB_T), &checksum, sizeof(checksum));
#ifdef CYGPKG_IO_FLASH
		if ((stat = cyg_flash_program((FLASH_BASE_ADDR+CURRENT_SETTING_OFFSET), (void *)buff, (sizeof(header)+sizeof(APMIB_T)+sizeof(checksum)), &err_addr)) != CYG_FLASH_ERR_OK) {
			printf("write CS failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
#endif
#ifdef COMPRESS_MIB_SETTING	
	}
#endif

	// check if hw, ds, cs checksum is ok
	offset = HW_SETTING_OFFSET;
#if 0
#ifdef COMPRESS_MIB_SETTING
	if(flash_mib_checksum_ok(offset) == 1)
	{
		COMP_TRACE(stderr,"\r\n HW_SETTING hecksum_ok\n");
	}
	else
	{
		fprintf(stderr,"flash mib checksum error!\n");
#endif // #ifdef COMPRESS_MIB_SETTING
#endif
		if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
			diag_printf("read hs header failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		//diag_printf("[%s:%d]header.len=0x%x\n", __FUNCTION__, __LINE__, header.len);
		header.len=WORD_SWAP(header.len);
		offset += sizeof(header);
		if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
			diag_printf("read hs MIB failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		status = CHECKSUM_OK(buff, header.len);
		if ( !status) {
			diag_printf("hs Checksum error!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
#if 0
#ifdef COMPRESS_MIB_SETTING	
	}
#endif	
#endif

	offset = DEFAULT_SETTING_OFFSET;
#ifdef COMPRESS_MIB_SETTING
	if(flash_mib_checksum_ok(offset) == 1)
	{
		COMP_TRACE(stderr,"\r\n DEFAULT_SETTING hecksum_ok\n");
	}
	else
	{
#endif // #ifdef COMPRESS_MIB_SETTING
		//diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
		if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
			diag_printf("read ds header failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		header.len=WORD_SWAP(header.len);
		offset += sizeof(header);
		if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
			diag_printf("read ds MIB failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		status = CHECKSUM_OK(buff, header.len);
		if ( !status) {
			diag_printf("ds Checksum error!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
#ifdef COMPRESS_MIB_SETTING
	}
#endif

	offset = CURRENT_SETTING_OFFSET;
#ifdef COMPRESS_MIB_SETTING
	if(flash_mib_checksum_ok(offset) == 1)
	{
		COMP_TRACE(stderr,"\r\n CURRENT_SETTING hecksum_ok\n");
	}
	else
	{
#endif // #ifdef COMPRESS_MIB_SETTING	
		if ( rtk_flash_read((char *)&header, offset, sizeof(header)) == 0) {
			diag_printf("read cs header failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		header.len=WORD_SWAP(header.len);
		offset += sizeof(header);
		if ( rtk_flash_read((char *)buff, offset, header.len) == 0) {
			diag_printf("read cs MIB failed!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
		status = CHECKSUM_OK(buff, header.len);
	
		if ( !status) {
			diag_printf("cs Checksum error!\n");
			retVal=-1;
			goto FLASH_DEFAULT_END;
		}
#ifdef COMPRESS_MIB_SETTING
	}
#endif	
	
	apmib_reinit();
	retVal=0;
FLASH_DEFAULT_END:
	if(pMib)
	{
		free(pMib);
		pMib=NULL;
	}
	if(pHwmib)
	{
		free(pHwmib);
		pHwmib=NULL;
	}
	if(buff)
	{
		free(buff);
		buff=NULL;
	}
	if(mib_tlv_data)
	{
		free(mib_tlv_data);
		mib_tlv_data = NULL;
	}
	return retVal;
}

///////////////////////////////////////////////////////////////////////////////
static int searchMIB(char *token)
{
	int idx = 0;
	char tmpBuf[100];
	int desired_config=0;

	if (!memcmp(token, "HW_", 3)) {
		config_area = HW_MIB_AREA;
		if (!memcmp(&token[3], "WLAN", 4) && token[8] == '_') {
			apmib_set_wlanidx(token[7] - '0');
			if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE)
				return -1;
			strcpy(tmpBuf, &token[9]);
			desired_config = config_area+1;
		}
		else
			strcpy(tmpBuf, &token[3]);
	}
	else if (!memcmp(token, "DEF_", 4)) {
		config_area = DEF_MIB_AREA;

		if (!memcmp(&token[4], "WLAN", 4) && token[9] == '_') {
			apmib_set_wlanidx(token[8] - '0');
			if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE)
				return -1;
#ifdef MBSSID
			if (!memcmp(&token[10], "VAP", 3) && token[14] == '_') {
				apmib_set_vwlanidx(token[13] - '0');
				if (apmib_get_vwlanidx() >= NUM_VWLAN_INTERFACE) {
					apmib_set_vwlanidx(0);
					return -1;
				}
				apmib_set_vwlanidx(apmib_get_vwlanidx + 1);
				strcpy(tmpBuf, &token[15]);
			}
			else
#endif
			strcpy(tmpBuf, &token[10]);
			desired_config = config_area+1;
		}
		else
			strcpy(tmpBuf, &token[4]);
	}
	else {
		config_area = MIB_AREA;

		if (!memcmp(&token[0], "WLAN", 4) && token[5] == '_') {
			apmib_set_wlanidx(token[4] - '0');
			if (apmib_get_wlanidx() >= NUM_WLAN_INTERFACE)
				return -1;
#ifdef MBSSID
			if (!memcmp(&token[6], "VAP", 3) && token[10] == '_') {
				apmib_set_vwlanidx(token[9] - '0');
				if (apmib_get_vwlanidx() >= NUM_VWLAN_INTERFACE) {
					apmib_set_vwlanidx(0);
					return -1;
				}
				apmib_set_vwlanidx(apmib_get_vwlanidx() + 1);
				strcpy(tmpBuf, &token[11]);
			}
#ifdef UNIVERSAL_REPEATER
			else if (!memcmp(&token[6], "VXD", 3) && token[9] == '_') {
				apmib_set_vwlanidx(NUM_VWLAN_INTERFACE);
				strcpy(tmpBuf, &token[10]);
			}
#endif
			else
#endif
			strcpy(tmpBuf, &token[6]);
			desired_config = config_area+1;
		}
		else
			strcpy(tmpBuf, &token[0]);
	}

	if ( config_area == HW_MIB_AREA ) {
		while (hwmib_table[idx].id) {
			if ( !strcmp(hwmib_table[idx].name, tmpBuf)) {
				if (desired_config && config_area != desired_config)
					return -1;
				return idx;
			}
			idx++;
		}
		idx=0;
		while (hwmib_wlan_table[idx].id) {
			if ( !strcmp(hwmib_wlan_table[idx].name, tmpBuf)) {
				config_area++;
				if (desired_config && config_area != desired_config)
					return -1;
				return idx;
			}
			idx++;
		}
		return -1;
	}
	else {
		while (mib_table[idx].id) {
			if ( !strcmp(mib_table[idx].name, tmpBuf)) {
				if (desired_config && config_area != desired_config)
					return -1;
				return idx;
			}
			idx++;
		}
		idx=0;
		while (mib_wlan_table[idx].id) {
			if ( !strcmp(mib_wlan_table[idx].name, tmpBuf)) {
				config_area++;
				if (desired_config && config_area != desired_config)
					return -1;
				return idx;
			}
			idx++;
		}
		return -1;
	}
}

///////////////////////////////////////////////////////////////////////////////
static void getMIB(char *name, int id, TYPE_T type, int num, int array_separate, char **val)
{
	unsigned char *array_val=(unsigned char*)malloc(2048);
	struct in_addr ia_val;
	void *value;
	unsigned char *tmpBuf=(unsigned char*)malloc(1024), *format=NULL, *buf, tmp1[400];
	int int_val, i;
	unsigned int uint_val=0;
	int index=1, tbl=0;
	char mibName[100]={0};

	if(!array_val||!tmpBuf)
	{
		fprintf(stderr,"malloc fail!\n");
		goto ret;
	}else
	{
		bzero(array_val,2048);
		bzero(tmpBuf,1024);
	}
	if (num ==0)
		goto ret;

	strcat(mibName, name);


getval:
	buf = &tmpBuf[strlen((char *)tmpBuf)];
	switch (type) {
	case BYTE_T:
		value = (void *)&int_val;
		format = (unsigned char *)DEC_FORMAT;
		break;
	case WORD_T:
		value = (void *)&int_val;
		format = (unsigned char *)DEC_FORMAT;
		break;
	case IA_T:
		value = (void *)&ia_val;
		format = (unsigned char *)STR_FORMAT;
		break;
	case BYTE5_T:
		value = (void *)array_val;
		format = (unsigned char *)BYTE5_FORMAT;
		break;
	case BYTE6_T:
		value = (void *)array_val;
		format = (unsigned char *)BYTE6_FORMAT;
		break;
	case BYTE13_T:
		value = (void *)array_val;
		format = (unsigned char *)BYTE13_FORMAT;
		break;

	case STRING_T:
		value = (void *)array_val;
		format = (unsigned char *)STR_FORMAT;
		break;

	case BYTE_ARRAY_T:
		value = (void *)array_val;
		break;

	case DWORD_T:
		value = (void *)&uint_val;
		format = (unsigned char *)UDEC_FORMAT;
		break;

	case WLAC_ARRAY_T:
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
	case MESH_ACL_ARRAY_T:
#endif
	case WDS_ARRAY_T:
	case DHCPRSVDIP_ARRY_T:	
	case SCHEDULE_ARRAY_T:
#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
	case CWMP_WLANCONF_ARRAY_T:
#endif
	case IPFILTER_ARRAY_T:
	case PORTFILTER_ARRAY_T:
	case MACFILTER_ARRAY_T:
	case URLFILTER_ARRAY_T:
	case TRIGGERPORT_ARRAY_T:

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	case QOS_ARRAY_T:
#endif
        
#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
#endif
#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
#endif
#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
	case DNSV6_T:
	case DHCPV6S_T:
	case DHCPV6C_T:
	case ADDR6_T:
	case ADDRV6_T:
	case TUNNEL6_T:
#endif
#endif
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
	case CERTUSER_ARRAY_T:
#endif

#if defined(VLAN_CONFIG_SUPPORTED)
	case VLANCONFIG_ARRAY_T:
#endif		
#ifdef CONFIG_RTL_VLAN_SUPPORT
	case VLAN_ARRAY_T:
	case NETIFACE_ARRAY_T:
	case VLAN_NETIF_BIND_ARRAY_T:
#endif
#ifdef WLAN_PROFILE
	case PROFILE_ARRAY_T:
#endif
		tbl = 1;
		value = (void *)array_val;
		array_val[0] = index++;
		break;
	default: printf("invalid mib!\n"); goto ret;
	}

	if ( !APMIB_GET(id, value)) {
		printf("Get MIB failed!\n");
		goto ret;
	}

	if ( type == IA_T )
		value = inet_ntoa(ia_val);

	if (type == BYTE_T || type == WORD_T)
		sprintf((char *)buf, (char *)format, int_val);
	else if ( type == IA_T || type == STRING_T ) 
	{
		sprintf((char *)buf, (char *)format, value);
		
		if (type == STRING_T ) 
		{
			int tmpBuf1_len = 1024;
			unsigned char * tmpBuf1 = malloc(tmpBuf1_len);
			if(tmpBuf1 == NULL)
			{
				fprintf(stderr,"malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
				goto ret;
			}
			memset(tmpBuf1,0,tmpBuf1_len);
			
			int srcIdx, dstIdx;
			for (srcIdx=0, dstIdx=0; buf[srcIdx]; srcIdx++, dstIdx++) 
			{
				if ( buf[srcIdx] == '"' || buf[srcIdx] == '\\' || buf[srcIdx] == '$' || buf[srcIdx] == '`' || buf[srcIdx] == ' ' )
					tmpBuf1[dstIdx++] = '\\';

				tmpBuf1[dstIdx] = buf[srcIdx];
			}
			if (dstIdx != srcIdx) 
			{
				memcpy(buf, tmpBuf1, dstIdx);
				buf[dstIdx] ='\0';
			}
			free(tmpBuf1);
		}
	}
	else if (type == BYTE5_T) {
		sprintf((char *)buf, (char *)format, array_val[0],array_val[1],array_val[2],
			array_val[3],array_val[4],array_val[5]);
		convert_lower((char *)buf);
	}
	else if (type == BYTE6_T ) {
		sprintf((char *)buf, (char *)format, array_val[0],array_val[1],array_val[2],
			array_val[3],array_val[4],array_val[5],array_val[6]);
		convert_lower((char *)buf);
	}
	else if (type == BYTE13_T) {
		sprintf((char *)buf, (char *)format, array_val[0],array_val[1],array_val[2],
			array_val[3],array_val[4],array_val[5],array_val[6],
			array_val[7],array_val[8],array_val[9],array_val[10],
			array_val[11],array_val[12]);
		convert_lower((char *)buf);
	}
	else if(type == BYTE_ARRAY_T ) {
		int max_chan_num=MAX_2G_CHANNEL_NUM_MIB;
		int chan;

		if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM)
#if defined(CONFIG_WLAN_HAL_8814AE)
			||(id >= MIB_HW_TX_POWER_CCK_C &&  id <=MIB_HW_TX_POWER_CCK_D)
			||(id >= MIB_HW_TX_POWER_HT40_1S_C &&  id <=MIB_HW_TX_POWER_HT40_1S_D)
#endif
			)
			max_chan_num = MAX_2G_CHANNEL_NUM_MIB;
		else if((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM)
#if defined(CONFIG_WLAN_HAL_8814AE)
				||(id >= MIB_HW_TX_POWER_5G_HT40_1S_C &&  id <=MIB_HW_TX_POWER_5G_HT40_1S_D)
#endif
			)
			max_chan_num = MAX_5G_CHANNEL_NUM_MIB;
#if defined(CONFIG_RTL_8812_SUPPORT)
		if(((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_A)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_B)) )
			max_chan_num = MAX_2G_CHANNEL_NUM_MIB;
		
		if(((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_A)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_B)) )
			max_chan_num = MAX_5G_DIFF_NUM;
#endif
		if(val == NULL || val[0] == NULL){
			for(i=0;i< max_chan_num;i++){
				sprintf((char *)tmp1, "%02x", array_val[i]);
				strcat((char *)buf, (char *)tmp1);
			}
			convert_lower((char *)buf);
		}
		else{
			chan = atoi(val[0]);
			if(chan < 1 || chan > max_chan_num){
				printf("invalid channel number\n");
				goto ret;
			}
			sprintf((char *)buf, "%d", *(((unsigned char *)value)+chan-1) );
		}
	}
	else if (type == DWORD_T)
		sprintf((char *)buf, (char *)format, uint_val);

	else if (type == DHCPRSVDIP_ARRY_T) {		
		DHCPRSVDIP_Tp pEntry=(DHCPRSVDIP_Tp)array_val;
		sprintf((char *)buf, DHCPRSVDIP_FORMAT, 			
			pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
			pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5],
			inet_ntoa(*((struct in_addr*)pEntry->ipAddr)), pEntry->hostName
	#ifdef KLD_ENABLED
			, pEntry->enabled
	#endif
			);
	}	
#if defined(VLAN_CONFIG_SUPPORTED)	
	else if (type == VLANCONFIG_ARRAY_T) 
	{
		OPMODE_T opmode=-1;
		int isLan=1;
    #ifdef RTK_USB3G_PORT5_LAN
        DHCP_T wan_dhcp = -1;
        apmib_get( MIB_DHCP, (void *)&wan_dhcp);
    #endif
		apmib_get( MIB_OP_MODE, (void *)&opmode);
		
		VLAN_CONFIG_Tp pEntry=(VLAN_CONFIG_Tp)array_val;

		if(strncmp(pEntry->netIface,"eth1",strlen("eth1")) == 0)
		{			
    #ifdef RTK_USB3G_PORT5_LAN		
            if(opmode == WISP_MODE || opmode == BRIDGE_MODE || wan_dhcp == USB3G)
    #else
			if(opmode == WISP_MODE || opmode == BRIDGE_MODE)
    #endif
				isLan=1;
			else
				isLan=0;
		}
		else if(strncmp("wlan0",pEntry->netIface, strlen(pEntry->netIface)) == 0)
		{						
			if(opmode == WISP_MODE)
				isLan=0;
			else
				isLan=1;
		}
		else
		{						
			isLan=1;
		}

		sprintf((char *)buf, VLANCONFIG_FORMAT, 			
			pEntry->netIface,pEntry->enabled,pEntry->tagged,pEntry->priority,pEntry->cfi, pEntry->vlanId,isLan);
	}		
#endif	
#ifdef CONFIG_RTL_VLAN_SUPPORT
	else if(type == VLAN_ARRAY_T)
	{
		VLAN_Tp pEntry=(VLAN_Tp)array_val;
        #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
		sprintf((char *)buf, "%d %d", pEntry->vlanId, pEntry->ForwardRule);	
        #else
		sprintf((char *)buf, "%d", pEntry->vlanId);	
        #endif
	}
	else if(type == NETIFACE_ARRAY_T)
	{
		NETIFACE_Tp pEntry=(NETIFACE_Tp)array_val;
		sprintf((char *)buf, "%d %d %s %d %d %d",pEntry->netifEnable, pEntry->netifId, pEntry->netifName, pEntry->netifPvid,pEntry->netifDefPriority,pEntry->netifDefCfi);		
	}
	else if(type == VLAN_NETIF_BIND_ARRAY_T)
	{
		VLAN_NETIF_BIND_Tp pEntry=(VLAN_NETIF_BIND_Tp)array_val;
		sprintf((char *)buf, "%d %d %d", pEntry->vlanId, pEntry->netifId, pEntry->tagged);		
	}
#endif
	else if (type == SCHEDULE_ARRAY_T) 
	{
		SCHEDULE_Tp pEntry=(SCHEDULE_Tp)array_val;
		sprintf((char *)buf, SCHEDULE_FORMAT, pEntry->eco, pEntry->day, pEntry->fTime, pEntry->tTime);
	
	}
#ifdef HOME_GATEWAY
	else if (type == PORTFW_ARRAY_T) {
		PORTFW_Tp pEntry=(PORTFW_Tp)array_val;
		sprintf(buf, PORTFW_FORMAT, inet_ntoa(*((struct in_addr*)pEntry->ipAddr)),
			 pEntry->fromPort, pEntry->toPort, pEntry->protoType);
		if ( strlen(pEntry->comment) ) {
			sprintf(tmp1, ", %s", pEntry->comment);
			strcat(buf, tmp1);
		}
	}
#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
		else if (type == CWMP_WLANCONF_ARRAY_T) {
			CWMP_WLANCONF_Tp pEntry=(CWMP_WLANCONF_Tp)array_val;
			//InstanceNum RootIdx VWlanIdx IsConfigured 
			sprintf((char *)buf, CWMP_WLANCONF_FMT, pEntry->InstanceNum, pEntry->RootIdx, pEntry->VWlanIdx, pEntry->IsConfigured, pEntry->RfBandAvailable);
			
		}
#endif		
	else if (type == PORTFILTER_ARRAY_T) {
		PORTFILTER_Tp pEntry=(PORTFILTER_Tp)array_val;
		sprintf(buf, PORTFILTER_FORMAT,
			 pEntry->fromPort, pEntry->toPort, pEntry->protoType);
		if ( strlen(pEntry->comment) ) {
			sprintf(tmp1, ", %s", pEntry->comment);
			strcat(buf, tmp1);
		}
	}
	else if (type == IPFILTER_ARRAY_T) {
		IPFILTER_Tp pEntry=(IPFILTER_Tp)array_val;
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        sprintf(buf, "%s, ", inet_ntoa(*((struct in_addr*)pEntry->ipAddr)));
        sprintf(tmp1, "%d.%d.%d.%d, ",  pEntry->ipAddrEnd[0],pEntry->ipAddrEnd[1], pEntry->ipAddrEnd[2], pEntry->ipAddrEnd[3]);
        strcat(buf, tmp1);
		sprintf(tmp1, "%d", pEntry->protoType);
        strcat(buf, tmp1);
        #else
		sprintf(buf, IPFILTER_FORMAT, inet_ntoa(*((struct in_addr*)pEntry->ipAddr)),
			 pEntry->protoType);
        #endif
		if ( strlen(pEntry->comment) ) {
			sprintf(tmp1, ", %s", pEntry->comment);
			strcat(buf, tmp1);
		}
#ifdef CONFIG_IPV6
		sprintf((char *)tmp1, ", %s, %d", pEntry->ip6Addr,pEntry->ipVer);
		strcat((char *)buf, (char *)tmp1);	
#endif
	}
	else if (type == MACFILTER_ARRAY_T) {
		MACFILTER_Tp pEntry=(MACFILTER_Tp)array_val;
		sprintf(buf, MACFILTER_COLON_FORMAT, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
			 pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5]);
		if ( strlen(pEntry->comment) ) {
			sprintf(tmp1, ", %s", pEntry->comment);
			strcat(buf, tmp1);
		}
	}
	else if (type == URLFILTER_ARRAY_T) {
		URLFILTER_Tp pEntry=(URLFILTER_Tp)array_val;
		if(!strncmp(pEntry->urlAddr,"http://",7))				
			sprintf(buf, STR_FORMAT, pEntry->urlAddr+7);
		else
			sprintf(buf, STR_FORMAT, pEntry->urlAddr);
		//if ( strlen(pEntry->comment) ) {
		//	sprintf(tmp1, ", %s", pEntry->comment);
		//	strcat(buf, tmp1);
		//}
	}
	else if (type == TRIGGERPORT_ARRAY_T) {
		TRIGGERPORT_Tp pEntry=(TRIGGERPORT_Tp)array_val;
		sprintf(buf, TRIGGERPORT_FORMAT,
			 pEntry->tri_fromPort, pEntry->tri_toPort, pEntry->tri_protoType,
			 pEntry->inc_fromPort, pEntry->inc_toPort, pEntry->inc_protoType);
		if ( strlen(pEntry->comment) ) {
			sprintf(tmp1, ", %s", pEntry->comment);
			strcat(buf, tmp1);
		}
	}
#ifdef GW_QOS_ENGINE
	else if (type == QOS_ARRAY_T) {
		QOS_Tp pEntry=(QOS_Tp)array_val;
            strcpy(tmp1, inet_ntoa(*((struct in_addr*)pEntry->local_ip_start)));
            strcpy(&tmp1[20], inet_ntoa(*((struct in_addr*)pEntry->local_ip_end)));
            strcpy(&tmp1[40], inet_ntoa(*((struct in_addr*)pEntry->remote_ip_start)));
            strcpy(&tmp1[60], inet_ntoa(*((struct in_addr*)pEntry->remote_ip_end)));
		sprintf(buf, QOS_FORMAT, pEntry->enabled,
                      pEntry->priority, pEntry->protocol, 
                      tmp1, &tmp1[20], 
                      pEntry->local_port_start, pEntry->local_port_end, 
                      &tmp1[40], &tmp1[60],
			   pEntry->remote_port_start, pEntry->remote_port_end, pEntry->entry_name);
	}
#endif

#ifdef QOS_BY_BANDWIDTH
	else if (type == QOS_ARRAY_T) {
		IPQOS_Tp pEntry=(IPQOS_Tp)array_val;
            strcpy(tmp1, inet_ntoa(*((struct in_addr*)pEntry->local_ip_start)));
            strcpy(&tmp1[20], inet_ntoa(*((struct in_addr*)pEntry->local_ip_end)));
		sprintf(buf, QOS_FORMAT, pEntry->enabled,
                      pEntry->mac[0],pEntry->mac[1],pEntry->mac[2],pEntry->mac[3],pEntry->mac[4],pEntry->mac[5],
                      pEntry->mode, tmp1, &tmp1[20], 
                      (int)pEntry->bandwidth, (int)pEntry->bandwidth_downlink,
			   pEntry->entry_name,pEntry->local_port_start,pEntry->local_port_end,
			   pEntry->protocol,pEntry->weight);
	}
#endif

#ifdef ROUTE_SUPPORT
	else if (type == STATICROUTE_ARRAY_T) {
		char strIp[20], strMask[20], strGw[20];
		STATICROUTE_Tp pEntry=(STATICROUTE_Tp)array_val;
		strcpy(strIp, inet_ntoa(*((struct in_addr*)pEntry->dstAddr)));
                strcpy(strMask, inet_ntoa(*((struct in_addr*)pEntry->netmask)));
                strcpy(strGw, inet_ntoa(*((struct in_addr*)pEntry->gateway)));
		sprintf(buf, "%s, %s, %s, %d, %d",strIp, strMask, strGw, pEntry->interface, pEntry->metric);
	}
#endif //ROUTE
#endif
#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	else if (type == IPSECTUNNEL_ARRAY_T) {
		IPSECTUNNEL_Tp pEntry=(IPSECTUNNEL_Tp)array_val;
		char strLcIp[20], strRtIp[20], strRtGw[20];
		strcpy(strLcIp, inet_ntoa(*((struct in_addr*)pEntry->lc_ipAddr)));
		strcpy(strRtIp, inet_ntoa(*((struct in_addr*)pEntry->rt_ipAddr)));
		strcpy(strRtGw, inet_ntoa(*((struct in_addr*)pEntry->rt_gwAddr)));

		sprintf(buf, IPSECTUNNEL_FORMAT, pEntry->tunnelId, pEntry->enable,  pEntry->connName, 
		pEntry->lcType, strLcIp, pEntry->lc_maskLen, 
		pEntry->rtType, strRtIp, pEntry->rt_maskLen, 
		strRtGw, pEntry->keyMode, 
		pEntry->conType, pEntry->espEncr, pEntry->espAuth, 
		pEntry->psKey, pEntry->ikeEncr, pEntry->ikeAuth, pEntry->ikeKeyGroup, pEntry->ikeLifeTime, 
		pEntry->ipsecLifeTime, pEntry->ipsecPfs, pEntry->spi, pEntry->encrKey, pEntry->authKey, pEntry->authType, pEntry->lcId, pEntry->rtId, pEntry->lcIdType, pEntry->rtIdType, pEntry->rsaKey
		);
	}
#endif
#ifdef CONFIG_IPV6
	else if(type == RADVDPREFIX_T)
		{
			radvdCfgParam_Tp pEntry=(radvdCfgParam_Tp)array_val;
			sprintf(buf,RADVD_FORMAT,
				/*enabled*/
				pEntry->enabled,
				/*interface*/
				pEntry->interface.Name,pEntry->interface.MaxRtrAdvInterval,pEntry->interface.MinRtrAdvInterval,
				pEntry->interface.MinDelayBetweenRAs,pEntry->interface.AdvManagedFlag,pEntry->interface.AdvOtherConfigFlag,pEntry->interface.AdvLinkMTU,
				pEntry->interface.AdvReachableTime,pEntry->interface.AdvRetransTimer,pEntry->interface.AdvCurHopLimit,pEntry->interface.AdvDefaultLifetime,
				pEntry->interface.AdvDefaultPreference,pEntry->interface.AdvSourceLLAddress,pEntry->interface.UnicastOnly,
				/*prefix 1*/
				pEntry->interface.prefix[0].Prefix[0],pEntry->interface.prefix[0].Prefix[1],pEntry->interface.prefix[0].Prefix[2],pEntry->interface.prefix[0].Prefix[3],
				pEntry->interface.prefix[0].Prefix[4],pEntry->interface.prefix[0].Prefix[5],pEntry->interface.prefix[0].Prefix[6],pEntry->interface.prefix[0].Prefix[7],
				pEntry->interface.prefix[0].PrefixLen,pEntry->interface.prefix[0].AdvOnLinkFlag,pEntry->interface.prefix[0].AdvAutonomousFlag,pEntry->interface.prefix[0].AdvValidLifetime,
				pEntry->interface.prefix[0].AdvPreferredLifetime,pEntry->interface.prefix[0].AdvRouterAddr,pEntry->interface.prefix[0].if6to4,pEntry->interface.prefix[0].enabled,
				/*prefix 2*/
				pEntry->interface.prefix[1].Prefix[0],pEntry->interface.prefix[1].Prefix[1],pEntry->interface.prefix[1].Prefix[2],pEntry->interface.prefix[1].Prefix[3],
				pEntry->interface.prefix[1].Prefix[4],pEntry->interface.prefix[1].Prefix[5],pEntry->interface.prefix[1].Prefix[6],pEntry->interface.prefix[1].Prefix[7],
				pEntry->interface.prefix[1].PrefixLen,pEntry->interface.prefix[1].AdvOnLinkFlag,pEntry->interface.prefix[1].AdvAutonomousFlag,pEntry->interface.prefix[1].AdvValidLifetime,
				pEntry->interface.prefix[1].AdvPreferredLifetime,pEntry->interface.prefix[1].AdvRouterAddr,pEntry->interface.prefix[1].if6to4,pEntry->interface.prefix[1].enabled);
		}
	else if(type == DNSV6_T)
		{
			dnsv6CfgParam_Tp pEntry=(dnsv6CfgParam_Tp)array_val;
			sprintf(buf,DNSV6_FORMAT,pEntry->enabled,pEntry->routerName);
		}
	else if(type == DHCPV6S_T)
		{
			dhcp6sCfgParam_Tp pEntry=(dhcp6sCfgParam_Tp)array_val;
			sprintf(buf,DHCPV6S_FORMAT,pEntry->enabled,pEntry->DNSaddr6,pEntry->addr6PoolS,pEntry->addr6PoolE,pEntry->interfaceNameds);
		}	
	else if(type == DHCPV6C_T)
	{
		dhcp6cCfgParam_Tp pEntry=(dhcp6cCfgParam_Tp)array_val;
		sprintf(buf,DHCPV6C_FORMAT,pEntry->enabled,pEntry->ifName,
			pEntry->dhcp6pd.sla_len,pEntry->dhcp6pd.sla_id,pEntry->dhcp6pd.ifName);
			
	}
	else if(type == ADDR6_T)
		{
			daddrIPv6CfgParam_Tp pEntry=(daddrIPv6CfgParam_Tp)array_val;
			sprintf(buf,ADDR6_FORMAT,pEntry->enabled,pEntry->prefix_len[0],pEntry->prefix_len[1],
				pEntry->addrIPv6[0][0],pEntry->addrIPv6[0][1],pEntry->addrIPv6[0][2],pEntry->addrIPv6[0][3],
				pEntry->addrIPv6[0][4],pEntry->addrIPv6[0][5],pEntry->addrIPv6[0][6],pEntry->addrIPv6[0][7],
				pEntry->addrIPv6[1][0],pEntry->addrIPv6[1][1],pEntry->addrIPv6[1][2],pEntry->addrIPv6[1][3],
				pEntry->addrIPv6[1][4],pEntry->addrIPv6[1][5],pEntry->addrIPv6[1][6],pEntry->addrIPv6[1][7]);
		}
	else if(type == ADDRV6_T)
	{
			addr6CfgParam_Tp pEntry=(addr6CfgParam_Tp)array_val;
			sprintf(buf,ADDRV6_FORMAT,pEntry->prefix_len,
				pEntry->addrIPv6[0],pEntry->addrIPv6[1],pEntry->addrIPv6[2],pEntry->addrIPv6[3],
				pEntry->addrIPv6[4],pEntry->addrIPv6[5],pEntry->addrIPv6[6],pEntry->addrIPv6[7]
			);
	}
	else if(type == TUNNEL6_T)
		{
			tunnelCfgParam_Tp pEntry=(tunnelCfgParam_Tp)array_val;
			sprintf(buf,"%d",pEntry->enabled);
		}
#endif
#endif
#ifdef TLS_CLIENT
	else if (type == CERTROOT_ARRAY_T) {
		CERTROOT_Tp pEntry=(CERTROOT_Tp)array_val;
		sprintf(buf, "%s", pEntry->comment);
	}
	else if (type == CERTUSER_ARRAY_T) {
		CERTUSER_Tp pEntry=(CERTUSER_Tp)array_val;
		sprintf(buf, "%s,%s", pEntry->comment, pEntry->pass);
	}	
#endif
	else if (type == WLAC_ARRAY_T) {
		MACFILTER_Tp pEntry=(MACFILTER_Tp)array_val;
		sprintf((char *)buf, MACFILTER_FORMAT, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
			 pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5]);
		if ( strlen((char *)pEntry->comment) ) {
			sprintf((char *)tmp1, ", %s", pEntry->comment);
			strcat((char *)buf, (char *)tmp1);
		}
	}
	
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	else if (type == MESH_ACL_ARRAY_T) {
		MACFILTER_Tp pEntry=(MACFILTER_Tp)array_val;
		sprintf((char *)buf, MACFILTER_FORMAT, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
			 pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5]);
		if ( strlen(pEntry->comment) ) {
			sprintf((char *)tmp1, ", %s", pEntry->comment);
			strcat((char *)buf, (char *)tmp1);
		}
	}
#endif

	else if (type == WDS_ARRAY_T) {
		WDS_Tp pEntry=(WDS_Tp)array_val;
		sprintf((char *)buf, WDS_FORMAT, pEntry->macAddr[0],pEntry->macAddr[1],pEntry->macAddr[2],
			 pEntry->macAddr[3],pEntry->macAddr[4],pEntry->macAddr[5], pEntry->fixedTxRate);
		if ( strlen((char *)pEntry->comment) ) {
			sprintf((char *)tmp1, ", %s", pEntry->comment);
			strcat((char *)buf, (char *)tmp1);
		}
	}
#ifdef WLAN_PROFILE
	else if (type == PROFILE_ARRAY_T) { 	
		WLAN_PROFILE_Tp pEntry=(WLAN_PROFILE_Tp)array_val;
		sprintf((char *)buf, "%s,enc=%d,auth=%d", pEntry->ssid,pEntry->encryption,pEntry->auth);
		if (pEntry->encryption == 1 || pEntry->encryption == 2) {			
			if (pEntry->encryption == WEP64)			
				sprintf(tmp1,",id=%d,key1=%02x%02x%02x%02x%02x,key2=%02x%02x%02x%02x%02x,key3=%02x%02x%02x%02x%02x,key4=%02x%02x%02x%02x%02x", 
					pEntry->wep_default_key,
					pEntry->wepKey1[0],pEntry->wepKey1[1],pEntry->wepKey1[2],pEntry->wepKey1[3],pEntry->wepKey1[4],
					pEntry->wepKey2[0],pEntry->wepKey2[1],pEntry->wepKey2[2],pEntry->wepKey2[3],pEntry->wepKey2[4],
					pEntry->wepKey3[0],pEntry->wepKey3[1],pEntry->wepKey3[2],pEntry->wepKey3[3],pEntry->wepKey3[4],
					pEntry->wepKey4[0],pEntry->wepKey4[1],pEntry->wepKey4[2],pEntry->wepKey4[3],pEntry->wepKey4[4]);
			else
				sprintf(tmp1,",id=%d,key1=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,key2=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,key3=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x,key4=%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
					pEntry->wep_default_key,
					pEntry->wepKey1[0],pEntry->wepKey1[1],pEntry->wepKey1[2],pEntry->wepKey1[3],pEntry->wepKey1[4],pEntry->wepKey1[5],pEntry->wepKey1[6],pEntry->wepKey1[7],pEntry->wepKey1[8],
					pEntry->wepKey1[9],pEntry->wepKey1[10],pEntry->wepKey1[11],pEntry->wepKey1[12],
					pEntry->wepKey2[0],pEntry->wepKey2[1],pEntry->wepKey2[2],pEntry->wepKey2[3],pEntry->wepKey2[4],pEntry->wepKey2[5],pEntry->wepKey2[6],pEntry->wepKey2[7],pEntry->wepKey2[8],
					pEntry->wepKey2[9],pEntry->wepKey2[10],pEntry->wepKey2[11],pEntry->wepKey2[12],
					pEntry->wepKey3[0],pEntry->wepKey3[1],pEntry->wepKey3[2],pEntry->wepKey3[3],pEntry->wepKey3[4],pEntry->wepKey3[5],pEntry->wepKey3[6],pEntry->wepKey3[7],pEntry->wepKey3[8],
					pEntry->wepKey3[9],pEntry->wepKey3[10],pEntry->wepKey3[11],pEntry->wepKey3[12],
					pEntry->wepKey4[0],pEntry->wepKey4[1],pEntry->wepKey4[2],pEntry->wepKey4[3],pEntry->wepKey4[4],pEntry->wepKey4[5],pEntry->wepKey4[6],pEntry->wepKey4[7],pEntry->wepKey4[8],
					pEntry->wepKey4[9],pEntry->wepKey4[10],pEntry->wepKey4[11],pEntry->wepKey4[12]);		
			strcat(buf, tmp1);			
		}
		
		else if (pEntry->encryption == 3 || pEntry->encryption == 4) {
			sprintf(tmp1, ",wpa_cipher=%d,psk=%s", pEntry->wpa_cipher, pEntry->wpaPSK); 		
			strcat(buf, tmp1);						
		}		
	}
#endif	

	if (--num > 0) {
		if (!array_separate)
			strcat((char *)tmpBuf, " ");
		else {
			if (tbl){
				if(type == STRING_T)
					printf("%s%d=\"%s\"\n", mibName, index-1, tmpBuf);
				else
					printf("%s%d=%s\n", mibName, index-1, tmpBuf);
			}
			else{
				if(type == STRING_T)
					printf("%s=\"%s\"\n", mibName, tmpBuf);
				else
					printf("%s=%s\n", mibName, tmpBuf);
			}
			tmpBuf[0] = '\0';
		}
		goto getval;
	}
ret:
	if (tbl) {
		if(type == STRING_T)
			printf("%s%d=\"%s\"\n", mibName, index-1, tmpBuf);
		else
			printf("%s%d=%s\n", mibName, index-1, tmpBuf);
	}
	else{
		if(type == STRING_T)
                                printf("%s=\"%s\"\n", mibName, tmpBuf);
		else
			printf("%s=%s\n", mibName, tmpBuf);
	}
	if(tmpBuf)
	{
		free(tmpBuf);
		tmpBuf=NULL;
	}
	if(array_val)
	{
		free(array_val);
		array_val=NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////
static void setMIB(char *name, int id, TYPE_T type, int len, int valNum, char **val)
{
	unsigned char key[200];
	struct in_addr ia_val;
	void *value=NULL;
	int int_val, i;
	unsigned int uint_val=0;
	MACFILTER_T wlAc;	// Use with MESH_ACL
	WDS_T wds;

#ifdef HOME_GATEWAY
	PORTFW_T portFw;
	PORTFILTER_T portFilter;
	IPFILTER_T ipFilter;
	MACFILTER_T macFilter;
	URLFILTER_T urlFilter;
	TRIGGERPORT_T triggerPort;

#ifdef GW_QOS_ENGINE
	QOS_T qos;
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_T qos;
#endif

#ifdef ROUTE_SUPPORT
	STATICROUTE_T staticRoute;
#endif
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	IPSECTUNNEL_T ipsecTunnel;
#endif

#ifdef CONFIG_IPV6
	char tmp[5];
	char *pstart ,*pend;
	int size;
	radvdCfgParam_t radvdCfgParam;
	dnsv6CfgParam_t dnsv6CfgParam;
	dhcp6sCfgParam_t dhcp6sCfgParam;
	dhcp6cCfgParam_t dhcp6cCfgParam;
	addrIPv6CfgParam_t addrIPv6CfgParam;
	addr6CfgParam_t addr6CfgParam;
	tunnelCfgParam_t tunnelCfgParam;
#endif
#endif
#ifdef TLS_CLIENT
	CERTROOT_T certRoot;
	CERTUSER_T certUser;
#endif
	DHCPRSVDIP_T dhcpRsvd;
	
#if defined(VLAN_CONFIG_SUPPORTED)	
	VLAN_CONFIG_T vlanConfig_entry;
#endif		
#ifdef CONFIG_RTL_VLAN_SUPPORT
	VLAN_T vlanEntry;
	NETIFACE_T netIfEntry;
	VLAN_NETIF_BIND_T vlan_netif_bindEntry;
#endif
#ifdef WLAN_PROFILE
		WLAN_PROFILE_T profile;
#endif
SCHEDULE_T wlschedule[2]={0};

	int entryNum;
	int max_chan_num=0, tx_power_cnt=0;

	switch (type) {
	case BYTE_T:
	case WORD_T:
		int_val = atoi(val[0]);
		value = (void *)&int_val;
		break;

	case IA_T:
		if ( !inet_aton(val[0], &ia_val) ) {
			printf("invalid internet address!\n");
			return;
		}
		value = (void *)&ia_val;
		break;

	case BYTE5_T:
		if ( strlen(val[0])!=10 || !string_to_hex(val[0], key, 10)) {
			printf("invalid value!\n");
			return;
		}
		value = (void *)key;
		break;

	case BYTE6_T:
		if ( strlen(val[0])!=12 || !string_to_hex(val[0], key, 12)) {
			printf("invalid value!\n");
			return;
		}
		value = (void *)key;
		break;

	case BYTE_ARRAY_T:
#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
	if(!(id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM) &&
			!(id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM)
#if defined(CONFIG_RTL_8812_SUPPORT)
			&& !(id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_B)
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
        && !(id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C &&  id <=MIB_HW_TX_POWER_5G_HT40_1S_D)
#endif
			){
				printf("invalid mib!\n");
				return;
			}
		if((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM)
#if defined(CONFIG_WLAN_HAL_8814AE)
			||(id >= MIB_HW_TX_POWER_CCK_C &&  id <=MIB_HW_TX_POWER_CCK_D)
			||(id >= MIB_HW_TX_POWER_HT40_1S_C &&  id <=MIB_HW_TX_POWER_HT40_1S_D)
#endif
			)
			max_chan_num = MAX_2G_CHANNEL_NUM_MIB;
		else if((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM)
#if defined(CONFIG_WLAN_HAL_8814AE)
			||(id >= MIB_HW_TX_POWER_5G_HT40_1S_C &&  id <=MIB_HW_TX_POWER_5G_HT40_1S_D)
#endif
			)
			max_chan_num = MAX_5G_CHANNEL_NUM_MIB;
#if defined(CONFIG_RTL_8812_SUPPORT)
		if(((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_A)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_B)) )
			max_chan_num = MAX_2G_CHANNEL_NUM_MIB;

		if(((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_A)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_B)) )
			max_chan_num = MAX_5G_DIFF_NUM;
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
		if(((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_C)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D) && (id <= MIB_HW_TX_POWER_DIFF_OFDM4T_CCK4T_D)) )
			max_chan_num = MAX_2G_CHANNEL_NUM_MIB;

		if(((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_C)) 
			|| ((id >= MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D) && (id <= MIB_HW_TX_POWER_DIFF_5G_80BW4S_160BW4S_D)) )
			max_chan_num = MAX_5G_DIFF_NUM;
#endif

			
			for (i=0; i<max_chan_num ; i++) {
				if(val[i] == NULL) break;
				if ( !sscanf(val[i], "%d", &int_val) ) {
					printf("invalid value!\n");
					return;
				}
				key[i+1] = (unsigned char)int_val;
				tx_power_cnt ++;
			}	
			if(tx_power_cnt != 1 && tx_power_cnt !=2 && tx_power_cnt != max_chan_num){
				unsigned char key_tmp[200];
				memcpy(key_tmp, key+1, tx_power_cnt);		
				APMIB_GET(id, key+1);
				memcpy(key+1, key_tmp, tx_power_cnt);
			}
			if(tx_power_cnt == 1){
				for(i=1 ; i <= max_chan_num; i++) {
					key[i] = key[1];
				}
			}
		else if(tx_power_cnt ==2){
			//key[1] is channel number to set
			//key[2] is tx power value
	                //key[3] is tx power key for check set mode
			if(key[1] < 1 || key[1] > max_chan_num){
				if((key[1]<1) || ((id >= MIB_HW_TX_POWER_CCK_A &&  id <=MIB_HW_TX_POWER_DIFF_OFDM)) ||
					 ((id >= MIB_HW_TX_POWER_5G_HT40_1S_A &&  id <=MIB_HW_TX_POWER_DIFF_5G_OFDM) && (key[1]>216))){
					printf("invalid channel number\n");
					return;
				}
			}
			key[3] = 0xff ;
		}
		key[0] = tx_power_cnt;
#endif
		value = (void *)key;
		break;
	case DWORD_T:
		uint_val = strtoul(val[0],0,10);//atoi(val[0]);
		value = (void *)&uint_val;
		break;

#ifdef HOME_GATEWAY
	case PORTFW_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_PORTFW_ADD;
			if ( valNum < 5 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( !inet_aton(val[1], (struct in_addr *)&portFw.ipAddr)) {
				printf("invalid internet address!\n");
				return;
			}
			portFw.fromPort = atoi(val[2]);
			portFw.toPort = atoi(val[3]);
			portFw.protoType = atoi(val[4]);
			if ( valNum > 5)
				strcpy(portFw.comment, val[5]);
			else
				portFw.comment[0] = '\0';

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_PORTFW_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_PORTFW_TBL_NUM, (void *)&entryNum)) {
				printf("Get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&portFw) = (char)int_val;
			if ( !APMIB_GET(MIB_PORTFW_TBL, (void *)&portFw)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_PORTFW_DELALL;

		value = (void *)&portFw;
		break;

	case PORTFILTER_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_PORTFILTER_ADD;
			if ( valNum < 4 ) {
				printf("input argument is not enough!\n");
				return;
			}
			portFilter.fromPort = atoi(val[1]);
			portFilter.toPort = atoi(val[2]);
			portFilter.protoType = atoi(val[3]);
			if ( valNum > 4)
				strcpy(portFilter.comment, val[4]);
			else
				portFilter.comment[0] = '\0';
#ifdef CONFIG_IPV6
			portFilter.ipVer = atoi(val[5]);
#endif

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_PORTFILTER_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_PORTFILTER_TBL_NUM, (void *)&entryNum)) {
				printf("Get port filter entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&portFilter) = (char)int_val;
			if ( !APMIB_GET(MIB_PORTFILTER_TBL, (void *)&portFilter)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_PORTFILTER_DELALL;

		value = (void *)&portFilter;
		break;

	case IPFILTER_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_IPFILTER_ADD;
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
			if ( valNum < 4 ) {
            #else
			if ( valNum < 3 ) {
            #endif
				printf("input argument is not enough!\n");
				return;
			}
			if ( !inet_aton(val[1], (struct in_addr *)&ipFilter.ipAddr)) {
				printf("invalid internet address!\n");
				return;
			}
            
            #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
            if (!(!strcmp(val[2], "0") || !strcmp(val[2], "0.0.0.0")))
                inet_aton(val[2], (struct in_addr *)&ipFilter.ipAddrEnd);
            else
                memset(ipFilter.ipAddrEnd, 0x00, 4);
            ipFilter.protoType = atoi(val[3]);
			if ( valNum > 4)
				strcpy((char *)ipFilter.comment, val[4]);
			else
				ipFilter.comment[0] = '\0';
#ifdef CONFIG_IPV6
			if ( valNum > 5)
				strcpy(ipFilter.ip6Addr, val[5]);
			else
				memset(ipFilter.ip6Addr,0,48);

			if(valNum > 6)
				ipFilter.ipVer = atoi(val[6]);
			else
				ipFilter.ipVer = 0;
#endif
            #else
			ipFilter.protoType = atoi(val[2]);
			if ( valNum > 3)
				strcpy((char *)ipFilter.comment, val[3]);
			else
				ipFilter.comment[0] = '\0';
#ifdef CONFIG_IPV6
			if ( valNum > 4)
				strcpy(ipFilter.ip6Addr, val[4]);
			else
				memset(ipFilter.ip6Addr,0,48);

			if(valNum > 5)
				ipFilter.ipVer = atoi(val[5]);
			else
				ipFilter.ipVer = 0;
#endif
            #endif            


		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_IPFILTER_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_IPFILTER_TBL_NUM, (void *)&entryNum)) {
				printf("Get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&ipFilter) = (char)int_val;
			if ( !APMIB_GET(MIB_IPFILTER_TBL, (void *)&ipFilter)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_IPFILTER_DELALL;
		
		value = (void *)&ipFilter;
		break;

	case MACFILTER_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_MACFILTER_ADD;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( strlen(val[1])!=12 || !string_to_hex(val[1], wlAc.macAddr, 12)) {
				printf("invalid value!\n");
				return;
			}

			if ( valNum > 2)
				strcpy(macFilter.comment, val[2]);
			else
				macFilter.comment[0] = '\0';

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_MACFILTER_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_MACFILTER_TBL_NUM, (void *)&entryNum)) {
				printf("Get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&macFilter) = (char)int_val;
			if ( !APMIB_GET(MIB_MACFILTER_TBL, (void *)&macFilter)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_MACFILTER_DELALL;
		value = (void *)&macFilter;
		break;

	case URLFILTER_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_URLFILTER_ADD;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			strcpy(urlFilter.urlAddr, val[1]);

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_URLFILTER_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_URLFILTER_TBL_NUM, (void *)&entryNum)) {
				printf("Get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&urlFilter) = (char)int_val;
			if ( !APMIB_GET(MIB_URLFILTER_TBL, (void *)&urlFilter)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_URLFILTER_DELALL;
		value = (void *)&urlFilter;
		break;
		
	case TRIGGERPORT_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_TRIGGERPORT_ADD;
			if ( valNum < 7 ) {
				printf("input argument is not enough!\n");
				return;
			}
			triggerPort.tri_fromPort = atoi(val[1]);
			triggerPort.tri_toPort = atoi(val[2]);
			triggerPort.tri_protoType = atoi(val[3]);
			triggerPort.inc_fromPort = atoi(val[4]);
			triggerPort.inc_toPort = atoi(val[5]);
			triggerPort.inc_protoType = atoi(val[6]);

			if ( valNum > 7)
				strcpy(triggerPort.comment, val[7]);
			else
				triggerPort.comment[0] = '\0';

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_TRIGGERPORT_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_TRIGGERPORT_TBL_NUM, (void *)&entryNum)) {
				printf("Get trigger-port entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&triggerPort) = (char)int_val;
			if ( !APMIB_GET(MIB_TRIGGERPORT_TBL, (void *)&triggerPort)) {
				printf("Get trigger-port table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_TRIGGERPORT_DELALL;

		value = (void *)&triggerPort;
		break;
#ifdef GW_QOS_ENGINE
	case QOS_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_QOS_ADD;
			if ( valNum < 13 ) {
				printf("input argument is not enough!\n");
				return;
			}
			qos.enabled = atoi(val[1]);
			if ( !inet_aton(val[4], (struct in_addr *)&qos.local_ip_start) ||
                        !inet_aton(val[5], (struct in_addr *)&qos.local_ip_end) ||
                        !inet_aton(val[8], (struct in_addr *)&qos.remote_ip_start) ||
                        !inet_aton(val[9], (struct in_addr *)&qos.remote_ip_end) 
                      ) {
				printf("invalid internet address!\n");
				return;
			}
			qos.priority = atoi(val[2]);
			qos.protocol = atoi(val[3]);
			qos.local_port_start = atoi(val[6]);
			qos.local_port_end = atoi(val[7]);
			qos.remote_port_start = atoi(val[10]);
			qos.remote_port_end = atoi(val[11]);
			strcpy(qos.entry_name, val[12]);

		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_QOS_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
				printf("Get QoS entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&qos) = (char)int_val;
			if ( !APMIB_GET(MIB_QOS_RULE_TBL, (void *)&qos)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_QOS_DELALL;

		value = (void *)&qos;
		break;
#endif

#ifdef QOS_BY_BANDWIDTH
	case QOS_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_QOS_ADD;
			if ( valNum < 12 ) {
				printf("input argument is not enough!\n");
				return;
			}
			qos.enabled = atoi(val[1]);
			if ( !inet_aton(val[4], (struct in_addr *)&qos.local_ip_start) ||
                        !inet_aton(val[5], (struct in_addr *)&qos.local_ip_end) 
                      ) {
				printf("invalid internet address!\n");
				return;
			}
			//strcpy(qos.mac, val[2]);
			if (strlen(val[2])!=12 || !string_to_hex(val[2], qos.mac, 12))  {
				printf("invalid MAC address!\n");
				return;			
			}
			qos.mode = atoi(val[3]);
			qos.bandwidth = atoi(val[6]);
			strcpy(qos.entry_name, val[7]);
			qos.bandwidth_downlink = atoi(val[8]);
			qos.local_port_start = atoi(val[9]);
			qos.local_port_end = atoi(val[10]);
			qos.protocol = atoi(val[11]);
			qos.weight = atoi(val[12]);
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_QOS_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_QOS_RULE_TBL_NUM, (void *)&entryNum)) {
				printf("Get QoS entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&qos) = (char)int_val;
			if ( !APMIB_GET(MIB_QOS_RULE_TBL, (void *)&qos)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_QOS_DELALL;

		value = (void *)&qos;
		break;
#endif

#ifdef ROUTE_SUPPORT
		case STATICROUTE_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_STATICROUTE_ADD;
			if ( valNum < 3 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( !inet_aton(val[1], (struct in_addr *)&staticRoute.dstAddr)) {
                                printf("invalid destination IP address!\n");
                                return;
                        }
			if ( !inet_aton(val[2], (struct in_addr *)&staticRoute.netmask)) {
                                printf("invalid netmask !\n");
                                return;
                        }
			if ( !inet_aton(val[3], (struct in_addr *)&staticRoute.gateway)) {
                                printf("invalid gateway address!\n");
                                return;
                        }
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_STATICROUTE_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_STATICROUTE_TBL_NUM, (void *)&entryNum)) {
				printf("Get trigger-port entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&staticRoute) = (char)int_val;
			if ( !APMIB_GET(MIB_STATICROUTE_TBL, (void *)&staticRoute)) {
				printf("Get trigger-port table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_STATICROUTE_DELALL;

		value = (void *)&staticRoute;
		break;
#endif //ROUTE

#endif

	case SCHEDULE_ARRAY_T:
//		#ifdef NEW_SCHEDULE_SUPPORT

		if ( !strcmp(val[0], "mod")) {

			id=MIB_WLAN_SCHEDULE_MOD;
			if ( valNum < 5 ) {
				printf("input argument is not enough!\n");
				return;
			}
			
			int_val = atoi(val[1]);
			//printf("%d int_val=%d\n",__LINE__,int_val);
			if ( !APMIB_GET(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&entryNum)) {
				printf("Get WLAN_SCHEDULE entry number error!");
				return;
			}
			if ( int_val > entryNum || int_val<1) {
				printf("Element number must be 1-10!\n");
				return;
			}
			//printf("%d entryNum=%d\n",__LINE__,entryNum);
			*((char *)&(wlschedule[0])) = (char)int_val;
			if ( !APMIB_GET(MIB_WLAN_SCHEDULE_TBL, (void *)&(wlschedule[0]))) {
				printf("Get table entry error!");
				return;
			}
			//printf("%d ec0=%d day=%d ft=%d tt=%d\n",__LINE__,wlschedule[0].eco,wlschedule[0].day,wlschedule[0].fTime,wlschedule[0].tTime);
			wlschedule[1]=wlschedule[0];
			wlschedule[1].eco=atoi(val[2]);
			wlschedule[1].day=atoi(val[3]);
			wlschedule[1].fTime=atoi(val[4]);
			wlschedule[1].tTime=atoi(val[5]);
			if((wlschedule[1].eco==0 ||wlschedule[1].eco==1)
				&&(wlschedule[1].day>=0 && wlschedule[1].day<=7)
				&&(wlschedule[1].fTime>=0 &&wlschedule[1].fTime<1440)
				&&(wlschedule[1].tTime>=0 &&wlschedule[1].tTime<1440)
				&&(wlschedule[1].fTime<=wlschedule[1].tTime)
				)
				{
					value = (void *)wlschedule;
					break;
				}
		}

			printf("cmd not support! try\n flash set [MIB] mod [index(1-%d)] [Enable(0/1)] [day(0-7)] [formTime(min,0-1440)] [toTime(min,0-1440)]\n",MAX_SCHEDULE_NUM);
			return;

		
//#endif

		break;
	

		break;
	case DHCPRSVDIP_ARRY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_DHCPRSVDIP_ADD;
			if ( valNum < 3 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( strlen(val[1])!=12 || !string_to_hex(val[1], dhcpRsvd.macAddr, 12)) {
				printf("invalid value!\n");
				return;
			}
			if (!inet_aton(val[2], (struct in_addr *)&dhcpRsvd.ipAddr)) {
				printf("invalid internet address!\n");
				return;
			}		
			if ( valNum > 3 )
				strcpy((char *)dhcpRsvd.hostName, val[3]);	
		#ifdef KLD_ENABLED
			if (valNum > 4)
				dhcpRsvd.enabled=val[4];
		#endif
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_DHCPRSVDIP_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_DHCPRSVDIP_TBL_NUM, (void *)&entryNum)) {
				printf("Get DHCP resvd IP entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&dhcpRsvd) = (char)int_val;
			if ( !APMIB_GET(MIB_DHCPRSVDIP_TBL, (void *)&dhcpRsvd)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_DHCPRSVDIP_DELALL;
		value = (void *)&dhcpRsvd;
		break;
#if defined(VLAN_CONFIG_SUPPORTED)	
		case VLANCONFIG_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_VLANCONFIG_ADD;
			if ( valNum < 6 ) {
				printf("input argument is not enough!\n");
				return;
			}
			vlanConfig_entry.enabled=(unsigned char)atoi(val[1]);
			sprintf(vlanConfig_entry.netIface, "%s", val[2]);
			vlanConfig_entry.tagged = (unsigned char)atoi(val[3]);
			vlanConfig_entry.priority = (unsigned char)atoi(val[4]);
			vlanConfig_entry.cfi = (unsigned char)atoi(val[5]);
			vlanConfig_entry.vlanId = (unsigned short)atoi(val[6]);
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_VLANCONFIG_DEL;
			if ( valNum < 8 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]); //index of entry
			if ( !APMIB_GET(MIB_VLANCONFIG_TBL_NUM, (void *)&entryNum)) {
				printf("Get VLAN config entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&vlanConfig_entry) = (char)int_val;
			if ( !APMIB_GET(MIB_VLANCONFIG_TBL, (void *)&vlanConfig_entry)) {
				printf("Get table entry error!");
				return;
			}
			vlanConfig_entry.enabled=0;

		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_VLANCONFIG_DELALL;
		value = (void *)&vlanConfig_entry;
		break;
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
	case VLAN_ARRAY_T:		
		if ( !strcmp(val[0], "add")) 
		{
			id = MIB_VLAN_ADD;
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
			if ( valNum < 3 ) {
            #else
			if ( valNum < 2 ) {
            #endif
				printf("input argument is not enough!\n");
				return;
			}
            #ifdef CONFIG_RTL_BRIDGE_VLAN_SUPPORT
			vlanEntry.vlanId=(unsigned short)atoi(val[1]);
            vlanEntry.ForwardRule=(unsigned short)atoi(val[2]);
            #else
			vlanEntry.vlanId=(unsigned short)atoi(val[1]);
            #endif
					
		}
		else if ( !strcmp(val[0], "del")) 
		{
			id = MIB_VLAN_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_VLAN_TBL_NUM, (void *)&entryNum)) {
				printf("Get vlan entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&vlanEntry) = (char)int_val;
			if ( !APMIB_GET(MIB_VLAN_TBL, (void *)&vlanEntry)) {
				printf("Get table entry error!");
				return;
			}		
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_VLAN_DELALL;
		value = (void *)&vlanEntry;
		break;
	case NETIFACE_ARRAY_T:
		if ( !strcmp(val[0], "add")) 
		{
			id = MIB_NETIFACE_ADD;
			if ( valNum < 4 ) {
				printf("input argument is not enough!\n");
				return;
			}					
			netIfEntry.netifId=(unsigned char)atoi(val[1]);
			strcpy(netIfEntry.netifName,val[2]);
			netIfEntry.netifPvid=(unsigned short)atoi(val[3]);
					
		}
		else if ( !strcmp(val[0], "del")) 
		{
			id = MIB_NETIFACE_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_NETIFACE_TBL_NUM, (void *)&entryNum)) {
				printf("Get vlan entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&netIfEntry) = (char)int_val;
			if ( !APMIB_GET(MIB_NETIFACE_TBL, (void *)&netIfEntry)) {
				printf("Get table entry error!");
				return;
			}		
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_NETIFACE_DELALL;
		value = (void *)&netIfEntry;
		break;
	case VLAN_NETIF_BIND_ARRAY_T:
		if ( !strcmp(val[0], "add")) 
		{
			id = MIB_VLAN_NETIF_BIND_ADD;
			if ( valNum < 4 ) {
				printf("input argument is not enough!\n");
				return;
			}					
			vlan_netif_bindEntry.vlanId=(unsigned short)atoi(val[1]);
			vlan_netif_bindEntry.netifId=(unsigned char)atoi(val[2]);
			vlan_netif_bindEntry.tagged=(unsigned char)atoi(val[3]);
					
		}
		else if ( !strcmp(val[0], "del")) 
		{
			id = MIB_VLAN_NETIF_BIND_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&entryNum)) {
				printf("Get vlan entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&vlan_netif_bindEntry) = (char)int_val;
			if ( !APMIB_GET(MIB_VLAN_NETIF_BIND_TBL, (void *)&vlan_netif_bindEntry)) {
				printf("Get table entry error!");
				return;
			}		
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_VLAN_NETIF_BIND_DELALL;
		value = (void *)&netIfEntry;
		break;
#endif
	case WLAC_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_WLAN_AC_ADDR_ADD;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( strlen(val[1])!=12 || !string_to_hex(val[1], wlAc.macAddr, 12)) {
				printf("invalid value!\n");
				return;
			}

			if ( valNum > 2)
				strcpy((char *)wlAc.comment, val[2]);
			else
				wlAc.comment[0] = '\0';
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_WLAN_AC_ADDR_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_WLAN_MACAC_NUM, (void *)&entryNum)) {
				printf("Get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&wlAc) = (char)int_val;
			if ( !APMIB_GET(MIB_WLAN_MACAC_ADDR, (void *)&wlAc)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_WLAN_AC_ADDR_DELALL;
		value = (void *)&wlAc;
		break;

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	case MESH_ACL_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_WLAN_MESH_ACL_ADDR_ADD;
			if ( valNum < 2 ) {
				printf("Mesh Acl Addr input argument is not enough!\n");
				return;
			}
			if ( strlen(val[1])!=12 || !string_to_hex(val[1], wlAc.macAddr, 12)) {
				printf("Mesh Acl Addr invalid value!\n");
				return;
			}

			if ( valNum > 2)
				strcpy(wlAc.comment, val[2]);
			else
				wlAc.comment[0] = '\0';
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_WLAN_MESH_ACL_ADDR_DEL;
			if ( valNum < 2 ) {
				printf("Mesh Acl Addr input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_WLAN_MESH_ACL_NUM, (void *)&entryNum)) {
				printf("Mesh Acl Addr get port forwarding entry number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Mesh Acl Addr element number is too large!\n");
				return;
			}
			*((char *)&wlAc) = (char)int_val;
			if ( !APMIB_GET(MIB_WLAN_MESH_ACL_ADDR, (void *)&wlAc)) {
				printf("Mesh Acl Addr get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_WLAN_MESH_ACL_ADDR_DELALL;
		value = (void *)&wlAc;
		break;
#endif	// CONFIG_RTK_MESH && _MESH_ACL_ENABLE_

	case WDS_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_WLAN_WDS_ADD;
			if ( valNum < 3 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( strlen(val[1])!=12 || !string_to_hex(val[1], wds.macAddr, 12)) {
				printf("invalid value!\n");
				return;
			}

			if ( valNum > 2)
				strcpy((char *)wds.comment, val[2]);
			else
				wds.comment[0] = '\0';
				
			wds.fixedTxRate = strtoul(val[3],0,10);//atoi(val[3]);	
		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_WLAN_WDS_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_WLAN_WDS_NUM, (void *)&entryNum)) {
				printf("Get wds number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&wds) = (char)int_val;
			if ( !APMIB_GET(MIB_WLAN_WDS, (void *)&wds)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_WLAN_WDS_DELALL;
		value = (void *)&wds;
		break;

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
	case IPSECTUNNEL_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			id = MIB_IPSECTUNNEL_ADD;
			if ( valNum < 27 ) {
				printf("input argument is not enough!\n");
				return;
			}
			if ( !inet_aton(val[5], (struct in_addr *)&ipsecTunnel.lc_ipAddr)) {
				printf("invalid local IP address!\n");
				return;
			}
			
			if ( !inet_aton(val[8], (struct in_addr *)&ipsecTunnel.rt_ipAddr)) {
				printf("invalid remote IP address!\n");
				return;
			}
			if ( !inet_aton(val[10], (struct in_addr *)&ipsecTunnel.rt_gwAddr)) {
				printf("invalid remote gateway address!\n");
				return;
			}
			ipsecTunnel.tunnelId =  atoi(val[1]); 
			ipsecTunnel.enable = atoi(val[2]);
			ipsecTunnel.lcType = atoi(val[4]);

			if(strlen(val[3]) > (MAX_NAME_LEN-1)){
				printf("Connection Name too long !\n");
				return;
			}else
				strcpy(ipsecTunnel.connName, val[3]); 
			ipsecTunnel.lc_maskLen = atoi(val[6]);
			ipsecTunnel.rt_maskLen  = atoi(val[9]);
			ipsecTunnel.keyMode= atoi(val[11]);
			ipsecTunnel.conType = atoi(val[12]);
			ipsecTunnel.espEncr = atoi(val[13]);
			ipsecTunnel.espAuth = atoi(val[14]);
			if(strlen(val[15]) >  (MAX_NAME_LEN-1)){
				printf("Preshared Key too long !\n");
				return;
			}else
				strcpy(ipsecTunnel.psKey, val[15]); 

			ipsecTunnel.ikeEncr = atoi(val[16]);
			ipsecTunnel.ikeAuth = atoi(val[17]);
			ipsecTunnel.ikeKeyGroup = atoi(val[18]);
			ipsecTunnel.ikeLifeTime= strtol(val[19], (char **)NULL, 10);

			ipsecTunnel.ipsecLifeTime= strtol(val[20], (char **)NULL, 10);
			ipsecTunnel.ipsecPfs= atoi(val[21]);
			if(strlen(val[22]) >  (MAX_SPI_LEN-1)){
				printf("SPI too long !\n");
				return;
			}else
				strcpy(ipsecTunnel.spi, val[22]); 

			if(strlen(val[23]) >  (MAX_ENCRKEY_LEN-1)){
				printf("Encryption key too long !\n");
				return;
			}else
				strcpy(ipsecTunnel.encrKey, val[23]); 

			if(strlen(val[24]) >  (MAX_AUTHKEY_LEN-1)){
				printf("Authentication key too long !\n");
				return;
			}else
				strcpy(ipsecTunnel.authKey, val[24]); 


		}
		else if ( !strcmp(val[0], "del")) {
			id = MIB_IPSECTUNNEL_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_IPSECTUNNEL_TBL_NUM, (void *)&entryNum)) {
				printf("Get ipsec tunnel number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&ipsecTunnel) = (char)int_val;
			if ( !APMIB_GET(MIB_IPSECTUNNEL_TBL, (void *)&ipsecTunnel)) {
				printf("Get table entry error!");
				return;
			}
		}
		else if ( !strcmp(val[0], "delall"))
			id = MIB_IPSECTUNNEL_DELALL;
		value = (void *)&ipsecTunnel;
		break;

#endif

#ifdef CONFIG_IPV6
	case RADVDPREFIX_T:
		if(valNum != 24 || valNum != 33)
			printf("input argumentis not enough");
		radvdCfgParam.enabled=atoi(val[0]);
		strcpy(radvdCfgParam.interface.Name,val[1]);
		radvdCfgParam.interface.MaxRtrAdvInterval=atoi(val[3]);
		radvdCfgParam.interface.MinRtrAdvInterval=atoi(val[4]);
		radvdCfgParam.interface.MinDelayBetweenRAs=atoi(val[5]);
		radvdCfgParam.interface.AdvManagedFlag=atoi(val[6]);
		radvdCfgParam.interface.AdvOtherConfigFlag=atoi(val[7]);
		radvdCfgParam.interface.AdvLinkMTU=atoi(val[8]);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		radvdCfgParam.interface.AdvReachableTime=strtoul(val[9],NULL,10);
		radvdCfgParam.interface.AdvRetransTimer=strtoul(val[10],NULL,10);
		radvdCfgParam.interface.AdvCurHopLimit=atoi(val[11]);
		strcpy(radvdCfgParam.interface.AdvDefaultPreference,val[12]);
		radvdCfgParam.interface.AdvSourceLLAddress=atoi(val[13]);
		radvdCfgParam.interface.UnicastOnly=atoi(val[14]);

		/*prefix1*/
		memset(tmp,0,5);
		pstart=val[15];
		pend=pstart;
		for(i=0;i<8;i++)
		{
			size=0;
			while(*pend !=':')
			{
				pend++;
				size++;
			}
			pend++;
			if(*pend == ':')
				pend++;
			memcpy(tmp,pstart,size);
			pstart=pend;
			radvdCfgParam.interface.prefix[0].Prefix[i]=strtol(tmp,NULL,16);
		}
		radvdCfgParam.interface.prefix[0].PrefixLen=atoi(val[16]);
		radvdCfgParam.interface.prefix[0].AdvOnLinkFlag=atoi(val[17]);
		radvdCfgParam.interface.prefix[0].AdvAutonomousFlag=atoi(val[18]);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		radvdCfgParam.interface.prefix[0].AdvValidLifetime=strtoul(val[19],NULL,10);
		radvdCfgParam.interface.prefix[0].AdvPreferredLifetime=strtoul(val[20],NULL,10);
		radvdCfgParam.interface.prefix[0].AdvRouterAddr=atoi(val[21]);
		strcpy(radvdCfgParam.interface.prefix[0].if6to4,val[22]);
		radvdCfgParam.interface.prefix[0].enabled=atoi(val[23]);

		/*prefix2*/
		memset(tmp,0,5);
		pstart=val[24];
		pend=pstart;
		for(i=0;i<8;i++)
		{
			size=0;
			while(*pend !=':')
			{
				pend++;
				size++;
			}
			pend++;
			if(*pend == ':')
				pend++;
			memcpy(tmp,pstart,size);
			pstart=pend;
			radvdCfgParam.interface.prefix[1].Prefix[i]=strtol(tmp,NULL,16);
		}
		radvdCfgParam.interface.prefix[1].PrefixLen=atoi(val[25]);
		radvdCfgParam.interface.prefix[1].AdvOnLinkFlag=atoi(val[26]);
		radvdCfgParam.interface.prefix[1].AdvAutonomousFlag=atoi(val[26]);
		/*replace atoi by strtoul to support max value test of ipv6 phase 2 test*/
		radvdCfgParam.interface.prefix[1].AdvValidLifetime=strtoul(val[28],NULL,10);
		radvdCfgParam.interface.prefix[1].AdvPreferredLifetime=strtoul(val[29],NULL,10);
		radvdCfgParam.interface.prefix[1].AdvRouterAddr=atoi(val[30]);
		strcpy(radvdCfgParam.interface.prefix[1].if6to4,val[31]);
		radvdCfgParam.interface.prefix[1].enabled=atoi(val[32]);
		value=(void *)&radvdCfgParam;
		break;
	case DNSV6_T:
		if(valNum < 2)
			printf("input argumentis not enough");
		dnsv6CfgParam.enabled=atoi(val[0]);
		strcpy(dnsv6CfgParam.routerName,val[1]);
		value=(void *)&dnsv6CfgParam;
		break;
	case DHCPV6S_T:
		if(valNum < 5)
			printf("input argumentis not enough");
		dhcp6sCfgParam.enabled=atoi(val[0]);
		strcpy(dhcp6sCfgParam.DNSaddr6,val[1]);
		strcpy(dhcp6sCfgParam.addr6PoolS,val[2]);
		strcpy(dhcp6sCfgParam.addr6PoolE,val[3]);
		strcpy(dhcp6sCfgParam.interfaceNameds,val[4]);
		value=(void *)&dhcp6sCfgParam;
		break;
	case DHCPV6C_T:
		if(valNum < 5)
			printf("input argumentis not enough");
		dhcp6cCfgParam.enabled=atoi(val[0]);
		strncpy(&dhcp6cCfgParam.ifName,val[1],NAMSIZE);
		dhcp6cCfgParam.dhcp6pd.sla_len=atoi(val[2]);
		dhcp6cCfgParam.dhcp6pd.sla_id=atoi(val[3]);
		strncpy(&dhcp6cCfgParam.dhcp6pd.ifName,val[4],NAMSIZE);
		value=(void *)&dhcp6cCfgParam;
		break;
	case ADDR6_T:
		if(valNum < 19)
			printf("input argumentis not enough");
		addrIPv6CfgParam.enabled=atoi(val[0]);
		addrIPv6CfgParam.prefix_len[0]=atoi(val[1]);
		addrIPv6CfgParam.prefix_len[1]=atoi(val[2]);

		addrIPv6CfgParam.addrIPv6[0][0]=atoi(val[3]);
		addrIPv6CfgParam.addrIPv6[0][1]=atoi(val[4]);
		addrIPv6CfgParam.addrIPv6[0][2]=atoi(val[5]);
		addrIPv6CfgParam.addrIPv6[0][3]=atoi(val[6]);
		addrIPv6CfgParam.addrIPv6[0][4]=atoi(val[7]);
		addrIPv6CfgParam.addrIPv6[0][5]=atoi(val[8]);
		addrIPv6CfgParam.addrIPv6[0][6]=atoi(val[9]);
		addrIPv6CfgParam.addrIPv6[0][7]=atoi(val[10]);

		addrIPv6CfgParam.addrIPv6[1][0]=atoi(val[11]);
		addrIPv6CfgParam.addrIPv6[1][1]=atoi(val[12]);
		addrIPv6CfgParam.addrIPv6[1][2]=atoi(val[13]);
		addrIPv6CfgParam.addrIPv6[1][3]=atoi(val[14]);
		addrIPv6CfgParam.addrIPv6[1][4]=atoi(val[15]);
		addrIPv6CfgParam.addrIPv6[1][5]=atoi(val[16]);
		addrIPv6CfgParam.addrIPv6[1][6]=atoi(val[17]);
		addrIPv6CfgParam.addrIPv6[1][7]=atoi(val[18]);
		value=(void *)&addrIPv6CfgParam;
		break;
		
	case ADDRV6_T:
		if(valNum < 9)
			printf("input argumentis not enough");
		addr6CfgParam.prefix_len=atoi(val[0]);
		addr6CfgParam.addrIPv6[0]=strtoul(val[1],NULL,16);
		addr6CfgParam.addrIPv6[1]=strtoul(val[2],NULL,16);
		addr6CfgParam.addrIPv6[2]=strtoul(val[3],NULL,16);
		addr6CfgParam.addrIPv6[3]=strtoul(val[4],NULL,16);
		addr6CfgParam.addrIPv6[4]=strtoul(val[5],NULL,16);
		addr6CfgParam.addrIPv6[5]=strtoul(val[6],NULL,16);
		addr6CfgParam.addrIPv6[6]=strtoul(val[7],NULL,16);
		addr6CfgParam.addrIPv6[7]=strtoul(val[8],NULL,16);
		value=(void *)&addr6CfgParam;
		break;

	case TUNNEL6_T:
		tunnelCfgParam.enabled=atoi(val[0]);
		value=(void *)&tunnelCfgParam;
		break;
#endif
#endif
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
	if ( !strcmp(val[0], "add")) {
		id = MIB_CERTROOT_ADD;
		strcpy(certRoot.comment, val[1]);
	}
	else if ( !strcmp(val[0], "del")) {
			id = MIB_CERTROOT_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_CERTROOT_TBL_NUM, (void *)&entryNum)) {
				printf("Get cert ca number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&certRoot) = (char)int_val;
			if ( !APMIB_GET(MIB_CERTROOT_TBL, (void *)&certRoot)) {
				printf("Get table entry error!");
				return;
			}			
	}
	else if ( !strcmp(val[0], "delall"))
			id = MIB_CERTROOT_DELALL;
	value = (void *)&certRoot;
	break;
	case CERTUSER_ARRAY_T:
	if ( !strcmp(val[0], "add")) {
		id = MIB_CERTUSER_ADD;
		strcpy(certUser.comment, val[1]);
		strcpy(certUser.pass , val[2]);
	}
	else if ( !strcmp(val[0], "del")) {
			id = MIB_CERTUSER_DEL;
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]);
			if ( !APMIB_GET(MIB_CERTUSER_TBL_NUM, (void *)&entryNum)) {
				printf("Get cert ca number error!");
				return;
			}
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&certUser) = (char)int_val;
			if ( !APMIB_GET(MIB_CERTUSER_TBL, (void *)&certUser)) {
				printf("Get table entry error!");
				return;
			}			
	}
	else if ( !strcmp(val[0], "delall"))
			id = MIB_CERTUSER_DELALL;
	value = (void *)&certUser;
	break;	
#endif
#ifdef WLAN_PROFILE
	case PROFILE_ARRAY_T:
		if ( !strcmp(val[0], "add")) {
			if (!strcmp(name, "PROFILE_TBL1"))							
				id = MIB_PROFILE_ADD1;
			else
				id = MIB_PROFILE_ADD2;				
			strcpy(profile.ssid, val[1]);
			profile.encryption = atoi(val[2]);
			profile.auth = atoi(val[3]);

			if (profile.encryption == 1 || profile.encryption == 2) {
				profile.wep_default_key = atoi(val[4]);
				if (profile.encryption == 1) {
					if(!string_to_hex(val[5], profile.wepKey1, 10))	return -1;
					if(!string_to_hex(val[6], profile.wepKey2, 10))	return -1;
					if(!string_to_hex(val[7], profile.wepKey3, 10))	return -1;
					if(!string_to_hex(val[8], profile.wepKey4, 10))	return -1;
				}
				else {
					if(!string_to_hex(val[5], profile.wepKey1, 26))	return -1;
					if(!string_to_hex(val[6], profile.wepKey2, 26))	return -1;
					if(!string_to_hex(val[7], profile.wepKey3, 26))	return -1;
					if(!string_to_hex(val[8], profile.wepKey4, 26))	return -1;
				}				
			}
			else if (profile.encryption == 3 || profile.encryption == 4) {
				profile.wpa_cipher = atoi(val[4]);
				strcpy(profile.wpaPSK, val[5]); 
			}	
		}
		else if ( !strcmp(val[0], "del")) { 	
			if (!strcmp(name, "PROFILE_TBL1"))										
				id = MIB_PROFILE_DEL1;
			else
				id = MIB_PROFILE_DEL2;				
			if ( valNum < 2 ) {
				printf("input argument is not enough!\n");
				return;
			}
			int_val = atoi(val[1]); 		
			if (!strcmp(name, "PROFILE_TBL1"))				
				APMIB_GET(MIB_PROFILE_NUM1, (void *)&entryNum);
			else
				APMIB_GET(MIB_PROFILE_NUM2, (void *)&entryNum);
				
			if ( int_val > entryNum ) {
				printf("Element number is too large!\n");
				return;
			}
			*((char *)&profile) = (char)int_val;

			if (!strcmp(name, "PROFILE_TBL1"))			
				APMIB_GET(MIB_PROFILE_TBL1, (void *)&profile);
			else
				APMIB_GET(MIB_PROFILE_TBL2, (void *)&profile);
		}
		else if ( !strcmp(val[0], "delall")) {
			if (!strcmp(name, "PROFILE_TBL1"))			
				id = MIB_PROFILE_DELALL1;
			else
				id = MIB_PROFILE_DELALL2;				
		}
		value = (void *)&profile;
		break;
#endif // WLAN_PROFILE

	case BYTE13_T:
		if ( strlen(val[0])!=26 || !string_to_hex(val[0], key, 26)) {
			printf("invalid value!\n");
			return;
		}
		value = (void *)key;
		break;

	case STRING_T:
		if ( strlen(val[0]) > len) {
			printf("string value too long!\n");
			return;
		}
		value = (void *)val[0];
		break;
	default: printf("invalid mib!\n"); return;
	}
	if(value != NULL)
	if ( !APMIB_SET(id, value))
		diag_printf("set MIB failed!\n");
	//else
		//diag_printf("[%s:%d]set MIB succeed\n", __FUNCTION__, __LINE__);

	if (config_area) {
		if (config_area == HW_MIB_AREA || config_area == HW_MIB_WLAN_AREA)
			apmib_update(HW_SETTING);
		else if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA)
			apmib_update(DEFAULT_SETTING);
		else
			apmib_update(CURRENT_SETTING);
	}
}
static void dumpAllHW(void)
{
	int idx=0, num;
	mib_table_entry_T *pTbl=NULL;

	if ( !apmib_init_HW()) {
		printf("Initialize AP MIB failed!\n");
		return;
	}
#ifdef MBSSID
	apmib_set_vwlanidx(0);
#endif
	 config_area=0;

next_tbl:
	if (++config_area > HW_MIB_WLAN_AREA)
	 	return;
	if (config_area == HW_MIB_AREA)
		pTbl = hwmib_table;
	else if (config_area == HW_MIB_WLAN_AREA)
		pTbl = hwmib_wlan_table;
	
	if(NULL == pTbl) return ;
next_wlan:
	while (pTbl[idx].id) {
			num = 1;
		if (num >0) {
			if ( config_area == HW_MIB_AREA ||
				config_area == HW_MIB_WLAN_AREA)
			{
				printf("HW_");
				if (config_area == HW_MIB_WLAN_AREA) {
					printf("WLAN%d_", apmib_get_wlanidx());
				}				
			}
#ifdef MIB_TLV
			if(pTbl[idx].type == TABLE_LIST_T) // ignore table root entry. keith
			{
				idx++;
				continue;
			}
#endif // #ifdef MIB_TLV			
			getMIB(pTbl[idx].name, pTbl[idx].id, pTbl[idx].type, num, 1 , NULL);
		}
		idx++;
	}
	idx = 0;

	if (config_area == HW_MIB_WLAN_AREA ) {
	#if !defined(CONFIG_CUTE_MAHJONG_SELECTABLE)  
		if ((apmib_get_wlanidx()+1)< NUM_WLAN_INTERFACE)
		{ 
			apmib_set_wlanidx(apmib_get_wlanidx()+1);
			goto next_wlan;
		}	
		else
	#endif
			apmib_set_wlanidx(0);		
	}
	
	goto next_tbl;
}



////////////////////////////////////////////////////////////////////////////////
static void dumpAll(void)
{
	int idx=0, num;
	mib_table_entry_T *pTbl=NULL;

	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return;
	}

#ifdef MBSSID
	apmib_set_vwlanidx(0);
#endif
	 config_area=0;

next_tbl:
	if (++config_area > MIB_WLAN_AREA)
	 	return;
	if (config_area == HW_MIB_AREA)
		pTbl = hwmib_table;
	else if (config_area == HW_MIB_WLAN_AREA)
		pTbl = hwmib_wlan_table;
	else if (config_area == DEF_MIB_AREA || config_area == MIB_AREA)
		pTbl = mib_table;
	else if (config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA)
		pTbl = mib_wlan_table;

	if(NULL == pTbl) return ;
next_wlan:
	while (pTbl[idx].id) {
		if ( pTbl[idx].id == MIB_WLAN_MACAC_ADDR)
			APMIB_GET(MIB_WLAN_MACAC_NUM, (void *)&num);		
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
		else if ( pTbl[idx].id == MIB_WLAN_MESH_ACL_ADDR)
			APMIB_GET(MIB_WLAN_MESH_ACL_NUM, (void *)&num);
#endif
#if defined(WLAN_PROFILE)
		else if ( pTbl[idx].id == MIB_PROFILE_TBL1)
			APMIB_GET(MIB_PROFILE_NUM1, (void *)&num);
		else if ( pTbl[idx].id == MIB_PROFILE_TBL2)
			APMIB_GET(MIB_PROFILE_NUM2, (void *)&num);
#endif		

		else if ( pTbl[idx].id == MIB_WLAN_WDS)
			APMIB_GET(MIB_WLAN_WDS_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_WLAN_SCHEDULE_TBL)
			APMIB_GET(MIB_WLAN_SCHEDULE_TBL_NUM, (void *)&num);		
			
#if defined(VLAN_CONFIG_SUPPORTED)				
		else if ( pTbl[idx].id == MIB_VLANCONFIG_TBL){
			APMIB_GET(MIB_VLANCONFIG_TBL_NUM, (void *)&num);
		}
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
		else if ( pTbl[idx].id == MIB_VLAN_TBL){
			APMIB_GET(MIB_VLAN_TBL_NUM, (void *)&num);
		}
		else if ( pTbl[idx].id == MIB_NETIFACE_TBL){
			APMIB_GET(MIB_NETIFACE_TBL_NUM, (void *)&num);
		}
		else if ( pTbl[idx].id == MIB_VLAN_NETIF_BIND_TBL){
			APMIB_GET(MIB_VLAN_NETIF_BIND_TBL_NUM, (void *)&num);
		}
#endif
#ifdef HOME_GATEWAY
#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
				else if ( pTbl[idx].id == MIB_CWMP_WLANCONF_TBL){
					APMIB_GET(MIB_CWMP_WLANCONF_TBL_NUM, (void *)&num);
				}
#endif//#if defined(CONFIG_APP_TR069) && defined(WLAN_SUPPORT)
		else if ( pTbl[idx].id == MIB_PORTFW_TBL)
			APMIB_GET(MIB_PORTFW_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_PORTFILTER_TBL)
			APMIB_GET(MIB_PORTFILTER_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_IPFILTER_TBL)
			APMIB_GET(MIB_IPFILTER_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_MACFILTER_TBL)
			APMIB_GET(MIB_MACFILTER_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_URLFILTER_TBL)
			APMIB_GET(MIB_URLFILTER_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_TRIGGERPORT_TBL)
			APMIB_GET(MIB_TRIGGERPORT_TBL_NUM, (void *)&num);
#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
		else if ( pTbl[idx].id == MIB_QOS_RULE_TBL)
			APMIB_GET(MIB_QOS_RULE_TBL_NUM, (void *)&num);
#endif
#ifdef ROUTE_SUPPORT
		else if ( pTbl[idx].id == MIB_STATICROUTE_TBL)
			APMIB_GET(MIB_STATICROUTE_TBL_NUM, (void *)&num);
#endif //ROUTE
#ifdef VPN_SUPPORT
		else if ( pTbl[idx].id == MIB_IPSECTUNNEL_TBL)
			APMIB_GET(MIB_IPSECTUNNEL_TBL_NUM, (void *)&num);
#endif
#endif // #ifdef HOME_GATEWAY
#ifdef TLS_CLIENT
		else if ( pTbl[idx].id == MIB_CERTROOT_TBL)
			APMIB_GET(MIB_CERTROOT_TBL_NUM, (void *)&num);
		else if ( pTbl[idx].id == MIB_CERTUSER_TBL)
			APMIB_GET(MIB_CERTUSER_TBL_NUM, (void *)&num);			
#endif
		else if ( pTbl[idx].id == MIB_DHCPRSVDIP_TBL)
		{
			APMIB_GET(MIB_DHCPRSVDIP_TBL_NUM, (void *)&num);			
		}
		else
		{
			num = 1;
		}

		if (num >0) {			
#ifdef MIB_TLV
			/*skip printf HW_ or DEF_ prefix when table root entry*/
			if(pTbl[idx].type == TABLE_LIST_T) // ignore table root entry. keith
			{
				idx++;
				continue;
			}
#endif // #ifdef MIB_TLV
			if ( config_area == HW_MIB_AREA ||
				config_area == HW_MIB_WLAN_AREA)
			{
				printf("HW_");
			}
			else if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA)
				printf("DEF_");

			if (config_area == HW_MIB_WLAN_AREA || config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA) {
#ifdef MBSSID
				if ((config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA) && apmib_get_vwlanidx() > 0)
					printf("WLAN%d_VAP%d_", apmib_get_wlanidx(), apmib_get_vwlanidx()-1);
				else
#endif
				printf("WLAN%d_", apmib_get_wlanidx());
			}
			
			getMIB(pTbl[idx].name, pTbl[idx].id,
						pTbl[idx].type, num, 1 , NULL);
		}
		idx++;
	}
	idx = 0;

    //diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	if (config_area == HW_MIB_WLAN_AREA || config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA) {
#ifdef MBSSID
		if (config_area == DEF_MIB_WLAN_AREA || config_area == MIB_WLAN_AREA) {
			
			if ((apmib_get_vwlanidx()+1) <= NUM_VWLAN_INTERFACE) {
			   apmib_set_vwlanidx(apmib_get_vwlanidx()+1);
				goto next_wlan;
			}
			else
				apmib_set_vwlanidx(0);
		}
#endif
		if ((apmib_get_wlanidx()+1) < NUM_WLAN_INTERFACE)
    {
      apmib_set_wlanidx(apmib_get_wlanidx()+1);
			goto next_wlan;
		}	
		else
			apmib_set_wlanidx(0);		
	}
	
   // diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	goto next_tbl;
}

//////////////////////////////////////////////////////////////////////////////////
static void showHelp(void)
{
	printf("Usage: flash cmd\n");
	printf("option:\n");
	printf("cmd:\n");
	printf("      default -- write flash parameters to default.\n");
	printf("      get [wlan interface-index] mib-name -- get a specific mib from flash\n");
	printf("          memory.\n");
	printf("      set [wlan interface-index] mib-name mib-value -- set a specific mib into\n");
	printf("          flash memory.\n");
	printf("      all -- dump all flash parameters.\n");
	printf("      gethw hw-mib-name -- get a specific mib from flash\n");
	printf("          memory.\n");
	printf("      sethw hw-mib-name mib-value -- set a specific mib into\n");
	printf("          flash memory.\n");
	printf("      allhw -- dump all hw flash parameters.\n");
	printf("      reset -- reset current setting to default.\n");
#ifdef WLAN_FAST_INIT
	printf("      set_mib -- get mib from flash and set to wlan interface.\n");
#endif
	printf("\n");
}

//////////////////////////////////////////////////////////////////////////////////
static void showAllHWMibName(void)
{
	int idx;
	mib_table_entry_T *pTbl;

	config_area = 0;
	while (config_area++ < 7) {
		idx = 0;
		if (config_area == HW_MIB_AREA || config_area == HW_MIB_WLAN_AREA) {
			if (config_area == HW_MIB_AREA)
				pTbl = hwmib_table;
			else
				pTbl = hwmib_wlan_table;
			while (pTbl[idx].id) {
				printf("HW_%s\n", pTbl[idx].name);
				idx++;
			}
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////
static void showAllMibName(void)
{
	int idx;
	mib_table_entry_T *pTbl;

	config_area = 0;
	while (config_area++ < 7) {
		idx = 0;
		if (config_area == HW_MIB_AREA || config_area == HW_MIB_WLAN_AREA) {
			if (config_area == HW_MIB_AREA)
				pTbl = hwmib_table;
			else
				pTbl = hwmib_wlan_table;
			while (pTbl[idx].id) {
				printf("HW_%s\n", pTbl[idx].name);
				idx++;
			}
		}
		else {
			if (config_area == DEF_MIB_AREA || config_area == MIB_AREA)
				pTbl = mib_table;
			else
				pTbl = mib_wlan_table;

			if (config_area == DEF_MIB_AREA || config_area == DEF_MIB_WLAN_AREA)
				printf("DEF_");

			while (pTbl[idx].id) {
				printf("%s\n", pTbl[idx].name);
				idx++;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
static void showSetACHelp(void)
{
#if 0
	printf("flash set MACAC_ADDR cmd\n");
	printf("cmd:\n");
	printf("      add mac-addr comment -- append a filter mac address.\n");
	printf("      del entry-number -- delete a filter entry.\n");
	printf("      delall -- delete all filter mac address.\n");
#endif	
}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
static void showSetMeshACLHelp(void)
{
#if 0
	printf("flash set MESH_ACL_ADDR cmd\n");
	printf("cmd:\n");
	printf("      add mac-addr comment -- append a filter mac address.\n");
	printf("      del entry-number -- delete a filter entry.\n");
	printf("      delall -- delete all filter mac address.\n");
#endif	
}
#endif

#ifdef VLAN_CONFIG_SUPPORTED
////////////////////////////////////////////////////////////////////////////////////
static void showSetVlanConfigHelp(void)
{
#if 0
	printf("flash set VLAN CONFIG  cmd\n");
	printf("cmd:\n");
	printf("      add enable iface -- update vlan config for specific iface.\n");
	printf("      del entry-number -- delete a vlan config entry.\n");
	printf("      delall -- delete all vlan config entry\n");
#endif		
}
#endif

///////////////////////////////////////////////////////////////////////////////////
static void showSetWdsHelp(void)
{
#if 0
	printf("flash set WDS cmd\n");
	printf("cmd:\n");
	printf("      add mac-addr comment -- append a WDS mac address.\n");
	printf("      del entry-number -- delete a WDS entry.\n");
	printf("      delall -- delete all WDS mac address.\n");
#endif	
}

#ifdef HOME_GATEWAY
///////////////////////////////////////////////////////////////////////////////////
static void showSetPortFwHelp(void)
{
#if 0
	printf("flash set PORTFW_TBL cmd\n");
	printf("cmd:\n");
	printf("      add ip from-port to-port protocol comment -- add a filter.\n");
	printf("      del entry-number -- delete a filter.\n");
	printf("      delall -- delete all filter.\n");
#endif	
}


///////////////////////////////////////////////////////////////////////////////////
static void showSetPortFilterHelp(void)
{
#if 0
	printf("flash set PORTFILTER_TBL cmd\n");
	printf("cmd:\n");
	printf("      add from-port to-port protocol comment -- add a filter.\n");
	printf("      del entry-number -- delete a filter.\n");
	printf("      delall -- delete all filter.\n");
#endif	
}

///////////////////////////////////////////////////////////////////////////////////
static void showSetIpFilterHelp(void)
{
#if 0
	printf("flash set IPFILTER_TBL cmd\n");
	printf("cmd:\n");
	printf("      add ip protocol comment -- add a filter.\n");
	printf("      del entry-number -- delete a filter.\n");
	printf("      delall -- delete all filter.\n");
#endif	
}

///////////////////////////////////////////////////////////////////////////////////
static void showSetMacFilterHelp(void)
{
#if 0
	printf("flash set MACFILTER_TBL cmd\n");
	printf("cmd:\n");
	printf("      add mac-addr comment -- add a filter.\n");
	printf("      del entry-number -- delete a filter.\n");
	printf("      delall -- delete all filter.\n");
#endif	
}
///////////////////////////////////////////////////////////////////////////////////
static void showSetUrlFilterHelp(void)
{
#if 0
	printf("flash set URLFILTER_TBL cmd\n");
	printf("cmd:\n");
	printf("      add url-addr -- add a filter.\n");
	printf("      del entry-number -- delete a filter.\n");
	printf("      delall -- delete all filter.\n");
#endif	
}
///////////////////////////////////////////////////////////////////////////////////
static void showSetTriggerPortHelp(void)
{
#if 0
	printf("flash set TRIGGER_PORT cmd\n");
	printf("cmd:\n");
	printf("   add trigger-from trigger-to trigger-proto incoming-from incoming-to incoming-proto comment -- add a trigger-port.\n");
	printf("   del entry-number -- delete a trigger-port.\n");
	printf("   delall -- delete all trigger-port.\n");
#endif	
}




#ifdef GW_QOS_ENGINE
///////////////////////////////////////////////////////////////////////////////////
//static void showSetQosHelp(void) {}
#endif
///////////////////////////////////////////////////////////////////////////////////
#ifdef ROUTE_SUPPORT
static void showSetStaticRouteHelp(void)
{
	printf("flash set STATICROUTE_TBL cmd\n");
	printf("cmd:\n");
	printf("   add dest_ip netmask gateway  -- add a static route.\n");
	printf("   del entry-number -- delete a static route.\n");
	printf("   delall -- delete all static route.\n");


}
#endif //ROUTE
#endif

#ifdef HOME_GATEWAY
#ifdef VPN_SUPPORT
static void  showSetIpsecTunnelHelp(void)
{
        printf("flash set IPSECTUNNEL_TBL cmd\n");
        printf("cmd:\n");
        printf("   add tunnel_id enable name local_type local_ip local_mask_len remote_type remote_ip remote_mask_len remote_gw keymode connectType espEncr espAuth psKey ike_encr ike_auth ike_keygroup ike_lifetime ipsec_lifetime ipsec_pfs spi encrKey authKey -- add a ipsec manual tunnel.\n");
        printf("   del entry-number -- delete a vpn tunnel.\n");
        printf("   delall -- delete all tunnel.\n");
}
#endif
#endif

#ifdef TLS_CLIENT
static void  showSetCertRootHelp(void)
{
        printf("flash set CERTROOT_TBL cmd\n");
        printf("cmd:\n");
        printf("   add comment.\n");
        printf("   del entry-number -- delete a certca .\n");
        printf("   delall -- delete all certca.\n");
}
static void  showSetCertUserHelp(void)
{
        printf("flash set CERTUSER_TBL cmd\n");
        printf("cmd:\n");
        printf("   add comment password.\n");
        printf("   del entry-number -- delete a certca .\n");
        printf("   delall -- delete all certca.\n");
}
#endif

#ifdef PARSE_TXT_FILE
////////////////////////////////////////////////////////////////////////////////
static int parseTxtConfig(char *filename, APMIB_Tp pConfig)
{
	char line[300], value[300];
	FILE *fp;
	int id;

	fp = fopen(filename, "r");
	if ( fp == NULL )
		return -1;

	acNum = 0;
	
#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_)
	meshAclNum = 0;
#endif

	wdsNum = 0;

#ifdef HOME_GATEWAY
	portFilterNum = ipFilterNum = macFilterNum = portFwNum = staticRouteNum=0;
	urlFilterNum = 0;

#if defined(GW_QOS_ENGINE) || defined(QOS_BY_BANDWIDTH)
	qosRuleNum = 0;
#endif
#endif
#ifdef TLS_CLIENT
	certRootNum =  certUserNum = 0;
#endif

#if defined(VLAN_CONFIG_SUPPORTED)
 vlanConfigNum=0;	
#endif
#ifdef CONFIG_RTL_VLAN_SUPPORT
vlanNum=0;
netIfNum=0;
vlan_netIf_bindNum=0;
#endif
	while ( fgets(line, 100, fp) ) {
		id = getToken(line, value);
		if ( id == 0 )
			continue;
		if ( set_mib(pConfig, id, value) < 0) {
			printf("Parse MIB [%d] error!\n", id );
			fclose(fp);
			return -1;
		}
	}

	fclose(fp);
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static int getToken(char *line, char *value)
{
	char *ptr=line, *p1;
	char token[300]={0};
	int len=0, idx;

	if ( *ptr == ';' )	// comments
		return 0;

	// get token
	while (*ptr && *ptr!=EOL) {
		if ( *ptr == '=' ) {
			memcpy(token, line, len);

			// delete ending space
			for (idx=len-1; idx>=0; idx--) {
				if (token[idx]!= SPACE )
					break;
			}
			token[idx+1] = '\0';
			ptr++;
			break;
		}
		ptr++;
		len++;
	}
	if ( !token[0] )
		return 0;

	// get value
	len=0;
	while (*ptr == SPACE ) ptr++; // delete space

	p1 = ptr;
	while ( *ptr && *ptr!=EOL) {
		ptr++;
		len++;
	}
	memcpy(value, p1, len );
	value[len] = '\0';

	idx = 0;
	while (mib_table[idx].id) {
		if (!strcmp(mib_table[idx].name, token))
			return mib_table[idx].id;
		idx++;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
static int set_mib(APMIB_Tp pMib, int id, void *value)
{
	unsigned char key[100];
	char *p1, *p2;
#ifdef HOME_GATEWAY
	char *p3, *p4, *p5;
#if defined(GW_QOS_ENGINE) || defined(VLAN_CONFIG_SUPPORTED) || defined(QOS_BY_BANDWIDTH)
	char *p6, *p7, *p8, *p9, *p10, *p11, *p12;
#endif


#else

#if defined(VLAN_CONFIG_SUPPORTED)	
	char *p3, *p4, *p5, *p6, *p7, *p8;
#endif

#endif
	struct in_addr inAddr;
	int i;
	MACFILTER_Tp pWlAc;
	WDS_Tp pWds;

#ifdef HOME_GATEWAY
	PORTFW_Tp pPortFw;
	PORTFILTER_Tp pPortFilter;
	IPFILTER_Tp pIpFilter;
	MACFILTER_Tp pMacFilter;
	URLFILTER_Tp pUrlFilter;

#ifdef GW_QOS_ENGINE
	QOS_Tp pQos;    
#endif

#ifdef QOS_BY_BANDWIDTH
	IPQOS_Tp pQos;    
#endif
#endif

#ifdef TLS_CLIENT
	CERTROOT_Tp pCertRoot;
	CERTUSER_Tp pCertUser;
#endif
	DHCPRSVDIP_Tp pDhcpRsvd;
#if defined(VLAN_CONFIG_SUPPORTED)	
	VLAN_CONFIG_Tp pVlanConfig;
	int j;
#endif	
#ifdef CONFIG_RTL_VLAN_SUPPORT
	VLAN_Tp pVlan;
	NETIFACE_Tp pNetif;
	VLAN_NETIF_BIND_Tp pVlanNetifBind;
#endif
	for (i=0; mib_table[i].id; i++) {
		if ( mib_table[i].id == id )
			break;
	}
	if ( mib_table[i].id == 0 )
		return -1;

	switch (mib_table[i].type) {
	case BYTE_T:
		*((unsigned char *)(((long)pMib) + mib_table[i].offset)) = (unsigned char)atoi(value);
		break;

	case WORD_T:
		*((unsigned short *)(((long)pMib) + mib_table[i].offset)) = (unsigned short)atoi(value);
		break;

	case STRING_T:
		if ( strlen(value)+1 > mib_table[i].size )
			return 0;
		strcpy((char *)(((long)pMib) + mib_table[i].offset), (char *)value);
		break;

	case BYTE5_T:
		if (strlen(value)!=10 || !string_to_hex(value, key, 10))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + mib_table[i].offset), key, 5);
		break;

	case BYTE6_T:
		if (strlen(value)!=12 || !string_to_hex(value, key, 12))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + mib_table[i].offset), key, 6);
		break;

	case BYTE13_T:
		if (strlen(value)!=26 || !string_to_hex(value, key, 26))
			return -1;
		memcpy((unsigned char *)(((long)pMib) + mib_table[i].offset), key, 13);
		break;
	
	case DWORD_T:
		*((unsigned long *)(((long)pMib) + mib_table[i].offset)) = (unsigned long)atoi(value);
		break;

	case IA_T:
		if ( !inet_aton(value, &inAddr) )
			return -1;
		memcpy((unsigned char *)(((long)pMib) + mib_table[i].offset), (unsigned char *)&inAddr,  4);
		break;

	// CONFIG_RTK_MESH Note: The statement haven't use maybe, Because mib_table haven't WLAC_ARRAY_T
	case WLAC_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid WLAC in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+mib_table[i].offset+acNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		acNum++;
		break;

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
	case MESH_ACL_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid Mesh Acl in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWlAc = (MACFILTER_Tp)(((long)pMib)+mib_table[i].offset+meshAclNum*sizeof(MACFILTER_T));
		memcpy(pWlAc->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWlAc->comment, p2);
		meshAclNum++;
		break;
#endif

	case WDS_ARRAY_T:
		getVal3((char *)value, &p1, &p2, &p3);
		if (p1 == NULL) {
			printf("Invalid WDS in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pWds = (WDS_Tp)(((long)pMib)+mib_table[i].offset+wdsNum*sizeof(WDS_T));
		memcpy(pWds->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pWds->comment, p2);
		pWds->fixedTxRate = (unsigned int)atoi(p3);	
		wdsNum++;
		break;

#ifdef HOME_GATEWAY
	case MACFILTER_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid MACFILTER in argument!\n");
			break;
		}
		if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
			return -1;

		pMacFilter = (MACFILTER_Tp)(((long)pMib)+mib_table[i].offset+macFilterNum*sizeof(MACFILTER_T));
		memcpy(pMacFilter->macAddr, key, 6);
		if (p2 != NULL )
			strcpy(pMacFilter->comment, p2);
		macFilterNum++;
		break;

	case URLFILTER_ARRAY_T:
		getVal2((char *)value, &p1, &p2);
		if (p1 == NULL) {
			printf("Invalid URLFILTER in argument!\n");
			break;
		}
		//if (strlen(p1)!=12 || !string_to_hex(p1, key, 12))
		//	return -1;

		pUrlFilter = (URLFILTER_Tp)(((long)pMib)+mib_table[i].offset+urlFilterNum*sizeof(URLFILTER_T));
		if(!strncmp(pEntry->urlAddr,"http://",7))				
			memcpy(pUrlFilter->urlAddr+7, key, 20);
		else
			memcpy(pUrlFilter->urlAddr, key, 20);
		
		//if (p2 != NULL )
		//	strcpy(pMacFilter->comment, p2);
		urlFilterNum++;
		break;

	case PORTFW_ARRAY_T:
		getVal5((char *)value, &p1, &p2, &p3, &p4, &p5);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL ) {
			printf("Invalid PORTFW arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;

		pPortFw = (PORTFW_Tp)(((long)pMib)+mib_table[i].offset+portFwNum*sizeof(PORTFW_T));
		memcpy(pPortFw->ipAddr, (unsigned char *)&inAddr, 4);
		pPortFw->fromPort = (unsigned short)atoi(p2);
		pPortFw->toPort = (unsigned short)atoi(p3);
		pPortFw->protoType = (unsigned char)atoi(p4);
		if ( p5 )
			strcpy( pPortFw->comment, p5 );
		portFwNum++;
		break;

	case IPFILTER_ARRAY_T:
//		getVal3((char *)value, &p1, &p2, &p3);
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        getVal6((char *)value, &p1, &p2, &p3,&p4,&p5,&p6);
        if (p1 == NULL || p3 == NULL
#ifdef CONFIG_IPV6
			p5 == NULL || p6 == NULL
#endif
        #else
		getVal5((char *)value, &p1, &p2, &p3,&p4,&p5);
		if (p1 == NULL || p2 == NULL
#ifdef CONFIG_IPV6
			p4 == NULL || p5 == NULL
#endif
        #endif
		) {
			printf("Invalid IPFILTER arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;
		pIpFilter = (IPFILTER_Tp)(((long)pMib)+mib_table[i].offset+ipFilterNum*sizeof(IPFILTER_T));
		memcpy(pIpFilter->ipAddr, (unsigned char *)&inAddr, 4);
        
        #ifdef RTL_IPFILTER_SUPPORT_IP_RANGE
        if (p2)
        {
            if ( !inet_aton(p2, &inAddr) )
    			return -1;
    		memcpy(pIpFilter->ipAddrEnd, (unsigned char *)&inAddr, 4);
        }
        pIpFilter->protoType = (unsigned char)atoi(p3);
        if ( p4 )
            strcpy( pIpFilter->comment, p4 );
#ifdef CONFIG_IPV6
        strcpy(pIpFilter->ip6Addr,p5);
        pIpFilter->ipVer = atoi(p6);
#endif

        #else
		pIpFilter->protoType = (unsigned char)atoi(p2);
		if ( p3 )
			strcpy( pIpFilter->comment, p3 );
#ifdef CONFIG_IPV6
		strcpy(pIpFilter->ip6Addr,p4);
		pIpFilter->ipVer = atoi(p5);
#endif
        #endif
		ipFilterNum++;
		break;

	case PORTFILTER_ARRAY_T:
		getVal4((char *)value, &p1, &p2, &p3, &p4);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid PORTFILTER arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;
		pPortFilter = (PORTFILTER_Tp)(((long)pMib)+mib_table[i].offset+portFilterNum*sizeof(PORTFILTER_T));
		pPortFilter->fromPort = (unsigned short)atoi(p1);
		pPortFilter->toPort = (unsigned short)atoi(p2);
		pPortFilter->protoType = (unsigned char)atoi(p3);
		if ( p4 )
			strcpy( pPortFilter->comment, p4 );
#ifdef CONFIG_IPV6
		if(p5)
			pPortFilter->ipVer = atoi(p5);
#endif
		portFilterNum++;
		break;
#ifdef GW_QOS_ENGINE
	case QOS_ARRAY_T:
		getVal12((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7, &p8, &p9, &p10, &p11, &p12);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL || p7 == NULL ||
		    p8 == NULL || p9 == NULL || p10 == NULL || p11 == NULL || p12 == NULL ) {
			printf("Invalid QoS arguments!\n");
			break;
		}
		pQos = (QOS_Tp)(((long)pMib)+mib_table[i].offset+qosRuleNum*sizeof(QOS_T));
		pQos->enabled = (unsigned char)atoi(p1);
		pQos->priority = (unsigned char)atoi(p2);
		pQos->protocol = (unsigned short)atoi(p3);
		if ( !inet_aton(p4, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p5, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_end, (unsigned char *)&inAddr, 4);
        
		pQos->local_port_start = (unsigned short)atoi(p6);
		pQos->local_port_end = (unsigned short)atoi(p7);
		if ( !inet_aton(p8, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p9, &inAddr) )
			return -1;
		memcpy(pQos->remote_ip_end, (unsigned char *)&inAddr, 4);

		pQos->remote_port_start = (unsigned short)atoi(p10);
		pQos->remote_port_end = (unsigned short)atoi(p11);
        	strcpy( pQos->entry_name, p12 );
		qosRuleNum++;        
		break;
#endif

#ifdef QOS_BY_BANDWIDTH
	case QOS_ARRAY_T:
		getVal12((char *)value, &p1, &p2, &p3, &p4, &p5, &p6, &p7,&p8,&p9,&p10,&p11,&p12);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL 
			|| p6 == NULL || p7 == NULL || p8 == NULL || p9 == NULL || p10 == NULL
			|| p11 == NULL) {
			printf("Invalid QoS arguments!\n");
			break;
		}
		pQos = (IPQOS_Tp)(((long)pMib)+mib_table[i].offset+qosRuleNum*sizeof(IPQOS_T));
		pQos->enabled = (unsigned char)atoi(p1);
        	//strcpy( pQos->mac, p2 );
		if (strlen(p2)!=12 || !string_to_hex(p2, pQos->mac, 12)) 
			return -1;		
		pQos->mode = (unsigned char)atoi(p3);
		if ( !inet_aton(p4, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_start, (unsigned char *)&inAddr, 4);
		if ( !inet_aton(p5, &inAddr) )
			return -1;
		memcpy(pQos->local_ip_end, (unsigned char *)&inAddr, 4);
        
		pQos->bandwidth = (unsigned long)atoi(p6);

        	strcpy( pQos->entry_name, p7 );
		pQos->local_port_start = atoi(p8);
		pQos->local_port_end = atoi(p9);
		pQos->protocol = atoi(p10);
		pQos->weight = atoi(p11);
		qosRuleNum++;        
		break;
#endif

#ifdef ROUTE_SUPPORT
	case STATICROUTE_ARRAY_T:
		getVal5((char *)value, &p1, &p2, &p3, &p4, &p5);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid PORTFILTER arguments!\n");
			break;
		}
		if ( !inet_aton(p1, &inAddr) )
			return -1;
		pStaticRoute = (STATICROUTE_Tp)(((long)pMib)+mib_table[i].offset+staticRouteNum*sizeof(STATICROUTE_T));
		if( !inet_aton(p1, &pStaticRoute->destAddr))
			return -1 ;
		if( !inet_aton(p2, &pStaticRoute->netmask))
			return -1 ;
		if( !inet_aton(p3, &pStaticRoute->gateway))
			return -1 ;
		pStaticRoute->interface=(unsigned char)atoi(p4);
		pStaticRoute->metric=(unsigned char)atoi(p5);
			
		staticRouteNum++;
		break;
#endif // ROUTE_SUPPORT
#endif
#ifdef TLS_CLIENT
	case CERTROOT_ARRAY_T:
		getVal1((char *)value, &p1);
		if (p1 == NULL ) {
			printf("Invalid CERTCA arguments!\n");
			break;
		}
		pCertRoot = (CERTROOT_Tp)(((long)pMib)+mib_table[i].offset+certRootNum*sizeof(CERTROOT_T));
		strcpy( pCertRoot->comment, p1 );
		certRootNum++;
		break;
	case CERTUSER_ARRAY_T:
		getVal2((char *)value,&p1, &p2);
		if (p1 == NULL || p2 = NULL) {
			printf("Invalid CERTPR arguments!\n");
			break;
		}
		pCertUser = (CERTUSER_Tp)(((long)pMib)+mib_table[i].offset+certUserNum*sizeof(CERTUSER_T));
		strcpy( pCertUser->pass, p1 );
		strcpy( pCertUser->comment, p2 );
		certUserNum++;
		break;		
#endif

	case DHCPRSVDIP_ARRY_T:
		getVal3((char *)value, &p1, &p2, &p3);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid DHCPRSVDIP in argument!\n");
			break;
		}	
		if (strlen(p2)!=12 || !string_to_hex(p2, key, 12))
			return -1;
		pDhcpRsvd= (DHCPRSVDIP_Tp)(((long)pMib)+mib_table[i].offset+dhcpRsvdIpNum*sizeof(DHCPRSVDIP_T));
		strcpy(pDhcpRsvd->hostName, p1);		
		memcpy(pDhcpRsvd->macAddr, key, 6);
		if( !inet_aton(p3, &pDhcpRsvd->ipAddr))
			return -1;
		dhcpRsvdIpNum++;
		break;
	case SCHEDULE_ARRAY_T:
	//may not used
		break;
#if defined(VLAN_CONFIG_SUPPORTED)		
	case VLANCONFIG_ARRAY_T:
	getVal6((char *)value, &p1, &p2, &p3, &p4, &p5, &p6);
		if (p1 == NULL || p2 == NULL || p3 == NULL || p4 == NULL || p5 == NULL || p6 == NULL ) {
			printf("Invalid VLAN Config arguments!\n");
			break;
		}	
		if (p2){
			pVlanConfig = (VLAN_CONFIG_Tp)(((long)pMib)+mib_table[i].offset);
			for(j=0;j<vlanConfigNum;j++){
			if(!strcmp((pVlanConfig+(j*sizeof(VLAN_CONFIG_T)))->netIface, p2){
				pVlanConfig =  (VLAN_CONFIG_Tp)(((long)pMib)+mib_table[i].offset+(j*sizeof(VLAN_CONFIG_T)));
				pVlanConfig->enabled = (unsigned char)atoi(p1);
				pVlanConfig->tagged = (unsigned char)atoi(p3);
				pVlanConfig->priority = (unsigned char)atoi(p4);
				pVlanConfig->cfi = (unsigned char)atoi(p5);
				pVlanConfig->vlanId = (unsigned short)atoi(p6);
	        	}
	        }
        	}
	break;
#endif	
#ifdef CONFIG_RTL_VLAN_SUPPORT
	case VLAN_ARRAY_T:
	/*
		getVal3((char *)value, &p1, &p2, &p3);
		if (p1 == NULL || p2 == NULL || p3 == NULL) {
			printf("Invalid vlan in argument!\n");
			break;
		}	
		
		pVlan= (VLAN_Tp)(((long)pMib)+mib_table[i].offset+vlanNum*sizeof(VLAN_T));
		vlanNum++;*/
		break;
	case NETIFACE_ARRAY_T:
		break;
	case VLAN_NETIF_BIND_ARRAY_T:
		break;
#endif
	default:
		return -1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
static void getVal2(char *value, char **p1, char **p2)
{
	value = getVal(value, p1);
	if ( value )
		getVal(value, p2);
	else
		*p2 = NULL;
}

#ifdef HOME_GATEWAY
////////////////////////////////////////////////////////////////////////////////
static void getVal3(char *value, char **p1, char **p2, char **p3)
{
	*p1 = *p2 = *p3 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	getVal(value, p3);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal4(char *value, char **p1, char **p2, char **p3, char **p4)
{
	*p1 = *p2 = *p3 = *p4 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	getVal(value, p4);
}

////////////////////////////////////////////////////////////////////////////////
static void getVal5(char *value, char **p1, char **p2, char **p3, char **p4, char **p5)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	getVal(value, p5);
}

static void getVal10(char *value, char **p1, char **p2, char **p3, char **p4, char **p5,
	char **p6, char **p7, char **p8, char **p9, char **p10)
{
	*p1 = *p2 = *p3 = *p4 = *p5 = NULL;

	value = getVal(value, p1);
	if ( !value )
		return;
	value = getVal(value, p2);
	if ( !value )
		return;
	value = getVal(value, p3);
	if ( !value )
		return;
	value = getVal(value, p4);
	if ( !value )
		return;
	value = getVal(value, p5);
	if ( !value )
		return;
	value = getVal(value, p6);
	if ( !value )
		return;
	value = getVal(value, p7);
	if ( !value )
		return;
	value = getVal(value, p8);
	if ( !value )
		return;
	value = getVal(value, p9);
	if ( !value )
		return;
	value = getVal(value, p10);
	if ( !value )
		return;
}

#endif // HOME_GATEWAY

#endif // PARSE_TXT_FILE

////////////////////////////////////////////////////////////////////////////////
static int getdir(char *fullname, char *path, int loop)
{
	char tmpBuf[100], *p, *p1;

	strcpy(tmpBuf, fullname);
	path[0] = '\0';

	p1 = tmpBuf;
	while (1) {
		if ((p=strchr(p1, '/'))) {
			if (--loop == 0) {
				*p = '\0';
				strcpy(path, tmpBuf);
				return 0;
			}
			p1 = ++p;
		}
		else
			break;
	}
	return -1;
}

////////////////////////////////////////////////////////////////////////////////

int read_flash_webpage(char *prefix, char *webfile)
{
	WEB_HEADER_T header;
	char *buf, tmpFile[100], tmpFile1[100], tmpBuf[100];
	int fh=0, i, j, loop, size;
	FILE_ENTRY_Tp pEntry;
#ifndef CONFIG_WEB_COMP_TWICE
	FILE_ENTRY_Tp entry_tmp;
#endif
	struct stat sbuf;
	char *file;
#ifndef CONFIG_WEB_COMP_TWICE
	char *tmp=NULL;
	int len;
#ifdef CONFIG_RESERVE_WEB_CACHE
	int max_page_size=0;
	char *web_cache=NULL;
#endif
#endif	
	if (webfile[0])
		file = webfile;
	else
		file = NULL;
//	printf("prefix=%s webfile=%s\n", prefix, webfile);
	if ( rtk_flash_read((char *)&header, WEB_PAGE_OFFSET, sizeof(header)) == 0) {
		printf("Read web header failed!\n");
		return -1;
	}

//#ifndef __mips__
	header.len = DWORD_SWAP(header.len);
//#endif

	if (memcmp(header.signature, WEB_HEADER, SIGNATURE_LEN)) {
		printf("Invalid web image! Expect %s\n",WEB_HEADER);
		return -1;
	}

// for debug
//printf("web size=%ld\n", header.len);
	buf = malloc(header.len);
	if (buf == NULL) {
		sprintf(tmpBuf, "Allocate buffer failed %d!\n", header.len);
		printf(tmpBuf);
		return -1;
	}

	if ( rtk_flash_read(buf, WEB_PAGE_OFFSET+sizeof(header), header.len) == 0) {
		printf("Read web image failed!\n");
		free(buf);
		return -1;
	}

	if ( !CHECKSUM_OK((unsigned char *)buf, header.len) ) {
		printf("Web image invalid %d!\n",header.len);
		free(buf);
		return -1;
	}
// for debug
//printf("checksum ok!\n");
#if defined(CONFIG_WEB_COMP_TWICE) || !defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)
	// save to a file
	//strcpy(tmpFile, "flashweb.bz2");
	strcpy(tmpFile, "flashweb.gz");
	fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", tmpFile );
		free(buf);
		return -1;
	}
	if ( write(fh, buf, header.len-1) != header.len -1) {
		printf("write file error %s!\n", tmpFile);
		free(buf);
		close(fh);
		return -1;
	}
	close(fh);
	free(buf);
#endif
	//sync();
#if !defined(CONFIG_WEB_COMP_TWICE) && defined(CONFIG_IRES_WEB_ADVANCED_SUPPORT)
	/*set up in-memory representations of web page header*/
	/*statistic web page number*/
	i= j = 0;

	/*2 is for pad + chksum.*/
	while(i < (header.len-2) ){
		pEntry = (FILE_ENTRY_Tp)&buf[i];
#ifdef _LITTLE_ENDIAN_
		pEntry->offset=DWORD_SWAP(pEntry->offset);
		pEntry->size=DWORD_SWAP(pEntry->size);
		pEntry->size_uncomp=DWORD_SWAP(pEntry->size_uncomp);
#endif
		//diag_printf("name:%s offset:%d size:%d uncomp_size:%d\n",
			//pEntry->name,pEntry->offset,pEntry->size,pEntry->size_uncomp);
		i = i + pEntry->size + sizeof(FILE_ENTRY_T);
		j++;
	}
	
	/*create space to store in-memory representation */
	len=(j)*sizeof(FILE_ENTRY_T);
	tmp = malloc(len);
	if(tmp == NULL){
		printf("malloc memory for file entry table error %d\n",len);
		return -1;
	}
	
	/*init in-memory representation entry*/
	i=j=0;	
	while(j < len){
		pEntry = (FILE_ENTRY_Tp)&buf[i];
		entry_tmp= (FILE_ENTRY_Tp)&tmp[j];
		memcpy(entry_tmp,pEntry,sizeof(FILE_ENTRY_T));
#ifdef CONFIG_RESERVE_WEB_CACHE
		if(max_page_size < entry_tmp->size_uncomp)
			max_page_size=entry_tmp->size_uncomp;
#endif
		i = i + pEntry->size + sizeof(FILE_ENTRY_T);
		j += sizeof(FILE_ENTRY_T);
	}
	
#ifdef CONFIG_RESERVE_WEB_CACHE
	if(max_page_size){
		web_cache=malloc(max_page_size);
		if(web_cache!=NULL){
			set_buf_ptr(web_cache);
			//printf("reserve %d for web\n",max_page_size);
		}
	}
#endif
	/*set in-memory representation space start address and len to global variable*/
	set_web_ptr(tmp);
	set_web_size(len);
	/*free unused memory*/
	free(buf);
#elif defined (CONFIG_IRES_WEB_ADVANCED_SUPPORT)
	// get decompress file length
	size=get_uncompress_length(tmpFile);
	if(size <= 0)	{
		diag_printf("error webpags file\n");
	}
	// malloc buf
	buf=malloc(size);
	if (buf == NULL) {
		sprintf(tmpBuf,"Allocate buffer failed %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		return -1;
	}
	// decompress to buf
	if(file_gz_uncompress_to_buf(tmpFile,buf,size)<0) {
		diag_printf("uncompress to buf failed\n");
		if(buf != NULL)
			free(buf);
		return -1;
	}

	//unlink the gz file
	unlink(tmpFile);

	//record the start address and size of webpage
	//NOTE:keep buf not freed...
	set_web_ptr(buf);
	set_web_size(size);
	//list_webpags(buf,size);
#else
	// decompress file
	sprintf(tmpFile1, "%sXXXXXX", tmpFile);
	//mkstemp(tmpFile1);

	//sprintf(tmpBuf, "bunzip2 -c %s > %s", tmpFile, tmpFile1);
	//system(tmpBuf);
	file_gz_uncompress(tmpFile, tmpFile1);

	unlink(tmpFile);
	//sync();

	if (stat(tmpFile1, &sbuf) != 0) {
		printf("Stat file error %s!\n", tmpFile1);
		return -1;
	}
	if (sbuf.st_size < sizeof(FILE_ENTRY_T) ) {
		sprintf(tmpBuf, "Invalid decompress file size %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		unlink(tmpFile1);
		return -1;
	}
// for debug
//printf("decompress size=%ld\n", sbuf.st_size);

	buf = malloc(sbuf.st_size);
	if (buf == NULL) {
		sprintf(tmpBuf,"Allocate buffer failed %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		return -1;
	}
	if ((fh = open(tmpFile1, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile1);
		free(buf);
		return -1;
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, sbuf.st_size) != sbuf.st_size) {
		printf("Read file error %ld!\n", sbuf.st_size);
		free(buf);
		close(fh);
		return -1;
	}
	close(fh);
	unlink(tmpFile1);
	//sync();
	size = sbuf.st_size;
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];

//#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
//#endif

		strcpy(tmpFile, prefix);
		strcat(tmpFile, "/");
		strcat(tmpFile, pEntry->name);
		//printf("tmpFile=%s\n", tmpFile);

		loop = 0;
		while (1) {
			if (getdir(tmpFile, tmpBuf, ++loop) < 0)
				break;
			if (tmpBuf[0] && stat(tmpBuf, &sbuf) < 0) { // not exist
 				if ( mkdir(tmpBuf, 0) < 0) {
					printf("Create directory %s failed!\n", tmpBuf);
					return -1;
				}
			}
		}

// for debug
//printf("write file %s, size=%ld\n", tmpFile, pEntry->size);

		fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
		if (fh == -1) {
			printf("Create output file error %s!\n", tmpFile );
			free(buf);
			return -1;
		}
// for debug
//if ( (i+sizeof(FILE_ENTRY_T)+pEntry->size) > size ) {
//printf("error in size, %ld !\n", pEntry->size);
//}
		if ( write(fh, &buf[i+sizeof(FILE_ENTRY_T)], pEntry->size) != pEntry->size ) {
			printf("Write file error %s, len=%d!\n", tmpFile, pEntry->size);
			free(buf);
			close(fh);
			return -1;
		}
		close(fh);
		// always set execuatble for script file
//		chmod(tmpFile,  S_IXUSR);

		i += (pEntry->size + sizeof(FILE_ENTRY_T));
	}
	free(buf);
#endif	
	return 0;
}

#ifdef VPN_SUPPORT
static int read_flash_rsa(char *outputFile)
{
	int fh;
	char *rsaBuf;

	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

	fh = open(outputFile,  O_RDWR|O_CREAT);
	if (fh == -1) {
		printf("Create WPA config file error!\n");
		return -1;
	}
	rsaBuf = malloc(sizeof(unsigned char) * MAX_RSA_FILE_LEN);
	apmib_get( MIB_IPSEC_RSA_FILE, (void *)rsaBuf);
	write(fh, rsaBuf, MAX_RSA_FILE_LEN);
	close(fh);
	free(rsaBuf);
	chmod(outputFile,  DEFFILEMODE);
	return 0;
}
#endif
#ifdef TLS_CLIENT
////////////////////////////////////////////////////////////////////////////////
static int read_flash_cert(char *prefix, char *certfile)
{
	CERT_HEADER_T header;
	char *buf, tmpFile[100], tmpFile1[100], tmpBuf[100];
	int fh=0, i, loop, size;
	FILE_ENTRY_Tp pEntry;
	struct stat sbuf;
	char *file;

	if (certfile[0])
		file = certfile;
	else
		file = NULL;

	if (!file) {
		if ( flash_read((char *)&header, CERT_PAGE_OFFSET, sizeof(header)) == 0) {
			printf("Read web header failed!\n");
			return -1;
		}
	}
	else {
		if ((fh = open(file, O_RDONLY)) < 0) {
			printf("Can't open file %s\n", file);
			return -1;
		}
		lseek(fh, 0L, SEEK_SET);
		if (read(fh, &header, sizeof(header)) != sizeof(header)) {
			printf("Read web header failed %s!\n", file);
			close(fh);
			return -1;
		}
	}
//#ifndef __mips__
	header.len = DWORD_SWAP(header.len);
//#endif
	
	if (memcmp(header.signature, CERT_HEADER, SIGNATURE_LEN)) {
		printf("Invalid cert image!\n");
		if (file)
			close(fh);
		return -1;
	}

// for debug
//printf("web size=%ld\n", header.len);
	buf = malloc(header.len);
	if (buf == NULL) {
		sprintf(tmpBuf, "Allocate buffer failed %ld!\n", header.len);
		printf(tmpBuf);
		if (file)
			close(fh);
		return -1;
	}

	if (!file) {
		if ( flash_read(buf, CERT_PAGE_OFFSET+sizeof(header), header.len) == 0) {
			printf("Read web image failed!\n");
			if(buf != NULL)
				free(buf);
			return -1;
		}
	}
	else {
		if (read(fh, buf, header.len) != header.len) {
			printf("Read web image failed!\n");
			close(fh);
			if(buf != NULL)
				free(buf);
			return -1;
		}
		close(fh);
	}

	if ( !CHECKSUM_OK(buf, header.len) ) {
		printf("Web image invalid!\n");
		free(buf);
		if (file)
			close(fh);
		return -1;
	}
// for debug
//printf("checksum ok!\n");

	// save to a file
	strcpy(tmpFile, "/tmp/cert.tmp");
	fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create output file error %s!\n", tmpFile );
		return -1;
	}
	if ( write(fh, buf, header.len-1) != header.len -1) {
		printf("write file error %s!\n", tmpFile);
		if(buf != NULL)
			free(buf);
		close(fh);
		return -1;
	}
	close(fh);
	free(buf);
	sync();

	// decompress file
	sprintf(tmpFile1, "%sXXXXXX", tmpFile);
	mkstemp(tmpFile1);

	//sprintf(tmpBuf, "bunzip2 -c %s > %s", tmpFile, tmpFile1);
	sprintf(tmpBuf, "cat %s  > %s", tmpFile, tmpFile1);
	system(tmpBuf);

	unlink(tmpFile);
	sync();

	if (stat(tmpFile1, &sbuf) != 0) {
		printf("Stat file error %s!\n", tmpFile1);
		return -1;
	}
	if (sbuf.st_size < sizeof(FILE_ENTRY_T) ) {
		sprintf(tmpBuf, "Invalid decompress file size %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		unlink(tmpFile1);
		return -1;
	}
// for debug
//printf("decompress size=%ld\n", sbuf.st_size);

	buf = malloc(sbuf.st_size);
	if (buf == NULL) {
		sprintf(tmpBuf,"Allocate buffer failed %ld!\n", sbuf.st_size);
		printf(tmpBuf);
		return -1;
	}
	if ((fh = open(tmpFile1, O_RDONLY)) < 0) {
		printf("Can't open file %s\n", tmpFile1);
		if(buf != NULL)
			free(buf);
		return -1;
	}
	lseek(fh, 0L, SEEK_SET);
	if ( read(fh, buf, sbuf.st_size) != sbuf.st_size) {
		printf("Read file error %ld!\n", sbuf.st_size);
		close(fh);
		if(buf != NULL)
			free(buf);
		return -1;
	}
	close(fh);
	unlink(tmpFile1);
	sync();
	size = sbuf.st_size;
	for (i=0; i<size; ) {
		pEntry = (FILE_ENTRY_Tp)&buf[i];

//#ifndef __mips__
		pEntry->size = DWORD_SWAP(pEntry->size);
//#endif

		strcpy(tmpFile, prefix);
		strcat(tmpFile, "/");
		strcat(tmpFile, pEntry->name);
		if(!strcmp(pEntry->name , ""))
			break;
		//printf("name = %s\n", pEntry->name);	
		loop = 0;
		while (1) {
			if (getdir(tmpFile, tmpBuf, ++loop) < 0)
				break;
			if (tmpBuf[0] && stat(tmpBuf, &sbuf) < 0) { // not exist
 				if ( mkdir(tmpBuf, S_IREAD|S_IWRITE) < 0) {
					printf("Create directory %s failed!\n", tmpBuf);
					return -1;
				}
			}
		}
// for debug
//printf("write file %s, size=%ld\n", tmpFile, pEntry->size);

		fh = open(tmpFile, O_RDWR|O_CREAT|O_TRUNC);
		if (fh == -1) {
			printf("Create output file error %s!\n", tmpFile );
			return -1;
		}
// for debug
//if ( (i+sizeof(FILE_ENTRY_T)+pEntry->size) > size ) {
//printf("error in size, %ld !\n", pEntry->size);
//}

		if ( write(fh, &buf[i+sizeof(FILE_ENTRY_T)], pEntry->size) != pEntry->size ) {
			printf("Write file error %s, len=%ld!\n", tmpFile, pEntry->size);
			close(fh);
			if(buf != NULL)
				free(buf);
			return -1;
		}
		close(fh);
		// always set execuatble for script file
//		chmod(tmpFile,  S_IXUSR);

		i += (pEntry->size + sizeof(FILE_ENTRY_T));
	}
	
	if(buf != NULL)
		free(buf);
	return 0;
}
#endif
////////////////////////////////////////////////////////////////////////////////
static void __inline__ WRITE_WPA_FILE(int fh, unsigned char *buf)
{
	if ( write(fh, buf, strlen((char *)buf)) != strlen((char *)buf) ) {
		printf("Write WPA config file error!\n");
		close(fh);
		exit(1);
	}
}

////////////////////////////////////////////////////////////////////////////////
static void generateWpaConf(char *outputFile, int isWds)
{
	int fh, intVal, encrypt, enable1x, wep;

	int buf1_len=1024,buf2_len=1024;
	unsigned char * buf1 = malloc(buf1_len);
	if(buf1 == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		return;
	}
	memset(buf1,0,buf1_len);
	
	unsigned char * buf2 = malloc(buf2_len);
	if(buf2 == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		free(buf1);
		return;
	}
	memset(buf2,0,buf2_len);


#if 0
//#ifdef UNIVERSAL_REPEATER	
	int isVxd = 0;
	
	if (strstr(outputFile, "-vxd")) 
		isVxd = 1;	
#endif		
	
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		free(buf1);
		free(buf2);
		return;
	}

	fh = open(outputFile, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("Create WPA config file error!\n");

		free(buf1);
		free(buf2);
		return;
	}
	if (!isWds) {
	apmib_get( MIB_WLAN_ENCRYPT, (void *)&encrypt);

#if 0
//#ifdef UNIVERSAL_REPEATER
	if (isVxd && (encrypt == ENCRYPT_WPA2_MIXED)) {
		apmib_get( MIB_WLAN_MODE, (void *)&intVal);
		if (intVal == AP_MODE || intVal == AP_WDS_MODE) 
			encrypt = ENCRYPT_WPA;		
	}
#endif			
	
	sprintf((char *)buf2, "encryption = %d\n", encrypt);
	WRITE_WPA_FILE(fh, buf2);

#if 0
//#ifdef UNIVERSAL_REPEATER
	if (isVxd) {
		if (strstr(outputFile, "wlan0-vxd"))
			apmib_get( MIB_REPEATER_SSID1, (void *)buf1);		
		else			
			apmib_get( MIB_REPEATER_SSID2, (void *)buf1);	
	}
	else
#endif
	apmib_get( MIB_WLAN_SSID,  (void *)buf1);
	sprintf((char *)buf2, "ssid = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ENABLE_1X, (void *)&enable1x);
	sprintf((char *)buf2, "enable1x = %d\n", enable1x);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal);
	sprintf((char *)buf2, "enableMacAuth = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ENABLE_SUPP_NONWPA, (void *)&intVal);
	if (intVal)
		apmib_get( MIB_WLAN_SUPP_NONWPA, (void *)&intVal);

	sprintf((char *)buf2, "supportNonWpaClient = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WEP, (void *)&wep);
	sprintf((char *)buf2, "wepKey = %d\n", wep);
	WRITE_WPA_FILE(fh, buf2);

	if ( encrypt==1 && enable1x ) {
		if (wep == 1) {
			apmib_get( MIB_WLAN_WEP64_KEY1, (void *)buf1);
			sprintf((char *)buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x\"\n", buf1[0],buf1[1],buf1[2],buf1[3],buf1[4]);
		}
		else {
			apmib_get( MIB_WLAN_WEP128_KEY1, (void *)buf1);
			sprintf((char *)buf2, "wepGroupKey = \"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x\"\n",
				buf1[0],buf1[1],buf1[2],buf1[3],buf1[4],
				buf1[5],buf1[6],buf1[7],buf1[8],buf1[9],
				buf1[10],buf1[11],buf1[12]);
		}
	}
	else
		strcpy((char *)buf2, "wepGroupKey = \"\"\n");
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA_AUTH, (void *)&intVal);
	sprintf((char *)buf2, "authentication = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal);
	sprintf((char *)buf2, "unicastCipher = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal);
	sprintf((char *)buf2, "wpa2UnicastCipher = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA2_PRE_AUTH, (void *)&intVal);
	sprintf((char *)buf2, "enablePreAuth = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_PSK_FORMAT, (void *)&intVal);
	if (intVal==0)
		sprintf((char *)buf2, "usePassphrase = 1\n");
	else
		sprintf((char *)buf2, "usePassphrase = 0\n");
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA_PSK, (void *)buf1);
	sprintf((char *)buf2, "psk = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
	sprintf((char *)buf2, "groupRekeyTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
#ifndef KLD_ENABLED
	apmib_get( MIB_WLAN_RS_PORT, (void *)&intVal);
	sprintf((char *)buf2, "rsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_IP, (void *)buf1);
	sprintf((char *)buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_PASSWORD, (void *)buf1);
	sprintf((char *)buf2, "rsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);
#else
	apmib_get( MIB_WLAN_RS_PORT1, (void *)&intVal);
	sprintf((char *)buf2, "rsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_IP1, (void *)buf1);
	sprintf((char *)buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_PASSWORD1, (void *)buf1);
	sprintf((char *)buf2, "rsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);	
#endif

	apmib_get( MIB_WLAN_RS_MAXRETRY, (void *)&intVal);
	sprintf((char *)buf2, "rsMaxReq = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_INTERVAL_TIME, (void *)&intVal);
	sprintf((char *)buf2, "rsAWhile = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal);
	sprintf((char *)buf2, "accountRsEnabled = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_PORT, (void *)&intVal);
	sprintf((char *)buf2, "accountRsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_IP, (void *)buf1);
	sprintf((char *)buf2, "accountRsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_PASSWORD, (void *)buf1);
	sprintf((char *)buf2, "accountRsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_UPDATE_ENABLED, (void *)&intVal);
	sprintf((char *)buf2, "accountRsUpdateEnabled = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_UPDATE_DELAY, (void *)&intVal);
	sprintf((char *)buf2, "accountRsUpdateTime = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_MAXRETRY, (void *)&intVal);
	sprintf((char *)buf2, "accountRsMaxReq = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_ACCOUNT_RS_INTERVAL_TIME, (void *)&intVal);
	sprintf((char *)buf2, "accountRsAWhile = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);
#if 0
#ifdef KLD_ENABLED	
	apmib_get( MIB_WLAN_REAUTH_TIME, (void *)&intVal);
	sprintf((char *)buf2, "rsReAuthTO = %d\n", intVal*60);
	WRITE_WPA_FILE(fh, buf2);


	apmib_get( MIB_WLAN_RS_PORT1, (void *)&intVal);
	sprintf((char *)buf2, "rsPort = %d\n", intVal);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_IP1, (void *)buf1);
	sprintf((char *)buf2, "rsIP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_PASSWORD1, (void *)buf1);
	sprintf((char *)buf2, "rsPassword = \"%s\"\n", buf1);
	WRITE_WPA_FILE(fh, buf2);

	apmib_get( MIB_WLAN_RS_IP2, (void *)buf1);
	if (memcmp(buf1, "\x0\x0\x0\x0", 4)) {
		sprintf((char *)buf2, "rs2IP = %s\n", inet_ntoa(*((struct in_addr *)buf1)));
		WRITE_WPA_FILE(fh, buf2);
		apmib_get( MIB_WLAN_RS_PORT2, (void *)&intVal);
		sprintf((char *)buf2, "rs2Port = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);
		apmib_get( MIB_WLAN_RS_PASSWORD2, (void *)buf1);
		sprintf((char *)buf2, "rs2Password = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);
		apmib_get( MIB_WLAN_ENABLE_MAC_AUTH2, (void *)&intVal);
		sprintf((char *)buf2, "rs2enableMacAuth = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);
	}
#endif
#endif

	}

#ifdef CONFIG_RTK_MESH
	else if (isWds==7) {
		
		apmib_get( MIB_WLAN_MESH_ENCRYPT, (void *)&encrypt);	
		sprintf((char *)buf2, "encryption = %d\n", encrypt);
		WRITE_WPA_FILE(fh, buf2);

		apmib_get( MIB_WLAN_MESH_ID,  (void *)buf1);
		sprintf((char *)buf2, "ssid = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);
		
		WRITE_WPA_FILE(fh, "enable1x = 0\n");
		WRITE_WPA_FILE(fh, "enableMacAuth = 0\n");
		WRITE_WPA_FILE(fh, "supportNonWpaClient = 0\n");
		WRITE_WPA_FILE(fh, "wepKey = 0\n");
		WRITE_WPA_FILE(fh,  "wepGroupKey = \"\"\n");

		apmib_get( MIB_WLAN_MESH_WPA_AUTH, (void *)&intVal);
		sprintf((char *)buf2, "authentication = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		//sprintf(buf2, "unicastCipher = %d\n", intVal);
		sprintf((char *)buf2, "unicastCipher = 1\n");
		WRITE_WPA_FILE(fh, buf2);
		
		apmib_get( MIB_WLAN_MESH_WPA2_CIPHER_SUITE, (void *)&intVal);	

		sprintf((char *)buf2, "wpa2UnicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "enablePreAuth = 0\n");

		apmib_get( MIB_WLAN_MESH_PSK_FORMAT, (void *)&intVal);
		if (intVal==0)
			sprintf((char *)buf2, "usePassphrase = 1\n");
		else
			sprintf((char *)buf2, "usePassphrase = 0\n");
		WRITE_WPA_FILE(fh, buf2);

		apmib_get( MIB_WLAN_MESH_WPA_PSK, (void *)buf1);
		sprintf((char *)buf2, "psk = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, "groupRekeyTime = 86400\n");
		WRITE_WPA_FILE(fh, "rsPort = 1812\n");
		WRITE_WPA_FILE(fh, "rsIP = 0.0.0.0\n");
		WRITE_WPA_FILE(fh, "rsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "rsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "rsAWhile = 5\n");
		WRITE_WPA_FILE(fh, "accountRsEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsPort = 1813\n");
		WRITE_WPA_FILE(fh, "accountRsIP = 0.0.0.0\n");
		WRITE_WPA_FILE(fh, "accountRsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateEnabled = 0\n");
		WRITE_WPA_FILE(fh, "accountRsUpdateTime = 60\n");
		WRITE_WPA_FILE(fh, "accountRsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, "accountRsAWhile = 5\n");
	}
#endif // CONFIG_RTK_MESH

	else {
		apmib_get( MIB_WLAN_WDS_ENCRYPT, (void *)&encrypt);
		if (encrypt == WDS_ENCRYPT_TKIP)		
			encrypt = ENCRYPT_WPA;
		else if (encrypt == WDS_ENCRYPT_AES)		
			encrypt = ENCRYPT_WPA2;		
		else
			encrypt = 0;
	
		sprintf((char *)buf2, "encryption = %d\n", encrypt);
		WRITE_WPA_FILE(fh, buf2);
		WRITE_WPA_FILE(fh, (unsigned char *)"ssid = \"REALTEK\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"enable1x = 1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"enableMacAuth = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"supportNonWpaClient = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"wepKey = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"wepGroupKey = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"authentication = 2\n");

		if (encrypt == ENCRYPT_WPA)
			intVal = WPA_CIPHER_TKIP;
		else
			intVal = WPA_CIPHER_AES;
			
		sprintf((char *)buf2, "unicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		sprintf((char *)buf2, "wpa2UnicastCipher = %d\n", intVal);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, (unsigned char *)"enablePreAuth = 0\n");

		apmib_get( MIB_WLAN_WDS_PSK_FORMAT, (void *)&intVal);
		if (intVal==0)
			sprintf((char *)buf2, "usePassphrase = 1\n");
		else
			sprintf((char *)buf2, "usePassphrase = 0\n");
		WRITE_WPA_FILE(fh, buf2);

		apmib_get( MIB_WLAN_WDS_PSK, (void *)buf1);
		sprintf((char *)buf2, "psk = \"%s\"\n", buf1);
		WRITE_WPA_FILE(fh, buf2);

		WRITE_WPA_FILE(fh, (unsigned char *)"groupRekeyTime = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsPort = 1812\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"rsAWhile = 10\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsEnabled = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsPort = 1813\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsIP = 192.168.1.1\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsPassword = \"\"\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsUpdateEnabled = 0\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsUpdateTime = 1000\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsMaxReq = 3\n");
		WRITE_WPA_FILE(fh, (unsigned char *)"accountRsAWhile = 1\n");
	}

	free(buf1);
	free(buf2);
	close(fh);
}

////////////////////////////////////////////////////////////////////////////////
#ifdef WLAN_FAST_INIT

#include <sys/socket.h>
#include <sys/ioctl.h>
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
#include <cyg/io/eth/rltk/819x/wrapper/wireless.h>
#include "cyg/io/eth/rltk/819x/wlan/ieee802_mib.h"
#endif

void calc_incr(unsigned char *mac, int idx)
{
	if( (*mac+idx) == 0x0 )
		calc_incr(mac-1,1);
	else
		*mac += idx;
}

int get_root_mac(unsigned char *mac)
{
	int fd;
	struct ifreq ifr;
	unsigned char zero_mac[6]={0}, broadcat_mac[6]={0xff};

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		printf("socket() fail\n");
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, "wlan0");
	if( ioctl(fd, SIOCGIFHWADDR, &ifr) < 0 ) {
		close(fd);
		return -1;
	}

	close(fd);
	if( !memcmp(ifr.ifr_hwaddr.sa_data,zero_mac,6) || !memcmp(ifr.ifr_hwaddr.sa_data,broadcat_mac,6) )
		return -1;

	memcpy(mac,ifr.ifr_hwaddr.sa_data,6);
	return 0;
}
#ifdef WLAN_PROFILE
static void set_profile(int id, struct wifi_mib *pmib)
{
	int i, i1, i2;
	WLAN_PROFILE_T profile;

	if (id == 0) {
		apmib_get(MIB_PROFILE_ENABLED1, (void *)&i1);
		apmib_get(MIB_PROFILE_NUM1, (void *)&i2);
	}
	else {
		apmib_get(MIB_PROFILE_ENABLED2, (void *)&i1);
		apmib_get(MIB_PROFILE_NUM2, (void *)&i2);		
	}

	pmib->ap_profile.enable_profile = ((i1 && i2) ? 1 : 0);
	if (i1 && i2) {
		printf("Init wireless[%d] profile...\r\n", id);
		for (i=0; i<i2; i++) {
			*((char *)&profile) = (char)(i+1);
			if (id == 0)
				apmib_get(MIB_PROFILE_TBL1, (void *)&profile);			
			else
				apmib_get(MIB_PROFILE_TBL2, (void *)&profile);							

			strcpy(pmib->ap_profile.profile[i].ssid, profile.ssid);
			pmib->ap_profile.profile[i].encryption = profile.encryption;
			pmib->ap_profile.profile[i].auth_type = profile.auth;
			pmib->ap_profile.profile[i].wep_default_key = profile.wep_default_key;
			memcpy(pmib->ap_profile.profile[i].wep_key1, profile.wepKey1, 13);
			memcpy(pmib->ap_profile.profile[i].wep_key2, profile.wepKey2, 13);
			memcpy(pmib->ap_profile.profile[i].wep_key3, profile.wepKey3, 13);						
			memcpy(pmib->ap_profile.profile[i].wep_key4, profile.wepKey4, 13);
			pmib->ap_profile.profile[i].wpa_cipher = profile.wpa_cipher;
			strcpy(pmib->ap_profile.profile[i].wpa_psk, profile.wpaPSK);			

//printf("\r\n pmib->ap_profile.profile[%d].ssid=[%s],__[%s-%u]\r\n",i, pmib->ap_profile.profile[i].ssid,__FILE__,__LINE__);			
		}		
	}
	pmib->ap_profile.profile_num = i2;
}
#endif

#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0
static int initWlan(char *ifname)
{
	struct wifi_mib *pmib;
	int i, intVal, intVal2, encrypt, enable1x, wep, mode/*, enable1xVxd*/;
	unsigned char mac[6];
	int buf1_len=1024, buf2_len=1024;
	unsigned char * buf1 = malloc(buf1_len);
	if(buf1 == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		return -1;
	}
	memset(buf1,0,buf1_len);

	unsigned char * buf2 = malloc(buf2_len);
	if(buf2 == NULL)
	{
		printf("malloc error in file:%s;function:%s;line:%d;\n",__FILE__,__FUNCTION__,__LINE__);
		free(buf1);
		return -1;
	}
	memset(buf2,0,buf2_len);
	
	int skfd;
	struct iwreq wrq, wrq_root;
	int wlan_band=0, channel_bound=0, aggregation=0;
	MACFILTER_T *pAcl=NULL;
	struct wdsEntry *wds_Entry=NULL;
	WDS_Tp pwds_EntryUI;
	int vwlan_idx;
#ifdef MBSSID
	int v_previous=0;
#ifdef CONFIG_RTL_819X
	int vap_enable=0, intVal4=0;
#endif
#endif
	vwlan_idx=apmib_get_vwlanidx();
#ifdef CONFIG_RTL_8881A_SELECTIVE
	int band;
#endif	
	//printf("ifname=%s wlan_idx=%d vwlan_idx=%d\n", ifname, wlan_idx, vwlan_idx);
	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (skfd < 0) {
		printf("socket() fail\n");
		free(buf1);
		free(buf2);
		return -1;
	}
	strncpy(wrq.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
		printf("Interface %s open failed!\n", ifname);
		close(skfd);
		
		free(buf1);
		free(buf2);
		return -1;
	}

	if ((pmib = (struct wifi_mib *)malloc(sizeof(struct wifi_mib))) == NULL) {
		printf("MIB buffer allocation failed!\n");
		close(skfd);
		
		free(buf1);
		free(buf2);
		return -1;
	}

	if (!apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		goto InitError;
	}

	// Disable WLAN MAC driver and shutdown interface first
	if (is_interface_up(ifname))
		RunSystemCmd(NULL_FILE, "ifconfig", ifname, "down", NULL_STR);

	if (vwlan_idx == 0) {
		// shutdown all WDS interface
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
		for (i=0; i<RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM; i++) {
			//sprintf(buf1, "ifconfig %s-wds%d down", ifname, i);
			//system(buf1);
			sprintf((char *)buf1, "%s-wds%d", ifname, i);
			if (is_interface_up((char *)buf1))
				RunSystemCmd(NULL_FILE, "ifconfig", buf1, "down", NULL_STR);
		}
#endif
#ifdef CONFIG_RTK_MESH
		if(apmib_get_wlanidx()==0)
		{
			sprintf((char *)buf1, "wlan-msh0");
			if (is_interface_up((char *)buf1))
				RunSystemCmd(NULL_FILE, "ifconfig", buf1, "down", NULL_STR);
		}
#endif

		// kill wlan application daemon
		//sprintf(buf1, "wlanapp.sh kill %s", ifname);
		//system(buf1);
	}
	else { // virtual interface
		sprintf((char *)buf1, "wlan%d", apmib_get_wlanidx());
		strncpy(wrq_root.ifr_name, (char *)buf1, IFNAMSIZ);
		if (ioctl(skfd, SIOCGIWNAME, &wrq_root) < 0) {
			printf("Root Interface %s open failed!\n", buf1);
			goto InitError;
		}
	}

	if (vwlan_idx == 0) {
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
		if (intVal == 0) {
			printf("RF type is NULL!\n");
			goto InitError;
		}
	}
	apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal);

	if (intVal == 1) {
		goto InitError;
	}

	// get mib from driver
	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);

	if (vwlan_idx == 0) {
		if (ioctl(skfd, 0x8B42, &wrq) < 0) {
			printf("Get WLAN MIB failed!\n");
			goto InitError;
		}
	}
	else {
		wrq_root.u.data.pointer = (caddr_t)pmib;
		wrq_root.u.data.length = sizeof(struct wifi_mib);				
		if (ioctl(skfd, 0x8B42, &wrq_root) < 0) {
			printf("Get WLAN MIB failed!\n");
			goto InitError;
		}		
	}

	// check mib version
	if (pmib->mib_version != MIB_VERSION) {
		printf("WLAN MIB version mismatch!\n");
		goto InitError;
	}

	if (vwlan_idx > 0) {	//if not root interface, clone root mib to virtual interface
		wrq.u.data.pointer = (caddr_t)pmib;
		wrq.u.data.length = sizeof(struct wifi_mib);
		if (ioctl(skfd, 0x8B43, &wrq) < 0) {
			printf("Set WLAN MIB failed!\n");
			goto InitError;
		}
		pmib->miscEntry.func_off = 0;	
	}

	// Set parameters to driver
	if (vwlan_idx == 0) {	
		apmib_get(MIB_HW_REG_DOMAIN, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11RegDomain = intVal;
	}

	apmib_get(MIB_WLAN_WLAN_MAC_ADDR, (void *)mac);
	if (!memcmp(mac, "\x00\x00\x00\x00\x00\x00", 6)) {
#ifdef WLAN_MAC_FROM_EFUSE
        if( !get_root_mac(mac) ){
                *(char *)mac |= LOCAL_ADMIN_BIT;
                calc_incr((char *)mac+MACADDRLEN-1,vwlan_idx);
        } else {
        		apmib_get(MIB_HW_WLAN_ADDR, (void *)mac);
			goto InitError;
        }
#else
#ifdef MBSSID
		if (vwlan_idx > 0 && vwlan_idx != NUM_VWLAN_INTERFACE) {
			switch (vwlan_idx)
			{
				case 1:
					apmib_get(MIB_HW_WLAN_ADDR1, (void *)mac);
					break;
				case 2:
					apmib_get(MIB_HW_WLAN_ADDR2, (void *)mac);
					break;
				case 3:
					apmib_get(MIB_HW_WLAN_ADDR3, (void *)mac);
					break;
				case 4:
					apmib_get(MIB_HW_WLAN_ADDR4, (void *)mac);
					break;
				default:
					printf("Fail to get MAC address of VAP%d!\n", vwlan_idx-1);
					goto InitError;
			}
		}
		else
#endif
		apmib_get(MIB_HW_WLAN_ADDR, (void *)mac);
#endif	//WLAN_MAC_FROM_EFUSE
	}

	// ifconfig all wlan interface when not in WISP
	// ifconfig wlan1 later interface when in WISP mode, the wlan0  will be setup in WAN interface
	apmib_get(MIB_OP_MODE, (void *)&intVal);
	apmib_get(MIB_WISP_WAN_ID, (void *)&intVal2);
	sprintf((char *)buf1, "wlan%d", intVal2);

	if(intVal==WISP_MODE 
	   && (ifname[6]=='v'||ifname[6]=='V')
	   && (ifname[7]=='x'||ifname[7]=='X')
	   && (ifname[8]=='d'||ifname[8]=='D')){
		if(ifname[4]=='0'){
			apmib_get(MIB_REPEATER_SSID1, (void *)buf1);
		}else if(ifname[4]=='1'){
			apmib_get(MIB_REPEATER_SSID2, (void *)buf1);
		}
	}else{
		// set AP/client/WDS mode
		apmib_get(MIB_WLAN_SSID, (void *)buf1);
	}

	intVal2 = strlen((char *)buf1);
	pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
	memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
	memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

	if ((pmib->dot11StationConfigEntry.dot11DesiredSSIDLen == 3) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'A') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[0] == 'a')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'N') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[1] == 'n')) &&
		((pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'Y') || (pmib->dot11StationConfigEntry.dot11DesiredSSID[2] == 'y'))) {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = 0;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
	}
	else {
		pmib->dot11StationConfigEntry.dot11SSIDtoScanLen = intVal2;
		memset(pmib->dot11StationConfigEntry.dot11SSIDtoScan, 0, 32);
		memcpy(pmib->dot11StationConfigEntry.dot11SSIDtoScan, buf1, intVal2);
	}

#if 0
	if (
#ifdef WLAN_MAC_FROM_EFUSE
		strcmp(buf1,"wlan0") && 
#endif
		(intVal != 2) ||
#ifdef MBSSID
		vwlan_idx > 0 ||
#endif
		strcmp(ifname, (char *)buf1))
#endif
#if 0
	if(intVal == WISP_MODE) 
	{
		//sprintf(buf2, "ifconfig %s hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
		//system(buf2);
		if((vwlan_idx > 0) && (vwlan_idx < NUM_VWLAN_INTERFACE))
		{
			set_mac_address(ifname, (char *)mac);
			memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), mac, 6);
		}
	}
	else
#endif
	{
		set_mac_address(ifname, (char *)mac);
		memcpy(&(pmib->dot11OperationEntry.hwaddr[0]), mac, 6);
	}

#ifdef BR_SHORTCUT	
	if (intVal == 2
#ifdef MBSSID
		&& vwlan_idx == 0
#endif
	) 
		pmib->dot11OperationEntry.disable_brsc = 1;
#endif
	
	apmib_get(MIB_HW_LED_TYPE, (void *)&intVal);
	pmib->dot11OperationEntry.ledtype = intVal;

#if 0 //defined(CONFIG_CUTE_MAHJONG_SELECTABLE)
	if (vwlan_idx == 0 && get_gpio_2g5g() == 5) {
		strcat(buf1, "-5G");
	}
#endif

	apmib_get(MIB_WLAN_MODE, (void *)&mode);
#ifdef CONFIG_RTL_MULTI_CLONE_SUPPORT
        // when CONFIG_RTL_MULTI_CLONE_SUPPORT enabled let wlan0-va1,wlan0-va2 can setting macclone
        apmib_get(MIB_WLAN_MACCLONE_ENABLED, (void *)&intVal);
        if ((intVal == 1) && (mode == 1)) {
            pmib->ethBrExtInfo.macclone_enable = 1;
        }
        else {
            pmib->ethBrExtInfo.macclone_enable = 0;
        }       
#endif
	if (mode == 1) {
		// client mode
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);
		if (intVal2 == 0)
		{
			pmib->dot11OperationEntry.opmode = 8;
#ifdef WLAN_PROFILE
			set_profile(*((char *)(ifname+4))-'0', pmib);
#endif
		}
		else {
			pmib->dot11OperationEntry.opmode = 32;
			apmib_get(MIB_WLAN_DEFAULT_SSID, (void *)buf1);
			intVal2 = strlen((char *)buf1);
			pmib->dot11StationConfigEntry.dot11DefaultSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DefaultSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DefaultSSID, buf1, intVal2);
		}
	}
	else
		pmib->dot11OperationEntry.opmode = 16;

	if (mode == 2)	// WDS only
		pmib->dot11WdsInfo.wdsPure = 1;
	else
		pmib->dot11WdsInfo.wdsPure = 0;

	if (vwlan_idx == 0) { // root interface	
		// set RF parameters
		apmib_get(MIB_HW_RF_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.dot11RFType = intVal;

#if defined(CONFIG_RTL_8196C) || defined(CONFIG_RTL_8198) || defined(CONFIG_RTL_819XD) || defined(CONFIG_RTL_8196E) || defined(CONFIG_RTL_8197F)
#ifdef CONFIG_WLAN_HAL_8814AE
	apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
        if (intVal == 1)
                pmib->dot11RFEntry.MIMO_TR_mode = 5; // 3T3R
        else if(intVal == 2)
                pmib->dot11RFEntry.MIMO_TR_mode = 3; // 2T2R
        else if(intVal == 3)
                pmib->dot11RFEntry.MIMO_TR_mode = 2; // 2T4R
        else
                pmib->dot11RFEntry.MIMO_TR_mode = 5; // 3T3R
#ifdef CONFIG_RTL_8814_8194_2T2R_SUPPORT
	printf("Force 2T2R for 8814/8194 !!!\n");
	pmib->dot11RFEntry.MIMO_TR_mode = 3; // 2T2R
#endif
#else
	apmib_get(MIB_HW_BOARD_VER, (void *)&intVal);
	if (intVal == 1)
		pmib->dot11RFEntry.MIMO_TR_mode = 3;	// 2T2R
	else if(intVal == 2)
		pmib->dot11RFEntry.MIMO_TR_mode = 4; // 1T1R
	else
		pmib->dot11RFEntry.MIMO_TR_mode = 1;	// 1T2R
#endif
	apmib_get(MIB_HW_TX_POWER_CCK_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_A, buf1, MAX_2G_CHANNEL_NUM_MIB);	
	
	apmib_get(MIB_HW_TX_POWER_CCK_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelCCK_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_HT40_1S_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_HT40_1S_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_HT40_2S, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffHT40_2S, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_HT20, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffHT20, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_OFDM, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiffOFDM, buf1, MAX_2G_CHANNEL_NUM_MIB);
	
#if defined(CONFIG_RTL_92D_SUPPORT)|| defined(CONFIG_RTL_8812_SUPPORT)
	apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT40_2S, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GHT40_2S, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_HT20, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GHT20, buf1, MAX_5G_CHANNEL_NUM_MIB);
	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_OFDM, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff5GOFDM, buf1, MAX_5G_CHANNEL_NUM_MIB);
#endif
	

#if defined(CONFIG_RTL_8812_SUPPORT)
	// 5G
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_A, (unsigned char*) buf1);	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_A, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_A, (unsigned char*)buf1);
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_A, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_A, (unsigned char*)buf1);
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_A, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_A, (unsigned char*)buf1);

	apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_B, (unsigned char*)buf1);	
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_B, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_B, (unsigned char*)buf1);
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_B, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_B, (unsigned char*)buf1);
	apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_B, (void *)buf1);
	assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_B, (unsigned char*)buf1);

	// 2G
	apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_A, buf1, MAX_2G_CHANNEL_NUM_MIB);	
	apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_A, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);

	apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
	apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_B, (void *)buf1);
	memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);
#endif
#if defined(CONFIG_WLAN_HAL_8814AE)
        //3 5G

		apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_A, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_A, (unsigned char*)buf1);

		apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_A, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_A, (unsigned char*)buf1);

		apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_B, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_B, (unsigned char*)buf1);

		apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_B, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_B, (unsigned char*)buf1);


        apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_C, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_C, (unsigned char*) buf1);   
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_C, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_C, (unsigned char*)buf1);
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_C, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_C, (unsigned char*)buf1);
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_C, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_C, (unsigned char*)buf1);
		apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_C, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_C, (unsigned char*)buf1);
		apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_C, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_C, (unsigned char*)buf1);
	
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_20BW1S_OFDM1T_D, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_20BW1S_OFDM1T_D, (unsigned char*)buf1);    
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW2S_20BW2S_D, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW2S_20BW2S_D, (unsigned char*)buf1);
		apmib_get(MIB_HW_TX_POWER_DIFF_5G_40BW3S_20BW3S_D, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_40BW3S_20BW3S_D, (unsigned char*)buf1);
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW1S_160BW1S_D, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW1S_160BW1S_D, (unsigned char*)buf1);
        apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW2S_160BW2S_D, (void *)buf1);
        assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW2S_160BW2S_D, (unsigned char*)buf1);
		apmib_get(MIB_HW_TX_POWER_DIFF_5G_80BW3S_160BW3S_D, (void *)buf1);
		assign_diff_AC(pmib->dot11RFEntry.pwrdiff_5G_80BW3S_160BW3S_D, (unsigned char*)buf1);

	
        //3 2G

		apmib_get(MIB_HW_TX_POWER_HT40_1S_C, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_C, buf1, MAX_2G_CHANNEL_NUM_MIB);
		apmib_get(MIB_HW_TX_POWER_HT40_1S_D, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelHT40_1S_D, buf1, MAX_2G_CHANNEL_NUM_MIB);

		apmib_get(MIB_HW_TX_POWER_CCK_C, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_C, buf1, MAX_2G_CHANNEL_NUM_MIB);	
		apmib_get(MIB_HW_TX_POWER_CCK_D, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevelCCK_D, buf1, MAX_2G_CHANNEL_NUM_MIB);

		apmib_get(MIB_HW_TX_POWER_DIFF_40BW3S_20BW3S_A, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_A, buf1, MAX_2G_CHANNEL_NUM_MIB);
		apmib_get(MIB_HW_TX_POWER_DIFF_40BW3S_20BW3S_B, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_B, buf1, MAX_2G_CHANNEL_NUM_MIB);

        apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_C, (void *)buf1);
        memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_C, buf1, MAX_2G_CHANNEL_NUM_MIB);   
        apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_C, (void *)buf1);
        memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_C, buf1, MAX_2G_CHANNEL_NUM_MIB);
    	apmib_get(MIB_HW_TX_POWER_DIFF_40BW3S_20BW3S_C, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_C, buf1, MAX_2G_CHANNEL_NUM_MIB);

        apmib_get(MIB_HW_TX_POWER_DIFF_20BW1S_OFDM1T_D, (void *)buf1);
        memcpy(pmib->dot11RFEntry.pwrdiff_20BW1S_OFDM1T_D, buf1, MAX_2G_CHANNEL_NUM_MIB);
        apmib_get(MIB_HW_TX_POWER_DIFF_40BW2S_20BW2S_D, (void *)buf1);
        memcpy(pmib->dot11RFEntry.pwrdiff_40BW2S_20BW2S_D, buf1, MAX_2G_CHANNEL_NUM_MIB);
		apmib_get(MIB_HW_TX_POWER_DIFF_40BW3S_20BW3S_D, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrdiff_40BW3S_20BW3S_D, buf1, MAX_2G_CHANNEL_NUM_MIB);

		apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_C, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_C, buf1, MAX_5G_CHANNEL_NUM_MIB);
		//printf("BUF pwrlevel5GHT40_1S_C=%s\n",buf1);
		
		apmib_get(MIB_HW_TX_POWER_5G_HT40_1S_D, (void *)buf1);
		memcpy(pmib->dot11RFEntry.pwrlevel5GHT40_1S_D, buf1, MAX_5G_CHANNEL_NUM_MIB);
		//printf("BUF pwrlevel5GHT40_1S_D=%s\n",buf1);
		
		//printf("pmib->dot11RFEntry.pwrlevel5GHT40_1S_A=%s\n",pmib->dot11RFEntry.pwrlevel5GHT40_1S_A);
		//printf("pmib->dot11RFEntry.pwrlevel5GHT40_1S_B=%s\n",pmib->dot11RFEntry.pwrlevel5GHT40_1S_B);
		//printf("pmib->dot11RFEntry.pwrlevel5GHT40_1S_C=%s\n",pmib->dot11RFEntry.pwrlevel5GHT40_1S_C);
		//printf("pmib->dot11RFEntry.pwrlevel5GHT40_1S_D=%s\n",pmib->dot11RFEntry.pwrlevel5GHT40_1S_D);
#endif
	apmib_get(MIB_HW_11N_TSSI1, (void *)&intVal);
	pmib->dot11RFEntry.tssi1 = intVal;

	apmib_get(MIB_HW_11N_TSSI2, (void *)&intVal);
	pmib->dot11RFEntry.tssi2 = intVal;

#if defined(CONFIG_RTL_8881A_SELECTIVE) && !defined(CONFIG_WLAN_HAL_8814AE)
	apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&band);
	if ((band == PHYBAND_5G)
#ifdef CONFIG_CUTE_MAHJONG_SELECTABLE	
		|| (get_gpio_2g5g() == 5) 
#endif	
	) {
		apmib_get(MIB_HW_11N_THER, (void *)&intVal);
		pmib->dot11RFEntry.ther = intVal;
	} else {
		apmib_get(MIB_HW_11N_THER_2, (void *)&intVal2);
		if (intVal2 == 0) {
			apmib_get(MIB_HW_11N_THER, (void *)&intVal);
			intVal2 = intVal;
		}
		pmib->dot11RFEntry.ther = intVal2;
	}
#else
	apmib_get(MIB_HW_11N_THER, (void *)&intVal);	
	pmib->dot11RFEntry.ther = intVal;
#endif

	apmib_get(MIB_HW_11N_TRSWITCH, (void *)&intVal);
	pmib->dot11RFEntry.trswitch = intVal;	
	
	apmib_get(MIB_HW_11N_TRSWPAPE_C9, (void *)&intVal);
	pmib->dot11RFEntry.trsw_pape_C9 = intVal;

	apmib_get(MIB_HW_11N_TRSWPAPE_CC, (void *)&intVal);
	pmib->dot11RFEntry.trsw_pape_CC = intVal;

#ifdef CONFIG_RTL_8881A_SELECTIVE
	apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&band);
	if ((band == PHYBAND_5G)
#ifdef CONFIG_CUTE_MAHJONG_SELECTABLE	
		|| (get_gpio_2g5g() == 5) 
#endif	
	) {
		apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
		pmib->dot11RFEntry.xcap = intVal;
	} else {		
		apmib_get(MIB_HW_11N_XCAP_2, (void *)&intVal2);
		if (intVal2 == 0) {
			apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
			intVal2 = intVal;
		}
		pmib->dot11RFEntry.xcap = intVal2;
	}
#else
	apmib_get(MIB_HW_11N_XCAP, (void *)&intVal);
	pmib->dot11RFEntry.xcap = intVal;
#endif

	apmib_get(MIB_HW_11N_TARGET_PWR, (void *)&intVal);
	pmib->dot11RFEntry.target_pwr = intVal;

#ifdef CONFIG_RTL_819XD	
	apmib_get(MIB_HW_11N_PA_TYPE, (void *)&intVal);
	pmib->dot11RFEntry.pa_type = intVal;
#endif

	if (pmib->dot11RFEntry.dot11RFType == 10) { // Zebra
		apmib_get(MIB_WLAN_RFPOWER_SCALE, (void *)&intVal);
		if(intVal == 1)
			intVal = 3;
		else if(intVal == 2)
				intVal = 6;
			else if(intVal == 3)
					intVal = 9;
				else if(intVal == 4)
						intVal = 17;
		if (intVal) {
			for (i=0; i<MAX_2G_CHANNEL_NUM_MIB; i++) {
				if(pmib->dot11RFEntry.pwrlevelCCK_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelCCK_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_B[i] = 1;
				}
#ifdef CONFIG_WLAN_HAL_8814AE
				if(pmib->dot11RFEntry.pwrlevelCCK_C[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_C[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_C[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_C[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelCCK_D[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelCCK_D[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelCCK_D[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelCCK_D[i] = 1;
				}
#endif
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_A[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_B[i] = 1;
				}
#ifdef CONFIG_WLAN_HAL_8814AE
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_C[i] = 1;
				}
				if(pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevelHT40_1S_D[i] = 1;
				}
#endif
			}	
			
#if defined(CONFIG_RTL_92D_SUPPORT)			
			for (i=0; i<MAX_5G_CHANNEL_NUM_MIB; i++) {
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_A[i] = 1;					
				}
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_B[i] = 1;
				}
#ifdef CONFIG_WLAN_HAL_8814AE
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_C[i] = 1;					
				}
				if(pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] != 0){ 
					if ((pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] - intVal) >= 1)
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] -= intVal;
					else
						pmib->dot11RFEntry.pwrlevel5GHT40_1S_D[i] = 1;
				}
#endif
			}
#endif //#if defined(CONFIG_RTL_92D_SUPPORT)						
		}	
	}	
#endif
		
		apmib_get(MIB_WLAN_BEACON_INTERVAL, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BeaconPeriod = intVal;

		apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
		pmib->dot11RFEntry.dot11channel = intVal;

		apmib_get(MIB_WLAN_RTS_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11RTSThreshold = intVal;

		apmib_get(MIB_WLAN_FRAG_THRESHOLD, (void *)&intVal);
		pmib->dot11OperationEntry.dot11FragmentationThreshold = intVal;

		apmib_get(MIB_WLAN_INACTIVITY_TIME, (void *)&intVal);
		pmib->dot11OperationEntry.expiretime = intVal;

		apmib_get(MIB_WLAN_PREAMBLE_TYPE, (void *)&intVal);
		pmib->dot11RFEntry.shortpreamble = intVal;

		apmib_get(MIB_WLAN_DTIM_PERIOD, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11DTIMPeriod = intVal;

		/*STBC and Coexist*/
		apmib_get(MIB_WLAN_STBC_ENABLED,(void *)&intVal);
		pmib->dot11nConfigEntry.dot11nSTBC = intVal;

		apmib_get(MIB_WLAN_LDPC_ENABLED,(void *)&intVal);
		pmib->dot11nConfigEntry.dot11nLDPC = intVal;
		

		apmib_get(MIB_WLAN_COEXIST_ENABLED,(void *)&intVal);
		pmib->dot11nConfigEntry.dot11nCoexist = intVal;

		apmib_get(MIB_WLAN_ACK_TIMEOUT,(void *)&intVal);
		pmib->miscEntry.ack_timeout = intVal;

		//### add by sen_liu 2011.3.29 TX Beamforming update to mib in 92D
		apmib_get(MIB_WLAN_TX_BEAMFORMING,(void *)&intVal);
		pmib->dot11RFEntry.txbf = intVal;
		//### end priv->pmib->dot11RFEntry.txbf
		
		// enable/disable the notification for IAPP
		apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;

		//max station num
		apmib_get(MIB_WLAN_STA_NUM,(void *)&intVal);
		pmib->dot11StationConfigEntry.supportedStaNum = intVal;
		
		// set 11g protection mode
		apmib_get(MIB_WLAN_PROTECTION_DISABLED, (void *)&intVal);
		pmib->dot11StationConfigEntry.protectionDisabled = intVal;

		// set block relay
		apmib_get(MIB_WLAN_BLOCK_RELAY, (void *)&intVal);
		pmib->dot11OperationEntry.block_relay = intVal;

		// set WiFi specific mode
		apmib_get(MIB_WIFI_SPECIFIC, (void *)&intVal);
		pmib->dot11OperationEntry.wifi_specific = intVal;

		// Set WDS
#ifdef RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS
		apmib_get(MIB_WLAN_WDS_ENABLED, (void *)&intVal);
		apmib_get(MIB_WLAN_WDS_NUM, (void *)&intVal2);
		if (intVal2 > RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM)
			intVal2 = RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS_NUM;
		pmib->dot11WdsInfo.wdsNum = 0;
#ifdef MBSSID 
		if (v_previous > 0) 
			intVal = 0;
#endif
		if (((mode == 2) || (mode == 3)) &&
			(intVal != 0) &&
			(intVal2 != 0)) {
			for (i=0; i<intVal2; i++) {
				buf1[0] = i+1;
				apmib_get(MIB_WLAN_WDS, (void *)buf1);
				pwds_EntryUI = (WDS_Tp)buf1;
				wds_Entry = &(pmib->dot11WdsInfo.entry[i]);
				memcpy(wds_Entry->macAddr, &(pwds_EntryUI->macAddr[0]), 6);
				wds_Entry->txRate = pwds_EntryUI->fixedTxRate;
				pmib->dot11WdsInfo.wdsNum++;
				//sprintf(buf2, "ifconfig %s-wds%d hw ether %02x%02x%02x%02x%02x%02x", ifname, i, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				//system(buf2);
				sprintf((char *)buf2, "%s-wds%d", ifname, i);
				//printf("####%s %02x%02x%02x%02x%02x%02x####\n", buf2, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
				set_mac_address((char *)buf2, (char *)mac);
			}
			pmib->dot11WdsInfo.wdsEnabled = intVal;
		}
		else
			pmib->dot11WdsInfo.wdsEnabled = 0;

		if (((mode == 2) || (mode == 3)) &&
			(intVal != 0)) {
			apmib_get(MIB_WLAN_WDS_ENCRYPT, (void *)&intVal);
			if (intVal == 0)
				pmib->dot11WdsInfo.wdsPrivacy = 0;
			else if (intVal == 1) {
				apmib_get(MIB_WLAN_WDS_WEP_KEY, (void *)buf1);
				pmib->dot11WdsInfo.wdsPrivacy = 1;
				if(!string_to_hex((char *)buf1, &(pmib->dot11WdsInfo.wdsWepKey[0]), 10))
				{
					free(buf1);
					free(buf2);
					return -1;
				}
			}
			else if (intVal == 2) {
				apmib_get(MIB_WLAN_WDS_WEP_KEY, (void *)buf1);
				pmib->dot11WdsInfo.wdsPrivacy = 5;
				if(!string_to_hex((char *)buf1, &(pmib->dot11WdsInfo.wdsWepKey[0]), 26))
				{
					free(buf1);
					free(buf2);
					return -1;
				}
			}
			else if (intVal == 3) {
				pmib->dot11WdsInfo.wdsPrivacy = 2;
				apmib_get(MIB_WLAN_WDS_PSK, (void *)buf1);
				strcpy((char *)pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)buf1);
			}
			else {
				pmib->dot11WdsInfo.wdsPrivacy = 4;
				apmib_get(MIB_WLAN_WDS_PSK, (void *)buf1);
				strcpy((char *)pmib->dot11WdsInfo.wdsPskPassPhrase, (char *)buf1);
			}
		}
#else
		pmib->dot11WdsInfo.wdsEnabled = 0;
		pmib->dot11WdsInfo.wdsNum = 0;
#endif	/* RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WDS */
		// enable/disable the notification for IAPP
		apmib_get(MIB_WLAN_IAPP_DISABLED, (void *)&intVal);
		if (intVal == 0)
			pmib->dot11OperationEntry.iapp_enable = 1;
		else
			pmib->dot11OperationEntry.iapp_enable = 0;

		pmib->dot11StationConfigEntry.dot11AclNum = 0;
		apmib_get(MIB_WLAN_MACAC_ENABLED, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11AclMode = intVal;
		if (intVal != 0) {
			apmib_get(MIB_WLAN_MACAC_NUM, (void *)&intVal);
			if (intVal != 0) {
				for (i=0; i<intVal; i++) {
					buf1[0] = i+1;
					apmib_get(MIB_WLAN_MACAC_ADDR, (void *)buf1);
					pAcl = (MACFILTER_T *)buf1;
					memcpy(&(pmib->dot11StationConfigEntry.dot11AclAddr[i][0]), &(pAcl->macAddr[0]), 6);
					pmib->dot11StationConfigEntry.dot11AclNum++;
				}
			}
		}

#if defined(CONFIG_RTK_MESH) && defined(_MESH_ACL_ENABLE_) // below code copy above ACL code
		// Copy Webpage setting to userspace MIB struct table
		pmib->dot1180211sInfo.mesh_acl_num = 0;
		apmib_get(MIB_WLAN_MESH_ACL_ENABLED, (void *)&intVal);
		pmib->dot1180211sInfo.mesh_acl_mode = intVal;
		if (intVal != 0) {
			apmib_get(MIB_WLAN_MESH_ACL_NUM, (void *)&intVal);
			if (intVal != 0) {
				for (i=0; i<intVal; i++) {
					buf1[0] = i+1;
					apmib_get(MIB_WLAN_MESH_ACL_ADDR, (void *)buf1);
					pAcl = (MACFILTER_T *)buf1;
					memcpy(&(pmib->dot1180211sInfo.mesh_acl_addr[i][0]), &(pAcl->macAddr[0]), 6);
					pmib->dot1180211sInfo.mesh_acl_num++;
				}
			}
		}
		apmib_get(MIB_WLAN_MESH_ENABLE, (void *)&intVal);
		if (((mode == AP_MESH_MODE) || (mode == MESH_MODE)) &&(intVal != 0) && set_mesh_mac_first) 
		{
			set_mesh_mac_first = 0;
			sprintf((char *)buf2, "wlan-msh0");
			set_mac_address((char *)buf2, (char *)mac);
	
		}
#endif

		// set nat2.5 disable when client and mac clone is set
		apmib_get(MIB_WLAN_MACCLONE_ENABLED, (void *)&intVal);
		if ((intVal == 1) && (mode == 1)) {
			//pmib->ethBrExtInfo.nat25_disable = 1;
			pmib->ethBrExtInfo.macclone_enable = 1;
		}
		else {
			//pmib->ethBrExtInfo.nat25_disable = 0;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}		

		// set nat2.5 disable and macclone disable when wireless isp mode
		apmib_get(MIB_OP_MODE, (void *)&intVal);
		if (intVal == 2) {
			pmib->ethBrExtInfo.nat25_disable = 0;
			pmib->ethBrExtInfo.macclone_enable = 0;
		}

#ifdef WIFI_SIMPLE_CONFIG
		pmib->wscEntry.wsc_enable = 0;
#endif
#ifdef HAVE_HS2_SUPPORT
			// enable/disable the notification for HS2
			apmib_get(MIB_WLAN_HS2_ENABLE, (void *)&intVal);
			if (intVal == 0)
				pmib->hs2Entry.hs_enable = 0;
			else
				pmib->hs2Entry.hs_enable = 1;
#endif

	// for 11n
		apmib_get(MIB_WLAN_CHANNEL_BONDING, &channel_bound);
		pmib->dot11nConfigEntry.dot11nUse40M = channel_bound;
		apmib_get(MIB_WLAN_CONTROL_SIDEBAND, &intVal);
		if(channel_bound ==0){
			pmib->dot11nConfigEntry.dot11n2ndChOffset = 0;
		}else {
			if(intVal == 0 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
			if(intVal == 1 )
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;	
#ifdef CONFIG_RTL_8812_SUPPORT
			apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
			if(intVal > 14)
			{
				printf("!!! adjust 5G 2ndoffset for 8812 !!!\n");//eric_pf3
			if(intVal==36 || intVal==44 || intVal==52 || intVal==60
				|| intVal==100 || intVal==108 || intVal==116 || intVal==124
				|| intVal==132 || intVal==140 || intVal==149 || intVal==157
				|| intVal==165 || intVal==173)
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
			else
				pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
			}
			else//2.4G
			{
				apmib_get(MIB_WLAN_BAND, (void *)&wlan_band);
				if(wlan_band == 75)//11ac
				{
					printf("!!! adjust 2.4G AC mode 2ndoffset for 8812 !!!\n");//ac2g
					if(intVal==1 || intVal==9)
						pmib->dot11nConfigEntry.dot11n2ndChOffset = 2;
					else
						pmib->dot11nConfigEntry.dot11n2ndChOffset = 1;
				}
			}
#endif
		}
		apmib_get(MIB_WLAN_SHORT_GI, &intVal);
		pmib->dot11nConfigEntry.dot11nShortGIfor20M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor40M = intVal;
		pmib->dot11nConfigEntry.dot11nShortGIfor80M = intVal;
		/*
		apmib_get(MIB_WLAN_11N_STBC, &intVal);
		pmib->dot11nConfigEntry.dot11nSTBC = intVal;
		apmib_get(MIB_WLAN_11N_COEXIST, &intVal);
		pmib->dot11nConfigEntry.dot11nCoexist = intVal;
		*/
		apmib_get(MIB_WLAN_AGGREGATION, &aggregation);
		if(aggregation ==0){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(aggregation ==1){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 0;
		}else if(aggregation ==2){
			pmib->dot11nConfigEntry.dot11nAMPDU = 0;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}
		else if(aggregation ==3){
			pmib->dot11nConfigEntry.dot11nAMPDU = 1;
			pmib->dot11nConfigEntry.dot11nAMSDU = 1;
		}

#if defined(CONFIG_RTL_819X) && defined(MBSSID)
		if(pmib->dot11OperationEntry.opmode & 0x00000010){// AP mode
			for (vwlan_idx = 1; vwlan_idx < RTLPKG_DEVS_ETH_RLTK_819X_WLAN_MBSSID_NUM+1; vwlan_idx++) {
			    apmib_set_vwlanidx(vwlan_idx);
				apmib_get(MIB_WLAN_WLAN_DISABLED, (void *)&intVal4);
				if (intVal4 == 0)
					vap_enable++;
				intVal4=0;
			}
			apmib_set_vwlanidx(0);
			vwlan_idx = 0; 
		}
		if (vap_enable && (mode ==  AP_MODE || mode ==  AP_WDS_MODE))	
			pmib->miscEntry.vap_enable=1;
		else
			pmib->miscEntry.vap_enable=0;
#endif
	}

	if (vwlan_idx != NUM_VWLAN_INTERFACE) { // not repeater interface
		apmib_get(MIB_WLAN_BASIC_RATES, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11BasicRates = intVal;

		apmib_get(MIB_WLAN_SUPPORTED_RATES, (void *)&intVal);
		pmib->dot11StationConfigEntry.dot11SupportedRates = intVal;

		apmib_get(MIB_WLAN_RATE_ADAPTIVE_ENABLED, (void *)&intVal);
		if (intVal == 0) {
			unsigned int uintVal=0;
			pmib->dot11StationConfigEntry.autoRate = 0;
			apmib_get(MIB_WLAN_FIX_RATE, (void *)&uintVal);
			pmib->dot11StationConfigEntry.fixedTxRate = uintVal;
		}
		else
			pmib->dot11StationConfigEntry.autoRate = 1;

		apmib_get(MIB_WLAN_HIDDEN_SSID, (void *)&intVal);
		pmib->dot11OperationEntry.hiddenAP = intVal;

#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
		apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
		pmib->dot11RFEntry.phyBandSelect = intVal;
		apmib_get(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
		pmib->dot11RFEntry.macPhyMode = intVal;
#endif

	// set band
		apmib_get(MIB_WLAN_BAND, (void *)&intVal);
		wlan_band = intVal;
		if ((mode != 1) && (pmib->dot11OperationEntry.wifi_specific == 1) && (wlan_band == 2))
			wlan_band = 3;

		if (wlan_band == 8) { // pure-11n
#if defined(CONFIG_RTL_92D_SUPPORT) || defined(CONFIG_RTL_8812_SUPPORT)
			if(pmib->dot11RFEntry.phyBandSelect == PHYBAND_5G){
				wlan_band += 4; // a+n
				pmib->dot11StationConfigEntry.legacySTADeny = 4;
			}
			else if (pmib->dot11RFEntry.phyBandSelect == PHYBAND_2G)
#endif
			{
				wlan_band += 3; // b+g+n
			pmib->dot11StationConfigEntry.legacySTADeny = 3;
			}
		}
		else if (wlan_band == 2) { // pure-11g
			wlan_band += 1; // b+g
			pmib->dot11StationConfigEntry.legacySTADeny = 1;
		}
		else if (wlan_band == 10) { // g+n
			wlan_band += 1; // b+g+n
			pmib->dot11StationConfigEntry.legacySTADeny = 1;
		}
		else if (wlan_band == 64) { // pure-11ac
			wlan_band += 12; // a+n
			pmib->dot11StationConfigEntry.legacySTADeny = 12;
		}
		else if (wlan_band == 72) { //ac+n
			wlan_band += 4; //a
			pmib->dot11StationConfigEntry.legacySTADeny = 4;
		}
		else
			pmib->dot11StationConfigEntry.legacySTADeny = 0;	

		pmib->dot11BssType.net_work_type = wlan_band;

#if defined(CONFIG_RTL_92D_SUPPORT)
		//apmib_get(MIB_WLAN_PHY_BAND_SELECT, (void *)&intVal);
	//	pmib->dot11RFEntry.phyBandSelect = intVal;
	//	apmib_get(MIB_WLAN_MAC_PHY_MODE, (void *)&intVal);
	//	pmib->dot11RFEntry.macPhyMode = intVal;
#endif
		
		// set guest access
		apmib_get(MIB_WLAN_ACCESS, (void *)&intVal);
		pmib->dot11OperationEntry.guest_access = intVal;

		// set WMM
		apmib_get(MIB_WLAN_WMM_ENABLED, (void *)&intVal);
		pmib->dot11QosEntry.dot11QosEnable = intVal;		
	}

	apmib_get(MIB_WLAN_AUTH_TYPE, (void *)&intVal);
	apmib_get(MIB_WLAN_ENCRYPT, (void *)&encrypt);
#ifdef CONFIG_RTL_WAPI_SUPPORT
	/*wapi is independed. disable WAPI first if not WAPI*/
	if(7 !=encrypt)
	{
		pmib->wapiInfo.wapiType=0;	
	}
#endif
	if ((intVal == 1) && (encrypt != 1)) {
		// shared-key and not WEP enabled, force to open-system
		intVal = 0;
	}
	pmib->dot1180211AuthEntry.dot11AuthAlgrthm = intVal;

	if (encrypt == 0)
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
	else if (encrypt == 1) {
		// WEP mode
		apmib_get(MIB_WLAN_WEP, (void *)&wep);
		if (wep == 1) {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
			apmib_get(MIB_WLAN_WEP64_KEY1, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY2, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY3, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), buf1, 5);
			apmib_get(MIB_WLAN_WEP64_KEY4, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), buf1, 5);
			apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&intVal);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = intVal;
			
		}
		else {
			pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
			apmib_get(MIB_WLAN_WEP128_KEY1, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[0]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY2, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[1]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY3, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[2]), buf1, 13);
			apmib_get(MIB_WLAN_WEP128_KEY4, (void *)buf1);
			memcpy(&(pmib->dot11DefaultKeysTable.keytype[3]), buf1, 13);
			apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&intVal);
			pmib->dot1180211AuthEntry.dot11PrivacyKeyIndex = intVal;
		}
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT	
	else if(7 == encrypt)
	{
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 7;
		pmib->dot1180211AuthEntry.dot11AuthAlgrthm = 0;
	}
#endif	
	else {
		// WPA mode
		pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
	}

#ifndef CONFIG_RTL8196B_TLD
#ifdef MBSSID
	if (vwlan_idx > 0 && pmib->dot11OperationEntry.guest_access)
		pmib->dot11OperationEntry.block_relay = 1;	
#endif
#endif

	// Set 802.1x flag
	enable1x = 0;
	if (encrypt < 2) {
		apmib_get(MIB_WLAN_ENABLE_1X, (void *)&intVal);
		apmib_get(MIB_WLAN_MAC_AUTH_ENABLED, (void *)&intVal2);
		if ((intVal != 0) || (intVal2 != 0))
			enable1x = 1;
	}
#ifdef CONFIG_RTL_WAPI_SUPPORT
	else if(encrypt == 7)
	{
		/*wapi*/
		enable1x = 0;
	}
#endif	
	else
		enable1x = 1;
	pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1x;
	apmib_get(MIB_WLAN_ACCOUNT_RS_ENABLED, (void *)&intVal);
	pmib->dot118021xAuthEntry.acct_enabled = intVal;

#ifdef CONFIG_RTL_WAPI_SUPPORT
	if(7 == encrypt)
	{
		//apmib_get(MIB_WLAN_WAPI_ASIPADDR,);
		apmib_get(MIB_WLAN_WAPI_AUTH,(void *)&intVal);
		pmib->wapiInfo.wapiType=intVal;

		apmib_get(MIB_WLAN_WAPI_MCAST_PACKETS,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyPktNum=intVal;
		
		apmib_get(MIB_WLAN_WAPI_MCASTREKEY,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyType=intVal;

		apmib_get(MIB_WLAN_WAPI_MCAST_TIME,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateMCastKeyTimeout=intVal;

		apmib_get(MIB_WLAN_WAPI_UCAST_PACKETS,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyPktNum=intVal;
		
		apmib_get(MIB_WLAN_WAPI_UCASTREKEY,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyType=intVal;

		apmib_get(MIB_WLAN_WAPI_UCAST_TIME,(void *)&intVal);
		pmib->wapiInfo.wapiUpdateUCastKeyTimeout=intVal;

		/*1: hex  -else passthru*/
		apmib_get(MIB_WLAN_WAPI_PSK_FORMAT,(void *)&intVal2);
		apmib_get(MIB_WLAN_WAPI_PSKLEN,(void *)&intVal);
		apmib_get(MIB_WLAN_WAPI_PSK,(void *)buf1);
		pmib->wapiInfo.wapiPsk.len=intVal;
		if(1 == intVal2 )
		{
			/*hex*/	
			if(!string_to_hex(buf1, buf2, pmib->wapiInfo.wapiPsk.len*2))
			{
				free(buf1);
				free(buf2);
				return -1;
			}
		}else
		{
			/*passthru*/
			strcpy(buf2,buf1);
		}
		memcpy(pmib->wapiInfo.wapiPsk.octet,buf2,pmib->wapiInfo.wapiPsk.len);
	}
#endif

#ifdef CONFIG_RTK_MESH

#ifdef CONFIG_NEW_MESH_UI
	//new feature:Mesh enable/disable
	//brian add new key:MIB_MESH_ENABLE
	pmib->dot1180211sInfo.meshSilence = 0;

	apmib_get(MIB_WLAN_MESH_ENABLE,(void *)&intVal);
	if (mode == AP_MESH_MODE || mode == MESH_MODE)
	{
		if( intVal )
			pmib->dot1180211sInfo.mesh_enable = 1;
		else
			pmib->dot1180211sInfo.mesh_enable = 0;
	}
	else
		pmib->dot1180211sInfo.mesh_enable = 0;

	// set mesh argument
	// brian change to shutdown portal/root as default
	if (mode == AP_MESH_MODE)
	{
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;
	}
	else if (mode == MESH_MODE)
	{
		if( !intVal )
			//pmib->dot11OperationEntry.opmode += 64; // WIFI_MESH_STATE = 0x00000040
			pmib->dot1180211sInfo.meshSilence = 1;

		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;		
	}
	else
	{
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;	
	}
	#if 0	//by brian, dont enable root by default
	apmib_get(MIB_MESH_ROOT_ENABLE, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_root_enable = intVal;
	#else
	pmib->dot1180211sInfo.mesh_root_enable = 0;
	#endif
#else
	if (mode == AP_MPP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 1;	
	}
	else if (mode == MPP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 1;
	}
	else if (mode == MAP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 1;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;
	}		
	else if (mode == MP_MODE)
	{
		pmib->dot1180211sInfo.mesh_enable = 1;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;		
	}
	else
	{
		pmib->dot1180211sInfo.mesh_enable = 0;
		pmib->dot1180211sInfo.mesh_ap_enable = 0;
		pmib->dot1180211sInfo.mesh_portal_enable = 0;	
	}

	apmib_get(MIB_WLAN_MESH_ROOT_ENABLE, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_root_enable = intVal;
#endif
	apmib_get(MIB_WLAN_MESH_MAX_NEIGHTBOR, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_max_neightbor = intVal;

	apmib_get(MIB_SCRLOG_ENABLED, (void *)&intVal);
	pmib->dot1180211sInfo.log_enabled = intVal;

	apmib_get(MIB_WLAN_MESH_ID, (void *)buf1);
	intVal2 = strlen(buf1);
	memset(pmib->dot1180211sInfo.mesh_id, 0, 32);
	memcpy(pmib->dot1180211sInfo.mesh_id, buf1, intVal2);

	apmib_get(MIB_WLAN_MESH_ENCRYPT, (void *)&intVal);
	apmib_get(MIB_WLAN_MESH_WPA_AUTH, (void *)&intVal2);

	if( intVal2 == 2 && intVal){
		pmib->dot11sKeysTable.dot11Privacy  = 4;
   		apmib_get(MIB_WLAN_MESH_WPA_PSK, (void *)buf1);
		strcpy((char *)pmib->dot1180211sInfo.dot11PassPhrase, (char *)buf1);
   
	}
	else
		pmib->dot11sKeysTable.dot11Privacy  = 0;
	
#ifdef 	_11s_TEST_MODE_	

	apmib_get(MIB_WLAN_MESH_TEST_PARAM1, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved1 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM2, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved2 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM3, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved3 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM4, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved4 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM5, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved5 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM6, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved6 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM7, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved7 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAM8, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved8 = intVal;
	
	apmib_get(MIB_WLAN_MESH_TEST_PARAM9, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserved9 = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAMA, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reserveda = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAMB, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedb = intVal;
	
	apmib_get(MIB_WLAN_MESH_TEST_PARAMC, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedc = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAMD, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedd = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAME, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservede = intVal;

	apmib_get(MIB_WLAN_MESH_TEST_PARAMF, (void *)&intVal);
	pmib->dot1180211sInfo.mesh_reservedf = intVal;
	
	apmib_get(MIB_WLAN_MESH_TEST_PARAMSTR1, (void *)buf1);
	intVal2 = strlen(buf1)<15 ? strlen(buf1) : 15;
	memset(pmib->dot1180211sInfo.mesh_reservedstr1, 0, 16);
	memcpy(pmib->dot1180211sInfo.mesh_reservedstr1, buf1, intVal2);
	
#endif
	
#endif // CONFIG_RTK_MESH

	// When using driver base WPA, set wpa setting to driver
#if 1
	int intVal3;
	apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal3);
//#ifdef CONFIG_RTL_8196B
// button 2009.05.21
#if 1
	if ((intVal3 & WPA_AUTH_PSK) && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
&& encrypt < 7
#endif
)
#else
	if (mode != 1 && (intVal3 & WPA_AUTH_PSK) && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
&& encrypt < 7
#endif
)
#endif
	{
		if (encrypt == 2)
			intVal = 1;
		else if (encrypt == 4)
			intVal = 2;
		else if (encrypt == 6)
			intVal = 3;
		else {
			printf("invalid ENCRYPT value!\n");
			goto InitError;
		}
		pmib->dot1180211AuthEntry.dot11EnablePSK = intVal;

		apmib_get(MIB_WLAN_WPA_PSK, (void *)buf1);
		strcpy((char *)pmib->dot1180211AuthEntry.dot11PassPhrase, (char *)buf1);

		apmib_get(MIB_WLAN_WPA_GROUP_REKEY_TIME, (void *)&intVal);
		pmib->dot1180211AuthEntry.dot11GKRekeyTime = intVal;			
	}
	else		
		pmib->dot1180211AuthEntry.dot11EnablePSK = 0;

#if 1
if (intVal3 != 0 && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
&& encrypt < 7
#endif
)
#else
	if (mode != 1 && intVal3 != 0 && encrypt >= 2 
#ifdef CONFIG_RTL_WAPI_SUPPORT
&& encrypt < 7
#endif
)
#endif
	{
		if (encrypt == 2 || encrypt == 6) {
			apmib_get(MIB_WLAN_WPA_CIPHER_SUITE, (void *)&intVal2);
			if (intVal2 == 1)
				intVal = 2;
			else if (intVal2 == 2)
				intVal = 8;
			else if (intVal2 == 3)
				intVal = 10;
			else {
				printf("invalid WPA_CIPHER_SUITE value!\n");
				goto InitError;
			}
			pmib->dot1180211AuthEntry.dot11WPACipher = intVal;			
		}
		
		if (encrypt == 4 || encrypt == 6) {
			apmib_get(MIB_WLAN_WPA2_CIPHER_SUITE, (void *)&intVal2);
			if (intVal2 == 1)
				intVal = 2;
			else if (intVal2 == 2)
				intVal = 8;
			else if (intVal2 == 3)
				intVal = 10;
			else {
				printf("invalid WPA2_CIPHER_SUITE value!\n");
				goto InitError;
			}
			pmib->dot1180211AuthEntry.dot11WPA2Cipher = intVal;			
		}
	}
#endif

#ifdef CONFIG_RTL_SIMPLE_CONFIG
		apmib_get(MIB_HW_WSC_PIN, (void *)buf1);
		strcpy((char *)pmib->dot11StationConfigEntry.sc_pin, (char *)buf1);
		apmib_get(MIB_SC_DEVICE_TYPE, (void *)&intVal);
		pmib->dot11StationConfigEntry.sc_device_type = intVal;
		apmib_get(MIB_SC_DEVICE_NAME, (void *)buf1);
		strcpy((char *)pmib->dot11StationConfigEntry.sc_device_name, (char *)buf1);
		//apmib_get(MIB_HW_SC_DEFAULT_PIN, (void *)buf1);
		strcpy((char *)pmib->dot11StationConfigEntry.sc_default_pin, sc_default_pin);
		apmib_get(MIB_WLAN_SC_PASSWD, (void *)buf1);
		strcpy((char *)pmib->dot11StationConfigEntry.sc_passwd, (char *)buf1);
		apmib_get(MIB_WLAN_SC_SYNC_PROFILE, (void *)&intVal);
		if(intVal !=0)
			intVal++;
		pmib->dot11StationConfigEntry.sc_sync_vxd_to_root = intVal;
#endif


	wrq.u.data.pointer = (caddr_t)pmib;
	wrq.u.data.length = sizeof(struct wifi_mib);
	if (ioctl(skfd, 0x8B43, &wrq) < 0) {
		printf("Set WLAN MIB failed!\n");
		goto InitError;
	}

#if 0
//#ifdef UNIVERSAL_REPEATER
	// set repeater interface
	if (!strcmp(ifname, "wlan0")) {
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&intVal);
		apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal2);		
		system("ifconfig wlan0-vxd down");
		if (intVal != 0 && mode != WDS_MODE && 
				!(mode==CLIENT_MODE && intVal2==ADHOC)) {
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strncpy(wrq.ifr_name, "wlan0-vxd", IFNAMSIZ);
			if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
				printf("Interface open failed!\n");
				free(pmib);
				return -1;
			}

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B42, &wrq) < 0) {
				printf("Get WLAN MIB failed!\n");
				free(pmib);
				return -1;
			}

			apmib_get(MIB_REPEATER_SSID1, (void *)buf1);
			intVal2 = strlen(buf1);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

			sprintf(buf1, "ifconfig %s-vxd hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			system(buf1);

			enable1xVxd = 0;
			if (encrypt == 0)
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			else if (encrypt == 1) {
				if (enable1x == 0) {
					if (wep == 1)
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
					else
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			else {
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal2);
				if (intVal2 == 2) {
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
					enable1xVxd = 1;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1xVxd;
			
			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B43, &wrq) < 0) {
				printf("Set WLAN MIB failed!\n");
				free(pmib);
				return -1;
			}
			close(skfd);
		}
	}

	if (!strcmp(ifname, "wlan1")) {
		apmib_get(MIB_REPEATER_ENABLED1, (void *)&intVal);
		system("ifconfig wlan1-vxd down");
		if (intVal != 0) {
			skfd = socket(AF_INET, SOCK_DGRAM, 0);
			strncpy(wrq.ifr_name, "wlan1-vxd", IFNAMSIZ);
			if (ioctl(skfd, SIOCGIWNAME, &wrq) < 0) {
				printf("Interface open failed!\n");
				free(pmib);
				return -1;
			}

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B42, &wrq) < 0) {
				printf("Get WLAN MIB failed!\n");
				free(pmib);
				return -1;
			}

			apmib_get(MIB_REPEATER_SSID1, (void *)buf1);
			intVal2 = strlen(buf1);
			pmib->dot11StationConfigEntry.dot11DesiredSSIDLen = intVal2;
			memset(pmib->dot11StationConfigEntry.dot11DesiredSSID, 0, 32);
			memcpy(pmib->dot11StationConfigEntry.dot11DesiredSSID, buf1, intVal2);

			sprintf(buf1, "ifconfig %s-vxd hw ether %02x%02x%02x%02x%02x%02x", ifname, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
			system(buf1);

			enable1xVxd = 0;
			if (encrypt == 0)
				pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			else if (encrypt == 1) {
				if (enable1x == 0) {
					if (wep == 1)
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 1;
					else
						pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 5;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			else {
				apmib_get(MIB_WLAN_WPA_AUTH, (void *)&intVal2);
				if (intVal2 == 2) {
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 2;
					enable1xVxd = 1;
				}
				else
					pmib->dot1180211AuthEntry.dot11PrivacyAlgrthm = 0;
			}
			pmib->dot118021xAuthEntry.dot118021xAlgrthm = enable1xVxd;

			wrq.u.data.pointer = (caddr_t)pmib;
			wrq.u.data.length = sizeof(struct wifi_mib);
			if (ioctl(skfd, 0x8B43, &wrq) < 0) {
				printf("Set WLAN MIB failed!\n");
				free(pmib);
				return -1;
			}
			close(skfd);
		}
	}
#endif
	close(skfd);
	if(pmib != NULL)
		free(pmib);
	free(buf1);
	free(buf2);
	return 0;
InitError:
	if(pmib != NULL)
		free(pmib);
	close(skfd);
	free(buf1);
	free(buf2);
	return -1;
}
#else
static int initWlan(char *ifname)
{
	return 0;
	
}
#endif //#if defined(RTLPKG_DEVS_ETH_RLTK_819X_WLAN_WLAN0)
#endif // WLAN_FAST_INIT

#ifdef HOME_GATEWAY
static int generatePPPConf(int is_pppoe, char *option_file, char *pap_file, char *chap_file)
{
	FILE *fd;
	char tmpbuf[200], buf1[100], buf2[100];
	int srcIdx, dstIdx;

	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

	if (is_pppoe) {
		apmib_get(MIB_PPP_USER_NAME, (void *)buf1);
		apmib_get(MIB_PPP_PASSWORD, (void *)buf2);

	}
	else {
		apmib_get(MIB_PPTP_USER_NAME, (void *)buf1);
		apmib_get(MIB_PPTP_PASSWORD, (void *)buf2);		
	}
	// delete '"' in the value
	for (srcIdx=0, dstIdx=0; buf1[srcIdx]; srcIdx++, dstIdx++) {
		if (buf1[srcIdx] == '"')
			tmpbuf[dstIdx++] = '\\';
		tmpbuf[dstIdx] = buf1[srcIdx];
	}
	if (dstIdx != srcIdx) {
		memcpy(buf1, tmpbuf, dstIdx);
		buf1[dstIdx] ='\0';
	}
	
	for (srcIdx=0, dstIdx=0; buf2[srcIdx]; srcIdx++, dstIdx++) {
		if (buf2[srcIdx] == '"')
			tmpbuf[dstIdx++] = '\\';
		tmpbuf[dstIdx] = buf2[srcIdx];
	}
	if (dstIdx != srcIdx) {
		memcpy(buf2, tmpbuf, dstIdx);
		buf2[dstIdx] ='\0';
	}
	
	fd = fopen(option_file, "w");
	if (fd == NULL) {
		printf("open file %s error!\n", option_file);
		return -1;
	}
	sprintf(tmpbuf, "name \"%s\"\n", buf1);
	fputs(tmpbuf, fd);
	if(strlen(buf2)>31)
	{
	  sprintf(tmpbuf, "-mschap\r\n");
		fputs(tmpbuf, fd);
		sprintf(tmpbuf, "-mschap-v2\r\n");
		fputs(tmpbuf, fd);		
	}
	fclose(fd);
	
	fd = fopen(pap_file, "w");
	if (fd == NULL) {
		printf("open file %s error!\n", pap_file);
		return -1;
	}
	fputs("#################################################\n", fd);
	sprintf(tmpbuf, "\"%s\"	*	\"%s\"\n", buf1, buf2);	
	fputs(tmpbuf, fd);

	fd = fopen(chap_file, "w");
	if (fd == NULL) {
		printf("open file %s error!\n", chap_file);
		return -1;
	}
	fputs("#################################################\n", fd);
	sprintf(tmpbuf, "\"%s\"	*	\"%s\"\n", buf1, buf2);	
	fputs(tmpbuf, fd);
	fclose(fd);
	return 0;
}
#endif // HOME_GATEWAY

#if 0
#ifdef WIFI_SIMPLE_CONFIG
enum { 
	MODE_AP_UNCONFIG=1, 			// AP unconfigured (enrollee)
	MODE_CLIENT_UNCONFIG=2, 		// client unconfigured (enrollee) 
	MODE_CLIENT_CONFIG=3,			// client configured (registrar) 
	MODE_AP_PROXY=4, 			// AP configured (proxy)
	MODE_AP_PROXY_REGISTRAR=5,		// AP configured (proxy and registrar)
	MODE_CLIENT_UNCONFIG_REGISTRAR=6		// client unconfigured (registrar) 
};

#define WRITE_WSC_PARAM(dst, tmp, str, val) {	\
	sprintf(tmp, str, val); \
	memcpy(dst, tmp, strlen(tmp)); \
	dst += strlen(tmp); \
}

static void convert_bin_to_str(unsigned char *bin, int len, char *out)
{
	int i;
	char tmpbuf[10];

	out[0] = '\0';

	for (i=0; i<len; i++) {
		sprintf(tmpbuf, "%02x", bin[i]);
		strcat(out, tmpbuf);
	}
}

static void convert_hex_to_ascii(unsigned long code, char *out)
{
	*out++ = '0' + ((code / 10000000) % 10);  
	*out++ = '0' + ((code / 1000000) % 10);
	*out++ = '0' + ((code / 100000) % 10);
	*out++ = '0' + ((code / 10000) % 10);
	*out++ = '0' + ((code / 1000) % 10);
	*out++ = '0' + ((code / 100) % 10);
	*out++ = '0' + ((code / 10) % 10);
	*out++ = '0' + ((code / 1) % 10);
	*out = '\0';
}

static int compute_pin_checksum(unsigned long int PIN)
{
	unsigned long int accum = 0;
	int digit;
	
	PIN *= 10;
	accum += 3 * ((PIN / 10000000) % 10); 	
	accum += 1 * ((PIN / 1000000) % 10);
	accum += 3 * ((PIN / 100000) % 10);
	accum += 1 * ((PIN / 10000) % 10); 
	accum += 3 * ((PIN / 1000) % 10); 
	accum += 1 * ((PIN / 100) % 10); 
	accum += 3 * ((PIN / 10) % 10);

	digit = (accum % 10);
	return (10 - digit) % 10;
}

#ifdef CONFIG_RTL_COMAPI_CFGFILE
static int defaultWscConf(char *in, char *out)
{
	int fh;
	struct stat status;
	char *buf, *ptr;
	int intVal, intVal2, is_client, is_registrar, len, is_wep=0, wep_key_type=0, wep_transmit_key=0;
	char tmpbuf[100], tmp1[100];
		
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

        apmib_get(MIB_WLAN_MODE, (void *)&is_client);
	if (is_client == CLIENT_MODE)
                return ;

	if (stat(in, &status) < 0) {
		printf("stat() error [%s]!\n", in);
		return -1;
	}

	buf = malloc(status.st_size+2048);
	if (buf == NULL) {
		printf("malloc() error [%d]!\n", (int)status.st_size+2048);
		return -1;		
	}

	ptr = buf;

	WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", MODE_AP_UNCONFIG);
	WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = 1\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = 1\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = 1\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = 1\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = 0\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = \n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = \n", NULL);
    apmib_get(MIB_HW_WSC_PIN, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", tmp1);
	WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = 1\n", NULL);
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \n", NULL);

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		printf("open() error [%s]!\n", in);
		return -1;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", in);
		return -1;	
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1);
		convert_bin_to_str(tmp1, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("open() error [%s]!\n", out);
		return -1;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		printf("Write() file error [%s]!\n", out);
		return -1;
	}
	close(fh);
	free(buf);

	return 0;
}
#endif

static int updateWscConf(char *in, char *out, int genpin)
{
	int fh;
	struct stat status;
	char *buf, *ptr;
	int intVal, intVal2, is_client, is_config, is_registrar, len, is_wep=0, wep_key_type=0, wep_transmit_key=0;
	char tmpbuf[100], tmp1[100];
		
	if ( !apmib_init()) {
		printf("Initialize AP MIB failed!\n");
		return -1;
	}

	apmib_get(MIB_HW_WSC_PIN, (void *)tmpbuf);
	if (genpin || !memcmp(tmpbuf, "\x0\x0\x0\x0\x0\x0\x0\x0", PIN_LEN)) {
		#include <sys/time.h>			
		struct timeval tod;
		unsigned long num;
		
		gettimeofday(&tod , NULL);

		apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1);			
		tod.tv_sec += tmp1[4]+tmp1[5];		
		srand(tod.tv_sec);
		num = rand() % 10000000;
		num = num*10 + compute_pin_checksum(num);
		convert_hex_to_ascii((unsigned long)num, tmpbuf);

		apmib_set(MIB_HW_WSC_PIN, (void *)tmpbuf);
//		apmib_update(CURRENT_SETTING);		
		apmib_update(HW_SETTING);		

		printf("Generated PIN = %s\n", tmpbuf);

		if (genpin)
			return 0;
	}

	if (stat(in, &status) < 0) {
		printf("stat() error [%s]!\n", in);
		return -1;
	}

	buf = malloc(status.st_size+2048);
	if (buf == NULL) {
		printf("malloc() error [%d]!\n", (int)status.st_size+2048);
		return -1;		
	}

	ptr = buf;
	apmib_get(MIB_WLAN_MODE, (void *)&is_client);
	apmib_get(MIB_WLAN_WSC_CONFIGURED, (void *)&is_config);
	apmib_get(MIB_WLAN_WSC_REGISTRAR_ENABLED, (void *)&is_registrar);	
#ifdef CONFIG_RTL8186_KLD_REPEATER
	int is_repeater_enabled;
	int wps_vxdAP_enabled=0;
	apmib_get(MIB_REPEATER_ENABLED1, (void *)&is_repeater_enabled);
#endif
	if (is_client == CLIENT_MODE) {
#ifdef CONFIG_RTL8186_KLD_REPEATER
		if (is_repeater_enabled && is_config) {
			intVal = MODE_AP_PROXY_REGISTRAR;
			wps_vxdAP_enabled = 1;
			WRITE_WSC_PARAM(ptr, tmpbuf, "disable_configured_by_exReg = %d\n", 1);
		}
		else
#endif
		{
			if (is_registrar) {
				if (!is_config)
					intVal = MODE_CLIENT_UNCONFIG_REGISTRAR;
				else
					intVal = MODE_CLIENT_CONFIG;			
			}
			else
				intVal = MODE_CLIENT_UNCONFIG;
		}
	}
	else {
		if (!is_config)
			intVal = MODE_AP_UNCONFIG;
		else
			intVal = MODE_AP_PROXY_REGISTRAR;
	}
	WRITE_WSC_PARAM(ptr, tmpbuf, "mode = %d\n", intVal);

	if (is_client) {
#ifdef CONFIG_RTL8186_KLD_REPEATER
		if (wps_vxdAP_enabled)
			apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&intVal);
		else
#endif
			intVal = 0;
	}
	else
		apmib_get(MIB_WLAN_WSC_UPNP_ENABLED, (void *)&intVal);
#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_upnp_enable = %d\n", intVal);
#else	
        WRITE_WSC_PARAM(ptr, tmpbuf, "upnp = %d\n", intVal);
#endif

	intVal = 0;
	apmib_get(MIB_WLAN_WSC_METHOD, (void *)&intVal);
	//Ethernet(0x2)+Label(0x4)+PushButton(0x80) Bitwise OR
	if (intVal == 1) //Pin+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN);
	else if (intVal == 2) //PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PBC);
	if (intVal == 3) //Pin+PBC+Ethernet
		intVal = (CONFIG_METHOD_ETH | CONFIG_METHOD_PIN | CONFIG_METHOD_PBC);
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_method = %d\n", intVal);

	apmib_get(MIB_WLAN_WSC_AUTH, (void *)&intVal2);
#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_auth = %d\n", intVal2);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "auth_type = %d\n", intVal2);
#endif

	apmib_get(MIB_WLAN_WSC_ENC, (void *)&intVal);
#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_wsc_enc = %d\n", intVal);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "encrypt_type = %d\n", intVal);
#endif
	if (intVal == WSC_ENCRYPT_WEP)
		is_wep = 1;

	if (is_client) {
#ifdef CONFIG_RTL8186_KLD_REPEATER
		if (wps_vxdAP_enabled)
			intVal = 1;
		else
#endif
		{
			apmib_get(MIB_WLAN_NETWORK_TYPE, (void *)&intVal);
			if (intVal == 0)
				intVal = 1;
			else
				intVal = 2;
		}
	}
	else
		intVal = 1;

#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wsc_connection_type = %d\n", intVal);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "connection_type = %d\n", intVal);
#endif

	apmib_get(MIB_WLAN_WSC_MANUAL_ENABLED, (void *)&intVal);

#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wsc_manual_enabled = %d\n", intVal);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "manual_config = %d\n", intVal);
#endif

	if (is_wep) { // only allow WEP in none-MANUAL mode (configured by external registrar)
		apmib_get(MIB_WLAN_ENCRYPT, (void *)&intVal);
		if (intVal != ENCRYPT_WEP) {
			printf("WEP mismatched between WPS and host system\n");
			free(buf);
			return -1;
		}
		apmib_get(MIB_WLAN_WEP, (void *)&intVal);
		if (intVal <= WEP_DISABLED || intVal > WEP128) {
			printf("WEP encrypt length error\n");
			free(buf);
			return -1;
		}
		apmib_get(MIB_WLAN_WEP_KEY_TYPE, (void *)&wep_key_type);
		apmib_get(MIB_WLAN_WEP_DEFAULT_KEY, (void *)&wep_transmit_key);
		wep_transmit_key++;
		WRITE_WSC_PARAM(ptr, tmpbuf, "wep_transmit_key = %d\n", wep_transmit_key);
		if (intVal == WEP64) {
			apmib_get(MIB_WLAN_WEP64_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
#ifdef CONFIG_RTL_COMAPI_CFGFILE
			WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_network_key = %s\n", tmp1);
#else
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
#endif

			apmib_get(MIB_WLAN_WEP64_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			apmib_get(MIB_WLAN_WEP64_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);


			apmib_get(MIB_WLAN_WEP64_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 5);
				tmp1[5] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 5, tmp1);
				tmp1[10] = '\0';
			}			
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
		else {
			apmib_get(MIB_WLAN_WEP128_KEY1, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
#ifdef CONFIG_RTL_COMAPI_CFGFILE
			WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_network_key = %s\n", tmp1);
#else
			WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
#endif

			apmib_get(MIB_WLAN_WEP128_KEY2, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key2 = %s\n", tmp1);

			apmib_get(MIB_WLAN_WEP128_KEY3, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key3 = %s\n", tmp1);

			apmib_get(MIB_WLAN_WEP128_KEY4, (void *)&tmpbuf);
			if (wep_key_type == KEY_ASCII) {
				memcpy(tmp1, tmpbuf, 13);
				tmp1[13] = '\0';
			}
			else {
				convert_bin_to_str(tmpbuf, 13, tmp1);
				tmp1[26] = '\0';
			}
			WRITE_WSC_PARAM(ptr, tmpbuf, "wep_key4 = %s\n", tmp1);
		}
	}
	else {
		apmib_get(MIB_WLAN_WPA_PSK, (void *)&tmp1);		
		
#ifdef CONFIG_RTL_COMAPI_CFGFILE
		WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_network_key = %s\n", tmp1);
#else
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);
#endif
		
	}

#ifdef CONFIG_RTL8186_KLD_REPEATER
	if (wps_vxdAP_enabled)
		apmib_get(MIB_REPEATER_SSID1, (void *)&tmp1);	
	else
#endif
		apmib_get(MIB_WLAN_SSID, (void *)&tmp1);	

#ifdef CONFIG_RTL_COMAPI_CFGFILE
        WRITE_WSC_PARAM(ptr, tmpbuf, "wlan0_ssid = %s\n", tmp1);	
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);	
#endif

#if 0	
//	}
//	else {			
		apmib_get(MIB_WLAN_WSC_PSK, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "network_key = %s\n", tmp1);		
		
		apmib_get(MIB_WLAN_WSC_SSID, (void *)&tmp1);
		WRITE_WSC_PARAM(ptr, tmpbuf, "ssid = %s\n", tmp1);
//	}
#endif

	apmib_get(MIB_HW_WSC_PIN, (void *)&tmp1);
	
#ifdef CONFIG_RTL_COMAPI_CFGFILE
	WRITE_WSC_PARAM(ptr, tmpbuf, "wsc_pin_code = %s\n", tmp1);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "pin_code = %s\n", tmp1);
#endif
	

	apmib_get(MIB_WLAN_CHANNEL, (void *)&intVal);
	if (intVal > 14)
		intVal = 2;
	else
		intVal = 1;
	WRITE_WSC_PARAM(ptr, tmpbuf, "rf_band = %d\n", intVal);

/*
	apmib_get(MIB_HW_MODEL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "model_num = \"%s\"\n", tmp1);	

	apmib_get(MIB_HW_SERIAL_NUM, (void *)&tmp1);	
	WRITE_WSC_PARAM(ptr, tmpbuf, "serial_num = \"%s\"\n", tmp1);	
*/
	apmib_get(MIB_DEVICE_NAME, (void *)&tmp1);	

	apmib_get(MIB_WLAN_WSC_CONFIGBYEXTREG, (void *)&intVal);
	WRITE_WSC_PARAM(ptr, tmpbuf, "config_by_ext_reg = %d\n", intVal);
	
#ifdef CONFIG_RTL_COMAPI_CFGFILE
    WRITE_WSC_PARAM(ptr, tmpbuf, "wsc_device_name = \"%s\"\n", tmp1);
#else
	WRITE_WSC_PARAM(ptr, tmpbuf, "device_name = \"%s\"\n", tmp1);
#endif

	len = (int)(((long)ptr)-((long)buf));
	
	fh = open(in, O_RDONLY);
	if (fh == -1) {
		printf("open() error [%s]!\n", in);
		return -1;
	}

	lseek(fh, 0L, SEEK_SET);
	if (read(fh, ptr, status.st_size) != status.st_size) {		
		printf("read() error [%s]!\n", in);
		return -1;	
	}
	close(fh);

	// search UUID field, replace last 12 char with hw mac address
	ptr = strstr(ptr, "uuid =");
	if (ptr) {
		char tmp2[100];
		apmib_get(MIB_HW_NIC0_ADDR, (void *)&tmp1);	
		convert_bin_to_str(tmp1, 6, tmp2);
		memcpy(ptr+27, tmp2, 12);		
	}

	fh = open(out, O_RDWR|O_CREAT|O_TRUNC);
	if (fh == -1) {
		printf("open() error [%s]!\n", out);
		return -1;
	}

	if (write(fh, buf, len+status.st_size) != len+status.st_size ) {
		printf("Write() file error [%s]!\n", out);
		return -1;
	}
	close(fh);
	free(buf);

	return 0;
}
#endif
#endif

#ifdef COMPRESS_MIB_SETTING

int flash_mib_checksum_ok(int offset)
{	
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen = 0;
	unsigned int real_size = 0;
	int zipRate=0;
	TLV_PARAM_HEADER_T header;
	CONFIG_DATA_T type;
	char *pcomp_sig;
	int status;
	
	switch(offset)
	{
		case HW_SETTING_OFFSET:
			pcomp_sig = COMP_HS_SIGNATURE;
			type = HW_SETTING;
			break;
		case DEFAULT_SETTING_OFFSET:
			type = DEFAULT_SETTING;
			pcomp_sig = COMP_DS_SIGNATURE;
			break;
		case CURRENT_SETTING_OFFSET:
			type = CURRENT_SETTING;
			pcomp_sig = COMP_CS_SIGNATURE;
			break;
		default:
			COMP_TRACE(stderr,"invalid offset!\n");
			return -1;
	}
	
	real_size = mib_get_real_len(type);

	rtk_flash_read((char*)&compHeader, offset, sizeof(COMPRESS_MIB_HEADER_T));
	//diag_printf("[%s:%d]compHeader.signature=%s\n", __FUNCTION__, __LINE__, compHeader.signature);
	//diag_printf("[%s:%d]compHeader.compLen=%x\n", __FUNCTION__, __LINE__,compHeader.compLen);
	//diag_printf("[%s:%d]compHeader.compRate=%x\n", __FUNCTION__, __LINE__,compHeader.compRate);
	//diag_printf("[%s:%d]real_size=%x\n", __FUNCTION__, __LINE__,real_size);
	
	if((strncmp((char *)compHeader.signature, pcomp_sig, COMP_SIGNATURE_LEN) == 0) &&
		(DWORD_SWAP(compHeader.compLen) > 0) && 
		(DWORD_SWAP(compHeader.compLen) <= real_size)
	) //check whether (compress mib data
	{
		zipRate = WORD_SWAP(compHeader.compRate);
		compLen = DWORD_SWAP(compHeader.compLen);

		compPtr=calloc(1,compLen+sizeof(COMPRESS_MIB_HEADER_T));
		if(compPtr==NULL)
		{
			COMP_TRACE(stderr,"\r\n ERR:compPtr malloc fail! __[%s-%u]",__FILE__,__LINE__);
			return -1;
		}
		expPtr=calloc(1,zipRate*compLen);
		if(expPtr==NULL)
		{
			COMP_TRACE(stderr,"\r\n ERR:expPtr malloc fail! __[%s-%u]",__FILE__,__LINE__);			
			free(compPtr);
			return -1;
		}

		rtk_flash_read((char *)compPtr, offset, compLen+sizeof(COMPRESS_MIB_HEADER_T));

		expLen = Decode(compPtr+sizeof(COMPRESS_MIB_HEADER_T), compLen, expPtr);
		//printf("expandLen read len: %d\n", expandLen);

		// copy the mib header from expFile
		memcpy((char *)&header, expPtr, sizeof(header));
		header.len=DWORD_SWAP(header.len);
		pContent = calloc(1,header.len);
		if(pContent==NULL)
		{
			COMP_TRACE(stderr,"\r\n ERR:pContent malloc fail! __[%s-%u]",__FILE__,__LINE__);			
			free(compPtr);
			free(expPtr);
			return -1;
		}
		memcpy(pContent, expPtr+sizeof(TLV_PARAM_HEADER_T), header.len);
		status = CHECKSUM_OK(pContent, header.len);
		if ( !status) {
			free(pContent);
			free(compPtr);
			free(expPtr);
			COMP_TRACE(stderr,"comp Checksum type[%u] error!\n", type);
			return -1;
		}
		if(compPtr != NULL)
			free(compPtr);
		if(expPtr != NULL)
			free(expPtr);
		if(pContent != NULL)
			free(pContent);

		return 1;
	} 
	else
	{
		COMP_TRACE(stderr,"\r\n Invalid comp sig or compress len __[%s-%u]",__FILE__,__LINE__);			
	}
	return -1;
}

int flash_mib_compress_write(CONFIG_DATA_T type, char *data, TLV_PARAM_HEADER_T *pheader, unsigned char *pchecksum)
{
	unsigned char* pContent = NULL;

	COMPRESS_MIB_HEADER_T compHeader;
	unsigned char *expPtr, *compPtr;
	unsigned int expLen = 0;
	unsigned int compLen;
	unsigned int real_size = 0;
	//int zipRate=0;
	char *pcomp_sig;
	unsigned char checksum  = *pchecksum;
	int dst = mib_get_flash_offset(type);

	if(dst == 0)
	{
		COMP_TRACE("\r\n ERR!! no flash offset! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	switch(type)
	{
		case HW_SETTING:
			pcomp_sig = COMP_HS_SIGNATURE;
			break;
		case DEFAULT_SETTING:
			pcomp_sig = COMP_DS_SIGNATURE;
			break;
		case CURRENT_SETTING:
			pcomp_sig = COMP_CS_SIGNATURE;
			break;
		default:
			COMP_TRACE("\r\n ERR!! no type match __[%s-%u]\n",__FILE__,__LINE__);
			return 0;

	}
	expLen = DWORD_SWAP(pheader->len)+sizeof(TLV_PARAM_HEADER_T);
	if(expLen == 0)
	{
		COMP_TRACE("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	real_size = mib_get_real_len(type);
	if(real_size == 0)
	{
		COMP_TRACE("\r\n ERR!! no expLen! __[%s-%u]\n",__FILE__,__LINE__);
		return 0;
	}
	
	if( (compPtr = malloc(real_size)) == NULL)
	{
		COMP_TRACE("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",real_size,__FILE__,__LINE__);
	}

	if( (expPtr = malloc(expLen)) == NULL)
	{
		COMP_TRACE("\r\n ERR!! malloc  size %u failed! __[%s-%u]\n",expLen,__FILE__,__LINE__);
		if(compPtr != NULL)
			free(compPtr);
	}
	
	if(compPtr != NULL && expPtr!= NULL)
	{
		//int status;
        pContent = &expPtr[sizeof(TLV_PARAM_HEADER_T)];	// point to start of MIB data 

		pheader->len=DWORD_SWAP(pheader->len);
        //diag_printf("[%s:%d]pheader->len=%x\n", __FUNCTION__, __LINE__, pheader->len);
		memcpy(pContent, data, pheader->len-sizeof(checksum));
		memcpy(pContent+pheader->len-sizeof(checksum), &checksum, sizeof(checksum));
		pheader->len=DWORD_SWAP(pheader->len);
		memcpy(expPtr, pheader, sizeof(TLV_PARAM_HEADER_T));

		compLen = Encode(expPtr, expLen, compPtr+sizeof(COMPRESS_MIB_HEADER_T));
		sprintf((char *)compHeader.signature,"%s",pcomp_sig);
		if(compLen==0)
        {
			free(compPtr);
			free(expPtr);
			diag_printf("Encode mib error, compLen=0![%s-%u]\n",__FILE__,__LINE__);			
			return 0;
        }
		compHeader.compRate = WORD_SWAP((expLen/compLen)+1);
		compHeader.compLen = DWORD_SWAP(compLen);
		memcpy(compPtr, &compHeader, sizeof(COMPRESS_MIB_HEADER_T));
       
		if ( rtk_flash_write((char *)compPtr, dst, compLen+sizeof(COMPRESS_MIB_HEADER_T))==0 )
//		if ( write(fh, (const void *)compPtr, compLen+sizeof(COMPRESS_MIB_HEADER_T))!=compLen+sizeof(COMPRESS_MIB_HEADER_T) ) 
		{
			free(compPtr);
			free(expPtr);
			printf("Write flash compress setting[%u] failed![%s-%u]\n",type,__FILE__,__LINE__);			
			return 0;
		}
#if 0 
fprintf(stderr,"\r\n Write compress data to 0x%x. Expenlen=%u, CompRate=%u, CompLen=%u, __[%s-%u]",dst, expLen, compHeader.compRate, compHeader.compLen,__FILE__,__LINE__);

#endif
		
		if(compPtr != NULL)
			free(compPtr);
		if(expPtr != NULL)
			free(expPtr);

		return 1;
			
	}
	else
	{
        if(compPtr != NULL)
			free(compPtr);
		if(expPtr != NULL)
			free(expPtr);	
		return 0;
	}

 //   diag_printf("[%s:%d]\n", __FUNCTION__, __LINE__);
	return 0;
}
#endif //#ifdef COMPRESS_MIB_SETTING

void validate_hw_setings(void)
{
	char hw_setting_start[6];

	if (rtk_flash_read(hw_setting_start, HW_SETTING_OFFSET, 6) == 0) {
		printf("read hs start failed!\n");
		return;
	}

	if (memcmp(hw_setting_start, COMP_HS_SIGNATURE, 6) == 0) {
		//convert to uncompressed format
		printf("convert hs format\n");
		apmib_update(HW_SETTING);
	}
}
