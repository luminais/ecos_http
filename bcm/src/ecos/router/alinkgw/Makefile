
MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

ifeq "$(TOPDIR)" ""
	TOPDIR=$(shell cd .. ; pwd)
endif

all: $(MODULE)

include $(TOPDIR)/rules.mak

#
# Set CFLAGS or EXTRA_CFLAGS
#
CURR_DIR=$(shell pwd)
EXTRA_CFLAGS = -DHAVE_CONFIG_H -I$(CURR_DIR)/include
EXTRA_CFLAGS += -I$(SRCBASE)/include -I$(shell pwd;)/../tenda_httpd_ucd -I$(shell pwd;)/../tc/
#
# Enumerate subdirectories
#
SUBDIRS = src

subdirs: $(patsubst %, _dir_%, $(SUBDIRS))
$(patsubst %, _dir_%, $(SUBDIRS)) :
	$(MAKE) CFLAGS="$(CFLAGS) $(EXTRA_CFLAGS)" ENDIAN=$(ENDIAN) -C $(patsubst _dir_%, %, $@)

SUB_OBJS = $(join $(SUBDIRS), $(patsubst %, /obj-%.o, $(SUBDIRS)))

#
# Node rule to make
#
$(MODULE): subdirs $(SUB_OBJS)
	$(XLD) -r $(ENDIAN) -o $@ $(SUB_OBJS)

clean:
	find . -name '*.[od]' -type f -print | xargs rm -f

.PHONY:	clean
