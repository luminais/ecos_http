include $(ECOS_AP_DIR)/config.mk

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
WLANSUPP += -DWIFI_SIMPLE_CONFIG 

ifeq ($(CONFIG_RTL_CLIENT_MODE_SUPPORT),y)
WLANSUPP += -DCONFIG_WLAN_CLIENT_MODE
ifeq ($(CONFIG_RTL_REPEATER_MODE_SUPPORT),y)
WLANSUPP += -DUNIVERSAL_REPEATER
endif
endif

# Digest Access switch
DASW	= -DDIGEST_ACCESS_SUPPORT

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS  -D__NO_FCNTL=1 -I.. $(WLANSUPP) $(UMSW) $(DASW) $(SSLSW) $(IFMODSW) -DDOWN_SIZE
#EXTRA_CFLAGS += -DHOME_GATEWAY
#EXTRA_CFLAGS += -I../../system -I../../network
ifeq ($(CONFIG_IPV6_ENABLE),y)
EXTRA_CFLAGS += -DCONFIG_IPV6
endif
ifeq ($(CONFIG_RTL_8881A),y)
EXTRA_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
endif
ifeq ($(CONFIG_RTL_SIMPLE_CONFIG), y)
EXTRA_CFLAGS += -DCONFIG_RTL_SIMPLE_CONFIG
endif
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
# objs
#------------------------------------------------------------------------------
obj-y =
obj-y += apmib.o mibtbl.o 

include $(ECOS_AP_DIR)/rules.mk
