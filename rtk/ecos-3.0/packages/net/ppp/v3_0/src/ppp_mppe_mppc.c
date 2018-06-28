
#define _KERNEL
#include <sys/param.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>

#include <cyg/ppp/net/ppp_defs.h>
#define PACKETPTR	struct mbuf *
#include <cyg/ppp/net/ppp_comp.h>
#include <cyg/ppp/net/ppp_mppe_mppc.h>
#include <cyg/ppp/sha1.h>

/* ARC4 alg**/
#define ARC4_MIN_KEY_SIZE	1
#define ARC4_MAX_KEY_SIZE	256
#define ARC4_BLOCK_SIZE		1
extern int pptp_kernel_debug;
static int backup_stateless;

struct rtl_arc4_ctx {
	u8 S[256];
	u8 x, y;
};

struct rtl_crypto_tfm 
{
	struct rtl_arc4_ctx  *crt_ctx;
};

struct ppp_mppe_state {
    struct rtl_crypto_tfm arc4_tfm;
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


static int rtl_arc4_set_key(struct rtl_crypto_tfm *tfm, const u8 *in_key,
			unsigned int key_len)
{
	struct rtl_arc4_ctx *ctx = tfm->crt_ctx;
	int i, j = 0, k = 0;

	ctx->x = 1;
	ctx->y = 0;

	for(i = 0; i < 256; i++)
		ctx->S[i] = i;

	for(i = 0; i < 256; i++)
	{
		u8 a = ctx->S[i];
		j = (j + in_key[k] + a) & 0xff;
		ctx->S[i] = ctx->S[j];
		ctx->S[j] = a;
		if(++k >= key_len)
			k = 0;
	}

	return 0;
}

static void rtl_arc4_crypt(struct rtl_crypto_tfm *tfm, u8 *out, const u8 *in)
{
	struct rtl_arc4_ctx *ctx = tfm->crt_ctx;
	u8 *const S = ctx->S;
	u8 x = ctx->x;
	u8 y = ctx->y;
	u8 a, b;

	a = S[x];
	y = (y + a) & 0xff;
	b = S[y];
	S[x] = b;
	S[y] = a;
	x = (x + 1) & 0xff;
	*out++ = *in ^ S[(a + b) & 0xff];

	ctx->x = x;
	ctx->y = y;
}

#define SHA1_SIGNATURE_SIZE 20
extern void rtl_SHA1_Init(SHA1_CTX *);
extern void rtl_SHA1_Update(SHA1_CTX *, const u8 *, u32);
extern void rtl_SHA1_Final(u8[SHA1_SIGNATURE_SIZE], SHA1_CTX *);

#define MPPC_MPPE_DEBUG
#define Malloc(x,y) malloc((y),M_TEMP,M_DONTWAIT)
#define Mdup(x,y,z) Mdup_rtl((y),(z))
#define Mstrdup(x,y) Mstrdup_rtl((y))
#define Freee(x)	free((x),M_TEMP)
#define FREE_M(m)							\
	do {								\
		if ((m)) {						\
			m_freem((m));					\
			(m) = NULL;					\
		}							\
	} while (0)

static inline void
rtl_arc4_setkey(struct ppp_mppe_state *state, const unsigned char *key,
	    const unsigned int keylen)
{
    rtl_arc4_set_key(&(state->arc4_tfm), key, keylen);
}

static inline void
rtl_arc4_encrypt(struct ppp_mppe_state *state, const unsigned char *in,
	     const unsigned int len, unsigned char *out)
{
    int i;
    for (i = 0; i < len; i++)
    {
       rtl_arc4_crypt(&(state->arc4_tfm), out+i, in+i);
    }
}

#define rtl_arc4_decrypt rtl_arc4_encrypt

/*
 * Key Derivation, from RFC 3078, RFC 3079.
 * Equivalent to Get_Key() for MS-CHAP as described in RFC 3079.
 */
static void
GetNewKeyFromSHA(struct ppp_mppe_state *state, unsigned char *MasterKey,
		 unsigned char *SessionKey, unsigned long SessionKeyLength,
		 unsigned char *InterimKey)
{
    /*Pads used in key derivation */
    static const unsigned char  SHAPad1[40] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    static const unsigned char  SHAPad2[40] = {
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2
    };

    unsigned char Digest[SHA1_SIGNATURE_SIZE];
    SHA1_CTX Context;

    rtl_SHA1_Init(&Context);
    rtl_SHA1_Update(&Context, MasterKey, SessionKeyLength);
    rtl_SHA1_Update(&Context, SHAPad1, sizeof(SHAPad1));
    rtl_SHA1_Update(&Context, SessionKey, SessionKeyLength);
    rtl_SHA1_Update(&Context, SHAPad2, sizeof(SHAPad2));
    rtl_SHA1_Final(Digest, &Context);

    memcpy(InterimKey, Digest, SessionKeyLength);
}

static void
rtl_mppe_change_key(struct ppp_mppe_state *state, int initialize)
{
    unsigned char InterimKey[MPPE_MAX_KEY_LEN];

    GetNewKeyFromSHA(state, state->master_key, state->session_key,
		     state->keylen, InterimKey);
    if (initialize) {
	memcpy(state->session_key, InterimKey, state->keylen);
    } else {
	rtl_arc4_setkey(state, InterimKey, state->keylen);
	rtl_arc4_encrypt(state, InterimKey, state->keylen, state->session_key);
    }
    if (state->keylen == 8) {
	if (state->bitkeylen == 40) {
	    state->session_key[0] = MPPE_SALT0;
	    state->session_key[1] = MPPE_SALT1;
	    state->session_key[2] = MPPE_SALT2;
	} else {
	    state->session_key[0] = MPPE_SALT0;
	}
    }
    rtl_arc4_setkey(state, state->session_key, state->keylen);
}

/* increase 12-bit coherency counter */
static inline void
rtl_mppe_increase_ccount(struct ppp_mppe_state *state)
{
    state->ccount = (state->ccount + 1) & MPPE_MAX_CCOUNT;
    if (state->mppe) {
	if (state->stateless) {
	    rtl_mppe_change_key(state, 0);
	    state->nextflushed = 1;
	} else {
	    if ((state->ccount & 0xff) == 0xff) {
		rtl_mppe_change_key(state, 0);
	    }
	}
    }
}

/* allocate space for a MPPE/MPPC (de)compressor.  */
/*   comp != 0 -> init compressor */
/*   comp = 0 -> init decompressor */
static void *
rtl_mppe_alloc(unsigned char *options, int opt_len, int comp)
{
    struct ppp_mppe_state *state;
    u8* fname;
    fname = comp ? "mppe_comp_alloc" : "mppe_decomp_alloc";
	
    if (opt_len < CILEN_MPPE) {
	diag_printf("%s: wrong options length: %u\n", fname, opt_len);
	return NULL;
    }

    if (options[0] != CI_MPPE || options[1] != CILEN_MPPE ||
	(options[2] & ~MPPE_STATELESS) != 0 ||
	options[3] != 0 || options[4] != 0 ||
	(options[5] & ~(MPPE_128BIT|MPPE_56BIT|MPPE_40BIT|MPPE_MPPC)) != 0 ||
	(options[5] & (MPPE_128BIT|MPPE_56BIT|MPPE_40BIT|MPPE_MPPC)) == 0) {
	diag_printf("%s: options rejected: o[0]=%02x, o[1]=%02x, "
	       "o[2]=%02x, o[3]=%02x, o[4]=%02x, o[5]=%02x\n", fname, options[0],
	       options[1], options[2], options[3], options[4], options[5]);
	return NULL;
    }
    state = (struct ppp_mppe_state *)Malloc(MB_PHYS,sizeof(struct ppp_mppe_state));
    if (state == NULL) {
	diag_printf("%s: cannot allocate space for %scompressor\n", fname,
	       comp ? "" : "de");
	return NULL;
    }
    memset(state, 0, sizeof(struct ppp_mppe_state));

    state->mppc = options[5] & MPPE_MPPC;	/* Do we use MPPC? */
    state->mppe = options[5] & (MPPE_128BIT | MPPE_56BIT |MPPE_40BIT);/* Do we use MPPE? */

    if (state->mppc) 
	{
		/* allocate MPPC history */
		state->hist = (u8*) Malloc(MB_PHYS,2*MPPE_HIST_LEN*sizeof(u8));
		if (state->hist == NULL) {
			Freee(state);
		    diag_printf("%s: cannot allocate space for MPPC history\n",fname);
		    return NULL;
		}

		/* allocate hashtable for MPPC compressor */
		if (comp) {
		    state->hash = (u16*)Malloc(MB_PHYS,MPPE_HIST_LEN*sizeof(u16));
		    if (state->hash == NULL) {
			Freee(state->hist);
			Freee(state);
			diag_printf("%s: cannot allocate space for MPPC history\n",fname);
			return NULL;
		    }
		}
    }

    if (state->mppe) /* specific for MPPE */
	{ 
		(state->arc4_tfm).crt_ctx = Malloc(MB_PHYS,sizeof(struct rtl_arc4_ctx));		
		if ((state->arc4_tfm).crt_ctx == NULL) {
		    Freee(state->hash);
		    Freee(state->hist);
		    Freee(state);
		    diag_printf("%s: cannot malloc\n", fname);
		    return NULL;
		}
		memcpy(state->master_key, options+CILEN_MPPE, MPPE_MAX_KEY_LEN);
		memcpy(state->session_key, state->master_key, MPPE_MAX_KEY_LEN);
    }
    return (void *) state;
}

static void *
mppe_comp_alloc(unsigned char *options, int opt_len)
{	
    return rtl_mppe_alloc(options, opt_len, 1);
}

static void *
mppe_decomp_alloc(unsigned char *options, int opt_len)
{
    return rtl_mppe_alloc(options, opt_len, 0);
}

/* cleanup the (de)compressor */
static void
mppe_comp_free(void *arg)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;

