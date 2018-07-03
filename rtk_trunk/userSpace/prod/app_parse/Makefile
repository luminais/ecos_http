MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)
CURR_DIR=$(shell pwd)
include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config

EXTRA_CFLAGS += -I$(CURR_DIR)/include/ 
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/app/include/uc_inlucde -I$(TENDA_APP_DIR)/prod/app/include/biz_include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/cgi/include/  
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/src/tc/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/net_drive/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/msg/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/include/
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/lib/cJSON
EXTRA_CFLAGS += -I$(TOP_DIR)
EXTRA_CFLAGS += -DWEBS -DUEMF -DOS="eCos" -DECOS -D__NO_FCNTL=1 -D_STRUCT_TIME -DWEBS_PAGE_ROM -D__ECOS -DBCM47XX -DROUTER_TINY -DCFG_GMAC -DBCMWPA2 -D_KERNEL -DCYGIMPL_TRACE_SPLX -DBSD=200108 -DTENDA_KDEV_ADD -DRTL_819X
#UPDATE_RANDOM := $(shell cat /dev/urandom|sed 's/[^a-zA-Z0-9]//g'|strings -n 6|head -n 1)
#SVN_VERSION := $(shell svn info|sed -n '/^Revision/{p}'|awk '{print $$2}')
#EXTRA_CFLAGS += -DSVN_VERSION="\"$(SVN_VERSION)\""
#EXTRA_CFLAGS += -DUPDATE_RANDOM="\"$(UPDATE_RANDOM)_$(SVN_VERSION)\""


SRCPATH = $(CURR_DIR)/src/

VPATH = $(SRCPATH)

VERSION:
	rm -rf cjson_ucloud_info.o
	chmod a+x  $(CURR_DIR)/pwd_producer.sh
	$(CURR_DIR)/pwd_producer.sh $(TOP_DIR)

ALL_OBJS  = cgi_handle.o cjson_energy.o cjson_common.o cjson_nickname.o cjson_wan.o \
			cjson_sys.o cjson_login.o cjson_safe.o cjson_wifi.o \
			cjson_ucloud_info.o cjson_ol_host.o cjson_run_net.o cjson_hand_qos.o cjson_upgrade.o

$(MODULE): VERSION $(ALL_OBJS)	
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)
	
clean:
	-find ./  -type f -name "*.[od]" -print0 | xargs -0 rm -f
	-rm -f $(MODULE)
	
.PHONY: clean
