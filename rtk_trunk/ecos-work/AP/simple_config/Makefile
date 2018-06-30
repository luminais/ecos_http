
include ../../.config

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif
EXTRA_CFLAGS += -I../common
#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

ifeq ($(CONFIG_SIMPLE_CONFIG_PBC_SUPPORT),y)
CFLAGS += -DCONFIG_SIMPLE_CONFIG_PBC_SUPPORT
endif

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

SOURCES = simple_config.c 1x_md5_sc.c 

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)
all: depend $(OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(OBJS)

	
.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
