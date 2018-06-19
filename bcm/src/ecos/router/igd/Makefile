#
# ecos router igd module Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.4 2010-05-26 14:23:59 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

include $(TOPDIR)/.config

all: $(MODULE) $(ALL_OBJS) $(ALL_OBJS:.o=.d)

include $(TOPDIR)/rules.mak

LIBUPNP = $(SRCBASE)/router/libupnp
SRCPATH = $(SRCBASE)/router/igd
DEVPATH = $(SRCPATH)/InternetGatewayDevice
OSLPATH = $(SRCPATH)/ecos

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(LIBUPNP)/include -I$(LIBUPNP)/ecos -I$(SRCPATH) -I$(DEVPATH) -I$(OSLPATH)
EXTRA_CFLAGS += -Wall

#
# Enumerate files to compile
#
VPATH = $(SRCPATH):$(DEVPATH):$(OSLPATH)

INDIRECT_SOURCES =  \
	igd_ecos_main.c \
	igd_ecos_osl.c \
	igd_mainloop.c \
	InternetGatewayDevice_table.c \
	InternetGatewayDevice.c \
	soap_x_layer3forwarding.c \
	soap_x_wancommoninterfaceconfig.c \
	soap_x_wanipconnection.c \
	xml_InternetGatewayDevice.c \
	xml_x_layer3forwarding.c \
	xml_x_wancommoninterfaceconfig.c \
	xml_x_wanipconnection.c

ALL_OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f

.PHONY:	clean
