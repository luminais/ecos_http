#
# ecos router wps module generic Makefile
#
# Copyright (C) 2010, Broadcom Corporation
# All Rights Reserved.
# 
# This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
# the contents of this file may not be disclosed to third parties, copied
# or duplicated in any form, in whole or in part, without the prior
# written permission of Broadcom Corporation.
#
# $Id: wps_generic.mk,v 1.6 2011-02-11 02:45:03 Exp $
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TOPDIR)/rules.mak

all: $(MODULE)

# Build type
BLDTYPE = release
#BLDTYPE = debug

# UPnP WFA device support, default YES
WPS_UPNP_DEVICE ?= 1
ifeq ("$(CONFIG_LIBUPNP)","")
WPS_UPNP_DEVICE= 0
endif
ifeq ($(WL_MODE),sta)
WPS_UPNP_DEVICE = 0
endif

#
# Set CFLAGS or EXTRA_CFLAGS or INCLUDE
#
CFLAGS += -DWPS_ROUTER
CFLAGS += -DWPS_AP_M2D
CFLAGS += -DWPS_ADDCLIENT_WWTP

EXTRA_CFLAGS += -Wall -Wnested-externs -D_REENTRANT -DBCMDRIVER
ifeq ($(BLDTYPE),debug)
EXTRA_CFLAGS += -D_TUDEBUGTRACE
endif
ifeq ($(WPS_UPNP_DEVICE),1)
EXTRA_CFLAGS += -DWPS_UPNP_DEVICE
endif

INCLUDE = -I$(SRCBASE)/include -I$(SRCBASE)/include/bcmcrypto \
	  -I$(SRCBASE)/router/shared -I$(SRCBASE)/wps/common/include -I$(SRCBASE)/wps/brcm_apps/include \
	  -I$(SRCBASE)/router/libbcm -I$(SRCBASE)/router/eapd
ifeq ($(WPS_UPNP_DEVICE),1)
INCLUDE += -I$(SRCBASE)/router/libupnp/include -I$(SRCBASE)/wps/brcm_apps/upnp/WFADevice \
	   -I$(SRCBASE)/router/libupnp/ecos
endif

EXTRA_CFLAGS += $(INCLUDE)
#EXTRA_CFLAGS += -Wno-unknown-pragmas

LDFLAGS = -r

WPSSRC = $(SRCBASE)/wps
WPSCOMSRC = $(WPSSRC)/common
WPSAPPSSRC = $(WPSSRC)/brcm_apps
OBJDIR = obj

#
# wps library variants
#
LIBWPS_AP = wps_ap
LIBWPS_AP_A = libwps_ap.a
LIBWPS_STA = wps_sta
LIBWPS_STA_A = libwps_sta.a
LIBWPS_APSTA = wps_apsta
LIBWPS_APSTA_A = libwps_apsta.a

#
# Enumerate files to compile
#
VPATH = $(WPSCOMSRC)/ap:$(WPSCOMSRC)/sta:$(WPSCOMSRC)/enrollee:$(WPSCOMSRC)/registrar:$(WPSCOMSRC)/shared: \
	$(WPSAPPSSRC)/apps:$(WPSAPPSSRC)/arch/bcm947xx:$(WPSAPPSSRC)/ecos:$(SRCBASE)/router/libbcm:
ifeq ($(WPS_UPNP_DEVICE),1)
VPATH += $(WPSAPPSSRC)/upnp/WFADevice:
endif

#
# wps common objects
#
WPSM_SRCS = tutrace.c dev_config.c slist.c enr_reg_sm.c reg_proto_utils.c \
	    reg_proto_msg.c tlv.c state_machine.c buffobj.c wps_utils.c

# wps common ap objects
WPSM_AP_SRCS = ap_api.c ap_eap_sm.c ap_ssr.c reg_sm.c
ifeq ($(WPS_UPNP_DEVICE),1)
WPSM_AP_SRCS += ap_upnp_sm.c
endif

# wps common sta enrollee objects
WPSM_STA_SRCS = enr_api.c sta_eap_sm.c enr_reg_sm.o reg_sm.o 

