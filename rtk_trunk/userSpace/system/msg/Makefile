MODULE = obj-$(shell pwd | sed "s/.*\///").o

all: $(MODULE)

include $(TENDA_APP_DIR)/tenda_rules.mk

EXTRA_CFLAGS = -I$(shell pwd)/include -I$(TENDA_APP_DIR)/system/include -I$(TENDA_APP_DIR)/include

ALL_OBJS = msg_ecos.o msg_sh.o msg_cli.o

$(MODULE): $(ALL_OBJS)
	$(LD) -r $(TDAPP_LDFLAGS) -o $(MODULE) $(ALL_OBJS) 


