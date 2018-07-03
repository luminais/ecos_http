include ../.config

NAME = appimg

ifeq ($(CONFIG_IRES_WEB_SUPPORT),y)
.PHONY: geniresweb
prepare = geniresweb
else
prepare =
endif
# sanity check
ifndef MODEL_NAME
$(error MODEL_NAME is not defined)
endif

ifndef ECOS_TOPDIR
export ECOS_TOPDIR=$(shell pwd)/../..
endif
ifndef ECOS_WORK_DIR
export ECOS_WORK_DIR=$(shell pwd)/..
endif

ifndef ECOS_REPOSITORY
ECOS_REPOSITORY:=$(ECOS_TOPDIR)/ecos-3.0/packages
export ECOS_REPOSITORY
endif

ifeq ($(CONFIG_KLD),y)
htmldir = kld/html
httpdir = kld/athttpd
else
ifeq ($(CONFIG_CUTE_MAHJONG),y)
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
htmldir = html
else
htmldir = html_cmj
endif
else
htmldir = html
endif
#httpdir = athttpd
endif

ifndef APP_DIR
export APP_DIR = $(shell pwd)
endif

ifeq ($(CONFIG_RTL_8197F),y)
PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/msdk-elf/bin:$(PATH)
else
PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/mips-elf/bin:$(PATH)
endif
export PATH

#------------------------------------------------------------------------------
export ECOS_INSTALL_DIR=$(ECOS_WORK_DIR)/$(MODEL_NAME)_install
export ECOS_AP_DIR=$(ECOS_WORK_DIR)/AP

-include $(ECOS_INSTALL_DIR)/include/pkgconf/ecos.mak

#CROSS_COMPILE = mips-elf-
CROSS_COMPILE = $(ECOS_COMMAND_PREFIX)

# bintuils
export CC = $(CROSS_COMPILE)gcc
export CXX = $(CROSS_COMPILE)gcc
export AR = $(CROSS_COMPILE)ar
export LD = $(CROSS_COMPILE)ld
export CPP = $(CROSS_COMPILE)gcc -E
# Separate C++ flags out from C flags.
ACTUAL_CFLAGS = $(ECOS_GLOBAL_CFLAGS)
ACTUAL_CFLAGS := $(subst -fno-rtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -frtti,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -Woverloaded-virtual,,$(ACTUAL_CFLAGS))
ACTUAL_CFLAGS := $(subst -fvtable-gc,,$(ACTUAL_CFLAGS))

ACTUAL_CXXFLAGS = $(subst -Wstrict-prototypes,,$(ECOS_GLOBAL_CFLAGS))

CFLAGS  = $(ACTUAL_CFLAGS) -D__ECOS
CXXFLAGS = $(ACTUAL_CXXFLAGS) -D__ECOS
LDFLAGS = $(ECOS_GLOBAL_LDFLAGS)

CFLAGS += -I$(ECOS_INSTALL_DIR)/include -include pkgconf/system.h
CXXFLAGS += -I$(ECOS_INSTALL_DIR)/include -include pkgconf/system.h
LDFLAGS += -L$(ECOS_INSTALL_DIR)/lib -L$(ECOS_AP_DIR) -Ttarget.ld
BUILD_TIME = echo `date -R`
#------------------------------------------------------------------------------
# COMMON_CFLAGS
#------------------------------------------------------------------------------
DYNAMIC_WAN_IP = 1
GET_SERVER_IP_BY_DOMAIN = 1
COMMON_CFLAGS =
#无线驱动还没有一直先关闭
#COMMON_CFLAGS += -DSYS_INIT_USING_MBOX
ifeq ($(CONFIG_RLX_PROFILING),y)
COMMON_CFLAGS += -DHAVE_RLX_PROFILING
endif
ifeq ($(CONFIG_KLD),y)
COMMON_CFLAGS += -DKLD_ENABLED
endif
ifeq ($(CONFIG_WATCHDOG),y)
COMMON_CFLAGS += -DHAVE_WATCHDOG
endif
ifeq ($(CONFIG_SOFT_WATCHDOG),y)
COMMON_CFLAGS += -DHAVE_SOFT_WATCHDOG
endif

