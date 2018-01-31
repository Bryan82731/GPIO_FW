#ifndef _DPDEF_H
#define _DPDEF_H
#include "autoconf.h"

#define IC_VERSION_DP_RevA 1
#define IC_VERSION_DP_RevB 2
#define IC_VERSION_DP_RevD 4
#define IC_VERSION_DP_RevF 6
#define IC_VERSION_EP_RevA 10
#define IC_VERSION_EP_RevB 11

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
#define LANGUAGE_ADDRESS 0x81070000
#else
	#ifdef CONFIG_512KB_FLASH
		#define LANGUAGE_ADDRESS 0x82010000
	#else
		#define LANGUAGE_ADDRESS 0x820F0000
	#endif
#endif

#define LANGUAGE_SIZE 0x8000 // For each Language Sector.

//DP Revf
//Hw Feature:Port Filter, Support Single/Dual IP mode but do not fixed DHCP issue
//Rom Code: lib.c arp.c ip.c tcp.c rtskt.c

#ifndef CONFIG_BUILDROM
#define _ATTRIBUTE_INIT   __attribute__((section(".init")))
#define _ATTRIBUTE_TRAP   __attribute__((section(".sram")))
#define _ATTRIBUTE_SRAM   __attribute__((section(".sram")))
#define _ATTRIBUTE_EXCEPT __attribute__((section(".sram")))
#define _ATTRIBUTE_IROM  
#define _ATTRIBUTE_STK  
#define _ATTRIBUTE_ROMDATA 
#define _ATTRIBUTE_ROMRODATA 
#define _ATTRIBUTE_FAR_CALL   
#define _ATTRIBUTE_ROEXCEPT 
#define CONST const
#define _ATTRIBUTE_WAKE  __attribute__((section(".wake")))
#define _ATTRIBUTE_ROM_BSS  
#else
#define _ATTRIBUTE_INIT   __attribute__((section(".romtext")))
#define _ATTRIBUTE_EXCEPT __attribute__((section(".sram")))
#define _ATTRIBUTE_TRAP   
#define _ATTRIBUTE_SRAM   
#define _ATTRIBUTE_STK    __attribute__((section(".stk")))
#define _ATTRIBUTE_IROM   __attribute__((section(".irom")))
#define _ATTRIBUTE_ROMDATA __attribute__((section(".romdata")))
#define _ATTRIBUTE_ROMRODATA __attribute__((section(".romrodata")))
#define _ATTRIBUTE_FAR_CALL   __attribute__((far_call))
#define _ATTRIBUTE_ROM_BSS   __attribute__((section(".bss.roms")))
#define _ATTRIBUTE_ROEXCEPT __attribute__((section(".roexcept")))
#define CONST 
#endif

#define _ATTRIBUTE_BSSX   __attribute__((section(".bssx")))
#define _ATTRIBUTE_CONF   __attribute__((section(".conf")))
//#define _ATTRIBUTE_ROMS   __attribute__((section(".roms")))
#define _ATTRIBUTE_BROMS  __attribute__((section(".broms")))
#define _ATTRIBUTE_ROMS      __attribute__((section(".bss.bssx")))

#define _ATTRIBUTE_LANGUAGE_EN     __attribute__((section(".language_en")))
#define _ATTRIBUTE_LANGUAGE_2      __attribute__((section(".language_2")))


#endif
