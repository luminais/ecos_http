MODULE = obj-$(shell pwd | sed "s/.*\///" ).o
all: $(MODULE)
include $(ECOS_AP_DIR)/rules.mk

#
# Set CFLAGS or EXTRA_CFLAGS
#

#-Werror


#
# Enumerate files to compile
#
DEBUG_CFLAGS =
obj-y = cJSON.o
ALL_OBJS += $(obj-y)

#
# Leave rule to make
#
$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS)

ifneq "" "$(ALL_OBJS:.o=.d)"
-include $(ALL_OBJS:.o=.d)
endif
