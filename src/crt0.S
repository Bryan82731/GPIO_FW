/*
 * Realtek Semiconductor Corp.
 *
 *
 *
 *
 */
#include <rlx/rlx_cpu_regs.h>
#include "bsp_cfg.h"
#include "sys/dpdef.h"

//section .bssx is used to differentiate from .bss, which need to be
//initialized to zero
//however, the time for initializing .bss to zero is within some function call
//far from the boot code, which may already use the stack for a while
//The idea is to keep the .bssx not to be initialized



#if CONFIG_VERSION >= IC_VERSION_EP_RevA
    .section ".bss.stack", "aw", @nobits
#else
    .section ".bssx"
#endif
    .globl  _stack_lo
_stack_lo:
    .rept 2048
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

#invlidate dcache & icache at the very beginning    
    mfc0  t0,  C0_CCTL
    nop
    and   t0,  0xFFFFFFFC
    mtc0  t0,  C0_CCTL
    or    t0,  0x00000003
    mtc0  t0,  C0_CCTL

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
	
#if 0
	//DMEM init at 0xbfc0_0000
    la    t1,  DMEM_BASE
    la    t2,  DMEM_TOP
    mtc3  t1,  $4        #DMEM BASE
    mtc3  t2,  $5        #DMEM TOP
#endif

#turn on DWAllocOn
#if CONFIG_VERSION >= IC_VERSION_EP_RevB
    mfc0  t0,  C0_CCTL
    nop
    or    t0,  0x80
    mtc0  t0,  C0_CCTL
#endif

#if CONFIG_VERSION == IC_VERSION_EP_RevA
#Enable L2MEM is already done in boot code
    mfc0  t0,  C0_CCTL, 1
    nop
    or    t0,  0x04000000
    mtc0  t0,  C0_CCTL, 1
#endif

#if 0    
	//Enable DMEM at 0xbfc0_0000
    mfc0  t0,  C0_CCTL      #read cctl
    nop
    or    t0,  0x400
    mtc0  t0,  C0_CCTL    
    nop
#endif

#if 0
    la    t2,  WDT_CONFIG
    lw    t2,  0x0(t2)
    nop
    beq   zero, t2, init_start
    nop
    la    t1,  WDT_BASE_ADDR
    and   t2,  0x1E 
    sra   t2,  t2,  1
    sw    t2,  0x04(t1) #Set TORR according to the configuration 
    lw    t2,  0x0(t1)
    nop
    or    t2,  0x01
    sw    t2,  0(t1)    #Enable WDT
    addi  t0,  zero, 0x76
    sw    t0,  0x0C(t1) #restart counter
#endif

init_start:
    la    t0, init
    jalr  t0
    nop

    ##################################################################
    # Just in case, go into infinite loop
    ##################################################################
1:
    b	  1b
    nop
	
    .end _boot
