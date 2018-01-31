/*
 ******************************************************************************************
 * Realtek Semiconductor Corp.
 *
 * rlx_irq.c:
 *   RLX IRQ services
 *
 * Viller Hsiao (villerhsiao@realtek.com.tw)
 *
 *****************************************************************************************
 */

#include <ucos_ii.h> 

/* IRQ handler table */


/* IRQ handler table */
RlxIrqHandlerType  rlx_irq_handler_table[RLX_IRQ_ID_MAX] \
__attribute__ ((section (FAST_BSS_SECTION)))
={
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

/*
 *****************************************************************************************
 * rlx_irq_init()
 *
 * Description: Enable interrupt in processor.
 *
 * Arguments  : None
 *
 * Returns    : None
 *****************************************************************************************
 */
void
rlx_irq_init (void)
{
  INT32U  intvec_val;
  INT32U  sr_val;

  /* Register handler for Low-Overhead Prioritized Interrupts */
  asm volatile("la %0, OSLopIrqHandler": "=r" (intvec_val));
  asm volatile("mtlxc0 %z0, $2"  : : "Jr" ((INT32U)(intvec_val)));

  /* Initalize SR: enable interrupt */
  asm volatile("mfc0   %0, $12"   : "=r"(sr_val));
  sr_val |= 0x00000001;
  asm volatile("mtc0   %z0, $12"  : : "Jr" ((INT32U)(sr_val)));

}

/*
 *****************************************************************************************
 * rlx_irq_set_mask()
 *
 * Description: Set make of the specific IRQ of processor.
 *
 * Arguments  : RlxIrqIdType --  The IRQ id you would like to mask/unmask
 *              BOOLEAN      --  OS_TRUE  - mask the IRQ
 *                               OS_FALSE - unmask the IRQ
 *
 * Returns    : BOOLEAN      --  OS_TRUE when success, OS_FALSE when fail
 *
 *****************************************************************************************
 */
BOOLEAN
rlx_irq_set_mask (RlxIrqIdType irq_id, BOOLEAN mask)
{
  INT32U  new_mask;
  INT32U  sr_val;
#if OS_CRITICAL_METHOD == 3  /* Allocate storage for CPU status register     */
  INT32U  cpu_sr;
#endif
  if (irq_id < RLX_IRQ_ID_MAX)
    {
      /* Shift to the mask bit in STATUS register */
      new_mask = (0x00000100) << ( irq_id);
      OS_ENTER_CRITICAL();

      if (mask == OS_TRUE)   /* Mask the interrupt: Set 0 to IM bit */
        {
          if (irq_id <= RLX_IRQ_ID_7) /* MIPS R3000 interrupts */
            { /* Acess IM[7:0] in CP0 STATUS */
              asm volatile("mfc0   %0, $12"   : "=r"(sr_val));
              sr_val &= ~new_mask;
              asm volatile("mtc0   %z0, $12"  : : "Jr" ((INT32U)(sr_val)));
            }
          else                        /* RLX5181 Low-Overhead Prioritized Interrupts */
            { /* Acess IM[15:8] in CP0 ESTATUS */
              asm volatile("mflxc0   %0, $0"   : "=r"(sr_val));
              sr_val &= ~new_mask;
              asm volatile("mtlxc0   %z0, $0"  : : "Jr" ((INT32U)(sr_val)));
            }
        }
      else                   /* Unmask the interrupt: Set 1 to IM bit */
        {
          if (irq_id <= RLX_IRQ_ID_7) /* MIPS R3000 interrupts */
            { /* Acess IM[7:0] in CP0 STATUS */
              asm volatile("mfc0   %0, $12"   : "=r"(sr_val));
              sr_val |= new_mask;
              asm volatile("mtc0   %z0, $12"  : : "Jr" ((INT32U)(sr_val)));
            }
          else                        /* RLX5181 Low-Overhead Prioritized Interrupts */
            { /* Acess IM[15:8] in CP0 ESTATUS */
              asm volatile("mflxc0   %0, $0"   : "=r"(sr_val));
              sr_val |= new_mask;
              asm volatile("mtlxc0   %z0, $0"  : : "Jr" ((INT32U)(sr_val)));
            }
        }

      OS_EXIT_CRITICAL();

      return OS_TRUE;
    }
  else  /* Invalid IRQ ID */
   {
      return OS_FALSE;
   }
}
 
/*
 *****************************************************************************************
 * rlx_irq_set_handler()
 *
 * Description: Register the handler for an specific IRQ.
 *
 * Arguments  : RlxIrqIdType      --  The IRQ you would like to register handler
 *              RlxIrqHandlerType --  The IRQ handler
 *
 * Returns    : BOOLEAN           --  OS_TRUE when success, OS_FALSE when fail
 *****************************************************************************************
 */
BOOLEAN
rlx_irq_set_handler (RlxIrqIdType irq_id, RlxIrqHandlerType handler)
{
  RlxIrqHandlerType *irq_hdl_p;
#if OS_CRITICAL_METHOD == 3  /* Allocate storage for CPU status register     */
  INT32U  cpu_sr;
#endif

  if (irq_id < RLX_IRQ_ID_MAX)
    {
      irq_hdl_p = &rlx_irq_handler_table[irq_id];
      /* Assign handler to irq table */
      OS_ENTER_CRITICAL();
      *irq_hdl_p = handler;
      OS_EXIT_CRITICAL();

      return OS_TRUE;
    }
  else /* Invalid ID */
    {
      return OS_FALSE;
    }
}

void rlx_irq_register(RlxIrqIdType irq_id, RlxIrqHandlerType handler)
{
    rlx_irq_set_handler(irq_id, handler);
    rlx_irq_set_mask(irq_id, OS_FALSE);
}

      
