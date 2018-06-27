#
# DHCPD module Makefile 
# 
# Copyright (C) 2016, Tenda Tech. Co., All Rights Reserved.
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Tenda Tech. Co.;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Tenda Tech. Co.
#
# Description: ecos Makefile of DHCPD module
# Author : zhuhuan
# Version: 1.0
# Created Time: Tue 19 Jan 2016 14:21:36 PM CST
#


include $(TENDA_APP_DIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#
#EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include 
EXTRA_CFLAGS = -I. -I$(shell cd ../ ; pwd)/common 
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/kdhcpd
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/nvram
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/cli/include/
EXTRA_CFLAGS +=-D_KERNEL -DBCM47XX -D__ECOS -DROUTER_TINY -DCFG_GMAC -DBCMWPA2

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

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
