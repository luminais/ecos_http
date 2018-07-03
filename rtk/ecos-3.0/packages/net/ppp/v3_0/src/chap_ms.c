//==========================================================================
//
//      src/chap_ms.c
//
//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2003 Free Software Foundation, Inc.                        
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
// ####BSDALTCOPYRIGHTBEGIN####                                             
// -------------------------------------------                              
// Portions of this software may have been derived from FreeBSD, OpenBSD,   
// or other sources, and if so are covered by the appropriate copyright     
// and license included herein.                                             
// -------------------------------------------                              
// ####BSDALTCOPYRIGHTEND####                                               
//==========================================================================

/*
 * chap_ms.c - Microsoft MS-CHAP compatible implementation.
 *
 * Copyright (c) 1995 Eric Rosenquist, Strata Software Limited.
 * http://www.strataware.com/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Eric Rosenquist.  The name of the author may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Modifications by Lauri Pesonen / lpesonen@clinet.fi, april 1997
 *
 *   Implemented LANManager type password response to MS-CHAP challenges.
 *   Now pppd provides both NT style and LANMan style blocks, and the
 *   prefered is set by option "ms-lanman". Default is to use NT.
 *   The hash text (StdText) was taken from Win95 RASAPI32.DLL.
 *
 *   You should also use DOMAIN\\USERNAME as described in README.MSCHAP80
 */

#ifndef lint
//static char rcsid[] = "$FreeBSD: src/usr.sbin/pppd/chap_ms.c,v 1.8 2000/02/24 21:10:28 markm Exp $";
#endif

#ifdef CHAPMS

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
//#include <syslog.h>
#include <unistd.h>
#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#if 0
#include "pppd.h"
#include "chap.h"
#include "chap_ms.h"
#include "md4.h"
#else
#include <cyg/ppp/pppd.h>
#include <cyg/ppp/chap.h>
#include <cyg/ppp/chap_ms.h>
#include <cyg/ppp/sha1.h>
#include <cyg/ppp/syslog.h>
static void	ascii2unicode __P((char[], int, u_char[]));
static void	NTPasswordHash __P((u_char *, int, u_char[MD4_SIGNATURE_SIZE]));
#ifndef USE_CRYPT
#include <openssl/md4.h>
#endif
#endif
#ifndef USE_CRYPT
#include <openssl/des.h>
#endif

typedef struct {
    u_char LANManResp[24];
    u_char NTResp[24];
    u_char UseNT;		/* If 1, ignore the LANMan response field */
} MS_ChapResponse;
/* We use MS_CHAP_RESPONSE_LEN, rather than sizeof(MS_ChapResponse),
   in case this struct gets padded. */


static void	ChallengeResponse __P((u_char *, u_char *, u_char *));
static void	DesEncrypt __P((u_char *, u_char *, u_char *));
static void	MakeKey __P((u_char *, u_char *));
static u_char	Get7Bits __P((u_char *, int));
static void	ChapMS_NT __P((char *, int, char *, int, MS_ChapResponse *));
#ifdef MSLANMAN
static void	ChapMS_LANMan __P((char *, int, char *, int, MS_ChapResponse *));
#endif

#ifdef USE_CRYPT
static void	Expand __P((u_char *, u_char *));
static void	Collapse __P((u_char *, u_char *));
#endif

//#if 1 //def MPPE
#if 1//def HAVE_PPTP
#define MPPE 1
static void	Set_Start_Key __P((u_char *, char *, int));
static void	SetMasterKeys __P((char *, int, u_char[24], int));
#define MPPE_MAX_KEY_LEN	16
u_char mppe_send_key[MPPE_MAX_KEY_LEN];
u_char mppe_recv_key[MPPE_MAX_KEY_LEN];
int mppe_keys_set = 0;		/* Have the MPPE keys been set? */

