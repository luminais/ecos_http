#
# ecos router root config Makefile
# 
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: Makefile,v 1.17 2010-07-24 12:50:36 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

# router config file
include $(TOPDIR)/.config

# sys config file
include $(BSPDIR)/.config

all: $(MODULE)
include $(TOPDIR)/rules.mak

#VPATH = $(ARCH_DIR)

#
# Set CFLAGS or EXTRA_CFLAGS
#
ifdef CONFIG_TENDA_HTTPD_UCD
EXTRA_CFLAGS = -I. -Werror   -I$(shell cd .. ; pwd)/tenda_httpd_ucd/ 
endif

ifdef CONFIG_TENDA_HTTPD_V3
EXTRA_CFLAGS = -I. -Werror   -I$(shell cd .. ; pwd)/tenda_httpd_v3/ 
endif

ifdef CONFIG_TENDA_HTTPD_NORMAL
EXTRA_CFLAGS = -I. -Werror   -I$(shell cd .. ; pwd)/tenda_httpd/ 
endif

ifdef CONFIG_NEW_INFRASTRUCTURE
EXTRA_CFLAGS += -I$(shell cd .. ; pwd)/infrastructure/include
endif

CFLAGS+=-I../nvram/
#
# Enumerate files to compile
#
# include .config if any compile option
ALL_OBJS = rc.o network.o services.o run_sh.o
ALL_OBJS += interface.o sntpc.o dns.o


ifdef CONFIG_VLAN
	ALL_OBJS += vlan_config.o
endif

ifdef CONFIG_BRIDGE
	ALL_OBJS += br_start.o 
endif

ifdef CONFIG_NAT
	ALL_OBJS += wan_link.o
	ALL_OBJS += firewall.o

# For WAN DHCPC Module
ifdef CONFIG_PPTP
	ALL_OBJS += pptp.o
endif
ifdef CONFIG_PPTP
	ALL_OBJS += l2tp.o
endif
ifdef CONFIG_PPP
	ALL_OBJS += ppp.o
endif
endif

ifdef CONFIG_PPPOE2
	ALL_OBJS += pppoe2.o
endif
# For DHCPC Module
ifdef CONFIG_DHCPC
	ALL_OBJS += bcmdhcpc.o 
endif

#ALL_OBJS += thread_check.o 
ifdef CONFIG_RESTART_CHECK
ALL_OBJS += restart_check.o 
endif
ALL_OBJS += wan_mode_check.o 
ALL_OBJS += wan_surfing_check.o 
ALL_OBJS += wl_restart_check.o
#ALL_OBJS += sdw_httpDownLoad.o
ALL_OBJS += md5c.o lm_aes.o lm_login_keep.o sdw_upgrade.o

ifdef CONFIG_CHINA_NET_CLIENT
EXTRA_CFLAGS += -DCONFIG_CHINA_NET_CLIENT
endif

#
# Recursively to next directory
#
SUBDIRS = 

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /%.o, $(SUBDIRS)))

ALL_OBJS += $(SUB_OBJS)

#
# Leave rule to make
#
$(MODULE): subdirs $(ALL_OBJS) $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	find . \( -name '*.[od]' -o -name '*.tmp' \) -type f -print | xargs rm -f

.PHONY:	clean
