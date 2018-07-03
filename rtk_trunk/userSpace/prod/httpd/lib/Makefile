#
# milli_httpd Makefile
#
# Copyright (C) 2010, Broadcom Corporation. All Rights Reserved.
# 
# Permission to use, copy, modify, and/or distribute this software for any
# purpose with or without fee is hereby granted, provided that the above
# copyright notice and this permission notice appear in all copies.
# 
# THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
# MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
# SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
# OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
# CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
#
# $Id: Makefile,v 1.3 2010-05-22 08:11:53 Exp $
#

include $(TOPDIR)/.config


MODULE = $(shell pwd | sed "s/.*\///" ).o

vpath %.c $(SRCBASE)/router/shared

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(SRCBASE)/router/httpd
EXTRA_CFLAGS += -I$(SRCBASE)/router/shared -I$(SRCBASE)/include
EXTRA_CFLAGS += -Werror

rem_upgrade.o: EXTRA_CFLAGS += -I$(SRCBSE)/bsp/kernel/ecos_install/include 

#
# Enumerate files to compile
#
ALL_OBJS = tarbase.o rem_upgrade.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
