/*****************************************************************************
Copyright (C), 吉祥腾达，保留所有版权
File name ： nvram.c
Description : nvram/envram配置接口，对flash进行读写操作
Author ：jack deng
Version ：V1.0
Date ：2015-12-19
Others ：
History ：
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
1) 日期    ：2015-12-19    修改者：jack
   内容     ：添加envram接口
   Version ：V1.1

2) 日期    ：2015-12-23    修改者：jack
   内容     ：添加nvram/envram erase接口
   Version ：V1.2
3) 日期    ：2015-12-23    修改者：jack
   内容     ：1. cli_nvram...与envram_..等接口函数，传参大小同步到ecos 博通平台。
                        2.添加envram命令支持
    Version ：V1.3

4) 日期    ：2016-01-04    修改者：jack
   内容     ：1. 添加nvram default ,envram default命令
              2. nvram初始化时，校验crc头部
    Version ：V1.4

*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <cyg/kernel/kapi.h>
#include "nvram.h"
#include "wifi.h"
//#define NVRAM_DEBUG

#ifdef __ECOS
#define NVRAM_LOCK()    cyg_scheduler_lock()
#define NVRAM_UNLOCK()  cyg_scheduler_unlock() 
#else 
#define NVRAM_LOCK()    do {} while (0) 
#define NVRAM_UNLOCK()  do {} while (0)
#endif 

#ifdef NVRAM_DEBUG
#define NVRAM_DBG       printf
#else
#define NVRAM_DBG
#endif

#define RTK_FLASH_SUCCESS   1
#define RTK_FLASH_FAIL      0

int rtk_flash_read(char *buf, int offset, int len);
int rtk_flash_write(char *buf, int offset, int len);


extern struct nvram_tuple tenda_nvram_defaults[];
extern struct nvram_tuple tenda_envram_defaults[];

static struct nvram_mtd_info s_nvram_mtd;
static struct nvram_mtd_info s_envram_mtd;

struct nvram_tuple *s_nvram_tuple[NVRAM_MTD_HASH_SIZE];
struct nvram_tuple *s_envram_tuple[ENVRAM_MTD_HASH_SIZE];
static char *nvram_usage="usage:nvram <set> <get> <unset> <show> <commit> <erase> <default>";
static char *envram_usage="usage:envram <set> <get> <unset> <show> <commit> <erase> <default>";

static void _nvram_free(struct nvram_tuple *t);

//================ NVRAM CRC ==============================
/* crc defines */
#define CRC8_INIT_VALUE  0xff       /* Initial CRC8 checksum value */
#define CRC8_GOOD_VALUE  0x9f       /* Good final CRC8 checksum value */
#define CRC16_INIT_VALUE 0xffff     /* Initial CRC16 checksum value */
#define CRC16_GOOD_VALUE 0xf0b8     /* Good final CRC16 checksum value */
#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */

static const uint8 crc8_table[256] = {
    0x00, 0xF7, 0xB9, 0x4E, 0x25, 0xD2, 0x9C, 0x6B,
    0x4A, 0xBD, 0xF3, 0x04, 0x6F, 0x98, 0xD6, 0x21,
    0x94, 0x63, 0x2D, 0xDA, 0xB1, 0x46, 0x08, 0xFF,
    0xDE, 0x29, 0x67, 0x90, 0xFB, 0x0C, 0x42, 0xB5,
    0x7F, 0x88, 0xC6, 0x31, 0x5A, 0xAD, 0xE3, 0x14,
    0x35, 0xC2, 0x8C, 0x7B, 0x10, 0xE7, 0xA9, 0x5E,
    0xEB, 0x1C, 0x52, 0xA5, 0xCE, 0x39, 0x77, 0x80,
    0xA1, 0x56, 0x18, 0xEF, 0x84, 0x73, 0x3D, 0xCA,
    0xFE, 0x09, 0x47, 0xB0, 0xDB, 0x2C, 0x62, 0x95,
    0xB4, 0x43, 0x0D, 0xFA, 0x91, 0x66, 0x28, 0xDF,
    0x6A, 0x9D, 0xD3, 0x24, 0x4F, 0xB8, 0xF6, 0x01,
    0x20, 0xD7, 0x99, 0x6E, 0x05, 0xF2, 0xBC, 0x4B,
    0x81, 0x76, 0x38, 0xCF, 0xA4, 0x53, 0x1D, 0xEA,
    0xCB, 0x3C, 0x72, 0x85, 0xEE, 0x19, 0x57, 0xA0,
    0x15, 0xE2, 0xAC, 0x5B, 0x30, 0xC7, 0x89, 0x7E,
    0x5F, 0xA8, 0xE6, 0x11, 0x7A, 0x8D, 0xC3, 0x34,
    0xAB, 0x5C, 0x12, 0xE5, 0x8E, 0x79, 0x37, 0xC0,
    0xE1, 0x16, 0x58, 0xAF, 0xC4, 0x33, 0x7D, 0x8A,
    0x3F, 0xC8, 0x86, 0x71, 0x1A, 0xED, 0xA3, 0x54,
    0x75, 0x82, 0xCC, 0x3B, 0x50, 0xA7, 0xE9, 0x1E,
    0xD4, 0x23, 0x6D, 0x9A, 0xF1, 0x06, 0x48, 0xBF,
    0x9E, 0x69, 0x27, 0xD0, 0xBB, 0x4C, 0x02, 0xF5,
    0x40, 0xB7, 0xF9, 0x0E, 0x65, 0x92, 0xDC, 0x2B,
    0x0A, 0xFD, 0xB3, 0x44, 0x2F, 0xD8, 0x96, 0x61,
    0x55, 0xA2, 0xEC, 0x1B, 0x70, 0x87, 0xC9, 0x3E,
    0x1F, 0xE8, 0xA6, 0x51, 0x3A, 0xCD, 0x83, 0x74,
    0xC1, 0x36, 0x78, 0x8F, 0xE4, 0x13, 0x5D, 0xAA,
    0x8B, 0x7C, 0x32, 0xC5, 0xAE, 0x59, 0x17, 0xE0,
    0x2A, 0xDD, 0x93, 0x64, 0x0F, 0xF8, 0xB6, 0x41,
    0x60, 0x97, 0xD9, 0x2E, 0x45, 0xB2, 0xFC, 0x0B,
    0xBE, 0x49, 0x07, 0xF0, 0x9B, 0x6C, 0x22, 0xD5,
    0xF4, 0x03, 0x4D, 0xBA, 0xD1, 0x26, 0x68, 0x9F
};

