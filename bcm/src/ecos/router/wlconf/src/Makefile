#
# Leaf Makefile
#
include $(TOPDIR)/.config

MODULE = $(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Enumerate files to compile
#
VPATH := $(SRCBASE)/router/wlconf

INDIRECT_SOURCES = wlconf.c

OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/include -I$(SRCBASE)/router/shared -I. -Werror

#
# Leave rule to make
#
$(MODULE): $(OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(OBJS)
