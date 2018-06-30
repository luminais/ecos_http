#
# ecos router igd module Makefile
# llm, 2015-12-29 10:27:17
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


LIBUPNP = ../libupnp
SRCPATH = ../igd
DEVPATH = $(SRCPATH)/InternetGatewayDevice
OSLPATH = $(SRCPATH)/ecos

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(LIBUPNP)/include -I$(LIBUPNP)/ecos -I$(SRCPATH) -I$(DEVPATH) -I$(OSLPATH) \
	-I$(TENDA_APP_DIR)/router_shared
	

#
# Enumerate files to compile
#

VPATH = $(SRCPATH):$(DEVPATH):$(OSLPATH)

# all:
	# echo $(VPATH)
INDIRECT_SOURCES =  \
	igd_ecos_main.c \
	igd_ecos_osl.c \
	igd_mainloop.c \
	InternetGatewayDevice_table.c \
	InternetGatewayDevice.c \
	soap_x_layer3forwarding.c \
	soap_x_wancommoninterfaceconfig.c \
	soap_x_wanipconnection.c \
	xml_InternetGatewayDevice.c \
	xml_x_layer3forwarding.c \
	xml_x_wancommoninterfaceconfig.c \
	xml_x_wanipconnection.c

ALL_OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk
