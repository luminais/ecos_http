ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../../../.. ; pwd)
endif

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ALL_OBJS =rtadvd.o rrenum.o advcap.o if.o config.o timer.o dump.o 

CFLAGS+= -DROUTEINFO 

all: $(MODULE)
    
include $(TOPDIR)/rules.mak

CFLAGS+= -I$(TOPDIR) -I$(TOPDIR)/rc

$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)


# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)
clean:
	rm $(MODULE) $(ALL_OBJS)

.PHONY:	clean