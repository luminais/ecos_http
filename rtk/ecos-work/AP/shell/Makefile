include $(ECOS_AP_DIR)/config.mk

# Build tag
#BUILD_TAG = echo `date` foo@`hostname`
BUILD_HOST = echo foo@`hostname`

# Compiler release
COMPILER_RELEASE = echo `mips-elf-gcc --version | head -1`

# Whoami
UNAME = `whoami | cut -c 1-2`

#------------------------------------------------------------------------------
# EXTRA_CFLAGS
#------------------------------------------------------------------------------
EXTRA_CFLAGS =

#------------------------------------------------------------------------------
# CFLAGS/CXXFLAGS
#------------------------------------------------------------------------------
CFLAGS += $(EXTRA_CFLAGS) -I.
#CFLAGS += -pg $(EXTRA_CFLAGS) -I.
CXXFLAGS += $(EXTRA_CFLAGS) -I.

#CFLAGS +=	-DBUILD_TAG=\""$(shell $(BUILD_TAG))"\"
CFLAGS += -DBUILD_HOST=\""$(shell $(BUILD_HOST))"\" \
	  -DCOMPILER_RELEASE=\""$(shell $(COMPILER_RELEASE))"\" \
	  -DBUILD_FLAGS="\"$(DEBUG_FLAGS) $(OPT_FLAGS)"\" \

#------------------------------------------------------------------------------
obj-y =
obj-y += buildtag.o \
	builtin_commands.o \
	init.o \
	shell_err.o \
	shell_thread.o \
	shelltask.o \
	thread_cleanup.o \
	thread_info.o

all: $(TARGET)
	rm -f  buildtag.o

include $(ECOS_AP_DIR)/rules.mk
