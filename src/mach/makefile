# EPOS Machine Mediators Makefile

include	../../makedefs

SUBDIRS := common $(MACH)

all:		$(SUBDIRS)

$(SUBDIRS):	FORCE
		(cd $@ && $(MAKE))

clean:
		make MAKE:="$(MAKECLEAN)" $(SUBDIRS)

FORCE:
