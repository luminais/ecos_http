include $(ECOS_AP_DIR)/config.mk

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS += -O2 -fno-strict-aliasing -pipe

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS +=-DINET6 -DPIM -DIOCTL_OK_ON_RAW_SOCKET
CFLAGS += $(EXTRA_CFLAGS) -I.
CFLAGS += -DUSE_RFC2292BIS
ifeq ($(CONFIG_NET_STACK_FREEBSD),y)
CFLAGS += -D__FreeBSD__=5
endif

CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
# Sources
#------------------------------------------------------------------------------
OBJS = callout.o debug.o kern.o mld6.o mrt.o pim6_proto.o routesock.o trace.o vif.o config.o inet6.o main.o mld6_proto.o pim6.o route.o timer.o vers.o
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))
obj-y+=$(PREFIX_OBJS)
include $(ECOS_AP_DIR)/rules.mk
