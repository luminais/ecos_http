include $(ECOS_AP_DIR)/config.mk

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS += -O2 -fno-strict-aliasing -pipe

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.
EXTRA_CFLAGS += -I../apmib

#------------------------------------------------------------------------------
# Sources
#------------------------------------------------------------------------------
OBJS = mrouted_main.o mrinfo.o callout.o config.o igmp.o inet.o kern.o prune.o vif.o
PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))
obj-y +=$(PREFIX_OBJS)

include $(ECOS_AP_DIR)/rules.mk

