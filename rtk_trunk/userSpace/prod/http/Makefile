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


include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config


# CONFIG_WEB_DIR="F3_normal_en"
# CONFIG_WEB_VERSION="en"
# CONFIG_Tenda_DEFAULT_PROFILE="F3_normal_en.c"
#tenda add -I$(BSPDIR)/include for use kautoconf.h

EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/cbb/src/tc -I$(TENDA_APP_DIR)/include -I$(TENDA_APP_DIR)/router_shared  -I./include -I. -I$(APP_DIR)/tenda_paltform/shared_common -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -include autoconf.h
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/cli/include/ -I$(TENDA_APP_DIR)/cbb/src/ddns/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/lib/cJSON/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/ipfilter/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/ipf
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -DWEBS_PAGE_ROM -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X

SRCPATH = src/
WEB_DIR = $(CONFIG_WEB_DIR)_mini

CGIBOOTPATH = cgi/
CGIPATH = cgi/ecos_cgi/
CGILAYERPATH = cgi/cgi_moddle_layer/
CGILIBPATH = cgi/cgi_lib/
VPATH = $(SRCPATH):$(CGIPATH):$(CGILAYERPATH):$(CGILIBPATH)

EXTRA_CFLAGS += -I$(CGIPATH)/include/ -I$(CGIBOOTPATH)/include/

ALL_OBJS	= main.o asp.o balloc.o base64.o default.o  \
		  ejlex.o ejparse.o form.o \
		  h.o handler.o mime.o misc.o page.o  \
		  ringq.o rom.o \
		  sock.o sockGen.o \
		  security.o sym.o uemf.o url.o value.o \
		  webs.o websuemf.o main_ecos.o cgi_devManage.o \
		  chlan.o cgi_web.o  \
		  cgi_firewall.o \
		  cgi_wifi.o cgi_wifi_scan.o  cgi_login.o cgi_systools.o cgi_parent.o \
          cgi_macfilter.o cgi_config.o cgi_interface.o cgi_wifi_common.o wifi_channel.o

# 去掉两个文件的编译cgi_wireless_pwr.o cgi_wireless_ctl.o 同时也注释掉了main.c中的wirelessPwrAspGoformDefine、wirelessCtlAspGoformDefine
ALL_OBJS += cgi_wan.o cgi_lan.o cgi_systime.o cgi_nat.o

ALL_OBJS += cgi_goform_define.o cgi_wizard.o cgi_wifiRelay.o


ALL_OBJS += cgi_remoteweb.o

ALL_OBJS += cgi_lib_wan.o cgi_lib_config.o cgi_lib_interface.o cgi_lib_wifi.o cgi_lib_nat.o\
            cgi_lib_lan.o cgi_lib_tc.o cgi_lib_macfilter.o cgi_lib_wifiRelay.o\
            cgi_lib_parent.o cgi_lib_sysmanage.o cgi_lib_system_status.o
# cgi files of system status, added by zhuhuan on 2016.03.15
ALL_OBJS += cgi_system_status.o 

# cgi files of common, added by zhuhuan on 2016.07.21
ALL_OBJS += cgi_common.o

# cgi files of traffic control, added by zhuhuan on 2016.07.22
ifeq ($(CONFIG_TC), y)
ALL_OBJS += cgi_tc.o
endif
ifeq ($(CONFIG_LED), y)
ALL_OBJS += cgi_power_save.o cgi_lib_power_save.o
endif


ifeq ($(CONFIG_MOBILE_WEB),y)
ALL_OBJS += cgi_mobile.o
endif

#ifeq ($(CONFIG_WEBS_PAGE_ROM),y)
EXTRA_CFLAGS += -DWEBS_PAGE_ROM
ALL_OBJS += webrom.o
#endif


ifeq ($(CONFIG_SUPPORT_GB2312),y)
	ALL_OBJS += cgi_encode.o
endif

ifeq ($(CONFIG_WPS_RTK),y)
ifneq ($(CONFIG_A9),y)
	ALL_OBJS += cgi_wps_dispatch.o
endif
endif
		
ifeq ($(CONFIG_A5),y)
	EXTRA_CFLAGS += -DA5
endif

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
	rm -f webrom.c webrom.d webrom.o webcomp
	mv *.o *.d obj/
	mv obj/$(MODULE) ./

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif

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
	cc -o src/webcomp -DWEBS -DUEMF -DOS="Linux" -I./include -DLINUX -D_STRUCT_TIMEVAL $(WEB_COMP_EXTRA_CFLAGS) src/webcomp.c

#
# Build a set of ROMable pages
#


webrom.c: webcomp

	rm web/$(CONFIG_WEB_DIR)_src_mini -rf
	./compress_web.sh web/$(CONFIG_WEB_DIR)_src ./compress_tools
	find web/$(CONFIG_WEB_DIR)_src_mini -type f ! -path "*.svn*" ! -path "*goform*" >web_files

	./src/webcomp web/$(CONFIG_WEB_DIR)_src_mini web_files >webrom.c

clean:
	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
	-rm -f $(MODULE)
	
.PHONY: clean
