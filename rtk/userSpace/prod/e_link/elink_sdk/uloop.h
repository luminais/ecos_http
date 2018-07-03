/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef _ULOOP_H__
#define _ULOOP_H__
#include "list.h"
#include <sys/time.h>



struct uloop_timeout;

typedef void (*uloop_timeout_handler)(struct uloop_timeout *t);



struct uloop_timeout
{
	struct list_head list;
	bool pending;

	uloop_timeout_handler cb;
	struct timeval time;
};

extern bool uloop_cancelled;

int uloop_timeout_add(struct uloop_timeout *timeout);
int uloop_timeout_set(struct uloop_timeout *timeout, int msecs);
int uloop_timeout_cancel(struct uloop_timeout *timeout);
int uloop_timeout_remaining(struct uloop_timeout *timeout);
bool uloop_cancelling(void);

static inline void uloop_end(void)
{
	uloop_cancelled = true;
}

int uloop_run_timeout(int timeout);
static inline int uloop_run(void)
{
	return uloop_run_timeout(-1);
}
void uloop_done(void);

#endif