    if (state != NULL) 
	{
		if (state->mppe) {
		    if ((state->arc4_tfm).crt_ctx != NULL)
				Freee((state->arc4_tfm).crt_ctx);
		}
		if (state->hist != NULL)
		    Freee(state->hist);
		if (state->hash != NULL)
		    Freee(state->hash);
		Freee(state);
    }
}

/* init MPPC/MPPE (de)compresor */
/*   comp != 0  -------> init compressor */
/*   comp == 0  -------> init decompressor */
static int
mppe_init(void *arg, unsigned char *options, int opt_len, int unit,
	  int hdrlen, int mru, int debug, int comp)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
    u8* fname;

    fname = comp ? "mppe_comp_init" : "mppe_decomp_init";

    if (opt_len < CILEN_MPPE) {
	if (debug)
	    diag_printf("%s: wrong options length: %u\n",fname, opt_len);
	return 0;
    }

    if (options[0] != CI_MPPE || options[1] != CILEN_MPPE ||
	(options[2] & ~MPPE_STATELESS) != 0 ||
	options[3] != 0 || options[4] != 0 ||
	(options[5] & ~(MPPE_56BIT|MPPE_128BIT|MPPE_40BIT|MPPE_MPPC)) != 0 ||
	(options[5] & (MPPE_56BIT|MPPE_128BIT|MPPE_40BIT|MPPE_MPPC)) == 0) {
	if (debug)
	    diag_printf("%s: options rejected: o[0]=%02x, o[1]=%02x, "
		   "o[2]=%02x, o[3]=%02x, o[4]=%02x, o[5]=%02x\n", fname,
		   options[0], options[1], options[2], options[3], options[4],
		   options[5]);
	return 0;
    }

    if ((options[5] & ~MPPE_MPPC) != MPPE_128BIT &&
	(options[5] & ~MPPE_MPPC) != MPPE_56BIT &&
	(options[5] & ~MPPE_MPPC) != MPPE_40BIT &&
	(options[5] & MPPE_MPPC) != MPPE_MPPC) {
	if (debug)
	    diag_printf("%s: don't know what to do: o[5]=%02x\n",
		   fname, options[5]);
	return 0;
    }

    state->mppc = options[5] & MPPE_MPPC;	/* Do we use MPPC? */
    state->mppe = options[5] & (MPPE_128BIT | MPPE_56BIT |
	MPPE_40BIT);				/* Do we use MPPE? */
   	state->stateless = options[2] & MPPE_STATELESS; /* Do we use stateless mode? */
	backup_stateless = state->stateless ;
    switch (state->mppe) {
    case MPPE_40BIT:     /* 40 bit key */
	state->keylen = 8;
	state->bitkeylen = 40;
	break;
    case MPPE_56BIT:     /* 56 bit key */
	state->keylen = 8;
	state->bitkeylen = 56;
	break;
    case MPPE_128BIT:    /* 128 bit key */
	state->keylen = 16;
	state->bitkeylen = 128;
	break;
    default:
	state->keylen = 0;
	state->bitkeylen = 0;
    }

    state->ccount = MPPE_MAX_CCOUNT;
    state->bits = 0;
    state->unit  = unit;
    state->debug = debug;
	//dzh add for open the debug output info for mppc/mppe
