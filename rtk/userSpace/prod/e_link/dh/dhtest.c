
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <getopt.h>
#include <assert.h>
#include "dh.h"
#include "bn.h"
#include "dh_wrap.h"

unsigned char data192[] = {
	0xD4, 0xA0, 0xBA, 0x02, 0x50, 0xB6, 0xFD, 0x2E,
	0xC6, 0x26, 0xE7, 0xEF, 0xD6, 0x37, 0xDF, 0x76,
	0xC7, 0x16, 0xE2, 0x2D, 0x09, 0x44, 0xB8, 0x8B
};

unsigned char data512[] = {
	0xDA, 0x58, 0x3C, 0x16, 0xD9, 0x85, 0x22, 0x89,
	0xD0, 0xE4, 0xAF, 0x75, 0x6F, 0x4C, 0xCA, 0x92,
	0xDD, 0x4B, 0xE5, 0x33, 0xB8, 0x04, 0xFB, 0x0F,
	0xED, 0x94, 0xEF, 0x9C, 0x8A, 0x44, 0x03, 0xED,
	0x57, 0x46, 0x50, 0xD3, 0x69, 0x99, 0xDB, 0x29,
	0xD7, 0x76, 0x27, 0x6B, 0xA2, 0xD3, 0xD4, 0x12,
	0xE2, 0x18, 0xF4, 0xDD, 0x1E, 0x08, 0x4C, 0xF6,
	0xD8, 0x00, 0x3E, 0x7C, 0x47, 0x74, 0xE8, 0x33
};

unsigned char data1024[] = {
	0x97, 0xF6, 0x42, 0x61, 0xCA, 0xB5, 0x05, 0xDD,
	0x28, 0x28, 0xE1, 0x3F, 0x1D, 0x68, 0xB6, 0xD3,
	0xDB, 0xD0, 0xF3, 0x13, 0x04, 0x7F, 0x40, 0xE8,
	0x56, 0xDA, 0x58, 0xCB, 0x13, 0xB8, 0xA1, 0xBF,
	0x2B, 0x78, 0x3A, 0x4C, 0x6D, 0x59, 0xD5, 0xF9,
	0x2A, 0xFC, 0x6C, 0xFF, 0x3D, 0x69, 0x3F, 0x78,
	0xB2, 0x3D, 0x4F, 0x31, 0x60, 0xA9, 0x50, 0x2E,
	0x3E, 0xFA, 0xF7, 0xAB, 0x5E, 0x1A, 0xD5, 0xA6,
	0x5E, 0x55, 0x43, 0x13, 0x82, 0x8D, 0xA8, 0x3B,
	0x9F, 0xF2, 0xD9, 0x41, 0xDE, 0xE9, 0x56, 0x89,
	0xFA, 0xDA, 0xEA, 0x09, 0x36, 0xAD, 0xDF, 0x19,
	0x71, 0xFE, 0x63, 0x5B, 0x20, 0xAF, 0x47, 0x03,
	0x64, 0x60, 0x3C, 0x2D, 0xE0, 0x59, 0xF5, 0x4B,
	0x65, 0x0A, 0xD8, 0xFA, 0x0C, 0xF7, 0x01, 0x21,
	0xC7, 0x47, 0x99, 0xD7, 0x58, 0x71, 0x32, 0xBE,
	0x9B, 0x99, 0x9B, 0xB9, 0xB7, 0x87, 0xE8, 0xAB
};


int wxy_test1()
{
	char pubkey[33] = {0};
    char prime[33] = {0};
	int ret = 0;
    
	ret = DH_init_wrap(pubkey, sizeof(pubkey), prime, sizeof(prime));	
	if(ret < 0)
    {
		printf("DH_init_wrap error. \n");
        return -1;
    }
    printf("pubkey=%s\n", pubkey);
    printf("prime=%s\n", prime);

    char realKey[16] = {0};
    char peerKey[33] = "25FF77C86AFB98DB212B774C6C197816";
    
	ret = DH_calculate_wrap(realKey, sizeof(realKey), peerKey, strlen(peerKey));
	if(ret < 0)
    {
		printf("DH_calculate_wrap error. \n");
        return -1;
    }
    
    return 0;
}

