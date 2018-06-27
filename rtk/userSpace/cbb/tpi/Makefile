MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(TENDA_APP_DIR)/include -I$(shell cd .. ; pwd)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/system/include -I$(TENDA_APP_DIR)/system/rc/include -I$(TENDA_APP_DIR)/system/msg/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared/ -I$(TENDA_APP_DIR)/net_drive/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/cli/include -I$(TENDA_APP_DIR)/prod/lib/cJSON
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/include
#-Werror


#
# Enumerate files to compile
#
obj-y = tpi_common.o
obj-y += tpi_lan.o
obj-y += tpi_wan.o
obj-y += tpi_debug.o
obj-y += tpi_http.o
obj-y += tpi_dhcp_server.o
obj-$(CONFIG_WAN_SURF_CHECK) += tpi_wan_surf_check.o
obj-$(CONFIG_WAN_MODE_CHECK) += tpi_wan_mode_check.o
obj-y += tpi_wifi.o
obj-$(CONFIG_RESTART_CHECK) += tpi_reboot_check.o
obj-$(CONFIG_WIFI_SWITCH_SCHED) += tpi_wifi_switch_sched.o
obj-y += tpi_sntp.o
obj-$(CONFIG_APCLIENT_DHCPC) += tpi_apclient_dhcpc.o
obj-$(CONFIG_NAT) += tpi_firewall.o
obj-$(CONFIG_DNSMASQ) += tpi_dnsmasq.o
obj-y += tpi_systools.o
obj-$(CONFIG_IGD) += tpi_upnp.o
obj-$(CONFIG_TC) += tpi_tc.o
obj-$(CONFIG_AUTO_CONN_CLIENT) += tpi_auto_conn_client.o
obj-$(CONFIG_AUTO_CONN_SERVER) += tpi_auto_conn_server.o
obj-$(CONFIG_WPS_RTK) += tpi_wps.o
obj-$(CONFIG_DDNS) += tpi_ddns.o
obj-$(CONFIG_TENDA_APP) += tpi_ucloud.o
obj-$(CONFIG_GET_MANUFACTURER) += tpi_manufacturer_config.o
obj-$(CONFIG_GET_MANUFACTURER) += tpi_manufacturer.o
obj-$(CONFIG_LED) += tpi_led.o
obj-$(CONFIG_IPTV) += tpi_igmp.o
obj-$(CONFIG_SUPPORT_SWITCH_LED_BY_GPIO) += tpi_switch_led.o
obj-$(CONFIG_ELINK) += tpi_elink_main.o
obj-$(CONFIG_ELINK) +=tpi_upload_sysinfo.o
ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
clean:
	find . -name '*.[od]' -type f -print | xargs rm -f