

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(ECOS_AP_DIR)/config.mk
RTL8192=y

#------------------------------------------------------------------------------
# User Management switch
UMSW	= -DUSER_MANAGEMENT_SUPPORT

# TLS client mode support
#TLS= -DTLS_CLIENT

# WLAN support
#-DWLAN_EASY_CONFIG
#WLANSUPP= -DWLAN_WPA -DWLAN_WDS -DWLAN_8185AG -DWLAN_WPA2 $(TLS)
WLANSUPP=
ifeq ($(CONFIG_RTL_WDS_SUPPORT),y)
WLANSUPP += -DCONFIG_WLAN_WDS_SUPPORT
endif

# The following two must be defined. Otherwise, there are some ASP Errors.
WLANSUPP += -DWIFI_SIMPLE_CONFIG -DUNIVERSAL_REPEATER

ifeq ($(CONFIG_RTL_CLIENT_MODE_SUPPORT),y)
WLANSUPP += -DCONFIG_WLAN_CLIENT_MODE
#ifeq ($(CONFIG_RTL_REPEATER_MODE_SUPPORT),y)
#WLANSUPP += -DUNIVERSAL_REPEATER
#endif
endif


# Digest Access switch
DASW	= -DDIGEST_ACCESS_SUPPORT

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS  -D__NO_FCNTL=1 -I.. $(WLANSUPP) $(UMSW) $(DASW) $(SSLSW) $(IFMODSW) -DDOWN_SIZE
#EXTRA_CFLAGS += -DHOME_GATEWAY
EXTRA_CFLAGS += -I./rltk  -I$(ECOS_AP_DIR)/system -I$(TENDA_APP_DIR)/router_shared -I$(TENDA_APP_DIR)/cbb/src/nvram
ifeq ($(CONFIG_IPV6_ENABLE),y)
EXTRA_CFLAGS += -DCONFIG_IPV6
endif
ifeq ($(CONFIG_WLAN_HAL_8822BE),y)
EXTRA_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
endif

ifeq ($(CONFIG_RTL_SIMPLE_CONFIG), y)
EXTRA_CFLAGS += -DCONFIG_RTL_SIMPLE_CONFIG
endif

EXTRA_CFLAGS += -DCONFIG_NET_STACK_FREEBSD

#hong add for wps
ifeq ($(CONFIG_WPS_RTK),y)
EXTRA_CFLAGS += -DHAVE_WPS
endif
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
# objs
#------------------------------------------------------------------------------
VPATH = ./rltk:./bcm

obj-y =
obj-y += wlconf.o
obj-$(RTL8192) += wlconf_rltk.o
obj-y += wl_utility_rltk.o


ALL_OBJS := $(obj-y)

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk
