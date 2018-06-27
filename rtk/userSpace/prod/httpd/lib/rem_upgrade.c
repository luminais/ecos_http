#include <stdio.h>
#include <stdlib.h>
#include <trxhdr.h>
#include <hndsoc.h>
#include <bcmendian.h>
#include <bcmutils.h>
#include <flashutl.h>
#include <httpd.h>

#ifdef	ROUNDUP
#undef	ROUNDUP
#endif
#define	ROUNDUP(x, y)	(((unsigned long)(x)+(y)-1) & ~(y))
#define UDBLKSIZE	2048
#define	UDCHUNK		0x10000

static char **udblks;
static int udnum;
static int udidx;

static char *udheap;
static int udheapnum;
static int udheapbi;

static void *udclalloc();
static void udclfree(void *p);

static int
udalloc(int len)
{
	struct mallinfo mem_info;
	int i;
	int hsize;

	udblks = 0;
	udheap = 0;

	/* Allocate udblks */
	udidx = 0;
	udnum = ROUNDUP(len, UDBLKSIZE)/UDBLKSIZE;

	/* Allocate udblks */
	udblks = (char **)calloc(udnum, sizeof(char *));
	if (udblks == NULL)
		return -1;

	/* Find maximum heap can provide */
	mem_info = mallinfo();
	if (mem_info.fordblks > len + 0x18000)
		hsize = ROUNDUP(len, UDBLKSIZE);
	else
		hsize = ROUNDUP(mem_info.fordblks - 0x18000, UDBLKSIZE);

	/* We need at least 64K for accelerate flash programming */
	if (hsize < 0x10000)
		return -1;

	for (i = 0; i < 32; i++) {
		if ((hsize >> i) == 1) {
			udheapbi = (1 << i);
			break;
		}
	}

	/* Allocate maximum avaliable from heap */
	udheapnum = hsize/UDBLKSIZE;
	udheap = (char *)malloc(hsize);
	if (udheap == NULL)
		return -1;

	/* Assign udblks */
	for (i = 0; i < udnum; i++) {
		if (i < udheapnum) {
			udblks[i] = udheap + UDBLKSIZE * i;
		}
		else {
			/* Allocate from cluster */
			udblks[i] = udclalloc();
			if (udblks[i] == NULL)
				return -1;
		}
	}

	return 0;
}

static void
udfree()
{
	int i;

	if (!udblks)
		return;

	if (!udheap)
		goto noheap;

	for (i = 0; i < udnum; i++) {
		if (i >= udheapnum)
			udclfree(udblks[i]);
	}

	/* Free udheap */
	free(udheap);
	udheap = 0;

noheap:
	/* Free udblks */
	free(udblks);
	udblks = 0;
	return;
}

static inline int
udrewind()
{
	udidx = 0;
	return 0;
}

static inline int
udseek(int len)
{
	int i = ROUNDUP(len, UDBLKSIZE);

	udidx += (i/UDBLKSIZE);
	if (udidx > udnum)
		udidx = udnum;

	return 0;
}

static inline int
udwrite(char *buf, int len)
{
	int left = len;
	int blen = UDBLKSIZE;
	int offset = 0;

	while (left) {
		if (udidx >= udnum)
			break;

		if (left < blen)
			blen = left;

		memcpy(udblks[udidx], buf+offset, blen);

		offset += blen;
		left -= blen;
		udidx++;
	}

	return offset;
}

static inline int
udread(char *buf, int len)
{
	int left = len;
	int blen = UDBLKSIZE;
	int offset = 0;

	while (left) {
		if (udidx >= udnum)
			break;

		if (left < blen)
			blen = left;

		memcpy(buf+offset, udblks[udidx], blen);

		offset += blen;
		left -= blen;
		udidx++;
	}

	return offset;
}

