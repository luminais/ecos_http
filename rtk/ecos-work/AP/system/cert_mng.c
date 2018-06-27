/*=============================================================================================
 * 	cert_mng.c
 *
 * Data: 2013/11/26
 * Author: patrick_cai
 * Description:
 * 	Certificate Management System controls all certificates in eCos. 
 * 	If one application wants to use certificates, this system is expected to be enabled.
 * 	
 * 	Certificates stores in the last CERT_MNG_SIZE bytes in flash, which in configurable.
 * 	This system mainly supplies certificates read, write, search, remove operations.
 * ============================================================================================
 */
#include "cert_mng.h"
#include <stdlib.h>
#ifdef HAVE_CERT_COMPRESS
#include <cyg/compress/zlib.h>
#include <cyg/compress/zconf.h>
#include <sys/stat.h> 
#include <stdio.h>
#endif
cyg_mutex_t cert_mng_mutex;

/*
 * Init certificate management system, return 1 on success, return -1 for malloc error, return 0 for flash write error
 * reinit == 1:force to reinit
 * reinit == 0:don't force
 */
int cert_mng_init(int reinit){
	struct cert_mng_header *mng_header;
	int ret = -1;

	// init mutex
	cyg_mutex_init(&cert_mng_mutex);
	
	// init mng header
	mng_header=(struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);
	if(mng_header==NULL){
		return CERT_MALLOC_ERR;
	}

	cert_mng_getMngHeader(mng_header);
	if(mng_header==NULL){ // flash read fails
		return CERT_MALLOC_ERR;
	}
	if(!reinit){
		//if((!memcmp(mng_header, CERT_MNG_TAG, 4)) && (mng_header->length!=0)){
		if(!memcmp(mng_header, CERT_MNG_TAG, 4)){
			free(mng_header);
			return 1;
		}
	}
	{
		printf("reinit cert mng header\n");
		// forget the old cert_mng_header, build a new header and write to flash
		memcpy(mng_header->tag, CERT_MNG_TAG, 4);
		mng_header->length=0;
		ret = rtk_flash_write((char *)mng_header, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN);
		free(mng_header);
		if(ret==1) return ret;
		else	return CERT_FLASH_WRITE_ERR;
	}
}

/*
 * Get Cert Mng Sys Header. If success, addr points to the header, else, addr=NULL
 */
void cert_mng_getMngHeader(void *addr)
{
	int ret;
	ret = rtk_flash_read((char *)addr, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN);	
	if(ret==0) addr = NULL; //read failed
	return;
}

/*
 * Get the max length(uncompressed if used) and total number of certificate in module
 * If getType is CERT_GET_IGNORE_TYPE, then ignore type
 * If getType is CERT_GET_TYPE, then return status with module and type
 * If not found, return 0
 * Else the max length and number is stored in maxSize and count, and return 1;
 * If err, return negative number;
 */
int  cert_mng_getStat(unsigned int module, unsigned int type, int *maxSize, int *count, int getType)
{
	if(!cyg_mutex_trylock(&cert_mng_mutex))
		return CERT_LOCK_ERR;

	struct cert_mng_header *mng_header;
	struct cert_mng_unit_header *unit_header;
	mng_header=(struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);
	if(mng_header==NULL){
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_MALLOC_ERR;
	}
	cert_mng_getMngHeader(mng_header);
	if(mng_header==NULL){
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_FLASH_READ_ERR;
	}
	unit_header=(struct cert_mng_unit_header *)malloc(CERT_UNIT_HEADER_LEN);
	if(unit_header==NULL)
	{
		free(mng_header);
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_MALLOC_ERR;
	}

	int total_len = mng_header->length;
	int step = 0;
	*count = 0;
	*maxSize = 0;

	while(step <= total_len)
	{
		if(!rtk_flash_read((char *)unit_header, CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step, CERT_UNIT_HEADER_LEN))
			goto getStatByModule_err;
#ifdef HAVE_CERT_COMPRESS
		if(getType)
		{
			if(((unit_header->module == module) || (unit_header->module == (module | CERT_COMPRESS_BIT))) && (unit_header->type == type))
			{
				*maxSize = (*maxSize>(unit_header->original_length))?(*maxSize):(unit_header->original_length);
				*count += 1;	
			}
		}else
		{
			if((unit_header->module == module) || (unit_header->module == (module | CERT_COMPRESS_BIT)))
			{
				*maxSize = (*maxSize>(unit_header->original_length))?(*maxSize):(unit_header->original_length);
				*count += 1;	
			}
		}
#else
		if(getType)
		{
			if((unit_header->module == module) && (unit_header->type == type))
			{
				*maxSize = (*maxSize>(unit_header->length))?(*maxSize):(unit_header->length);
				*count += 1;
			}
		}else
		{
			if(unit_header->module == module)
			{
				*maxSize = (*maxSize>(unit_header->length))?(*maxSize):(unit_header->length);
				*count += 1;
			}
		}
#endif
		step += CERT_UNIT_HEADER_LEN + unit_header->length;
	}//end while

	free(mng_header);
	free(unit_header);
	cyg_mutex_unlock(&cert_mng_mutex);

	if(*count > 0)
		return 1;
	else
		return 0;
getStatByModule_err:
	free(mng_header);
	free(unit_header);
	cyg_mutex_unlock(&cert_mng_mutex);
	return CERT_FLASH_READ_ERR;	
}

