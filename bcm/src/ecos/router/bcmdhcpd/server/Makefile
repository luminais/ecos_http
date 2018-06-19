#
# BCM DHCP module Makefile for eCos router
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.1 2010-06-04 10:40:48 Exp $
#

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
#EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include -Werror
EXTRA_CFLAGS = -I. -I$(shell cd ../../ ; pwd)/common -I$(BSPDIR)/net/kdhcpd -Werror

ifdef CONFIG_TENDA_HTTPD_UCD
EXTRA_CFLAGS += -I$(shell cd ../../ ; pwd)/tenda_httpd_ucd/ 
endif

ifdef CONFIG_TENDA_HTTPD_V3
EXTRA_CFLAGS += -I$(shell cd ../../ ; pwd)/tenda_httpd_v3/ 
endif

ifdef CONFIG_TENDA_HTTPD_NORMAL
EXTRA_CFLAGS += -I$(shell cd ../../ ; pwd)/tenda_httpd/ 
endif


#
# Enumerate files to compile
#
ALL_OBJS = dhcpd.o dhcpd_lease.o dhcpd_packet.o dhcpd_ecos.o scheduler.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
