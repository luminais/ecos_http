#
# Makefile for WPA for the ECOS 
#
#
# $Id: Makefile,v 1.8 2008/10/13 01:10:14 davidhsu Exp $
#

include $(ECOS_WORK_DIR)/.config
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

ifeq ($(CONFIG_APMIB),y)
EXTRA_CFLAGS += -I../network -I../apmib
endif

EXTRA_CFLAGS += -I../common -I../system
EXTRA_CFLAGS += -I../include
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.


PREFIX?=$(notdir $(shell pwd))_

###### WPA2 #######
ENABLE_WPA2 = true
ENABLE_WPA2_PREAUTH = true
ENABLE_WPA2_CLIENT = true
ENABLE_RADIUS_2SET = false
ENABLE_USE_ALARM = 

ifeq ($(strip $(ENABLE_WPA2)),true)
CFLAGS +=  -DRTL_WPA2
endif # ENABLE_WPA2

ifeq ($(strip $(ENABLE_WPA2_PREAUTH)),true)
CFLAGS +=  -DRTL_WPA2_PREAUTH
endif # ENABLE_WPA2_PREAUTH


ifeq ($(strip $(ENABLE_WPA2_CLIENT)),true)
CFLAGS +=  -DRTL_WPA2_CLIENT
endif # ENABLE_WPA2_CLIENT

ifeq ($(strip $(ENABLE_RADIUS_2SET)),true)
CFLAGS +=  -DRTL_RADIUS_2SET
endif #ENABLE_RADIUS_2SET

ifeq ($(strip $(ENABLE_USE_ALARM)),true)
CFLAGS +=  -DUSE_ALARM
endif

ifeq ($(CONFIG_RTL_8197F),y)
CONFIG_CPU_LITTLE_ENDIAN=y
endif

CFLAGS += -D_ON_RTL8181_TARGET -D_DAEMON_SIDE -D_RTL_WPA_UNIX

ifeq ($(CONFIG_CPU_LITTLE_ENDIAN),y)
CFLAGS += -DLIBNET_LIL_ENDIAN -DLIB1X_LIL_ENDIAN -DAUTH_LITTLE_ENDIAN
else
CFLAGS += -DLIBNET_BIG_ENDIAN -DLIB1X_BIG_ENDIAN -DAUTH_BIG_ENDIAN
endif


CFLAGS += -DRTL_1X_SUPPORT_MULTI_INTERFACE
#------------------------------------------------------------------------
# source files
#------------------------------------------------------------------------

RTL_WPA_SRCS = auth_1x.c \
			1x_auth_pae.c 1x_bauth_sm.c 1x_common.c 1x_krc_sm.c \
			1x_nal.c 1x_kxsm.c 1x_radius.c 1x_ptsm.o 1x_reauth_sm.c \
			1x_cdsm.c 1x_config.c 1x_ioctl.c 1x_parser.c \
			1x_kmsm_auth.c 1x_kmsm_supp.c 1x_kmsm_eapolkey.c \
			1x_info_auth.c 1x_kmsm_prf.c 1x_kmsm_aes.c 1x_acct_sm.c 1x_rc4.c libnet.c 1x_auth_md5c.c 1x_kmsm_hmac.c


ifeq ($(strip $(ENABLE_WPA2)),true)
RTL_WPA_SRCS	+= 1x_pmk_cache.c
endif # ENABLE_WPA2
RTL_WPA_OBJS = ${RTL_WPA_SRCS:.c=.o}


PREFIX_WPA_OBJS=$(join $(dir $(RTL_WPA_OBJS)),$(addprefix $(PREFIX),$(notdir $(RTL_WPA_OBJS))))
#------------------------------------------------------------------------
#	Transition rules (add -o to put object in right directory)
#------------------------------------------------------------------------

#ifeq ($(ASUS),1)
#iwcontrol.o: dlisten/iwcontrol.c
#	$(CC) $(RTL_WPA_CFLAG) $(RTL_WPA_IFLAGS) $(RTL_WPA_DEFFLAGS) -c -o $*.o $<
#endif

###################
#all: build_lib build_listen ${RTL_WPA_TARGET} wpa_bin pretest
#all: pretest build_listen ${RTL_WPA_TARGET} wpa_bin

all: $(RTL_WPA_OBJS) $(PREFIX_WPA_OBJS) 
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) $(PREFIX_WPA_OBJS)
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(PREFIX)%.o:%.o
	@cp $^ $@

##-------------------------------------------------

clean:
	rm -f *.o; 