#ifdef HAVE_CERT_COMPRESS
/*
 * Uncompress a gzFile stored in compressed with length len to uncompressed.
 * If success, return uncompressed length;
 * else, return err code.
 */
static int cert_file_uncompress(char *compressed, unsigned int len, char *uncompressed)
{
	FILE *fd = fopen(CERT_COMPRESS_FILE, "w+");
	if(fwrite(compressed, 1, len, fd) != len){
		printf("<%s:%d>fwrite error\n", __FUNCTION__, __LINE__);
		fclose(fd);
		return CERT_FILE_IO_ERR;
	}
	fclose(fd);

	// uncompress cert.gz, and memcpy it to uncompressed
	gzFile gzfile = gzopen(CERT_COMPRESS_FILE, "r");
	// use a buf to do uncompress, uncompress CERT_UNCOMPRESS_BUFSIZE each time
	char *buf = (char *)malloc(CERT_UNCOMPRESS_BUFSIZE);
	if(buf==NULL)	return CERT_MALLOC_ERR;
	memset(buf, '\0', CERT_UNCOMPRESS_BUFSIZE);

	int uncompressed_count = 0;
	int written = 0;
	for(;;)
	{
		uncompressed_count = gzread(gzfile, buf, CERT_UNCOMPRESS_BUFSIZE);
		if (uncompressed_count < 0) {
			printf("%s: failed gzread\n", __FUNCTION__);
			free(buf);
			return CERT_FILE_IO_ERR;
		}else if(uncompressed_count == 0){
			printf("%s: uncompressed_count==0\n", __FUNCTION__);
			break;
		}
		// copy uncompressed cert to dest
		//printf("strlen(buf)=%d\n", strlen(buf));
		memcpy(((char *)uncompressed)+written, buf, ((strlen(buf)<=CERT_UNCOMPRESS_BUFSIZE)?strlen(buf):CERT_UNCOMPRESS_BUFSIZE));
		written += (strlen(buf)<=CERT_UNCOMPRESS_BUFSIZE)?strlen(buf):CERT_UNCOMPRESS_BUFSIZE;	
		memset(buf, '\0', CERT_UNCOMPRESS_BUFSIZE);
	}				
	free(buf);
	gzclose(gzfile);
	unlink(CERT_COMPRESS_FILE);

	return written;
}
#endif

/* 
 * rtk_flash_write() currently only support flash write in BLOCK_SIZE aligned. 
 * Write an interface to support flash write in bytes aligned.
 * Return 1 for success, else, return err code
 */
static int cert_flash_write(char *mem, unsigned int flash_offset, int length)
{
#if 0
	// this buffer is for store the data in one flash block.
	char *buffer = (char *)malloc(FLASH_BLOCK_SIZE);
	if(buffer==NULL)
		return CERT_MALLOC_ERR;
	memset(buffer, '\0', FLASH_BLOCK_SIZE);

	int block_start_offset = flash_offset & (~(FLASH_BLOCK_SIZE-1));
	int block_used = flash_offset - block_start_offset;
	int block_left = FLASH_BLOCK_SIZE - block_used;
	int written = 0;
	int to_write;
	int block_num=0;

	if(!rtk_flash_read(buffer, block_start_offset, FLASH_BLOCK_SIZE))
		return CERT_FLASH_READ_ERR;

	while(written < length)
	{
		to_write = (block_left >= (length-written)) ? (length-written) : block_left;

		memcpy(buffer+block_used, mem+written, to_write);
		if(!rtk_flash_write(buffer, block_start_offset, FLASH_BLOCK_SIZE)){
			free(buffer);
			return CERT_FLASH_WRITE_ERR;
		}

		written += to_write;
		block_start_offset += FLASH_BLOCK_SIZE;
		block_used = 0;
		block_left = FLASH_BLOCK_SIZE;
		block_num++;
		memset(buffer, '\0', FLASH_BLOCK_SIZE);
	}//end while

	free(buffer);
	return 1;
#else
	int ret = rtk_flash_write(mem, flash_offset, length);
	return ret;
#endif
}

/* 
 * Search for one certificate, type 1 for stage 1 search, type 2 for stage 2 deeper search
 * If found, return its offset addr in flash, if not, return 0. if error, return CERT_XX_ERR
 */
