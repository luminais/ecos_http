MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../../.. ; pwd)
endif

CURR_DIR=$(shell pwd)
EXTRA_CFLAGS = -DHAVE_CONFIG_H -I$(CURR_DIR)/include
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(TOPDIR)/prod/http/include/
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(TOPDIR)/prod/lib/cJSON/
EXTRA_CFLAGS += -I$(CURR_DIR)/../aes_cbc
EXTRA_CFLAGS += -I$(CURR_DIR)/../timer
EXTRA_CFLAGS += -I$(CURR_DIR)/../elink_client
EXTRA_CFLAGS += -I$(CURR_DIR)/../common
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/cbb/include
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/prod/http/cgi/include
#opssl
EXTRA_CFLAGS += -I$(DIR_ECOS)/AP/openssl-0.9.8o/include
#cJSON.o
ALL_OBJS =  uuid.o uloop.o common.o elinksdk.o
#   usock.o ulog.o
  
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk
