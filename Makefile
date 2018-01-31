#######################################################################
#
#  Realtek Semiconductor Corp.
#  Makefile
#
#  phinex hung (phinexhung@realtek.com)
#  July 7, 2009
#


#########################################################################
# Path Setting for RSDK Related functions
#########################################################################
SRC_DIR    =./src
ROMSRC_DIR =./romsrc
FLASH_DIR  =./flash
FLASH_RLX  =./flashfp
OBJ_DIR    =./obj
ROMOBJ_DIR =./romobj
#RSDK      = /cygdrive/c/rsdk
#RSDK       = /usr/local/rsdk-1.3.6/cygwin/newlib
CC         =$(RSDK)/bin/rsdk-elf-gcc
ASM        =$(CC)
OBJDUMP    =$(RSDK)/bin/rsdk-elf-objdump
OBJCOPY    =$(RSDK)/bin/rsdk-elf-objcopy
STRIP      =$(RSDK)/bin/rsdk-elf-strip
LD         =$(RSDK)/bin/rsdk-elf-ld
IMG2B      =$(RSDK)/bin/img2bin
AR         =$(RSDK)/bin/rsdk-elf-ar
ROMNAME    =$(CONFIG_ROMNAME)
BINPREFIX  =/bin
HOST_OS    := $(shell uname -o)

ifeq ($(HOST_OS),Cygwin)
EXESUFFIX =.exe
else
EXESUFFIX =
endif

BUILD_NUMBER_FILE = .buildnum

APP_NAME=$(CONFIG_PRODUCT)

#########################################################################
# Path Setting for KConfig
#########################################################################
ROOTDIR       = $(shell pwd)
PATH	     := $(PATH):$(ROOTDIR)/tools
CONFIGTOOLDIR = $(ROOTDIR)/tools/config
SCRIPTSDIR    = $(CONFIGTOOLDIR)/kconfig
HOSTCC        = cc
	
ifeq (.config,$(wildcard .config))
include .config

ISEP       := $(shell [ $(CONFIG_VERSION) -ge 10 ] && echo true)
ISFP       := $(shell [ $(CONFIG_VERSION) -ge 15 ] && echo true)

VER_MAJOR= 1
ifeq ($(ISEP), true)
VER_MINOR= 3 
VER_EXTRA= 0 
else
VER_MINOR= 1 
VER_EXTRA= 6 
endif

ifeq ($(CONFIG_SOFTWARE_KVM_ENABLED), y)
VER_EXTRA = $(CONFIG_CHIPSET)+10
else ifeq ($(CONFIG_512KB_FLASH), y)
VER_EXTRA = $(CONFIG_CHIPSET)+20
else
VER_EXTRA = $(CONFIG_CHIPSET)
endif

ifeq ($(ISEP), true)
ARCH   = 4281
#FLASH_RLX  =./flashrlxep
	ifneq ($(CONFIG_BUILDROM), y)
		ifeq ($(CONFIG_VERSION), 10)		
			LIBDIR = dashlib/eplibts
		else
			LIBDIR = dashlib/eplib			
		endif
		ifeq ($(CONFIG_VERSION), 15)
			LIBDIR = dashlib/fplib
		endif		
	else
		LIBDIR = .
	endif
	#RSDK    = /lib/rsdk492
	#RSDK    = /lib/rsdk-4.9.2/cygwin/newlib
	 RSDK	 = /home/work/RSDK/rsdk-4.9.2/cygwin/newlib

else
	ARCH=4181
	LIBDIR = dashlib/dplib
	#FLASH_RLX  =./flashrlxdp
	RSDK   = /usr/local/rsdk-1.3.6/cygwin/newlib
endif

ifeq ($(CONFIG_BUILDROM), y)
all:  $(BUILD_NUMBER_FILE)  ./librom.a $(OBJ_DIR)/linker.ld $(OBJ_DIR)/$(APP_NAME)_dbg.exe 
else
all:  $(BUILD_NUMBER_FILE)  $(OBJ_DIR)/linker.ld $(OBJ_DIR)/mri $(OBJ_DIR)/$(APP_NAME)_dbg.exe 
endif
else
all: config_error
endif

ifeq ($(CONFIG_SMBIOS_RTK),y)
#SMBIOS     = smbios_no_user.bin
SMBIOS     = smbios.bin
else
SMBIOS     = smbios_asus.bin
endif



#SMBIOS     = smbios_no_user.bin
#ifeq ($(CONFIG_CHIPSET_AMDSOC),y)
#SMBIOS     = smbios_no_user.bin
#else
#SMBIOS     = smbios_with_asf.bin
#endif

ifeq ($(CONFIG_DEBUG), y)
#DEBUG=-gstabs+
DEBUG = -g
endif

############################################################################
#
# Config stuff, we recall ourselves to load the new config.arch before
# running the kernel and other config scripts
#

include $(CONFIGTOOLDIR)/Makefile.conf

SCRIPTS_BINARY_config     = conf
SCRIPTS_BINARY_menuconfig = mconf
SCRIPTS_BINARY_qconfig    = qconf
SCRIPTS_BINARY_gconfig    = gconf
SCRIPTS_BINARY_xconfig    = gconf
.PHONY: config menuconfig qconfig gconfig xconfig
menuconfig: mconf
qconfig: qconf
gconfig: gconf
xconfig: $(SCRIPTS_BINARY_xconfig)
config menuconfig qconfig gconfig xconfig: Kconfig conf
	$(SCRIPTSDIR)/$(SCRIPTS_BINARY_$@) Kconfig
	@if [ ! -f .config ]; then \
		echo; \
		echo "You have not saved your config, please re-run 'make $@'"; \
		echo; \
		exit 1; \
	 fi
	@chmod u+x $(CONFIGTOOLDIR)/setconfig
	@$(CONFIGTOOLDIR)/setconfig final

ifeq ($(CONFIG_BUILDROM), y)
LIBS  += -lrom
DPLIB := librom.a
else
#LIBS  += -lrom
endif

DFLAGS = #-save-temps
CFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -c -EL -G0 -Wall  
XFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -O1 -G0 -Wall  
ifeq ($(ISEP), true)
SFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -Os -G0 -Wall -mips16 -mno-data-in-code 
else
SFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -Os -G0 -Wall -mips16  
endif
ROMFLAGLC = -march=$(ARCH) $(DEBUG) -mlong-calls -fno-optimize-sibling-calls -fno-builtin -c -EL -Os -G0 -Wall
ROMFLAGO2 = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -O1 -G0 -Wall
#romapp will call ddrcode, and the offset address is larger than 26 bit
#ROMFLAGO2 = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -O1 -G0 -Wall -mlong-calls
ROMFLAGOS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -Os -G0 -Wall -mips16 -mno-data-in-code   
ROMFLAGNO = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -G0 -Wall  

ASFLAGS = $(CFLAGS) -I$(SRC_DIR)/include
DPFLAGS = -h -D -l --show-raw-insn -m mips:$(ARCH) -S
IFLAGS =  -I$(SRC_DIR)/include -I$(RSDK)/include
ifeq ($(CONFIG_BUILDROM), y)
LDFLAGS = -N -n -EL -G0 -L$(LIBDIR)
else
ifeq ($(CONFIG_VERSION), $(filter $(CONFIG_VERSION), 6 10 11 15))
LDFLAGS = -R $(LIBDIR)/dashrom.elf -N -n -EL -G0 -L$(LIBDIR)
else  
LDFLAGS = -c $(OBJ_DIR)/mri -N -n -EL -G0 -L$(LIBDIR)
endif
endif

ifeq ($(CONFIG_BUILDROM), y)
IFLAGS += -I$(ROMSRC_DIR)/include
endif

LDFLAGS  += --defsym __BUILD_DATE=$$(date +'%Y%m%d')
LDFLAGS  += --defsym __BUILD_NUMBER=$$(cat $(BUILD_NUMBER_FILE))
LDFLAGS  += --defsym __MAJOR_VER=$(VER_MAJOR)
LDFLAGS  += --defsym __MINOR_VER=$(VER_MINOR)
LDFLAGS  += --defsym __EXTRA_VER=$(VER_EXTRA)


