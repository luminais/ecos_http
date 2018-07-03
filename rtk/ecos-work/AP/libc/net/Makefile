include $(ECOS_AP_DIR)/config.mk

obj-y =
obj-y += ether_addr.o inet_network.o inet_lnaof.o
obj-$(CONFIG_PING6) += ip6opt.o rthdr.o
obj-$(CONFIG_IPV6_NDP) += gmt2local.o

include $(ECOS_AP_DIR)/rules.mk
