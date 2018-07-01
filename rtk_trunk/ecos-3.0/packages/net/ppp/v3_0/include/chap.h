//==========================================================================
//
//      include/chap.h
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
 * chap.h - Challenge Handshake Authentication Protocol definitions.
 *
 * Copyright (c) 1993 The Australian National University.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the Australian National University.  The name of the University
 * may not be used to endorse or promote products derived from this
 * software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Copyright (c) 1991 Gregory M. Christy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the author.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $FreeBSD: src/usr.sbin/pppd/chap.h,v 1.7 1999/08/28 01:19:01 peter Exp $
 */

#ifndef __CHAP_INCLUDE__

/* Code + ID + length */
#define CHAP_HEADERLEN		4

/*
 * CHAP codes.
 */

#define CHAP_DIGEST_MD5		5	/* use MD5 algorithm */
#define MD5_SIGNATURE_SIZE	16	/* 16 bytes in a MD5 message digest */
#define CHAP_MD5		5
#define CHAP_MICROSOFT		0x80	/* use Microsoft-compatible alg. */
#define CHAP_MICROSOFT_V2	0x81

#define MS_CHAP_RESPONSE_LEN	49	/* Response length for MS-CHAP */

/*
 * Offsets within the response field for MS-CHAP2
 */
#define MS_CHAP2_PEER_CHALLENGE	0
#define MS_CHAP2_PEER_CHAL_LEN	16
#define MS_CHAP2_RESERVED_LEN	8
#define MS_CHAP2_NTRESP		24
#define MS_CHAP2_NTRESP_LEN	24
#define MS_CHAP2_FLAGS		48

#define CHAP_CHALLENGE		1
#define CHAP_RESPONSE		2
#define CHAP_SUCCESS		3
#define CHAP_FAILURE    	4

/*
 *  Challenge lengths (for challenges we send) and other limits.
 */
#define MIN_CHALLENGE_LENGTH	32
#define MAX_CHALLENGE_LENGTH	64
#define MAX_RESPONSE_LENGTH	64	/* sufficient for MD5 or MS-CHAP */


#define CHAL_MAX_PKTLEN	(PPP_HDRLEN + CHAP_HEADERLEN + 4 + MAX_CHALLENGE_LENGTH + MAXNAMELEN)
#define RESP_MAX_PKTLEN	(PPP_HDRLEN + CHAP_HEADERLEN + 4 + MAX_RESPONSE_LENGTH + MAXNAMELEN)

/*
 * Each interface is described by a chap structure.
 */

typedef struct chap_state {
    int unit;			/* Interface unit number */
    int clientstate;		/* Client state */
    int serverstate;		/* Server state */
    u_char challenge[MAX_CHALLENGE_LENGTH]; /* last challenge string sent */
    u_char chal_len;		/* challenge length */
    u_char chal_id;		/* ID of last challenge */
    u_char chal_type;		/* hash algorithm for challenges */
    u_char id;			/* Current id */
    char *chal_name;		/* Our name to use with challenge */
    int chal_interval;		/* Time until we challenge peer again */
    int timeouttime;		/* Timeout time in seconds */
    int max_transmits;		/* Maximum # of challenge transmissions */
    int chal_transmits;		/* Number of transmissions of challenge */
    int resp_transmits;		/* Number of transmissions of response */
    u_char response[MAX_RESPONSE_LENGTH];	/* Response to send */
    u_char resp_length;		/* length of response */
    u_char resp_id;		/* ID for response messages */
    u_char resp_type;		/* hash algorithm for responses */
    char *resp_name;		/* Our name to send with response */
	unsigned char priv[64];
} chap_state;


/* Return the digest alg. ID for the most preferred digest type. */
#define CHAP_DIGEST(mdtype) \
    ((mdtype) & MDTYPE_MD5)? CHAP_MD5: \
    ((mdtype) & MDTYPE_MICROSOFT_V2)? CHAP_MICROSOFT_V2: \
    ((mdtype) & MDTYPE_MICROSOFT)? CHAP_MICROSOFT: \
    0

/* Return the bit flag (lsb set) for our most preferred digest type. */
#define CHAP_MDTYPE(mdtype) ((mdtype) ^ ((mdtype) - 1)) & (mdtype)

/* Return the bit flag for a given digest algorithm ID. */
#define CHAP_MDTYPE_D(digest) \
    ((digest) == CHAP_MICROSOFT_V2)? MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? MDTYPE_MICROSOFT: \
    ((digest) == CHAP_MD5)? MDTYPE_MD5: \
    0

/* Can we do the requested digest? */
#define CHAP_CANDIGEST(mdtype, digest) \
    ((digest) == CHAP_MICROSOFT_V2)? (mdtype) & MDTYPE_MICROSOFT_V2: \
    ((digest) == CHAP_MICROSOFT)? (mdtype) & MDTYPE_MICROSOFT: \
    ((digest) == CHAP_MD5)? (mdtype) & MDTYPE_MD5: \
    0

/*
 * The code for each digest type has to supply one of these.
 */
struct chap_digest_type {
	int code;

	/*
	 * Note: challenge and response arguments below are formatted as
	 * a length byte followed by the actual challenge/response data.
	 */
	void (*generate_challenge)(unsigned char *challenge);
	int (*verify_response)(int id, char *name,
		unsigned char *secret, int secret_len,
		unsigned char *challenge, unsigned char *response,
		char *message, int message_space);
	void (*make_response)(unsigned char *response, int id, char *our_name,
		unsigned char *challenge, char *secret, int secret_len,
		unsigned char *priv);
	int (*check_success)(unsigned char *pkt, int len, unsigned char *priv);
	void (*handle_failure)(unsigned char *pkt, int len);

	struct chap_digest_type *next;
};

/*
 * Client (peer) states.
 */
#define CHAPCS_INITIAL		0	/* Lower layer down, not opened */
#define CHAPCS_CLOSED		1	/* Lower layer up, not opened */
#define CHAPCS_PENDING		2	/* Auth us to peer when lower up */
#define CHAPCS_LISTEN		3	/* Listening for a challenge */
#define CHAPCS_RESPONSE		4	/* Sent response, waiting for status */
#define CHAPCS_OPEN		5	/* We've received Success */

/*
 * Server (authenticator) states.
 */
#define CHAPSS_INITIAL		0	/* Lower layer down, not opened */
#define CHAPSS_CLOSED		1	/* Lower layer up, not opened */
#define CHAPSS_PENDING		2	/* Auth peer when lower up */
#define CHAPSS_INITIAL_CHAL	3	/* We've sent the first challenge */
#define CHAPSS_OPEN		4	/* We've sent a Success msg */
#define CHAPSS_RECHALLENGE	5	/* We've sent another challenge */
#define CHAPSS_BADAUTH		6	/* We've sent a Failure msg */

/*
 * Timeouts.
 */
#define CHAP_DEFTIMEOUT		3	/* Timeout time in seconds */
#define CHAP_DEFTRANSMITS	10	/* max # times to send challenge */

extern chap_state chap[];

void ChapAuthWithPeer __P((int, char *, int));
void ChapAuthPeer __P((int, char *, int));

extern struct protent chap_protent;

#define __CHAP_INCLUDE__
#endif /* __CHAP_INCLUDE__ */