#########################################################################
# Sources
#########################################################################


ifneq ($(ISEP), true)
ASM_SOURCE_APP_SPEC = crt0.S
endif

ifeq ($(CONFIG_VERSION), 2)
ASM_SOURCE_APP_SPEC +=  vb.S
endif

ifeq ($(CONFIG_VERSION), 1)
ASM_SOURCE_APP_SPEC +=  spi.S
endif

C_SOURCE_APP = const.c hook.c

C_SOURCE = init.c telnet.c  flash.c #log.c

#C_SOURCE_APP += 8168h.c PCIE_HOST.c


#C_OPTOBJS := $(addprefix $(OBJ_DIR)/, $(C_SOURCE:.c=_opt.o))
#C_SPSOBJS := $(addprefix $(OBJ_DIR)/, $(C_SOURCE_SPACE:.c=_sp.o))
#XFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -O1 -G0 -Wall  
#SFLAGS = -march=$(ARCH) $(DEBUG) -fno-optimize-sibling-calls -fno-builtin -c -EL -Os -G0 -Wall -mips16 -mno-data-in-code 
#$(OBJ_DIR)/%_opt.o: %.c
#	$(CC) $(XFLAGS) $(DFLAGS) $(IFLAGS)  $< -o $@
#$(OBJ_DIR)/%_sp.o: %.c
#	$(CC) $(SFLAGS) $(DFLAGS) $(IFLAGS)  $< -o $@

#for loading asf parameters 
C_SOURCE += asf.c
#C_SOURCE_SPACE += asf.c

C_SOURCE_SPACE = app.c   OOBMAC_COMMON.c

ifeq ($(CONFIG_SMBIOS_ENABLED), y)
C_SOURCE += genpage.c
endif

ifneq ($(CONFIG_SSH_ENABLED), )
C_SOURCE_SPACE += ssh.c	
endif

ifeq ($(CONFIG_DDR_ENABLED), y)
C_SOURCE += dram.c
endif

ifeq ($(CONFIG_VNC_ENABLED), y)
C_SOURCE += vnc.c des.c vga.c
endif

ifeq ($(CONFIG_DASH_ENABLED), y)
C_SOURCE_SPACE += dash.c client.c tcr.c
endif

ifeq ($(CONFIG_MAIL_WAKEUP_ENABLED), y)
C_SOURCE_SPACE += mail_wakeup.c 
endif

C_SOURCE_SPACE+=language.c

ifeq ($(CONFIG_HTTP_ENABLED), y)
C_SOURCE += post.c http.c tls.c #tls_client.c
ifeq ($(CONFIG_PRINTER_WEB_PAGE), y)
C_SOURCE += htm_p.c
endif
ifneq ($(CONFIG_PRINTER_WEB_PAGE), y)
C_SOURCE += htm.c
endif
endif

ifeq ($(CONFIG_mDNS_OFFLOAD_ENABLED), y)
C_SOURCE_SPACE += mDNS.c
endif

ifeq ($(CONFIG_TEREDO_ENABLED), y)
C_SOURCE_SPACE += teredo.c
endif

#When build ROM, it would skip the followng sources
ifneq ($(CONFIG_BUILDROM), y)

ifeq ($(CONFIG_PROVISIONING), y)
ifneq ($(ISEP), true)
C_SOURCE += genCSR.c soap.c
endif
endif

ifeq ($(CONFIG_DEBUG), y)
ifneq ($(ISEP), true)
C_SOURCE += sktprint.c
endif
endif

#IF NOT IC_VERSION_EP_RevB
ifneq ($(CONFIG_VERSION), 11)
C_SOURCE_SPACE += eventlog.c gmac.c   bsp.c sensor.c netlib.c
else
C_SOURCE_SPACE += gmac.c cmac.c bsp.c sensor.c
ifneq ($(ISFP), true)
C_SOURCE += rlx_cache.c
endif
endif

ifneq ($(ISEP), true)
C_SOURCE_SPACE += netbios.c # flash.c x509parse.c 
C_SOURCE_SPACE += md4.c 
endif

C_SOURCE_SPACE += # briank.rtk add 20140506
C_SOURCE_SPACE += x509parse.c 

ifeq ($(CONFIG_VERSION), 10)
C_SOURCE_SPACE += hwpf.c 
endif

ifeq ($(CONFIG_VERSION), 11)
C_SOURCE_SPACE += hwpf.c 
endif

#may be need for EPRevA
C_SOURCE_SPACE +=  lib.c arp.c rtskt.c ip.c tcp.c dhcp.c icmp.c


ifeq ($(CONFIG_CODEC_ENABLED), y)
ifneq ($(ISEP), true)
C_SOURCE += aes.c rsasw.c b64.c ssl_tls.c tlshw.c sha1.c md5.c
endif
endif

ifeq ($(CONFIG_USB_ENABLED), y)
ifneq ($(ISEP), true)
C_SOURCE_SPACE += usbdesc.c usb_ata.c
#usb_interrupt.c 
endif 
ifeq ($(CONFIG_VERSION), 10)
#for EP test chip, EHCI issue
C_SOURCE_SPACE += usb_interrupt.c ehci.c usb_ata.c
endif
ifeq ($(CONFIG_VERSION), 11)
#for RTL8111EP ES2 , EHCI issue
#C_SOURCE_SPACE += usb.c usb_interrupt.c ehci.c usb_ata.c
endif

ifeq ($(CONFIG_VERSION), 15)
#for RTL8111EP ES2 , EHCI issue
C_SOURCE_SPACE += usb.c  usb_ata.c usbdesc.c
#usb_interrupt.c
endif
endif


ifeq ($(CONFIG_UDP_ENABLED), y)
C_SOURCE_SPACE += udp.c
endif

ifeq ($(CONFIG_DASH_ENABLED), y)
C_SOURCE_SPACE += pldm.c
endif

ifeq ($(CONFIG_ASF_ENABLED), y)
ifneq ($(ISEP), true)
C_SOURCE_SPACE += snmp.c rmcp.c
endif
endif

ifeq ($(CONFIG_SOFTWARE_KVM_ENABLED), y)
#C_SOURCE_SPACE += snmp.c
endif

ifeq ($(CONFIG_8021X_ENABLED), y)
ifneq ($(CONFIG_VERSION), 10)
C_SOURCE_SPACE += eap.c des.c 
endif
endif

ifeq ($(CONFIG_MSAD_ENABLED), y)
ifneq ($(ISEP), true)
C_SOURCE_SPACE += kerberos.c gssapi.c spnego.c rc4.c hash.c krb_aes.c  
C_SOURCE += pac.c
endif
endif

else
C_SOURCE_SPACE +=  dummy.c  
endif

ifeq ($(CONFIG_UART_DEBUG), y)
ifeq ($(CONFIG_VERSION), 15)
C_SOURCE_SPACE += uart.c
endif
endif

C_SOURCE_SPACE += bsp.c usb.c  usbdevice_interrupt.c  usb_ata.c usbdesc.c parammd.c mmd.c mdio_bitbang.c testio_gpio.c i2c_gpio.c i2c_gpio_slave.c
ifeq ($(CONFIG_BUILDROM), y)
ROM_SOURCEO2 = os_cpu_c.c rlx_irq.c os_core.c os_flag.c os_mbox.c os_mem.c os_mutex.c os_q.c os_sem.c os_task.c os_time.c os_tmr.c
ROM_SOURCEOS = cmac.c rtskt.c tcp.c arp.c ip.c udp.c dhcp.c icmp.c netbios.c bsp.c hwpf.c tlshw.c rlx_cache.c testio.c dram.c flash_nand.c
#arp.c ip.c udp.c icmp.c netbios.c no change
#cmac.c hwpf.c fwupdate will use
#rtskt.c tcp.c  bsp.c need to check
#rtskt.c tcp.c will part in RAM
ROM_SOURCEO2 += fwupdate.c romapp.c flash_fp.c
#fwupdate.c romapp.c flash_ep.c fwupdate will use
ROM_SOURCEO2 += eventlog.c smbios.c
ROM_SOURCEO2 += rmcp.c snmp.c sensor.c 
ROM_SOURCEOS += b64.c sha1.c md5.c bignum.c rsa.c ssl_tls.c rsasw.c cert.c aes.c uart.c
#md4.c #NOT in EP rom
ROM_SOURCEO2 += pldm.c
ROM_SOURCEO2 += sktprint.c lib.c netlib.c gmac.c log.c
#ROM_SOURCEOS += kerberos.c gssapi.c spnego.c rc4.c hash.c krb_aes.c pac.c #NOT in EP rom
#ROM_SOURCEOS += x509parse.c eap.c genCSR.c soap.c #NOT in EP rom
#ROM_SOURCEO2 += usb.c ehci.c usb_ata.c usb_interrupt.c usbdesc.c #FP will put in RAM
ROM_SOURCENO =  
ROM_SOURCELC = finit.c 
ROMASM_SOURCE = crt0.S os_cpu_a.S os_cpu_traps_a.S
endif

