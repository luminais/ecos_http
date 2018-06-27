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
VPATH := $(SRCBASE)/wl/exe:$(SRCBASE)/shared

INDIRECT_SOURCES = wlu_ecos.c

# wl utility
INDIRECT_SOURCES += wlu.c miniopt.c bcm_app_utils.c
INDIRECT_SOURCES += $(wildcard $(SRCBASE)/wl/exe/wlu_common.c)


OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/include -I$(SRCBASE)/wl/exe -I$(SRCBASE)/router/shared -I.

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(OBJS)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(OBJS)
endif

# prebuild
prebuild: $(MODULE)