# wps gpio library
BCMGPIO_SRCS = bcmgpio.c
WPSM_SRCS += $(BCMGPIO_SRCS)

#
# wps monitor objects
#
WPSM_SRCS += wps_monitor.c wps_aplockdown.c wps_pb.c wps_led.c wps_eap.c \
	     wps_ie.c wps_ui.c

# wps monitor ap objects
WPSM_AP_SRCS += wps_ap.o
ifeq ($(WPS_UPNP_DEVICE),1)
WPSM_AP_SRCS += wps_libupnp.c
endif
#ifdef BCMWFI
ifeq ($(CONFIG_WFI),y)
WPSM_AP_SRCS += wps_wfi.c
EXTRA_CFLAGS += -DBCMWFI
endif
#endif
ifeq ($(WPS_UPNP_DEVICE),1)
WPSM_UPNP_SRCS = soap_x_wfawlanconfig.c WFADevice.c WFADevice_table.c \
		 xml_WFADevice.c xml_x_wfawlanconfig.c
endif

# wps monitor sta enrollee objects
WPSM_STA_SRCS += wps_enr_core.c

# wps monitor osl and arch objects
WPSM_OSL_SRCS = wps_ecos_main.c wps_ecos_osl.c
WPSM_STA_OSL_SRCS = wps_enr_ecos_osl.c
WPSM_ARCH_SRCS = wps_gpio.c wps_hal.c wps_wl.c
WPSM_STA_ARCH_SRCS = wps_sta_wl.c

ifeq ($(WL_MODE),sta)
EXTRA_CFLAGS += -DBCMWPSAPSTA
WPSM_LIBS += -l$(LIBWPS_STA)
WPSM_OSL_SRCS += $(WPSM_STA_OSL_SRCS)
WPSM_ARCH_SRCS += $(WPSM_STA_ARCH_SRCS)
LIBWPS_A = $(LIBWPS_STA_A)
endif
ifeq ($(WL_MODE),apsta)
EXTRA_CFLAGS += -DBCMWPSAP -DBCMWPSAPSTA
WPSM_LIBS += -l$(LIBWPS_APSTA)
WPSM_OSL_SRCS += $(WPSM_STA_OSL_SRCS)
WPSM_ARCH_SRCS += $(WPSM_STA_ARCH_SRCS)
LIBWPS_A = $(LIBWPS_APSTA_A)
endif
ifeq ($(WL_MODE),ap)
EXTRA_CFLAGS += -DBCMWPSAP
WPSM_LIBS += -l$(LIBWPS_AP)
LIBWPS_A = $(LIBWPS_AP_A)
endif

WPSM_OBJS := $(WPSM_SRCS:.c=.o)
WPSM_AP_OBJS := $(WPSM_AP_SRCS:.c=.o)
WPSM_STA_OBJS := $(WPSM_STA_SRCS:.c=.o)
WPSM_ARCH_OBJS := $(WPSM_ARCH_SRCS:.c=.o)
WPSM_OSL_OBJS := $(WPSM_OSL_SRCS:.c=.o)
WPSM_UPNP_OBJS := $(WPSM_UPNP_SRCS:.c=.o)

# library rules
$(LIBWPS_AP_A) : $(WPSM_OBJS:.o=.d) $(WPSM_AP_OBJS:.o=.d)
	$(XAR) cr $@ $(WPSM_OBJS) $(WPSM_AP_OBJS)

$(LIBWPS_STA_A) : $(WPSM_OBJS:.o=.d) $(WPSM_STA_OBJS:.o=.d)
	$(XAR) cr $@ $(WPSM_OBJS) $(WPSM_STA_OBJS)

$(LIBWPS_APSTA_A) : $(WPSM_OBJS:.o=.d) $(WPSM_AP_OBJS:.o=.d) $(WPSM_STA_OBJS:.o=.d)
	$(XAR) cr $@ $(WPSM_OBJS) $(WPSM_AP_OBJS) $(WPSM_STA_OBJS)
