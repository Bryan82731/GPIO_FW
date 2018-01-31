#########################################################################
# Path Setting
#########################################################################
APP_NAME=flashprog
#RSDK= /cygdrive/c/rsdk
#RSDK = /usr/local/rsdk-1.3.7/cygwin/newlib
RSDK  = /lib/rsdk492/cygwin/newlib
LIBS= -lc -lgcc

#########################################################################
# MPU specuific arguments
#########################################################################
ifeq (.chip,$(wildcard .chip))
CHIP_VERSION := $(shell cat .chip) 
ISEP       := $(shell [ $(CHIP_VERSION) -ge 10 ] && echo true)

ifeq ($(ISEP), true)
DNAME = -DRTL8111EP
else
DNAME = -DRTL8111DP
endif

ifeq ($(ISEP), true)
ARCH = 4281
#$(info 4281!)
else
ARCH = 4181
endif


APP_NAME := $(APP_NAME)-$(ARCH)

CC=$(RSDK)/bin/rsdk-elf-gcc
ASM=$(CC)
OBJDUMP=$(RSDK)/bin/rsdk-elf-objdump
OBJCOPY=$(RSDK)/bin/rsdk-elf-objcopy
STRIP=$(RSDK)/bin/rsdk-elf-strip
LD=$(RSDK)/bin/rsdk-elf-ld
IMG2B=$(RSDK)/bin/img2bin

LIBDIR = -L.
CFLAGS = -march=$(ARCH) -g -c -EL -G0 -Wall -I. $(DNAME)
ASFLAGS = $(CFLAGS) -I.
LDFLAGS = -N -n -EL -G0 $(LIBDIR)
DFLAGS = -h -D -l --show-raw-insn -m mips:$(ARCH) -S
IFLAGS = -I. 


#########################################################################
# Sources
#########################################################################

C_SOURCE = app.c rlx_cache.c 

ifeq ($(ISEP), true)
C_SOURCE += flash_fp.c
endif

ASM_SOURCE = crt0.S  

ifneq ($(ISEP), true)
ASM_SOURCE += spi.S
endif

else
all: config_error
endif

LINK_MAP=$(APP_NAME).map

SOURCE_PATH=.:$(AP_DIR):$(PORT_DIR):$(OS_DIR):$(EVB_DIR)
vpath %.c $(SOURCE_PATH)
vpath %.S $(SOURCE_PATH)
vpath %.s $(SOURCE_PATH)

vpath %.o .
#########################################################################
# Objects
#########################################################################
C_OBJS :=  $(C_SOURCE:.c=.o)
ASM_OBJS :=  $(ASM_SOURCE:.S=.o)


#########################################################################
# Rules
#########################################################################
%.o: %.c
	$(CC) $(CFLAGS) $(IFLAGS)  $< -o $@

%.o: %.S
	$(ASM) $(ASFLAGS) $(IFLAGS) $< -o $@

$(APP_NAME)_dbg.exe: $(C_OBJS) $(ASM_OBJS) linker.ld
	$(LD) $^ -T linker.ld  -o $(APP_NAME)_dbg.exe $(LDFLAGS) -Map $(LINK_MAP) $(LIBS)
	$(OBJDUMP) $(DFLAGS) $(APP_NAME)_dbg.exe > $(APP_NAME).s
	$(STRIP) --remove-section=.comment $(APP_NAME)_dbg.exe -o $(APP_NAME).exe
	gcc -O2 -o padrom padrom.c
	gcc -O2 -o genconf genconf.c

linker.ld: linker.ld.c
	$(CC) -Xpreprocessor -P -E $(DNAME) $< -o $@

usb-file:    
	cp app_usb_flash.c app.c
	cp startice_usb_flash.cmd startice.cmd

rom-file:   
	cp app_normal.c app.c
	cp startice_normal.cmd startice.cmd

usb:  usb-file $(APP_NAME)_dbg.exe

rom:  rom-file $(APP_NAME)_dbg.exe

all:  $(APP_NAME)_dbg.exe 

padrom: padrom.c
	gcc -O2 -o padrom padrom.c

bin:  $(APP_NAME).exe
	$(OBJCOPY) -O binary $(APP_NAME).exe $(APP_NAME).img 
	$(IMG2B)  $(APP_NAME).img $(APP_NAME).bin

clean:
	rm -rf *.o
	rm -rf linker.ld

config_error:
	@echo "******************************************************"
	@echo "You have not compiled a new rom."
	@echo "The build sequence for the upper source tree is:"
	@echo "1. 'make menuconfig', 'make config' or 'make xconfig'"
	@echo "2. 'make clean'"
	@echo "3. 'make'"
	@echo "******************************************************"
	@exit 1	


#########################################################################
# End

