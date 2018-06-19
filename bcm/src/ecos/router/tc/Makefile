#
# ecos router Stream Control module Makefile
# 
# Copyright (C) 2010, Tenda Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.0 2010/13/56 Exp $
#	$author: stanley$
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

include $(TOPDIR)/.config

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#	eg: BSPDIR=../../src/ecos/bsp
EXTRA_CFLAGS = -I$(BSPDIR)/include -I$(shell pwd)	-I$(shell cd .. ; pwd)/include    
#-Werror
ifdef CONFIG_TENDA_HTTPD_UCD
EXTRA_CFLAGS +=	-I$(shell cd .. ; pwd)/tenda_httpd_ucd/ 
endif

ifdef CONFIG_TENDA_HTTPD_V3
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/tenda_httpd_v3/ 
endif

ifdef CONFIG_TENDA_HTTPD_NORMAL
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/tenda_httpd/ 
endif


#
# Enumerate files to compile
#
ALL_OBJS = tc.o

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean

