LIB = $(MIPS)_$(PREFIX)_mini_upnp.a

include $(USER_CONFIG)

ifeq ($(CONFIG_WPS_RTK),y)
all: 
	$(AR) -x $(LIB)
	$(AR) rcs $(APP_LIB) *.o
	rm *.o
endif
clean:
	@echo "do nothing"