ifneq ($(CONFIG_VERSION), 1)
C_SOURCE += wcom.c
endif

ifeq ($(CONFIG_VERSION), 10)
C_SOURCE += cmac.c    
endif

ASM_SOURCE = $(ASM_SOURCE_APP_SPEC)

LINK_SCRIPT=$(LINK_DIR)/$(APP_NAME).ld
LINK_MAP=$(APP_NAME).map

SOURCE_PATH=.:$(AP_DIR):$(SRC_DIR)
ifeq ($(CONFIG_BUILDROM), y)
SOURCE_PATH += $(ROMSRC_DIR)
endif

vpath %.c $(SOURCE_PATH)
vpath %.S $(SOURCE_PATH)
vpath %.s $(SOURCE_PATH)
vpath %.o .:$(OBJ_DIR)

#########################################################################
# Objects
#########################################################################
C_OBJS := $(addprefix $(OBJ_DIR)/, $(C_SOURCE_APP:.c=.o))
C_OPTOBJS := $(addprefix $(OBJ_DIR)/, $(C_SOURCE:.c=_opt.o))
C_SPSOBJS := $(addprefix $(OBJ_DIR)/, $(C_SOURCE_SPACE:.c=_sp.o))
ifeq ($(CONFIG_BUILDROM), y)
ROMOBJOS := $(addprefix $(ROMOBJ_DIR)/rom_s_, $(ROM_SOURCEOS:.c=.o))
ROMOBJNO := $(addprefix $(ROMOBJ_DIR)/rom_no_, $(ROM_SOURCENO:.c=.o))
ROMOBJO2   := $(addprefix $(ROMOBJ_DIR)/rom_, $(ROM_SOURCEO2:.c=.o))    
ROMOBJLC := $(addprefix $(ROMOBJ_DIR)/rom_l_, $(ROM_SOURCELC:.c=.o))
ASM_ROMOBJS := $(addprefix $(ROMOBJ_DIR)/rom_, $(ROMASM_SOURCE:.S=.o))
endif
ASM_OBJS := $(addprefix $(OBJ_DIR)/, $(ASM_SOURCE:.S=.o))

#########################################################################
# Rules
#########################################################################

.PHONY: all clean

$(ROMOBJ_DIR)/rom_l_%.o: %.c
	$(CC) $(ROMFLAGLC) $(DFLAGS) $(IFLAGS)  $< -o $@

$(ROMOBJ_DIR)/rom_s_%.o: %.c
	$(CC) $(ROMFLAGOS) $(DFLAGS) $(IFLAGS)  $< -o $@

$(ROMOBJ_DIR)/rom_%.o: %.c
	$(CC) $(ROMFLAGO2) $(DFLAGS) $(IFLAGS)  $< -o $@

$(ROMOBJ_DIR)/rom_no_%.o: %.c
	$(CC) $(ROMFLAGNO) $(DFLAGS) $(IFLAGS)  $< -o $@

$(OBJ_DIR)/%_opt.o: %.c
	$(CC) $(XFLAGS) $(DFLAGS) $(IFLAGS)   $< -o $@

