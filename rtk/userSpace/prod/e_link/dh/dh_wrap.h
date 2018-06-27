#ifndef DH_WRAP_HEADER__
#define DH_WRAP_HEADER__

#define dbg(fmt, arg...) printf(fmt, ##arg)

extern int bn_print(const BIGNUM *a);
extern void cb(int p, int n, void *arg);
extern unsigned char turn_char_to_num(char ch1);
extern int bn_print(const BIGNUM *a);

extern int DH_generate_parameters_ex(DH *ret, int prime_len, int generator, BN_GENCB *cb);
extern int DH_compute_key(unsigned char *key, const BIGNUM *pub_key, DH *dh);
extern DH *DH_generate_parameters(int prime_len, int generator,void (*callback)(int,int,void *), void *cb_arg);
	     



extern int DH_init_wrap(unsigned char *pubkey, int publen, 
									unsigned char *prime, int plen);
extern int DH_calculate_wrap(unsigned char *realKey, int keyLen,
    							unsigned char *peerPubKey, int peerPubKeyLen);


#endif /* DH_WRAP_HEADER__ */