int
main(int argc, char *argv[])
{
	DH *a;
	DH *b = NULL;
	unsigned char *abuf = NULL, *bbuf = NULL;
	unsigned char *apubbuf = NULL, *bpubbuf = NULL;
	int i, alen, blen, apublen, bpublen, aout, bout, ret = 1, size = 0;
	char opt;

	while ((opt = getopt(argc, argv, "s:")) != EOF) {
		switch (opt) {
		case 's':
			size = (int)strtoul(optarg, NULL, 0);
			break;
		default:
			dbg("invalid option");
			return ret;
		}
	}

	if (size == 192) {
		a = DH_init(data192, sizeof(data192), 3);
	} else if (size == 512) {
		a = DH_init(data512, sizeof(data512), 2);
	} else if (size == 1024) {
		a = DH_init(data1024, sizeof(data1024), 2);
	} else {
		a = DH_generate_parameters(NULL, 128, DH_GENERATOR_5, cb, NULL);
		if (a == NULL) goto err;
	}

	if (!DH_check(a, &i)) goto err;
	if (i & DH_CHECK_P_NOT_PRIME)
		dbg("p value is not prime\n");
	if (i & DH_CHECK_P_NOT_SAFE_PRIME)
		dbg("p value is not a safe prime\n");
	if (i & DH_UNABLE_TO_CHECK_GENERATOR)
		dbg("unable to check the generator value\n");
	if (i & DH_NOT_SUITABLE_GENERATOR)
		dbg("the g value is not a generator\n");

	dbg("\np    =");
	bn_print(a->p);
	dbg("\ng    =");
	bn_print(a->g);
	dbg("\n");

	b = DH_new();
	if (b == NULL) goto err;

	b->p = BN_dup(a->p);
	b->g = BN_dup(a->g);
	if ((b->p == NULL) || (b->g == NULL)) goto err;

	apubbuf = (unsigned char *)OPENSSL_malloc(DH_size(a));
	/* dbg("%s\n", file2str("/proc/uptime")); */
	if (!(apublen = DH_generate_key(apubbuf, a))) goto err;
	/* dbg("%s\n", file2str("/proc/uptime")); */
	dbg("pri 1=");
	bn_print(a->priv_key);
	dbg("\npub 1=");
	for (i = 0; i < apublen; i++) dbg("%02X", apubbuf[i]);
	dbg("\n");

	bpubbuf = (unsigned char *)OPENSSL_malloc(DH_size(b));
	if (!(bpublen = DH_generate_key(bpubbuf, b))) goto err;
	dbg("pri 2=");
	bn_print(b->priv_key);
	dbg("\npub 2=");
	for (i = 0; i < bpublen; i++) dbg("%02X", bpubbuf[i]);
	dbg("\n");

	alen = DH_size(a);
	abuf = (unsigned char *)OPENSSL_malloc(alen);
	/* dbg("%s\n", file2str("/proc/uptime")); */
	aout = DH_compute_key(abuf, bpubbuf, bpublen, a);
	/* dbg("%s\n", file2str("/proc/uptime")); */

	dbg("key1 =");
	for (i = 0; i < aout; i++) dbg("%02X", abuf[i]);
	dbg("\n");

	blen = DH_size(b);
	bbuf = (unsigned char *)OPENSSL_malloc(blen);
	bout = DH_compute_key(bbuf, apubbuf, apublen, b);

	dbg("key2 =");
	for (i = 0; i < bout; i++) dbg("%02X", abuf[i]);
	dbg("\n");

	if ((aout < 4) || (bout != aout) || (memcmp(abuf, bbuf, aout) != 0))
		ret = 1;
	else
		ret = 0;
err:
	if (abuf != NULL) OPENSSL_free(abuf);
	if (bbuf != NULL) OPENSSL_free(bbuf);
	if (b != NULL) DH_free(b);
	if (a != NULL) DH_free(a);
	fprintf(stderr, "%s: %s\n", *argv, ret?"FAILED":"PASSED");

    wxy_test1();
    
	return (ret);
}