static uint8 hndcrc8(
    unsigned char *pdata,   /* pointer to array of data to process */
    unsigned int  nbytes,   /* number of input data bytes to process */
    unsigned char crc   /* either CRC8_INIT_VALUE or previous return value */
)
{
    /* hard code the crc loop instead of using CRC_INNER_LOOP macro
     * to avoid the undefined and unnecessary (uint8 >> 8) operation.
     */
    while (nbytes-- > 0)
        crc = crc8_table[(crc ^ *pdata++) & 0xff];

    return crc;
}

/* returns the CRC8 of the nvram */
static uint8 nvram_calc_crc(struct nvram_header *nvh,unsigned int max)
{
    struct nvram_header tmp;
    uint8 crc;

    if ((nvh->len > max) || (nvh->len < sizeof(struct nvram_header))) {
        return (nvh->crc_ver_init+1);
    }
    /* Little-endian CRC8 over the last 11 bytes of the header */
    tmp.crc_ver_init = htonl((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
    tmp.config_refresh = htonl(nvh->config_refresh);
    tmp.config_ncdl = htonl(nvh->config_ncdl);

    crc = hndcrc8((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
        sizeof(struct nvram_header) - NVRAM_CRC_START_POSITION,
        CRC8_INIT_VALUE);

    /* Continue CRC8 over data bytes */
    crc = hndcrc8((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);

    return crc;
}

/* String hash */
static inline unsigned int hash(const char *s)
{
    unsigned int hashval = 0;

    while (*s)
        hashval = 31 *hashval + *s++;

    return hashval;
}

//==================== FLASH READ/WRITE ======================
int tenda_flash_read(char *buff,int offset, int len)
{
    return rtk_flash_read(buff,offset,len);
}

int tenda_flash_write(char *buff,int offset, int len)
{
    return rtk_flash_write(buff,offset,len);
}
extern uint64_t get_flash_size(void);
int tenda_get_flash_size(void)
{
    int flash_size = get_flash_size();;
    return flash_size;
}

//===================== NVRAM PRIVITE API =========================
static struct nvram_tuple *_nvram_realloc(struct nvram_tuple *t, const char *name, const char *value)
{
    if (!(t = malloc(sizeof(struct nvram_tuple) + strlen(name) + 1 +
                     strlen(value) + 1))) {
        printf("_nvram_realloc: our of memory\n");
        return NULL;
    }

    /* Copy name */
    t->name = (char *) &t[1];
    strcpy(t->name, name);

    /* Copy value */
    t->value = t->name + strlen(name) + 1;
    strcpy(t->value, value);

    return t;
}

/* Get the value of an NVRAM variable. Should be locked. */
static char *_nvram_get(struct nvram_mtd_info *mtd,const char *name)
{
    unsigned int i;
    struct nvram_tuple *t;
    char *value;

    if (!name)
        return NULL;
    

    /* Hash the name */
    i = hash(name) % mtd->nvram_hash_size;

    /* Find the associated tuple in the hash table */
    for (t = mtd->nvram_hash[i]; t && t->name && strcmp(t->name, name); t = t->next);

    value = t ? t->value : NULL;

    return value;
}
/*每一台DUT特有的参数lq*/
char *special_nvram[] = 
	{
		"wl1_hwaddr",
		"et0macaddr",
		"wl0.1_hwaddr",
		"wl1.1_hwaddr",
		"wps_device_pin",
		"wl0.2_hwaddr",
		"wl1.2_hwaddr",
		"wl_conf_hwaddr",
		"wl0.3_hwaddr",
		"wl1.3_hwaddr",
		"uc_serialnum",
		"country_offset_power",
		"HW_WLAN0_11N_THER",
		"HW_WLAN0_TX_POWER_DIFF_5G_40BW2S_20BW2S_A",
		"HW_WLAN0_TX_POWER_DIFF_5G_40BW2S_20BW2S_B",
		"HW_WLAN1_11N_TRSWITCH",
		"BOARD_NAME",
		"HW_WLAN1_TX_POWER_DIFF_OFDM",
		"HW_WLAN1_11N_XCAP",
		"HW_WLAN0_11N_TSSI1",
		"HW_WLAN0_11N_TSSI2",
		"HW_WLAN0_11N_XCAP",
		"HW_WLAN0_11N_TRSWITCH",
		"HW_WLAN0_TX_POWER_5G_HT40_1S_A",
		"HW_WLAN0_TX_POWER_5G_HT40_1S_B",
		"HW_WLAN1_11N_TRSWPAPE_C9",
		"HW_WLAN0_11N_TRSWPAPE_C9",
		"wl0_country_pwr_power",
		"HW_WLAN0_RF_TYPE",
		"HW_WLAN1_TX_POWER_DIFF_HT20",
		"HW_WLAN1_11N_TRSWPAPE_CC",
		"HW_WLAN0_11N_TRSWPAPE_CC",
		"HW_WLAN1_TX_POWER_HT40_1S_A",
		"country_code",
		"HW_WLAN1_TX_POWER_HT40_1S_B",
		"HW_WLAN1_TX_POWER_CCK_A",
		"HW_WLAN1_TX_POWER_CCK_B",
		"HW_BOARD_VER",
		"HW_WLAN0_TX_POWER_DIFF_5G_20BW1S_OFDM1T_A",
		"wl0_hwaddr",
		"HW_WLAN0_TX_POWER_DIFF_5G_20BW1S_OFDM1T_B",
		"HW_WLAN1_11N_TSSI1",
		"HW_WLAN1_11N_TSSI2",
		"wl1_country_pwr_power",
		"HW_WLAN1_LED_TYPE",
		"HW_WLAN0_REG_DOMAIN",
		"HW_WLAN0_LED_TYPE",
		"HW_WLAN1_RF_TYPE",
		"HW_WLAN1_TX_POWER_DIFF_HT40_2S",
		"HW_WLAN1_11N_THER",
		"HW_WLAN1_REG_DOMAIN",
		"auto_sync_count",
	};
/*判断该参数是否为每一台DUT特有的参数lq*/
inline int _nvram_check_in_special_list(const char *key)
{
	int i = 0;
	int array_size = 0;
	if(key == NULL)
		return 1;
	array_size = (sizeof(special_nvram))/(sizeof(special_nvram[0]));
	for(i = 0;i < array_size;i++)
	{
		if(0 == strcmp(key,special_nvram[i]))
		{
			return 1;
		}
	}
	return 0;
}
/* Get all NVRAM variables. Should be locked. */
static int _nvram_getall(struct nvram_mtd_info *mtd, char *buf, int count,int filter)
{
    unsigned int i;
    struct nvram_tuple *t;
    int len = 0;
    int mac_clone_disable = 0;
    bzero(buf, count);

	i = hash("wan0_macclone_mode") % mtd->nvram_hash_size;

    /* Find the associated tuple in the hash table */
    for (t = mtd->nvram_hash[i]; t && t->name && strcmp(t->name, "wan0_macclone_mode"); t = t->next);

    if(t != NULL)
    {
    	 if(0 == strcmp(t->value,"default"))
		 mac_clone_disable = 1;
    }
		
    /* Write name=value\0 ... \0\0 */
    for (i = 0; i < mtd->nvram_hash_size; i++) {
        for (t = mtd->nvram_hash[i]; t; t = t->next) {
	     if(filter && _nvram_check_in_special_list(t->name))
	     		continue;
		 //
	     if(mac_clone_disable && (0 == strcmp(t->name,"wan0_hwaddr")))
	     		continue;
            if ((count - len) > (strlen(t->name) + 1 + strlen(t->value) + 1))
                len += sprintf(buf + len, "%s=%s", t->name, t->value) + 1;
            else
                break;
        }
    }

    return 0;
}

/* Get the value of an NVRAM variable. Should be locked. */
static int _nvram_set(struct nvram_mtd_info *mtd, const char *name, const char *value)
{
    uint32 i;
    struct nvram_tuple *t, *u, *next, **prev;

    /* Hash the name */
    i = hash(name) % mtd->nvram_hash_size;

    /* Find the associated tuple in the hash table */
    for (prev = &mtd->nvram_hash[i], t = *prev; t && strcmp(t->name, name); prev = &t->next, t = *prev);

    /* (Re)allocate tuple */
    if (!(u = _nvram_realloc(t, name, value)))
        return -12; /* -ENOMEM */

    /* Value reallocated */
    if (t && t == u)
        return 0;

    /* Move old tuple to the dead table */
    if (t) {
        *prev = t->next;
        t->next = mtd->nvram_dead;
        mtd->nvram_dead = t;
    }

    /* Add new tuple to the hash table */
    u->next = mtd->nvram_hash[i];
    mtd->nvram_hash[i] = u;

    /* add by jack,2016-1-20,free mem */
    for (t = mtd->nvram_dead; t; t = next) {
        next = t->next;
        _nvram_free(t);
    }
    mtd->nvram_dead = NULL;

    return 0;
}

/* Unset the value of an NVRAM variable. Should be locked. */
static int _nvram_unset(struct nvram_mtd_info *mtd, const char *name)
{
    unsigned int i;
    struct nvram_tuple *t, **prev;

    if (!name)
        return 0;
    
    
    /* Hash the name */
    i = hash(name) % mtd->nvram_hash_size;

    /* Find the associated tuple in the hash table */
    for (prev = &mtd->nvram_hash[i], t = *prev; t && strcmp(t->name, name);
         prev = &t->next, t = *prev);

    /* Move it to the dead table */
    if (t) {
        *prev = t->next;
        t->next = mtd->nvram_dead;
        mtd->nvram_dead = t;
    }

    return 0;
}

static void _nvram_free(struct nvram_tuple *t)
{
    if (t)
        free(t);
}

static void nvram_free(struct nvram_mtd_info *mtd)
{
    unsigned int i;
    struct nvram_tuple *t, *next;

    /* Free hash table */
    for (i = 0; i < mtd->nvram_hash_size; i++) {
        for (t = mtd->nvram_hash[i]; t; t = next) {
            next = t->next;
            _nvram_free(t);
        }
        mtd->nvram_hash[i] = NULL;
    }

    /* Free dead table */
    for (t = mtd->nvram_dead; t; t = next) {
        next = t->next;
        _nvram_free(t);
    }
    mtd->nvram_dead = NULL;

    /* Indicate to per-port code that all tuples have been freed */
    _nvram_free(NULL);
}

static int _nvram_read(struct nvram_mtd_info *mtd,void *buf)
{
    if (tenda_flash_read(buf, mtd->base_addr, mtd->mtd_size) == RTK_FLASH_SUCCESS)
        return 0;
    else
        return -1;
}

static int nvram_rehash(struct nvram_mtd_info *mtd, struct nvram_header *header)
{
    //char buf[] = "0xXXXXXXXX";
    char *name, *value, *end, *eq;

    /* (Re)initialize hash table */
    nvram_free(mtd);

    /* Parse and set "name=value\0 ... \0\0" */
    name = (char *) &header[1];
    end = (char *) header + mtd->mtd_size - 2;
    end[0] = end[1] = '\0';
    for (; *name; name = value + strlen(value) + 1) {
        if (!(eq = strchr(name, '=')))
            break;
        *eq = '\0';
        value = eq + 1;
        _nvram_set(mtd, name, value);
        *eq = '=';
    }

#if 0
    /* Set special SDRAM parameters */
    if (!_nvram_get("sdram_init")) {
        sprintf(buf, "0x%04X", (uint16)(header->crc_ver_init >> 16));
        _nvram_set("sdram_init", buf);
    }
    if (!_nvram_get("sdram_config")) {
        sprintf(buf, "0x%04X", (uint16)(header->config_refresh & 0xffff));
        _nvram_set("sdram_config", buf);
    }
    if (!_nvram_get("sdram_refresh")) {
        sprintf(buf, "0x%04X", (uint16)((header->config_refresh >> 16) & 0xffff));
        _nvram_set("sdram_refresh", buf);
    }
    if (!_nvram_get("sdram_ncdl")) {
        sprintf(buf, "0x%08X", header->config_ncdl);
        _nvram_set("sdram_ncdl", buf);
    }
#endif
    return 0;
}

/* Regenerate NVRAM. Should be locked. */
static int _nvram_commit(struct nvram_mtd_info *mtd, struct nvram_header *header)
{
//  char *init, *config, *refresh, *ncdl;
    char *ptr, *end;
    int i;
    struct nvram_tuple *t;

    /* Regenerate header */
    header->magic = NVRAM_MAGIC;
    header->crc_ver_init = (NVRAM_VERSION << 8);
#if 0
    if (!(init = _nvram_get("sdram_init")) ||
        !(config = _nvram_get("sdram_config")) ||
        !(refresh = _nvram_get("sdram_refresh")) ||
        !(ncdl = _nvram_get("sdram_ncdl"))) {
        header->crc_ver_init |= SDRAM_INIT << 16;
        header->config_refresh = SDRAM_CONFIG;
        header->config_refresh |= SDRAM_REFRESH << 16;
        header->config_ncdl = 0;
    } else {
        header->crc_ver_init |= (strtoul(init, NULL, 0) & 0xffff) << 16;
        header->config_refresh = strtoul(config, NULL, 0) & 0xffff;
        header->config_refresh |= (strtoul(refresh, NULL, 0) & 0xffff) << 16;
        header->config_ncdl = strtoul(ncdl, NULL, 0);
    }
#endif
    /* Clear data area */
    ptr = (char *) header + sizeof(struct nvram_header);
    bzero(ptr, mtd->mtd_size - sizeof(struct nvram_header));

    /* Leave space for a double NUL at the end */
    end = (char *) header + mtd->mtd_size - 2;


    /* Write out all tuples */
    for (i = 0; i < mtd->nvram_hash_size; i++) {
        for (t = mtd->nvram_hash[i]; t; t = t->next) {
            if ((ptr + strlen(t->name) + 1 + strlen(t->value) + 1) > end)
                break;
            ptr += sprintf(ptr, "%s=%s", t->name, t->value) + 1;
        }
    }

    /* End with a double NUL */
    ptr += 2;

    /* Set new length */
    header->len = ROUNDUP(ptr - (char *) header, 4);

    /* Set new CRC8 */
    header->crc_ver_init |= nvram_calc_crc(header,mtd->mtd_size);

    /* Reinitialize hash table */
    return nvram_rehash(mtd,header);
}



static int _nvram_init(struct nvram_mtd_info *mtd)
{
    struct nvram_header *header;
    int ret;

    if (!(header = (struct nvram_header *) malloc(mtd->mtd_size))) {
        printf("nvram_init: out of memory\n");
        return -12; /* -ENOMEM */
    }

    if ((ret = _nvram_read(mtd, header)) == 0) {
        mtd->has_init = 1;
        printf("@@@[%s] crc=0x%x org=0x%x\n",mtd->name,nvram_calc_crc(header,mtd->mtd_size),(uint8) header->crc_ver_init);
        if (header->magic == NVRAM_MAGIC 
                && (nvram_calc_crc(header,mtd->mtd_size) == (uint8) header->crc_ver_init)) {
            nvram_rehash(mtd, header);
        } else {
            if (strcmp(mtd->name,ENVRAM_MTD_NAME) == 0) {
                envram_default();
            } else {
                nvram_default();
            }
        }
    }
    free(header);
    return ret;
}

static void _nvram_exit(struct nvram_mtd_info *mtd)
{
    nvram_free(mtd);
}

char *nvram_get(const char *name)
{
    char *value;
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return NULL;
    }

    NVRAM_LOCK();
    value = _nvram_get(mtd_info, name);
    NVRAM_UNLOCK();

    return value;
}

int nvram_getall(char *buf, int count,int filter)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
    ret = _nvram_getall(mtd_info, buf, count,filter);
    NVRAM_UNLOCK();

    return ret;
}

int nvram_set(const char *name, const char *value)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
//    NVRAM_DBG("%s:name=%s value=%s\n",__func__,name,value);
    ret = _nvram_set(mtd_info, name, value);
    NVRAM_UNLOCK();

    return ret;
}

int nvram_unset(const char *name)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
    ret = _nvram_unset(mtd_info, name);
    NVRAM_UNLOCK();

    return ret;
}