ifeq ($(CONFIG_SYSTEM_REINIT),y)
COMMON_CFLAGS += -DHAVE_SYSTEM_REINIT
endif

ifeq ($(CONFIG_IRES_WEB_SUPPORT),y)
COMMON_CFLAGS += -DHAVE_IRES_WEB
COMMON_CFLAGS += -DCONFIG_IRES_WEB_SUPPORT
endif

ifeq ($(CONFIG_SAME_LAN_MAC),y)
COMMON_CFLAGS += -DCONFIG_SAME_LAN_MAC
endif

ifeq ($(CONFIG_IRES_WEB_ADVANCED_SUPPORT),y)
COMMON_CFLAGS += -DHAVE_IRES_WEB
COMMON_CFLAGS += -DCONFIG_IRES_WEB_ADVANCED_SUPPORT
endif

ifeq ($(DYNAMIC_WAN_IP), 1)
COMMON_CFLAGS += -DCONFIG_DYNAMIC_WAN_IP
ifeq ($(GET_SERVER_IP_BY_DOMAIN), 1)
COMMON_CFLAGS += -DCONFIG_GET_SERVER_IP_BY_DOMAIN
endif
endif

ifeq ($(CONFIG_RTL_8367R),y)
COMMON_CFLAGS += -DCONFIG_RTL_8367R
endif

ifeq ($(CONFIG_WLAN_HAL_8822BE),y)
COMMON_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
COMMON_CFLAGS += -DCONFIG_RTL_11AC_SUPPORT
COMMON_CFLAGS += -DCONFIG_RTL_92D_SUPPORT
endif
COMMON_CFLAGS += -DCONFIG_WLANIDX_MUTEX

ifeq ($(CONFIG_APMIB),y)
#COMMON_CFLAGS += -DHAVE_APMIB -DHOME_GATEWAY -DQOS_BY_BANDWIDTH -DMBSSID -DMIB_TLV -DWIFI_SIMPLE_CONFIG
COMMON_CFLAGS += -DHAVE_APMIB 
ifeq ($(CONFIG_HOME_GATEWAY),y)
COMMON_CFLAGS += -DHOME_GATEWAY
ifeq ($(CONFIG_DOS),y)
COMMON_CFLAGS += -DDOS_SUPPORT
endif
endif

ifeq ($(CONFIG_RTL_VAP_SUPPORT),y)
COMMON_CFLAGS += -DMBSSID
endif

ifeq ($(CONFIG_APP_SIMPLE_CONFIG), y)
COMMON_CFLAGS += -DHAVE_SIMPLE_CONFIG
ifeq ($(CONFIG_SIMPLE_CONFIG_PBC_SUPPORT), y)
COMMON_CFLAGS += -DSIMPLE_CONFIG_PBC_SUPPORT
endif
endif

ifeq ($(CONFIG_TWINKLE_RSSI),y)
COMMON_CFLAGS += -DHAVE_TWINKLE_RSSI
endif

COMMON_CFLAGS += -DWIFI_SIMPLE_CONFIG
COMMON_CFLAGS += -DUNIVERSAL_REPEATER 


ifeq ($(CONFIG_FIREWALL),y)
COMMON_CFLAGS += -DHAVE_FIREWALL

ifeq ($(CONFIG_QOS),y)
COMMON_CFLAGS += -DQOS_BY_BANDWIDTH
endif
endif
endif

ifeq ($(CONFIG_RTL_8196E),y)
ifeq ($(CONFIG_RTL_ULINKER),y)
COMMON_CFLAGS += -DCONFIG_RTL_ULINKER
endif
endif

ifeq ($(CONFIG_WLAN_HAL_8814AE),y)
COMMON_CFLAGS += -DCONFIG_WLAN_HAL_8814AE
ifeq ($(CONFIG_SLOT_0_8194AE),y)
COMMON_CFLAGS += -DCONFIG_SLOT_0_8194AE
endif
COMMON_CFLAGS += -DCONFIG_RTL_8812_SUPPORT
COMMON_CFLAGS += -DCONFIG_RTL_11AC_SUPPORT

ifeq ($(CONFIG_USE_PCIE_SLOT_0),y)
ifeq ($(CONFIG_USE_PCIE_SLOT_1),y)
COMMON_CFLAGS += -DCONFIG_RTL_8881A_SELECTIVE
COMMON_CFLAGS += -DCONFIG_RTL_92D_SUPPORT
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D
COMMON_CFLAGS += -DFOR_DUAL_BAND
endif
endif
endif


ifeq ($(CONFIG_RTL_8197F),y)
ifeq ($(CONFIG_USE_PCIE_SLOT_0),y)
COMMON_CFLAGS += -DCONFIG_RTL_92D_SUPPORT
#COMMON_CFLAGS += -DFOR_DUAL_BAND
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN_D
endif
endif

ifeq ($(CONFIG_PPPOE),y)
ifneq ($(CONFIG_WPS),y)
CONFIG_PORTINGSSL=y
endif
endif

ifeq ($(CONFIG_RTL_92D_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_RTL_92D_SUPPORT
endif

ifeq ($(CONFIG_RTL_AC2G_256QAM),y)
COMMON_CFLAGS += -DCONFIG_RTL_AC2G_256QAM
endif

ifeq ($(CONFIG_SLOT_1_8812), y)
ifeq ($(CONFIG_SLOT_0_8192EE),y)
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
endif
endif

ifeq ($(CONFIG_SLOT_0_8812), y)
ifeq ($(CONFIG_SLOT_1_8192EE),y)
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
endif
endif

ifeq ($(CONFIG_RTL_8881A), y)
ifeq ($(CONFIG_SLOT_0_8192EE),y)
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
endif
ifeq ($(CONFIG_SLOT_0_88E),y)
COMMON_CFLAGS += -DCONFIG_RTL_DUAL_PCIESLOT_BIWLAN
endif
COMMON_CFLAGS += -DCONFIG_RTL_8881A_SUPPORT
endif
ifeq ($(CONFIG_ECOS_AP),y)
COMMON_CFLAGS += -DCONFIG_ECOS_AP_SUPPORT
endif
ifeq ($(CONFIG_NOETH),y)
COMMON_CFLAGS += -DHAVE_NOETH
endif

ifeq ($(CONFIG_NOWIFI),y)
COMMON_CFLAGS += -DHAVE_NOWIFI
else
ifeq ($(CONFIG_RTL_WDS_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_WLAN_WDS_SUPPORT
endif
ifeq ($(CONFIG_RTL_MESH_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_RTK_MESH
COMMON_CFLAGS += -D_MESH_ACL_ENABLE_
COMMON_CFLAGS += -DCONFIG_NEW_MESH_UI
endif
ifeq ($(CONFIG_RTL_CLIENT_MODE_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_WLAN_CLIENT_MODE
ifeq ($(CONFIG_RTL_REPEATER_MODE_SUPPORT),y)
COMMON_CFLAGS += -DUNIVERSAL_REPEATER
endif
ifeq ($(CONFIG_RTL_MULTI_REPEATER_MODE_SUPPORT),y)
COMMON_CFLAGS += -DRTL_MULTI_REPEATER_MODE_SUPPORT
endif
endif

ifeq ($(CONFIG_RTL_MULTI_CLONE_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_RTL_MULTI_CLONE_SUPPORT
endif

ifeq ($(CONFIG_RTL_8881A_SELECTIVE_MODE_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_RTL_8881A_SELECTIVE
endif
endif
ifeq ($(CONFIG_JFFS2_SUPPORT),y)
COMMON_CFLAGS += -DJFFS2_SUPPORT
endif

ifeq ($(CONFIG_IRES_WEB_ADVANCED_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_IRES_WEB_ADVANCED_SUPPORT
endif

COMMON_CFLAGS += -DBUILD_TIME=\""$(shell $(BUILD_TIME))"\"

ifeq ($(CONFIG_BRIDGE),y)
COMMON_CFLAGS += -DHAVE_BRIDGE
endif

ifeq ($(CONFIG_ETH1_BRIDGED),y)
COMMON_CFLAGS += -DHAVE_ETH1_BRIDGED
endif

ifeq ($(CONFIG_WPS),y)
#COMMON_CFLAGS += -DHAVE_WPS -DCONFIG_REPEATER_WPS_SUPPORT
COMMON_CFLAGS += -DHAVE_WPS 
endif

ifeq ($(CONFIG_WLAN_SCHEDULE),y)
COMMON_CFLAGS += -DHAVE_WLAN_SCHEDULE
endif

ifeq ($(CONFIG_NTP),y)
COMMON_CFLAGS += -DHAVE_NTP
endif

ifeq ($(CONFIG_RTLDD),y)
COMMON_CFLAGS += -DHAVE_RTLDD
endif

ifeq ($(CONFIG_HTTPD),y)
COMMON_CFLAGS += -DHAVE_HTTPD
ifeq ($(CONFIG_HTTP_FILE_SERVER_SUPPORT),y)
COMMON_CFLAGS += -DHTTP_FILE_SERVER_SUPPORTED
endif
endif

ifeq ($(CONFIG_DHCPD),y)
COMMON_CFLAGS += -DHAVE_DHCPD
endif

ifeq ($(CONFIG_ULINK_DHCP_AUTO),y)
COMMON_CFLAGS += -DULINK_DHCP_AUTO
endif

ifeq ($(CONFIG_BRIDGE_REPEATER_DHCPAUTO),y)
COMMON_CFLAGS += -DBRIDGE_REPEATER
COMMON_CFLAGS += -DDHCP_AUTO_SUPPORT
endif

ifeq ($(CONFIG_DHCPC),y)
COMMON_CFLAGS += -DHAVE_DHCPC
endif

ifeq ($(CONFIG_DNSPROXY),y)
COMMON_CFLAGS += -DHAVE_DNSPROXY
ifeq ($(CONFIG_DOMAIN_NAME_QUERY_SUPPORT),y)
COMMON_CFLAGS += -DECOS_DOMAIN_NAME_QUERY_SUPPORT
endif
endif

ifeq ($(CONFIG_RADVD),y)
COMMON_CFLAGS += -DHAVE_RADVD
endif

ifeq ($(CONFIG_RADVD_SHELL),y)
COMMON_CFLAGS += -DHAVE_RADVD_SHELL
endif

ifeq ($(CONFIG_STATIC_ROUTE),y)
COMMON_CFLAGS += -DSTATIC_ROUTE
endif

ifeq ($(CONFIG_IGMPPROXY_USERMODE),y)
COMMON_CFLAGS += -DHAVE_IGMPPROXY
endif

ifeq ($(CONFIG_PATHSEL),y)
COMMON_CFLAGS += -DHAVE_PATHSEL
endif

ifeq ($(CONFIG_ROUTED),y)
COMMON_CFLAGS += -DROUTE_SUPPORT
EXTRA_OBJS = routed/main.o
else
EXTRA_OBJS = 
endif

ifeq ($(CONFIG_ROUTE6D),y)
COMMON_CFLAGS += -DROUTE6D_SUPPORT
endif

#ifeq ($(CONFIG_WLAN_PROFILE),y)
ifeq ($(CONFIG_RTL_SUPPORT_MULTI_PROFILE), y)
COMMON_CFLAGS += -DWLAN_PROFILE
endif

ifeq ($(CONFIG_NET_STACK_FREEBSD),y)
COMMON_CFLAGS += -DCONFIG_NET_STACK_FREEBSD
endif
ifeq ($(CONFIG_RTL_NFJROM_MP),y)
COMMON_CFLAGS += -DHAVE_MP_DAEMON
endif
ifeq ($(CONFIG_MINIIGD),y)
COMMON_CFLAGS += -DHAVE_MINIIGD
endif

ifeq ($(CONFIG_SSL),y)
COMMON_CFLAGS += -DHAVE_SSL
endif

ifeq ($(CONFIG_IAPP),y)
COMMON_CFLAGS += -DHAVE_IAPP
endif

ifeq ($(CONFIG_IWCONTROL),y)
COMMON_CFLAGS += -DHAVE_IWCONTROL
endif

ifeq ($(CONFIG_AUTH),y)
COMMON_CFLAGS += -DHAVE_AUTH
endif

ifeq ($(CONFIG_RTL_HS2_SUPPORT),y)
COMMON_CFLAGS += -DHAVE_HS2_SUPPORT
endif

ifeq ($(CONFIG_SYSLOG),y)
COMMON_CFLAGS += -DSYSLOG_SUPPORT
endif

ifeq ($(CONFIG_REMOTELOG),y)
COMMON_CFLAGS += -DREMOTELOG_SUPPORT
endif

ifeq ($(CONFIG_NAPT),y)
COMMON_CFLAGS += -DHAVE_NAPT
endif

ifeq ($(CONFIG_NAPT_NO_DYN),y)
COMMON_CFLAGS += -DHAVE_NAPT_NO_DYN
endif
ifeq ($(CONFIG_NATD),y)
COMMON_CFLAGS += -DHAVE_NATD
endif

ifeq ($(CONFIG_ALG),y)
COMMON_CFLAGS += -DHAVE_ALG
endif

ifeq ($(CONFIG_FASTPATH),y)
COMMON_CFLAGS += -DHAVE_FASTPATH
endif

ifeq ($(CONFIG_HW_NAT),y)
COMMON_CFLAGS += -DCONFIG_RTL_HARDWARE_NAT
COMMON_CFLAGS += -DCONFIG_RTL_LAYERED_DRIVER_L3
endif

ifeq ($(CONFIG_VLAN),y)
COMMON_CFLAGS += -DHAVE_VLAN
COMMON_CFLAGS += -DCONFIG_RTL_VLAN_SUPPORT
endif

ifeq ($(CONFIG_BRIDGE_VLAN),y)
COMMON_CFLAGS += -DHAVE_BRIDGE_VLAN
COMMON_CFLAGS += -DCONFIG_RTL_BRIDGE_VLAN_SUPPORT
endif

ifeq ($(CONFIG_DOUBLE_NAPT),y)
COMMON_CFLAGS += -DCONFIG_RTL_SUPPORT_DOUBLE_ALIAS
endif

ifeq ($(CONFIG_MULTI_LANGUAGE),y)
COMMON_CFLAGS += -DHAVE_MULTI_LANGUAGE
endif

ifeq ($(CONFIG_PING6),y)
COMMON_CFLAGS += -DCONFIG_PING6
endif

ifeq ($(CONFIG_MLD_PROXY), y)
COMMON_CFLAGS += -DCONFIG_MLD_PROXY
endif

ifeq ($(CONFIG_IPV6_ROUTE6),y)
COMMON_CFLAGS += -DCONFIG_IPV6_ROUTE6
endif

ifeq ($(CONFIG_IPV6_NDP),y)
COMMON_CFLAGS += -DCONFIG_IPV6_NDP
endif

ifeq ($(CONFIG_DBG_STAT),y)
COMMON_CFLAGS += -DECOS_DBG_STAT
endif

ifeq ($(CONFIG_TENDA_ATE_REALTEK),y)
CONFIG_TELNETD=y
COMMON_CFLAGS += -DCONFIG_TENDA_ATE_REALTEK
endif

ifeq ($(CONFIG_TELNETD),y)
COMMON_CFLAGS += -DHAVE_TELNETD
endif

ifeq ($(CONFIG_TELNET_USERNAME_PASSWORD),y)
COMMON_CFLAGS += -DHAVE_TELNET_USERNAME_PASSWORD
endif

ifeq ($(CONFIG_TR069),y)
COMMON_CFLAGS += -DHAVE_TR069
ifneq ($(CONFIG_CWMP_WITH_NO_SSL),y)
# this macro is for SSL certificates in web page
COMMON_CFLAGS += -DECOS_CWMP_WITH_SSL
endif
ifeq ($(CONFIG_CWMP_WITH_MATRIXSSL),y)
COMMON_CFLAGS += -D_CWMP_WITH_MATRIXSSL
endif
ifeq ($(CONFIG_CWMP_WITH_OPENSSL),y)
COMMON_CFLAGS += -D_CWMP_WITH_OPENSSL
endif
ifeq ($(CONFIG_USER_CWMP_WITH_TR143), y)
COMMON_CFLAGS += -D_PRMT_TR143_
endif
endif

ifeq ($(CONFIG_IPV6FIREWALL),y)
COMMON_CFLAGS += -DHAVE_IPV6FIREWALL
endif

ifeq ($(CONFIG_DHCP6S),y)
COMMON_CFLAGS += -DHAVE_DHCP6S
endif

ifeq ($(CONFIG_NBSERVER),y)
COMMON_CFLAGS += -DHAVE_NBSERVER
endif

ifeq ($(CONFIG_CUSTOMIZE_MIDDLE_EAST), y)
COMMON_CFLAGS += -DCUSTOMIZE_MIDDLE_EAST
COMMON_CFLAGS += -DCONFIG_RTL_DHCP_PPPOE
endif

ifeq ($(CONFIG_MCASE_TOOLS),y)
COMMON_CFLAGS += -DCONFIG_MCASE_TOOLS
endif

ifeq ($(CONFIG_WEB_COMP_TWICE),y)
COMMON_CFLAGS += -DCONFIG_WEB_COMP_TWICE
endif

ifeq ($(CONFIG_RESERVE_WEB_CACHE),y)
COMMON_CFLAGS += -DCONFIG_RESERVE_WEB_CACHE
endif

ifeq ($(CONFIG_HAVE_LZMA),y)
COMMON_CFLAGS += -DHAVE_LZMA
endif

ifeq ($(CONFIG_NETSNIPER),y)
COMMON_CFLAGS += -DCONFIG_RTL_NETSNIPER_SUPPORT
endif

ifeq ($(CONFIG_NETSNIPER_WANTYPE),y)
COMMON_CFLAGS += -DCONFIG_RTL_NETSNIPER_WANTYPE_SUPPORT
endif

ifeq ($(CONFIG_SPI_FIREWALL),y)
COMMON_CFLAGS += -DCONFIG_RTL_SPI_FIREWALL_SUPPORT
endif

ifeq ($(CONFIG_RTL_PHY_POWER_CTRL),y)
COMMON_CFLAGS += -DCONFIG_RTL_PHY_POWER_CTRL
endif

ifneq ($(CONFIG_HW_SETTING_OFFSET),)
COMMON_CFLAGS += -DECOS_HW_SETTING_OFFSET=$(CONFIG_HW_SETTING_OFFSET)
endif
 
ifneq ($(CONFIG_DEFAULT_SETTING_OFFSET),)
COMMON_CFLAGS += -DECOS_DEFAULT_SETTING_OFFSET=$(CONFIG_DEFAULT_SETTING_OFFSET)
endif
 
ifneq ($(CONFIG_CURRENT_SETTING_OFFSET),)
COMMON_CFLAGS += -DECOS_CURRENT_SETTING_OFFSET=$(CONFIG_CURRENT_SETTING_OFFSET)
endif
 
ifneq ($(CONFIG_WEB_PAGES_OFFSET),)
COMMON_CFLAGS += -DECOS_WEB_PAGES_OFFSET=$(CONFIG_WEB_PAGES_OFFSET)
endif
 
ifneq ($(CONFIG_ECOS_IMAGE_OFFSET),)
COMMON_CFLAGS += -DECOS_IMAGE_OFFSET=$(CONFIG_ECOS_IMAGE_OFFSET)
endif
 
ifneq ($(CONFIG_FLASH_SIZE),)
COMMON_CFLAGS += -DECOS_FlASH_SIZE=$(CONFIG_FLASH_SIZE)
endif

ifneq ($(CONFIG_MEM_CHAIN_API),)
COMMON_CFLAGS += -DECOS_MEM_CHAIN_API

ifneq ($(CONFIG_MEM_CHAIN_UPGRADE),)
COMMON_CFLAGS += -DECOS_MEM_CHAIN_UPGRADE
endif
endif

ifneq ($(CONFIG_CERT_MNG),)
COMMON_CFLAGS += -DHAVE_CERT_MNG
COMMON_CFLAGS += -DCERT_MNG_SIZE=$(CONFIG_CERT_MNG_SIZE)
ifeq ($(CONFIG_CERT_COMPRESS),y)
COMMON_CFLAGS += -DHAVE_CERT_COMPRESS
endif
endif

ifneq ($(CONFIG_RTL_REPORT_LINK_STATUS),)
COMMON_CFLAGS += -DECOS_RTL_REPORT_LINK_STATUS
endif

COMMON_CFLAGS += -DLIMITS=64

ifeq ($(CONFIG_SUPPORT_FOR_RUSSIA_CUSTOMER), y)
COMMON_CFLAGS += -DSUPPORT_FOR_RUSSIA_CUSTOMER
endif

ifeq ($(CONFIG_DAYLIGHT_SAVING_TIME), y)
COMMON_CFLAGS += -DDAYLIGHT_SAVING_TIME
endif

ifeq ($(CONFIG_RTL_DNSMASQ),y)
COMMON_CFLAGS += -DHAVE_RTL_DNSMASQ
endif

ifeq ($(CONFIG_RTL_DFS_SUPPORT),y)
COMMON_CFLAGS += -DCONFIG_RTL_DFS_SUPPORT
endif

ifeq ($(CONFIG_RADVD),y)
#ifeq ($(CONFIG_IPV6_CE_ROUTER_SUPPORT),y)
COMMON_CFLAGS += -DSUPPORT_RDNSS_OPTION -DSUPPORT_DNSSL_OPTION
#endif
endif

ifeq ($(CONFIG_IPV6_ENABLE),y)
COMMON_CFLAGS += -DHAVE_IPV6
COMMON_CFLAGS += -DSUPPORT_DEFEAT_IP_SPOOL_DOS
endif


COMMON_CFLAGS += -DHAVE_TENDA_NVRAM


#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(COMMON_CFLAGS)
CXXFLAGS += $(COMMON_CFLAGS)

export CFLAGS CXXFLAGS LDFLAGS COMMON_CFLAGS CONFIG_ECOS_IMAGE_OFFSET

#------------------------------------------------------------------------------
# APP_LIB
#------------------------------------------------------------------------------
export APP_LIB = $(shell pwd)/libapp.a

#------------------------------------------------------------------------------
# obj-y
#------------------------------------------------------------------------------
obj-y =  
obj-y += network
obj-y += common 
obj-y += tools
obj-$(CONFIG_RTL_NFJROM_MP) += mp-daemon
obj-$(CONFIG_SSL) += openssl-0.9.8o
obj-$(CONFIG_HTTPD) += $(htmldir)
obj-$(CONFIG_WLAN_SCHEDULE) += wlschedule
obj-$(CONFIG_MINIIGD) += miniigd
obj-$(CONFIG_WPS) += mini_upnp
obj-$(CONFIG_WPS) += wsc
ifneq ($(CONFIG_SSL),y)
obj-$(CONFIG_PORTINGSSL) += wsc/portingssl
endif
#obj-$(CONFIG_HTTPD) += goahead-2.1.1
obj-$(CONFIG_IAPP) += IAPP
obj-$(CONFIG_DHCPD) += dhcpd
obj-$(CONFIG_DHCPC) += dhcpc
obj-$(CONFIG_ULINK_DHCP_AUTO) += ulink-dhcp-auto
obj-$(CONFIG_BRIDGE_REPEATER_DHCPAUTO) += dhcp-auto 
obj-$(CONFIG_NATD) += natd
obj-$(CONFIG_FIREWALL) += ipfw
obj-$(CONFIG_IPV6FIREWALL) += ip6fw
obj-$(CONFIG_SNTP) += sntp
obj-$(CONFIG_RTLDD) += rtldd
obj-$(CONFIG_DNSPROXY) += dnsproxy
obj-$(CONFIG_PATHSEL) += 11s-PathSelection
obj-$(CONFIG_ROUTED) += routed
obj-$(CONFIG_ROUTE6D) += route6d
obj-$(CONFIG_TELNETD) += telnetd
obj-$(CONFIG_DHCP6S) += wide-dhcpv6
obj-$(CONFIG_TR069) += tr069
obj-$(CONFIG_NBSERVER) += nbserver
obj-$(CONFIG_IWCONTROL) += auth/src/dlisten
obj-$(CONFIG_AUTH) += auth
obj-$(CONFIG_RTL_HS2_SUPPORT) += hs2
obj-$(CONFIG_RTL_DNSMASQ) += dnsmasq-2.66/src
obj-$(CONFIG_NTP) += ntpclient
obj-$(CONFIG_RADVD) += radvd-0.9.1
obj-$(CONFIG_APP_SIMPLE_CONFIG) += simple_config
obj-$(CONFIG_TWINKLE_RSSI) += twinkle-rssi
#------------------------------------------------------------------------------
obj-clean := $(foreach obj,$(obj-y) $(obj-n),$(obj)-clean)
obj-install := $(foreach obj,$(obj-y),$(obj)-install)

#------------------------------------------------------------------------------
#$(warning $(obj-y))

#------------------------------------------------------------------------------
# LIBS
#------------------------------------------------------------------------------
lib-y = 
lib-y += apmib
#lib-y += libc

#------------------------------------------------------------------------------
# APPS
#------------------------------------------------------------------------------
app-y = 
app-y += shell
app-y += system
#app-y += utils					//changed by yp

obj-y += mini_upnp
app-$(CONFIG_HTTPD) += $(httpdir)
app-$(CONFIG_MCASE_TOOLS) += mcast-tools
#app-$(CONFIG_RADVD) += radvd-0.9.1
app-$(CONFIG_IGMPPROXY_USERMODE) += mrouted
#app-y += tenda_app_platform
app-y += $(USER_DIR)
app-$(CONFIG_TENDA_APP) += $(TOP_DIR)/tenda_app

#------------------------------------------------------------------------------
# Default Rules
#------------------------------------------------------------------------------
LIB_OBJS = $(lib-y:%=%/built-in.o)
APP_OBJS = $(app-y:%=%/built-in.o)
lib-clean := $(foreach lib,$(lib-y) $(lib-n),$(lib)-clean)
app-clean := $(foreach app,$(app-y) $(app-n),$(app)-clean)

#------------------------------------------------------------------------------
all: $(prepare) $(obj-y) $(lib-y) $(app-y)
	$(CC) -o $(NAME) $(LDFLAGS) $(LIB_OBJS) $(APP_OBJS) $(EXTRA_OBJS)
	#$(CC) -o $(NAME) $(LDFLAGS) $(LIB_OBJS) $(APP_OBJS)
#	$(CC) -o $(NAME) -Wl,-Map linkmap.txt $(LDFLAGS) shell/*.o $(LIB_OBJS) $(APP_OBJS)
	mips-elf-objcopy -O binary $(NAME) $(NAME).bin
	cp $(NAME) rtkload/ecos.elf
	mips-elf-nm --numeric-sort $(NAME) > System.map
#	mips-elf-objdump -S -d  $(NAME) > asm-mix.txt
#	mips-elf-objdump -d  $(NAME) > asm.txt
#	mips-elf-objdump -x $(NAME) > hhh.txt
#	mips-elf-nm --numeric-sort $(NAME) > sym.txt
#	mips-elf-size $(NAME) > sss.txt

clean:
	for i in ${obj-y} $(lib-y) $(app-y); do \
		make -C $$i clean; \
	done
	rm -f $(APP_LIB) $(NAME) $(NAME).bin System.map

$(obj-y):
	[ -d "$@" ] && $(MAKE) -C $@

$(lib-y):
	[ -d "$@" ] && $(MAKE) -C $@

$(app-y): 
	[ -d "$@" ] && $(MAKE) -C $@

rtkload:
	make -C rtkload

rtkload-clean:
	make -C rtkload clean


geniresweb:
	make -C $(htmldir)
#
# Generic rules
#

%:
	[ ! -d $* ] || $(MAKE) -C $*
	
%-clean:
	@dir=$*;	\
	if  echo $* | grep "/" ; then	\
		ENTER_DIR=$${dir%%/*};	\
		MAKE_DIR=$${dir#*/};	\
		cd $$ENTER_DIR && $(MAKE) $${MAKE_DIR}-clean; \
	else	\
		$(MAKE) -C $$dir clean;	\
	fi
	
%-only:
	@dir=$*;	\
	if  echo $* | grep "/" ; then	\
		ENTER_DIR=$${dir%%/*};	\
		MAKE_DIR=$${dir#*/};	\
		cd $$ENTER_DIR && $(MAKE) $${MAKE_DIR}-only; \
	else	\
		$(MAKE) -C $$dir;	\
	fi
	

%-install:
	[ ! -d $* ] || $(MAKE) -C $* install

$(obj-n) $(obj-install) $(obj-clean) $(lib-clean) $(app-clean): dummy

.PHONY: all clean install
.PHONY: $(obj-y) $(lib-y) $(app-y) rtkload rtkload-clean
.PHONY: dummy 

#
# A rule to do nothing
#
dummy:
