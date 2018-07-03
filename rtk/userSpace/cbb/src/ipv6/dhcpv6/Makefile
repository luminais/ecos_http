ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd ../../../.. ; pwd)
endif

include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ALL_OBJS = dhcp6s_ecos.o dhcp6s.o common.o config.o prefixconf.o timer.o cfparse.o cftoken.o if.o base64.o auth.o dhcp6_ctl.o addrconf.o lease.o \
         dhcp6c.o dhcp6c_ecos.o dhcp6c_ia.o dhcp6c_script.o
		 
all:$(MODULE)

include $(TOPDIR)/rules.mak

CFLAGS+= -I$(TOPDIR) -I$(TOPDIR)/rc

EXTRA_CFLAGS = -I$(TOPDIR)/include -I./ -DTIME_WITH_SYS_TIME -DHAVE_STDARG_H

$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)

clean:
	$(RM) *.o *.d

.PHONY:	clean