int nvram_commit(void)
{
    struct nvram_header *header;
    int ret;
    uint32 *src, *dst;
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    if (!(header = (struct nvram_header *) malloc(mtd_info->mtd_size))) {
        printf("nvram_commit: out of memory\n");
        return -12; /* -ENOMEM */
    }

    NVRAM_LOCK();

    /* Regenerate NVRAM */
    ret = _nvram_commit(mtd_info, header);
    if (ret)
        goto done;

    src = (uint32 *) &header[1];
    dst = src;

    header->magic = NVRAM_MAGIC;
    if(tenda_flash_write((char *) header, mtd_info->base_addr, mtd_info->mtd_size)
        == RTK_FLASH_FAIL)
        ret = -1;

done:
    NVRAM_UNLOCK();
    free(header);

    return ret;
}

#ifdef __CONFIG_A9__
#define nvram_safe_get(name) (nvram_get(name) ? : "")

/*******初始化A9配置*******/
void init_apclient_conf()
{
	/*first time to enabled wl0.1 interface*/
	validate_vif_ssid(nvram_get("wl_ssid"));

	/*cp wl0 to wl*/
	copy_wl_index_to_unindex("0");
	
	/*cp wl to wl0 and wl0.1*/
	wl_unit("0",1);	 	 	
	wl_unit("0.1",1);	 	
	
	/*set wireless mode*/
	nvram_set("wl_ure", "1");
	nvram_set("wl0_mode", "wet");
	nvram_set("wl0.1_mode", "ap");

#ifdef __CONFIG_AUTO_CONN_CLIENT__
	/*恢复出厂设置时，连接上级的SSID不能为Tenda_Extender_1,
	  否则会导致有自动桥接服务器时直接连接连接到服务器的虚拟接口
	  默认SSID也不能是Tenda+mac后六位，烧录软件为Tenda_888888，
	  一旦上电，如果环境中存在该SSID就会自动连接上，导致ip发生变化，对产测有影响
	  因此改为一个用户不太可能使用的SSID:Tenda_Extender_default
	*/
	nvram_set("wl0_ssid", "Tenda_Extender_default");
#endif


	/*set lan ifname*/
	nvram_set("lan_ifnames", "eth0 wlan0 wlan0-vxd0");
	nvram_unset("wan_ifnames");

	/*set wps mode*/
	nvram_set("wps_mode", "enabled");
}

