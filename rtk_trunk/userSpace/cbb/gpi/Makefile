MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

include $(TENDA_APP_DIR)/.config
include $(TENDA_APP_DIR)/tenda_rules.mk

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
#EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include -I$(TENDA_APP_DIR)/rc 
#-Werror


#
# Enumerate files to compile
#
obj-y = gpi_common.o
obj-$(CONFIG_WAN_SURF_CHECK) += gpi_wan_surf_check.o
obj-$(CONFIG_WAN_MODE_CHECK) += gpi_wan_mode_check.o
obj-y += gpi_wifi.o
obj-y += gpi_wan.o
obj-y += gpi_sntp.o
obj-$(CONFIG_DDNS) += gpi_ddns.o
obj-$(CONFIG_APCLIENT_DHCPC) += gpi_apclient_dhcpc.o
obj-$(CONFIG_NAT) += gpi_firewall.o
obj-$(CONFIG_RESTART_CHECK) += gpi_reboot_check.o
obj-$(CONFIG_WIFI_SWITCH_SCHED) += gpi_wifi_switch_sched.o
obj-y += gpi_dhcp_server.o
obj-$(CONFIG_IGD) += gpi_upnp.o
obj-$(CONFIG_TENDA_APP) += gpi_ucloud.o
ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
clean:
	find . -name '*.[od]' -type f -print | xargs rm -f