#ifdef MPPC_MPPE_DEBUG	
	state->debug = 1;
#endif
    state->histptr = MPPE_HIST_LEN;
    if (state->mppc) {	/* reset history if MPPC was negotiated */
	memset(state->hist, 0, 2*MPPE_HIST_LEN*sizeof(u8));
    }

    if (state->mppe) { /* generate initial session keys */
		rtl_mppe_change_key(state, 1);
    }

    if (comp) { /* specific for compressor */
	state->nextflushed = 1;
    } else { /* specific for decompressor */
	state->mru = mru;
	state->flushexpected = 1;
    }
    return 1;
}

static int
mppe_comp_init(void *arg, unsigned char *options, int opt_len, int unit,
	       int hdrlen, int debug)
{
    return mppe_init(arg, options, opt_len, unit, hdrlen, 0, debug, 1);
}

static int
mppe_decomp_init(void *arg, unsigned char *options, int opt_len, int unit,
		 int hdrlen, int mru, int debug)
{
    return mppe_init(arg, options, opt_len, unit, hdrlen, mru, debug, 0);
}

static void
mppe_comp_reset(void *arg)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;

    if (state->debug)
		diag_printf("%s%d: resetting MPPC/MPPE compressor\n",__FUNCTION__, state->unit);

    state->nextflushed = 1;
    if (state->mppe)
	rtl_arc4_setkey(state, state->session_key, state->keylen);
}

static void
mppe_decomp_reset(void *arg)
{
    return;
}

static void
mppe_stats(void *arg, struct compstat *stats)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;
    *stats = state->stats;
}

/* inserts 1 to 8 bits into the output buffer */
static inline void putbits8(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n) {
	*l = (*l) - n;
	val <<= *l;
	*buf = *buf | (val & 0xff);
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++;
	*l = 8 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 8) & 0xff);
	*(++buf) = val & 0xff;
    }
}

/* inserts 9 to 16 bits into the output buffer */
static inline void putbits16(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n - 8) {
	(*i)++;
	*l = 8 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 8) & 0xff);
	*(++buf) = val & 0xff;
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++; (*i)++;
	*l = 16 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 16) & 0xff);
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
    }
}

/* inserts 17 to 24 bits into the output buffer */
static inline void putbits24(u8 *buf, u32 val, const u32 n, u32 *i, u32 *l)
{
    buf += *i;
    if (*l >= n - 16) {
	(*i)++; (*i)++;
	*l = 16 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 16) & 0xff);
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	    *(++buf) = 0;
	}
    } else {
	(*i)++; (*i)++; (*i)++;
	*l = 24 - n + (*l);
	val <<= *l;
	*buf = *buf | ((val >> 24) & 0xff);
	*(++buf) = (val >> 16) & 0xff;
	*(++buf) = (val >> 8) & 0xff;
	*(++buf) = val & 0xff;
    }
}

static int
mppc_compress(struct ppp_mppe_state *state, unsigned char *ibuf,
	      unsigned char *obuf, int isize, int osize)
{
    u32 olen, off, len, idx, i, l;
    u8 *hist, *sbuf, *p, *q, *r, *s;

    /*
  	Maximum MPPC packet expansion is 12.5%((((isize*9)/8)+1)+2). This is the worst case when
  	all octets in the input buffer are >= 0x80 and we cannot find any
  	repeated tokens. Additionally we have to reserve 2 bytes for MPPE/MPPC
  	status bits and coherency counter.
  	*/
	if((((isize * 9) / 8)) > osize)
	{
		diag_printf("\n... May buffer overflow \n\n");
	}

    hist = state->hist + MPPE_HIST_LEN;
    /* check if there is enough room at the end of the history */
    if (state->histptr + isize >= 2*MPPE_HIST_LEN) {
	state->bits |= MPPE_BIT_RESET;
	state->histptr = MPPE_HIST_LEN;
	memcpy(state->hist, hist, MPPE_HIST_LEN);
    }
    /* add packet to the history; isize must be <= MPPE_HIST_LEN */
    sbuf = state->hist + state->histptr;
    memcpy(sbuf, ibuf, isize);
    state->histptr += isize;

    /* compress data */
    r = sbuf + isize;
    *obuf = olen = i = 0;
    l = 8;
    while (i < isize - 2) {
	s = q = sbuf + i;
	idx = ((40543*((((s[0]<<4)^s[1])<<4)^s[2]))>>4) & 0x1fff;
	p = hist + state->hash[idx];
	state->hash[idx] = (u16) (s - hist);
	off = s - p;
	if (off > MPPE_HIST_LEN - 1 || off < 1 || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++) {
	    /* no match found; encode literal byte */
	    if (ibuf[i] < 0x80) {		/* literal byte < 0x80 */
		putbits8(obuf, (u32) ibuf[i], 8, &olen, &l);
	    } else {				/* literal byte >= 0x80 */
		putbits16(obuf, (u32) (0x100|(ibuf[i]&0x7f)), 9, &olen, &l);
	    }
	    ++i;
	    continue;
	}
	if (r - q >= 64) {
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++ || *p++ != *s++ || *p++ != *s++ || *p++ != *s++ ||
	    *p++ != *s++;
	    if (s - q == 64) {
		p--; s--;
		while((*p++ == *s++) && (s < r) && (p < q));
	    }
	} else {
	    while((*p++ == *s++) && (s < r) && (p < q));
	}
	len = s - q - 1;
	i += len;

	/* at least 3 character match found; code data */
	/* encode offset */
	if (off < 64) {			/* 10-bit offset; 0 <= offset < 64 */
	    putbits16(obuf, 0x3c0|off, 10, &olen, &l);
	} else if (off < 320) {		/* 12-bit offset; 64 <= offset < 320 */
	    putbits16(obuf, 0xe00|(off-64), 12, &olen, &l);
	} else if (off < 8192) {	/* 16-bit offset; 320 <= offset < 8192 */
	    putbits16(obuf, 0xc000|(off-320), 16, &olen, &l);
	} else {
	    /* This shouldn't happen; we return 0 what means "packet expands",
 	    and we send packet uncompressed. */
	    if (state->debug)
		diag_printf("%s%d: wrong offset value: %d\n",
		       __FUNCTION__, state->unit, off);
	    return 0;
	}
	/* encode length of match */
	if (len < 4) {			/* length = 3 */
	    putbits8(obuf, 0, 1, &olen, &l);
	} else if (len < 8) {		/* 4 <= length < 8 */
	    putbits8(obuf, 0x08|(len&0x03), 4, &olen, &l);
	} else if (len < 16) {		/* 8 <= length < 16 */
	    putbits8(obuf, 0x30|(len&0x07), 6, &olen, &l);
	} else if (len < 32) {		/* 16 <= length < 32 */
	    putbits8(obuf, 0xe0|(len&0x0f), 8, &olen, &l);
	} else if (len < 64) {		/* 32 <= length < 64 */
	    putbits16(obuf, 0x3c0|(len&0x1f), 10, &olen, &l);
	} else if (len < 128) {		/* 64 <= length < 128 */
	    putbits16(obuf, 0xf80|(len&0x3f), 12, &olen, &l);
	} else if (len < 256) {		/* 128 <= length < 256 */
	    putbits16(obuf, 0x3f00|(len&0x7f), 14, &olen, &l);
	} else if (len < 512) {		/* 256 <= length < 512 */
	    putbits16(obuf, 0xfe00|(len&0xff), 16, &olen, &l);
	} else if (len < 1024) {	/* 512 <= length < 1024 */
	    putbits24(obuf, 0x3fc00|(len&0x1ff), 18, &olen, &l);
	} else if (len < 2048) {	/* 1024 <= length < 2048 */
	    putbits24(obuf, 0xff800|(len&0x3ff), 20, &olen, &l);
	} else if (len < 4096) {	/* 2048 <= length < 4096 */
	    putbits24(obuf, 0x3ff000|(len&0x7ff), 22, &olen, &l);
	} else if (len < 8192) {	/* 4096 <= length < 8192 */
	    putbits24(obuf, 0xffe000|(len&0xfff), 24, &olen, &l);
	} else {
	    /* This shouldn't happen; we return 0 what means "packet expands",
 	    and send packet uncompressed. */
	    if (state->debug)
		diag_printf("%s%d: wrong length of match value: %d\n",
		       __FUNCTION__, state->unit, len);
	    return 0;
	}
    }

    /* Add remaining octets to the output */
    while(isize - i > 0) {
	if (ibuf[i] < 0x80) {	/* literal byte < 0x80 */
	    putbits8(obuf, (u32) ibuf[i++], 8, &olen, &l);
	} else {		/* literal byte >= 0x80 */
	    putbits16(obuf, (u32) (0x100|(ibuf[i++]&0x7f)), 9, &olen, &l);
	}
    }
    /* Reset unused bits of the last output octet */
    if ((l != 0) && (l != 8)) {
	putbits8(obuf, 0, l, &olen, &l);
    }
    return (int) olen;
}