/***********是否同步桥接接口配置***********/


int sync_bridge_conf()
{
	char relaytype[8] = {0}, quick_set[8] = {0}, sec_type[16] = {0};
	int need_sync = 0;
	strncpy(relaytype, nvram_safe_get("wl0_relaytype"), sizeof(relaytype));
	strncpy(quick_set, nvram_safe_get("restore_quick_set"), sizeof(quick_set));
	strncpy(sec_type, nvram_safe_get("wl0_akm"), sizeof(sec_type));
/*lq恢复出厂设置，对自动桥接和一键桥接进行同上级*/	
	if(strcmp(quick_set, "0") && strcmp(relaytype, "web"))
	{
		nvram_set("restore_quick_set", "0");
		nvram_set("wl0.1_ssid", nvram_safe_get("wl0_ssid"));
		if(0 == strcmp(sec_type, ""))
		{
			nvram_set("wl0.1_akm", "");
			nvram_set("wl0.1_crypto", "");
		}
		else
		{
			nvram_set("wl0.1_akm", "psk psk2");
			nvram_set("wl0.1_crypto", "aes");
			nvram_set("wl0.1_wpa_psk", nvram_safe_get("wl0_wpa_psk"));
		}
	}
/* lq页面桥接,进行配置同步*/
	else if(!strcmp(relaytype, "web"))
	{
		if(strcmp(quick_set, "0") )
		{
			nvram_set("restore_quick_set", "0");
			need_sync = 1;
		}
		
		if(strcmp(nvram_safe_get("extend_ssid"),nvram_safe_get("wl0.1_ssid")))
		{
			need_sync = 1;
		}

		if((!strcmp(nvram_safe_get("wl0.1_akm"),"") && strcmp(nvram_safe_get("extend_pass"),"")))
		{
			need_sync = 1;
		}
		if(strcmp(nvram_safe_get("wl0.1_akm"),"") && strcmp(nvram_safe_get("extend_pass"),nvram_safe_get("wl0.1_wpa_psk")))
		{
			need_sync = 1;
		}

		if(need_sync)
		{
			nvram_set("wl0.1_ssid", nvram_safe_get("extend_ssid"));
			if(!strcmp(nvram_safe_get("extend_pass"),""))
			{
				nvram_set("wl0.1_akm", "");
				nvram_set("wl0.1_crypto", "");
			}
			else
			{
				nvram_set("wl0.1_akm", "psk psk2");
				nvram_set("wl0.1_crypto", "aes");
				nvram_set("wl0.1_wpa_psk", nvram_safe_get("extend_pass"));
			}
		}
		else
			return 0;

	}
	else
		return 0;

	nvram_commit();
	return 1;
}
#endif

void nvram_default(void)
{
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;
    struct nvram_tuple *t;
    char *ssid = NULL;
    /* Clear nvram ram table */
    nvram_free(mtd_info);

    /* Clear nvram mtd */
    nvram_erase();

    /* Restore defaults */
    printf("nvram restore from default\n");
    for (t = tenda_nvram_defaults; t->name; t++) {
        _nvram_set(mtd_info,t->name, t->value);
    }

    printf("nvram restore from envram\n");
    envram_to_nvram();

	/* Indonesia has no 40MHz and 80MHz bandwidth in 5G band */
	if(0 == strcmp(nvram_safe_get("country_code"),"ID"))
	{
		nvram_set(WLAN5G_BANDWIDTH, "20");
	}
/*lq 如果定制需要，直接使用产测工具修改*/
    ssid = nvram_get("wl0_ssid");
    if(NULL != ssid && !strcmp(ssid,"Tenda"))
    {
	  set_ssid_mac(WL_24G);
	  set_ssid_mac(WL_5G);
    }

#ifdef __CONFIG_A9__
	init_apclient_conf();
#endif

    nvram_commit();
}

void nvram_check_do_restore(void)
{
    char *str;
    str = nvram_get("restore_defaults");
    if (str && (strcmp(str,"1") == 0)) {
        printf("Nvram Restore.....!!!\n");
        nvram_default();
    }
}
int nvram_init(void)
{
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;
    int ret = 0;
 
    /* for envram init */
    if (envram_init(0,NULL)) {
        printf("%s:envram init fail,so exit!!!\n",__func__);
        return -1;
    }

    if (mtd_info->has_init) {
        //printf("Nvram has init!!!\n");
        return 0;
    }
    memset(mtd_info, 0, sizeof(struct nvram_mtd_info));

    strncpy(mtd_info->name,NVRAM_MTD_NAME,sizeof(mtd_info->name));
    mtd_info->base_addr = tenda_get_flash_size();
    mtd_info->base_addr -= NVRAM_MTD_SIZE;
    mtd_info->mtd_size  = NVRAM_MTD_SIZE - 1;
    mtd_info->nvram_hash_size = NVRAM_MTD_HASH_SIZE;
#if 0
    mtd_info->nvram_hash = (struct nvram_tuple **)malloc(
                                sizeof(struct nvram_tuple)
                                 * NVRAM_MTD_HASH_SIZE);
#else
    mtd_info->nvram_hash = s_nvram_tuple;
#endif
    mtd_info->nvram_dead = NULL;
    printf("Nvram is initing! base_addr=0x%x size=0x%x\n",mtd_info->base_addr,mtd_info->mtd_size);
    ret = _nvram_init(mtd_info);
    if (!ret) {
        nvram_check_do_restore();
    }
    return ret;
}