static int cert_search(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int search_type)
{
	int found = 0;
	int step = 0;
	int ret_offset = 0;
	struct cert_mng_header *mng_header;
	struct cert_mng_unit_header *unit_header;

	mng_header=(struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);
	if(mng_header==NULL){
		return CERT_MALLOC_ERR;
	}
	cert_mng_getMngHeader(mng_header);
	//printf("<%s:%d>0: search_type=%d mng_header->length=%d\n", __FUNCTION__, __LINE__, search_type, mng_header->length);
	unit_header=(struct cert_mng_unit_header *)malloc(CERT_UNIT_HEADER_LEN);
	if(unit_header==NULL){
		free(mng_header);
		return CERT_MALLOC_ERR;
	}

	while(step < mng_header->length)
	{
		if(!rtk_flash_read((char *)unit_header, CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step, CERT_UNIT_HEADER_LEN)){
			free(mng_header);
			free(unit_header);
			return CERT_FLASH_READ_ERR;
		}
#ifdef HAVE_CERT_COMPRESS
		if(((unit_header->module==module) || (unit_header->module==(module | CERT_COMPRESS_BIT))) && (unit_header->type==type))
#else
		if((unit_header->module==module)&&(unit_header->type==type))
#endif
		{
			//if search_type is 1: just search stage 1
			//else go to search stage 2: deep search, compare content of cert
			if(search_type==CERT_EASY_SEARCH)
				found = 1;
			else
#ifndef HAVE_CERT_COMPRESS
				found = cert_deepSearch(CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step, unit_header, cert_file, cert_len);
#else
				found = cert_deepSearch(CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step, unit_header, cert_file, cert_len);
#endif
		}
		//printf("<%s:%d>step=%d, module=%d, type=%d, found=%d\n", __FUNCTION__, __LINE__,step, (unsigned char)(unit_header->module), (unsigned char)(unit_header->type), found);
		if(found==1){ 
			printf("Found!!!!!\n");
			ret_offset = CERT_MNG_START_OFFSET + CERT_MNG_HEADER_LEN + step;
			free(mng_header);
			free(unit_header);
			return ret_offset;
		}
	
		step += CERT_UNIT_HEADER_LEN + unit_header->length;
		continue;
	}// end while

	free(mng_header);
	free(unit_header);
	return 0;
}

// note: this offset is the offset for one cert unit
// flash_offset: the offset of this cert unit in flash
// flash_file  : the cert unit header
// cert_file   : the cert file to be searched.
// cert_len    : length of the cert file to be searched
static int cert_deepSearch(unsigned int flash_offset, struct cert_mng_unit_header *flash_file_header, void *cert_file, unsigned int cert_len)
{
	int found = 0;
	int retry = CERT_RETRY;
	unsigned int length = flash_file_header->length;
#ifdef HAVE_CERT_COMPRESS
	unsigned int orig_len = flash_file_header->original_length;
#endif
	unsigned int cksum = flash_file_header->checksum;

#ifdef HAVE_CERT_COMPRESS
	if(cert_len != orig_len)
#else
	if(cert_len != length)
#endif
		return found;

	unsigned char *flash_cert = (unsigned char *)malloc(length);
	memset(flash_cert, '\0', length);

	if(flash_cert==NULL)
		return CERT_MALLOC_ERR;

	while(retry){
		// here we can't just use length, because if compressing, we just need to read the length while length is original_length here
		if(rtk_flash_read(flash_cert, flash_offset+CERT_UNIT_HEADER_LEN, length))
		{
			if(!cert_cksumOK(cksum, length, flash_cert)){
				diag_printf("<%s:%d>%d:Checksum Error\n", __FUNCTION__,__LINE__,retry);
				retry--;
				continue;
			}else
			{
				// if cksum is OK
				retry = CERT_RETRY;
#ifdef HAVE_CERT_COMPRESS
				// need to check if flash_cert is compressed or not first. 
				// If not compressed, just memcmp; else, uncompress and memcmp.
				if(flash_file_header->module & CERT_COMPRESS_BIT)
				{
					// in this case, flash_cert is compressed!
					//printf("<%s:%d>Case A\n", __FUNCTION__, __LINE__);
					// malloc a space for uncompressing
					char *uncompressed_file = (char *)malloc(orig_len);
					if(uncompressed_file==NULL)
					{
						printf("<%s:%d>malloc err!!!\n",__FUNCTION__,__LINE__);
						free(flash_cert);
						return CERT_MALLOC_ERR;
					}

					if(cert_file_uncompress(flash_cert, length, uncompressed_file) <= 0)
					{
						free(flash_cert);
						free(uncompressed_file);
						printf("%s:%d, uncompress err\n",__FUNCTION__,__LINE__);		
						return CERT_FILE_IO_ERR;
					}

					// do the cmp
					if(!memcmp(uncompressed_file, cert_file, cert_len))
					{
						found = 1;
						break;
					}
					free(uncompressed_file);
				}else
				{
					// in this case, flash_cert isn't compressed!
					//printf("<%s:%d>Case B\n", __FUNCTION__, __LINE__);

					// do the cmp
					if(!memcmp(flash_cert, cert_file, cert_len))
					{
						found = 1;
						break;
					}
				}
#else
				if(!memcmp(flash_cert, cert_file, cert_len)){
					found = 1;
					break;
				}
#endif
			}//end if checksumOK
		}else
		{
			free(flash_cert);
			return CERT_FLASH_READ_ERR;
		}
	}//end while
	free(flash_cert);
	return found;
}

// note: if found, it will return the offset addr of this cert unit in flash, instead of this cert content offset addr in flash
int cert_mng_search(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int search_type)
{
	//return cert_search(module, type, cert_file, cert_len, search_type);
	int ret = cert_search(module, type, cert_file, cert_len, search_type);
	if(ret > 0)
		return 1;
	else 
		return 0;
}