#ifdef DEBUGMPPEKEY
/* For MPPE debug */
/* Use "[]|}{?/><,`!2&&(" (sans quotes) for RFC 3079 MS-CHAPv2 test value */
static char *mschap_challenge = NULL;
/* Use "!@\#$%^&*()_+:3|~" (sans quotes, backslash is to escape #) for ... */
static char *mschap2_peer_challenge = NULL;
#endif

#include <cyg/ppp/fsm.h>		/* Need to poke MPPE options */
#include <cyg/ppp/ccp.h>
#include <cyg/ppp/net/ppp_comp.h>
#endif

static void
ChallengeResponse(challenge, pwHash, response)
    u_char *challenge;	/* IN   8 octets */
    u_char *pwHash;	/* IN  16 octets */
    u_char *response;	/* OUT 24 octets */
{
    char    ZPasswordHash[21];

    BZERO(ZPasswordHash, sizeof(ZPasswordHash));
    BCOPY(pwHash, ZPasswordHash, MD4_SIGNATURE_SIZE);

#if 0
    log_packet(ZPasswordHash, sizeof(ZPasswordHash), "ChallengeResponse - ZPasswordHash", LOG_DEBUG);
#endif

    DesEncrypt(challenge, ZPasswordHash +  0, response + 0);
    DesEncrypt(challenge, ZPasswordHash +  7, response + 8);
    DesEncrypt(challenge, ZPasswordHash + 14, response + 16);

#if 0
    log_packet(response, 24, "ChallengeResponse - response", LOG_DEBUG);
#endif
}


#ifdef USE_CRYPT
static void
DesEncrypt(clear, key, cipher)
    u_char *clear;	/* IN  8 octets */
    u_char *key;	/* IN  7 octets */
    u_char *cipher;	/* OUT 8 octets */
{
    u_char des_key[8];
    u_char crypt_key[66];
    u_char des_input[66];

    MakeKey(key, des_key);

    Expand(des_key, crypt_key);
    setkey(crypt_key);

#if 0
    CHAPDEBUG((LOG_INFO, "DesEncrypt: 8 octet input : %02X%02X%02X%02X%02X%02X%02X%02X",
	       clear[0], clear[1], clear[2], clear[3], clear[4], clear[5], clear[6], clear[7]));
#endif

    Expand(clear, des_input);
    encrypt(des_input, 0);
    Collapse(des_input, cipher);

#if 0
    CHAPDEBUG((LOG_INFO, "DesEncrypt: 8 octet output: %02X%02X%02X%02X%02X%02X%02X%02X",
	       cipher[0], cipher[1], cipher[2], cipher[3], cipher[4], cipher[5], cipher[6], cipher[7]));
#endif
}

#else /* USE_CRYPT */

static void
DesEncrypt(clear, key, cipher)
    u_char *clear;	/* IN  8 octets */
    u_char *key;	/* IN  7 octets */
    u_char *cipher;	/* OUT 8 octets */
{
    des_cblock		des_key;
    des_key_schedule	key_schedule;

    MakeKey(key, des_key);

    des_set_key(&des_key, key_schedule);

#if 0
    CHAPDEBUG((LOG_INFO, "DesEncrypt: 8 octet input : %02X%02X%02X%02X%02X%02X%02X%02X",
	       clear[0], clear[1], clear[2], clear[3], clear[4], clear[5], clear[6], clear[7]));
#endif

    des_ecb_encrypt((des_cblock *)clear, (des_cblock *)cipher, key_schedule, 1);

#if 0
    CHAPDEBUG((LOG_INFO, "DesEncrypt: 8 octet output: %02X%02X%02X%02X%02X%02X%02X%02X",
	       cipher[0], cipher[1], cipher[2], cipher[3], cipher[4], cipher[5], cipher[6], cipher[7]));
#endif
}

#endif /* USE_CRYPT */


static u_char Get7Bits(input, startBit)
    u_char *input;
    int startBit;
{
    register unsigned int	word;

    word  = (unsigned)input[startBit / 8] << 8;
    word |= (unsigned)input[startBit / 8 + 1];

    word >>= 15 - (startBit % 8 + 7);

    return word & 0xFE;
}

