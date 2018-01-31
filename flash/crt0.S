/*
 * Realtek Semiconductor Corp.
 *
 *
 *
 *
 */
#include "os_cpu_reg.h"
#include "bsp_cfg.h"

    .section ".bss"
    .globl  _stack_lo
_stack_lo:
    .rept 1024
    .byte 0
    .endr

    .balign 16
    .globl  _stack_hi
_stack_hi:
    .rept 8
    .long 0
    .endr

    ##################################################################
    # Entry point of the entire application
    ##################################################################
.section ".boot", "ax"
    .global _boot
    .ent _boot
    .set noreorder

_boot:
 #    .byte 0x52, 0x65, 0x61, 0x6c, 0x74, 0x65, 0x6b
 #    .byte 0x44, 0x41, 0x53, 0x48, 0x31, 0x2E, 0x31

    ##################################################################
    # After reset
    # BEV = 1, IM = 0, CU = 0  
    ##################################################################
    mfc0    s4, CP0_STATUS        #save original in s4
    nop
    and     t0, s4, ~SR_BEV
    or      t0, (SR_CU0 | SR_CU3)        # make sure coprocessors enabled
    mtc0    t0, CP0_STATUS
    mtc0    zero, CP0_CAUSE     # zero the cause register
    nop

    ###########################
    # DMEM configuration      #
    ###########################

    la	    t2,	DMEM_BASE
    la      t3, DMEM_TOP	
    mtc3	t2,	$4		#DMEM BASE
    mtc3	t3,	$5		#DMEM TOP
	
    mfc0	t0, 	C0_CCTL		#read cctl
    nop
    or	t0, 	0x400		        #set bit 10(DMEMon)
    mtc0	t0, 	C0_CCTL		#write back to cctl
    nop

    ##################################################################
    # Initialize Stack Pointer
    #   _stack is initialized by the linker script to point to the
    #    starting location of the stack in DRAM
    ##################################################################
    la    sp,     _stack_hi
    
    ##################################################################
    # Initialize Global Pointer
    #   _gp is initialized by the linker script to point to "middle"
    #   of the small variables region
    ##################################################################
    la	  gp,     _gp

    la	  t0, main
    jalr	t0
    nop		

    ##################################################################
    # Just in case, go into infinite loop
    ##################################################################
1:
    b	  1b
    nop
	
    .end _boot
