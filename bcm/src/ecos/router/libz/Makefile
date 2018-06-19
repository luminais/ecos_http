#
# ecos router shared functions Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-09-15 10:59:36 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -Wall -I$(SRCBASE)/shared/zlib -DDEF_WBITS=-15

#
# Enumerate files to compile
#
vpath %.c $(SRCBASE)/shared/zlib
vpath %.h $(SRCBASE)/shared/zlib

ALL_OBJS = crc32.o adler32.o uncompr.o trees.o zutil.o \
	infblock.o infcodes.o inffast.o inflate.o inftrees.o infutil.o gzio.o deflate.o

#
# Leave rule to make
#
$(MODULE):  $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	all clean