/* 
 * Pullup the remaining certificates, type 1 for pullup one by one, type 2 for whole pullup. Default is 2
 * If success, return 1
 * flash_offset: flash offset of the removed cert unit
 * removed_length: length of the removed cert
 */
static int cert_pullup(unsigned int flash_offset, unsigned int removed_length, unsigned int pullupType)
{
	int remaining_offset, remaining_size;
	char *remaining = NULL;
	char *buffer =NULL;
	struct cert_mng_header *mng_header = (struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);		
	if(mng_header==NULL)
		return CERT_MALLOC_ERR;	
	if(!rtk_flash_read(mng_header, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN)){
		free(mng_header);
		return CERT_FLASH_READ_ERR;
	}
	remaining_offset = flash_offset + removed_length + CERT_UNIT_HEADER_LEN;
	remaining_size = (mng_header->length + CERT_MNG_HEADER_LEN) - (flash_offset-CERT_MNG_START_OFFSET) - (removed_length + CERT_UNIT_HEADER_LEN);
	//printf("<%s:%d>\nremoved_offset=0x%x, removed_size=%d\nremaining_offset=0x%x, remaining_size=%d\n",__FUNCTION__,__LINE__,flash_offset, removed_length+CERT_UNIT_HEADER_LEN, remaining_offset, remaining_size);		
	if(remaining_size==0)
	{
		//no cert after the cert unit to be removed. Just update the length in mng header
		char *block_to_write = (char *)malloc(FLASH_BLOCK_SIZE);
		if(block_to_write==NULL)
		{
			free(mng_header);
			return CERT_MALLOC_ERR;
		}
		memset(block_to_write, '\0', FLASH_BLOCK_SIZE);
		if(rtk_flash_read(block_to_write, CERT_MNG_START_OFFSET, FLASH_BLOCK_SIZE))
		{
			((struct cert_mng_header *)block_to_write)->length -= (removed_length + CERT_UNIT_HEADER_LEN);
		}else
		{
			free(mng_header);
			free(block_to_write);
			return CERT_FLASH_READ_ERR;	
		}

		if(rtk_flash_write(block_to_write, CERT_MNG_START_OFFSET, FLASH_BLOCK_SIZE)){
			free(mng_header);
			free(block_to_write);
			return 1;
		}
		else{
			free(mng_header);
			free(block_to_write);
			return CERT_FLASH_WRITE_ERR;
		}
	}

	// the cert to be removed is in the middle of cert units
	remaining = (char *)malloc(remaining_size);
	if(remaining == NULL)
		return CERT_MALLOC_ERR;

	//pullup remaining_size bytes in flash from remaining_offset to flash_offset
	if(rtk_flash_read(remaining, remaining_offset, remaining_size))
	{
		//printf("<%s:%d>read success!\n", __FUNCTION__,__LINE__);
		if(cert_flash_write(remaining, flash_offset, remaining_size)!=1)
			goto remove_err;

		//update the header
		buffer = (char *)malloc(FLASH_BLOCK_SIZE);
		if(buffer==NULL)
			goto remove_err;
		memset(buffer, '\0', FLASH_BLOCK_SIZE);
		{
			if(!rtk_flash_read(buffer, CERT_MNG_START_OFFSET, FLASH_BLOCK_SIZE))
				goto remove_err2;
			((struct cert_mng_header *)buffer)->length -= (removed_length + CERT_UNIT_HEADER_LEN);
			if(!rtk_flash_write(buffer, CERT_MNG_START_OFFSET, FLASH_BLOCK_SIZE))
				goto remove_err2;
		}

		free(buffer);
		free(mng_header);
		free(remaining);
		return 1;
	}else
		goto remove_err;

remove_err2:
	free(buffer);
remove_err:
	free(mng_header);
	free(remaining);
	return CERT_FLASH_WRITE_ERR;
}

/*
 * Remove Cert
 * if not found, return -1
 * if remove failed, return 0
 * if success, return 1
 * removeType == search type: 1=CERT_EASY_SEARCH - search stage 1; 2=CERT_DEEP_SEARCH - search stage 2.
 */
int cert_mng_remove(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len)
{
	if(cyg_mutex_trylock(&cert_mng_mutex)){
		// succeed getting this lock
		//printf("<%s:%d>doRemove\n",__FUNCTION__, __LINE__);
		int ret = cert_doRemove(module, type, cert_file, cert_len, CERT_DEEP_SEARCH);	
		cyg_mutex_unlock(&cert_mng_mutex);
		return ret;
	}else{
		printf("<%s:%d>no lock\n", __FUNCTION__, __LINE__);
		return CERT_LOCK_ERR;	
	}
}