#define INPUT_Buf 2000
#define OUTPUT_Buf INPUT_Buf
unsigned char comp_input_buf[INPUT_Buf];
unsigned char comp_output_buf[OUTPUT_Buf];

unsigned char decomp_input_buf[INPUT_Buf];
unsigned char decomp_output_buf[OUTPUT_Buf];

int
mppe_compress(
	void* arg,			/*state */
	struct mbuf** mret,		/*return comressed mbuf chain here*/
	struct mbuf *m_input, 		/*from here*/
	int orig_len, 			/*uncompressed length*/
	int max_len				/*max compressed length*/
	)				
{
	#define	M_FRAG		0x0400

    struct ppp_mppe_state *state = (struct ppp_mppe_state *) arg;
    int proto, olen, complen, off;
    unsigned char *wptr;
	int malloc_flag = 0;
	
	int isize,osize;
	u_char * ibuf,*obuf;
    u_char *cp, adrs, ctrl;
    u_char *p, *rptr ;
    struct mbuf *m, *dmp, *mp, *n;
	int multi_mbuf = 0;
	*mret = NULL;

	//if(m_input->m_next != NULL && (m_input->m_len != m_input->m_pkthdr.len))
//	diag_printf("%s,%d.m_flags(%d)\n",__FUNCTION__,__LINE__,m_input->m_flags);	
	if( (m_input->m_flags & M_FRAG) || (!(m_input->m_flags & M_EXT)))	
//	if((!(m_input->m_flags & M_EXT)))
	{
		int offset = 0;
		struct mbuf * tmp;
		multi_mbuf = 1;		//multiple mbuf flag
		memset(comp_input_buf,0,sizeof(comp_input_buf));
		ibuf = &comp_input_buf;
 		for (tmp = m_input; (tmp != NULL) && (offset < orig_len); tmp = tmp->m_next)
 		{
			memcpy(ibuf+offset, tmp->m_data, tmp->m_len);
			offset+= tmp->m_len;
		}
	}
	else
	{
		//diag_printf("%s,%d. M_ext type\n",__FUNCTION__,__LINE__);
		multi_mbuf = 0;		
		ibuf = mtod(m_input, u_char *);			//input buffer
	}	
	isize = orig_len;
	
    /* Check that the protocol is in the range we handle. */
    proto = PPP_PROTOCOL(ibuf);
    if (proto < 0x0021 || proto > 0x00fa)
    {
		return 0;
    }
	obuf = (u_char *)(&comp_output_buf);
	osize = OUTPUT_Buf;

    wptr = obuf;
    /* Copy over the PPP header */
    wptr[0] = PPP_ADDRESS(ibuf);
    wptr[1] = PPP_CONTROL(ibuf);
    wptr[2] = PPP_COMP >> 8;
    wptr[3] = PPP_COMP;
    wptr += PPP_HDRLEN + (MPPE_OVHD / 2); /* Leave two octets for MPPE/MPPC bits */
	
    //off = (proto > 0xff) ? 2 : 3; /* PFC - skip first protocol byte if 0 */
    off = 2;
    ibuf += off;

    rtl_mppe_increase_ccount(state);

    if (state->nextflushed) {
	state->bits |= MPPE_BIT_FLUSHED;
	state->nextflushed = 0;
	if (state->mppe && !state->stateless) {
	    /*
 	     * If this is the flag packet, the key has been already changed in
 	     * rtl_mppe_increase_ccount() so we dont't do it once again.
 	     */
	    if ((state->ccount & 0xff) != 0xff) {
		rtl_arc4_setkey(state, state->session_key, state->keylen);
	    }
	}
	if (state->mppc) { /* reset history */		
	    state->bits |= MPPE_BIT_RESET;
	    state->histptr = MPPE_HIST_LEN;
	    memset(state->hist + MPPE_HIST_LEN, 0, MPPE_HIST_LEN*sizeof(u8));
	}
    }


    if (state->mppc && !state->mppe) { /* Do only compression */
	complen = mppc_compress(state, ibuf, wptr, isize - off,
				osize - PPP_HDRLEN - (MPPE_OVHD / 2));
//	complen += PPP_HDRLEN + (MPPE_OVHD / 2);

	/*
 	 * TODO: Implement an heuristics to handle packet expansion in a smart
 	 * way. Now, when a packet expands, we send it as uncompressed and
 	 * when next packet is sent we have to reset compressor's history.
 	 * Maybe it would be better to send such packet as compressed in order
 	 * to keep history's continuity.
 	 */
	if ((complen > isize) || (complen > osize - PPP_HDRLEN) ||
	    (complen == 0) ) {		

	    /* packet expands */
	    state->nextflushed = 1;
		//may memcpy over flow!!!!!!!!!!
		//if out buf <= input buffer
		if((osize - PPP_HDRLEN - (MPPE_OVHD / 2)) > isize -off)
	    {
	    	memcpy(wptr, ibuf, isize - off);
		}
		else
		{
			return 0;
		}
	    olen = isize - (off - 2) + MPPE_OVHD;
		//dzh add for compute the comp length!
		complen = isize - off + PPP_HDRLEN + (MPPE_OVHD / 2);		
	    (state->stats).inc_bytes += olen;
	    (state->stats).inc_packets++;
	} else {
	    state->bits |= MPPE_BIT_COMP;
	    olen = complen + PPP_HDRLEN + (MPPE_OVHD / 2);
		//dzh add for compute the comp length!
		complen += PPP_HDRLEN + (MPPE_OVHD / 2);
	    (state->stats).comp_bytes += olen;
	    (state->stats).comp_packets++;
	}	
    } else { /* Do encryption with or without compression */
	state->bits |= MPPE_BIT_ENCRYPTED;	
	if (!state->mppc && state->mppe) { /* Do only encryption */
	    /* read from ibuf, write to wptr, adjust for PPP_HDRLEN */
	    rtl_arc4_encrypt(state, ibuf, isize - off, wptr);		
	    olen = isize - (off - 2) + MPPE_OVHD;		
		//reset the packet header length! dzh modify.
		complen = isize - off + PPP_HDRLEN + (MPPE_OVHD / 2);		
	    (state->stats).inc_bytes += olen;
	    (state->stats).inc_packets++;
	} else { /* Do compression and then encryption - RFC3078 */
	    complen = mppc_compress(state, ibuf, wptr, isize - off,
				    osize - PPP_HDRLEN - (MPPE_OVHD / 2));
	    if ((complen > isize) || (complen > osize - PPP_HDRLEN) ||
		(complen == 0)) {
		/* packet expands */
		state->nextflushed = 1;
		rtl_arc4_encrypt(state, ibuf, isize - off, wptr);
		olen = isize - (off - 2) + MPPE_OVHD;
		//dzh add for compute the comp length!
		complen = isize - off + PPP_HDRLEN + (MPPE_OVHD / 2);
		(state->stats).inc_bytes += olen;
		(state->stats).inc_packets++;
	    } else {
		state->bits |= MPPE_BIT_COMP;
		rtl_arc4_encrypt(state, wptr, complen, wptr);
		olen = complen + PPP_HDRLEN + (MPPE_OVHD / 2);		
		//reset the packet header length! dzh modify.
		complen = complen + PPP_HDRLEN + (MPPE_OVHD / 2);		
		(state->stats).comp_bytes += olen;
		(state->stats).comp_packets++;
	    }
	}
    }

    /* write status bits and coherency counter into the output buffer */
    wptr = obuf + PPP_HDRLEN;
    wptr[0] = MPPE_CTRLHI(state);
    wptr[1] = MPPE_CTRLLO(state);

    state->bits = 0;

    (state->stats).unc_bytes += isize;
    (state->stats).unc_packets++;

		/* copy the compress data to original mbuf.
		*
		*/
#define	MPPC_MPPE_ALIGN(m, len) do {						\
	(m)->m_data += (MCLBYTES - (len)) & ~(sizeof(long) - 1);		\
} while (0)									

	if(multi_mbuf)
	{
		MGETHDR(dmp, M_DONTWAIT, MT_DATA);
		if (dmp == NULL)
		{
			diag_printf("%s,%d, MGETHDR Error\n\n",__FUNCTION__,__LINE__);
			return 0;
		}
		mp = dmp;
		if(complen >=MINCLSIZE){
		 	dmp->m_next = NULL;
			MCLGET(dmp, M_DONTWAIT);			
			MPPC_MPPE_ALIGN(dmp,complen);
		}
		else{			
			MH_ALIGN(dmp, complen);		
		}
		
		dmp->m_pkthdr.len = complen;
		dmp->m_len = complen;
		memcpy(dmp->m_data,comp_output_buf,complen); 		
		//free the original packet.
		//m_freem(m_input);
	}
	else
	{	
		mp = m_input;
		if(m_input->m_flags & M_EXT)
		{
			#if 0
			m_input->m_len = 0; 
			m_input->m_pkthdr.len = 0;
			m_input->m_next = NULL;			
			bzero(&(m_input)->m_pkthdr, sizeof((m_input)->m_pkthdr));
			#endif
			
			if (m_input->m_ext.ext_buf != NULL) 
				m_input->m_data = m_input->m_ext.ext_buf;
			else
			{
				diag_printf("%s,%d, ext_buf error\n\n",__FUNCTION__,__LINE__);
				return 0 ;
			}
			
			// align the packet !!!
			MPPC_MPPE_ALIGN(m_input,complen);
			m_input->m_pkthdr.len = complen;
			m_input->m_len = complen;
			m_input->m_next = NULL;
			memcpy(m_input->m_data,comp_output_buf,complen);
		}
		else if((m_input->m_flags & M_PKTHDR) != 0)
		{
			#if 0
			m_input->m_len = 0 ;
			m_input->m_pkthdr.len = 0;			
			m_input->m_next = NULL;						
			bzero(&(m_input)->m_pkthdr, sizeof((m_input)->m_pkthdr));
			#endif
			
			m_input->m_data = m_input->m_pktdat;			

			// align the packet !!!			
			MH_ALIGN(m_input, complen);		
			m_input->m_pkthdr.len = complen;
			m_input->m_len = complen;
			m_input->m_next = NULL;
			memcpy(m_input->m_data,comp_output_buf,complen);
		}
		else{
			diag_printf("%s,%d, should't happened\n\n",__FUNCTION__,__LINE__);
			return 0;
		}
		
	}

	*mret = mp;
    return olen;
}

