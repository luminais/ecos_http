#
# ecos router pppd module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.5 2010-07-19 08:34:32 Exp $
#

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
PPPDIR = $(TOPDIR)/ppp/package
EXTRA_CFLAGS = -I$(shell pwd) -I$(BSPDIR)/include -I$(PPPDIR)/include/ecos -I$(SRCBASE)/include/bcmcrypto
EXTRA_CFLAGS += -DNO_DRAND48 -include typedefs.h
ifdef	CONFIG_CHAPMS
EXTRA_CFLAGS += -DCHAPMS -DUSE_CRYPT
endif

# debug

ifeq ($(PPPDEBUG),1)
EXTRA_CFLAGS += -DDEBUGALL
EXTRA_CFLAGS += -DDEBUGMAIN
EXTRA_CFLAGS += -DDEBUGSYS
EXTRA_CFLAGS += -DDEBUGFSM
EXTRA_CFLAGS += -DDEBUGLCP
EXTRA_CFLAGS += -DDEBUGIPCP
EXTRA_CFLAGS += -DDEBUGIPV6CP
EXTRA_CFLAGS += -DDEBUGUPAP
EXTRA_CFLAGS += -DDEBUGCHAP
endif

ifneq ($(findstring -DDEBUG, $(EXTRA_CFLAGS)),)
EXTRA_CFLAGS += -DDEBUG
endif

#
# Enumerate files to compile
#
ALL_OBJS = pppmain.o sys-ecos.o utils.o \
	fsm.o lcp.o magic.o auth.o chap-new.o chap-md5.o upap.o ipcp.o ccp.o

ifdef	CONFIG_CHAPMS
ALL_OBJS += chap_ms.o pppcrypt.o
endif

ifdef CONFIG_CHINA_NET_CLIENT
ALL_OBJS += tenda_pppencrypt.o
EXTRA_CFLAGS += -DCONFIG_CHINA_NET_CLIENT
endif

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
