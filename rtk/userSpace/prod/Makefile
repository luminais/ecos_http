
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config
#
# Set CFLAGS or EXTRA_CFLAGS
#

#
# Enumerate subdirectories
#
obj-y = http
obj-y += lib
obj-$(CONFIG_ATE) += ate
obj-$(CONFIG_ALINKGW) += alinkgw
obj-$(CONFIG_HTTPD) += httpd
obj-$(CONFIG_TENDA_APP) += app_parse
obj-$(CONFIG_ELINK) += e_link

SUBDIRS = $(obj-y)

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $@ $(SUB_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
