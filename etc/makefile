# EPOS Configuration Makefile

include	../makedefs

all: $(APP)/$(APPLICATION)_traits.h eposcc.conf eposmkbi.conf $(INCLUDE)/system/config.h
		sed -e 's/^#define ARCH.*$$/#define ARCH $(ARCH)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MACH.*$$/#define MACH $(MACH)/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define APPLICATION.*$$/#define APPLICATION $(APPLICATION)/' -i $(INCLUDE)/system/config.h

$(APP)/$(APPLICATION)_traits.h:
		$(INSTALL) $(INCLUDE)/system/traits.h $@
		
eposcc.conf:
		@echo "ARCH=$(ARCH)" > $@
		@echo "MACH=$(MACH)" >> $@
		@echo "TOOLS_PREFIX=$(COMP_PREFIX)" >> $@
		@echo "APP_CODE=$(APP_CODE_ADDR)" >> $@
		@echo "APP_DATA=$(APP_DATA_ADDR)" >> $@
		@echo "MACH_CC_FLAGS='$(MACH_CC_FLAGS)'" >> $@
		@echo "MACH_LD_FLAGS='$(MACH_LD_FLAGS)'" >> $@
		@echo "MACH_CODE_NAME='$(MACH_CODE_NAME)'" >> $@
		@echo "MACH_DATA_NAME='$(MACH_DATA_NAME)'" >> $@

eposmkbi.conf:	
		@echo "MODE=$(MODE)" > $@
		@echo "MACH=$(MACH)" >> $@
		@echo "ARCH=$(ARCH)" >> $@
		@echo "CLOCK=$(ARCH_CLOCK)" >> $@
		@echo "WORD_SIZE=$(ARCH_WORD_SIZE)" >> $@
		@echo "ENDIANESS=$(ARCH_ENDIANESS)" >> $@
		@echo "MEM_BASE=$(MEM_BASE)" >> $@
		@echo "MEM_TOP=$(MEM_TOP)" >> $@
		@echo "BOOT_LENGTH_MIN=$(BOOT_LENGTH_MIN)" >> $@
		@echo "BOOT_LENGTH_MAX=$(BOOT_LENGTH_MAX)" >> $@

clean: $(APP)/$(APPLICATION)_traits.h
		sed -e 's/^#define ARCH.*$$/#define ARCH xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define MACH.*$$/#define MACH xxx/' -i $(INCLUDE)/system/config.h
		sed -e 's/^#define APP.*$$/#define APPLICATION xxx/' -i $(INCLUDE)/system/config.h
		$(CLEAN) *.conf

FORCE: