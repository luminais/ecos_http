MODULE = obj-$(shell pwd | sed "s/.*\///").o


all:$(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config

EXTRA_CFLAGS = -I$(TENDA_APP_DIR)/include -I$(shell pwd)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/system/msg/include -I$(TENDA_APP_DIR)/system/rc/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/cli/include -I$(TENDA_APP_DIR)/cbb/src/gpio_api
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/button/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/router_shared
#llm add
EXTRA_CFLAGS += -include cyg/kernel/kapi.h
# zhuhuan add for auto connect of A9
ifeq ($(CONFIG_AUTO_CONN_CLIENT),y)
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/auto_conn/auto_conn_client/
endif 

VPATH = $(SRCBASE)/router/libbcm/

obj-y = msg rc common

subdirs: $(patsubst %, _dir_%, $(obj-y))
$(patsubst %, _dir_%, $(obj-y)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

subobjs = $(join $(obj-y), $(patsubst %, /obj-%.o, $(obj-y)))

ifeq ($(CONFIG_SUPPORT_SWITCH_LED_BY_GPIO), y) 
ALL_OBJS += sys_lan_wan_led.o sys_ecos.o sys_option.o sys_init_services.o sys_timer.o sys_led.o sys_vlan.o
else
ALL_OBJS += sys_ecos.o sys_option.o sys_init_services.o sys_timer.o sys_led.o sys_vlan.o
endif

ifeq ($(CONFIG_EXTEND_LED),y)
ALL_OBJS += sys_extend_timer.o
endif

ifneq ($(CONFIG_WPS),y)
#ALL_OBJS += bcmgpio.o
endif

$(MODULE): $(ALL_OBJS) subdirs $(subobjs)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS) $(subobjs)

clean:
	find . -name '*[od]' -type f -print | xargs rm -f
.PHONY: clean

