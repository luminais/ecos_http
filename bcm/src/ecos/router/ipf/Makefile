#
# ecos router ipfilter tools Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-08-20 04:09:29 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

IPF_PKG := ip_fil3.4.35

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(BSPDIR)/include -I$(BSPDIR)/net/ipfilter/$(IPF_PKG) -I$(BSPDIR)/net/ipfilter/include -Werror

#
# Enumerate files to compile
#
VPATH = $(BSPDIR)/net/ipfilter/$(IPF_PKG)
IPNAT = ipnat.o natparse.o common.o
IPF = parse.o opt.o facpri.o
IPNAT += printnat.o
FILS = printstate.o
CMD = natcmd.o filcmd.o

ifdef CONFIG_IPFILTER_LOG
EXTRA_CFLAGS += -DIPFILTER_LOG
endif

ALL_OBJS = $(IPNAT) $(IPF) $(CMD)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f

.PHONY:	clean