// do not in flash, return -1
// remove and pullup succeed, return 1
// remove and pullup failed, return 0
static int cert_doRemove(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int removeType)
{
	int unit_offset = 0;
	int ret;
	struct cert_mng_unit_header *unit_header = NULL;
	
	// if this cert not in flash, return -1
	unit_offset=cert_search(module, type, cert_file, cert_len, removeType);
	//printf("<%s:%d> unit_offset=0x%x\n", __FUNCTION__, __LINE__, unit_offset);
	if(unit_offset<=0)
		return -1;	
	unit_header = (struct cert_mng_unit_header *)malloc(CERT_UNIT_HEADER_LEN);
	if(unit_header==NULL)
		return CERT_MALLOC_ERR;
	//printf("<%s:%d>0\n",__FUNCTION__,__LINE__);
	if(!rtk_flash_read(unit_header, unit_offset, CERT_UNIT_HEADER_LEN))
		return CERT_FLASH_READ_ERR;

	//printf("<%s:%d>1\n",__FUNCTION__,__LINE__);
	ret = cert_pullup(unit_offset, unit_header->length, CERT_PULLUP_TOTAL);	
	//printf("<%s:%d>2,ret=%d\n",__FUNCTION__,__LINE__,ret);
	free(unit_header);
	if(ret==1) //pullup successful
		return 1;
	else
		return 0;
}

/*
 * Write certificate to flash
 * cert length is required
 * If success, return 1
 */
int cert_mng_write(unsigned int module, unsigned int type, void *cert_file, unsigned int length)
{
	//printf("<%s:%d>enter\n", __FUNCTION__, __LINE__);
	if(!cyg_mutex_trylock(&cert_mng_mutex))
		return CERT_LOCK_ERR;
	
	struct cert_mng_header *mng_header = (struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);
	size_t block_size = FLASH_BLOCK_SIZE;
	char *block_to_write = (char *)malloc(block_size);
	char *new_unit = NULL;

	if(mng_header==NULL){
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_MALLOC_ERR;
	}
	if(module==CERT_WAPI)
	{
		if(!rtk_flash_read(mng_header, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN)){
			cyg_mutex_unlock(&cert_mng_mutex);
			free(mng_header);
			free(block_to_write);
			return CERT_FLASH_READ_ERR;
		}
		//if WAPI, the stratage is:
		//	if enough space in cert mng sys, write in the last,
		//	else return err.
		if((CERT_MNG_HEADER_LEN + mng_header->length + CERT_UNIT_HEADER_LEN + length) <= CERT_SIZE)
		{
			// build a cert unit
			new_unit = (char *)malloc(length + CERT_UNIT_HEADER_LEN);
			if(new_unit==NULL){
				free(block_to_write);
				free(mng_header);
				cyg_mutex_unlock(&cert_mng_mutex);
				return CERT_MALLOC_ERR;
			}
			if(!build_cert_unit(module, type, length, cert_file, new_unit))
				goto write_err2;

			// write new unit to flash	
			if(cert_flash_write(new_unit, CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+mng_header->length, CERT_UNIT_HEADER_LEN+((struct cert_mng_unit_header *)new_unit)->length) != 1)
				goto write_err2;
			// update cert mng header
			// write to blocks which don't contain cert mng header. Needs to modify the first block.
			memset(block_to_write, '\0', block_size);
			if(!rtk_flash_read(block_to_write, CERT_MNG_START_OFFSET, block_size))	
				goto write_err2;
			((struct cert_mng_header *)block_to_write)->length += CERT_UNIT_HEADER_LEN+((struct cert_mng_unit_header *)new_unit)->length;
			if(!rtk_flash_write(block_to_write, CERT_MNG_START_OFFSET, block_size))
				goto write_err2;	

			free(block_to_write);
		} 
		else{//not enough space
			cyg_mutex_unlock(&cert_mng_mutex);
			printf("Not enough space to store certificate\n");
			free(mng_header);
			free(block_to_write);
			return CERT_FLASH_WRITE_ERR;
		}
	}
	else //if not WAPI
	{
		int remove_ret = cert_doRemove(module,type,cert_file, length, CERT_EASY_SEARCH);
		//printf("<%s:%d>remove_ret=%d\n",__FUNCTION__,__LINE__,remove_ret);
		if(remove_ret==0)
		{
			// cert in flash but remove failed!
			cyg_mutex_unlock(&cert_mng_mutex);
			free(mng_header);
			return CERT_FLASH_WRITE_ERR;
		}

		if(!rtk_flash_read(mng_header, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN)){
			cyg_mutex_unlock(&cert_mng_mutex);
			free(mng_header);
			return CERT_FLASH_READ_ERR;
		}

		new_unit = (char *)malloc(length*3 + CERT_UNIT_HEADER_LEN); 
		if(new_unit==NULL){
			cyg_mutex_unlock(&cert_mng_mutex);
			return CERT_MALLOC_ERR;
		}
		if(!build_cert_unit(module, type, length, cert_file, new_unit))
			goto write_err;

		if(cert_flash_write(new_unit, CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+mng_header->length, CERT_UNIT_HEADER_LEN+((struct cert_mng_unit_header *)new_unit)->length) != 1)
			goto write_err2;
		// update cert mng header
		// write to blocks which don't contain cert mng header. Needs to modify the first block.
		memset(block_to_write, '\0', block_size);
		if(!rtk_flash_read(block_to_write, CERT_MNG_START_OFFSET, block_size))	
			goto write_err2;
		((struct cert_mng_header *)block_to_write)->length += CERT_UNIT_HEADER_LEN+((struct cert_mng_unit_header *)new_unit)->length;
		if(!rtk_flash_write(block_to_write, CERT_MNG_START_OFFSET, block_size))
			goto write_err2;	

		free(block_to_write);
	}

	free(mng_header);
	free(new_unit);
	cyg_mutex_unlock(&cert_mng_mutex);
	printf("<%s:%d>write_OK\n",__FUNCTION__,__LINE__);
	return 1;

write_err2:
	free(block_to_write);
write_err:
	free(mng_header);
	free(new_unit);
	printf("<%s:%d>write err!!!\n", __FUNCTION__, __LINE__);
	cyg_mutex_unlock(&cert_mng_mutex);
	return CERT_FLASH_WRITE_ERR;
}