int
sys_upgrade(char *url, FILE *stream, int *total)
{
	char *cpbuf = NULL;
	int cplen;
	struct trx_header trx;
	struct trx_header flash;
	uint32 bisz[BISZ_SIZE];
	unsigned long crc;
	int offset;
	int trxlen;
	int left;
	int ok = -3;
	int len;
	int rlen;
	unsigned char *fpbuf;

	if (!stream) {
		printf("%s: NULL stream.\n", __func__);
		return -1;
	}

	/* Allocate a copy buffer */
	cplen = UDBLKSIZE;
	cpbuf = (char *)malloc(cplen);
	if (!cpbuf) {
		printf("%s: Out of memroy for copy buffer.\n", __func__);
		return -1;
	}

	rlen = fread(cpbuf, 1, cplen, stream);
	*total -= rlen;
	if (rlen != cplen) {
		printf("%s: fread trx header failed.\n", __func__);
		goto fail;
	}

	memcpy(&trx, cpbuf, sizeof(trx));
	trxlen = ltoh32(trx.len);

	/* check header */
	if (ltoh32(trx.magic) != TRX_MAGIC ||
	    trxlen > TRX_MAX_LEN ||
	    trxlen < sizeof(trx)) {
		printf("%s: Bad header (magic 0x%x len %d)\n",
			__func__, ltoh32(trx.magic), trxlen);
		goto fail;
	}

	/* Allocate udblks */
	if (udalloc(trxlen) != 0) {
		printf("%s:udalloc failed.\n", __func__);
		goto fail;
	}

	/* Write the first chunk */
	udwrite(cpbuf, cplen);

	/* Each time, pull a cplen to ud */
	left = trxlen - cplen;
	len = cplen;
	while (left > 0) {
		if (len > left)
			len = left;

		/* Read it */
		rlen = fread(cpbuf, 1, len, stream);
		*total -= rlen;
		if (rlen != len) {
 			printf("%s(%d):fread rlen = %d\n", __func__, __LINE__, rlen);
			goto fail;
		}

		udwrite(cpbuf, len);
		left -= len;
	}

	/* Check CRC before writing if possible */
	udrewind();
	crc = CRC32_INIT_VALUE;
	offset = 12;
	left = trxlen;
	len = cplen;
	while (left > 0) {
		if (len > left)
			len = left;

		udread(cpbuf, len);
		crc = hndcrc32((unsigned char *)cpbuf+offset, len-offset, crc);

		left -= len;
		offset = 0;
	}

	if (crc != ltoh32(trx.crc32)) {
		printf("%s: Bad CRC, crc=%08lx trx.crc=%08lx\n",
			__func__, crc, (unsigned long)trx.crc32);
		goto fail;
	}

	/* Start writing to flash */
	printf("%s: Programming\n", __func__);
	if (sysFlashInit(NULL)) {
		printf("%s: error initializing flash\n", __func__);
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
		printf("%s: unable to find os at offset %x in flash\n", __func__,
			offset);
		goto fail;
	}

	/* Write the whole chunk to flash */
	cyg_scheduler_lock();

	udrewind();
	fpbuf = udheap;
	len = udheapbi;
	left = trxlen;
	while (left > 0) {
		if (len > left)
			len = left;

		if (left == trxlen)
			udseek(len);
		else
			udread(fpbuf, len);

		diag_printf("Write %x bytes to %x\n", len, offset);
		if (!sysFlashWrite(offset, fpbuf, len)) {
			cyg_scheduler_unlock();
			printf("%s: error writing FLASH chunk 0x%x size %x\n", __func__,
				offset, trxlen);
			goto fail;
		}

		offset += len;
		left -= len;
	}

	cyg_scheduler_unlock();
	ok = 0;
	printf("%s: Done\n", __func__);

fail:
	if (cpbuf)
		free(cpbuf);

	udfree();
	return (ok);
}

#define _KERNEL
#include <sys/param.h>
#include <sys/mbuf.h>

static void *
udclalloc()
{
	void *p;

	MCLALLOC(p, M_DONTWAIT);
	return p;
}

static void
udclfree(void *p)
{
	MCLFREE(p);
}
#undef _KERNEL