/***************************/
/*** Decompression stuff ***/
/***************************/
static inline u32 getbits(const u8 *buf, const u32 n, u32 *i, u32 *l)
{
    static const u32 m[] = {0x00, 0x01, 0x03, 0x07, 0x0f, 0x1f, 0x3f, 0x7f, 0xff};
    u32 res, ol;

    ol = *l;
    if (*l >= n) {
	*l = (*l) - n;
	res = (buf[*i] & m[ol]) >> (*l);
	if (*l == 0) {
	    *l = 8;
	    (*i)++;
	}
    } else {
	*l = 8 - n + (*l);
	res = (buf[(*i)++] & m[ol]) << 8;
	res = (res | buf[*i]) >> (*l);
    }

    return res;
}

static inline u32 getbyte(const u8 *buf, const u32 i, const u32 l)
{
    if (l == 8) {
	return buf[i];
    } else {
	return (((buf[i] << 8) | buf[i+1]) >> l) & 0xff;
    }
}

static inline void lamecopy(u8 *dst, u8 *src, u32 len)
{
    while (len--)
	*dst++ = *src++;
}

static int
mppc_decompress(struct ppp_mppe_state *state, unsigned char *ibuf,
		unsigned char *obuf, int isize, int osize)
{
    u32 olen, off, len, bits, val, sig, i, l;
    u8 *history, *s;
    history = state->hist + state->histptr;
    olen = len = i = 0;
    l = 8;
    bits = isize * 8;
    while (bits >= 8) {
	val = getbyte(ibuf, i++, l);
	if (val < 0x80) {		/* literal byte < 0x80 */
	    if (state->histptr < 2*MPPE_HIST_LEN) {
		/* copy uncompressed byte to the history */
		(state->hist)[(state->histptr)++] = (u8) val;
	    } else {
		/* buffer overflow; drop packet */
		if (state->debug)
		    diag_printf("%s%d: trying to write outside history "
			   "buffer\n", __FUNCTION__, state->unit);
		return DECOMP_ERROR;
	    }
	    olen++;
	    bits -= 8;
	    continue;
	}

	sig = val & 0xc0;
	if (sig == 0x80) {		/* literal byte >= 0x80 */
	    if (state->histptr < 2*MPPE_HIST_LEN) {
		/* copy uncompressed byte to the history */
		(state->hist)[(state->histptr)++] = 
		    (u8) (0x80|((val&0x3f)<<1)|getbits(ibuf, 1 , &i ,&l));
	    } else {
		/* buffer overflow; drop packet */
		if (state->debug)
		    diag_printf("%s%d: trying to write outside history "
			   "buffer\n", __FUNCTION__, state->unit);
		return DECOMP_ERROR;
	    }
	    olen++;
	    bits -= 9;
	    continue;
	}

	/* Not a literal byte so it must be an (offset,length) pair */
	/* decode offset */
	sig = val & 0xf0;
	if (sig == 0xf0) {		/* 10-bit offset; 0 <= offset < 64 */
	    off = (((val&0x0f)<<2)|getbits(ibuf, 2 , &i ,&l));
	    bits -= 10;
	} else {
	    if (sig == 0xe0) {		/* 12-bit offset; 64 <= offset < 320 */
		off = ((((val&0x0f)<<4)|getbits(ibuf, 4 , &i ,&l))+64);
		bits -= 12;
	    } else {
		if ((sig&0xe0) == 0xc0) {/* 16-bit offset; 320 <= offset < 8192 */
		    off = ((((val&0x1f)<<8)|getbyte(ibuf, i++, l))+320);
		    bits -= 16;
		    if (off > MPPE_HIST_LEN - 1) {
			if (state->debug)
			    diag_printf("%s%d: too big offset value: %d\n",
				   __FUNCTION__, state->unit, off);
			return DECOMP_ERROR;
		    }
		} else {		/* this shouldn't happen */
		    if (state->debug)
			diag_printf("%s%d: cannot decode offset value\n",
			       __FUNCTION__, state->unit);
		    return DECOMP_ERROR;
		}
	    }
	}
	/* decode length of match */
	val = getbyte(ibuf, i, l);
	if ((val & 0x80) == 0x00) {			/* len = 3 */
	    len = 3;
	    bits--;
	    getbits(ibuf, 1 , &i ,&l);
	} else if ((val & 0xc0) == 0x80) {		/* 4 <= len < 8 */
	    len = 0x04 | ((val>>4) & 0x03);
	    bits -= 4;
	    getbits(ibuf, 4 , &i ,&l);
	} else if ((val & 0xe0) == 0xc0) {		/* 8 <= len < 16 */
	    len = 0x08 | ((val>>2) & 0x07);
	    bits -= 6;
	    getbits(ibuf, 6 , &i ,&l);
	} else if ((val & 0xf0) == 0xe0) {		/* 16 <= len < 32 */
	    len = 0x10 | (val & 0x0f);
	    bits -= 8;
	    i++;
	} else {
	    bits -= 8;
	    val = (val << 8) | getbyte(ibuf, ++i, l);
	    if ((val & 0xf800) == 0xf000) {		/* 32 <= len < 64 */
		len = 0x0020 | ((val >> 6) & 0x001f);
		bits -= 2;
		getbits(ibuf, 2 , &i ,&l);
	    } else if ((val & 0xfc00) == 0xf800) {	/* 64 <= len < 128 */
		len = 0x0040 | ((val >> 4) & 0x003f);
		bits -= 4;
		getbits(ibuf, 4 , &i ,&l);
	    } else if ((val & 0xfe00) == 0xfc00) {	/* 128 <= len < 256 */
		len = 0x0080 | ((val >> 2) & 0x007f);
		bits -= 6;
		getbits(ibuf, 6 , &i ,&l);
	    } else if ((val & 0xff00) == 0xfe00) {	/* 256 <= len < 512 */
		len = 0x0100 | (val & 0x00ff);
		bits -= 8;
		i++;
	    } else {
		bits -= 8;
		val = (val << 8) | getbyte(ibuf, ++i, l);
		if ((val & 0xff8000) == 0xff0000) {	/* 512 <= len < 1024 */
		    len = 0x000200 | ((val >> 6) & 0x0001ff);
		    bits -= 2;
		    getbits(ibuf, 2 , &i ,&l);
		} else if ((val & 0xffc000) == 0xff8000) {/* 1024 <= len < 2048 */
		    len = 0x000400 | ((val >> 4) & 0x0003ff);
		    bits -= 4;
		    getbits(ibuf, 4 , &i ,&l);
		} else if ((val & 0xffe000) == 0xffc000) {/* 2048 <= len < 4096 */
		    len = 0x000800 | ((val >> 2) & 0x0007ff);
		    bits -= 6;
		    getbits(ibuf, 6 , &i ,&l);
		} else if ((val & 0xfff000) == 0xffe000) {/* 4096 <= len < 8192 */
		    len = 0x001000 | (val & 0x000fff);
		    bits -= 8;
		    i++;
		} else {				/* this shouldn't happen */
		    if (state->debug)
			diag_printf("%s%d: wrong length code: 0x%X\n",
			       __FUNCTION__, state->unit, val);
		    return DECOMP_ERROR;
		}
	    }
	}
	s = state->hist + state->histptr;
	state->histptr += len;
	olen += len;
	if (state->histptr < 2*MPPE_HIST_LEN) {
	    /* copy uncompressed bytes to the history */
	    lamecopy(s, s - off, len);
	} else {
	    /* buffer overflow; drop packet */
	    if (state->debug)
		diag_printf("%s%d: trying to write outside history "
		       "buffer\n", __FUNCTION__, state->unit);
	    return DECOMP_ERROR;
	}
    }

    /* Do PFC decompression */
    len = olen;
    if ((history[0] & 0x01) != 0) {
	obuf[0] = 0;
	obuf++;
	len++;
    }
	
    if (len <= osize) {
	/* copy uncompressed packet to the output buffer */
	
	memcpy(obuf, history, olen);
    } else {
	/* buffer overflow; drop packet */
	if (state->debug)
	    diag_printf("%s%d: too big uncompressed packet: %d\n",
		   __FUNCTION__, state->unit, len + (PPP_HDRLEN / 2));
	return DECOMP_ERROR;
    }
#if 0
	{
		int i = 0;
		
		diag_printf("\n decompress value as such \n");
		for(i = 0 ;i < 32; ++i)
		{
			diag_printf("%x ",obuf[i]);
			if(i ==15)
				diag_printf("\n");
		}
		diag_printf("\n decompress value end **********\n");
	}
	
	diag_printf("{%s,%d},len value is:%d\n",__FUNCTION__,__LINE__,len);
#endif	
    return (int) len;
}

int
mppe_decompress(void *arg, struct mbuf *cmp, struct mbuf **dmpp)
{
    struct ppp_mppe_state *state = (struct ppp_mppe_state *)arg;
    int seq, bits, uncomplen;
	int isize,osize;
	u_char * ibuf,*obuf;
    u_char *cp, adrs, ctrl;
    u_char *p, *rptr ;
    struct mbuf *m, *dmp, *mret, *n;

    struct mbuf * m_input;
	m_input = cmp;
	int multi_mbuf = 0;

	int malloc_flag = 0;	
	isize = cmp->m_pkthdr.len;
	/*
	 * Prepare the input buffer.
	 */
	//if(cmp->m_next != NULL /*&& (cmp->m_len != isize)*/)	 
	if(!(m_input->m_flags & M_EXT))
	{
		int offset = 0;
		struct mbuf * tmp;
		multi_mbuf = 1;
		memset(decomp_input_buf,0,sizeof(decomp_input_buf));
		ibuf = &decomp_input_buf;
		for (tmp = cmp; (tmp != NULL) && (offset < isize); tmp = tmp->m_next)
		{
			memcpy(ibuf+offset, tmp->m_data, tmp->m_len);
			offset+= tmp->m_len;
		}
	}	
	else
	{
		multi_mbuf = 0;
		ibuf = mtod(cmp, u_char *); 		//input buffer
	}
	*dmpp = NULL;	
	obuf = (u_char *)(&decomp_output_buf);
	osize = OUTPUT_Buf;

    if (isize <= PPP_HDRLEN + MPPE_OVHD) {
	if (state->debug) {
	    diag_printf("%s%d: short packet (len=%d)\n",  __FUNCTION__,
		   state->unit, isize);
	}
	return DECOMP_ERROR;
    }

    /* Get coherency counter and control bits from input buffer */
    seq = MPPE_CCOUNT(ibuf);
    bits = MPPE_BITS(ibuf);
	
    if(state->stateless) {		
	/* RFC 3078, sec 8.1. */
	rtl_mppe_increase_ccount(state);
	if ((seq != state->ccount) && state->debug)
	    diag_printf("%s%d: bad sequence number: %d, expected: %d\n",
		   __FUNCTION__, state->unit, seq, state->ccount);
	while (seq != state->ccount){
			rtl_mppe_increase_ccount(state);
		}
    } else {    
	/* RFC 3078, sec 8.2. */
	if (state->flushexpected) { /* discard state */
	    if ((bits & MPPE_BIT_FLUSHED)) { /* we received expected FLUSH bit */
		while (seq != state->ccount)
		    rtl_mppe_increase_ccount(state);
		state->flushexpected = 0;
	    } else /* drop packet*/
	    	{	    	
			return DECOMP_ERROR;
		}
	} else { /* normal state */
	    rtl_mppe_increase_ccount(state);
	    if (seq != state->ccount) {
		/* Packet loss detected, enter the discard state. */
		if (state->debug)
		    diag_printf("%s%d: bad sequence number: %d, expected: %d\n",
			   __FUNCTION__, state->unit, seq, state->ccount);
			state->flushexpected = 1;			
			return DECOMP_ERROR;
	    }
	}
	if (state->mppe && (bits & MPPE_BIT_FLUSHED)) {
	    rtl_arc4_setkey(state, state->session_key, state->keylen);
	}
    }
	
    if (state->mppc && (bits & (MPPE_BIT_FLUSHED | MPPE_BIT_RESET))) {
	state->histptr = MPPE_HIST_LEN;
	if ((bits & MPPE_BIT_FLUSHED)) {
	    memset(state->hist + MPPE_HIST_LEN, 0, MPPE_HIST_LEN*sizeof(u8));
	} else
	    if ((bits & MPPE_BIT_RESET)) {
		memcpy(state->hist, state->hist + MPPE_HIST_LEN, MPPE_HIST_LEN);
	    }
    }

    /* Fill in the first part of the PPP header. The protocol field
       comes from the decompressed data. */
	cp = mtod(cmp, u_char *);
    obuf[0] = PPP_ADDRESS(cp);
    obuf[1] = PPP_CONTROL(cp);
    obuf += PPP_HDRLEN / 2;	

    if (state->mppe) 
	{ /* process encrypted packet */		
		if ((bits & MPPE_BIT_ENCRYPTED)) {
		    /* OK, packet encrypted, so decrypt it */
		    if (state->mppc && (bits & MPPE_BIT_COMP)) {
			rtl_arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2), isize -
				     PPP_HDRLEN - (MPPE_OVHD / 2), ibuf + PPP_HDRLEN +
				     (MPPE_OVHD / 2));		
			uncomplen = mppc_decompress(state, ibuf + PPP_HDRLEN +
						    (MPPE_OVHD / 2), obuf, isize -
						    PPP_HDRLEN - (MPPE_OVHD / 2),
						    osize - (PPP_HDRLEN / 2));
			if (uncomplen == DECOMP_ERROR) {
			    state->flushexpected = 1;
			    return DECOMP_ERROR;
			}
			
			uncomplen += PPP_HDRLEN / 2;
			(state->stats).comp_bytes += isize;
			(state->stats).comp_packets++;
		    } else {

			uncomplen = isize - MPPE_OVHD;
			/* Decrypt the first byte in order to check if it is
	 		   compressed or uncompressed protocol field */
			rtl_arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2), 1, obuf);
			/* Do PFC decompression */
			if ((obuf[0] & 0x01) != 0) {
			    obuf[1] = obuf[0];
			    obuf[0] = 0;
			    obuf++;
			    uncomplen++;
			}
			
			/* And finally, decrypt the rest of the frame. */
			rtl_arc4_decrypt(state, ibuf + PPP_HDRLEN +	(MPPE_OVHD / 2) + 1,
				     isize - PPP_HDRLEN - (MPPE_OVHD / 2) - 1, obuf + 1);

			//uncomplen += PPP_HDRLEN / 2;			
			//reset the packet header length! dzh modify.
			uncomplen = isize - PPP_HDRLEN ;
			(state->stats).inc_bytes += isize;
			(state->stats).inc_packets++;
		    }
		}
		else 
		{ 
			/* this shouldn't happen */
		    if (state->debug)
			diag_printf("%s%d: encryption negotiated but not an "
			       "encrypted packet received\n", __FUNCTION__, state->unit);
		    rtl_mppe_change_key(state, 0);
		    state->flushexpected = 1;
		    return DECOMP_ERROR;
		}
	} 
	else 
	{
		if (state->mppc) 
		{ /* no MPPE, only MPPC */
		    if ((bits & MPPE_BIT_COMP)) 
			{
				uncomplen = mppc_decompress(state, ibuf + PPP_HDRLEN +
							    (MPPE_OVHD / 2), obuf, isize -
							    PPP_HDRLEN - (MPPE_OVHD / 2),
							    osize - (PPP_HDRLEN / 2));
				if (uncomplen == DECOMP_ERROR) {
			   		state->flushexpected = 1;	
				    return DECOMP_ERROR;
				}
				
				uncomplen += PPP_HDRLEN / 2;				
				(state->stats).comp_bytes += isize;
				(state->stats).comp_packets++;
		    }
			else
			{				
				memcpy(obuf, ibuf + PPP_HDRLEN + (MPPE_OVHD / 2), isize -
				       PPP_HDRLEN - (MPPE_OVHD / 2));				
				uncomplen = isize - MPPE_OVHD;
				(state->stats).inc_bytes += isize;
				(state->stats).inc_packets++;
		    }		
		} 
		else 
		{ /* this shouldn't happen */
		    if (state->debug)
			diag_printf("%s%d: error - not an  MPPC or MPPE frame "
			       "received\n", __FUNCTION__, state->unit);
		    state->flushexpected = 1;
		    return DECOMP_ERROR;
		}
	}

    (state->stats).unc_bytes += uncomplen;
    (state->stats).unc_packets++;


	/* copy the compress data to original mbuf.
	*
	*/	
#define	MPPC_MPPE_ALIGN(m, len) do {						\
		(m)->m_data += (MCLBYTES - (len)) & ~(sizeof(long) - 1);		\
	} while (0) 					
	
	if(multi_mbuf)
	{
		MGETHDR(dmp, M_DONTWAIT, MT_DATA);
		if (dmp == NULL)
		{
			diag_printf("%s,%d, error!\n",__FUNCTION__,__LINE__);
			return 0;
		}
		mret = dmp;
		if(uncomplen >=MINCLSIZE){
			dmp->m_next = NULL;
			MCLGET(dmp, M_DONTWAIT);			
			MPPC_MPPE_ALIGN(dmp,uncomplen);
		}
		else{			
			MH_ALIGN(dmp, uncomplen); 	
		}
		
		dmp->m_pkthdr.len = uncomplen;
		dmp->m_len = uncomplen;
		memcpy(dmp->m_data,decomp_output_buf,uncomplen); 		
		//free the original packet.
		//m_freem(m_input);
	}
	else
	{	
		mret = m_input;
		if(m_input->m_flags & M_EXT)
		{
			m_input->m_len = 0; 
			m_input->m_pkthdr.len = 0;
			m_input->m_next = NULL; 		
			bzero(&(m_input)->m_pkthdr, sizeof((m_input)->m_pkthdr));
			if (m_input->m_ext.ext_buf != NULL) 
				m_input->m_data = m_input->m_ext.ext_buf;
			else{
				diag_printf(" Malloc mbuf error\n\n");
				return  -1;
			}
			// align the packet !!!
			MPPC_MPPE_ALIGN(m_input,uncomplen);
			m_input->m_pkthdr.len = uncomplen;
			m_input->m_len = uncomplen;
			memcpy(m_input->m_data,decomp_output_buf,uncomplen);
		}
		else if((m_input->m_flags & M_PKTHDR) != 0)
		{
			m_input->m_len = 0 ;
			m_input->m_pkthdr.len = 0;			
			m_input->m_next = NULL; 					
			bzero(&(m_input)->m_pkthdr, sizeof((m_input)->m_pkthdr));
			m_input->m_data = m_input->m_pktdat;			

			// align the packet !!! 		
			MH_ALIGN(m_input, uncomplen); 	
			m_input->m_pkthdr.len = uncomplen;
			m_input->m_len = uncomplen;			
			memcpy(m_input->m_data,decomp_output_buf,uncomplen);
		}
		else{
			diag_printf(" should't happened \n\n");
			return -1;
		}
	}
	*dmpp = mret;
	return DECOMP_OK;
//  return uncomplen;
}
struct compressor ppp_mppe = {
    .compress_proto =	CI_MPPE,
    .comp_alloc =	mppe_comp_alloc,
    .comp_free =	mppe_comp_free,
    .comp_init =	mppe_comp_init,
    .comp_reset =	mppe_comp_reset,
    .compress =		mppe_compress,
    .comp_stat =	mppe_stats,
    .decomp_alloc =	mppe_decomp_alloc,
    .decomp_free =	mppe_comp_free,
    .decomp_init =	mppe_decomp_init,
    .decomp_reset =	mppe_decomp_reset,
    .decompress =	mppe_decompress,
    .incomp =		NULL,
    .decomp_stat =	mppe_stats,
};

