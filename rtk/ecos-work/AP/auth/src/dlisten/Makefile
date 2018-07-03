#
# Makefile for WPA for ECOS
#
#
# $Id: Makefile,v 1.1.1.1 2007/08/06 10:04:42 root Exp $
#

WPA_ROOT_DIR = ../..
include $(ECOS_WORK_DIR)/.config
include $(USER_CONFIG)

EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

ifeq ($(CONFIG_APMIB),y)
EXTRA_CFLAGS += -I../network -I../apmib
endif


EXTRA_CFLAGS += -I../common -I../system
EXTRA_CFLAGS += -I../../include

CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

dlisten_SRCS = iwcontrol.c iwreq.c  iwcommon.c iwconfig.c
dlisten_OBJS = ${dlisten_SRCS:.c=.o}


#RTL_WPA_LIBS = ${WPA_ROOT_DIR}/libnet.a \
#				${WPA_ROOT_DIR}/libpcap.a \
#				${WPA_ROOT_DIR}/libcrypto.a

CFLAGS += -DGLIBC22_HEADERS


ifeq ($(CONFIG_WPS_RTK),y)
CFLAGS += -DCONFIG_WPS_RTK
endif

ifeq ($(CONFIG_RTL_92D_DMDP),y)
CFLAGS += -DFOR_DUAL_BAND
endif



#
#	Transition rules (add -o to put object in right directory)
#


###################
all: $(dlisten_OBJS)
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) $(dlisten_OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<


##-------------------------------------------------

clean:
	rm -f *.o

