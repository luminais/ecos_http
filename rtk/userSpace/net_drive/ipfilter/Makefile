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

all: $(INSTALLHDRS) $(MODULE)

include $(ECOS_AP_DIR)/rules.mk
include $(ECOS_WORK_DIR)/.config

CURR_DIR = $(shell pwd)
IPF_PKG := ip_fil3.4.35

HDRFILES = ip_fil.h ip_nat.h ip_frag.h ip_state.h ip_proxy.h ip_auth.h ip_compat.h ipl.h \
	ip_ftp_pxy.c ip_rcmd_pxy.c ip_raudio_pxy.c ip_netbios_pxy.c ip_h323_pxy.c ip_ipsec_pxy.c ip_pptp_pxy.c ip_l2tp_pxy.c
ifdef CONFIG_TENDA_WLJB
HDRFILES += ip_http_pxy.c
endif

ifdef CONFIG_TENDA_CD_RTSP_IPTV
HDRFILES += ip_rtsp_pxy.c
endif

INSTALLHDRS = $(patsubst %, $(NETDRV_DIR)/include/netinet/%, $(HDRFILES))



#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS  = -I$(CURR_DIR)/include -DINET -DKERNEL -D__INSIDE_NET -DBCMDMA32 
EXTRA_CFLAGS += -I$(CURR_DIR)/$(IPF_PKG) -I$(NETDRV_DIR)/include
EXTRA_CFLAGS += -I$(NETDRV_DIR)/ipdev -I$(NETDRV_DIR)/kdev -I$(TAPT_DIR)/shared
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
VPATH = $(CURR_DIR)/$(IPF_PKG)

#MAINOBJ = ip_fil.o fil.o 
#ip_nat.o ip_frag.o ip_state.o ip_proxy.o ip_auth.o
DEVOBJ  = ipl_dev.o 
#nat_dev.o
FASTOBJ = ip_filfast.o macfilter.o
#ip_fastnat.o 
#HACKOBJ = ip_def.o
URLFIL  = urlfilter.o
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
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
	
ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

$(INSTALLHDRS): $(foreach file,$(HDRFILES),$(CURR_DIR)/$(IPF_PKG)/$(file))
	mkdir -p $(NETDRV_DIR)include/netinet
	@echo "copy $@"
	cp $(CURR_DIR)/$(IPF_PKG)/$(notdir $@) $(NETDRV_DIR)/include/netinet/$(notdir $@)

.PHONY:	clean
