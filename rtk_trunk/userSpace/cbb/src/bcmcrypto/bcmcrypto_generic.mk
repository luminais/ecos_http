#
# ecos router bcmcrypto module generic Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: bcmcrypto_generic.mk,v 1.1 2010-08-20 10:21:13 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/router/libbcmcrypto/openssl
EXTRA_CFLAGS += -Wall

#
# Enumerate files to compile
#
VPATH := $(SRCBASE)/bcmcrypto

ifeq ($(strip $(CONFIG_WL_CONF)),)
$(error CONFIG_WL_CONF is undefined)
endif

WLCFGDIR := $(SRCBASE)/wl/config
WLCFGFILE := $(shell ls $(WLCFGDIR)/$(CONFIG_WL_CONF))
include $(WLCFGFILE)

#NAS
INDIRECT_SOURCES =

# NAS, Wireless driver STA mode will include them
ifneq ($(BCMSUP_PSK),1)
INDIRECT_SOURCES += aeskeywrap.c passhash.c prf.c sha1.c hmac.c md5.c
endif
ifneq ($(BCMCCX),1)
INDIRECT_SOURCES += md4.c
endif

# SES, WPS
INDIRECT_SOURCES += bn.c dh.c random.c
# WPS
INDIRECT_SOURCES += sha256.c hmac_sha256.c

OBJS := $(INDIRECT_SOURCES:.c=.o)
