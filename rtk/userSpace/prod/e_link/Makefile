
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

#CFLAGS += -Werror
CFLAGS +=  -I$(TENDA_APP_DIR)/cbb/include


all: clean $(MODULE)

clean:
	find ./ -name "*.[od]" | xargs rm -f

include $(TENDA_APP_DIR)/tenda_rules.mk
include $(TENDA_APP_DIR)/.config


PATH:=$(ECOS_TOPDIR)/ecos-3.0/tools/bin:$(ECOS_TOPDIR)/gnutools/mips-elf/bin:$(PATH)
export PATH

#
# Enumerate subdirectories
#
SUBDIRS = common elink_client  aes_cbc dh elink_sdk 
ENDIAN = $(TDAPP_LDFLAGS)
subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#

ifneq ($(wildcard $(MODULE).gz),$(MODULE).gz)
$(MODULE): subdirs $(SUB_OBJS)
	$(LD) -r $(ENDIAN) -o $@ $(SUB_OBJS)
endif

# prebuild
prebuild: $(MODULE)
	@gzip -f $(MODULE)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
