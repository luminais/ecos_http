
/* $Id: mtd.h,v 1.1 2009/11/13 13:22:47 jasonwang Exp $ */

#ifndef __MTD_MTD_H__
#define __MTD_MTD_H__

//#ifdef __KERNEL__

//#include <linux/version.h>
#include "types.h"
//#include <linux/mtd/compatmac.h>
//#include <linux/module.h>

//#endif /* __KERNEL__ */

struct erase_info_user {
	unsigned long start;
	unsigned long length;
};

struct mtd_oob_buf {
	loff_t start;
	ssize_t length;
	unsigned char *ptr;
};


#define MTD_CHAR_MAJOR 90
#define MTD_BLOCK_MAJOR 31
#define MAX_MTD_DEVICES 16



#define MTD_ABSENT		0
#define MTD_RAM			1
#define MTD_ROM			2
#define MTD_NORFLASH		3
#define MTD_NANDFLASH		4
#define MTD_PEROM		5
#define MTD_OTHER		14
#define MTD_UNKNOWN		15



#define MTD_CLEAR_BITS		1       // Bits can be cleared (flash)
#define MTD_SET_BITS		2       // Bits can be set
#define MTD_ERASEABLE		4       // Has an erase function
#define MTD_WRITEB_WRITEABLE	8       // Direct IO is possible
#define MTD_VOLATILE		16      // Set for RAMs
#define MTD_XIP			32	// eXecute-In-Place possible
#define MTD_OOB			64	// Out-of-band data (NAND flash)
#define MTD_ECC			128	// Device capable of automatic ECC

// Some common devices / combinations of capabilities
#define MTD_CAP_ROM		0
#define MTD_CAP_RAM		(MTD_CLEAR_BITS|MTD_SET_BITS|MTD_WRITEB_WRITEABLE)
#define MTD_CAP_NORFLASH        (MTD_CLEAR_BITS|MTD_ERASEABLE)
#define MTD_CAP_NANDFLASH       (MTD_CLEAR_BITS|MTD_ERASEABLE|MTD_OOB)
//#define MTD_WRITEABLE		(MTD_CLEAR_BITS|MTD_SET_BITS)


// Types of automatic ECC/Checksum available
#define MTD_ECC_NONE		0 	// No automatic ECC available
#define MTD_ECC_RS_DiskOnChip   1       // Automatic ECC on DiskOnChip

struct mtd_info_user {
	u_char type;
	u_long flags;
	u_long size;	 // Total size of the MTD
	u_long erasesize;
	u_long oobblock;  // Size of OOB blocks (e.g. 512)
	u_long oobsize;   // Amount of OOB data per block (e.g. 16)
        u_long ecctype;
        u_long eccsize;
};

#define MEMGETINFO              _IOR('M', 1, struct mtd_info_user)
#define MEMERASE                _IOW('M', 2, struct erase_info_user)
#define MEMWRITEOOB             _IOWR('M', 3, struct mtd_oob_buf)
#define MEMREADOOB              _IOWR('M', 4, struct mtd_oob_buf)

//#ifndef __KERNEL__

typedef struct mtd_info_user mtd_info_t;
typedef struct erase_info_user erase_info_t;

	/* User-space ioctl definitions */


//#else /* __KERNEL__ */


#define MTD_ERASE_PENDING      	0x01
#define MTD_ERASING		0x02
#define MTD_ERASE_SUSPEND	0x04
#define MTD_ERASE_DONE          0x08
#define MTD_ERASE_FAILED        0x10

struct erase_info {
	struct mtd_info *mtd;
	u_long addr;
	u_long len;
	u_long fail_addr;
	u_long time;
	u_long retries;
	unsigned dev;
	unsigned cell;
	void (*callback) (struct erase_info *self);
	u_long priv;
	u_char state;
	struct erase_info *next;
};

struct mtd_erase_region_info {
	uint64_t offset;		/* At which this region starts, from the beginning of the MTD */
	uint32_t erasesize;		/* For this region */
	uint32_t numblocks;		/* Number of blocks of erasesize in this region */
	unsigned long *lockmap;		/* If keeping bitmap of locks */
};

