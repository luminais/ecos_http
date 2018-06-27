#ifndef __MKFLASHIMAGE_H__
#define __MKFLASHIMAGE_H__


#include "autoconf.h"



struct nvram_tuple {
	char *name;
	char *value;
	struct nvram_tuple *next;
};


extern struct nvram_tuple tenda_nvram_defaults[];
extern struct nvram_tuple tenda_envram_defaults[];
extern struct nvram_tuple tenda_factory_defaults[];

//定义需要单独添加进去的nvram变量
#define WL_SSID 	"wl_ssid"
#define EXTEND_SSID	"extend_ssid"
#define WL0_SSID	"wl0_ssid"
#define DEFAULT_SSID	"default_ssid"

#define TRACE 

#define  error_exit(reason)   do{ \
								printf("%s[%d] error happen : %s\n", __FUNCTION__, __LINE__, reason);\
								exit(-1);\
							}while(0)



#define BOOT_LENGTH_ON_FLASH (0x8000) // 32K
#define BOOT_ENVRAM_OFFSET (0x12000) // 24K ENVRAM参数放在24K到32K的位置
#define BOOT_ENVRAM_SPACE  (0x1000) // 4K




#define BOOTHEADERLEN 16			//boot头部长度
#define IMAGEHEADERLEN 28			//升级文件头部长度


#define NVRAM_SPACE		0x8000 // 32K
#define NVRAM_NUM                1000


#define BOOT_NVRAM_NUM    128

#define	ROUNDUP(x, y)		((((x)+((y)-1))/(y))*(y))

#define MB (1024 *1024)
#define MAX_PATH_LENGTH 256

char cfepath[MAX_PATH_LENGTH] = {0};
char imagepath[MAX_PATH_LENGTH] = {0};
char envrampath[MAX_PATH_LENGTH] = {0};
char flashpath[MAX_PATH_LENGTH] = {0};

char nvrampath[MAX_PATH_LENGTH] = {0};
int memsize = 0;
int flashsize = 0;		//flash的大小
char flashname[64] = {0}; 	//生成flash的名字

typedef unsigned int uint32;

struct nvram_header {
	uint32 magic;
	uint32 len;
	uint32 crc_ver_init;	/* 0:7 crc, 8:15 ver, 16:31 sdram_init */
	uint32 config_refresh;	/* 0:15 sdram_config, 16:31 sdram_refresh */
	uint32 config_ncdl;	/* ncdl values for memc */
};



struct nvram_tuple envramHead, nvramHead,factoyrnvramHead;

/*
	-c, --cfe	         <path>		    cfe path
	-i,  --image       <path>	    upgrade image path
	-n, --nvram	   <path>	          nvram parameters path	
	-e, --envram    <path>		    envram parameters path

	These down are iptional.
	-f, --flashsize	   <1|2|3|4|8>  Flash size 
	-m, --memsize   <8|16|32|64|128>	  memory size
	-v, --verbose              Dump more information to stderr

*/

int verbose = 1; //For debug, turn it on

static struct option long_options[] = {  
	{"cfe", required_argument, NULL, 'c'},  
	{"image",  required_argument, 	NULL, 'i'},    
	{"out", required_argument, NULL, 'o'},  

	{"verbose", no_argument, NULL, 'v'},	
	{"help", no_argument, NULL, 'h'},
	{0, 0, 0, 0}	
};	

static void show_usage(void)
{
	printf(
	"       usage : mkFlashImage -v -c cfez.bin -i ../ecos_lzma.trx -n nvram.txt -e envram.txt -o FlashImage\n"
	"       mkFlashImage --verbose --cfe cfez.bin --image ../ecos_lzma.trx --nvram nvram.txt --envram envram.txt --out FlashImage\n\n"
	"       -c, --cfe          <path>		    cfe path\n"
	"       -i,  --image       <path>	    upgrade image path\n"
	"       -n, --nvram       <path>	          nvram parameters path\n"	
	"       -e, --envram     <path>		    envram parameters path\n"
	"       -o, --out          <path>		    Create flash image path \n\n"	
	"       These down are iptional.\n"
	"       -f, --flashsize     <1|2|3|4|8>  Flash size \n"
	"       -m, --memsize    <8|16|32|64|128>	  memory size\n"
	"       -v, --verbose     Dump more information to stderr\n"
	"       -h, --help          show help infomation\n");

	exit(0);
}

/* Flash infomation are here. */
#define MORE_FLASH_SIZE (1*MB)

struct flash_info {
	char *name; 
	int real_size;
	
};
enum flash_size_type
{
	FLASH_1M = 1,
	FLASH_2M = 2,
	FLASH_4M ,
	FLASH_8M ,
	FLASH_16M ,	
};

struct flash_info flash_1M[] = {
	
	{"IC-GD25Q80BSIG/GD25Q80BSIGR-SO8-GD ", 1*MB},
	{"IC-W25Q80BVSSIG-SO8-WINBOND", 1*MB +0x400},
	{"IC-W25Q80DVSSIG-SO8-WINBOND ", 1*MB +0x400},		
	{"IC-MX25L8006EM2I-12G-SO8-MXIC", 1*MB +0x40},
	{"IC-KH25L8006EM2I-12G-SOP8-KH", 1*MB +0x40},
	{NULL}
};


struct flash_info flash_2M[] = {
	
	{"IC-GD25Q16BSIG/GigaDevice-SOP8", 2*MB},
	{"IC-GD25Q16CSIG/GigaDevice-SOP8", 2*MB+0x400},
	{"IC-W25Q16BVSSIG-SO8-Winbond", 2*MB},
	{"IC-W25Q16DVSSIG-SO8-WINBOND", 2*MB+0x300},
	{"IC-MX25L1605A", 2*MB},
	{"IC-S25FL216K0PMFI011-SOP8-SPASION", 2*MB},
	{"IC-KH25L1606EM2I-12G-SOP8", 2*MB+0x40},
	{NULL}
};

struct flash_info flash_4M[] = 
{
	{"IC-GD25Q32BSIG/GD25Q32BSIGR-SO8", 4*MB+0x400},
	{"IC-MX25L3206EM2I-12G-SO8", 4*MB+0x40},
	{"IC-W25Q32BV-SO8", 4*MB+0x400},
	{NULL}
};


struct flash_info flash_8M[] = 
{
	{"IC-GD25Q64BSIG/GD25Q32BSIGR-SO8", 8*MB+0x400},
	{"IC-MX25L6406EM2I-12G-S08-卷装-MXIC", 8*MB+0x40},
	{"IC-W25Q64BV-SO8", 8*MB},
	{"IC-W25Q64FV-SO8", 8*MB+0x300},
	{NULL}

};

struct flash_info flash_16M[] = 
{
	{"IC-GD25Q128BSIG/GD25Q128BSIGR-SO16-GD", 16*MB+0x400},
	{"IC-GD25Q128CSIG/GD25Q128CSIGR-SO8-GD", 16*MB+0x300},
	{"IC-W25Q128FVFIG-SO16-WINBOND", 16*MB+0x300},
	{NULL}
};

struct flash_info *allFlash[] = 
{
	flash_1M,
	flash_2M,
	flash_4M,
	flash_8M,
	flash_16M
};


#define NVRAM_CRC_START_POSITION	9 /* magic, len, crc8 to be skipped */
#define NVRAM_CRC_VER_MASK	0xffffff00 /* for crc_ver_init */
#define CRC8_INIT_VALUE  0xff       /* Initial CRC8 checksum value */
#define NVRAM_VERSION       1


typedef unsigned char uint8;

static const uint8 hnd_crc8_table[256] = {
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



#endif





