$(OBJ_DIR)/%_sp.o: %.c
	$(CC) $(SFLAGS) $(DFLAGS) $(IFLAGS)  $< -o $@

$(OBJ_DIR)/%.o: %.c
	$(CC) $(CFLAGS) $(DFLAGS) $(IFLAGS)  $< -o $@

$(ROMOBJ_DIR)/rom_%.o: %.S
	$(ASM) $(ASFLAGS) $(DFLAGS) $(IFLAGS) $< -o $@

$(OBJ_DIR)/%.o: %.S
	$(ASM) $(ASFLAGS) $(DFLAGS) $(IFLAGS) $< -o $@
	
$(OBJ_DIR)/linker.ld: $(SRC_DIR)/linker.ld.c
	$(CC) -Xpreprocessor $(IFLAGS) -P -E  $< -o $@ 		

$(OBJ_DIR)/mri: $(SRC_DIR)/mri.c
	$(CC) -Xpreprocessor $(IFLAGS) -P -E  $< -o $@ 		
	
.PHONY: $(BUILD_NUMBER_FILE)

$(BUILD_NUMBER_FILE):  
#	@svn info | grep Revision | cut -d " "  -f 2 > $(BUILD_NUMBER_FILE)
	@date --date="now"  +"%y%j%H%M" > $(BUILD_NUMBER_FILE)