/**
 * struct mtd_oob_ops - oob operation operands
 * @mode:	operation mode
 *
 * @len:	number of data bytes to write/read
 *
 * @retlen:	number of data bytes written/read
 *
 * @ooblen:	number of oob bytes to write/read
 * @oobretlen:	number of oob bytes written/read
 * @ooboffs:	offset of oob data in the oob area (only relevant when
 *		mode = MTD_OPS_PLACE_OOB or MTD_OPS_RAW)
 * @datbuf:	data buffer - if NULL only oob data are read/written
 * @oobbuf:	oob data buffer
 *
 * Note, it is allowed to read more than one OOB area at one go, but not write.
 * The interface assumes that the OOB write requests program only one page's
 * OOB area.
 */
struct mtd_oob_ops {
	unsigned int	mode;
	size_t		len;
	size_t		retlen;
	size_t		ooblen;
	size_t		oobretlen;
	uint32_t	ooboffs;
	uint8_t		*datbuf;
	uint8_t		*oobbuf;
};

#define MTD_MAX_OOBFREE_ENTRIES_LARGE	32
#define MTD_MAX_ECCPOS_ENTRIES_LARGE	640

struct mtd_info {
	u_char type;
	uint32_t flags;
	uint64_t size;	 // Total size of the MTD

	/* "Major" erase size for the device. Naive users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	uint32_t erasesize;
	/* Minimal writable flash unit size. In case of NOR flash it is 1 (even
	 * though individual bits can be cleared), in case of NAND flash it is
	 * one NAND page (or half, or one-fourths of it), in case of ECC-ed NOR
	 * it is of ECC block size, etc. It is illegal to have writesize = 0.
	 * Any driver registering a struct mtd_info must ensure a writesize of
	 * 1 or larger.
	 */
	uint32_t writesize;

	/*
	 * Size of the write buffer used by the MTD. MTD devices having a write
	 * buffer can write multiple writesize chunks at a time. E.g. while
	 * writing 4 * writesize bytes to a device with 2 * writesize bytes
	 * buffer the MTD driver can (but doesn't have to) do 2 writesize
	 * operations, but not 4. Currently, all NANDs have writebufsize
	 * equivalent to writesize (NAND page size). Some NOR flashes do have
	 * writebufsize greater than writesize.
	 */
	uint32_t writebufsize;

	uint32_t oobsize;   // Amount of OOB data per block (e.g. 16)
	uint32_t oobavail;  // Available OOB bytes per block

	/*
	 * If erasesize is a power of 2 then the shift is stored in
	 * erasesize_shift otherwise erasesize_shift is zero. Ditto writesize.
	 */
	unsigned int erasesize_shift;
	unsigned int writesize_shift;
	/* Masks based on erasesize_shift and writesize_shift */
	unsigned int erasesize_mask;
	unsigned int writesize_mask;

	/*
	 * read ops return -EUCLEAN if max number of bitflips corrected on any
	 * one region comprising an ecc step equals or exceeds this value.
	 * Settable by driver, else defaults to ecc_strength.  User can override
	 * in sysfs.  N.B. The meaning of the -EUCLEAN return code has changed;
	 * see Documentation/ABI/testing/sysfs-class-mtd for more detail.
	 */
	unsigned int bitflip_threshold;

	// Kernel-only stuff starts here.
	const char *name;
	int index;

	/* ECC layout structure pointer - read only! */
	struct nand_ecclayout *ecclayout;

	/* max number of correctible bit errors per ecc step */
	unsigned int ecc_strength;

	/* Data for variable erase regions. If numeraseregions is zero,
	 * it means that the whole device has erasesize as given above.
	 */
	int numeraseregions;
	struct mtd_erase_region_info *eraseregions;

