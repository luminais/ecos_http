#
# Leaf Makefile
#
include $(TOPDIR)/.config

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE) $(ALL_OBJS) $(ALL_OBJS:.o=.d)

include $(TOPDIR)/rules.mak

EAPD_SRC_DIR = $(SRCBASE)/router/eapd
CUR_DIR = $(shell pwd)

#
# Set CFLAGS or EXTRA_CFLAGS
#
EXTRA_CFLAGS += -I$(EAPD_SRC_DIR) -I$(SRCBASE)/shared -I$(SRCBASE)/include -I$(SRCBASE)/router/shared -I$(SRCBASE)/router/nas
EXTRA_CFLAGS += -Wall -Werror -DBCMWPA2 -DEAPD_WKSP_AUTO_CONFIG
EXTRA_CFLAGS += -DEAPDDUMP

#
# Enumerate files to compile
#
VPATH = $(EAPD_SRC_DIR)

INDIRECT_SOURCES = eapd.c nas_eap.c wps_eap.c ses_eap.c eapd_ecos.c

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
