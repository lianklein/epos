# EPOS Application Makefile

include ../makedefs

all: install

$(APPLICATION):	$(APPLICATION).o $(LIB)/*
		$(ALD) $(ALDFLAGS) -o $@ $(APPLICATION).o

$(APPLICATION).o: $(APPLICATION).cc $(SRC)
		$(ACC) $(ACCFLAGS) -o $@ $<

install: $(APPLICATION)
ifndef  GDB
		$(STRIP) -o $(APPLICATION).tmp $<
else
		$(INSTALL) $< $(APPLICATION).tmp
endif
		$(INSTALL) $(APPLICATION).tmp $(IMG)/$(APPLICATION)
		$(CLEAN) $(APPLICATION).tmp

clean:
		$(CLEAN) *.o $(APPLICATION)