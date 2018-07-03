

/*
* cgi.c -- CGI processing (for the GoAhead Web server
*
* Copyright (c) GoAhead Software Inc., 1995-2000. All Rights Reserved.
*
* See the file "license.txt" for usage and redistribution license requirements
*
* $Id: //depot/MRVL/main/firmware/AP5x/prod/ap/ap5x/sdk/http/src/upload.c#1 $
*/

/********************************** Description *******************************/
/*
*	This module implements the /cgi-bin handler. CGI processing differs from
*	goforms processing in that each CGI request is executed as a separate 
*	 process, rather than within the webserver process. For each CGI request the
*	environment of the new process must be set to include all the CGI variables
*	and its standard input and output must be directed to the socket.  This
*	is done using temporary files.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <trxhdr.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <flashutl.h>
#include <flash.h>

#include <bcmnvram.h>
#include <osl.h>
#include <siutils.h>

#include  "uemf.h"
#include  "wsIntrn.h"

#include "sys_module.h"
#include "msg.h"

#ifndef	MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#endif
#define FLASH_BLOCK_MAX 512
#define FLASH_BLOCK_COUNT 2048
#define FLASH_ALL_MAX (FLASH_BLOCK_MAX*FLASH_BLOCK_COUNT)

//llm add
#ifdef REALTEK
extern int tenda_get_flash_size(void);

#define RTK_FLASH_SIZE tenda_get_flash_size()	//flash大小
/* lq修改flash分区**/

#define RTK_BOOT_SIZE (120*1024)	//BOOT区大小
#define RTK_ENVRAM_SIZE (8*1024)	//envram大小
#define RTK_NVRAM_SISZ (32*1024)	//nvram大小，还没有备份
#define RTK_TRX_SIZE (RTK_FLASH_SIZE-RTK_BOOT_SIZE-RTK_ENVRAM_SIZE-RTK_NVRAM_SISZ)	//升级文件大小
#define TRX_MAX_LEN RTK_TRX_SIZE



/* Reverse the bytes in a 32-bit value */
#define SWAP32(val) \
	((uint32)((((uint32)(val) & (uint32)0x000000ffU) << 24) | \
		  (((uint32)(val) & (uint32)0x0000ff00U) <<  8) | \
		  (((uint32)(val) & (uint32)0x00ff0000U) >>  8) | \
		  (((uint32)(val) & (uint32)0xff000000U) >> 24)))
#define swap32(val) ({ \
		uint32 _val = (val); \
		SWAP32(_val); \
	})
//大小端导致判断出错
//#define	ltoh32(i) swap32(i)
#define	ltoh32(i) (i)
/*-----------------------------------------------------------------------*/
/* crc defines , form bcmutils.c, llm add */
#define CRC32_INIT_VALUE 0xffffffff /* Initial CRC32 checksum value */
#define CRC32_GOOD_VALUE 0xdebb20e3 /* Good final CRC32 checksum value */

#define CRC_INNER_LOOP(n, c, x) \
	(c) = ((c) >> 8) ^ crc##n##_table[((c) ^ (x)) & 0xff]

#define UPLOAD_SUCCESSFUL   100
#define UPLOAD_FALI   202
#define UPLOAD_FILE_FAIL   201

