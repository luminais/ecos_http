MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../.. ; pwd)
endif

include $(TOPDIR)/.config
include $(TOPDIR)/rules.mak

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(shell cd .. ; pwd)/include -I$(TOPDIR)/rc
#-Werror


#
# Enumerate files to compile
#
obj-y += api_common.o
obj-$(CONFIG_APCLIENT_DHCPC) += api_apclient_dhcpc.o

ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
clean:
	find . -name '*.[od]' -type f -print | xargs rm -f