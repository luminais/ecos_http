/***********************************************************
Copyright (C), 1998-2013, Tenda Tech. Co., Ltd.
FileName: sys_backupcfg.c
Description:nvram 配置备份以及恢复             //模块描述      
Author: wwk;
Version : 1.0
Date: 2013-3-28
Function List:    // 主要函数及其功能
1. backupConfig           -------备份配置到备份区
2. restoreConfig   ------- 从备份区恢复配置
3. restoreNvramValue      ------- 解析配置到内存
4. checkIfNeedBackupcfg       -------检测是否需要备份

History:         // 历史修改记录
<author>   <time>   <version >   <desc>
wwk and cym        2013-3-28   1.0        新建模块
************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <flashutl.h>
#include <bcmnvram.h>
#include <bcmendian.h>
#include <time.h>

#include "sys_backupcfg.h"


#define NVRAM_SPACE		0x8000	/*nvram 大小*/
#define CFG_HEADER_LEN 	20

#define NVRAM_LOCK()	do {} while (0)
#define NVRAM_UNLOCK()	do {} while (0)


int backupTag = 1;
extern char *_nvram_get(const char *name);
extern int _nvram_set(const char *name, const char *value);

#define NVRAM_CRC_START_POSITION	9 /* magic, len, crc8 to be skipped */
#define NVRAM_CRC_VER_MASK	0xffffff00 /* for crc_ver_init */
#define CRC8_INIT_VALUE  0xff       /* Initial CRC8 checksum value */

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

#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]

uint8 BCMROMFN(hndcrc)(
	uint8 *pdata,	/* pointer to array of data to process */
	uint  nbytes,	/* number of input data bytes to process */
	uint8 crc	/* either CRC8_INIT_VALUE or previous return value */
)
{
	/* hard code the crc loop instead of using CRC_INNER_LOOP macro
	 * to avoid the undefined and unnecessary (uint8 >> 8) operation.
	 */
	while (nbytes-- > 0)
		crc = crc8_table[(crc ^ *pdata++) & 0xff];

	return crc;
}


uint8 BCMROMFN( nvram_calc_crc_value)(struct nvram_header *nvh)
{
	struct nvram_header tmp;
	uint8 crc;

	/* Little-endian CRC8 over the last 11 bytes of the header */
	tmp.crc_ver_init = htol32((nvh->crc_ver_init & NVRAM_CRC_VER_MASK));
	tmp.config_refresh = htol32(nvh->config_refresh);
	tmp.config_ncdl = htol32(nvh->config_ncdl);

	crc = hndcrc((uint8 *) &tmp + NVRAM_CRC_START_POSITION,
		sizeof(struct nvram_header) - NVRAM_CRC_START_POSITION,
		CRC8_INIT_VALUE);

	/* Continue CRC8 over data bytes */
	crc = hndcrc((uint8 *) &nvh[1], nvh->len - sizeof(struct nvram_header), crc);

	return crc;
}

/************************************************************
Function:	 backupConfig                //函数名
Description: 备份配置到备份区     //函数功能描述
Input:      
	无                                    //对输入值说明，包括每个
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	无
Others:                                         //其他说明
************************************************************/

void backupConfig(void)
{
	struct nvram_header  *header;
	
	
	unsigned int read_total =0;
	unsigned int flash_size;
	unsigned int offset_cfg,offset_backupcfg;
	uint8 new_crc_value = 0;
	unsigned char old_crc_valua;
	if (!(header = (struct nvram_header  *) malloc(NVRAM_SPACE ))) 
	{
		printf("nvram_commit: out of memory\n");
		return ; /* -ENOMEM */
	}

	if (sysFlashInit(NULL)) 
	{
		diag_printf("%s: error initializing flash\n", __func__);
		free(header);
		return ;
	}
	flash_size=get_flashsize();
	if(flash_size ==0 )
	{
		printf("get flashsize error");
		return;
	}

	offset_cfg = flash_size - NVRAM_SPACE;
	offset_backupcfg = flash_size - 2*NVRAM_SPACE -2* NVRAM_SPACE;/*flash 以64K大小进行擦除*/
	
	printf("offset_nvram=%d,offset_backup=%d\n",offset_cfg,offset_backupcfg);

	NVRAM_LOCK();

	if (sysFlashInit(NULL))
	{
		printf("%s: error initializing flash\n", __func__);
		return;
	}
	memset(header,0,NVRAM_SPACE);
	read_total=sysFlashRead(offset_cfg, (char *)header, NVRAM_SPACE);

	new_crc_value =nvram_calc_crc_value(header);
	old_crc_valua = header->crc_ver_init & 0x000000FF;
	if(old_crc_valua == new_crc_value )
	{
		sysFlashWrite(offset_backupcfg, ( char *)header, NVRAM_SPACE);
		
	}
	
	NVRAM_UNLOCK();
	
	free(header);

	return;
}
/************************************************************
Function:	 restoreConfig                //函数名
Description: 从备份区恢复配置到nvram      //函数功能描述
Input:      
	无                                    //对输入值说明，包括每个
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	无
Others:                                         //其他说明
************************************************************/
void restoreConfig(void)
{
	
	struct nvram_header *buff;
	unsigned int read_total;
	unsigned int flash_size;
	unsigned int offset_cfg ,offset_backupcfg;
	uint32 new_crc_value = 0;
	unsigned char old_crc_valua;
	if (!(buff = ( struct nvram_header *) malloc(NVRAM_SPACE))) 
	{
		printf("nvram_commit: out of memory\n");
		return; /* -ENOMEM */
	}


	if (sysFlashInit(NULL)) 
	{
		diag_printf("%s: error initializing flash\n", __func__);
		free(buff);
		return ;
	}
	flash_size = get_flashsize();
	if(flash_size ==0 )
	{
		printf("get flashsize error");
		return;
	} 
	
	offset_cfg= flash_size - NVRAM_SPACE;
	offset_backupcfg=flash_size - 2*NVRAM_SPACE - 2*NVRAM_SPACE;
	
	printf("offset_nvram=%d,offset_backup=%d\n",offset_cfg,offset_backupcfg);

	NVRAM_LOCK();

	if (sysFlashInit(NULL))
	{
		printf("%s: error initializing flash\n", __func__);
		return;
	}
	read_total=sysFlashRead(offset_backupcfg, (unsigned char *)buff, NVRAM_SPACE);
	new_crc_value =nvram_calc_crc_value( buff);
	old_crc_valua =buff->crc_ver_init & 0x000000FF;
	if(old_crc_valua == new_crc_value)
	{
		sysFlashWrite(offset_cfg, (unsigned char *)buff, NVRAM_SPACE);
		restoreNvramValue(buff);
	}
	
	NVRAM_UNLOCK();
	
	free(buff);

	return;
}
/************************************************************
Function:	 restoreNvramValue                //函数名
Description: 解析配置到内存        //函数功能描述
Input:      
	struct nvram_header *header_backupcfg                //flash block     
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	无
Others:                                         //其他说明
************************************************************/
 int restoreNvramValue(struct nvram_header * header_backupcfg)
{
	char buf[] = "0xXXXXXXXX", *name, *value, *end, *eq;
	uint32 *src, *dst;
	uint i;//
	struct nvram_header *header_value;
	
	if (!(header_value = (struct nvram_header *) malloc( NVRAM_SPACE))) 
	{
		printf("nvram_init: out of memory\n");
		return -12; /* -ENOMEM */
	}
	
	src = (uint32 *) header_backupcfg;
	dst = (uint32 *) header_value;

	for (i = 0; i < sizeof(struct nvram_header); i += 4)
		*dst++ = *src++;

	for (; i < header_value ->len && i < NVRAM_SPACE; i += 4)
		*dst++ = ltoh32(*src++);

	/* Parse and set "name=value\0 ... \0\0" */
	name = (char *) &header_value[1];
	end = (char *) header_value + NVRAM_SPACE - 2;
	end[0] = end[1] = '\0';
	for (; *name; name = value + strlen(value) + 1) 
	{
		if (!(eq = strchr(name, '=')))
			break;
		*eq = '\0';
		value = eq + 1;
		_nvram_set(name, value);
		*eq = '=';
	}

	/* Set special SDRAM parameters */
	if (!_nvram_get("sdram_init")) {
		sprintf(buf, "0x%04X", (uint16)(header_backupcfg->crc_ver_init >> 16));
		_nvram_set("sdram_init", buf);
	}
	if (!_nvram_get("sdram_config")) {
		sprintf(buf, "0x%04X", (uint16)(header_backupcfg->config_refresh & 0xffff));
		_nvram_set("sdram_config", buf);
	}
	if (!_nvram_get("sdram_refresh")) {
		sprintf(buf, "0x%04X", (uint16)((header_backupcfg->config_refresh >> 16) & 0xffff));
		_nvram_set("sdram_refresh", buf);
	}
	if (!_nvram_get("sdram_ncdl")) {
		sprintf(buf, "0x%08X", header_backupcfg->config_ncdl);
		_nvram_set("sdram_ncdl", buf);
	}

	printf("\n=============restore!\n");

	free(header_value);
	
	return 0;
}
/************************************************************
Function:	 checkIfNeedRestorecfg                //函数名
Description: 检测是否需要恢复配置         	          //函数功能描述
Input:      
	无                                    //对输入值说明，包括每个
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	需要返回1
	不需要返回0
Others:                                         //其他说明
************************************************************/
int checkIfNeedRestorecfg(void)
{
	struct nvram_header *buffer;
	uint off_cfg;
	unsigned int flash_size;
	uint32 new_crc_value= 0;
	unsigned char old_crc_valua;
	
	if (sysFlashInit(NULL)) 
	{
		diag_printf("%s: error initializing flash\n", __func__);
		return -1;
	}
	flash_size=get_flashsize();
	if(flash_size ==0 )
	{
		printf("get flashsize error");
		return -1;
	}
	if (!(buffer =  (struct nvram_header *)malloc( NVRAM_SPACE )) )
	{
		printf("nvram_init: out of memory\n");
		return -12; /* -ENOMEM */
	}
	
	
	off_cfg=flash_size-NVRAM_SPACE;
	memset(buffer,0,sizeof(buffer));
	NVRAM_LOCK();
	sysFlashRead(off_cfg ,(uchar*)buffer,NVRAM_SPACE);
	NVRAM_UNLOCK();
	new_crc_value= nvram_calc_crc_value(buffer);
	old_crc_valua=buffer->crc_ver_init & 0x000000FF;
	if(old_crc_valua == new_crc_value )
	{
		free(buffer);
		return 0;
	}
	free(buffer);

	return 1;
}

/************************************************************
Function:	 checkIfNeedBackupcfg                //函数名
Description: 检测是否需要备份配置         	          //函数功能描述
Input:      
	无                                    //对输入值说明，包括每个
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	需要返回1
	不需要返回0
Others:                                         //其他说明
************************************************************/
int checkIfNeedBackupcfg(void)
{
	unsigned char backcfg_header[32],cfg_header[32];
	int backcfg_header_len,cfg_header_len;
	uint off_cfg,off_backupcfg;
	unsigned int flash_size;

	if (sysFlashInit(NULL)) 
	{
		diag_printf("%s: error initializing flash\n", __func__);
		return -1;
	}
	flash_size=get_flashsize();
	if(flash_size ==0 )
	{
		printf("get flashsize error");
		return -1;
	}
	
	off_cfg=flash_size-NVRAM_SPACE;
	off_backupcfg=flash_size - 2*NVRAM_SPACE - 2*NVRAM_SPACE;
	
	memset(backcfg_header,0,sizeof(backcfg_header));
	memset(cfg_header,0,sizeof(cfg_header));
	NVRAM_LOCK();

	backcfg_header_len = sysFlashRead(off_backupcfg,(uchar*)backcfg_header,CFG_HEADER_LEN);

	cfg_header_len =  sysFlashRead(off_cfg,(uchar*)cfg_header,CFG_HEADER_LEN);
	NVRAM_UNLOCK();

	if ( backcfg_header_len!=CFG_HEADER_LEN || cfg_header_len!=CFG_HEADER_LEN )
	{
		return -1;
	}
	
	if ( memcmp(backcfg_header,cfg_header,CFG_HEADER_LEN)!=0 )
	{
		return 1;
	}

	return 0;
}
/************************************************************
Function:	 backupTimer                //函数名
Description: 配置备份,系统随机检测配置区和备份区数据
是否一样，否则备份数据，如果检测到备份标示backupcfgTag 
为1时备份数据							  //函数功能描述
Input:      
	无                                    //对输入值说明，包括每个
Output:                                         //对输出参数说明
	无
Return:                                         //函数返回值说明
	
Others:                                         //其他说明
************************************************************/
void backupTimer()
{

	if (backupTag ==1)
	{
		if(  1==checkIfNeedBackupcfg() )
		{
			backupConfig();
			printf("\n==>backup Config sucess!!\n");
		}
		
		backupTag =0;
	}
	return;
	
}


