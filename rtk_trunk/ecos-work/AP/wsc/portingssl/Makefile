
include $(ECOS_WORK_DIR)/.config

#------------------------------------------------------------------------------
OUTDIR	= .
INCLUDES	= -I./crypto/include -I./crypto/include/openssl -I./

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =
ifeq ($(CONFIG_MIPS16),y)
EXTRA_CFLAGS += -mips16
endif

EXTRA_CFLAGS +=  -DOPENSSL_FIPS -D__linux__ -DRSDK_BUILT -DOPENSSL_NO_SPEED -DOPENSSL_THREADS -D_REENTRANT \
		-DDSO_DLFCN -DHAVE_DLFCN_H -DOPENSSL_NO_KRB5 -DB_ENDIAN -DTERMIO \
		$(INCLUDES)

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#------------------------------------------------------------------------------
.SUFFIXES: .o .c .cpp .cxx .s .S

#------------------------------------------------------------------------------
all: libcrypto
	$(AR) rcs $(APP_LIB) $(OBJS)

include Files.mk

OBJS = $(OBJFILES)

.PHONY: libcrypto
libcrypto:  $(OBJS) Makefile

.c.o:
	$(CC) $(CFLAGS) -o $@ -c $<

.cpp.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

.cxx.o:
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	rm -f $(OBJS) *.o *~ 
