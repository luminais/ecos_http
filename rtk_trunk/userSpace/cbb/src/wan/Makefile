#
# Leaf Makefile
#
include $(TENDA_APP_DIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk

ifdef CONFIG_CHINA_NET_CLIENT
EXTRA_CFLAGS += -DCONFIG_CHINA_NET_CLIENT
endif

EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared

obj-y = wan_link.o
obj-y += dns.o
# ldf revise temporary, solve compile error
obj-$(CONFIG_PPP) += ppp.o
obj-$(CONFIG_PPPOE2) += pppoe2.o
obj-$(CONFIG_DHCPC) += bcmdhcpc.o 
obj-$(CONFIG_PPTP) += pptp.o
obj-$(CONFIG_L2TP) += l2tp.o

ALL_OBJS += $(obj-y)

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY: clean
