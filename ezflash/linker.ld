ENTRY(_boot)

MEMORY
{
	sram  :  o = 0x80200000, l = 16k  /* 128KB  0x80240000*/
        dmem  :  o = 0x80100200, l = 4k    /* 256KB  0x80140000*/
        rom   :  o = 0x80300000, l = 256k  /* 256KB  0x80340000*/
	/* DMEM: 0x80100000 - 0x8010FFFF */
        flash   : o = 0x80400000, l = 512k /* 512KB  0x80308000*/
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