// note: length is the length of cert content
static int build_cert_unit(unsigned int module, unsigned int type, unsigned int length, void *cert_file, char *dest)
{
	if((dest==NULL)||(cert_file==NULL))
		return 0;

	((struct cert_mng_unit_header *)dest)->module =(unsigned char)module;
	((struct cert_mng_unit_header *)dest)->type = (unsigned char)type;
#ifdef HAVE_CERT_COMPRESS
	int length_after_compress=0;
	char c = '\0';
	// build a gz file
	gzFile outfile = gzopen(CERT_COMPRESS_FILE, "w+");
	FILE *fd = NULL;
	if(outfile==NULL)
		return 0;
	// compress the cert
	int compressed_len = gzwrite(outfile, cert_file, length);
	if(compressed_len != length){
		printf("<%s:%d>compress error, compressed_len=%d!\n",__FUNCTION__,__LINE__, compressed_len);
		return 0;
	}
	sleep(3);
	gzclose(outfile);

	//printf("<%s:%d>coping compressed certs...\n", __FUNCTION__,__LINE__);

	// get the size of gz file
	struct stat *fileinfo = malloc(sizeof(struct stat));
	stat(CERT_COMPRESS_FILE, fileinfo);
	length_after_compress = fileinfo->st_size;
	//printf("<%s:%d>length_after_compress: %d\n", __FUNCTION__, __LINE__, length_after_compress);

	if(length_after_compress<=length)
	{
		// in this case, length is shorter after compression, so use the content after compression
		// note that now we have a extra field: original_length. See cert_mng.h
		((struct cert_mng_unit_header *)dest)->original_length = length;
		// update the module name, module |= 1<<7;
		((struct cert_mng_unit_header *)dest)->module =((unsigned char)module) | CERT_COMPRESS_BIT;
		// update real length and copy the compressed cert file(gz);
		//printf("<%s:%d>Case A\n", __FUNCTION__,__LINE__);
		fd = fopen(CERT_COMPRESS_FILE, "r");
		if(fd==NULL){
			printf("fopen failed!!!!!!!\n");
			return 0;
		}
#if 0
		length=0;
		while(length < length_after_compress)
		{
			c=fgetc(fd);
			dest[CERT_UNIT_HEADER_LEN + length] = c;
			length++;
		}
		printf("<%s:%d>after copy: length=%d\n", __FUNCTION__, __LINE__, length);
#else
		fread(dest+CERT_UNIT_HEADER_LEN, 1, length_after_compress, fd);
#endif

		// update padding
		//((struct cert_mng_unit_header *)dest)->padding = cert_toAlign(length);
		// length, ensure it's 4 bytes alignment
		((struct cert_mng_unit_header *)dest)->length = length;
		// update checksum
		// note that we do cksum after compress, so when reading, we must do cksum first
		((struct cert_mng_unit_header *)dest)->checksum = cert_cksum(length, dest+CERT_UNIT_HEADER_LEN);
	}
	else{
		//printf("<%s:%d>Case B\n", __FUNCTION__,__LINE__);
		// in this case, length is longer after compression, so just use the original content of cert
		// note that now we have a extra field: original_length. See cert_mng.h.
		// Here we don't store the compressed result, thus original_length field == length field
		((struct cert_mng_unit_header *)dest)->original_length = length;
	
		//((struct cert_mng_unit_header *)dest)->padding = cert_toAlign(length);
		// length, ensure it's 4 bytes alignment
		((struct cert_mng_unit_header *)dest)->length = length;
		// copy the cert content and do checksum
		memcpy(dest+CERT_UNIT_HEADER_LEN, cert_file, length);
		((struct cert_mng_unit_header *)dest)->checksum = cert_cksum(length, dest+CERT_UNIT_HEADER_LEN);
	}
	//printf("<%s:%d>original_length = %d\n",__FUNCTION__, __LINE__, ((struct cert_mng_unit_header *)dest)->original_length);
#else	
	((struct cert_mng_unit_header *)dest)->length = length;
	// copy the cert content and do checksum
	memcpy(dest+CERT_UNIT_HEADER_LEN, cert_file, length);
	((struct cert_mng_unit_header *)dest)->checksum = cert_cksum(length, dest+CERT_UNIT_HEADER_LEN);
#endif
	// add padding, ensure content part of cert unit is 4 bytes alignment
	//if(cert_toAlign(length) != 0)
	//	memset(dest+CERT_UNIT_HEADER_LEN+length,'\0',(unsigned int)(((struct cert_mng_unit_header *)dest)->padding));
	
	//printf("<%s:%d>module'=%d, type'=%d, length'=%d\n",__FUNCTION__,__LINE__, ((struct cert_mng_unit_header *)dest)->module, ((struct cert_mng_unit_header *)dest)->type, ((struct cert_mng_unit_header *)dest)->length);
#ifdef HAVE_CERT_COMPRESS
	if(fd!=NULL)	fclose(fd);
	// destory cert.gz
	unlink(CERT_COMPRESS_FILE);
#endif
	return 1;
}

/*
 * Read one type of cert to memory addr.
 * return the size of this cert
 */
int cert_mng_read(unsigned int module, unsigned int type, void *addr, unsigned int given_len)
{
	//printf("<%s:%d>enter\n", __FUNCTION__, __LINE__);
	if(addr==NULL)
		return CERT_MALLOC_ERR;
	if(!cyg_mutex_trylock(&cert_mng_mutex))
		return CERT_LOCK_ERR;
	
	int unit_offset=0;
	int len = -1;
	struct cert_mng_unit_header *unit_header = (struct cert_mng_unit_header*)malloc(CERT_UNIT_HEADER_LEN);
	if(unit_header==NULL){
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_MALLOC_ERR;
	}
	// only go search stage 1, doesn't need cert_file and cert_len argument, just pass NULL or 0;
	//printf("<%s:%d>A\n",__FUNCTION__,__LINE__);
	unit_offset = cert_search(module, type, NULL, 0, CERT_EASY_SEARCH);
	if(unit_offset>0)
	{
		//printf("<%s:%d>1, unit_offset=0x%x\n",__FUNCTION__,__LINE__, unit_offset);
		if(!rtk_flash_read((char *)unit_header, unit_offset , CERT_UNIT_HEADER_LEN))
			goto read_err;
		len = unit_header->length;
		//printf("<%s:%d>2, cert length stored in header = %d\n",__FUNCTION__,__LINE__, len);
#ifndef HAVE_CERT_COMPRESS
		if(len > given_len) goto read_err;
		// read the content of this cert to addr
		if(!rtk_flash_read((char *)addr, unit_offset+CERT_UNIT_HEADER_LEN, len))
			goto read_err;

		// checksum
		if(cert_cksumOK(unit_header->checksum, (unsigned int)(unit_header->length), (unsigned char *)addr)){
			cyg_mutex_unlock(&cert_mng_mutex);
			free(unit_header);
			//printf("<%s:%d>4,len=%d\n",__FUNCTION__,__LINE__,len);
			return len;
		}else{
			printf("<%s:%d> cksum error!\n", __FUNCTION__, __LINE__);
			goto read_err;
		}
#else
		//printf("<%s:%d>b\n",__FUNCTION__,__LINE__);
		int orig_len = unit_header->original_length;
		if(orig_len > given_len)	goto read_err;
		if(unit_header->module & CERT_COMPRESS_BIT)
		{
			// in this case, cert is compressed!
			//printf("<%s:%d>case A\n", __FUNCTION__, __LINE__);

			// read the compressed gzFile in cert unit out to cert.gz
			char *cert_compressed = (char *)malloc(len);	
			if(cert_compressed==NULL)
				goto read_err;
			if(!rtk_flash_read((void *)cert_compressed, unit_offset+CERT_UNIT_HEADER_LEN, len)){
				free(cert_compressed);
				goto read_err;
			}
			if(!cert_cksumOK(unit_header->checksum, len, (unsigned char *)cert_compressed)){
				printf("<%s:%d> cksum error!\n", __FUNCTION__, __LINE__);
				goto read_err;
			}

			int written = cert_file_uncompress(cert_compressed, len, (char *)addr);

			//printf("\n<%s:%d>f\n",__FUNCTION__,__LINE__);
			cyg_mutex_unlock(&cert_mng_mutex);
			free(unit_header);

			//return uncompressed_count;
			return written;
		}else
		{
			// in this case, cert isn't compressed!
			//printf("<%s:%d>case B\n", __FUNCTION__, __LINE__);
			if(len > given_len) goto read_err;

			// read the content of this cert to addr
			if(!rtk_flash_read((char *)addr, unit_offset+CERT_UNIT_HEADER_LEN, len))
				goto read_err;

			// checksum
			if(cert_cksumOK(unit_header->checksum, (unsigned int)(unit_header->length), (unsigned char *)addr)){
				cyg_mutex_unlock(&cert_mng_mutex);
				free(unit_header);
				//printf("<%s:%d>4,len=%d\n",__FUNCTION__,__LINE__,len);
				return len;
			}else{
				printf("<%s:%d> cksum error!\n", __FUNCTION__, __LINE__);
				goto read_err;
			}
		}
#endif
	}
	//printf("<%s:%d>3\n",__FUNCTION__,__LINE__);
read_err:
	cyg_mutex_unlock(&cert_mng_mutex);
	free(unit_header);
	return CERT_FLASH_READ_ERR;
}

