
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o


include $(TENDA_APP_DIR)/.config

all: $(MODULE)


#
# Set CFLAGS or EXTRA_CFLAGS
#


#
# Enumerate subdirectories
#
obj-y =
ifeq ($(CONFIG_GPIO_APIV2),y)
obj-y = gpio_apiv2
else
obj-y = gpio_api
endif
obj-y += network
obj-y += utils
obj-y += systools
# obj-y += nas eapd
obj-y += wlconf
# ifneq ($(CONFIG_DHCPC)$(CONFIG_DHCPD),)
obj-y += dhcp
# endif
obj-y += ipf
obj-y += bcmsntp
obj-y += cli
obj-y += lib 
# obj-y += shared
# obj-y += bcmcrypto
obj-y += nvram
obj-$(CONFIG_NAT) += wan
obj-$(CONFIG_DDNS) += ddns
# obj-$(CONFIG_LIBZ) += libz
# obj-$(CONFIG_TELNETD) += telnetd 
# obj-$(CONFIG_8021X) += 8021x
obj-$(CONFIG_TC) += tc
# obj-$(CONFIG_BATCH_UPGRADE) += batchupgrade
obj-$(CONFIG_DNSMASQ) += dnsmasq
obj-$(CONFIG_WPS_RTK) += wps
obj-$(CONFIG_AUTO_CONN) += auto_conn

obj-$(CONFIG_PPP) += ppp
# obj-$(CONFIG_IPV6) += ipv6
obj-$(CONFIG_IGD) += igd
obj-$(CONFIG_LIBUPNP) += libupnp
# obj-$(CONFIG_NETBOOT) += netboot
obj-$(CONFIG_MEMCHECK) += mem_check
obj-y += button
obj-y += login_keep

subdirs: $(patsubst %, _dir_%, $(obj-y))
$(patsubst %, _dir_%, $(obj-y)) :
	$(MAKE) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(obj-y), $(patsubst %, /obj-%.o, $(obj-y)))

#
# Node rule to make
#

$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $@ $(SUB_OBJS)

	

clean:
	for i in $(obj-y); do \
		make -C $$i clean; \
	done
	rm -f $(TARGET)

.PHONY: all clean 
.PHONY: $(obj-y)