#ifdef USE_CRYPT

/* in == 8-byte string (expanded version of the 56-bit key)
 * out == 64-byte string where each byte is either 1 or 0
 * Note that the low-order "bit" is always ignored by by setkey()
 */
static void Expand(in, out)
    u_char *in;
    u_char *out;
{
        int j, c;
        int i;

        for(i = 0; i < 64; in++){
		c = *in;
                for(j = 7; j >= 0; j--)
                        *out++ = (c >> j) & 01;
                i += 8;
        }
}

/* The inverse of Expand
 */
static void Collapse(in, out)
    u_char *in;
    u_char *out;
{
        int j;
        int i;
	unsigned int c;

	for (i = 0; i < 64; i += 8, out++) {
	    c = 0;
	    for (j = 7; j >= 0; j--, in++)
		c |= *in << j;
	    *out = c & 0xff;
	}
}
#endif

static void MakeKey(key, des_key)
    u_char *key;	/* IN  56 bit DES key missing parity bits */
    u_char *des_key;	/* OUT 64 bit DES key with parity bits added */
{
    des_key[0] = Get7Bits(key,  0);
    des_key[1] = Get7Bits(key,  7);
    des_key[2] = Get7Bits(key, 14);
    des_key[3] = Get7Bits(key, 21);
    des_key[4] = Get7Bits(key, 28);
    des_key[5] = Get7Bits(key, 35);
    des_key[6] = Get7Bits(key, 42);
    des_key[7] = Get7Bits(key, 49);

#ifndef USE_CRYPT
    des_set_odd_parity((des_cblock *)des_key);
#endif

#if 0
    CHAPDEBUG((LOG_INFO, "MakeKey: 56-bit input : %02X%02X%02X%02X%02X%02X%02X",
	       key[0], key[1], key[2], key[3], key[4], key[5], key[6]));
    CHAPDEBUG((LOG_INFO, "MakeKey: 64-bit output: %02X%02X%02X%02X%02X%02X%02X%02X",
	       des_key[0], des_key[1], des_key[2], des_key[3], des_key[4], des_key[5], des_key[6], des_key[7]));
#endif
}

void
ChallengeHash(u_char PeerChallenge[16], u_char *rchallenge,
	      char *username, u_char Challenge[8])
    
{
    SHA1_CTX	sha1Context;
    u_char	sha1Hash[SHA1_SIGNATURE_SIZE];
    char	*user;

    /* remove domain from "domain\username" */
    if ((user = strrchr(username, '\\')) != NULL)
	++user;
    else
	user = username;

    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, PeerChallenge, 16);
    rtl_SHA1_Update(&sha1Context, rchallenge, 16);
    rtl_SHA1_Update(&sha1Context, (unsigned char *)user, strlen(user));
    rtl_SHA1_Final(sha1Hash, &sha1Context);

    BCOPY(sha1Hash, Challenge, 8);
}
/*
 * Convert the ASCII version of the password to Unicode.
 * This implicitly supports 8-bit ISO8859/1 characters.
 * This gives us the little-endian representation, which
 * is assumed by all M$ CHAP RFCs.  (Unicode byte ordering
 * is machine-dependent.)
 */
static void
ascii2unicode(char ascii[], int ascii_len, u_char unicode[])
{
    int i;

    BZERO(unicode, ascii_len * 2);
    for (i = 0; i < ascii_len; i++)
	unicode[i * 2] = (u_char) ascii[i];
}

static void
NTPasswordHash(u_char *secret, int secret_len, u_char hash[MD4_SIGNATURE_SIZE])
{
#if 1 //def __NetBSD__
    /* NetBSD uses the libc md4 routines which take bytes instead of bits */
    int			mdlen = secret_len;
#else
    int			mdlen = secret_len * 8;
#endif
    MD4_CTX		md4Context;

    MD4_Init(&md4Context);
    /* MD4Update can take at most 64 bytes at a time */
    while (mdlen > 512) {
	MD4_Update(&md4Context, secret, 512);
	secret += 64;
	mdlen -= 512;
    }
    MD4_Update(&md4Context, secret, mdlen);
    MD4_Final(hash, &md4Context);

}

