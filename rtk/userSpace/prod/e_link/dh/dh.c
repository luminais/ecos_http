/*
 * dh.c: Diffie Hellman implementation.
 *
 * Code copied from openssl distribution and
 * Modified just enough so that compiles and runs standalone
 *
 * Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * $Id: dh.c,v 1.9.108.1 2010-08-13 18:00:15 Exp $
 */
/* Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com)
 * All rights reserved.
 *
 * This package is an SSL implementation written
 * by Eric Young (eay@cryptsoft.com).
 * The implementation was written so as to conform with Netscapes SSL.
 *
 * This library is free for commercial and non-commercial use as long as
 * the following conditions are aheared to.  The following conditions
 * apply to all code found in this distribution, be it the RC4, RSA,
 * lhash, DES, etc., code; not just the SSL code.  The SSL documentation
 * included with this distribution is covered by the same copyright terms
 * except that the holder is Tim Hudson (tjh@cryptsoft.com).
 *
 * Copyright remains Eric Young's, and as such any Copyright notices in
 * the code are not to be removed.
 * If this package is used in a product, Eric Young should be given attribution
 * as the author of the parts of the library used.
 * This can be in the form of a textual message at program startup or
 * in documentation (online or textual) provided with the package.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *    "This product includes cryptographic software written by
 *     Eric Young (eay@cryptsoft.com)"
 *    The word 'cryptographic' can be left out if the rouines from the library
 *    being used are not cryptographic related :-).
 * 4. If you include any Windows specific code (or a derivative thereof) from
 *    the apps directory (application code) you must include an acknowledgement:
 *    "This product includes software written by Tim Hudson (tjh@cryptsoft.com)"
 *
 * THIS SOFTWARE IS PROVIDED BY ERIC YOUNG ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * The licence and distribution terms for any publically available version or
 * derivative of this code cannot be changed.  i.e. this code cannot simply be
 * copied and put under another distribution licence
 * [including the GNU Public Licence.]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typedefs.h"
#include "dh.h"

static int dh_bn_mod_exp(const DH *dh, BIGNUM *r,
                         const BIGNUM *a, const BIGNUM *p,
                         const BIGNUM *m, BN_CTX *ctx,
                         BN_MONT_CTX *m_ctx);

DH *
DH_new(void)
{
	DH *ret;

	ret = (DH *)OPENSSL_malloc(sizeof(DH));
	if (ret == NULL) {
		return (NULL);
	}

	ret->p = NULL;
	ret->g = NULL;
	ret->length = 0;
	ret->pub_key = NULL;
	ret->priv_key = NULL;
	ret->q = NULL;
	ret->j = NULL;
	ret->seed = NULL;
	ret->seedlen = 0;
	ret->counter = NULL;
	ret->method_mont_p = NULL;
	ret->flags = DH_FLAG_CACHE_MONT_P;
	return (ret);
}

void
DH_free(DH *r)
{
	if (r == NULL) return;
	if (r->p != NULL) BN_clear_free(r->p);
	if (r->g != NULL) BN_clear_free(r->g);
	if (r->q != NULL) BN_clear_free(r->q);
	if (r->j != NULL) BN_clear_free(r->j);
	if (r->seed) OPENSSL_free(r->seed);
	if (r->counter != NULL) BN_clear_free(r->counter);
	if (r->pub_key != NULL) BN_clear_free(r->pub_key);
	if (r->priv_key != NULL) BN_clear_free(r->priv_key);
	if (r->method_mont_p)
		BN_MONT_CTX_free((BN_MONT_CTX *)r->method_mont_p);
	OPENSSL_free(r);
}

int
DH_generate_key(unsigned char *pubbuf, DH *dh)
{
	int ret = 0;
	unsigned l;
	BN_CTX *ctx = NULL;
	BN_MONT_CTX *mont;
	BIGNUM *pub_key = NULL, *priv_key = NULL;


	if (dh->pub_key != NULL) {
		if (pubbuf)
			return BN_bn2bin(dh->pub_key, pubbuf);
		else
			return BN_num_bytes(dh->pub_key);
	}
	/* first time in here */
	priv_key = BN_new();
	if (priv_key == NULL) goto err;

	pub_key = BN_new();
	if (pub_key == NULL) goto err;

	ctx = BN_CTX_new();
	if (ctx == NULL) goto err;

	if (dh->flags & DH_FLAG_CACHE_MONT_P) {
		if ((dh->method_mont_p = BN_MONT_CTX_new()) != NULL)
			if (!BN_MONT_CTX_set((BN_MONT_CTX *)dh->method_mont_p,
			                     dh->p, ctx))
				goto err;
	}

	mont = (BN_MONT_CTX *)dh->method_mont_p;

	l = dh->length ? dh->length : BN_num_bits(dh->p)-1; /* secret exponent length */
	if (!BN_rand(priv_key, l, 0, 0))
		goto err;
	if (!dh_bn_mod_exp(dh, pub_key, dh->g, priv_key, dh->p, ctx, mont))
		goto err;

	dh->pub_key = pub_key;
	dh->priv_key = priv_key;
	if (pubbuf)
		ret = BN_bn2bin(pub_key, pubbuf);
	else
		ret = BN_num_bytes(dh->pub_key);
