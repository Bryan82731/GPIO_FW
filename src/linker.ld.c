#include <sys/dpdef.h>
#include <sys/autoconf.h>

ENTRY(_boot)
#ifdef CONFIG_BUILDROM
EXTERN(rlx_trap_handler)
#endif

//sram -> L2MEM(RAM) code .text
//sram1 -> L2MEM(RAM) romdata
//stk -> L2MEM(RAM) 
//all dmem is for hw descriptor and heap use
MEMORY
{
#if   CONFIG_VERSION == 99
	//L2MEM
//	boot   :  o = 0x800C0000, l = 32k	//L2MEM ROM, boot code
//	boot   :  o = 0x80080000, l = 32k	//L2MEM ROM, boot code  modified by melody 20161215
    sram   :  o = 0x800C0000, l = 150k	//0x80080000~0x800BFFFF --L2MEM(ROM)/0x800C0000-0x800EFFFF L2MEM(RAM);sram + stk + sram1 = 192KB
	//IMEM
	imem0	:	o = 0x80000000, l = 1k
	irom   :  o = 0x80088000, l = 224k	//L2MEM ROM, ROM instruction	
	//DMEM
	dmem0	:	o = 0x80040000, l = 256k
#elif CONFIG_VERSION >=  IC_VERSION_FP_RevA   
	sram   :  o = 0x800C0000, l = 155k		//Use L2MEM	
#endif	


//FP L2MEM ROM = EP IMEM0(ROM/irom) + IMEM1(ROM/boot)
#if defined(CONFIG_BUILDROM)
	boot   :  o = 0x80080000, l = 32k	//L2MEM ROM, boot code
#elif (CONFIG_VERSION == IC_VERSION_EP_RevB)    
	boot   :  o = 0x8A020000, l = 32k    
	//boot   :  o = 0x8B010000, l = 32k    
	//boot   :  o = 0x88000000, l = 32k    
	//bootd  :  o = 0x8B018000, l = 32k
#elif CONFIG_VERSION ==  IC_VERSION_EP_RevA    
	boot   :  o = 0x88000000, l = 32k    
	bootd  :  o = 0x8B010000, l = 64k
#elif CONFIG_VERSION > IC_VERSION_DP_RevA 
	boot   :  o = 0x9fc00000, l = 16k    
#endif

#ifdef CONFIG_BUILDROM	
	irom   :  o = 0x80088000, l = 224k	//L2MEM ROM, ROM instruction
    sram1  :  o = 0x800E6C00, l = 37k  	//Rom data and stk in L2RAM(Cacheable)
    stk    :  o = 0x800DE000, l = 35k	    //stk only form ROM task use
#endif

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
#elif CONFIG_VERSION >=  IC_VERSION_FP_RevA
    flash  :  o = 0x82030000, l = 768k
    conf   :  o = 0x82020000, l = 64k 
    wake   :  o = 0x82022000, l =  8k 
	ddrdata	   :  o =  0x88800000, l =  4M
	ddrcode	   :  o =  0x88c00000, l =  4M
	//Reserved
	VGA_ROM  :  o = 0x82000000, l = 20K //use unused flash region, temp solution
	VGA_DDR	   :  o =  0x88000000, l =  8M //for VGA 
#else
#endif

    language_en  :  o = LANGUAGE_ADDRESS, l =  32k
    language_2   :  o = LANGUAGE_ADDRESS+LANGUAGE_SIZE, l =  32k


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
#if CONFIG_VERSION == IC_VERSION_DP_RevA
          *(.init);
          *(.bootrom);
  } > flash
#else
  } > boot 
#endif

 .language_en : {
   *(.language_en);
  } > language_en

   .language_2 : {
   *(.language_2);
  } > language_2
 


#ifdef CONFIG_BUILDROM 
  .irom :{
	 __trap_handler_start_lma = .;
     *(.trap_handler);
	 __trap_handler_end_lma = .;
     *(.irom);
     *(.rlxprof_text);
     *(.romtext);
     *rom_*.o(*.text);
     librom.a(.text);
  } > irom 
#endif

	.imem0 : {
     *(.trap_handler);
	} > imem0  AT> irom
	
#if CONFIG_VERSION  > IC_VERSION_DP_RevA
  .init : {
#if CONFIG_VERSION == IC_VERSION_DP_RevB
	 obj/vb.o(*.text); 
#endif
         obj/init*.o(*.text);
         *(.init);
  } > flash
