MODULE = obj-$(shell pwd | sed "s/.*\///" ).o

all: $(MODULE)

#
# Enumerate subdirectories
#

SUBDIRS = tpi api gpi src

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
	find . -path ./src/tenda_app -prune -o -type f -name '*.[od]' -print0 | xargs -0 rm -f

.PHONY:	clean 
.PHONY: $(patsubst %, _dir_%, $(SUBDIRS))
