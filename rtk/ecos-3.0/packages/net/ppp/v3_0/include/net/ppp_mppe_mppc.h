#ifndef _PPP_MPPE_MPPE_H_
#define _PPP_MPPE_MPPE_H_

typedef	unsigned char		u8;
typedef unsigned short 		u16;
typedef	unsigned int		u32;
/*
 * State for a mppc/mppe "(de)compressor".
 */
#if 0
struct arc4_ctx {
	u8 S[256];
	u8 x, y;
};

struct crypto_tfm 
{
	struct arc4_ctx  *crt_ctx;
};
struct ppp_mppe_state {
    struct crypto_tfm arc4_tfm;
    u8		master_key[MPPE_MAX_KEY_LEN];
    u8		session_key[MPPE_MAX_KEY_LEN];
    u8		mppc;		/* do we use compression (MPPC)? */
    u8		mppe;		/* do we use encryption (MPPE)? */
    u8		keylen;		/* key length in bytes */
    u8		bitkeylen;	/* key length in bits */
    u16		ccount;		/* coherency counter */
    u16		bits;		/* MPPC/MPPE control bits */
    u8		stateless;	/* do we use stateless mode? */
    u8		nextflushed;	/* set A bit in the next outgoing packet;
				   used only by compressor*/
    u8		flushexpected;	/* drop packets until A bit is received;
				   used only by decompressor*/
    u8		*hist;		/* MPPC history */
    u16		*hash;		/* Hash table; used only by compressor */
    u16		histptr;	/* history "cursor" */
    int		unit;
    int		debug;
    int		mru;
    struct compstat stats;
};
#endif

#define MPPE_HIST_LEN		8192	/* MPPC history size */
#define MPPE_MAX_CCOUNT		0x0FFF	/* max. coherency counter value */
//#define MPPE_MAX_CCOUNT		(1<<12)	/* max. coherency counter value */

#define MPPE_BIT_FLUSHED	0x80	/* bit A */
#define MPPE_BIT_RESET		0x40	/* bit B */
#define MPPE_BIT_COMP		0x20	/* bit C */
#define MPPE_BIT_ENCRYPTED	0x10	/* bit D */

#define MPPE_SALT0		0xD1	/* values used in MPPE key derivation */
#define MPPE_SALT1		0x26	/* according to RFC3079 */
#define MPPE_SALT2		0x9E

#define MPPE_CCOUNT(x)		((((x)[4] & 0x0f) << 8) + (x)[5])
#define MPPE_BITS(x)		((x)[4] & 0xf0)
#define MPPE_CTRLHI(x)		((((x)->ccount & 0xf00)>>8)|((x)->bits))
#define MPPE_CTRLLO(x)		((x)->ccount & 0xff)

#endif /* _PPP_MPPE_MPPE_H_ */
