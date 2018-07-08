MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include -I$(TENDA_APP_DIR)/rc
#-Werror


#
# Enumerate files to compile
#
obj-y = api_debug.o
obj-y += api_lan.o
obj-y += api_wan.o
obj-y += api_wifi.o
obj-y += api_http.o
obj-y += api_dhcp_server.o
obj-$(CONFIG_WAN_SURF_CHECK) += api_wan_surf_check.o
obj-$(CONFIG_WAN_MODE_CHECK) += api_wan_mode_check.o
obj-y += api_sntp.o
obj-$(CONFIG_APCLIENT_DHCPC) += api_apclient_dhcpc.o
obj-$(CONFIG_NAT) += api_firewall.o
obj-$(CONFIG_RESTART_CHECK) += api_reboot_check.o
obj-$(CONFIG_WIFI_SWITCH_SCHED) += api_wifi_switch_sched.o
obj-$(CONFIG_DNSMASQ) += api_dnsmasq.o
obj-y += api_systools.o
obj-$(CONFIG_IGD) += api_upnp.o
obj-$(CONFIG_TC) += api_tc.o
obj-$(CONFIG_AUTO_CONN_CLIENT) += api_auto_conn_client.o
obj-$(CONFIG_AUTO_CONN_SERVER) += api_auto_conn_server.o
obj-$(CONFIG_WPS_RTK) += api_wps.o
obj-$(CONFIG_DDNS) += api_ddns.o
obj-$(CONFIG_TENDA_APP) += api_ucloud.o
obj-$(CONFIG_GET_MANUFACTURER) += api_manufacturer.o
obj-$(CONFIG_LED) += api_led.o
obj-$(CONFIG_IPTV) += api_igmp.o
obj-$(CONFIG_SUPPORT_SWITCH_LED_BY_GPIO) += api_switch_led.o
obj-y += api_login_keep.o
ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
clean:
	find . -name '*.[od]' -type f -print | xargs rm -f
