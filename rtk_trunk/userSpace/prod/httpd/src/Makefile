#
# milli_httpd Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.13 2010-08-04 12:00:58 Exp $
#

include $(TENDA_APP_DIR)/.config


MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(ECOS_AP_DIR)/rules.mk

SRCPATH = .:$(SRCBASE)/router/shared:$(SRCBASE)/router/httpd

vpath %.c $(SRCPATH)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(SRCBASE)/router/httpd
EXTRA_CFLAGS += -I$(SRCBASE)/router/shared -I$(SRCBASE)/shared -I$(SRCBASE)/shared/zlib
EXTRA_CFLAGS += -I$(SRCBASE)/wps/brcm_apps/include
EXTRA_CFLAGS += -I$(TOPDIR)/cbb/src/bcmdhcpd/common -I$(TOPDIR)/cbb/src/bcmdhcpd/client -I$(TOPDIR)/cbb/src/bcmdhcpd/server
EXTRA_CFLAGS += -I$(BSPDIR)/net/include -I$(BSPDIR)/net/kdhcpc


#
# Enumerate files to compile
#
ALL_OBJS = cgi.o ej.o tar.o broadcom.o httpd.o httpd_ecos.o 

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
