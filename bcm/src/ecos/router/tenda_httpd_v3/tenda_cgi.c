

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
#include  "route_cfg.h"

#ifndef	MIN
#define	MIN(a,b)	(((a)<(b))?(a):(b))
#endif
#define FLASH_BLOCK_MAX 512
#define FLASH_BLOCK_COUNT 2048
#define FLASH_ALL_MAX (FLASH_BLOCK_MAX*FLASH_BLOCK_COUNT)

extern void SetErrorArg(char *msg,char *url);
extern void sys_reboot(void);
extern int CFG_read_prof(char * file, int max);
extern int CFG_write_prof(char * file, int size);
extern int find_nvram2(char *var, bool full_match);

void upgrade(webs_t wp, char_t *cgiName, char_t *query);
void DownloadCfg(webs_t wp, char_t *path, char_t *query);
void UploadCfg(webs_t wp, char_t *path, char_t *query);
void DownloadSPIFlash(webs_t wp, char_t *path, char_t *query);

int webCgiGetUploadFile(webs_t wp, char_t *fileBuf);

#define ERASE_AND_WRITE_BLOCK	32*1024	//MIN FLASH ERASE BLOCK
//#define FLASH_FW_SIZE			(1024-128-32)*1024//BOOT=128K, NVRAM=32K


//#define WRITE_FW_DEBUG 1
#ifdef WRITE_FW_DEBUG	
static int l = 0;
//roy modify+++
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
}
#endif
//roy modify+++
int tenda_upload_fw(char *stream, int offset_in,int flash_offset)
{
	struct trx_header *trx,trx1;
	struct upgrade_mem *up_buf,*p;
	
	
	unsigned char *wb_buf, *rd_buf;
	
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


	
	up_buf = (struct upgrade_mem *)stream;
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
	flash_size = get_flashsize();
	flash_size = flash_size - 128*1024 - 32*1024; //BOOT=128K, NVRAM=32K
	printf (" %s %d flash_size=%d=\n", __FUNCTION__, __LINE__, flash_size);

	if(buflen > flash_size){
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
	
	
	if(wb_buf)
		free(wb_buf);
	if(rd_buf)
		free(rd_buf);
		
	return ok;
}



/***********************************************************************/

#define BOUNDRY_SIZE_BUF   64
#define BOUNDRY_TAG_SIZE   29
#define DASH                '-'
#define LINE_END0	0x0d
#define LINE_END1	0x0a
#define LINE_FEED         '\n'
	
static int do_upgrade_check(char *stream, int offset_in, int *flash_offset)
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
			printf("%s: Bad header (magic 0x%x len %08lx)\n",
					__func__, ltoh32(trx->magic), ltoh32(trx->len));
		ok = -3;
		goto fail;
	}

	buflen = ltoh32(trx->len);
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
		printf("crc=%08lx trx.crc=%08lx\n", crc, (unsigned long)trx->crc32);
		ok = -3;
		goto fail;
	}

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

	//add by z10312 防止中文版本页面可升级其他版本 2015/07/21
	if ((ltoh32(flash.flag_version) & TRX_CH_IMAGE_TAG)  &&  !(ltoh32(trx->flag_version) & TRX_CH_IMAGE_TAG))
	{
		printf (" %s %d  TRX_CH_IMAGE_TAG  error!  \n", __FUNCTION__, __LINE__);
		ok = -3;
		goto fail;
	}
	

	*flash_offset = offset;
	
fail:
	return (ok);
}

//+++

