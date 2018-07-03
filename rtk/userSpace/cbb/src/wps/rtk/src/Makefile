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

all:	BUILT_TIME $(MODULE)
	echo "jamtest1$(CONFIG_RTL_8197F)"
	
OPENSSLDIR	= ../portingssl
UPNPDIR		= $(TENDA_APP_DIR)/../ecos-work/AP/mini_upnp
IXMLINC		= ./
INCS = -I$(OPENSSLDIR)/crypto/include -I$(UPNPDIR) -I$(IXMLINC)

include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config

#EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/cbb/src/tc -I$(TENDA_APP_DIR)/include -I$(TENDA_APP_DIR)/router_shared  -I./include -I. -I$(APP_DIR)/tenda_paltform/shared_common -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -include autoconf.h
EXTRA_CFLAGS += $(INCS)
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/../ecos-work/AP/system -I$(TENDA_APP_DIR)/../ecos-work/AP/apmib -I$(TENDA_APP_DIR)/../ecos-work/AP/common
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/../ecos-3.0/packages/services/gfx/mw/v3_0/include -I$(TENDA_APP_DIR)/../ecos-3.0/packages/io/common/v3_0/include
#EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X
EXTRA_CFLAGS += -D__linux__ -DRSDK_BUILT -DB_ENDIAN
EXTRA_CFLAGS += -DWPS2DOTX
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG
endif
ifeq ($(CONFIG_KLD),y)
EXTRA_CFLAGS += -DCONFIG_RTL865X_KLD
endif
EXTRA_CFLAGS += -DHAVE_WATCHDOG -DHAVE_SOFT_WATCHDOG -DHAVE_SYSTEM_REINIT -DCONFIG_SAME_LAN_MAC -DHAVE_IRES_WEB -DCONFIG_IRES_WEB_ADVANCED_SUPPORT
EXTRA_CFLAGS +=	-DCONFIG_DYNAMIC_WAN_IP -DCONFIG_GET_SERVER_IP_BY_DOMAIN -DHAVE_APMIB  
EXTRA_CFLAGS +=	-DHOME_GATEWAY -DDOS_SUPPORT -DMBSSID -DWIFI_SIMPLE_CONFIG -DUNIVERSAL_REPEATER 
EXTRA_CFLAGS +=	-DHAVE_FIREWALL -DQOS_BY_BANDWIDTH -DCONFIG_WLAN_WDS_SUPPORT -DCONFIG_WLAN_CLIENT_MODE 
EXTRA_CFLAGS +=	-DUNIVERSAL_REPEATER -DCONFIG_IRES_WEB_ADVANCED_SUPPORT
EXTRA_CFLAGS +=	-DBUILD_TIME=\""Tue, 02 Feb 2016 02:22:09 -0500"\" -DHAVE_BRIDGE -DHAVE_WPS  
#EXTRA_CFLAGS +=	-DHAVE_WLAN_SCHEDULE -DHAVE_NTP -DHAVE_RTLDD -DHAVE_HTTPD -DHAVE_DHCPD
EXTRA_CFLAGS +=	-DHAVE_WLAN_SCHEDULE -DHAVE_NTP -DHAVE_RTLDD -DHAVE_DHCPD
EXTRA_CFLAGS +=	-DHAVE_DHCPC -DHAVE_DNSPROXY -DECOS_DOMAIN_NAME_QUERY_SUPPORT -DSTATIC_ROUTE 
EXTRA_CFLAGS +=	-DHAVE_IGMPPROXY -DCONFIG_NET_STACK_FREEBSD -DHAVE_MINIIGD -DHAVE_IAPP
EXTRA_CFLAGS +=	-DHAVE_IWCONTROL -DSYSLOG_SUPPORT -DHAVE_NAPT -DHAVE_NAPT_NO_DYN -DHAVE_ALG 
EXTRA_CFLAGS +=	-DHAVE_FASTPATH -DHAVE_VLAN -DCONFIG_RTL_VLAN_SUPPORT -DHAVE_BRIDGE_VLAN
EXTRA_CFLAGS +=	-DCONFIG_RTL_BRIDGE_VLAN_SUPPORT -DCONFIG_RTL_SUPPORT_DOUBLE_ALIAS 
EXTRA_CFLAGS +=	-DCONFIG_RTL_PHY_POWER_CTRL -DECOS_MEM_CHAIN_API -DECOS_MEM_CHAIN_UPGRADE
EXTRA_CFLAGS +=	-DECOS_RTL_REPORT_LINK_STATUS -DLIMITS=64 -DDAYLIGHT_SAVING_TIME
#BSPLJF++ L_ENDIAN for 8197F
EXTRA_CFLAGS += -DL_ENDIAN

#hong add debug for test
#EXTRA_CFLAGS +=	-DDEBUG

EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/nvram


SRCPATH = ./
VPATH = $(SRCPATH)

obj-y	= wsc.o txpkt.o rxpkt.o utils.o simplecfg_mini_upnp_main.o wscd_conf.o

ALL_OBJS := $(obj-y)

DEPEND = ./.depend	
SOURCES = wsc.c txpkt.c rxpkt.c utils.c simplecfg_mini_upnp_main.c wscd_conf.c	
#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif

#clean:
#	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
#	-rm -f $(MODULE)

BUILT_TIME:
	@echo  \#define BUILT_TIME \"`TZ=UTC date -u "+%Y.%m.%d-%H:%M%z" `\" > ./built_time	
	
#.PHONY: clean
clean:
	echo "jamtest$(CONFIG_RTL_8197F)"
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~ built_time
	rm wscd_conf.c
wscd_conf.c: CONF_FILE
depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi
CONF_FILE:
	./bin2c.pl wscd_conf < wscd.conf > ./wscd_conf.c
-include $(DEPEND)

.PHONY: clean all depend
