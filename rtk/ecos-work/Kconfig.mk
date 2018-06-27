# ===========================================================================
# Kernel configuration targets
# These targets are used from top-level makefile

.PHONY: menuconfig

Kconfig := Kconfig

menuconfig: $(DIR_ROOT)/config/mconf
	@MODEL_NAME="`grep '^MODEL_NAME=*' .config`"
	$(DIR_ROOT)/config/mconf $(Kconfig)
	@if [ ! -z "${MODEL_NAME}" ];then \
		echo "MODEL_NAME=${MODEL_NAME}">>.config; \
	fi
oldconfig: $(DIR_ROOT)/config/conf
	@MODEL_NAME="`grep '^MODEL_NAME=*' .config`"
	$(DIR_ROOT)/config/conf -o $(Kconfig)
	@if [ ! -z "${MODEL_NAME}" ];then \
		echo "MODEL_NAME=${MODEL_NAME}">>.config; \
	fi