static const uint32 crc32_table[256] = {
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

/*
 * crc input is CRC32_INIT_VALUE for a fresh start, or previous return value if
 * accumulating over multiple pieces.
 */
uint32
hndcrc32(uint8 *pdata, uint nbytes, uint32 crc)
{
	uint8 *pend;
#ifdef __mips__
	uint8 tmp[4];
	ulong *tptr = (ulong *)tmp;

	if (nbytes > 3) {
		/* in case the beginning of the buffer isn't aligned */
		pend = (uint8 *)((uint)(pdata + 3) & ~0x3);
		nbytes -= (pend - pdata);
		while (pdata < pend)
			CRC_INNER_LOOP(32, crc, *pdata++);
	}

	if (nbytes > 3) {
		/* handle bulk of data as 32-bit words */
		pend = pdata + (nbytes & ~0x3);
		while (pdata < pend) {
			*tptr = *(ulong *)pdata;
			pdata += sizeof(ulong *);
			CRC_INNER_LOOP(32, crc, tmp[0]);
			CRC_INNER_LOOP(32, crc, tmp[1]);
			CRC_INNER_LOOP(32, crc, tmp[2]);
			CRC_INNER_LOOP(32, crc, tmp[3]);
		}
	}

	/* 1-3 bytes at end of buffer */
	pend = pdata + (nbytes & 0x03);
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#else
	pend = pdata + nbytes;
	while (pdata < pend)
		CRC_INNER_LOOP(32, crc, *pdata++);
#endif /* __mips__ */

	return crc;
}
/*-------------------------------------------------------------------------*/

/*
	tenda_flash_这两个函数return 1表示成功，return 0 表示失败
	sysFlashRead这个返回写的字节数
	sysFlashWrite成功返回1，失败返回0
*/
extern int tenda_flash_read(char *buff,int offset, int len);
extern int tenda_flash_write(char *buff,int offset, int len);

/*替换掉这两个函数*/
//#define sysFlashRead(off, buf, len) tenda_flash_read(buf, off, len)
//#define sysFlashWrite(off, buf, len) tenda_flash_write(buf, off, len)

#define sysFlashRead(off, buf, len) upgrade_read_flash(off, buf, len)
#define sysFlashWrite(off, buf, len) upgrade_write_flash(off, buf, len)


#endif
#if 1	/*写失败时重试可以搞搞*/
int upgrade_read_flash(int offset, char *buff, int len)
{
#ifdef BCM
	return sysFlashRead(offset, buff, len);
#endif

#ifdef REALTEK
	return tenda_flash_read(buff, offset, len);
#endif	
}

int upgrade_write_flash(int offset, char *buff, int len)
{
#ifdef BCM
	return sysFlashWrite(offset, buff, len);
#endif
	
#ifdef REALTEK
	/*check*/
	if(offset < (RTK_BOOT_SIZE + RTK_ENVRAM_SIZE))
	{
		printf("\n@@YOU WILL WRITE BOOT! REJUCT!@@\n");
		return 0;
	}
	return tenda_flash_write(buff, offset, len);
#endif	
}
#endif
extern void sys_reboot(void);
extern int CFG_read_prof(char * file, int max);
extern int CFG_write_prof(char * file, int size);
static void SetErrorArg(char *msg,char *url);

void upgrade(webs_t wp, char_t *cgiName, char_t *query);
void DownloadCfg(webs_t wp, char_t *path, char_t *query);
void UploadCfg(webs_t wp, char_t *path, char_t *query);
void DownloadSPIFlash(webs_t wp, char_t *path, char_t *query);

int webCgiGetUploadFile(webs_t wp, char_t *fileBuf);

#define ERASE_AND_WRITE_BLOCK	32*1024	//MIN FLASH ERASE BLOCK
//#define FLASH_FW_SIZE			(1024-128-32)*1024//BOOT=128K, NVRAM=32K


#define WRITE_FW_DEBUG 1
#ifdef WRITE_FW_DEBUG	
static int l = 0;
//roy modify+++
static char gWLErrorMsg[64] = {0};
static char gWLErrorRtnUrl[64] = {0};
static void SetErrorArg(char *msg,char *url)
{
	memset(gWLErrorMsg,0,sizeof(gWLErrorMsg));
	memset(gWLErrorRtnUrl,0,sizeof(gWLErrorRtnUrl));
	strcpy(gWLErrorMsg,msg);
	strcpy(gWLErrorRtnUrl,url);
}
static int printf_hex(int off, unsigned char *buf, int len)
{
	int i = 0;
	for(i=0;i<16;i++)
	{
		if((l++)%16==0) {
			diag_printf("\n");
		}
		diag_printf("%02x ", *(buf+i));
	}
	
	return 0;
}
#endif


static int upload_fw_now = 0;

int get_upload_fw_flag(void)
{	
	return upload_fw_now;
}
//roy modify+++
int tenda_upload_fw(char *stream, int offset_in,int flash_offset)
{
	struct trx_header *trx,trx1;
	struct upgrade_mem *up_buf,*p;
	
	
	unsigned char *wb_buf, *rd_buf = NULL;
	
	wb_buf = (unsigned char *)malloc(ERASE_AND_WRITE_BLOCK);
	if(!wb_buf)
		return -1;
#ifdef WRITE_FW_DEBUG	
	rd_buf = (unsigned char *)malloc(ERASE_AND_WRITE_BLOCK);
	if(!rd_buf){
		free(wb_buf);
		return -1;
	}
#endif
	
	char *pstr = NULL;
	int rem_len = 0, buf_len = 0, wt_len=0;

	int ok = 0;
	int buflen,buflen_write;

	char *stream2;

	upload_fw_now = 1;
	
	up_buf = (struct upgrade_mem *)stream;

	if(!up_buf)
	{
		free(wb_buf);
		free(rd_buf);
		return -1;
	}
	stream2 = up_buf->data+offset_in;
	
	diag_printf("[%c][%c][%c][%c][%c] Write to flash, please wait...\n",
		stream2[0],stream2[1],stream2[2],stream2[3],stream2[4]);

	memcpy((char *)&trx1, stream2, sizeof(trx1));
	trx = &trx1;

	buflen = ltoh32(trx->len);


	/* Write this chunk to flash */
	p = up_buf;
	pstr = p->data + offset_in;
	buf_len = p->inuse - offset_in;
	rem_len==0;
	
	cyg_scheduler_lock();

	int flash_size = 0;
#ifdef BCM
	flash_size = get_flashsize();
	flash_size = flash_size - 128*1024 - 32*1024; //BOOT=128K, NVRAM=32K
	printf (" %s %d flash_size=[ %d ]\n", __FUNCTION__, __LINE__, flash_size);
#endif
#ifdef REALTEK	//llm add，这命名有点歧义，实际意义为升级文件分区大小
	flash_size = RTK_TRX_SIZE;
#endif

	if(buflen > flash_size){
		printf (" %s %d trx is too large!\n", __FUNCTION__, __LINE__);
		ok = -1;
		goto write_done;
	}
	
	while( p )
	{
		if(pstr == NULL){
			if(p->next == NULL){
				break;
			}
			p = p->next;
			buf_len = p->inuse;
			pstr = p->data;
		}
		
		if(rem_len==0){
			memset(wb_buf, 0x0, ERASE_AND_WRITE_BLOCK);
			wt_len = ((buf_len >= ERASE_AND_WRITE_BLOCK)?ERASE_AND_WRITE_BLOCK:buf_len);
			memcpy(wb_buf, pstr, wt_len);
			if(buf_len==ERASE_AND_WRITE_BLOCK){
				rem_len = 0;
				buf_len = 0;
				pstr = NULL;
			}else if(buf_len<ERASE_AND_WRITE_BLOCK){
				rem_len = wt_len;
				buf_len = 0;
				pstr = NULL;
			}else{
				rem_len = 0;
				buf_len -= ERASE_AND_WRITE_BLOCK;
				pstr += ERASE_AND_WRITE_BLOCK;
			}
		}else{
			wt_len = ((buf_len >= (ERASE_AND_WRITE_BLOCK-rem_len))?(ERASE_AND_WRITE_BLOCK-rem_len):buf_len);
			memcpy(wb_buf+rem_len, pstr, wt_len);
			if(buf_len==(ERASE_AND_WRITE_BLOCK-rem_len)){
				rem_len = 0;
				buf_len = 0;
				pstr = NULL;
			}
			else if(buf_len<(ERASE_AND_WRITE_BLOCK-rem_len)){
				rem_len += wt_len;
				buf_len = 0;
				pstr = NULL;
			}else{
				rem_len = 0;
				buf_len -= wt_len;
				pstr += wt_len;
			}
		}
		
		if(rem_len == 0 || p->next == NULL)//wt_len==ERASE_AND_WRITE_BLOCK
		{
			diag_printf("\nflash_offset=%08x, wt_len=%d", flash_offset, ERASE_AND_WRITE_BLOCK);
			//sysFlashErase(flash_offset, ERASE_AND_WRITE_BLOCK);
			if (!sysFlashWrite(flash_offset, wb_buf,  ERASE_AND_WRITE_BLOCK)) {
				ok = -1;
				goto write_done;
			}
#ifdef WRITE_FW_DEBUG	
			memset(rd_buf, 0x0, ERASE_AND_WRITE_BLOCK);
			sysFlashRead(flash_offset, rd_buf,  ERASE_AND_WRITE_BLOCK);
			if(memcmp(rd_buf, wb_buf, ERASE_AND_WRITE_BLOCK)!=0)
			{
				diag_printf("flash_offset wb=%08x\n", flash_offset);
				printf_hex(flash_offset, wb_buf, ERASE_AND_WRITE_BLOCK);
				diag_printf("flash_offset rd=%08x\n", flash_offset);
				printf_hex(flash_offset, rd_buf, ERASE_AND_WRITE_BLOCK);
			}
#endif
			rem_len = 0;
			flash_offset += ERASE_AND_WRITE_BLOCK;
		}
	}

write_done:
	
	cyg_scheduler_unlock();
	upload_fw_now = 0;
	
	if(wb_buf)
		free(wb_buf);
	if(rd_buf)
		free(rd_buf);
		
	return ok;
}

/***********************************************************************/

#define BOUNDRY_SIZE_BUF   64
#define DASH                '-'
#define LINE_END0	0x0d
#define LINE_END1	0x0a

int do_upgrade_check(char *stream, int offset_in, int *flash_offset)
{
	struct trx_header *trx,trx1;
	struct trx_header flash;
	uint32 bisz[BISZ_SIZE];
	unsigned int offset;
	int ok = 0;
	unsigned long crc;
	unsigned long buflen,buflen_checked;

	struct upgrade_mem *up_buf,*p;

	char *stream2;

	up_buf = (struct upgrade_mem *)stream;

	stream2 = up_buf->data+offset_in;

	diag_printf("[%c][%c][%c][%c][%c]\n",
		stream2[0],stream2[1],stream2[2],stream2[3],stream2[4]);
	memcpy((char *)&trx1, stream2, sizeof(trx1));
	trx = &trx1;

#if 0	
	trx = (struct trx_header *)stream;
#endif	
	
	/* check header */
	if (ltoh32(trx->magic) != TRX_MAGIC ||
	    ltoh32(trx->len) > TRX_MAX_LEN ||
	    ltoh32(trx->len) < sizeof(struct trx_header)) {
			diag_printf("%s: Bad header (magic 0x%x len %08lx)\n",
					__func__, ltoh32(trx->magic), ltoh32(trx->len));
		ok = -3;
		goto fail;
	}

	buflen = ltoh32(trx->len);
	diag_printf("trx->len: %x\n", ltoh32(trx->len));
	p = up_buf;
	buflen_checked = 0;
	
	//the oldway
	//crc = hndcrc32((unsigned char *)stream+12, buflen-12, CRC32_INIT_VALUE);

	//new way,we have many pieces of buf
	//the first node
	buflen_checked = p->inuse-(offset_in+12);
	buflen -= 12;// no inlcude trx head
	if(buflen_checked > buflen){
		//only have one node, see webs.c
		buflen_checked = buflen;
	}

	diag_printf("printf upgrade file hex:\n");
	int i;
	for(i = 0; i < 12; i++)
		printf("%02x ", *(unsigned char*)(p->data+offset_in+i));
	printf("\n");
	
	crc = hndcrc32(p->data+offset_in+12,buflen_checked, CRC32_INIT_VALUE);
	buflen -= buflen_checked;
	p = p->next;
		
	while((buflen > 0) && p){
		buflen_checked = (buflen >= p->inuse)?p->inuse:buflen;
		crc = hndcrc32(p->data,buflen_checked, crc);

		buflen -= buflen_checked;
		p = p->next;
	}

	/* Check CRC before writing if possible */
	if (crc != ltoh32(trx->crc32)) {
		printf("http_put_file: Bad CRC\n");
		printf("crc=%08lx trx.crc=%08lx\n", crc, (unsigned long)ltoh32(trx->crc32));
		ok = -3;
		goto fail;
	}
/*lrl 不同产品添加魔术字互斥*/	
#ifdef __CONFIG_FW_ID__
	if (0 == (ltoh32(trx->flag_version) & __CONFIG_FW_ID__))
#elif defined __CONFIG_WEB_VERSION__
	/*lq 添加版本互斥*/
	if ((0 == (ltoh32(trx->flag_version) & TRX_CH_IMAGE_TAG)) && !strcmp("cn",__CONFIG_WEB_VERSION__))
#endif
	{
		printf (" %s %d  TRX_IMAGE_TAG  error!  \n", __FUNCTION__, __LINE__);
		ok = -3;
		goto fail;
	}
/* llm add, REALTEK中升级文件头并没有写入flash，因此不检查flash上的头 */
#ifdef BCM
	if (sysFlashInit(NULL)) {
		diag_printf("%s: error initializing flash\n", __func__);
		ok = -3;
		goto fail;
	}

	/* Figure out where the os starts in flash, assume it is at 256KB */
	offset = 256 * 1024;
	/* Set to 128KB if the bootloader is smaller */
	if (sysFlashRead(BISZ_OFFSET, (uchar *)bisz, sizeof(bisz)) == sizeof(bisz) &&
	    bisz[BISZ_MAGIC_IDX] == BISZ_MAGIC &&
	    bisz[BISZ_DATAEND_IDX] - bisz[BISZ_TXTST_IDX] < 128 * 1024) {
		offset = 128 * 1024;
	}

	/* Check it is indeed a TRX at the offset after the bootloader */
	if (sysFlashRead(offset, (uchar *)&flash, sizeof(flash)) != sizeof(flash) ||
	    ltoh32(flash.magic) != TRX_MAGIC) {
		diag_printf("%s: unable to find os at offset %x in flash\n", __func__,
			offset);
		ok = -3;
		goto fail;
	}

	*flash_offset = offset;
#endif

#ifdef REALTEK
	*flash_offset = RTK_BOOT_SIZE + RTK_ENVRAM_SIZE;
#endif

	diag_printf("%s: Upgrade file offset %x in flash\n", __func__, *flash_offset);
fail:
	return (ok);
}

//+++

int webDoCgi(webs_t wp, char_t* query, char_t *cgiName)
{
	
	diag_printf("Enter webDoCgi fun cgiName = %s clen =%ld ,query =%s\n",cgiName,wp->clen2,query);

	if(strncmp(cgiName, "upgrade", 7) == 0)
	{
		upgrade(wp, cgiName, query);
	}
	else if (strncmp(cgiName, "UploadCfg", 9) == 0)
	{
		UploadCfg(wp,cgiName,query);
	}	
	else if (strncmp(cgiName, "DownloadCfg", 11) == 0)
	{
		DownloadCfg(wp,cgiName,query);
	}
	else if (strncmp(cgiName, "DownloadFlash", 13) == 0)
	{
		DownloadSPIFlash(wp,cgiName,query);
	}
	/*pxy add 2013.06.14*/
	else if (strncmp(cgiName, "DownloadSyslog", 14) == 0)
	{
		DownloadSyslog(wp,cgiName,query);
	}
	else if (strncmp(cgiName, "exportSysLog", 12) == 0)
	{
		DownloadSyslog(wp,cgiName,query);
	}
	
	return 0;
}

int webs_Tenda_CGI_BIN_Handler(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg, 
						  char_t *url, char_t *path, char_t* query)
{
 	char_t		cgiBuf[FNAMESIZE];
	char_t		*cp, *cgiName;

	a_assert(websValid(wp));
	a_assert(url && *url);
	a_assert(path && *path == '/');
	websStats.cgiHits++;
	/*
	*	Extract the form name and then build the full path name.  The form
	*	name will follow the first '/' in path.
	*/
	gstrncpy(cgiBuf, path, TSZ(cgiBuf));
	if ((cgiName = gstrchr(&cgiBuf[1], '/')) == NULL) {
		websError(wp, 200, T("Missing CGI name"));
		return 1;
	}
	cgiName++;
	if ((cp = gstrchr(cgiName, '/')) != NULL) {
		*cp = '\0';
	}
#if 0 //roy modify
	fmtAlloc(&cgiPath, FNAMESIZE, T("%s/%s/%s"), websGetDefaultDir(),
		CGI_BIN, cgiName);
#endif
	webDoCgi(wp, query, cgiName);

	return 1;
}

int webCgiGetUploadFile(webs_t wp, char_t *fileBuf)
{
	char bufBoundry[BOUNDRY_SIZE_BUF];
	int count;
	int i;
	int skip=4;
	int curlen;
	char_t *pCurMem = NULL;

	struct upgrade_mem *up_buf;

/*
google ie start:
------WebKitFormBoundary2gjb0eP8DnLkSEWe
Content-Disposition: form-data; name="fileCfg"; filename="RouterCfm.cfg"

#PROFILE
*/
/*
microsoft ie start:
-----------------------------7da232f90c58
Content-Disposition: form-data; name="fileCfg"; filename="C:\Documents and Settings\user\\RouterCfm.cfg"
Content-Type: text/plain

#PROFILE
*/
//roy add,2011/05/26
	if(fileBuf == NULL || wp->query == NULL)
		return -1;
//roy modify
	up_buf =  (struct upgrade_mem *)fileBuf;
	pCurMem = up_buf->data;
	//pCurMem = fileBuf;
	curlen = up_buf->inuse;
	count = 0;
#if 0
	while(*pCurMem == DASH) {
		pCurMem++;
		//diag_printf("[%c]",*pCurMem);
		count++;
	}
	i = 0;
	while(*pCurMem != LINE_END0){
		bufBoundry[i] = *pCurMem;
		//diag_printf("[%c]",*pCurMem);
		pCurMem++;
		i++;
		count++;
	}
	bufBoundry[i] = '\0';
	if(strstr(bufBoundry,"WebKitFormBoundary") != NULL){
		skip = 3;
	}

	while(skip){
		if(*pCurMem == LINE_END1)  skip--;
		//diag_printf("[%c]",*pCurMem);
		pCurMem++;
		count++;
	}
#else
	for (i = 0; i < curlen; i++) {
		/* Search end of headers */
		if((i+4) >= curlen)
			break;
		if (*(pCurMem+0) == LINE_END0 && *(pCurMem+1) == LINE_END1 &&
		    *(pCurMem+2) == LINE_END0 && *(pCurMem+3) == LINE_END1) {
			count += 4;
			break;
		}
		count++;
		pCurMem++;
	}
#endif
	printf("%c %c %c %c\n", (up_buf->data)[0], (up_buf->data)[1],(up_buf->data)[2],(up_buf->data)[3]);

	printf("%c %c %c %c\n", (up_buf->data)[count], (up_buf->data)[count+1],(up_buf->data)[count+2],(up_buf->data)[count+3]);
	return count;
}

int webCgiGetUploadFile2(webs_t wp, char_t *fileBuf)
{
	char bufBoundry[BOUNDRY_SIZE_BUF];
	int count;
	int i;
	int skip=4;
	int curlen;
	char_t *pCurMem = NULL;

/*
google ie start:
------WebKitFormBoundary2gjb0eP8DnLkSEWe
Content-Disposition: form-data; name="fileCfg"; filename="RouterCfm.cfg"

#PROFILE
*/
/*
microsoft ie start:
-----------------------------7da232f90c58
Content-Disposition: form-data; name="fileCfg"; filename="C:\Documents and Settings\user\\RouterCfm.cfg"
Content-Type: text/plain

#PROFILE
*/
//roy add,2011/05/26
	if(fileBuf == NULL || wp->query == NULL)
		return -1;

	pCurMem = fileBuf;
	curlen = wp->clen2;
	count = 0;
#if 0	
	while(*pCurMem == DASH) {
		pCurMem++;
		//diag_printf("[%c]",*pCurMem);
		count++;
	}
	i = 0;
	while(*pCurMem != LINE_END0){
		bufBoundry[i] = *pCurMem;
		//diag_printf("[%c]",*pCurMem);
		pCurMem++;
		i++;
		count++;
	}
	bufBoundry[i] = '\0';
	if(strstr(bufBoundry,"WebKitFormBoundary") != NULL){
		skip = 3;
	}

	while(skip){
		if(*pCurMem == LINE_END1)  skip--;
		//diag_printf("[%c]",*pCurMem);
		pCurMem++;
		count++;
	}
#else
	for (i = 0; i < curlen; i++) {
		/* Search end of headers */
		if((i+4) >= curlen)
			break;
		if (*(pCurMem+0) == LINE_END0 && *(pCurMem+1) == LINE_END1 &&
		    *(pCurMem+2) == LINE_END0 && *(pCurMem+3) == LINE_END1) {
			count += 4;
			break;
		}
		count++;
		pCurMem++;
	}
#endif
	return count;

	return 0;
}

#define UPGRADE_SUCCESSFUL   100  //升级成功
#define UPGRADE_FALI   202  //升级失败
#define UPGRADE_FILE_FAIL   201   //升级文件错误
#define UPGRADE_FILE_BIG_FAIL   203   //升级文件过大


extern void upgrade_mem_free(char *head);
extern void tapf_watchdog_disable(void);

void upgrade(webs_t wp, char_t *cgiName, char_t *query)
{

	int offset,flash_offset;
	char ret_buf[32] = {0};
	int ret = UPGRADE_SUCCESSFUL; 

	diag_printf("Start upgrade...\n");
	tapf_watchdog_disable();		
	if( (offset = webCgiGetUploadFile(wp, query) ) <= 0 || offset >=wp->clen2) 
	{
		
		SetErrorArg("Get upgrade file failed!","direct_reboot.asp");
		ret = UPGRADE_FILE_BIG_FAIL;  
		goto upgrade_reboot;
	}
	flash_offset = 0;

	if(do_upgrade_check(query,offset,&flash_offset)<0){
		
		ret = UPGRADE_FILE_FAIL;
		SetErrorArg("Check upgrade file error!","direct_reboot.asp");
		goto upgrade_reboot;
	}
upgrade_reboot:

	//提前返回页面升级结果
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	sprintf(ret_buf, "{\"errCode\":\"%d\"}", ret );
	websWrite(wp,T("%s"),ret_buf);
	websDone(wp, 200);

#ifdef REALTEK
	/*realtek 升级跳过头, llm add*/
	offset += sizeof(struct trx_header);
#endif

	if(ret==UPGRADE_SUCCESSFUL)
	{
		if(tenda_upload_fw(query,offset,flash_offset) < 0)
		{	
			ret = UPGRADE_FALI;
			SetErrorArg("Write upgrade file error!","direct_reboot.asp");
		}
		else 
		{	
			diag_printf("Upgrade success...");
		}
	}
	if( query) 
	{
		upgrade_mem_free(query);
		wp->query = NULL;
	}
	//sys_reboot();
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=reboot");
	return;
}

/*
 *	download system log to the user's PC
 *	pxy add 2013.06.14
 */
extern int syslog_open();
extern int syslog_close();
extern char *syslog_get();

static char systips[]="#This file shows the router's system log.\r\n";

void DownloadSyslog(webs_t wp, char_t *path, char_t *query)
{
	int wrote = 0,MibLen;//, err;
	char_t* date;
	char line[256], *val;
	char *name = NULL;
	int num = 1;

	diag_printf("\nDownloadSyslog:\npath:%s;query: %s\n",path,query);


	if ((date = websGetDateString(NULL)) != NULL) 
	{
		websWrite(wp, T("HTTP/1.0 200 OK\r\nDate: %s\r\n"), date);
		bfree(B_L, date);
	}

	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);

	if ((date = websGetDateString(NULL)) != NULL) {
		websWrite(wp, T("Last-modified: %s\r\n"), date);
		bfree(B_L, date);
	}

	char log_time[32],log_type[32],log_data[128];
	char buf[256];

	syslog_open();
	
	while(syslog_get2(log_time,log_type,log_data)!=0){
		
		sprintf(buf,"%03d %20s %16s %s",num,log_time,log_type,log_data);
		wrote += strlen(buf);
		wrote += 2;
		num++;
	}

	syslog_close();

	websWrite(wp, T("Content-length: %d\r\n"), wrote + strlen(systips));
	websSetRequestBytes(wp, wrote + strlen(systips));
	websWrite(wp, T("Content-type: config/conf\r\n"));
	websWrite(wp, T("Connection: close\r\n"));
	websWrite(wp, T("\r\n"));

	num = 1;
	wrote = 0;

	wrote += websWriteDataNonBlock(wp, systips, strlen(systips));


	syslog_open();
	
	while(syslog_get2(log_time,log_type,log_data)!=0){
		
		sprintf(buf,"%03d %20s %16s %s",num,log_time,log_type,log_data);
		wrote += websWriteDataNonBlock(wp, buf, strlen(buf));
		wrote += websWriteDataNonBlock(wp, "\r\n", 2);
		num++;
	}

	syslog_close();

	websSetRequestWritten(wp, wrote);
	websDone(wp, 200);

}

/*****************************************************************************
 函 数 名  : calculate_nvram
 功能描述  : 计算导出配置校验值
 输入参数  : char* pMibBuf  
 输出参数  : 无
 返 回 值  : 
 
 修改历史      :
  1.日    期   : 2016年7月20日
    作    者   : liquan
    修改内容   : 新生成函数

*****************************************************************************/
int calculate_nvram(char* pMibBuf)
{
	char *name = NULL;
	char *temp = NULL;
	int  count = 0;
	unsigned int  cal_num = 0;
	int i = 0;
	
	for(name = pMibBuf; *name; name += strlen(name) + 1)
	{
		temp= name;
		count = 0;
		i = 0;
		while(i < strlen(name))
		{
			count +=*(temp+i);
			i++;
		}
		cal_num += count;
	}
	
	return cal_num;

}

static char tips[]="#Please don't change this file by hand\r\n";

void DownloadCfg(webs_t wp, char_t *path, char_t *query)
{
	int wrote,MibLen;//, err;
	char_t* date;
	char *pMibBuf;
	char line[256], *val;
	char *name = NULL;
	int crc = 0;
	
	diag_printf("\nDownloadCfg:\npath:%s;query: %s\n",path,query);

	pMibBuf = (char *)malloc(NVRAM_SPACE);
	
	if(pMibBuf == NULL)
	{
		return;
	}

	memset (pMibBuf,0,NVRAM_SPACE);
	
	MibLen = CFG_read_prof(pMibBuf, NVRAM_SPACE);

	if ((date = websGetDateString(NULL)) != NULL) 
	{
		websWrite(wp, T("HTTP/1.0 200 OK\r\nDate: %s\r\n"), date);
		bfree(B_L, date);
	}

	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);

	if ((date = websGetDateString(NULL)) != NULL) {
		websWrite(wp, T("Last-modified: %s\r\n"), date);
		bfree(B_L, date);
	}
	
	crc = calculate_nvram(pMibBuf);
	memset(line,0,sizeof(line));
	sprintf(line,"@%d\r\n",crc);
	MibLen += strlen(line) + strlen(tips);
	
	websWrite(wp, T("Content-length: %d\r\n"), MibLen);
	websSetRequestBytes(wp, MibLen);
	websWrite(wp, T("Content-type: config/conf\r\n"));
	websWrite(wp, T("Connection: close\r\n"));
	//websWrite(wp, T("Content-type: %s\r\n"), websGetRequestType(wp));

	websWrite(wp, T("\r\n"));

	wrote = 0;

	wrote += websWriteDataNonBlock(wp, tips, strlen(tips));

	wrote += websWriteDataNonBlock(wp, line, strlen(line));

	for(name = pMibBuf; *name; name += strlen(name) + 1){
		//filter, only give the  data in router_defaults
		memset(line,0,sizeof(line));
		strncpy(line,name,sizeof(line));
		val = strchr((const char*)line, '=');
		if(!val)
			continue;
		/* cut the var name */
		*val = 0;

		//printf("%s\n",name);

		wrote += websWriteDataNonBlock(wp, name, strlen(name));
		wrote += websWriteDataNonBlock(wp, "\r\n", 2);
	}

	websSetRequestWritten(wp, wrote);
	websDone(wp, 200);

	free(pMibBuf);
}

