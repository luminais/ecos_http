#
# ipfilter module Makefile for ecos bsp
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.9 2010-08-27 03:45:54 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(BSPDIR)" ""
	BSPDIR=$(shell cd .. ; pwd)
endif

include $(BSPDIR)/.config

#roy +++ 2010/09/17 get CONFIG_TENDA_WLJB and CONFIG_STAR_WARS_ENABLE
include $(TOPDIR)/.config
#---

CURRPATH = $(shell pwd)
IPF_PKG := ip_fil3.4.35

HDRFILES = ip_fil.h ip_nat.h ip_frag.h ip_state.h ip_proxy.h ip_auth.h ip_compat.h ipl.h \
	ip_ftp_pxy.c ip_rcmd_pxy.c ip_raudio_pxy.c ip_netbios_pxy.c ip_h323_pxy.c ip_ipsec_pxy.c ip_pptp_pxy.c ip_l2tp_pxy.c
ifdef CONFIG_TENDA_WLJB
HDRFILES += ip_http_pxy.c
endif

ifdef CONFIG_TENDA_CD_RTSP_IPTV
HDRFILES += ip_rtsp_pxy.c
endif

INSTALLHDRS = $(patsubst %, $(BSPDIR)/include/netinet/%, $(HDRFILES))

all: $(INSTALLHDRS) $(MODULE)

include $(BSPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(CURRPATH)/include -DINET -DKERNEL -Werror
EXTRA_CFLAGS += -I$(CURRPATH)/$(IPF_PKG) -I$(BSPDIR)/include
EXTRA_CFLAGS += -I$(BSPDIR)/net/ipdev -I$(BSPDIR)/net/kdev

#roy +++ 2010/09/17
ifdef CONFIG_TENDA_WLJB
EXTRA_CFLAGS += -DTENDA_WLJB
endif

ifdef CONFIG_STAR_WARS_ENABLE
EXTRA_CFLAGS += -DSTAR_WARS_ENABLE
endif
#---

#roy +++ 2012/05/07
ifdef CONFIG_TENDA_CD_RTSP_IPTV
EXTRA_CFLAGS += -DIPF_RTSP_PROXY
endif
#---

#
# Enumerate files to compile
#
VPATH = $(CURRPATH)/$(IPF_PKG)

MAINOBJ = ip_fil.o fil.o ip_nat.o ip_frag.o ip_state.o ip_proxy.o ip_auth.o
DEVOBJ = ipl_dev.o nat_dev.o
FASTOBJ = ip_filfast.o ip_fastnat.o macfilter.o
HACKOBJ = ip_def.o
URLFIL = urlfilter.o
WAN2LANFIL = wan2lanfilter.o
SDWHANDLE =url_match.o sdw_filter.o sdw_js_inject.o


ifdef CONFIG_IPFILTER_LOG
EXTRA_CFLAGS += -DIPFILTER_LOG
MAINOBJ += ip_log.o
endif
ifdef CONFIG_AL_SECURITY
AL_SECURITY	= al_security.o
endif

# ALL_OBJS = $(MAINOBJ) $(FASTOBJ) $(URLFIL) $(DEVOBJ) $(HACKOBJ)
# ALL_OBJS = $(MAINOBJ) $(DEVOBJ) $(FASTOBJ) $(URLFIL) 
# ALL_OBJS = $(MAINOBJ) $(DEVOBJ) $(FASTOBJ) $(URLFIL) $(HACKOBJ)
ALL_OBJS = $(MAINOBJ) $(DEVOBJ) $(FASTOBJ) $(URLFIL) $(WAN2LANFIL) $(SDWHANDLE)
ifdef CONFIG_AL_SECURITY
ALL_OBJS += $(AL_SECURITY)
endif
#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

$(INSTALLHDRS): $(foreach file,$(HDRFILES),$(CURRPATH)/$(IPF_PKG)/$(file))
	mkdir -p $(BSPDIR)/include/netinet
	@echo "copy $@"
	cp $(CURRPATH)/$(IPF_PKG)/$(notdir $@) $(BSPDIR)/include/netinet/$(notdir $@)

.PHONY:	clean
