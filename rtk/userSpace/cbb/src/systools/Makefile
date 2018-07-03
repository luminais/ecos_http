#
# nvram Makefile
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


ALL_OBJS = build_log.o


SVN_VERSION := $(shell svn info|sed -n '/^Revision/{p}'|awk '{print $$2}')
EXTRA_CFLAGS += -DSVN_VERSION="\"$(SVN_VERSION)\""


$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk



