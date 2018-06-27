#ifndef __RA_H__
#define __RA_H__

/************************************************************************
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
 *************************************************************************/
#define RA_MAXPREFIX 4
#define RA_PREFIX_LEN 128
#define RA_INTF_LEN 32

struct routeradv_prefix_opt {
    char prefix_addr[RA_PREFIX_LEN];
    int prefix_len;
    unsigned int pref_lifetime;
    unsigned int valid_lifetime;
    int rtadvd_aFlag;
};

struct routeradv_opt {
    int rtadvd_mFlag;
    int rtadvd_oFlag;
    unsigned int ra_maxinterval;
    unsigned int ra_mininterval;
    char ra_interface[RA_INTF_LEN];
    struct routeradv_prefix_opt rapfopt[RA_MAXPREFIX];
};


extern struct routeradv_opt rtadvd_opt;
#endif //__RA_H__