void nvram_exit(void)
{
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;

    _nvram_exit(mtd_info);
    free(mtd_info->nvram_hash);
    memset(mtd_info, 0, sizeof(struct nvram_mtd_info));
}

int nvram_erase(void)
{
    struct nvram_mtd_info *mtd_info = &s_nvram_mtd;
	char *file;

	file = malloc(mtd_info->mtd_size);
	if (file == NULL)
		return -1;

	NVRAM_LOCK();
	memset(file, 0xff, mtd_info->mtd_size);
    if (tenda_flash_write((char *) file, mtd_info->base_addr, mtd_info->mtd_size)
            == RTK_FLASH_FAIL) {
        printf("Erase NVRAM fail! \n");
    	free(file);
    	NVRAM_UNLOCK();
        return -1;
    }
    printf("Erase NVRAM to blank! \n");
	free(file);
	NVRAM_UNLOCK();

	return 0;
}

//=============envram=======================
static char *mtd_envram_get(const char *name)
{
    char *value;
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return NULL;
    }

    NVRAM_LOCK();
    value = _nvram_get(mtd_info, name);
    NVRAM_UNLOCK();

    return value;
}

static int mtd_envram_getall(char *buf, int count)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
    ret = _nvram_getall(mtd_info, buf, count,0);
    NVRAM_UNLOCK();

    return ret;
}

