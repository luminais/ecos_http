#
# ecos router supported library Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.11 2010-07-04 02:09:34 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

include $(TOPDIR)/.config

include $(TOPDIR)/rules.mak

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(TOPDIR)/include/crypto
EXTRA_CFLAGS += -I$(SRCBASE)/include

#
# Enumerate files to compile
#

VPATH = $(SRCBASE)/router/shared

ALL_OBJS = ping.o iflib.o \
	   ether.o ifaddrs.o inflate.o base64.o \
	   libc2.o ecos_oslib.o hcreate.o hash_func.o syslog.o \
	   libsocket.o libloipc.o blowfish.o

# Added for PPP MSCHAP 
ALL_OBJS += crypt.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
