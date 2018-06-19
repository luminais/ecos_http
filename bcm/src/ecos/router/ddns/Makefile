# Makefile


MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

CURR_DIR=$(shell pwd)
EXTRA_CFLAGS = -DHAVE_CONFIG_H -I$(CURR_DIR)/include
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(shell pwd;)/../tenda_httpd

ALL_OBJS = ddns.o provider.o oray.o ddns_ecos.o

all: $(MODULE)

include $(TOPDIR)/rules.mak

ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)

clean:
	rm $(MODULE) $(ALL_OBJS)
