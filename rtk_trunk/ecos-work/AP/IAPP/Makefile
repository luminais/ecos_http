include $(ECOS_WORK_DIR)/.config
#CC=rsdk-linux-gcc
CFLAGS := -s -Wall -c -Os $(CFLAGS)
LDFLAGS = -s -Wall

ifeq ($(SLINK),1)
CFLAGS  += -ffunction-sections -fdata-sections
LDFLAGS += --static -s -Wl,--gc-sections
endif

all:		iapp.o version.o
	$(AR) rcs $(APP_LIB) $(COMMON_LIB) iapp.o version.o

clean:
			rm -f *.o
			rm -f core
iapp.o:		iapp.c iapp.h common.h misc.h
			$(CC) $(CFLAGS) -o iapp.o iapp.c

version.o:	version.c
			$(CC) $(CFLAGS) -o version.o version.c