static int mtd_envram_set(const char *name, const char *value)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
    ret = _nvram_set(mtd_info, name, value);
    NVRAM_UNLOCK();

    return ret;
}

static int mtd_envram_unset(const char *name)
{
    int ret;
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    NVRAM_LOCK();
    ret = _nvram_unset(mtd_info, name);
    NVRAM_UNLOCK();

    return ret;
}

static int mtd_envram_commit(void)
{
    struct nvram_header *header;
    int ret;
    uint32 *src, *dst;
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    if (!mtd_info->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd_info->name);
        return -1;
    }

    if (!(header = (struct nvram_header *) malloc(mtd_info->mtd_size))) {
        printf("nvram_commit: out of memory\n");
        return -12; /* -ENOMEM */
    }

    NVRAM_LOCK();

    /* Regenerate NVRAM */
    ret = _nvram_commit(mtd_info, header);
    if (ret)
        goto done;

    src = (uint32 *) &header[1];
    dst = src;

    header->magic = NVRAM_MAGIC;
    if (tenda_flash_write((char *) header, mtd_info->base_addr, mtd_info->mtd_size)
            == RTK_FLASH_FAIL)
        ret = -1;

done:
    NVRAM_UNLOCK();
    free(header);

    return ret;
}


static int mtd_envram_init(void)
{
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;
    int ret = 0;

    if (mtd_info->has_init) {
        //printf("Envram has init!!!\n");
        return 0;
    }

    memset(mtd_info, 0, sizeof(struct nvram_mtd_info));

    strncpy(mtd_info->name,ENVRAM_MTD_NAME,sizeof(mtd_info->name));
    mtd_info->base_addr = ENVRAM_MTD_BASE_ADDR;
    mtd_info->mtd_size  = ENVRAM_MTD_SIZE;
    mtd_info->nvram_hash_size = ENVRAM_MTD_HASH_SIZE;
#if 0
    mtd_info->nvram_hash = (struct nvram_tuple **)malloc(
                                sizeof(struct nvram_tuple)
                                 * NVRAM_MTD_HASH_SIZE);
#else
    mtd_info->nvram_hash = s_envram_tuple;
#endif
    mtd_info->nvram_dead = NULL;
    printf("Envram is initing! base_addr=0x%x size=0x%x\n",mtd_info->base_addr,mtd_info->mtd_size);
    ret = _nvram_init(mtd_info);
    return ret;
}

static void mtd_envram_exit(void)
{
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;

    _nvram_exit(mtd_info);
    free(mtd_info->nvram_hash);
    memset(mtd_info, 0, sizeof(struct nvram_mtd_info));
}

static int mtd_envram_erase(void)
{
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;
	char *file;

	file = malloc(mtd_info->mtd_size);
	if (file == NULL)
		return -1;

	NVRAM_LOCK();
	memset(file, 0xff, mtd_info->mtd_size);
    if (tenda_flash_write((char *) file, mtd_info->base_addr, mtd_info->mtd_size)
            == RTK_FLASH_FAIL) {
        printf("Erase ENVRAM fail! \n");
        free(file);
        NVRAM_UNLOCK();
        return -1;
    }
    printf("Erase ENVRAM to blank! \n");
	free(file);
	NVRAM_UNLOCK();

	return 0;
}



//=============tenda bcm envram api ===================

int envram_init(int argc, char* argv[])
{
    return mtd_envram_init();
}
int envram_exit(int argc, char* argv[])
{
    mtd_envram_exit();
    return 0;
}

int envram_get(int argc, char* argv[])
{
    char *var = NULL;
    char *name = NULL;
    if (argc < 3)
    	return -1;

    name = argv[2];
    var = mtd_envram_get(name);

    if (var) {
        argv[2] = var;
        printf("%s\n", var);
    }
    return 0;
}

char *__envram_get(const char *name)
{
    return mtd_envram_get(name);
}

int __envram_set(const char *name, const char *value)
{
    return mtd_envram_set(name,value);
}

int envram_set(int argc, char* argv[])
{
    char *value;
    char *name = NULL;
    if (argc < 3)
    	return -1;

    name = argv[2];
    value = strchr(name, '=');
	if (value == NULL || strlen(value) >= 512){
		printf("%s:value illegal: %s\n",__func__,value?value:"NULL");
		return -1;
	}	
	*value++ = 0;

    mtd_envram_set(name,value);
    return 0;
}

int envram_unset(int argc, char* argv[])
{
    if (argc < 3)
    	return -1;

    mtd_envram_unset(argv[2]);
    return 0;
}

int envram_commit(int argc, char* argv[])
{
    return mtd_envram_commit();
}

int envram_show(int argc, char* argv[])
{
    char *fb = NULL;
	char *sp = NULL;

    if ((fb = malloc(ENVRAM_MTD_SIZE)) == NULL) {
        printf("%s:error alloc buf!\n",__func__);
        return -1;
    }

    mtd_envram_getall(fb, ENVRAM_MTD_SIZE);

    for (sp = fb; *sp; sp += strlen(sp)+1) {
        printf("%s\n", sp);
    }

    free(fb);
    return 0;

}

