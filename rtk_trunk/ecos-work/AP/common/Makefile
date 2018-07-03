
include $(ECOS_WORK_DIR)/.config

PREFIX?=$(notdir $(shell pwd))_
#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

ifeq ($(CONFIG_APMIB),y)
EXTRA_CFLAGS += -I../network -I../apmib
endif
ifeq ($(CONFIG_CUTE_MAHJONG),y)
EXTRA_CFLAGS += -DCONFIG_CUTE_MAHJONG_SELECTABLE
endif

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I. 
#CFLAGS += $(EXTRA_CFLAGS) -I.  -DHAVE_LZMA
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

ifeq ($(CONFIG_HAVE_LZMA),y)
SOURCES = common.c md5.c LzmaDecode.c
else
SOURCES = common.c md5.c
endif

#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)

PREFIX_OBJS=$(join $(dir $(OBJS)),$(addprefix $(PREFIX),$(notdir $(OBJS))))
#PREFIX_OBJS=$(addprefix $(PREFIX),$(OBJS))

all: depend $(OBJS) Makefile $(PREFIX_OBJS)
	$(AR) rs $(APP_LIB) $(PREFIX_OBJS)

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

$(PREFIX)%.o: %.o
	@cp $^ $@

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~ 

depend: $(SOURCES)
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend
