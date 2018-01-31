ENTRY(_boot)

MEMORY
{
	sram  :  o = 0x800C0000, l = 64k
}

SECTIONS
{

  .boot  : {
     *(.boot); 
     _end_boot = .;
   } >  sram

  /* program address : AT(load address) */

  .text . : {

		__text_begin = .;
		*(.text);
		__text_end = .;
  } > sram

  .rodata  .       : { *(.rodata .rodata.* .gnu.linkonce.r.*) } > sram
  .rodata1 .       : { *(.rodata1)  } > sram


  .data  : {
      __data_begin = .;
     *(.data) *(.sdata) *(.lit4) *(.lit8) . = ALIGN(4); 
      __data_end = .; 
   } > sram


  _gp = ALIGN(16) ;

  .bss  : {
        . = ALIGN(4);
        __bss_start = .;
        *(.sbss) *(.bss) *(.scommon) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > sram

}
