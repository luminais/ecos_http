#
# ecos router libupnp Makefile
# llm, 2015-12-29 10:32:51
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

LIBUPNP = $(shell pwd)
SRCPATH = $(LIBUPNP)/upnp:$(LIBUPNP)/md5

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(LIBUPNP)/include -I$(LIBUPNP)/ecos \
	       -I$(TENDA_APP_DIR)/include \
		   -I$(TENDA_APP_DIR)/router_shared \
		   -I$(LIBUPNP)/md5

#
# Enumerate files to compile
#
vpath %.c $(SRCPATH) $(LIBUPNP)/ecos

ALL_SRCS = upnp.c upnp_ssdp.c upnp_http.c upnp_gena.c upnp_soap.c \
	   upnp_description.c upnp_device.c upnp_util.c upnp_msg.c \
	   upnp_ecos_osl.c md5.c

ALL_OBJS = ${ALL_SRCS:.c=.o}

#
# Leave rule to make
#

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)


clean:
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	all clean 
	
include $(TENDA_APP_DIR)/tenda_rules.mk