ifeq ($(CONFIG_BUILDROM), y)
$(DPLIB): $(ROMOBJOS) $(ROMOBJ16) $(ASM_ROMOBJS) $(ROMOBJO2) $(ROMOBJNO) $(ROMOBJLC)
	make -C $(ROMOBJ_DIR)
	$(AR) -rs $(DPLIB) $(ROMOBJ_DIR)/*.o
	$(LD) -EL -i $(ROMOBJ_DIR)/*.o -o allrom.o
endif

$(OBJ_DIR)/$(APP_NAME)_dbg.exe: $(C_SPSOBJS) $(C_OPTOBJS) $(C_OBJS) $(C_SPSOBJS) $(ASM_OBJS) 
#ifeq ($(CONFIG_8812AE_ENABLED), y) 
#	$(LD) wifi/obj/*.o $^ -T $(OBJ_DIR)/linker.ld -o $(OBJ_DIR)/$(APP_NAME)_dbg.exe $(LDFLAGS) -Map $(OBJ_DIR)/$(LINK_MAP) $(LIBS)
#else
	$(LD) $^ -T $(OBJ_DIR)/linker.ld -o $(OBJ_DIR)/$(APP_NAME)_dbg.exe $(LDFLAGS) -Map $(OBJ_DIR)/$(LINK_MAP) $(LIBS)
#endif
	$(OBJDUMP) $(DPFLAGS) $(OBJ_DIR)/$(APP_NAME)_dbg.exe > $(OBJ_DIR)/$(APP_NAME)-$(ARCH).s &
	$(STRIP) --remove-section=.comment $(OBJ_DIR)/$(APP_NAME)_dbg.exe -o $(OBJ_DIR)/$(APP_NAME).exe
	#$(STRIP) --remove-section=.text --remove-section=.rodata --remove-section=.data --remove-section=.init --remove-section=.sram $(OBJ_DIR)/$(APP_NAME)_dbg.exe -O binary $(OBJ_DIR)/$(APP_NAME)_NT.exe
	$(STRIP) --remove-section=.text --remove-section=.rodata --remove-section=.data --remove-section=.init --remove-section=.sram --remove-section=.conf --remove-section=.wake --remove-section=.language_en --remove-section=.language_2 $(OBJ_DIR)/$(APP_NAME)_dbg.exe -o $(OBJ_DIR)/$(APP_NAME)_NT.exe
	$(OBJDUMP) $(DPFLAGS) $(OBJ_DIR)/$(APP_NAME)_NT.exe > $(OBJ_DIR)/$(APP_NAME)-$(ARCH)_NT.s
ifeq ($(CONFIG_BUILDROM), y)
	$(OBJCOPY) --weaken $(OBJ_DIR)/$(APP_NAME)_dbg.exe dashrom.elf
endif
#	$(OBJCOPY) --only-keep-debug dashrom.elf $(OBJ_DIR)/dashrom.dbg
#	$(OBJCOPY) --add-gnu-debuglink $(OBJ_DIR)/dashrom.dbg $(OBJ_DIR)/$(APP_NAME)_dbg.exe
#	$(OBJCOPY) --add-gnu-debuglink dashrom.elf $(OBJ_DIR)/$(APP_NAME)_dbg.exe

ifeq ($(CONFIG_BUILDROM), y)
rom: $(OBJ_DIR)/$(APP_NAME).exe
	$(OBJCOPY) -O binary -j.boot $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/bootrom.img 	
	$(OBJCOPY) -O binary -j.romrodata -j.romdata $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/bootrom_data.img 
	$(OBJCOPY) -O binary -j.boot -j.romrodata -j.romdata $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/boot.img 
	$(OBJCOPY) -O binary -j.irom $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/irom.img 
	$(OBJCOPY) -O binary -j .boot -j.romrodata -j.romdata -j.irom $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/irom.0x800b0000.img 
	$(IMG2B) $(OBJ_DIR)/bootrom_data.img $(OBJ_DIR)/bootrom_data.bin
	$(IMG2B) $(OBJ_DIR)/bootrom.img $(OBJ_DIR)/bootrom.bin
	$(IMG2B) $(OBJ_DIR)/boot.img $(OBJ_DIR)/boot.bin
	$(IMG2B) $(OBJ_DIR)/irom.img $(OBJ_DIR)/irom.bin
	$(IMG2B) $(OBJ_DIR)/irom.0x800b0000.img  $(OBJ_DIR)/l2rom.bin
	split -d -b 128k $(OBJ_DIR)/irom.img
	$(BINPREFIX)/cp $(OBJ_DIR)/boot.img romflash/boot.img
	$(BINPREFIX)/cp $(OBJ_DIR)/irom.img romflash/irom.img
	$(BINPREFIX)/cp $(OBJ_DIR)/boot.img $(FLASH_RLX)/boot.img
	$(BINPREFIX)/cp $(OBJ_DIR)/irom.img $(FLASH_RLX)/irom.img
	$(BINPREFIX)/cp $(OBJ_DIR)/bootrom_data.img $(FLASH_RLX)/bootrom_data.img
	$(BINPREFIX)/mv ./x00 romflash/irom1.img
	@if test -f ./x01; then $(BINPREFIX)/mv ./x01 romflash/irom2.img; fi
	echo $(CONFIG_VERSION) > $(FLASH_RLX)/.chip
	$(OBJCOPY) -O binary -j.conf $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_DIR)/conf.img	
endif

rom-boot : $(OBJ_DIR)/$(APP_NAME).exe 
ifneq ($(CONFIG_VERSION), 1)
	$(OBJCOPY) -O binary -j.init -j.sram -j.text -j.rodata -j.data $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/all.img
#$(OBJCOPY) -O binary -j.boot -j.init -j.sram -j.text -j.rodata -j.data $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/all.img
else
	$(OBJCOPY) -O binary -j.boot -j.init -j.sram -j.text -j.rodata -j.data -j.romrodata -j.romdata $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/all.img	
endif
ifeq ($(CONFIG_WAKE_UP_PATTERN_ENABLED), y)
	$(OBJCOPY) -O binary -j.conf -j.wake $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_DIR)/conf.img
else
	$(OBJCOPY) -O binary -j.conf $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_DIR)/conf.img
endif
#--writable-text


	$(OBJCOPY) -O binary -j.language_en $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_DIR)/language1.img
	$(OBJCOPY) -O binary -j.language_2 $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_DIR)/language2.img	
	$(OBJCOPY) -O binary -j.language_en $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_RLX)/language1.img
	$(OBJCOPY) -O binary -j.language_2 $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_RLX)/language2.img	
	$(OBJCOPY) -O binary  -j.language_en  -j.language_2  $(OBJ_DIR)/$(APP_NAME).exe $(FLASH_RLX)/language.img

	split --suffix-length=6 -d -b 128k $(OBJ_DIR)/all.img
	$(BINPREFIX)/mv ./x000000 $(FLASH_DIR)/1.img
	@if test -f ./x000001; then $(BINPREFIX)/mv ./x000001 $(FLASH_DIR)/2.img; fi
	@if test -f ./x000002; then $(BINPREFIX)/mv ./x000002 $(FLASH_DIR)/3.img; fi
	$(BINPREFIX)/mv $(OBJ_DIR)/all.img $(FLASH_DIR)/all.img
	$(BINPREFIX)/cp $(FLASH_DIR)/all.img $(FLASH_RLX)/all.img

ifeq ($(CONFIG_SIMPLE_BACKUP), y) 
	#briank.test 20140609
	$(FLASH_DIR)/genconf_bc $(FLASH_DIR)/conf.img $(FLASH_DIR)/$(SMBIOS) $(FLASH_DIR)/cert$(SUFFIX).img $(FLASH_DIR)/pldm.img $(FLASH_DIR)/newconf.img	
else
	$(FLASH_DIR)/genconf $(FLASH_DIR)/conf.img $(FLASH_DIR)/$(SMBIOS) $(FLASH_DIR)/cert$(SUFFIX).img $(FLASH_DIR)/pldm.img $(FLASH_DIR)/newconf.img 
endif	
	$(BINPREFIX)/cp $(FLASH_DIR)/newconf.img $(FLASH_RLX)/newconf.img
	#$(FLASH_DIR)/padrom $(FLASH_DIR)/newconf.img $(FLASH_DIR)/all.img $(FLASH_DIR)/romdata.bin $(ROMNAME)

ifeq ($(CONFIG_MULTILANGUAGE_ENABLED), y) 
	$(FLASH_DIR)/padrom_m.exe $(FLASH_DIR)/newconf.img $(FLASH_DIR)/all.img $(FLASH_DIR)/language1.img $(FLASH_DIR)/language2.img $(ROMNAME)
else
	$(FLASH_DIR)/padrom $(FLASH_DIR)/newconf.img $(FLASH_DIR)/all.img $(FLASH_DIR)/language1.img $(ROMNAME) 
endif

	./crc32$(EXESUFFIX) $(ROMNAME) 65536 458752 16
ifeq ($(CONFIG_USB_FLASH), y)
	cat $(ROMNAME) $(FLASH_DIR)/usbimage.bin > usbboot.bin
endif
	echo $(CONFIG_VERSION) > $(FLASH_RLX)/.chip

pgrom:  rom-boot

bin:  $(OBJ_DIR)/$(APP_NAME).exe
	$(OBJCOPY) -O binary -j.boot $(OBJ_DIR)/$(APP_NAME).exe $(OBJ_DIR)/$(APP_NAME).img 
	$(IMG2B)  $(OBJ_DIR)/$(APP_NAME).img $(OBJ_DIR)/$(APP_NAME).bin
	
lang :
ifeq ($(CONFIG_MULTILANGUAGE_ENABLED), y) 
	$(FLASH_DIR)/padrom_m.exe $(FLASH_DIR)/newconf.img $(FLASH_DIR)/all.img $(FLASH_DIR)/language1.img $(FLASH_DIR)/language2.img $(ROMNAME)
else
	$(FLASH_DIR)/padrom $(FLASH_DIR)/newconf.img $(FLASH_DIR)/all.img $(FLASH_DIR)/language1.img $(ROMNAME) 
endif

clean:
	rm -rf $(OBJ_DIR)/*
ifeq ($(CONFIG_BUILDROM), y)
	rm -rf $(ROMOBJ_DIR)/*.o
	rm $(DPLIB)
endif
	
config_error:
	@echo "******************************************************"
	@echo "You have not run make config."
	@echo "The build sequence for this source tree is:"
	@echo "1. 'make menuconfig', 'make config' or 'make xconfig'"
	@echo "2. 'make clean'"
	@echo "3. 'make'"
	@echo "******************************************************"
	@exit 1	

#########################################################################
# End
#########################################################################
