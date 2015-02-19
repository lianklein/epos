# EPOS Image Makefile

include	../makedefs

IMAGE := $(addsuffix .img,$(APPLICATION))
OUTPUT := $(addsuffix .out,$(APPLICATION))

all:		$(IMAGE)

run:		$(IMAGE)
ifeq ($(NODES),1)
		$(EMULATOR) $(IMAGE) | $(TEE) $(OUTPUT)
else
#		$(TERM) "$(EMULATOR) $(IMAGE) -net nic,model=pcnet,macaddr=56:34:12:0:54:08 -net dump,file=r-$(APPLICATION).pcap -net socket,listen=:1234 -net user,net=10.0.1.0/24,host=10.0.1.1,dhcpstart=10.0.1.8 | $(TEE) r-$(OUTPUT)
		$(TERM) "$(EMULATOR) $(IMAGE) -net nic,model=pcnet,macaddr=56:34:12:0:54:08 -net dump,file=r-$(APPLICATION).pcap -net socket,listen=:1234 | $(TEE) r-$(OUTPUT) \
		&& $(TCPDUMP) r-$(APPLICATION).pcap > r-$(APPLICATION).net \
		&& read -p 'Press [Enter] key to close ...'" &
		sleep 2
		$(TERM) "$(EMULATOR) $(IMAGE) -net nic,model=pcnet,macaddr=56:34:12:0:54:09 -net dump,file=s-$(APPLICATION).pcap -net socket,connect=:1234 | $(TEE) s-$(OUTPUT) \
		&& $(TCPDUMP) s-$(APPLICATION).pcap > s-$(APPLICATION).net \
		&& read -p 'Press [Enter] key to close ...'" &
endif

debug:		$(IMAGE)
		$(EMULATOR) $(IMAGE) | $(TEE) $(OUTPUT) &
		$(DEBUGGER) $(APP)/$(APPLICATION)

clean:
		$(CLEAN) *_boot *_setup *_init *_system $(APPLICATION)
