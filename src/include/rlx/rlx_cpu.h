#ifndef _RLX_CPU_H_
#define _RLX_CPU_H_


#include "rlx_types.h"

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

OS_CPU_SR  OS_CPU_SR_Save(void);               /* See os_cpu_a.s */
void       OS_CPU_SR_Restore(OS_CPU_SR);       /* See os_cpu_a.s */


#define  OS_ENTER_CRITICAL()   cpu_sr = OS_CPU_SR_Save();
#define  OS_EXIT_CRITICAL()    OS_CPU_SR_Restore(cpu_sr);

/* RLX IRQ services */
void       rlx_irq_init (void);
BOOLEAN    rlx_irq_set_mask (RlxIrqIdType irq_id, BOOLEAN mask);


/* RLX Cache services */
void       rlx_dcache_flush(INT32U address, INT32U size);
void       rlx_dcache_invalidate(INT32U address, INT32U size);
void       rlx_dcache_store(INT32U address, INT32U size);
void       rlx_dcache_invalidate_all(void);
void       rlx_icache_invalidate_all(void);
BOOLEAN    rlx_icache_lock(INT32U mode);
#endif