#endif

  .sram : {
obj/8168h*.o(.text);
  	obj/vga*.o(.text);
  	obj/vnc*.o(.text);
  	obj/des*.o(.text);
	obj/smbus*.o(.text);
	 #if 0
     obj/flash*.o(.text);
     //obj/app*.o(.text);
     obj/wcom*.o(.text);
     obj/tcr*.o(.text);
     obj/cmac*.o(.text);
     obj/gmac*.o(.text);
	 #endif

#if !defined(CONFIG_REMOTE_DEBUG) || (CONFIG_VERSION >= IC_VERSION_DEP_RevA) 
     obj/telnet*.o(.text);
     obj/usb*.o(.text);
#if  CONFIG_VERSION > IC_VERSION_DP_RevF     
     #if  CONFIG_VERSION < IC_VERSION_EP_RevB
     obj/ehci*.o(.text);
     obj/hwpf*.o(.text);
     //obj/app*.o(.text);
     obj/lib*.o(.text);
     obj/pldm*.o(.text);
     obj/bsp*.o(.text); 
     obj/tcp*.o(.text);
     obj/rtskt*.o(.text);
     obj/ip*.o(.text);
     obj/arp*.o(.text);
     #endif
#if  CONFIG_VERSION > IC_VERSION_EP_RevA
     //obj/sensor*.o(.text);
     //obj/asf*.o(.text);
#ifndef CONFIG_MAIL_WAKEUP_ENABLED
     obj/ssh*.o(.text);
     obj/client*.o(.text);
     obj/icmp*.o(.text);
     obj/udp*.o(.text);
     obj/snmp*.o(.text);
#endif
#endif
#endif
#endif
     *(.sram);
	obj/parammd*.o(.text);
	obj/mmd*.o(.text);
	obj/mdio_bitbang*.o(.text);
	obj/testio_gpio*.o(.text);
	obj/i2c_gpio*.o(.text);
	
  } > sram AT> flash

  .ddr	: {
//	  obj/ddrcode_opt.o
  } > ddr
  __sram_start_lma = LOADADDR(.sram);
  __sram_start_vma = ADDR(.sram);
  __sram_size = SIZEOF(.sram);

  .text :  {
#if CONFIG_VERSION == IC_VERSION_DP_RevA
        *(.trap);
#endif
	*(.text);	
	*(.text.startup);
  } > flash
  
  __text_start_lma = LOADADDR(.text);
  __text_start_vma = ADDR(.text);
  __text_size = SIZEOF(.text);

#if CONFIG_VERSION == IC_VERSION_DP_RevA
  .romrodata 0x81070000 : {
          . = ALIGN(4);
   *(.romrodata);
  } > flash7

  .romdata : {
          . = ALIGN(4);
   *(.romdata);
  } > sram1 AT > flash8
#endif

#ifdef CONFIG_BUILDROM

  .stk : {
      . = ALIGN(4);
      __romstk_start = . ;
      *(.stk);
      __romstk_end   = . ;
      . = ALIGN(4);
  } > stk

  .romrodata : {
          . = ALIGN(4);
	  *(.romrodata);
          librom.a(.rodata .rodata.* .gnu.linkonce.r.*) ; 
	  *(.broms);
          . = ALIGN(4);
  } > boot

  .romdata : {
          . = ALIGN(4);
	  *(.romdata);
	  librom.a(.sdata .data .lit4 .lit8);
          *(.rlxprof_data);
          . = ALIGN(4);
  } > sram1 AT > boot

  _gp = ALIGN(16) ;

  __romdata_start_lma = LOADADDR(.romdata);
  __romdata_start_vma = ADDR(.romdata);
  __romdata_size = SIZEOF(.romdata);

  .rombss : {
	  . = ALIGN(4);
          *(.bss.stack);
          *(.bss.bssx);
	  __rombss_start  = . ;
	  *(.bss.roms);
          librom.a(.sbss .bss .scommon COMMON);
	  __rombss_end = .;
	  . = ALIGN(4);
  } > sram1
#endif

  .rodata : { . = ALIGN(4) ;
          *(.roexcept);
	  *(.rodata .rodata.* .gnu.linkonce.r.*) ; 
                . = ALIGN(4);
  } >  flash

  .data : {
#if CONFIG_VERSION == IC_VERSION_DP_RevA
     *(.rom) . = ALIGN(4);
#else
     . = ALIGN(4);
#endif
     *(.data) *(.sdata) *(.lit4) *(.lit8) . = ALIGN(4); 
   } > sram AT> flash

  __data_start_lma = LOADADDR(.data);
  __data_start_vma = ADDR(.data);
  __data_size = SIZEOF(.data);
   
  .bss  : {
#if CONFIG_VERSION <= IC_VERSION_DP_RevB
        . = ALIGN(4);
        *(.bssx)
#endif
        __bss_start = .;
        *(.sbss) *(.bss) *(.scommon) *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    #ifdef CONFIG_DDR_ENABLED
    } > ddrdata
    #else
    } > sram
    #endif
    
  .eh_frame   0x80500000 : {*(.eh_frame)}   
  .rel.dyn        0 : { *(.rel.dyn)}

  /* Stabs debugging sections.  */
  .stab          0 : { *(.stab) }
  .stabstr       0 : { *(.stabstr) }
  .stab.excl     0 : { *(.stab.excl) }
  .stab.exclstr  0 : { *(.stab.exclstr) }
  .stab.index    0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment       0 : { *(.comment) }

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) }
  .debug_funcnames 0 : { *(.debug_funcnames) }
  .debug_typenames 0 : { *(.debug_typenames) }
  .debug_varnames  0 : { *(.debug_varnames) }
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
