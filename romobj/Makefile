#

#RSDK    := /usr/local/rsdk
#RSDK      = /cygdrive/c/rsdk
RSDK       = /lib/rsdk492/cygwin/newlib
AR      := $(RSDK)/bin/rsdk-elf-ar
OBJCOPY := $(RSDK)/bin/rsdk-elf-objcopy

libcobj := lib_a-assert.o lib_a-atoi.o lib_a-ctime.o lib_a-ctype_.o \
lib_a-fgets.o lib_a-findfp.o lib_a-fiprintf.o lib_a-fwalk.o lib_a-fwrite.o \
lib_a-impure.o lib_a-lcltime.o lib_a-lcltime_r.o lib_a-memchr.o lib_a-memcmp.o \
lib_a-memcpy.o lib_a-memset.o lib_a-mktime.o lib_a-mktm_r.o lib_a-printf.o \
lib_a-rand.o lib_a-refill.o lib_a-snprintf.o lib_a-sprintf.o lib_a-sscanf.o \
lib_a-stdio.o lib_a-strcat.o lib_a-strchr.o lib_a-strcmp.o lib_a-strcpy.o \
lib_a-strftime.o lib_a-strlen.o lib_a-strncat.o lib_a-strncmp.o \
lib_a-strncpy.o lib_a-strpbrk.o lib_a-strrchr.o lib_a-strstr.o lib_a-strtok.o \
lib_a-strtok_r.o lib_a-strtol.o lib_a-sysgettod.o lib_a-sysopen.o lib_a-time.o \
lib_a-tzlock.o lib_a-tzvars.o lib_a-vfiprintf.o lib_a-vfprintf.o \
lib_a-vfscanf.o lib_a-wcrtomb.o lib_a-wcsrtombs.o lib_a-wctomb_r.o \
lib_a-wsetup.o lib_a-abort.o lib_a-abs.o lib_a-asctime.o lib_a-asctime_r.o \
lib_a-div.o lib_a-fclose.o lib_a-fflush.o lib_a-fread.o lib_a-fvwrite.o \
lib_a-gettzinfo.o lib_a-iswspace.o lib_a-labs.o lib_a-locale.o lib_a-makebuf.o \
lib_a-mbrtowc.o lib_a-mbtowc_r.o lib_a-memmove.o lib_a-sccl.o lib_a-signal.o \
lib_a-strtoll_r.o lib_a-strtoul.o lib_a-strtoull_r.o lib_a-ungetc.o \
lib_a-putchar_u.o lib_a-putchar.o lib_a-putc.o lib_a-wbuf.o lib_a-mempcpy.o\
lib_a-memccpy.o lib_a-fputs.o lib_a-puts.o lib_a-bcopy.o lib_a-gettimeofdayr.o\
lib_a-svfscanf.o lib_a-svfprintf.o
libsysobj := close.o fstat.o getpid.o kill.o lseek.o open.o read.o \
	     isatty.o write.o _exit.o mem.o peri.o gettod.o
librlxobj := rlx_cp3_asm_taroko.o rlx_cp3_taroko.o
libgccobj := _udivdi3.o _umoddi3.o _clz.o


all: 
	for i in $(libcobj); do $(AR) -x ../libc.a $$i; done  
	for i in $(libsysobj); do $(AR) -x ../libsys.a $$i; done
	for i in $(librlxobj); do $(AR) -x ../librlx.a $$i; done
	for i in $(libgccobj); do $(AR) -x ../libgcc.a $$i; done
#peri.o define some uart functions as same in uart.c
	$(OBJCOPY) --redefine-sym uart_close=uart_close_sys	peri.o
	$(OBJCOPY) --redefine-sym uart_read=uart_read_sys	peri.o
	$(OBJCOPY) --redefine-sym uart_write=uart_write_sys peri.o