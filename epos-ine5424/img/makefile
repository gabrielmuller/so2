# EPOS Image Makefile

include	../makedefs

SYSTEM		:= $(shell find . -name $(MACH)_\* -and -not -name $(MACH)_boot)
IMAGES 		:= $(subst .img,,$(shell find . -name \*.img -printf "%f\n")) \
		$(IMAGES) $(subst .bin,,$(shell find . -name \*.bin -printf "%f\n")) \
		$(IMAGES) $(subst .hex,,$(shell find . -name \*.bin -printf "%f\n"))
IMAGE		:= $(addsuffix $(MACH_IMGSUFF),$(APPLICATION))
OUTPUT		:= $(addsuffix .out,$(APPLICATION))
PEER		:= p-$(APPLICATION)
PEER_IMAGE	:= p-$(IMAGE)
PEER_OUTPUT	:= p-$(OUTPUT)

ifeq ($(MMOD),emote3)
FLASH		:= $(addsuffix .hex,$(APPLICATION))
else
FLASH		:= $(IMAGE)
endif

DEBUGGER	:= $(MACH_DEBUGGER)
ifeq ($(DEBUG),1)
ifneq ($(wildcard $(IMG)/$(MACH)_setup),)
DEBUGGER	:= $(DEBUGGER) -ex "add-symbol-file $(call DBSEC,$(SRC)/setup/$(MACH)_setup,.text)"
endif
ifneq ($(wildcard $(IMG)/$(MACH)_init),)
DEBUGGER	:= $(DEBUGGER) -ex "add-symbol-file $(call DBSEC,$(SRC)/init/$(MACH)_init,.text)"
endif
ifneq ($(wildcard $(IMG)/$(MACH)_system),)
DEBUGGER	:= $(DEBUGGER) -ex "add-symbol-file $(call DBSEC,$(SRC)/system/$(MACH)_system,.text)"
endif
PEER_DEBUGGER	:= $(DEBUGGER) -ex "add-symbol-file $(call DBSEC,$(APP)/$(APPLICATION),.text)"
PEER_DEBUGGER	:= $(subst 1235,1236,$(PEER_DEBUGGER))
endif

PEER_EMULATOR	:= $(subst 1235,1236,$(EMULATOR))

NETWORK	        := -netdev socket,id=vlan0,listen=:1234           -object filter-dump,id=dump0,netdev=vlan0,file=$(APPLICATION).pcap   -device rtl8139,id=eth0,netdev=vlan0,mac=56:34:12:00:54:08 -device rtl8139,id=eth1,mac=00:00:00:00:00:01
# add the rest of the line to enable a dhcp server -netdev user,id=1,ipv6=off,restrict=on,net=10.0.1.0/24,host=10.0.1.1,dhcpstart=10.0.1.8
PEER_NETWORK	:= -netdev socket,id=vlan0,connect=127.0.0.1:1234 -object filter-dump,id=dump0,netdev=vlan0,file=$(PEER).pcap -device rtl8139,id=eth0,netdev=vlan0,mac=56:34:12:00:54:09 -device rtl8139,id=eth1,mac=00:00:00:00:00:02

ifeq ($(NODES),1)

all:		strip $(IMAGE)

else

all:		strip $(IMAGE) $(PEER_IMAGE)

$(IMAGE):	$(APPLICATION)
		$(MKBI) -x 0 -y 0 -z 0 $@ $<

$(PEER_IMAGE):	$(APPLICATION)
		$(MKBI) -x 10 -y 10 -z 0 $@ $<

endif

eposcfg: FORCE
		(cd $(TLS)/eposcfg && $(MAKE) clean all)
		(cd $(ETC) && $(MAKE) all)

flash:		strip $(FLASH)
		$(FLASHER) $(IMG)/$(FLASH)

run:		all run1

run1:		eposcfg
ifneq ($(STIME),0)
ifeq ($(NODES),1)
		timeout --foreground --kill-after=2s --signal=9 $(STIME)s $(EMULATOR)$(IMAGE) | $(TEE) $(OUTPUT)
else
		$(TERM) "timeout --foreground --kill-after=2s --signal=9 $(STIME)s $(EMULATOR)$(IMAGE) $(NETWORK) | $(TEE) $(OUTPUT) \
		&& $(TCPDUMP) $(APPLICATION).pcap > $(APPLICATION).net \
		&& read -p 'Press [Enter] key to close ...'" &
		sleep 2
		$(TERM) "timeout --foreground --kill-after=2s --signal=9 $(STIME)s $(EMULATOR)$(PEER_IMAGE) $(PEER_NETWORK) | $(TEE) $(PEER_OUTPUT) \
		&& $(TCPDUMP) $(PEER).pcap > $(PEER).net \
		&& read -p 'Press [Enter] key to close ...'" &
endif
endif

debug:		$(IMAGE)
ifeq ($(NODES),1)
		$(EMULATOR)$(IMAGE) | $(TEE) $(OUTPUT) &
		$(DEBUGGER) $(APP)/$(APPLICATION)
else
		$(TERM) "$(EMULATOR)$(IMAGE) $(NETWORK) | $(TEE) $(OUTPUT) \
		&& $(TCPDUMP) $(APPLICATION).pcap > $(APPLICATION).net \
		&& read -p 'Press [Enter] key to close ...'" &
		$(DEBUGGER) $(APP)/$(APPLICATION) &
		sleep 2
		$(TERM) "$(PEER_EMULATOR)$(PEER_IMAGE) $(PEER_NETWORK) | $(TEE) $(PEER_OUTPUT) \
		&& $(TCPDUMP) $(PEER).pcap > $(PEER).net \
		&& read -p 'Press [Enter] key to close ...'" &
		$(PEER_DEBUGGER) $(APP)/$(PEER) &
endif

runall:		FORCE
		$(foreach app,$(IMAGES),$(MAKE) APPLICATION=$(app) prerun_$(app) run1 posrun_$(app);)

strip:		$(SYSTEM) $(APPLICATION)
		@for file in $^; do $(STRIP) "$${file}"; done;

clean:		FORCE
		$(CLEAN) $(SYSTEM) $(APPLICATION) $(IMAGE)
ifneq ($(NODES),1)
		$(CLEAN) $(PEER_IMAGE)
endif

.PHONY: prebuild_$(APPLICATION) posbuild_$(APPLICATION) prerun_$(APPLICATION)
prerun_$(APPLICATION):
#		@echo "Cooling down for 10s ..."
#		sleep 10
		@echo "Running $(APPLICATION) ..."
posrun_$(APPLICATION):
		@echo "done!"
		
FORCE:
