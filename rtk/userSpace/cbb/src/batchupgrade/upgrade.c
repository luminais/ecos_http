#include <stdio.h>
#include <batchupgrade.h>
#include <typedefs.h>

#include <trxhdr.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <bcmutils.h>

struct upgrade_mem{
	struct upgrade_mem *next;
	int totlen;
	int inuse;
	char *data;
};

extern int sysFlashInit(char *flash_str);
extern int sysFlashRead(uint off, uchar *buf, uint numbytes);
extern int sysFlashWrite(uint off, uchar *src, uint numbytes);

int get_flash_offset(int *offset)
{
	int len;
	uint32 bisz[BISZ_SIZE];
	struct trx_header flash;
	
	if (sysFlashInit(NULL)) {
		return READ_FLASH_FAIL;
	}
	
	/* Figure out where the os starts in flash, assume it is at 256KB */
	len = 256 * 1024;
	/* Set to 128KB if the bootloader is smaller */
	if (sysFlashRead(BISZ_OFFSET, (uchar *)bisz, sizeof(bisz)) == sizeof(bisz) &&
	    bisz[BISZ_MAGIC_IDX] == BISZ_MAGIC &&
	    bisz[BISZ_DATAEND_IDX] - bisz[BISZ_TXTST_IDX] < 128 * 1024) {
		len = 128 * 1024;
	}
	
	/* Check it is indeed a TRX at the offset after the bootloader */
	if (sysFlashRead(len, (uchar *)&flash, sizeof(flash)) != sizeof(flash) ||
	    ltoh32(flash.magic) != TRX_MAGIC) {
		UPGRADE_DEBUG("%s: unable to find os at len %x in flash\n", __func__, len);
		return READ_FLASH_FAIL;
	}
	
	*offset = len;
	return TPI_RET_OK;
}

TPI_BOOL validate_image_format(char *stream, int httpd_offset)
{
	struct upgrade_mem *ubuf;
	char *image = NULL;

	ubuf = (struct upgrade_mem *)stream;
	image = ubuf->data + httpd_offset;

	if(memcmp(image, MAGIC_IMAGE_HEAD, sizeof(MAGIC_IMAGE_HEAD) - 1) != 0)
		return TPI_FALSE;

	return TPI_TRUE;	
}
