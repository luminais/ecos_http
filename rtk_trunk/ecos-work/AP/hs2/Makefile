include $(ECOS_WORK_DIR)/.config
#CC=rsdk-linux-gcc
CFLAGS := -Os $(CFLAGS)
LDFLAGS = -s -Wall

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -s -Wl,--gc-sections
endif

SOURCES = hs2.c hs2_utility.c
SOURCES += hs2_wlan0.c hs2_wlan1.c


#------------------------------------------------------------------------------
DEPEND = ./.depend

OBJS = $(SOURCES:.c=.o)

all:depend  $(OBJS) Makefile
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) $(OBJS)


.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(DEPEND)
	rm -f $(OBJS) *.o *~
hs2_wlan0.c hs2_wlan1.c:DESC_FILE
DESC_FILE:
	./bin2c.pl hs2_wlan0_conf < hs2_wlan0.conf > ./hs2_wlan0.c
	./bin2c.pl hs2_wlan1_conf < hs2_wlan1.conf > ./hs2_wlan1.c

depend:$(SOURCES) 
	@if [ ! -e $(DEPEND) ]; then \
		$(CPP) $(CFLAGS) -MM $^ > $(DEPEND); \
	fi

-include $(DEPEND)

.PHONY: clean all depend


