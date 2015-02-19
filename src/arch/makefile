# EPOS Architecture Mediators Makefile

include	../../makedefs

SUBDIRS := $(ARCH)

all:		$(SUBDIRS)

$(SUBDIRS):	FORCE
		(cd $@ && $(MAKE))

clean:
		make MAKE:="$(MAKECLEAN)" $(SUBDIRS)

FORCE:
