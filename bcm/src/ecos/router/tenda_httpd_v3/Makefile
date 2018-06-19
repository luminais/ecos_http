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

all: $(MODULE)

include $(TOPDIR)/rules.mak
include $(TOPDIR)/.config
include $(BSPDIR)/.config

#tenda add -I$(BSPDIR)/include for use kautoconf.h

EXTRA_CFLAGS += -I$(BSPDIR)/include -I$(SRCBASE)/wps/brcm_apps/include -I$(shell pwd)/include -I$(WPS_HAL)/include -I$(shell pwd)/../tc
ALL_OBJS	= main.o asp.o balloc.o base64.o default.o  \
		  ejlex.o ejparse.o form.o \
		  h.o handler.o mime.o misc.o page.o  \
		  ringq.o rom.o \
		  sock.o sockGen.o \
		  security.o sym.o uemf.o url.o value.o \
		  webs.o websuemf.o main_ecos.o route_cfg.o mtenda.o firewall.o tenda_cgi.o \
 		  wireless.o wireless_2.o wds_scan.o wps.o station_info.o tc_cgi.o  chlan.o
#		  webs.o websuemf.o mtenda.o wireless.o firewall.o tenda_cgi.o

EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME 


#ifeq ($(CONFIG_WEBS_PAGE_ROM),y)
	EXTRA_CFLAGS += -DWEBS_PAGE_ROM
	ALL_OBJS += webrom.o
#endif

ifdef CONFIG_CHINA_NET_CLIENT
	EXTRA_CFLAGS += -DCONFIG_CHINA_NET_CLIENT
endif

	
ifeq ($(CONFIG_A5),y)
	EXTRA_CFLAGS += -DA5
endif





#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
	rm -f webrom.c webrom.d webrom.o webcomp
	
# 
# This tool needs to be built using the native C compiler
#

WEB_COMP_EXTRA_CFLAGS =

ifeq ($(CONFIG_TENDA_MULTI),y)
	WEB_COMP_EXTRA_CFLAGS += -D__CONFIG_TENDA_MULTI__ 
endif

ifneq ($(CONFIG_WEB_VERSION),)
	WEB_COMP_EXTRA_CFLAGS += -D__CONFIG_WEB_VERSION__='$(CONFIG_WEB_VERSION)'
endif

ifeq ($(CONFIG_WL_USE_AP),y)
	WEB_COMP_EXTRA_CFLAGS += -D__NEED_DO_COMPILE0__
endif

webcomp:
	cc -o webcomp -DWEBS -DUEMF -DOS="Linux" -DLINUX -D_STRUCT_TIMEVAL $(WEB_COMP_EXTRA_CFLAGS) webcomp.c

#
# Build a set of ROMable pages
#

webrom.c: webcomp
	find web/$(CONFIG_WEB_DIR) -type f ! -path "*.svn*" >web_files
#	find web/$(CONFIG_WEB_DIR) -name "*.*" >web_files
	./webcomp web/$(CONFIG_WEB_DIR) web_files >webrom.c

clean:
	rm -f $(MODULE) $(ALL_OBJS) *.d
	rm -f webrom.c webcomp web_files

.PHONY:	clean

