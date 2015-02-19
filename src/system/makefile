# EPOS System Makefile

include ../../makedefs

OBJS := $(subst .cc,.o,$(shell find *.cc))

all:		install

install: $(MACH)_install_$(MODE)

$(MACH)_install_library: $(MACH)_system_library
		$(INSTALL) $(MACH)_system.o $(LIB)
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_install_builtin: $(MACH)_system_builtin
ifndef 	GDB
		$(STRIP) -o $(MACH)_system.tmp $(MACH)_system
else
		$(INSTALL) $(MACH)_system $(MACH)_system.tmp
endif
		$(INSTALL) $(MACH)_system.tmp $(IMG)/$(MACH)_system
		$(CLEAN) $(MACH)_system.tmp
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_install_kernel: $(MACH)_system_kernel
ifndef 	GDB
		$(STRIP) -o $(MACH)_system.tmp $(MACH)_system
else
		$(INSTALL) $(MACH)_system $(MACH)_system.tmp
endif
		$(INSTALL) $(MACH)_system.tmp $(IMG)/$(MACH)_system
		$(CLEAN) $(MACH)_system.tmp
		$(INSTALL) $(MACH)_application.o $(LIB)

$(MACH)_system_library: $(OBJS)
		$(INSTALL) system_scaffold.o $(MACH)_system.o
		$(INSTALL) application_scaffold.o $(MACH)_application.o

$(MACH)_system_builtin: $(OBJS)
		$(LD) $(LDFLAGS) -L$(CCLIB) --nmagic \
		--section-start $(MACH_CODE_NAME)=$(SYS_CODE_ADDR) \
		--section-start $(MACH_DATA_NAME)=$(SYS_DATA_ADDR) \
		--entry=_init -o $(MACH)_system \
		$(LIB)/$(ARCH)_crtbegin.o \
		system_scaffold.o \
		$(LIB)/$(ARCH)_crtend.o \
		--whole-archive \
		-l$(LSYS) -l$(LMACH) -l$(LARCH) \
		--no-whole-archive \
		-l$(LUTIL) -lgcc
		$(INSTALL) application_scaffold.o $(MACH)_application.o

$(MACH)_system_kernel: $(OBJS) 
		$(LD) $(LDFLAGS) -L$(CCLIB) --nmagic \
		--section-start $(MACH_CODE_NAME)=$(SYS_CODE_ADDR) \
		--section-start $(MACH_DATA_NAME)=$(SYS_DATA_ADDR) \
		--entry=_init -o $(MACH)_system \
		$(LIB)/$(ARCH)_crtbegin.o \
		system_scaffold.o \
		$(LIB)/$(ARCH)_crtend.o \
		--whole-archive \
		-l$(LSYS) -l$(LMACH) -l$(LARCH) \
		--no-whole-archive \
		-l$(LUTIL) -lgcc 
		$(INSTALL) application_scaffold.o $(MACH)_application.o
		
clean:
		$(CLEAN) *.o *_system

FORCE:
