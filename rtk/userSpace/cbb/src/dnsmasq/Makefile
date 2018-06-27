#
# dns Makefile
# llm, 2015-12-26 16:12:06
#

MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)


#
# Enumerate subdirectories
#
SUBDIRS = src

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
	find . -name '*.[od]' -type f -print0 | xargs -0 rm -f

.PHONY:	clean 
.PHONY: $(patsubst %, _dir_%, $(SUBDIRS))
