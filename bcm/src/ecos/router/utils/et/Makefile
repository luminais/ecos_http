#
# Leaf Makefile
#
include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Enumerate files to compile
#
INDIRECT_SOURCES =
# et utility
INDIRECT_SOURCES += etu_ecos.c

OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/include

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(OBJS)
endif

# prebuild
prebuild: $(MODULE)