void UploadCfg(webs_t wp, char_t *path, char_t *query)
{	
	char_t *pBufStart;
	int offset,flash_offset,ok,size,ret = UPLOAD_SUCCESSFUL;
	char ret_buf[PI_BUFLEN_32] = {0};

	if( (offset = webCgiGetUploadFile2(wp, query) ) <= 0 || offset >=wp->clen2) 
	{
		ret = UPLOAD_FILE_FAIL;
		goto error_out;
	}

	
	pBufStart = query + offset;
	flash_offset = 0;
	size = wp->clen2 - offset;
	
	ok = CFG_write_prof(pBufStart, size);

	if(ok != 0){
		ret = UPLOAD_FALI;
		goto error_out;
	}

 error_out:
	websWrite(wp, T("HTTP/1.0 200 OK\r\n\r\n"));
	sprintf(ret_buf, "{\"errCode\":\"%d\"}", ret );
	websWrite(wp,T("%s"),ret_buf);
	websDone(wp, 200);
	
	msg_send(MODULE_RC,RC_SYSTOOLS_MODULE,"string_info=reboot");
	return;
}

extern  flash_desc_t  *flashutl_desc;

void DownloadSPIFlash(webs_t wp, char_t *path, char_t *query)
{
	 int i,wrote = 0;
	 int  readn;
	 char_t* date;


	unsigned char lbuf[FLASH_BLOCK_MAX];
	unsigned int flsh_cfg_base_off= 0;
	
	diag_printf("\nDownloadSPIFlashImage\n");

	if ((date = websGetDateString(NULL)) != NULL) 
	{
		websWrite(wp, T("HTTP/1.0 200 OK\r\nDate: %s\r\n"), date);
		bfree(B_L, date);
	}

	websWrite(wp, T("Server: %s\r\n"), WEBS_NAME);

	if ((date = websGetDateString(NULL)) != NULL) {
		websWrite(wp, T("Last-modified: %s\r\n"), date);
		bfree(B_L, date);
	}
#if defined(BCM)
	if (sysFlashInit(NULL)) {
		diag_printf("%s: error initializing flash\n", __func__);
		goto done;
	}

	wrote = flashutl_desc->size;
#elif defined(REALTEK)
	wrote = RTK_FLASH_SIZE;
#endif
	
	websWrite(wp, T("Content-length: %d\r\n"), wrote);
	websSetRequestBytes(wp, wrote);

	websWrite(wp, T("Content-type: config/conf\r\n"));
	websWrite(wp, T("Connection: close\r\n"));

	websWrite(wp, T("\r\n"));

	wrote = 0;
#if defined(BCM)
	for (i = 0 ;flashutl_desc->size - wrote>0; i ++ ) {
		if ((readn = sysFlashRead(flsh_cfg_base_off, lbuf, FLASH_BLOCK_MAX)) != FLASH_BLOCK_MAX) 
		{
			diag_printf("Read flash err=%d\n", readn);
			goto done ;
		}
		
		websWriteBlock(wp, lbuf, readn);
		flsh_cfg_base_off = flsh_cfg_base_off + readn;
		wrote+=readn;
	}
#elif defined(REALTEK)
	for (i = 0 ;RTK_FLASH_SIZE - wrote>0; i ++ ) {
		if ((readn = sysFlashRead(flsh_cfg_base_off, lbuf, FLASH_BLOCK_MAX)) != 1) 
		{
			diag_printf("Read flash err=%d\n", readn);
			goto done ;
		}
		websWriteBlock(wp, lbuf, FLASH_BLOCK_MAX);
		flsh_cfg_base_off = flsh_cfg_base_off + FLASH_BLOCK_MAX;
		wrote+=FLASH_BLOCK_MAX;
	}
#endif
	
 done:
	websSetRequestWritten(wp, wrote);
	websDone(wp, 200);

	printf("\nDownloadFlashImage ok ...\n");
}


