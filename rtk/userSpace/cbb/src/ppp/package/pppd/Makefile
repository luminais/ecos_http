#
# PPPD module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# Description: ecos Makefile of PPPD module
# Author : zhuhuan
# Version: 1.0
# Created Time: Tue 19 Jan 2016 14:21:36 PM CST
#


MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

# router config
include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#
PPPDIR = $(TENDA_APP_DIR)/cbb/src/ppp/package
EXTRA_CFLAGS = -I$(shell pwd) -I$(TENDA_APP_DIR)/net_drive/include -I$(PPPDIR)/include/ecos -I$(TENDA_APP_DIR)/include/bcmcrypto
EXTRA_CFLAGS += -DNO_DRAND48 -include typedefs.h
EXTRA_CFLAGS += -D_KERNEL -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -DCHAPMS -DUSE_CRYPT
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/lib
EXTRA_CFLAGS += -I$(PPPDIR)/include
ifdef	CONFIG_CHAPMS
EXTRA_CFLAGS += -DCHAPMS -DUSE_CRYPT
endif
EXTRA_CFLAGS += -include config.h

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
endif

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $@ $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean