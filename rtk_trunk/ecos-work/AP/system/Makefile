include $(ECOS_AP_DIR)/config.mk

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_WPS),y)
OPENSSLDIR	= ../wsc/portingssl
UPNPDIR		= ../mini_upnp
INCS = -I$(OPENSSLDIR)/crypto/include  -I$(UPNPDIR)
EXTRA_CFLAGS += -D__linux__ -DRSDK_BUILT -DB_ENDIAN $(INCS)
endif
EXTRA_CFLAGS += -I../network -I../apmib
EXTRA_CFLAGS += -I../common
ifeq ($(CONFIG_IPV6_ENABLE),y)
EXTRA_CFLAGS += -DCONFIG_IPV6
endif
ifeq ($(CONFIG_GPIO_APIV2),y)
EXTRA_CFLAGS += -DCONFIG_GPIO_APIV2
ifneq ($(CONFIG_SYS_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_SYS_LED_GPIO_NR=$(CONFIG_SYS_LED_GPIO_NR)
endif
ifneq ($(CONFIG_WPS_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_WPS_LED_GPIO_NR=$(CONFIG_WPS_LED_GPIO_NR)
endif
ifneq ($(CONFIG_24G_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_24G_LED_GPIO_NR=$(CONFIG_24G_LED_GPIO_NR)
endif
ifneq ($(CONFIG_5G_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_5G_LED_GPIO_NR=$(CONFIG_5G_LED_GPIO_NR)
endif
ifneq ($(CONFIG_LAN_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_LAN_LED_GPIO_NR=$(CONFIG_LAN_LED_GPIO_NR)
endif
ifneq ($(CONFIG_WAN_LED_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_WAN_LED_GPIO_NR=$(CONFIG_WAN_LED_GPIO_NR)
endif
ifneq ($(CONFIG_RESET_BTN_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_RESET_BTN_GPIO_NR=$(CONFIG_RESET_BTN_GPIO_NR)
endif
ifneq ($(CONFIG_WPS_BTN_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_WPS_BTN_GPIO_NR=$(CONFIG_WPS_BTN_GPIO_NR)
endif
ifneq ($(CONFIG_WIFI_BTN_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_WIFI_BTN_GPIO_NR=$(CONFIG_WIFI_BTN_GPIO_NR)
endif
ifneq ($(CONFIG_WPS_BTN_GPIO_NR),n)
EXTRA_CFLAGS += -DCONFIG_WPS_BTN_GPIO_NR=$(CONFIG_WPS_BTN_GPIO_NR)
endif
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
ifeq ($(CONFIG_CUTE_MAHJONG_RTK_UI),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_RTK_UI
endif
endif
ifeq ($(CONFIG_RTL_XDG),y)
EXTRA_CFLAGS += -DCONFIG_RTL_XDG
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
obj-y = sys_utility.o sys_conf.o hw_settings.o sys_init.o version.o set_bridge.o
ifeq ($(CONFIG_DAYLIGHT_SAVING_TIME), y)
obj-y += set_dst.o
endif
ifeq ($(CONFIG_VLAN),y)
obj-y += set_vlan.o
endif
ifeq ($(CONFIG_HOME_GATEWAY),y)     
obj-y +=  set_wan.o set_fw.o
endif
obj-y += sys_ramfs.o
ifneq ($(CONFIG_NOGPIO),y)
# no wps button/led, reset button and system led on 96cs
ifneq ($(CONFIG_RTL_8196CS),y)
obj-y += rtl_gpio.o
endif
endif
ifeq ($(CONFIG_CERT_MNG), y)
obj-y += cert_mng.o
ifeq ($(CONFIG_CERT_COMPRESS),y)
CFLAGS += -I$(ECOS_INSTALL_DIR)/include/
CXXFLAGS += -I$(ECOS_INSTALL_DIR)/include/
endif
endif
obj-$(CONFIG_IPV6_ENABLE) += set_ipv6.o
obj-$(CONFIG_RTK_PWM) += pwm-rtk.o

cxxobj-y = 
cxxobj-y += traps.o

cxxobj-$(CONFIG_SOFT_WATCHDOG) += softdog.o
cxxobj-y += sysrq.o

include $(ECOS_AP_DIR)/rules.mk