static void
ChapMS_NT(rchallenge, rchallenge_len, secret, secret_len, response)
    char *rchallenge;
    int rchallenge_len;
    char *secret;
    int secret_len;
    MS_ChapResponse    *response;
{
    int			i;
    MD4_CTX		md4Context;
    u_char		hash[MD4_SIGNATURE_SIZE];
    u_char		unicodePassword[MAX_NT_PASSWORD * 2];

    /* Initialize the Unicode version of the secret (== password). */
    /* This implicitly supports 8-bit ISO8859/1 characters. */
    BZERO(unicodePassword, sizeof(unicodePassword));
    for (i = 0; i < secret_len; i++)
	unicodePassword[i * 2] = (u_char)secret[i];

    MD4_Init(&md4Context);
    MD4_Update(&md4Context, unicodePassword, secret_len * 2);	/* Unicode is 2 bytes/char */

    MD4_Final(hash, &md4Context); 	/* Tell MD4 we're done */

    ChallengeResponse(rchallenge, hash, response->NTResp);
}
	
static void
ChapMS2_NT(u_char *rchallenge, u_char PeerChallenge[16], char *username,
	   char *secret, int secret_len, u_char NTResponse[24])
{
	u_char	unicodePassword[MAX_NT_PASSWORD * 2];
	u_char	PasswordHash[MD4_SIGNATURE_SIZE];
	u_char	Challenge[8];

	ChallengeHash(PeerChallenge, rchallenge, username, Challenge);

	/* Hash the Unicode version of the secret (== password). */
	ascii2unicode(secret, secret_len, unicodePassword);
	NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);

	ChallengeResponse(Challenge, PasswordHash, NTResponse);
}


void
GenerateAuthenticatorResponse(u_char PasswordHashHash[MD4_SIGNATURE_SIZE],
			      u_char NTResponse[24], u_char PeerChallenge[16],
			      u_char *rchallenge, char *username,
			      u_char authResponse[MS_AUTH_RESPONSE_LENGTH+1])
{
    /*
     * "Magic" constants used in response generation, from RFC 2759.
     */
    u_char Magic1[39] = /* "Magic server to client signing constant" */
	{ 0x4D, 0x61, 0x67, 0x69, 0x63, 0x20, 0x73, 0x65, 0x72, 0x76,
	  0x65, 0x72, 0x20, 0x74, 0x6F, 0x20, 0x63, 0x6C, 0x69, 0x65,
	  0x6E, 0x74, 0x20, 0x73, 0x69, 0x67, 0x6E, 0x69, 0x6E, 0x67,
	  0x20, 0x63, 0x6F, 0x6E, 0x73, 0x74, 0x61, 0x6E, 0x74 };
    u_char Magic2[41] = /* "Pad to make it do more than one iteration" */
	{ 0x50, 0x61, 0x64, 0x20, 0x74, 0x6F, 0x20, 0x6D, 0x61, 0x6B,
	  0x65, 0x20, 0x69, 0x74, 0x20, 0x64, 0x6F, 0x20, 0x6D, 0x6F,
	  0x72, 0x65, 0x20, 0x74, 0x68, 0x61, 0x6E, 0x20, 0x6F, 0x6E,
	  0x65, 0x20, 0x69, 0x74, 0x65, 0x72, 0x61, 0x74, 0x69, 0x6F,
	  0x6E };

    int		i;
    SHA1_CTX	sha1Context;
    u_char	Digest[SHA1_SIGNATURE_SIZE];
    u_char	Challenge[8];

    rtl_SHA1_Init(&sha1Context);
	
    rtl_SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    rtl_SHA1_Update(&sha1Context, NTResponse, 24);
	
    rtl_SHA1_Update(&sha1Context, Magic1, sizeof(Magic1));
    rtl_SHA1_Final(Digest, &sha1Context);

    ChallengeHash(PeerChallenge, rchallenge, username, Challenge);

    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, Digest, sizeof(Digest));
	
    rtl_SHA1_Update(&sha1Context, Challenge, sizeof(Challenge));
    rtl_SHA1_Update(&sha1Context, Magic2, sizeof(Magic2));
	
    rtl_SHA1_Final(Digest, &sha1Context);

    /* Convert to ASCII hex string. */
    for (i = 0; i < MAX((MS_AUTH_RESPONSE_LENGTH / 2), sizeof(Digest)); i++)
	sprintf((char *)&authResponse[i * 2], "%02X", Digest[i]);
}


static void
GenerateAuthenticatorResponsePlain
		(char *secret, int secret_len,
		 u_char NTResponse[24], u_char PeerChallenge[16],
		 u_char *rchallenge, char *username,
		 u_char authResponse[MS_AUTH_RESPONSE_LENGTH+1])
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];

    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash),
		   PasswordHashHash);

    GenerateAuthenticatorResponse(PasswordHashHash, NTResponse, PeerChallenge,
				  rchallenge, username, authResponse);
}


#ifdef MPPE
/*
 * Set mppe_xxxx_key from the NTPasswordHashHash.
 * RFC 2548 (RADIUS support) requires us to export this function (ugh).
 */
void
mppe_set_keys(u_char *rchallenge, u_char PasswordHashHash[MD4_SIGNATURE_SIZE])
{
    SHA1_CTX	sha1Context;
    u_char	Digest[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */
#if 1
    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    rtl_SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    rtl_SHA1_Update(&sha1Context, rchallenge, 8);
    rtl_SHA1_Final(Digest, &sha1Context);
#else
	SHA1_Init(&sha1Context);
	SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
	SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
	SHA1_Update(&sha1Context, rchallenge, 8);
	SHA1_Final(Digest, &sha1Context);
#endif
    /* Same key in both directions. */
    BCOPY(Digest, mppe_send_key, sizeof(mppe_send_key));
    BCOPY(Digest, mppe_recv_key, sizeof(mppe_recv_key));

    mppe_keys_set = 1;
}

/*
 * Set mppe_xxxx_key from MS-CHAP credentials. (see RFC 3079)
 */
static void
Set_Start_Key(u_char *rchallenge, char *secret, int secret_len)
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];

    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash), PasswordHashHash);

    mppe_set_keys(rchallenge, PasswordHashHash);
}

/*
 * Set mppe_xxxx_key from MS-CHAPv2 credentials. (see RFC 3079)
 *
 * This helper function used in the Winbind module, which gets the
 * NTHashHash from the server.
 */
void
mppe_set_keys2(u_char PasswordHashHash[MD4_SIGNATURE_SIZE],
	       u_char NTResponse[24], int IsServer)
{
    SHA1_CTX	sha1Context;
    u_char	MasterKey[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */
    u_char	Digest[SHA1_SIGNATURE_SIZE];	/* >= MPPE_MAX_KEY_LEN */

    u_char SHApad1[40] =
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    u_char SHApad2[40] =
	{ 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2,
	  0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2, 0xf2 };

    /* "This is the MPPE Master Key" */
    u_char Magic1[27] =
	{ 0x54, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74,
	  0x68, 0x65, 0x20, 0x4d, 0x50, 0x50, 0x45, 0x20, 0x4d,
	  0x61, 0x73, 0x74, 0x65, 0x72, 0x20, 0x4b, 0x65, 0x79 };
    /* "On the client side, this is the send key; "
       "on the server side, it is the receive key." */
    u_char Magic2[84] =
	{ 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
	  0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
	  0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20, 0x6b, 0x65, 0x79,
	  0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x73,
	  0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73, 0x69, 0x64, 0x65,
	  0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
	  0x6b, 0x65, 0x79, 0x2e };
    /* "On the client side, this is the receive key; "
       "on the server side, it is the send key." */
    u_char Magic3[84] =
	{ 0x4f, 0x6e, 0x20, 0x74, 0x68, 0x65, 0x20, 0x63, 0x6c, 0x69,
	  0x65, 0x6e, 0x74, 0x20, 0x73, 0x69, 0x64, 0x65, 0x2c, 0x20,
	  0x74, 0x68, 0x69, 0x73, 0x20, 0x69, 0x73, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x72, 0x65, 0x63, 0x65, 0x69, 0x76, 0x65, 0x20,
	  0x6b, 0x65, 0x79, 0x3b, 0x20, 0x6f, 0x6e, 0x20, 0x74, 0x68,
	  0x65, 0x20, 0x73, 0x65, 0x72, 0x76, 0x65, 0x72, 0x20, 0x73,
	  0x69, 0x64, 0x65, 0x2c, 0x20, 0x69, 0x74, 0x20, 0x69, 0x73,
	  0x20, 0x74, 0x68, 0x65, 0x20, 0x73, 0x65, 0x6e, 0x64, 0x20,
	  0x6b, 0x65, 0x79, 0x2e };
    u_char *s;

    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, PasswordHashHash, MD4_SIGNATURE_SIZE);
    rtl_SHA1_Update(&sha1Context, NTResponse, 24);
    rtl_SHA1_Update(&sha1Context, Magic1, sizeof(Magic1));	
    rtl_SHA1_Final(MasterKey, &sha1Context);

    /*
     * generate send key
     */
    if (IsServer)
	s = Magic3;
    else
	s = Magic2;
    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, MasterKey, 16);
    rtl_SHA1_Update(&sha1Context, SHApad1, sizeof(SHApad1));
    rtl_SHA1_Update(&sha1Context, s, 84);
    rtl_SHA1_Update(&sha1Context, SHApad2, sizeof(SHApad2));
    rtl_SHA1_Final(Digest, &sha1Context);

    BCOPY(Digest, mppe_send_key, sizeof(mppe_send_key));

    /*
     * generate recv key
     */
    if (IsServer)
	s = Magic2;
    else
	s = Magic3;
    rtl_SHA1_Init(&sha1Context);
    rtl_SHA1_Update(&sha1Context, MasterKey, 16);
    rtl_SHA1_Update(&sha1Context, SHApad1, sizeof(SHApad1));
    rtl_SHA1_Update(&sha1Context, s, 84);
    rtl_SHA1_Update(&sha1Context, SHApad2, sizeof(SHApad2));
    rtl_SHA1_Final(Digest, &sha1Context);

    BCOPY(Digest, mppe_recv_key, sizeof(mppe_recv_key));

#if 0
	diag_printf("\n*****mppe_send_key************\n");
	{
		int i;
		for(i = 0 ; i < MPPE_MAX_KEY_LEN ; ++i)
		{
			diag_printf("%0x ",mppe_send_key[i]);
		}
		diag_printf("\n******mppe_recv_key************\n");		
		for(i = 0 ; i < MPPE_MAX_KEY_LEN ; ++i)
		{
			diag_printf("%0x ",mppe_recv_key[i]);
		}		
	}
	diag_printf("\n**********************************\n");	
#endif
    mppe_keys_set = 1;
}

/*
 * Set mppe_xxxx_key from MS-CHAPv2 credentials. (see RFC 3079)
 */
static void
SetMasterKeys(char *secret, int secret_len, u_char NTResponse[24], int IsServer)
{
    u_char	unicodePassword[MAX_NT_PASSWORD * 2];
    u_char	PasswordHash[MD4_SIGNATURE_SIZE];
    u_char	PasswordHashHash[MD4_SIGNATURE_SIZE];
    /* Hash (x2) the Unicode version of the secret (== password). */
    ascii2unicode(secret, secret_len, unicodePassword);
    NTPasswordHash(unicodePassword, secret_len * 2, PasswordHash);
    NTPasswordHash(PasswordHash, sizeof(PasswordHash), PasswordHashHash);
    mppe_set_keys2(PasswordHashHash, NTResponse, IsServer);
}

#endif /* MPPE */