int webDoCgi(webs_t wp, char_t* query, char_t *cgiName)
{
	
	diag_printf("Enter webDoCgi fun cgiName = %s clen =%ld\n",cgiName,wp->clen2);

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

unsigned long webCgiStripBoundry(char *pMemStart, char *pMemEnd, char *boundry_symb)
{

/*
microsoft ie end:
/r/n-----------------------------7da232f90c58--/r/n
*/
/*
google ie end:
/r/n------WebKitFormBoundary2gjb0eP8DnLkSEWe--/r/n
*/
	if(*pMemEnd == '\0')
		*pMemEnd--;/*skip the end char*/
	
	while(*pMemEnd == LINE_END1 || *pMemEnd == LINE_END0 || *pMemEnd == DASH){
		//printf("[%02x]",*pMemEnd &0xFF);
		pMemEnd--;/*strip --/r/n*/
	}
	pMemEnd -= strlen(boundry_symb);/*strip boundry*/


	while(*pMemEnd == DASH || *pMemEnd == LINE_END1)
	{
		//printf("[%02x]",*pMemEnd&0xFF);
		pMemEnd--;/*strip /n--------*/
	}
	if(*pMemEnd == LINE_END0)
		*pMemEnd = '\0';/*strip /r*/
	else
		return 0;/*wrong format*/

	if(pMemEnd > pMemStart)
		return (pMemEnd - pMemStart);
	else
		return 0;

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
}


extern void upgrade_mem_free(char *head);
extern int watchdog ;
extern si_t *bcm947xx_sih;
void upgrade(webs_t wp, char_t *cgiName, char_t *query)
{

	int offset,flash_offset;

	diag_printf("Start upgrade...\n");
	watchdog = 0;
	si_watchdog_ms(bcm947xx_sih, 0);
	
	if( (offset = webCgiGetUploadFile(wp, query) ) <= 0 || offset >=wp->clen2) 
	{

		SetErrorArg("Get upgrade file failed!","direct_reboot.asp");
		goto do_error;
	}

	flash_offset = 0;

	if(do_upgrade_check(query,offset,&flash_offset)<0){
		SetErrorArg("Check upgrade file error!","direct_reboot.asp");
		goto do_error;
	}

	if(tenda_upload_fw(query,offset,flash_offset) < 0){
		SetErrorArg("Write upgrade file error!","direct_reboot.asp");
		goto do_error;
	}

	upgrade_mem_free(query);
	wp->query = NULL;
	
	websRedirect(wp, T("/direct_reboot.asp"));
	sys_reboot();
	
	diag_printf("Upgrade success...");
	
	return;
	
do_error:
	upgrade_mem_free(query);
	wp->query = NULL;
	websRedirect(wp, T("error.asp")); 
	sys_reboot();
	return;
}

static char tips[]="#Please don't change this file by hand\r\n";

void DownloadCfg(webs_t wp, char_t *path, char_t *query)
{
	int wrote,MibLen;//, err;
	char_t* date;
	char *pMibBuf;
	char line[256], *val;
	char *name = NULL;

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

	websWrite(wp, T("Content-length: %d\r\n"), MibLen);
	websSetRequestBytes(wp, MibLen);
	websWrite(wp, T("Content-type: config/conf\r\n"));
	websWrite(wp, T("Connection: close\r\n"));
	//websWrite(wp, T("Content-type: %s\r\n"), websGetRequestType(wp));

	websWrite(wp, T("\r\n"));

	wrote = 0;

	wrote += websWriteDataNonBlock(wp, tips, strlen(tips));

	//name1=value1\0name2=value2\0name3=...
	for(name = pMibBuf; *name; name += strlen(name) + 1){
		//filter, only give the  data in router_defaults
		memset(line,0,sizeof(line));
		strncpy(line,name,sizeof(line));
		val = strchr((const char*)line, '=');
		if(!val)
			continue;
		/* cut the var name */
		*val = 0;
		if (find_nvram(line, true)) {
			wrote += websWriteDataNonBlock(wp, name, strlen(name));
			//write \r\n ,so we can use notepad to edit cfg
			wrote += websWriteDataNonBlock(wp, "\r\n", 2);
		}
	}

	websSetRequestWritten(wp, wrote);
	websDone(wp, 200);

	free(pMibBuf);
}

void UploadCfg(webs_t wp, char_t *path, char_t *query)
{	
	char_t *pBufStart;
	int offset,flash_offset,ok,size;

	if( (offset = webCgiGetUploadFile2(wp, query) ) <= 0 || offset >=wp->clen2) 
	{

		SetErrorArg("CGI get config file failed!","system_backup.asp");
		goto error_out;
	}

	
	pBufStart = query + offset;
	flash_offset = 0;
	size = wp->clen2 - offset;
	
	ok = CFG_write_prof(pBufStart, size);

	if(ok != 0){
		SetErrorArg("CGI write config file failed!","system_backup.asp");
		goto error_out;
	}

	websRedirect(wp, T("/direct_reboot.asp"));
	sys_reboot();
	return;
 error_out:
 	websRedirect(wp, T("/error.asp")); 
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

	if (sysFlashInit(NULL)) {
		diag_printf("%s: error initializing flash\n", __func__);
		goto done;
	}

	wrote = flashutl_desc->size;
	
	websWrite(wp, T("Content-length: %d\r\n"), wrote);
	websSetRequestBytes(wp, wrote);

	websWrite(wp, T("Content-type: config/conf\r\n"));
	websWrite(wp, T("Connection: close\r\n"));

	websWrite(wp, T("\r\n"));

	wrote = 0;

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
	
 done:
	websSetRequestWritten(wp, wrote);
	websDone(wp, 200);

	printf("\nDownloadFlashImage ok ...\n");
}


