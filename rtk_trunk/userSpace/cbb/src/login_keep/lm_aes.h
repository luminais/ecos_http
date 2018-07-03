#ifndef _LM_AES_H_
#define _LM_AES_H_

typedef unsigned char u8;
typedef unsigned int u32;

#define AES_MIN_KEY_SIZE    16
#define AES_MAX_KEY_SIZE    32

#define AES_BLOCK_SIZE      16

struct aes_ctx {
    int key_length;
    u32 E[60];
    u32 D[60];
};

extern int aes_set_key(void *ctx_arg, const u8 *in_key, unsigned int key_len, u32 *flags);
extern int lm_aes_encrypt(void *ctx_arg, u8 *out, u32 ol, const u8 *in, u32 il, u32 *length);
extern int lm_aes_decrypt(void *ctx_arg, u8 *out, u32 ol, const u8 *in, u32 il, u32 *length);

#endif /* _LM_AES_H_ */

