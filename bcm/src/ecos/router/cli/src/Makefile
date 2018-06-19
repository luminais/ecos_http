#
# Leaf Makefile
#
include $(TOPDIR)/.config

MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
cflags-y = -I$(shell cd .. ; pwd)/include -I$(TOPDIR)/rc
#cflags-y += -Werror

cflags-$(CONFIG_DHCPC) += -I$(TOPDIR)/dhcpc/include
cflags-$(CONFIG_DHCPD) += -I$(TOPDIR)/dhcpd/include
cflags-$(CONFIG_PPPOE) += -I$(TOPDIR)/pppoe/include
cflags-$(CONFIG_DNSMASQ) += -I$(TOPDIR)/dnsmasq/include
cflags-$(CONFIG_HTTPD) += -I$(TOPDIR)/httpd/include

EXTRA_CFLAGS = $(cflags-y)

#
# Enumerate files to compile
#

#obj-y = cli_main.o cli_io.o cli_basic.o cli_net.o cli_route.o cli_arp.o cli_os.o tenda_cli.o
#for cut image size
obj-y = cli_main.o cli_io.o cli_basic.o cli_net.o cli_route.o cli_os.o  cli_envram.o cli_arp.o tenda_cli.o
#obj-y = cli_main.o cli_io.o cli_basic.o cli_net.o cli_route.o cli_arp.o cli_os.o tenda_cli.o cli_envram.o


obj-$(CONFIG_UTILS) += cli_utils.o 

#
# Leave rule to make
#
$(MODULE): $(obj-y:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(obj-y)

clean:
	rm -f *.o

.PHONY:	clean
