#
# Leaf Makefile
#
include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS = -I$(SRCBASE)/router/shared -I$(SRCBASE)/router/nas -I$(SRCBASE)/include -I$(SRCBASE)/include/bcmcrypto
EXTRA_CFLAGS += -I$(SRCBASE)/router/eapd
EXTRA_CFLAGS += -Wall -Werror -DBCMDRIVER
EXTRA_CFLAGS += -DNAS_WKSP_BUILD_NAS_AUTH
EXTRA_CFLAGS += -DNAS_WKSP_BUILD_NAS_SUPPL
EXTRA_CFLAGS += -DNAS_RADIUS
EXTRA_CFLAGS += -DNAS_WKSP_ON_DEMAND
EXTRA_CFLAGS += -DBCMSUPPL
EXTRA_CFLAGS += -DBCMWPA2

#
# Enumerate files to compile
#
VPATH = $(SRCBASE)/router/nas:$(SRCBASE)/router/shared:$(SRCBASE)/bcmcrypto:$(SRCBASE)/shared

INDIRECT_SOURCES = mppe.c nas_wl.c nas.c wpa.c nas_wksp.c unix.c
INDIRECT_SOURCES += nas_radius.c nas_wksp_radius.c
#INDIRECT_SOURCES += bcmwpa.c ecos_timer.c
INDIRECT_SOURCES += ecos_timer.c
INDIRECT_SOURCES += nas_ecos.c

ALL_OBJS := $(INDIRECT_SOURCES:.c=.o)

#
# Leave rule to make
#
ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): $(ALL_OBJS:.o=.d)
	$(XLD) -r $(ENDIAN) -o $(MODULE) $(ALL_OBJS)
endif

# prebuild
prebuild: $(MODULE)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY: clean
