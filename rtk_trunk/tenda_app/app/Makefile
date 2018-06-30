MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE) clean_od
CURR_DIR=$(shell pwd)
include $(USER_DIR)/tenda_rules.mk
include $(USER_CONFIG)

EXTRA_CFLAGS += -I$(CURR_DIR)/include/biz_include -I$(CURR_DIR)/include/uc_inlucde
EXTRA_CFLAGS += -I$(CURR_DIR)/include/
EXTRA_CFLAGS += -I$(USER_DIR)/include/

EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -DWEBS_PAGE_ROM -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X
ifeq ($(CONFIG_GUEST),y)
EXTRA_CFLAGS += -D__CONFIG_GUEST__
endif
ifeq ($(CONFIG_APP_SIGNAL_STRENGTH_HIDE),y)
EXTRA_CFLAGS += -D__CONFIG_APP_SIGNAL_STRENGTH_HIDE__
endif
SRCPATH = $(CURR_DIR)/src/

VPATH = $(SRCPATH)

ALL_OBJS  = biz_m_energy.o biz_m_ol_host.o biz_m_rub_net.o biz_m_dev_nickname.o \
            biz_m_wan.o biz_m_wifi.o biz_m_hand_qos.o biz_m_login.o \
            biz_m_safe_check.o biz_m_sys.o biz_m_ucloud_info.o \
            biz_m_upgrade.o biz_cjson_module_cmd.o biz_main.o \
			biz_others.o biz_list.o
ifeq ($(CONFIG_APP_COSTDOWN),y)
EXTRA_CFLAGS += -DCONFIG_APP_COSTDOWN
ALL_OBJS += biz_costdown_reg.o
else
ALL_OBJS += biz_register.o
endif

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -EL -o $(MODULE) $(ALL_OBJS)
clean_od:
	-find ./  -type f -name "biz_*.[od]" -print0 | xargs -0 rm -f
    
clean:
	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
	-rm -f $(MODULE)
	
.PHONY: clean clean_od
