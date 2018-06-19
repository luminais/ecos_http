/*
 * TLV option of DHCP client protocol.
 *
 * Copyright (C) 2010, Broadcom Corporation
 * All Rights Reserved.
 * 
 * This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
 * the contents of this file may not be disclosed to third parties, copied
 * or duplicated in any form, in whole or in part, without the prior
 * written permission of Broadcom Corporation.
 * 
 * $Id: dhcp_option.c,v 1.2 2010-06-07 06:06:23 Exp $
 *
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dhcp_var.h>
#include <dhcp_option.h>

unsigned char *
dhcp_get_option(struct dhcphdr *dhcp, int type, int dest_len, void *dest)
{
	unsigned char *ptr = dhcp->options;
	unsigned char *end = ptr + sizeof(dhcp->options);

	unsigned char overload_need = OVERLOAD_NONE;
	unsigned char overload_done = OVERLOAD_NONE;
	int tag;
	int optlen;

	while (ptr < end) {
		/* Matching code */
		tag = (int)ptr[OPTION_TAG];

		/* Skip padding */
		if (tag == DHCP_PADDING) {
			ptr++;
			continue;
		}

		/* Check TLV ranging */
		if (tag == DHCP_END) {
			if ((overload_need & OVERLOAD_FILE) !=
			   (overload_done & OVERLOAD_FILE)) {
				/* Set options starting from file field */
				ptr = dhcp->file;
				end = ptr + sizeof(dhcp->file);

				overload_done |= OVERLOAD_FILE;
			}
			else if ((overload_need & OVERLOAD_SNAME) !=
				(overload_done & OVERLOAD_SNAME)) {
				/* Set options starting from sname field */
				ptr = dhcp->sname;
				end = ptr + sizeof(dhcp->sname);

				overload_done |= OVERLOAD_SNAME;
			}
			else {
				return 0;
			}

			/* Continue to parse overload portion */
			continue;
		}

		/*
		 * Make sure there is enough space to
		 * hold DHCP_END.
		 */
		if (ptr + (OPTION_HDRLEN + ptr[OPTION_LEN]) > end-1)
			return 0;

		/* Check code matching */
		if (tag == type) {
			if (dest) {
				/* Give up when the option length is too long */
				optlen = (int)ptr[OPTION_LEN];
				if (optlen > dest_len)
					return 0;
				memcpy(dest, &ptr[OPTION_VALUE], optlen);
			}
			return &ptr[OPTION_VALUE];
		}

		/* Advance to next */
		ptr += (OPTION_HDRLEN + ptr[OPTION_LEN]);
	}

	return 0;
}

int
dhcp_add_option(struct dhcphdr *dhcp, unsigned char type, unsigned char len, void *value)
{
	unsigned char *ptr = dhcp->options;
	unsigned char *end = ptr + sizeof(dhcp->options);
	unsigned char *opt_list = 0;

	/*
	 * The add option has two basic assumption.
	 * 1. If this option was not added before,
	 *    it will be append the end of the options.
	 * 2. When it is found, we will take it as
	 *	  a server list append request.  The value
	 *    will be insert to the found TLV.
	 */
	while (*ptr != DHCP_END && ptr < end) {
		if (*ptr == DHCP_PADDING)
			ptr++;
		else {
			/* Added before, must be a option list */
			if (ptr[OPTION_TAG] == type)
				opt_list = ptr;

			ptr += OPTION_HDRLEN + ptr[OPTION_LEN];
		}
	}

	/*
	 * If not a option list to insert,
	 * append it to end of the options.
	 */
	if (opt_list == 0) {
		/*
		 * The TLV takes 2+len bytes,
		 * and DHCP_END takes one byte
		 */
		if (ptr + OPTION_HDRLEN + len > end-1)
			return 0;

		/* Append TLV */
		ptr[OPTION_TAG] = type;
		ptr[OPTION_LEN] = len;
		memcpy(&ptr[OPTION_VALUE], value, len);

		/* End it */
		ptr += OPTION_HDRLEN + len;
		*ptr = DHCP_END;
		return (OPTION_HDRLEN + len);
	}
	else {
		unsigned char *next_tag;
		int move_len;

		/* Make sure we have more space to insert. */
		if (ptr + len > end-1)
			return 0;

		/* Insert V */
		next_tag = opt_list + OPTION_HDRLEN + opt_list[OPTION_LEN];
		move_len = (ptr + 1) - next_tag;

		/* Shift "len" bytes of space */ 
		memmove(next_tag + len, next_tag, move_len);

		/* Inser value */
		memcpy(next_tag, value, len);
		opt_list[OPTION_LEN] += len;
		return len;
	}
}

int
dhcp_option_len(unsigned char *ptr)
{
	unsigned char *end = ptr;

	/* Search DHCP_END */
	while (*end != DHCP_END) {
		if (*end == DHCP_PADDING)
			end++;
		else
			end += OPTION_HDRLEN + end[OPTION_LEN];
	}

	return (int)(end+1 - ptr);
}