#ifdef MSLANMAN
static u_char *StdText = (u_char *)"KGS!@#$%"; /* key from rasapi32.dll */

static void
ChapMS_LANMan(rchallenge, rchallenge_len, secret, secret_len, response)
    char *rchallenge;
    int rchallenge_len;
    char *secret;
    int secret_len;
    MS_ChapResponse	*response;
{
    int			i;
    u_char		UcasePassword[MAX_NT_PASSWORD]; /* max is actually 14 */
    u_char		PasswordHash[MD4_SIGNATURE_SIZE];

    /* LANMan password is case insensitive */
    BZERO(UcasePassword, sizeof(UcasePassword));
    for (i = 0; i < secret_len; i++)
       UcasePassword[i] = (u_char)toupper(secret[i]);
    DesEncrypt( StdText, UcasePassword + 0, PasswordHash + 0 );
    DesEncrypt( StdText, UcasePassword + 7, PasswordHash + 8 );
    ChallengeResponse(rchallenge, PasswordHash, response->LANManResp);
}
#endif

void
ChapMS(cstate, rchallenge, rchallenge_len, secret, secret_len)
    chap_state *cstate;
    char *rchallenge;
    int rchallenge_len;
    char *secret;
    int secret_len;
{
    MS_ChapResponse	response;
#ifdef MSLANMAN
    extern int ms_lanman;
#endif

#if 0
    CHAPDEBUG((LOG_INFO, "ChapMS: secret is '%.*s'", secret_len, secret));
#endif
    BZERO(&response, sizeof(response));

    /* Calculate both always */
    ChapMS_NT(rchallenge, rchallenge_len, secret, secret_len, &response);

#ifdef MSLANMAN
    ChapMS_LANMan(rchallenge, rchallenge_len, secret, secret_len, &response);

    /* prefered method is set by option  */
    response.UseNT = !ms_lanman;
#else
    response.UseNT = 1;
#endif

    BCOPY(&response, cstate->response, MS_CHAP_RESPONSE_LEN);
    cstate->resp_length = MS_CHAP_RESPONSE_LEN;
	
#ifdef MPPE
		Set_Start_Key(rchallenge, secret, secret_len);
#endif
}


/*
 * If PeerChallenge is NULL, one is generated and the PeerChallenge
 * field of response is filled in.  Call this way when generating a response.
 * If PeerChallenge is supplied, it is copied into the PeerChallenge field.
 * Call this way when verifying a response (or debugging).
 * Do not call with PeerChallenge = response.
 *
 * The PeerChallenge field of response is then used for calculation of the
 * Authenticator Response.
 */


void
chapms2_make_response(chap_state *cstate, u_char *rchallenge, int rchallenge_len,char *secret, int secret_len,
			int authenticator)
{
	/* ARGSUSED */
	unsigned char response[RESP_MAX_PKTLEN];
	int i;
	u_char *PeerChallenge;
	u_char *p = &response[MS_CHAP2_PEER_CHALLENGE];
	
#ifdef DEBUGMPPEKEY
	PeerChallenge=mschap2_peer_challenge,
#else
	PeerChallenge=NULL;
#endif
	


	BZERO(response, sizeof(response));

	/* Generate the Peer-Challenge if requested, or copy it if supplied. */
	if (!PeerChallenge)
		for (i = 0; i < MS_CHAP2_PEER_CHAL_LEN; i++)
			*p++ = (u_char) (drand48() * 0xff);
	else
		BCOPY(PeerChallenge, &response[MS_CHAP2_PEER_CHALLENGE],
			  MS_CHAP2_PEER_CHAL_LEN);
	

	/* Generate the NT-Response */
	ChapMS2_NT(rchallenge, &response[MS_CHAP2_PEER_CHALLENGE], cstate->resp_name,
		   secret, secret_len, &response[MS_CHAP2_NTRESP]);




	/* Generate the Authenticator Response. */
	GenerateAuthenticatorResponsePlain(secret, secret_len,
					   &response[MS_CHAP2_NTRESP],
					   &response[MS_CHAP2_PEER_CHALLENGE],
					   rchallenge, cstate->resp_name, cstate->priv);
	
	BCOPY(&response, cstate->response, MS_CHAP2_RESPONSE_LEN);
	cstate->resp_length=MS_CHAP2_RESPONSE_LEN;


#ifdef MPPE
	SetMasterKeys(secret, secret_len,
		  &response[MS_CHAP2_NTRESP], authenticator);
#endif


}


