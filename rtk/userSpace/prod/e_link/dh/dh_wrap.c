/*
	Add By wangxinyu, For dh wrapper easy to use.
	2016.8.13
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bn.h>
#include <openssl/dh.h>

//#include "dh.h"
//#include "bn.h"
#include "dh_wrap.h"


/*
    Generate 128 bits Publick key, Use generator 5.
*/
static DH *gDeviceDHKey = NULL; 
#define DH_GENERATOR DH_GENERATOR_5
#define DH_128_KEY_SIZE_BYTE (16) 



unsigned char turn_char_to_num(char ch1)
{
	unsigned char num = 0;

	switch(ch1)
	{
		case 'a'...'z' :
			num = ch1-'a' +10;
			break;
		case 'A'...'Z':
			num = ch1-'A' +10;
			break;
		case '0'...'9':
			num = ch1-'0';
			break;
		default:
            ;
			//printf("%s:error input\n", __FUNCTION__);
	}
	
	return num;
}

/*
	Input :
		pubkey: pubkey Buffer, 
		publen: at lease 16 bytes.(>= 16)
		prime : prime Buffer,
		plen  : at lease 16 bytes.(>= 16)
	Out:
		pubkey : A publick key send to peer. (Hex array)
		prime  : A Big prime Number sent to peer.  (Hex array)

	Attention: Defaut generator, we Use 5.
	wangxinyu.yy@gmail.com  2016.8.13
*/
int DH_init_wrap(unsigned char *pubkey, int publen, 
									unsigned char *prime, int plen)
{
	DH *a;
    int apublen;
    int i = 0;
    unsigned char *p;
    unsigned char tmpbuf[DH_128_KEY_SIZE_BYTE] = {0};
    unsigned char apubbuf[DH_128_KEY_SIZE_BYTE] = {0};

	if(publen < 16 || plen < 16)
    {
    	printf("Input buffer is too small.\n");
		return -1;
    }


	if(((a = DH_new()) == NULL) || !DH_generate_parameters_ex(a, 128,
				DH_GENERATOR, NULL))
	{
    	printf("Error DH_init_wrap DH_new.\n");
		return -1;
	}
	gDeviceDHKey = a;
    
    //a = gDeviceDHKey = DH_generate_parameters(NULL, 128, DH_GENERATOR, NULL, NULL);
	//a = gDeviceDHKey = DH_generate_parameters(128, DH_GENERATOR,NULL, NULL);
	     
	
    if (a == NULL) 
	{
		printf("wxy_test1 error1.\n");
        return -1;
    }
#if 0
    if (!(apublen = DH_generate_key(apubbuf, a)))
	{
		printf("wxy_test1 error2.\n");
        return -1;
    }
#endif
	if (!(apublen = DH_generate_key(a)))
	{
		printf("wxy_test1 error2.\n");
        return -1;
    }
	
    dbg("\np    =");
    bn_print(a->p);
    dbg("\ng    =");
    bn_print(a->g);
    dbg("\n");
    dbg("pri a=");
	bn_print(a->priv_key);
    dbg("\npub a=");
	bn_print(a->pub_key);
    dbg("\npru a=");
	
	//for (i = 0; i < apublen; i++) dbg("%02X", apubbuf[i]);
	//dbg("\n");

	BN_bn2bin(a->pub_key, pubkey);
	//p =  pubkey;
	//for (i = 0; i < DH_128_KEY_SIZE_BYTE; i++) 
     //   p += sprintf(p, "%02X", tmpbuf[i]);

	BN_bn2bin(a->p, prime);
	//p =  prime;
//	for (i = 0; i < DH_128_KEY_SIZE_BYTE; i++) 
   //     p += sprintf(p, "%02X", tmpbuf[i]);

    return 0;
}

/*
	Input:
		realKey : key buffer.
		keyLen : at least 16 . (>= 16)
		peerPubKey:    A hex array.
		peerPubKeyLen: Must be 16 
	Output:
		realKey : 16 Bytes key.

	Attention: 
		This out key will be used by AES directly. (Not A hex string, hex array)
*/
int DH_calculate_wrap(unsigned char *realKey, int keyLen,
    							unsigned char *peerPubKey, int peerPubKeyLen)
 {
 	DH *a;
	int alen;
    unsigned char *abuf;
    unsigned char *p;
    int i = 0;
    unsigned char bpubkey[DH_128_KEY_SIZE_BYTE] = {0};
	int bpublen;

    if(keyLen < 16 || peerPubKeyLen != 16)
    {
    	printf("Input buffer length error.\n");
		return -1;
    }
    
	a = gDeviceDHKey;
	if(!a)
		return -1;

	/* Change peer key from string to array.  
	p = peerPubKey;
    for(i = 0; i < peerPubKeyLen/2; i ++)
	{
		bpubkey[i] = ((turn_char_to_num(*p) ) << 4)| turn_char_to_num(*(p+1)) ;
		p += 2;
	}
	bpublen = sizeof(bpubkey);*/

	const BIGNUM *bn;
	bn = BN_bin2bn(peerPubKey, peerPubKeyLen, NULL);
	
	//alen = DH_compute_key(realKey, peerPubKey, peerPubKeyLen, a);
	alen = DH_compute_key(realKey, bn,a);
	printf("alen=%d.\n", alen);

	dbg("RealKey =");
	for (i = 0; i < alen; i++) 
        dbg("%02X", realKey[i]);
	dbg("\n");
	BN_free(bn);
    DH_free(gDeviceDHKey);
    gDeviceDHKey = NULL;
    return 0;
}

void cb(int p, int n, void *arg)
{
	if (p == 0) dbg(".");
	if (p == 1) dbg("+");
	if (p == 2) dbg("*");
	if (p == 3) dbg("\n");
}

static const char *Hex = "0123456789ABCDEF";

int bn_print(const BIGNUM *a)
{
	
	int i, j, v, z = 0;
	int ret = 0;

	if ((a->neg) && (dbg("-") != 1)) goto end;
	if ((a->top == 0) && (dbg("0") != 1)) goto end;
	for (i = a->top - 1; i >= 0; i--) {
		for (j = BN_BITS2 - 4; j >= 0; j -= 4) {
			/* strip leading zeros */
			v = ((int)(a->d[i] >> (long)j)) & 0x0f;
			if (z || (v != 0)) {
				dbg("%c", Hex[v]);
				z = 1;
			}
		}
	}
	ret = 1;
end:
	return (ret);

}