	/*
	 * Do not call via these pointers, use corresponding mtd_*()
	 * wrappers instead.
	 */
	int (*_erase) (struct mtd_info *mtd, struct erase_info *instr);
#if 0
	int (*_point) (struct mtd_info *mtd, loff_t from, size_t len,
		       size_t *retlen, void **virt, resource_size_t *phys);
	int (*_unpoint) (struct mtd_info *mtd, loff_t from, size_t len);
	unsigned long (*_get_unmapped_area) (struct mtd_info *mtd,
					     unsigned long len,
					     unsigned long offset,
					     unsigned long flags);
#endif
	int (*_read) (struct mtd_info *mtd, loff_t from, size_t len,
		      size_t *retlen, u_char *buf);
	int (*_write) (struct mtd_info *mtd, loff_t to, size_t len,
		       size_t *retlen, const u_char *buf);
#if 0
	int (*_panic_write) (struct mtd_info *mtd, loff_t to, size_t len,
			     size_t *retlen, const u_char *buf);
	int (*_read_oob) (struct mtd_info *mtd, loff_t from,
			  struct mtd_oob_ops *ops);
	int (*_write_oob) (struct mtd_info *mtd, loff_t to,
			   struct mtd_oob_ops *ops);
	int (*_get_fact_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				    size_t len);
	int (*_read_fact_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, u_char *buf);
	int (*_get_user_prot_info) (struct mtd_info *mtd, struct otp_info *buf,
				    size_t len);
	int (*_read_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len, size_t *retlen, u_char *buf);
	int (*_write_user_prot_reg) (struct mtd_info *mtd, loff_t to,
				     size_t len, size_t *retlen, u_char *buf);
	int (*_lock_user_prot_reg) (struct mtd_info *mtd, loff_t from,
				    size_t len);
	int (*_writev) (struct mtd_info *mtd, const struct kvec *vecs,
			unsigned long count, loff_t to, size_t *retlen);
	void (*_sync) (struct mtd_info *mtd);
#endif
	int (*_lock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_unlock) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
	int (*_is_locked) (struct mtd_info *mtd, loff_t ofs, uint64_t len);
#if 0
	int (*_block_isbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_block_markbad) (struct mtd_info *mtd, loff_t ofs);
	int (*_suspend) (struct mtd_info *mtd);
	void (*_resume) (struct mtd_info *mtd);
	/*
	 * If the driver is something smart, like UBI, it may need to maintain
	 * its own reference counting. The below functions are only for driver.
	 */
	int (*_get_device) (struct mtd_info *mtd);
	void (*_put_device) (struct mtd_info *mtd);

	/* Backing device capabilities for this device
	 * - provides mmap capabilities
	 */
	struct backing_dev_info *backing_dev_info;

	struct notifier_block reboot_notifier;  /* default mode before reboot */

	/* ECC status information */
	struct mtd_ecc_stats ecc_stats;
	/* Subpage shift (NAND) */
	int subpage_sft;
#endif
	void *priv;

	struct module *owner;
//	struct device dev;
	int usecount;
};

#if 0
	/* Kernel-side ioctl definitions */

extern int add_mtd_device(struct mtd_info *mtd);
extern int del_mtd_device (struct mtd_info *mtd);

extern struct mtd_info *__get_mtd_device(struct mtd_info *mtd, int num);

static inline struct mtd_info *get_mtd_device(struct mtd_info *mtd, int num)
{
	struct mtd_info *ret;
	
	ret = __get_mtd_device(mtd, num);

	if (ret && ret->module && !try_inc_mod_count(ret->module))
		return NULL;

	return ret;
}

static inline void put_mtd_device(struct mtd_info *mtd)
{
       if (mtd->module)
	       __MOD_DEC_USE_COUNT(mtd->module);
}


struct mtd_notifier {
	void (*add)(struct mtd_info *mtd);
	void (*remove)(struct mtd_info *mtd);
	struct mtd_notifier *next;
};


extern void register_mtd_user (struct mtd_notifier *new);
extern int unregister_mtd_user (struct mtd_notifier *old);


#ifndef MTDC
#define MTD_ERASE(mtd, args...) (*(mtd->erase))(mtd, args)
#define MTD_POINT(mtd, a,b,c,d) (*(mtd->point))(mtd, a,b,c, (u_char **)(d))
#define MTD_UNPOINT(mtd, arg) (*(mtd->unpoint))(mtd, (u_char *)arg)
#define MTD_READ(mtd, args...) (*(mtd->read))(mtd, args)
#define MTD_WRITE(mtd, args...) (*(mtd->write))(mtd, args)
#define MTD_READOOB(mtd, args...) (*(mtd->read_oob))(mtd, args)
#define MTD_WRITEOOB(mtd, args...) (*(mtd->write_oob))(mtd, args)
#define MTD_SYNC(mtd) do { if (mtd->sync) (*(mtd->sync))(mtd);  } while (0) 
#endif /* MTDC */

/* Debugging macros */

#ifdef DEBUGLVL
#define DEBUG(n, args...) if (DEBUGLVL>(n)) printk(KERN_DEBUG args)
#else
#define DEBUG(n, args...)
#endif
#endif
//#endif /* __KERNEL__ */


#endif /* __MTD_MTD_H__ */