err:
	if ((pub_key != NULL) && (dh->pub_key == NULL))
		BN_free(pub_key);
	if ((priv_key != NULL) && (dh->priv_key == NULL))
		BN_free(priv_key);
	if (ctx)
		BN_CTX_free(ctx);
	return (ret);
}


int
DH_compute_key_bn(unsigned char *key, BIGNUM *peer_key, DH *dh)
{
	BN_CTX *ctx;
	BN_MONT_CTX *mont;
	BIGNUM *tmp;
	int ret = -1;

	ctx = BN_CTX_new();
	if (ctx == NULL) goto err;
	BN_CTX_start(ctx);
	tmp = BN_CTX_get(ctx);

	if (dh->priv_key == NULL) {
		goto err;
	}
	if ((dh->method_mont_p == NULL) && (dh->flags & DH_FLAG_CACHE_MONT_P)) {
		if ((dh->method_mont_p = BN_MONT_CTX_new()) != NULL)
			if (!BN_MONT_CTX_set((BN_MONT_CTX *)dh->method_mont_p,
			                     dh->p, ctx))
				goto err;
	}

	mont = (BN_MONT_CTX *)dh->method_mont_p;
	if (!dh_bn_mod_exp(dh, tmp, peer_key, dh->priv_key, dh->p, ctx, mont)) {
		goto err;
	}

	ret = BN_bn2bin(tmp, key);
err:
	if (peer_key)
		BN_clear_free(peer_key);
	BN_CTX_end(ctx);
	BN_CTX_free(ctx);
	return (ret);
}


int
DH_compute_key(unsigned char *key, unsigned char *pubbuf, int buflen, DH *dh)
{
	BIGNUM *peer_key = NULL;

	peer_key = BN_bin2bn(pubbuf, buflen, NULL);
	return DH_compute_key_bn(key, peer_key, dh);
}

DH *
DH_init(unsigned char *pbuf, int plen, int g)
{
	DH *dh;

	dh = DH_new();
	dh->p = BN_bin2bn(pbuf, plen, NULL);
	dh->g = BN_new();
	BN_set_word(dh->g, g);
	return dh;
}

static int
dh_bn_mod_exp(const DH *dh, BIGNUM *r,
              const BIGNUM *a, const BIGNUM *p,
              const BIGNUM *m, BN_CTX *ctx,
              BN_MONT_CTX *m_ctx)
{
	if (a->top == 1) {
		BN_ULONG A = a->d[0];
		return BN_mod_exp_mont_word(r, A, p, m, ctx, m_ctx);
	} else
		return BN_mod_exp_mont(r, a, p, m, ctx, m_ctx);
}

int DH_size(const DH *dh)
{
	return (BN_num_bytes(dh->p));
}

/* We generate DH parameters as follows
 * find a prime q which is prime_len/2 bits long.
 * p=(2*q)+1 or (p-1)/2 = q
 * For this case, g is a generator if
 * g^((p-1)/q) mod p != 1 for values of q which are the factors of p-1.
 * Since the factors of p-1 are q and 2, we just need to check
 * g^2 mod p != 1 and g^q mod p != 1.
 *
 * Having said all that,
 * there is another special case method for the generators 2, 3 and 5.
 * for 2, p mod 24 == 11
 * for 3, p mod 12 == 5  <<<<< does not work for safe primes.
 * for 5, p mod 10 == 3 or 7
 *
 * Thanks to Phil Karn <karn@qualcomm.com> for the pointers about the
 * special generators and for answering some of my questions.
 *
 * I've implemented the second simple method :-).
 * Since DH should be using a safe prime (both p and q are prime),
 * this generator function can take a very very long time to run.
 */
