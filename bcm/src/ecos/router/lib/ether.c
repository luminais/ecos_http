/*
 * Ethernet utility
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 *
 * $Id: ether.c,v 1.1.1.1 2010-04-09 10:37:02 Exp $
 */
#include <sys/param.h>
#include <net/ethernet.h>

/*
 * The re-entrant thread-safe versions of ether_ntoa().
 */
char *
ether_ntoa_r(const struct ether_addr *addr, char *buf)
{
	unsigned char *p = (unsigned char *)addr->octet;

	diag_sprintf(buf, "%02X:%02X:%02X:%02X:%02X:%02X",
	             p[0], p[1], p[2], p[3], p[4], p[5]);
	return buf;
}

/* 
 * Function converts the Ethernet host address addr given
 * in network byte order to a string in standard
 * hex-digits-and-colons notation, omitting leading zeroes.
 * The string is returned in a  statically allocated buffer,
 * which subsequent calls will overwrite.
 */
char *
ether_ntoa(const struct ether_addr *addr)
{
	static char a[] = "00:00:00:00:00:00";

	return ether_ntoa_r(addr, a);
}

/*
 * The re-entrant thread-safe versions of ether_aton().
 */
struct ether_addr *
ether_aton_r(const char *asc, struct ether_addr *addr)
{
	int i;
	unsigned char hex;
	unsigned char *s = (unsigned char *)asc;

	/* Eat white space */
	while (*s == ' ' || *s == '\t')
		s++;

	/*
	 * Expect 6 hex octets separated by ':' or
	 * space/null for the last octet.
	 */
	for (i = 0; i < 6; i++) {
		/* Process the first nibble */
		hex = 0;
		if (*s >= '0' && *s <= '9') {
			hex = *s - '0';
		}
		else if (*s >= 'A' && *s <= 'F') {
			hex = *s - 'A' + 0xa;
		}
		else if (*s >= 'a' && *s <= 'f') {
			hex = *s - 'a' + 0xa;
		}
		else {
			return 0;
		}

		/* Process the next nibble */
		s++;
		hex <<= 4;
		if (*s >= '0' && *s <= '9') {
			hex += *s - '0';
		}
		else if (*s >= 'A' && *s <= 'F') {
			hex += *s - 'A' + 0xa;
		}
		else if (*s >= 'a' && *s <= 'f') {
			hex += *s - 'a' + 0xa;
		}
		else {
			return 0;
		}

		/* Check ':' or end of string */
		s++;
		if (i < 5) {
			if (*s != ':') {
				return 0;
			}
			else {
				s++;
			}
		}
		else {
			if (*s != '\0' && *s != ' ' && *s != '\t') {
				return 0;
			}
		}

		addr->octet[i] = hex;
	}

	return addr;
}

/*
 * Converts the 48-bit Ethernet host address asc from the
 * standard hex-digits-and-colons notation into binary data
 * in network byte order and returns a pointer to it in a
 * statically allocated buffer, which subsequent calls will
 * overwrite. ether_aton() returns NULL if the address is
 * invalid.
 */
struct ether_addr *
ether_aton(const char *asc)
{
	static struct ether_addr n;

	return ether_aton_r(asc, &n);
}
