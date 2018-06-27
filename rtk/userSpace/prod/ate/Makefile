MODULE = obj-$(shell pwd | sed "s/.*\///" ).o
all: $(MODULE)
include $(ECOS_AP_DIR)/rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(TENDA_APP_DIR)/include -I$(TENDA_APP_DIR)/prod/ate/include -I$(TENDA_APP_DIR)/prod/http/include -I$(TENDA_APP_DIR)/cbb/include -I$(TENDA_APP_DIR)/router_shared -I$(TENDA_APP_DIR)/cbb/src/gpio_api
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/system/include -I$(TENDA_APP_DIR)/system/rc/include -I$(TENDA_APP_DIR)/system/msg/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include -I$(TENDA_APP_DIR)/cbb/src/tc -I./include -I. -I$(APP_DIR)/tenda_paltform/shared_common -I$(TENDA_APP_DIR)/cbb/src/wlconf/rltk
EXTRA_CFLAGS += -include autoconf.h
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/cli/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/lib/cJSON/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/button/include/
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -DWEBS_PAGE_ROM -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X
#-Werror


#
# Enumerate files to compile
#
ifeq ($(CONFIG_TENDA_ATE_REALTEK),y)
obj-y = ate_ecos.o ate_cgi.o ate_realtek.o
else
obj-y = ate_ecos.o ate_cgi.o
endif
ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