/*
 * Read all certificates of one kind of module to addr pointer array, whose number is count
 * If getType is CERT_GET_INGORE_TYPE, type will be ignored
 * Else, will get cert with module and type
 * If success, return 1;
 */
int  cert_mng_fullRead(unsigned int module, unsigned int type, void **addr, unsigned int count, unsigned int given_length, int getType)
{
	if((addr==NULL)||(count==0))
		return CERT_FLASH_READ_ERR;
	if(!cyg_mutex_trylock(&cert_mng_mutex))
		return CERT_LOCK_ERR;
	
	int read=0;
	int step=0;
	int retry=CERT_RETRY;
	struct cert_mng_header *mng_header = (struct cert_mng_header *)malloc(CERT_MNG_HEADER_LEN);
	if(mng_header==NULL){
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_LOCK_ERR;
	}
	struct cert_mng_unit_header *unit_header = (struct cert_mng_header_unit *)malloc(CERT_UNIT_HEADER_LEN);
	if(unit_header==NULL){
		free(mng_header);
		cyg_mutex_unlock(&cert_mng_mutex);
		return CERT_LOCK_ERR;
	}

	if(!rtk_flash_read((char *)mng_header, CERT_MNG_START_OFFSET, CERT_MNG_HEADER_LEN))
		goto fullread_err;
	while((step<mng_header->length) && (read<count) && (retry>0))
	{
		if(!rtk_flash_read((char *)unit_header, CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step, CERT_UNIT_HEADER_LEN))
			goto fullread_err;
#ifdef HAVE_CERT_COMPRESS
		if(getType)
			if(!(((unit_header->module == module)||(unit_header->module == (module | CERT_COMPRESS_BIT))) && (unit_header->type==type))) continue;
		else
			if(!((unit_header->module != module) || (unit_header->module != (module | CERT_COMPRESS_BIT)))) continue;
		if(unit_header->original_length > given_length) goto fullread_err;
#else
		if(getType)
			if(!((unit_header->module == module) && (unit_header->type== type))) continue;
		else
			if(unit_header->module!=module)	continue;

		if(unit_header->length > given_length) goto fullread_err;
#endif
		// if goes to next statement, means found cert of this module, read the content of this cert unit to memory
		// NOTE: here we assume addr is big enough to store certificate.
		if(!rtk_flash_read((char *)(addr[read]), CERT_MNG_START_OFFSET+CERT_MNG_HEADER_LEN+step+CERT_UNIT_HEADER_LEN, unit_header->length))
			goto fullread_err;
		// do cksum for the content
		if(!cert_cksumOK(unit_header->checksum, unit_header->length, (unsigned char *)(addr[read])))
		{	//checksum err;
			diag_printf("<%s:%d>%d: checksum error\n", __FUNCTION__, __LINE__, retry);
			retry--;
			break;
		}
#ifdef HAVE_CERT_COMPRESS
		if(unit_header->module & CERT_COMPRESS_BIT)
		{
			// in this case, this cert content stored in addr is compressed, needs uncompressing!
			//printf("<%s:%d>Case A\n", __FUNCTION__, __LINE__);
			// build cert.gz file
			int len = unit_header->length;
			if(cert_file_uncompress((char *)(addr[read]), len, (char *)(addr[read])) <= 0)
				goto fullread_err;
		}else
			// in this case, this cert content stored in addr isn't compresssed, just do cksum and return.
			//printf("<%s:%d>Case B\n", __FUNCTION__, __LINE__);
#endif
		read++;
		retry = CERT_RETRY;
		step += unit_header->length + CERT_UNIT_HEADER_LEN;
	}//end while
	if(retry==0){
		diag_printf("FullRead: checksum is wrong!!!\n");	
		goto fullread_err;
	}
	if(read>count){
		diag_printf("FullRead: There're more than %d certificates in flash\n", count);
		goto fullread_err;
	}else if(read<count)
	{
		diag_printf("FullRead: There're less than %d certificates in flash, actual number is %d\n", count, read);
		while(read<count)
		{
			addr[read]=NULL;
			read++;
		}
	}

	free(mng_header);
	free(unit_header);
	cyg_mutex_unlock(&cert_mng_mutex);
	return 1;
	
fullread_err:
	free(mng_header);
	free(unit_header);
	cyg_mutex_unlock(&cert_mng_mutex);
	return CERT_FLASH_READ_ERR;	
}

/*
 * Checksum, return the sum
 */
static unsigned char cert_cksum(unsigned int length, unsigned char *cert_file)
{
	int i;
	unsigned char sum=0;

	for (i=0; i<length; i++)
		sum += cert_file[i];

	sum = ~sum + 1;
	return sum;	
}

/*
 * Validate checksum, if correct, return 1, else return 0
 * cksum:     checksum of this cert unit
 * length:    length of the cert content
 * cert_file: content of the cert
 */
static int cert_cksumOK(unsigned char cksum, unsigned int length, unsigned char *cert_file)
{
	int i;
	unsigned char sum=cksum;

	for (i=0; i<length; i++)
		sum += cert_file[i];

	if (sum == 0)
		return 1;
	else
		return 0;	
}
