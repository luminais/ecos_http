/*================================================
 * 	cert_mng.h
 *
 * Data: 2013/11/26 
 * Author: patrick_cai
 * Description:
 * 	Header file of certificate management system.
 * ===============================================
 */	
#ifndef CERT_MNG_H_
#define CERT_MNG_H_

#include <cyg/kernel/kapi.h>
#include "sys_utility.h"

/*===========Constants & Variables================*/
#ifndef CERT_SIZE
#ifndef CERT_MNG_SIZE
#define CERT_SIZE		0x2000 		//default 8K
#else 
#define CERT_SIZE 		CERT_MNG_SIZE	//use config data
#endif
#endif

#ifndef CERT_FLASH_SIZE
#ifndef ECOS_FLASH_SIZE
#define CERT_FLASH_SIZE		0x200000
#else
#define CERT_FLASH_SIZE		ECOS_FlASH_SIZE
#endif
#endif

#define FLASH_BLOCK_SIZE	4096
#define CERT_MNG_START_OFFSET	(CERT_FLASH_SIZE - CERT_SIZE)
#define CERT_MNG_TAG 		"CERT"
#define CERT_MNG_HEADER_LEN 	(sizeof(struct cert_mng_header))
#define CERT_UNIT_HEADER_LEN 	(sizeof(struct cert_mng_unit_header))

#ifdef  HAVE_CERT_COMPRESS
#define CERT_COMPRESS_FILE	"/var/cert.gz"
#define CERT_UNCOMPRESS_BUFSIZE	4096
#define CERT_COMPRESS_BIT	CERT_BIT(7)
#define CERT_BIT(x)		(1<<(x))
#endif

#define CERT_MALLOC_ERR		-1
#define CERT_FLASH_READ_ERR	-2
#define CERT_CKSUM_ERR		-3
#define CERT_LOCK_ERR		-4
#define CERT_FLASH_WRITE_ERR	-5
#ifdef HAVE_CERT_COMPRESS
#define CERT_FILE_IO_ERR	-6
#endif

#define CERT_RETRY		3
#define CERT_PULLUP_TOTAL	2
#define CERT_PULLUP_EACH	1
#define CERT_EASY_SEARCH	1
#define CERT_DEEP_SEARCH	2
#define CERT_GET_TYPE		1
#define CERT_GET_IGNORE_TYPE	0

/*=================User Definitions===================*/
enum cert_module{
	CERT_WAPI	=0,
	CERT_1X		=1,
	CERT_HTTPS	=2,
	CERT_TR069	=3,
};

enum cert_type{
	CERT_CA		=0,
	CERT_CLIENT	=1,
};

/*=================Core Definitions===================*/
struct cert_mng_header
{
	unsigned char tag[4]; // should be CERT
	unsigned int length;  // length of cert mng content, exclude this header
};
struct cert_mng_unit_header
{
	unsigned char module;
	unsigned char type;
	unsigned char checksum;		// checksum of the cert content, if using compressing, it's the checksum of compressed certificate.
	unsigned char reserved;
	unsigned int length;		// length of the cert content
#ifdef HAVE_CERT_COMPRESS
	unsigned int original_length;	// if compressed, length is the length stored in this unit. Original_length is the actual length of certificate, it will be used in uncompressing.
#endif
};

/*==============Function Definitions==============*/
//API
int  cert_mng_init(int reinit);
void cert_mng_getMngHeader(void *addr);
int  cert_mng_getStat(unsigned int module, unsigned int type, int *maxSize, int *count, int getType);
int  cert_mng_search(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int search_type);
int  cert_mng_fullRead(unsigned int module, unsigned int type, void **addr, unsigned int count, unsigned int given_len, int getType);
int  cert_mng_read(unsigned int module, unsigned int type, void *addr, unsigned int given_len);
int  cert_mng_write(unsigned int module, unsigned int type, void *cert_file, unsigned int length);
int  cert_mng_remove(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len);

//Internal
static int cert_pullup(unsigned int flash_offset, unsigned int removed_length, unsigned int pullupType);
static int cert_doRemove(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int removeType);
static int cert_search(unsigned int module, unsigned int type, void *cert_file, unsigned int cert_len, int search_type);
static int cert_deepSearch(unsigned int flash_offset, struct cert_mng_unit_header *flash_file, void *cert_file, unsigned int cert_len);
static int build_cert_unit(unsigned int module, unsigned int type, unsigned int length, void *cert_file, char *dest);
#ifdef HAVE_CERT_COMPRESS
static int cert_file_uncompress(char *compressed, unsigned int length, char *uncompressed);
#endif
static int cert_flash_write(char *mem, unsigned int offset, int length);
static unsigned char cert_cksum(unsigned int length, unsigned char *cert_file);
static int cert_cksumOK(unsigned char cksum, unsigned int length, unsigned char *cert_file);

#endif // end define CERT_MNG_H_