int envram_erase(int argc, char *argv[])
{

    mtd_envram_erase();
    return 0;
}

int envram_to_nvram(void)
{
    struct nvram_mtd_info *mtd = &s_envram_mtd;
    struct nvram_tuple *t;
    int i;

    if (!mtd->has_init) {
        printf("%s: %s has not init!!!\n",__func__,mtd->name);
        return -1;
    }

    /* Write name=value\0 ... \0\0 */
    for (i = 0; i < mtd->nvram_hash_size; i++) {
        for (t = mtd->nvram_hash[i]; t; t = t->next) {
            nvram_set(t->name, t->value);
        }
    }
    set_minor_mac(WL_24G,WL_24G_GUEST,1);
    set_minor_mac(WL_24G,WL_24G_VA,2);
    set_minor_mac(WL_24G,WL_24G_REPEATER,0);
    set_minor_mac(WL_5G,WL_5G_GUEST,1);
    set_minor_mac(WL_5G,WL_5G_VA,2);
    set_minor_mac(WL_5G,WL_5G_REPEATER,0);
	
    return 0;
}

void envram_default(void)
{
    struct nvram_mtd_info *mtd_info = &s_envram_mtd;
    struct nvram_tuple *t;

    /* Clear nvram ram table */
    nvram_free(mtd_info);

    /* Clear nvram mtd */
    envram_erase(0,NULL);

    /* Restore defaults */
    printf("envram restore from default\n");

    for (t = tenda_envram_defaults; t->name; t++) {
        _nvram_set(mtd_info,t->name, t->value);
    }

    envram_commit(0,NULL);
}


/* NVRAM related commands */
static int cli_nvram_get(int argc, char *argv[])
{
    char *var = NULL;
    if (argc < 3)
        return -1;

    var = nvram_get(argv[2]);
    if (var)
        printf("%s\n", var);

    return 0;
}

static int cli_nvram_set(int argc, char* argv[])
{
    char *name;
    char *value;

    if (argc < 3)
        return -1;

    name = argv[2];
    value = strchr(name, '=');
    if (value == NULL)
        return 0;

    *value++ = 0;

    nvram_set(name, value);
    return 0;
}

static int cli_nvram_unset(int argc, char *argv[])
{
    if (argc < 3)
        return -1;

    nvram_unset(argv[2]);
    return 0;
}

static int cli_nvram_commit(int argc, char* argv[])
{
    nvram_commit();
    return 0;
}

static int cli_nvram_show(int argc, char* argv[])
{
    char *fb = NULL;
	char *sp = NULL;
    int len;

    if ((fb = malloc(NVRAM_MTD_SIZE)) == NULL) {
        printf("error alloc buf!\n");
        return -1;
    }

    nvram_getall(fb, NVRAM_MTD_SIZE,0);

    if (argc < 3) {
        for (sp = fb; *sp; sp += strlen(sp)+1) {
            printf("%s\n", sp);
        }
    }
    else {
        len = strlen(argv[2]);
        for (sp = fb; *sp; sp += strlen(sp)+1) {
            if (strncmp(sp, argv[2], len) == 0)
                printf("%s\n", sp);
        }
    }

    free(fb);
    return 0;
}

/*****************************************************************************
Function      : td_acs_check_assoc_scb()
Description   : 
Input         : 
Output        : 
Return        : 
Others        : 

*****************************************************************************/
int nvram_cmd_main(int argc,char **argv)
{
    int td_argc = argc;
    char **td_argv = argv;
    nvram_init();

    if (argc > 0) {
        if (strcmp("nvram",argv[0])) {
            td_argc++;
            td_argv--;
        }
    }

    if (td_argc < 2) {
        printf("%s\n",nvram_usage);
        return -1;
    }

    if (strcmp(td_argv[1],"show") == 0) {
        cli_nvram_show(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"set") == 0) {
        cli_nvram_set(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"unset") == 0) {
        cli_nvram_unset(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"get") == 0) {
        cli_nvram_get(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"commit") == 0) {
        cli_nvram_commit(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"erase") == 0) {
        nvram_erase();
    } else if (strcmp(td_argv[1],"default") == 0) {
        nvram_default();
    }else {
        printf("%s\n",nvram_usage);
    }
    
    return 0;
}


int envram_cmd_main(int argc,char **argv)
{
    int td_argc = argc;
    char **td_argv = argv;

    envram_init(0,NULL);

    if (argc > 0) {
        if (strcmp("envram",argv[0])) {
            td_argc++;
            td_argv--;
        }
    }

    if (td_argc < 2) {
        printf("%s\n",envram_usage);
        return -1;
    }

    if (strcmp(td_argv[1],"show") == 0) {
        envram_show(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"set") == 0) {
        envram_set(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"unset") == 0) {
        envram_unset(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"get") == 0) {
        envram_get(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"commit") == 0) {
        envram_commit(td_argc,td_argv);
    } else if (strcmp(td_argv[1],"erase") == 0) {
        envram_erase(0,NULL);
    } else if (strcmp(td_argv[1],"default") == 0) {
        envram_default();
    }else {
        printf("%s\n",envram_usage);
    }
    
    return 0;

}
