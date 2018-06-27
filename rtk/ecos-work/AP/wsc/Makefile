include $(ECOS_WORK_DIR)/.config
ifneq ($(CONFIG_SSL),y)
CRYPTLIB = portingssl
endif
WSCSRC = src

all:
ifneq ($(CONFIG_SSL),y)
	make -C $(CRYPTLIB)
endif
	make -C $(WSCSRC)
	
clean:
ifneq ($(CONFIG_SSL),y)
	make -C $(CRYPTLIB) clean
endif
	make -C $(WSCSRC) clean
