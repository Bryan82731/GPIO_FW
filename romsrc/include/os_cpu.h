/*
*****************************************************************************************
*                                               uC/OS-II
*                                        The Real-Time Kernel
*
*                            (c) Copyright 2006, Micrium, Inc., Weston, FL
*                                         All Rights Reserved
*
*                                               MIPS32 4K
*
*                                                 MPLAB
*
* File:    os_cpu.h
* Version: v1.03
*****************************************************************************************
*/

#ifndef _OS_CPU_H_
#define _OS_CPU_H_

#ifdef  OS_CPU_GLOBALS
#define OS_CPU_EXT
#else
#define OS_CPU_EXT  extern
#endif

/*
*****************************************************************************************
*                                      DATA TYPES
*                                 (Compiler Specific)
*****************************************************************************************
*/


typedef           void           VOID;
typedef unsigned  char           BOOLEAN;
typedef unsigned  char           INT8U;        /* Unsigned  8-bit quantity */
typedef signed    char           INT8S;        /* Signed    8-bit quantity */
typedef unsigned  short          INT16U;       /* Unsigned 16-bit quantity */
typedef signed    short          INT16S;       /* Signed   16-bit quantity */
typedef unsigned  int            INT32U;       /* Unsigned 32-bit quantity */
typedef signed    int            INT32S;       /* Signed   32-bit quantity */
typedef float                    FP32;
typedef double                   FP64;

typedef unsigned  int            OS_STK;       /* Each stack entry is 32 bits wide */
typedef unsigned  int  volatile  OS_CPU_SR;    /* The CPU Status Word is 32-bits wide.  
                                                  This variable MUST be volatile for proper 
                                                  operation.  Refer to             */
                                               /* os_cpu_a.s for more details.     */

/* Definiton of null pointer */
#ifndef NULL
#define NULL  ((void *)0) 
#endif


enum RLX_IRQ_ID
{
  /* MIPS R3000 IRQs */
  RLX_IRQ_ID_0 = 0,  /* IP[0]: SW Interrupt 0 */
  RLX_IRQ_ID_1,      /* IP[1]: SW Interrupt 1 */
  RLX_IRQ_ID_2,      /* IP[2]: HW Interrupt 0 */
  RLX_IRQ_ID_3,      /* IP[3]: HW Interrupt 1 */
  RLX_IRQ_ID_4,      /* IP[4]: HW Interrupt 2 */
  RLX_IRQ_ID_5,      /* IP[5]: HW Interrupt 3 */
  RLX_IRQ_ID_6,      /* IP[6]: HW Interrupt 4 */
  RLX_IRQ_ID_7,      /* IP[7]: HW Interrupt 5 */
  /* RLX4181/5181 specific IRQs */
  RLX_IRQ_ID_8,      /* IP[8]:  Prioritized HW Interrupt 0 */
  RLX_IRQ_ID_9,      /* IP[9]:  Prioritized HW Interrupt 1 */
  RLX_IRQ_ID_10,     /* IP[10]: Prioritized HW Interrupt 2 */
  RLX_IRQ_ID_11,     /* IP[11]: Prioritized HW Interrupt 3 */
  RLX_IRQ_ID_12,     /* IP[12]: Prioritized HW Interrupt 4 */
  RLX_IRQ_ID_13,     /* IP[13]: Prioritized HW Interrupt 5 */
  RLX_IRQ_ID_14,     /* IP[14]: Prioritized HW Interrupt 6 */
  RLX_IRQ_ID_15,     /* IP[15]: Prioritized HW Interrupt 7 */
  RLX_IRQ_ID_MAX     /* Number of total IRQ */
};

typedef INT32U RlxIrqIdType;

/* type of IRQ handler */
typedef void (*RlxIrqHandlerType) (void);

/*
*****************************************************************************************
*                                     CRITICAL SECTIONS MANAGEMENT
*
* Method #1: Disable/Enable interrupts using simple instructions.  After a critical 
*            section, interrupts will be enabled even if they were disabled before 
*            entering the critical section.
*
* Method #2: Disable/Enable interrupts and preserve the state of interrupts.  
*            In other words, if interrupts were disabled before entering the critical 
*            section, they will be disabled when leaving the critical section.
*
* Method #3: Disable/Enable interrupts and preserve the state of interrupts.  Generally 
*            speaking, you would store the state of the interrupt disable flag in the 
*            local variable 'cpu_sr' and then disable interrupts.  'cpu_sr' is allocated 
*            in all of uC/OS-II's functions that need to disable interrupts.  You would 
*            restore the interrupt disable state by copying back 'cpu_sr'
*            into the CPU's status register.
*****************************************************************************************
*/

#define  OS_CRITICAL_METHOD    3

#define  OS_ENTER_CRITICAL()   cpu_sr = OS_CPU_SR_Save();
#define  OS_EXIT_CRITICAL()    OS_CPU_SR_Restore(cpu_sr);

/*
***************************************************************************************** 
*                             RLX
*****************************************************************************************
*/

#define  OS_STK_GROWTH    1                    /* Stack grows from HIGH to LOW memory */
#define  OS_TASK_SW()     asm volatile("syscall");

/*
***************************************************************************************** 
*                             Macro extended by Realtek
*****************************************************************************************
*/
#ifdef CONFIG_FAST_CTXSW
#define FAST_BSS_SECTION  ".dmem.bss"
#else
#define FAST_BSS_SECTION  ".bss"
#endif


#define TASK_STACK_DECLARE(_name, _size)    \
OS_STK  _name[_size] __attribute__ ((section (FAST_BSS_SECTION))) = { 0 }



/*
*****************************************************************************************
*                             FUNCTION PROTOTYPES
*****************************************************************************************
*/

void       OSCtxSw(void);
void       OSIntCtxSw(void);
void       OSStartHighRdy(void);

OS_CPU_SR  OS_CPU_SR_Save(void);               /* See os_cpu_a.s */
void       OS_CPU_SR_Restore(OS_CPU_SR);       /* See os_cpu_a.s */

/* RLX IRQ services */
void       rlx_irq_init (void) ;
BOOLEAN    rlx_irq_set_mask (RlxIrqIdType irq_id, BOOLEAN mask);
BOOLEAN    rlx_irq_set_handler (RlxIrqIdType irq_id, RlxIrqHandlerType handler);

/* RLX Cache services */
void       rlx_dcache_flush(INT32U address, INT32U size) ;
void       rlx_dcache_invalidate(INT32U address, INT32U size);
void       rlx_dcache_store(INT32U address, INT32U size) ;
void       rlx_dcache_invalidate_all(void) ;
void       rlx_icache_invalidate_all(void) ;
BOOLEAN    rlx_icache_lock(INT32U mode);
void       rlx_dmem_enable(INT32U base, INT32U top);
void       rlx_dcache_flush_all(void);
void rlx_disable_dcache();
void rlx_enable_dcache();
void rlx_imem_writeable();
void rlx_imem_readonly();
void rlx_enable_WMP(unsigned int base, unsigned int top);
void rlx_disable_WMP();
#endif
