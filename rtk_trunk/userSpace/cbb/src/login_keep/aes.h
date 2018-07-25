#ifndef _AES_H_
#define _AES_H_

#include <stdio.h>
#include <string.h>

/********************************** 常量和宏 **********************************/
#define AES_ENCRYPT     1
#define AES_DECRYPT     0
#define AES_BLOCK_SIZE 16
/********************************** 数据类型 **********************************/
typedef int           sint32;
typedef unsigned int  uint32;
typedef unsigned char byte;

typedef struct _aes_context_
{
	sint32 nr;           /*!<  number of rounds  */
	uint32 *rk;          /*!<  AES round keys    */
	uint32 buf[68];      /*!<  unaligned data    */
}aes_context;

#ifdef __cplusplus
extern "C" {
#endif

/********************************** 函数声明 **********************************/
sint32  aes_setkey_enc(aes_context *ctx, const byte* key, sint32 key_bitsize);
sint32  aes_setkey_dec(aes_context *ctx, const byte* key, sint32 key_bitsize);
void    aes_crypt_ecb(aes_context *ctx, sint32 mode, const byte input[16], byte output[16]);
sint32  aes_encrypt(aes_context *ctx, const byte* input, uint32 inlen, uint32 from, byte* output, uint32 *outlen);
sint32  aes_decrypt(aes_context *ctx, const byte* input, uint32 inlen, uint32 from, byte* output, uint32 *outlen);

/*********************************** 类定义 ***********************************/
#ifdef __cplusplus
}
#endif

#endif