int
chapms2_check_success(chap_state *cstate,unsigned char *msg, int len)
{


	if ((len < MS_AUTH_RESPONSE_LENGTH + 2) ||
	    strncmp((char *)msg, "S=", 2) != 0) {
		/* Packet does not start with "S=" */
		diag_printf("MS-CHAPv2 Success packet is badly formed.");
		return 0;
	}
	msg += 2;
	len -= 2;
	if (len < MS_AUTH_RESPONSE_LENGTH
	    || memcmp(msg, cstate->priv, MS_AUTH_RESPONSE_LENGTH)) {
		/* Authenticator Response did not match expected. */
		diag_printf("MS-CHAPv2 mutual authentication failed.");
		return 0;
	}
	/* Authenticator Response matches. */
	msg += MS_AUTH_RESPONSE_LENGTH; /* Eat it */
	len -= MS_AUTH_RESPONSE_LENGTH;
	if ((len >= 3) && !strncmp((char *)msg, " M=", 3)) {
		msg += 3; /* Eat the delimiter */
	} else if (len) {
		/* Packet has extra text which does not begin " M=" */
		diag_printf("MS-CHAPv2 Success packet is badly formed.");
		return 0;
	}
	
	return 1;
}

void
chapms2_handle_failure(unsigned char *inp, int len)
{
	int err;
	char *p, *msg;
	//diag_printf("%s:%d",__FUNCTION__,__LINE__);
	/* We want a null-terminated string for strxxx(). */
	msg = malloc(len + 1);
	if (!msg) {
		diag_printf("Out of memory in chapms_handle_failure");
		return;
	}
	BCOPY(inp, msg, len);
	msg[len] = 0;
	p = msg;

	/*
	 * Deal with MS-CHAP formatted failure messages; just print the
	 * M=<message> part (if any).  For MS-CHAP we're not really supposed
	 * to use M=<message>, but it shouldn't hurt.  See
	 * chapms[2]_verify_response.
	 */
	if (!strncmp(p, "E=", 2))
		err = strtol(p+2, NULL, 10); /* Remember the error code. */
	else
		goto print_msg; /* Message is badly formatted. */

	if (len && ((p = strstr(p, " M=")) != NULL)) {
		/* M=<message> field found. */
		p += 3;
	} else {
		/* No M=<message>; use the error code. */
		switch (err) {
		case MS_CHAP_ERROR_RESTRICTED_LOGON_HOURS:
			p = "E=646 Restricted logon hours";
			break;

		case MS_CHAP_ERROR_ACCT_DISABLED:
			p = "E=647 Account disabled";
			break;

		case MS_CHAP_ERROR_PASSWD_EXPIRED:
			p = "E=648 Password expired";
			break;

		case MS_CHAP_ERROR_NO_DIALIN_PERMISSION:
			p = "E=649 No dialin permission";
			break;

		case MS_CHAP_ERROR_AUTHENTICATION_FAILURE:
			p = "E=691 Authentication failure";
			break;

		case MS_CHAP_ERROR_CHANGING_PASSWORD:
			/* Should never see this, we don't support Change Password. */
			p = "E=709 Error changing password";
			break;

		default:
			free(msg);
			diag_printf("Unknown MS-CHAP authentication failure: %.*v",
			      len, inp);
			return;
		}
	}
	//diag_printf("%s:%d",__FUNCTION__,__LINE__);
print_msg:
	if (p != NULL)
		diag_printf("MS-CHAP authentication failed: %v", p);
	free(msg);
}

#endif /* CHAPMS */
