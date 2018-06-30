
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)



EXTRA_CFLAGS =
EXTRA_CFLAGS += -I. -I$(shell cd .. ; pwd)/common


ALL_OBJS = bcmsntp.o



$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk