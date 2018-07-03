#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lm_aes.h"

static inline 
u32 generic_rotr32 (const u32 x, const unsigned bits)
{
    const unsigned n = bits % 32;
    return (x >> n) | (x << (32 - n));
}

static inline 
u32 generic_rotl32 (const u32 x, const unsigned bits)
{
    const unsigned n = bits % 32;
    return (x << n) | (x >> (32 - n));
}

#define rotl generic_rotl32
#define rotr generic_rotr32

/*
 * #define byte(x, nr) ((unsigned char)((x) >> (nr*8))) 
 */
inline static u8
byte(const u32 x, const unsigned n)
{
    return x >> (n << 3);
}

#if 0//luminais mark
#define u32_in(x) le32_to_cpu(*(const u32 *)(x))
#define u32_out(to, from) (*(u32 *)(to) = cpu_to_le32(from))
#else
#define u32_in(x) (*(const u32 *)(x))
#define u32_out(to, from) (*(u32 *)(to) = (from))
#endif

#define E_KEY ctx->E
#define D_KEY ctx->D

static u8 pow_tab[256];
static u8 log_tab[256];
static u8 sbx_tab[256];
static u8 isb_tab[256];
static u32 rco_tab[10];
static u32 ft_tab[4][256];
static u32 it_tab[4][256];

static u32 fl_tab[4][256];
static u32 il_tab[4][256];

static inline u8
f_mult (u8 a, u8 b)
{
    u8 aa = log_tab[a], cc = aa + log_tab[b];

    return pow_tab[cc + (cc < aa ? 1 : 0)];
}

#define ff_mult(a,b)    (a && b ? f_mult(a, b) : 0)

#define f_rn(bo, bi, n, k)                  \
    bo[n] =  ft_tab[0][byte(bi[n],0)] ^             \
             ft_tab[1][byte(bi[(n + 1) & 3],1)] ^       \
             ft_tab[2][byte(bi[(n + 2) & 3],2)] ^       \
             ft_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rn(bo, bi, n, k)                  \
    bo[n] =  it_tab[0][byte(bi[n],0)] ^             \
             it_tab[1][byte(bi[(n + 3) & 3],1)] ^       \
             it_tab[2][byte(bi[(n + 2) & 3],2)] ^       \
             it_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

#define ls_box(x)               \
    ( fl_tab[0][byte(x, 0)] ^           \
      fl_tab[1][byte(x, 1)] ^           \
      fl_tab[2][byte(x, 2)] ^           \
      fl_tab[3][byte(x, 3)] )

#define f_rl(bo, bi, n, k)                  \
    bo[n] =  fl_tab[0][byte(bi[n],0)] ^             \
             fl_tab[1][byte(bi[(n + 1) & 3],1)] ^       \
             fl_tab[2][byte(bi[(n + 2) & 3],2)] ^       \
             fl_tab[3][byte(bi[(n + 3) & 3],3)] ^ *(k + n)

#define i_rl(bo, bi, n, k)                  \
    bo[n] =  il_tab[0][byte(bi[n],0)] ^             \
             il_tab[1][byte(bi[(n + 3) & 3],1)] ^       \
             il_tab[2][byte(bi[(n + 2) & 3],2)] ^       \
             il_tab[3][byte(bi[(n + 1) & 3],3)] ^ *(k + n)

void
lm_gen_tabs (void)
{
    u32 i, t;
    u8 p, q;

    /* log and power tables for GF(2**8) finite field with
       0x011b as modular polynomial - the simplest primitive
       root is 0x03, used here to generate the tables */

    for (i = 0, p = 1; i < 256; ++i) {
        pow_tab[i] = (u8) p;
        log_tab[p] = (u8) i;

        p ^= (p << 1) ^ (p & 0x80 ? 0x01b : 0);
    }

    log_tab[1] = 0;

    for (i = 0, p = 1; i < 10; ++i) {
        rco_tab[i] = p;

        p = (p << 1) ^ (p & 0x80 ? 0x01b : 0);
    }

    for (i = 0; i < 256; ++i) {
        p = (i ? pow_tab[255 - log_tab[i]] : 0);
        q = ((p >> 7) | (p << 1)) ^ ((p >> 6) | (p << 2));
        p ^= 0x63 ^ q ^ ((q >> 6) | (q << 2));
        sbx_tab[i] = p;
        isb_tab[p] = (u8) i;
    }

    for (i = 0; i < 256; ++i) {
        p = sbx_tab[i];

        t = p;
        fl_tab[0][i] = t;
        fl_tab[1][i] = rotl (t, 8);
        fl_tab[2][i] = rotl (t, 16);
        fl_tab[3][i] = rotl (t, 24);

        t = ((u32) ff_mult (2, p)) |
            ((u32) p << 8) |
            ((u32) p << 16) | ((u32) ff_mult (3, p) << 24);

        ft_tab[0][i] = t;
        ft_tab[1][i] = rotl (t, 8);
        ft_tab[2][i] = rotl (t, 16);
        ft_tab[3][i] = rotl (t, 24);

        p = isb_tab[i];

        t = p;
        il_tab[0][i] = t;
        il_tab[1][i] = rotl (t, 8);
        il_tab[2][i] = rotl (t, 16);
        il_tab[3][i] = rotl (t, 24);

        t = ((u32) ff_mult (14, p)) |
            ((u32) ff_mult (9, p) << 8) |
            ((u32) ff_mult (13, p) << 16) |
            ((u32) ff_mult (11, p) << 24);

        it_tab[0][i] = t;
        it_tab[1][i] = rotl (t, 8);
        it_tab[2][i] = rotl (t, 16);
        it_tab[3][i] = rotl (t, 24);
    }
}

#define star_x(x) (((x) & 0x7f7f7f7f) << 1) ^ ((((x) & 0x80808080) >> 7) * 0x1b)

#define imix_col(y,x)       \
    u   = star_x(x);        \
    v   = star_x(u);        \
    w   = star_x(v);        \
    t   = w ^ (x);          \
   (y)  = u ^ v ^ w;        \
   (y) ^= rotr(u ^ t,  8) ^ \
          rotr(v ^ t, 16) ^ \
          rotr(t,24)

/* initialise the key schedule from the user supplied key */

#define loop4(i)                                    \
{   t = rotr(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[4 * i];     E_KEY[4 * i + 4] = t;    \
    t ^= E_KEY[4 * i + 1]; E_KEY[4 * i + 5] = t;    \
    t ^= E_KEY[4 * i + 2]; E_KEY[4 * i + 6] = t;    \
    t ^= E_KEY[4 * i + 3]; E_KEY[4 * i + 7] = t;    \
}

#define loop6(i)                                    \
{   t = rotr(t,  8); t = ls_box(t) ^ rco_tab[i];    \
    t ^= E_KEY[6 * i];     E_KEY[6 * i + 6] = t;    \
    t ^= E_KEY[6 * i + 1]; E_KEY[6 * i + 7] = t;    \
    t ^= E_KEY[6 * i + 2]; E_KEY[6 * i + 8] = t;    \
    t ^= E_KEY[6 * i + 3]; E_KEY[6 * i + 9] = t;    \
    t ^= E_KEY[6 * i + 4]; E_KEY[6 * i + 10] = t;   \
    t ^= E_KEY[6 * i + 5]; E_KEY[6 * i + 11] = t;   \
}

#define loop8(i)                                    \
{   t = rotr(t,  8); ; t = ls_box(t) ^ rco_tab[i];  \
    t ^= E_KEY[8 * i];     E_KEY[8 * i + 8] = t;    \
    t ^= E_KEY[8 * i + 1]; E_KEY[8 * i + 9] = t;    \
    t ^= E_KEY[8 * i + 2]; E_KEY[8 * i + 10] = t;   \
    t ^= E_KEY[8 * i + 3]; E_KEY[8 * i + 11] = t;   \
    t  = E_KEY[8 * i + 4] ^ ls_box(t);    \
    E_KEY[8 * i + 12] = t;                \
    t ^= E_KEY[8 * i + 5]; E_KEY[8 * i + 13] = t;   \
    t ^= E_KEY[8 * i + 6]; E_KEY[8 * i + 14] = t;   \
    t ^= E_KEY[8 * i + 7]; E_KEY[8 * i + 15] = t;   \
}

int
aes_set_key(void *ctx_arg, const u8 *in_key, unsigned int key_len, u32 *flags)
{
    struct aes_ctx *ctx = ctx_arg;
    u32 i, t, u, v, w;

    if (key_len != 16 && key_len != 24 && key_len != 32) {
        printf("[%s][%d] invalid key_len\n", __FUNCTION__, __LINE__);
        //*flags |= CRYPTO_TFM_RES_BAD_KEY_LEN;
        return -EINVAL;
    }

    ctx->key_length = key_len;

    E_KEY[0] = u32_in (in_key);
    E_KEY[1] = u32_in (in_key + 4);
    E_KEY[2] = u32_in (in_key + 8);
    E_KEY[3] = u32_in (in_key + 12);

    switch (key_len) {
    case 16:
        t = E_KEY[3];
        for (i = 0; i < 10; ++i)
            loop4 (i);
        break;

    case 24:
        E_KEY[4] = u32_in (in_key + 16);
        t = E_KEY[5] = u32_in (in_key + 20);
        for (i = 0; i < 8; ++i)
            loop6 (i);
        break;

    case 32:
        E_KEY[4] = u32_in (in_key + 16);
        E_KEY[5] = u32_in (in_key + 20);
        E_KEY[6] = u32_in (in_key + 24);
        t = E_KEY[7] = u32_in (in_key + 28);
        for (i = 0; i < 7; ++i)
            loop8 (i);
        break;
    }

    D_KEY[0] = E_KEY[0];
    D_KEY[1] = E_KEY[1];
    D_KEY[2] = E_KEY[2];
    D_KEY[3] = E_KEY[3];

    for (i = 4; i < key_len + 24; ++i) {
        imix_col (D_KEY[i], E_KEY[i]);
    }

    return 0;
}

/* encrypt a block of text */

#define f_nround(bo, bi, k) \
    f_rn(bo, bi, 0, k);     \
    f_rn(bo, bi, 1, k);     \
    f_rn(bo, bi, 2, k);     \
    f_rn(bo, bi, 3, k);     \
    k += 4

#define f_lround(bo, bi, k) \
    f_rl(bo, bi, 0, k);     \
    f_rl(bo, bi, 1, k);     \
    f_rl(bo, bi, 2, k);     \
    f_rl(bo, bi, 3, k)

static void aes_encrypt(void *ctx_arg, u8 *out, const u8 *in)
{
    const struct aes_ctx *ctx = ctx_arg;
    u32 b0[4], b1[4];
    const u32 *kp = E_KEY + 4;

    b0[0] = u32_in (in) ^ E_KEY[0];
    b0[1] = u32_in (in + 4) ^ E_KEY[1];
    b0[2] = u32_in (in + 8) ^ E_KEY[2];
    b0[3] = u32_in (in + 12) ^ E_KEY[3];

    if (ctx->key_length > 24) {
        f_nround (b1, b0, kp);
        f_nround (b0, b1, kp);
    }

    if (ctx->key_length > 16) {
        f_nround (b1, b0, kp);
        f_nround (b0, b1, kp);
    }

    f_nround (b1, b0, kp);
    f_nround (b0, b1, kp);
    f_nround (b1, b0, kp);
    f_nround (b0, b1, kp);
    f_nround (b1, b0, kp);
    f_nround (b0, b1, kp);
    f_nround (b1, b0, kp);
    f_nround (b0, b1, kp);
    f_nround (b1, b0, kp);
    f_lround (b0, b1, kp);

    u32_out (out, b0[0]);
    u32_out (out + 4, b0[1]);
    u32_out (out + 8, b0[2]);
    u32_out (out + 12, b0[3]);
}

/* decrypt a block of text */

#define i_nround(bo, bi, k) \
    i_rn(bo, bi, 0, k);     \
    i_rn(bo, bi, 1, k);     \
    i_rn(bo, bi, 2, k);     \
    i_rn(bo, bi, 3, k);     \
    k -= 4

#define i_lround(bo, bi, k) \
    i_rl(bo, bi, 0, k);     \
    i_rl(bo, bi, 1, k);     \
    i_rl(bo, bi, 2, k);     \
    i_rl(bo, bi, 3, k)

static void aes_decrypt(void *ctx_arg, u8 *out, const u8 *in)
{
    const struct aes_ctx *ctx = ctx_arg;
    u32 b0[4], b1[4];
    const int key_len = ctx->key_length;
    const u32 *kp = D_KEY + key_len + 20;

    b0[0] = u32_in (in) ^ E_KEY[key_len + 24];
    b0[1] = u32_in (in + 4) ^ E_KEY[key_len + 25];
    b0[2] = u32_in (in + 8) ^ E_KEY[key_len + 26];
    b0[3] = u32_in (in + 12) ^ E_KEY[key_len + 27];

    if (key_len > 24) {
        i_nround (b1, b0, kp);
        i_nround (b0, b1, kp);
    }

    if (key_len > 16) {
        i_nround (b1, b0, kp);
        i_nround (b0, b1, kp);
    }

    i_nround (b1, b0, kp);
    i_nround (b0, b1, kp);
    i_nround (b1, b0, kp);
    i_nround (b0, b1, kp);
    i_nround (b1, b0, kp);
    i_nround (b0, b1, kp);
    i_nround (b1, b0, kp);
    i_nround (b0, b1, kp);
    i_nround (b1, b0, kp);
    i_lround (b0, b1, kp);

    u32_out (out, b0[0]);
    u32_out (out + 4, b0[1]);
    u32_out (out + 8, b0[2]);
    u32_out (out + 12, b0[3]);
}

int lm_aes_encrypt(void *ctx_arg, u8 *out, u32 ol, const u8 *in, u32 il, u32 *length)
{
    int i, rd;
    unsigned char tmp[32] = {0};
    u8 *in_p, *out_p;

    if(NULL == out || NULL == in)
    {
        printf("[%s][%d]out or in should not be NULL\n", __FUNCTION__, __LINE__);
        return -1;
    }
    rd = il/16;
    if(0 != il%16)
        rd++;
    if(ol < 16*rd)
    {
        printf("[%s][%d]invalid length\n", __FUNCTION__, __LINE__);
        return -1;
    }

    in_p = (u8 *)in;
    out_p = (u8 *)out;
    for(i=0; i<rd; i++)
    {
        memset(tmp, 0x0, sizeof(tmp));
        aes_encrypt(ctx_arg, tmp, in_p+i*16);
        memcpy(out_p+i*16, tmp, 16);
    }

    *length = 16*rd;
    return 0;
}

int lm_aes_decrypt(void *ctx_arg, u8 *out, u32 ol, const u8 *in, u32 il, u32 *length)
{
    int i, rd;
    int data_len = 0, total_len = 0;
    unsigned char tmp[32] = {0};
    u8 *in_p, *out_p;

    if(NULL == out || NULL == in)
    {
        printf("[%s][%d]out or in should not be NULL\n", __FUNCTION__, __LINE__);
        return -1;
    }

    if(il%16 != 0 || ol < (il-16))
    {
        printf("[%s][%d]invalid length\n", __FUNCTION__, __LINE__);
        return -1;
    }

    in_p = (u8 *)in;
    out_p = (u8 *)out;
    rd = il/16;
    for(i=0; i<rd; i++)
    {
        memset(tmp, 0x0, sizeof(tmp));
        aes_decrypt(ctx_arg, tmp, in_p+i*16);
        data_len = strlen(tmp);
        total_len += data_len;
        memcpy(out_p+i*16, tmp, data_len);
    }

    *length = total_len;

    return 0;
}