/* Actually there is no reason to insist that 'generator' be a generator.
 * It's just as OK (and in some sense better) to use a generator of the
 * order-q subgroup.
 */
DH *
DH_generate_parameters(DH* dh, int prime_len, int generator,
                       void (*callback)(int, int, void *), void *cb_arg)
{
	BIGNUM *p = NULL, *t1, *t2;
	DH *ret = NULL;
	int g, ok = -1;
	BN_CTX *ctx = NULL;

	ret = dh ? dh : DH_new();
	if (ret == NULL) goto err;
	ctx = BN_CTX_new();
	if (ctx == NULL) goto err;
	BN_CTX_start(ctx);
	t1 = BN_CTX_get(ctx);
	t2 = BN_CTX_get(ctx);
	if (t1 == NULL || t2 == NULL) goto err;

	if (generator <= 1) {
		goto err;
	}
	if (generator == DH_GENERATOR_2) {
		if (!BN_set_word(t1, 24)) goto err;
		if (!BN_set_word(t2, 11)) goto err;
		g = 2;
	}

	else if (generator == DH_GENERATOR_5) {
		if (!BN_set_word(t1, 10)) goto err;
		if (!BN_set_word(t2, 3)) goto err;
		/* BN_set_word(t3, 7); just have to miss
		 * out on these ones :-(
		 */
		g = 5;
	} else {
		/* in the general case, don't worry if 'generator' is a
		 * generator or not: since we are using safe primes,
		 * it will generate either an order-q or an order-2q group,
		 * which both is OK
		 */
		if (!BN_set_word(t1, 2)) goto err;
		if (!BN_set_word(t2, 1)) goto err;
		g = generator;
	}

	p = BN_generate_prime(NULL, prime_len, 1, t1, t2, callback, cb_arg);
	if (p == NULL) goto err;
	if (callback != NULL) callback(3, 0, cb_arg);
	ret->p = p;
	ret->g = BN_new();
	if (!BN_set_word(ret->g, g)) goto err;
	ok = 1;
err:
	if (ok == -1) {
		ok = 0;
	}

	if (ctx != NULL) {
		BN_CTX_end(ctx);
		BN_CTX_free(ctx);
	}
	if (!ok && (ret != NULL)) {
		DH_free(ret);
		ret = NULL;
	}
	return (ret);
}

/* Check that p is a safe prime and
 * if g is 2, 3 or 5, check that is is a suitable generator
 * where
 * for 2, p mod 24 == 11
 * for 3, p mod 12 == 5
 * for 5, p mod 10 == 3 or 7
 * should hold.
 */

int
DH_check(const DH *dh, int *ret)
{
	int ok = 0;
	BN_CTX *ctx = NULL;
	BN_ULONG l;
	BIGNUM *q = NULL;

	*ret = 0;
	ctx = BN_CTX_new();
	if (ctx == NULL) goto err;
	q = BN_new();
	if (q == NULL) goto err;

	if (BN_is_word(dh->g, DH_GENERATOR_2)) {
		l = BN_mod_word(dh->p, 24);
		if (l != 11) *ret |= DH_NOT_SUITABLE_GENERATOR;
	}

	else if (BN_is_word(dh->g, DH_GENERATOR_5)) {
		l = BN_mod_word(dh->p, 10);
		if ((l != 3) && (l != 7))
			*ret |= DH_NOT_SUITABLE_GENERATOR;
	} else
		*ret |= DH_UNABLE_TO_CHECK_GENERATOR;

	if (!BN_is_prime(dh->p, BN_prime_checks, NULL, ctx, NULL))
		*ret |= DH_CHECK_P_NOT_PRIME;
	else {
		if (!BN_rshift1(q, dh->p)) goto err;
		if (!BN_is_prime(q, BN_prime_checks, NULL, ctx, NULL))
			*ret |= DH_CHECK_P_NOT_SAFE_PRIME;
	}
	ok = 1;
err:
	if (ctx != NULL) BN_CTX_free(ctx);
	if (q != NULL) BN_free(q);
	return (ok);
}

