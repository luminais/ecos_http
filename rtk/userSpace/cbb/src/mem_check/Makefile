#
# nvram Makefile
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


#
# Set CFLAGS or EXTRA_CFLAGS
#
DEBUG_CFLAGS =
EXTRA_CFLAGS =
EXTRA_CFLAGS += -I. -I$(TENDA_APP_DIR)/router_shared


#
# Enumerate files to compile
#
ALL_OBJS = mem_check.o


#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk



