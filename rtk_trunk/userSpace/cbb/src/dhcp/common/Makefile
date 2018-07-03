#
# Leaf Makefile
#
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(shell pwd)
EXTRA_CFLAGS += -I$(TENDA_APP_DIR)/nvram
EXTRA_CFLAGS += -DBCM47XX -D__ECOS -DROUTER_TINY -DCFG_GMAC -DBCMWPA2

#
# Enumerate files to compile
#
ALL_OBJS = dhcp_option.o dhcp_chksum.o

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk

