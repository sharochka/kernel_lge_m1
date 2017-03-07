#
# kbuild file for firmware/
#

# Create $(fwabs) from $(CONFIG_EXTRA_FIRMWARE_DIR) -- if it doesn't have a
# leading /, it's relative to $(srctree).
fwdir := $(subst ",,$(CONFIG_EXTRA_FIRMWARE_DIR))
fwabs := $(addprefix $(srctree)/,$(filter-out /%,$(fwdir)))$(filter /%,$(fwdir))

fw-external-y := $(subst ",,$(CONFIG_EXTRA_FIRMWARE))

# There are three cases to care about:
# 1. Building kernel with CONFIG_FIRMWARE_IN_KERNEL=y -- $(fw-shipped-y) should
#    include the firmware files to include, according to .config
# 2. 'make modules_install', which will install firmware for modules, and 
#    _also_ for the in-kernel drivers when CONFIG_FIRMWARE_IN_KERNEL=n
# 3. 'make firmware_install', which installs all firmware, unconditionally.

# For the former two cases we want $(fw-shipped-y) and $(fw-shipped-m) to be
# accurate. In the latter case it doesn't matter -- it'll use $(fw-shipped-all).
# But be aware that the config file might not be included at all.

ifdef CONFIG_ACENIC_OMIT_TIGON_I
acenic-objs := acenic/tg2.bin
fw-shipped- += acenic/tg1.bin
else
acenic-objs := acenic/tg1.bin acenic/tg2.bin
endif

ifdef CONFIG_MACH_MSM8909_M1_SPR_US
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1_spr_us/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1_SPR_US) += $(mit200_firmware)
endif

ifdef CONFIG_MACH_MSM8909_M1_TMO_US
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1_spr_us/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1_TMO_US) += $(mit200_firmware)
endif

ifdef CONFIG_MACH_MSM8909_M1_MPCS_US
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1_spr_us/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1_MPCS_US) += $(mit200_firmware)
endif

ifdef CONFIG_MACH_MSM8909_M1_TRF_US
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1_spr_us/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1_TRF_US) += $(mit200_firmware)
endif

ifdef CONFIG_MACH_MSM8909_M1_TRF_US_VZW
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1_spr_us/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1_TRF_US_VZW) += $(mit200_firmware)
endif

ifdef CONFIG_MACH_MSM8909_M1DS_GLOBAL_COM
mit200_fw_file := $(wildcard $(srctree)/firmware/melfas/m1ds_global_com/*)
mit200_firmware := $(subst $(srctree)/firmware/,,$(mit200_fw_file))
fw-shipped-$(CONFIG_MACH_MSM8909_M1DS_GLOBAL_COM) += $(mit200_firmware)
endif

ifdef CONFIG_FIRMWARE_IN_KERNEL
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_MPR) += keyspan/mpr.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA18X) += keyspan/usa18x.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA19) += keyspan/usa19.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA19QI) += keyspan/usa19qi.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA19QW) += keyspan/usa19qw.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA19W) += keyspan/usa19w.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA28) += keyspan/usa28.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA28XA) += keyspan/usa28xa.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA28XB) += keyspan/usa28xb.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA28X) += keyspan/usa28x.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA49W) += keyspan/usa49w.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_USA49WLC) += keyspan/usa49wlc.fw
else
fw-shipped- += keyspan/mpr.fw keyspan/usa18x.fw keyspan/usa19.fw	\
	keyspan/usa19qi.fw keyspan/usa19qw.fw keyspan/usa19w.fw		\
	keyspan/usa28.fw keyspan/usa28xa.fw keyspan/usa28xb.fw		\
	keyspan/usa28x.fw keyspan/usa49w.fw keyspan/usa49wlc.fw
endif
fw-shipped-$(CONFIG_USB_SERIAL_TI) += ti_3410.fw ti_5052.fw \
				      mts_cdma.fw mts_gsm.fw mts_edge.fw
fw-shipped-$(CONFIG_USB_SERIAL_EDGEPORT) += edgeport/boot.fw edgeport/boot2.fw \
					    edgeport/down.fw edgeport/down2.fw
fw-shipped-$(CONFIG_USB_SERIAL_EDGEPORT_TI) += edgeport/down3.bin
fw-shipped-$(CONFIG_USB_SERIAL_WHITEHEAT) += whiteheat_loader.fw whiteheat.fw \
					   # whiteheat_loader_debug.fw
fw-shipped-$(CONFIG_USB_SERIAL_KEYSPAN_PDA) += keyspan_pda/keyspan_pda.fw
fw-shipped-$(CONFIG_USB_SERIAL_XIRCOM) += keyspan_pda/xircom_pgs.fw
fw-shipped-$(CONFIG_USB_VICAM) += vicam/firmware.fw
fw-shipped-$(CONFIG_VIDEO_CPIA2) += cpia2/stv0672_vp4.bin
fw-shipped-$(CONFIG_YAM) += yam/1200.bin yam/9600.bin

fw-shipped-all := $(fw-shipped-y) $(fw-shipped-m) $(fw-shipped-)

# Directories which we _might_ need to create, so we have a rule for them.
firmware-dirs := $(sort $(addprefix $(objtree)/$(obj)/,$(dir $(fw-external-y) $(fw-shipped-all))))

quiet_cmd_mkdir = MKDIR   $(patsubst $(objtree)/%,%,$@)
      cmd_mkdir = mkdir -p $@

quiet_cmd_ihex  = IHEX    $@
      cmd_ihex  = $(OBJCOPY) -Iihex -Obinary $< $@

quiet_cmd_copy  = COPY    $@ $(srctree) $(objtree)
      cmd_copy  = cp -u $(srctree)/$@ $@

quiet_cmd_ihex2fw  = IHEX2FW $@
      cmd_ihex2fw  = $(objtree)/$(obj)/ihex2fw $< $@

quiet_cmd_h16tofw  = H16TOFW $@
      cmd_h16tofw  = $(objtree)/$(obj)/ihex2fw -w $< $@

quiet_cmd_fwbin = MK_FW   $@
      cmd_fwbin = FWNAME="$(patsubst firmware/%.gen.S,%,$@)";		     \
		  FWSTR="$(subst /,_,$(subst .,_,$(subst -,_,$(patsubst	     \
				firmware/%.gen.S,%,$@))))";		     \
		  ASM_WORD=$(if $(CONFIG_64BIT),.quad,.long);		     \
		  ASM_ALIGN=$(if $(CONFIG_64BIT),3,2);			     \
		  PROGBITS=$(if $(CONFIG_ARM),%,@)progbits;		     \
		  echo "/* Generated by firmware/Makefile */"		> $@;\
		  echo "    .section .rodata"				>>$@;\
		  echo "    .p2align $${ASM_ALIGN}"			>>$@;\
		  echo "_fw_$${FWSTR}_bin:"				>>$@;\
		  echo "    .incbin \"$(2)\""				>>$@;\
		  echo "_fw_end:"					>>$@;\
		  echo "   .section .rodata.str,\"aMS\",$${PROGBITS},1"	>>$@;\
		  echo "    .p2align $${ASM_ALIGN}"			>>$@;\
		  echo "_fw_$${FWSTR}_name:"				>>$@;\
		  echo "    .string \"$$FWNAME\""			>>$@;\
		  echo "    .section .builtin_fw,\"a\",$${PROGBITS}"	>>$@;\
		  echo "    .p2align $${ASM_ALIGN}"			>>$@;\
		  echo "    $${ASM_WORD} _fw_$${FWSTR}_name"		>>$@;\
		  echo "    $${ASM_WORD} _fw_$${FWSTR}_bin"		>>$@;\
		  echo "    $${ASM_WORD} _fw_end - _fw_$${FWSTR}_bin"	>>$@;

# One of these files will change, or come into existence, whenever
# the configuration changes between 32-bit and 64-bit. The .S files
# need to change when that happens.
wordsize_deps := $(wildcard include/config/64bit.h include/config/32bit.h \
		include/config/ppc32.h include/config/ppc64.h \
		include/config/superh32.h include/config/superh64.h \
		include/config/x86_32.h include/config/x86_64.h)

# Workaround for make < 3.81, where .SECONDEXPANSION doesn't work.
# It'll end up depending on these targets, so make them a PHONY rule which
# depends on _all_ the directories in $(firmware-dirs), and it'll work out OK.
PHONY += $(objtree)/$$(%) $(objtree)/$(obj)/$$(%)
$(objtree)/$$(%) $(objtree)/$(obj)/$$(%): $(firmware-dirs)
	@true

# For the $$(dir %) trick, where we need % to be expanded first.
.SECONDEXPANSION:

$(patsubst %,$(obj)/%.gen.S, $(fw-shipped-y)): %: $(wordsize_deps) \
		| $(objtree)/$$(dir %)
	$(call cmd,fwbin,$(patsubst %.gen.S,%,$@))
$(patsubst %,$(obj)/%.gen.S, $(fw-external-y)): %: $(wordsize_deps) \
		include/config/extra/firmware/dir.h | $(objtree)/$$(dir %)
	$(call cmd,fwbin,$(fwabs)/$(patsubst $(obj)/%.gen.S,%,$@))

# The .o files depend on the binaries directly; the .S files don't.
$(patsubst %,$(obj)/%.gen.o, $(fw-shipped-y)): %.gen.o: %
$(patsubst %,$(obj)/%.gen.o, $(fw-external-y)): $(obj)/%.gen.o: $(fwdir)/%

# .ihex is used just as a simple way to hold binary files in a source tree
# where binaries are frowned upon. They are directly converted with objcopy.
$(obj)/%: $(obj)/%.ihex | $(objtree)/$(obj)/$$(dir %)
	$(call cmd,ihex)

$(obj)/%.fw: $(objtree)/$(obj)/$$(dir %.fw)
	$(call cmd,copy)

$(obj)/%.img: $(objtree)/$(obj)/$$(dir %.img)
	$(call cmd,copy)

$(obj)/%.bin: $(objtree)/$(obj)/$$(dir %.bin)
ifneq ($(objtree),$(srctree))
	$(call cmd,copy)

endif

$(obj)/%.txt: $(objtree)/$(obj)/$$(dir %.txt)
ifneq ($(objtree),$(srctree))
	$(call cmd,copy)
endif
# Don't depend on ihex2fw if we're installing and it already exists.
# Putting it after | in the dependencies doesn't seem sufficient when
# we're installing after a cross-compile, because ihex2fw has dependencies
# on stuff like /usr/lib/gcc/ppc64-redhat-linux/4.3.0/include/stddef.h and 
# thus wants to be rebuilt. Which it can't be, if the prebuilt kernel tree
# is exported read-only for someone to run 'make install'.
ifeq ($(INSTALL):$(wildcard $(obj)/ihex2fw),install:$(obj)/ihex2fw)
ihex2fw_dep :=
else
ihex2fw_dep := $(obj)/ihex2fw
endif

# .HEX is also Intel HEX, but where the offset and length in each record
# is actually meaningful, because the firmware has to be loaded in a certain
# order rather than as a single binary blob. Thus, we convert them into our
# more compact binary representation of ihex records (<linux/ihex.h>)
$(obj)/%.fw: $(obj)/%.HEX $(ihex2fw_dep) | $(objtree)/$(obj)/$$(dir %)
	$(call cmd,ihex2fw)

# .H16 is our own modified form of Intel HEX, with 16-bit length for records.
$(obj)/%.fw: $(obj)/%.H16 $(ihex2fw_dep) | $(objtree)/$(obj)/$$(dir %)
	$(call cmd,h16tofw)

$(firmware-dirs):
	$(call cmd,mkdir)

obj-y				 += $(patsubst %,%.gen.o, $(fw-external-y))
obj-$(CONFIG_FIRMWARE_IN_KERNEL) += $(patsubst %,%.gen.o, $(fw-shipped-y))

# Remove .S files and binaries created from ihex
# (during 'make clean' .config isn't included so they're all in $(fw-shipped-))
targets := $(fw-shipped-) $(patsubst $(obj)/%,%, \
				$(shell find $(obj) -name \*.gen.S 2>/dev/null))

# Without this, built-in.o won't be created when it's empty, and the
# final vmlinux link will fail.
obj-n := dummy

hostprogs-y := ihex2fw
