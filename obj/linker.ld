ENTRY(_boot)
MEMORY
{
 sram : o = 0x800C0000, l = 155k
 boot : o = 0x9fc00000, l = 16k
    flash : o = 0x82030000, l = 768k
    conf : o = 0x82020000, l = 64k
    wake : o = 0x82022000, l = 8k
 ddrdata : o = 0x88800000, l = 4M
 ddrcode : o = 0x88c00000, l = 4M
 VGA_ROM : o = 0x82000000, l = 20K
 VGA_DDR : o = 0x88000000, l = 8M
    language_en : o = 0x820F0000, l = 32k
    language_2 : o = 0x820F0000 +0x8000, l = 32k
}
SECTIONS
{
  .conf : {
   *(.conf);
  } > conf
  .wake : {
   *(.wake);
  } > wake
  .boot : {
   *(.boot);
  } > boot
 .language_en : {
   *(.language_en);
  } > language_en
   .language_2 : {
   *(.language_2);
  } > language_2
 .imem0 : {
     *(.trap_handler);
 } > imem0 AT> irom
  .init : {
         obj/init*.o(*.text);
         *(.init);
  } > flash
  .sram : {
obj/8168h*.o(.text);
   obj/vga*.o(.text);
   obj/vnc*.o(.text);
   obj/des*.o(.text);
 obj/smbus*.o(.text);
     obj/telnet*.o(.text);
     obj/usb*.o(.text);
     obj/ssh*.o(.text);
     obj/client*.o(.text);
     obj/icmp*.o(.text);
     obj/udp*.o(.text);
     obj/snmp*.o(.text);
     *(.sram);
 obj/parammd*.o(.text);
 obj/mmd*.o(.text);
 obj/mdio_bitbang*.o(.text);
 obj/testio_gpio*.o(.text);
 obj/i2c_gpio*.o(.text);
  } > sram AT> flash
  .ddr : {
  } > ddr
  __sram_start_lma = LOADADDR(.sram);
  __sram_start_vma = ADDR(.sram);
  __sram_size = SIZEOF(.sram);
  .text : {
 *(.text);
 *(.text.startup);
  } > flash
  __text_start_lma = LOADADDR(.text);
  __text_start_vma = ADDR(.text);
  __text_size = SIZEOF(.text);
  .rodata : { . = ALIGN(4) ;
          *(.roexcept);
   *(.rodata .rodata.* .gnu.linkonce.r.*) ;
                . = ALIGN(4);
  } > flash
  .data : {
     . = ALIGN(4);
     *(.data) *(.sdata) *(.lit4) *(.lit8) . = ALIGN(4);
   } > sram AT> flash
  __data_start_lma = LOADADDR(.data);
  __data_start_vma = ADDR(.data);
  __data_size = SIZEOF(.data);
  .bss : {
        __bss_start = .;
        *(.sbss) *(.bss) *(.scommon) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > sram
  .eh_frame 0x80500000 : {*(.eh_frame)}
  .rel.dyn 0 : { *(.rel.dyn)}
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
  .debug 0 : { *(.debug) }
  .line 0 : { *(.line) }
  .debug_srcinfo 0 : { *(.debug_srcinfo) }
  .debug_sfnames 0 : { *(.debug_sfnames) }
  .debug_aranges 0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  .debug_info 0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev 0 : { *(.debug_abbrev) }
  .debug_line 0 : { *(.debug_line) }
  .debug_frame 0 : { *(.debug_frame) }
  .debug_str 0 : { *(.debug_str) }
  .debug_loc 0 : { *(.debug_loc) }
  .debug_macinfo 0 : { *(.debug_macinfo) }
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames 0 : { *(.debug_varnames) }
  .gptab.sdata : { *(.gptab.data) *(.gptab.sdata) }
  .gptab.sbss : { *(.gptab.bss) *(.gptab.sbss) }
  .mdebug.abi32 : { KEEP(*(.mdebug.abi32)) }
  .mdebug.abiN32 : { KEEP(*(.mdebug.abiN32)) }
  .mdebug.abi64 : { KEEP(*(.mdebug.abi64)) }
  .mdebug.abiO64 : { KEEP(*(.mdebug.abiO64)) }
  .mdebug.eabi32 : { KEEP(*(.mdebug.eabi32)) }
  .mdebug.eabi64 : { KEEP(*(.mdebug.eabi64)) }
  .gcc_compiled_long32 : { KEEP(*(.gcc_compiled_long32)) }
  .gcc_compiled_long64 : { KEEP(*(.gcc_compiled_long64)) }
  /DISCARD/ : { *(.note.GNU-stack) }
}
