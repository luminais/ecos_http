MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../../.. ; pwd)
endif

CURR_DIR=$(shell pwd)
EXTRA_CFLAGS = -DHAVE_CONFIG_H -I$(CURR_DIR)/include
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(TOPDIR)/prod/http/include/

ALL_OBJS = ddns.o provider.o oray.o ddns_ecos.o

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk
