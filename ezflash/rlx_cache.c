/*
 * Realtek Semiconductor Corp.
 *
 * rlx_cache.c
 *    Cache services of RLX
 *
 * Viller Hsiao (villerhsiao@realtek.com.tw)
 * Jan. 07, 2008
 */

#include <bsp.h>


/* The possible op value in cache instruction */
#define CACHE_CMD_INVAL        0x11   /* The line is invalidated */
#define CACHE_CMD_WBINVAL      0x15   /* The line is written back if dirty, 
                                       * and invalidated regardless of state 
	                               */
#define CACHE_CMD_WB           0x19   /* The line is ritten back if dirty, 
                                       * and left in the clean state
                                       */

/* The setting regarding cache in CP0_CCTL */
#define CP0_CCTL_DMEMON		   0x00000400  /* Enable the DMEM             */
#define CP0_CCTL_DMEMOFF	   0x00000800  /* Disable the DMEM            */
#define CP0_CCTL_IMEMFILL          0x00000010  /* IMEM fill                   */
#define CP0_CCTL_IMEMOFF           0x00000020  /* IMEM off                    */
#define CP0_CCTL_DINVAL            0x00000001  /* The value of DInval         */
#define CP0_CCTL_IINVAL            0x00000002  /* The value of IInval         */
#define CP0_CCTL_DWB               0x00000100  /* The value of DWB            */
#define CP0_CCTL_DWBINVAL          0x00000200  /* The value of DWBInval       */
#define CP0_CCTL_ILOCK_MASK        0x0000000C  /* The mask of ILock           */
#define CP0_CCTL_ILOCK_OFFSET      2           /* The offset of ILock         */
#define CP0_CCTL_ILOCK_NORMAL      0x00        /* ILock = 00: normal operation */
#define CP0_CCTL_ILOCK_LOCKGATHER  0x02        /* ILock = 10: LockGather mode  */
#define CP0_CCTL_ILOCK_LOCKDOWN    0x03        /* ILock = 11: LockedDown mode  */


/*
 *****************************************************************************************
 * rlx_dcache_flush()
 *
 * Description: Perform data cache write back and invalidate with a range of memory
 *
 * Arguments  : address -- Start address of data to flush
 *              size    -- Size of data to flush
 *
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_flush(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WBINVAL), "r"(addr) );
    }
   
}

/*
 *****************************************************************************************
 * rlx_dcache_invalidate()
 *
 * Description: Perform data cache invalidate with a range of memory
 *
 * Arguments  : address -- Start address of data to invalidate
 *              size    -- Size of data to invalidate
 *
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_INVAL), "r"(addr) );
    }
   
}


/*
 *****************************************************************************************
 * rlx_dcache_store()
 *
 * Description: Perform data cache write back with a range of memory
 *
 * Arguments  : address -- Start address of data to write back
 *              size    -- Size of data to write back
 *
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_store(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WB), "r"(addr) );
    }
   
}


/*
 *****************************************************************************************
 * rlx_dcache_invalidate_all()
 *
 * Description: Flush the whole data cache
 *
 * Arguments  : None
 *
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate_all(void)
{
  INT32U  cctl_val = 0;
  
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
  cctl_val |= CP0_CCTL_DINVAL;
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));

}


/*
 *****************************************************************************************
 * rlx_icache_invalidate_all()
 *
 * Description: Invalidate the whole instruction cache
 *
 * Arguments  : None
 *
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_icache_invalidate_all(void)
{
  INT32U  cctl_val = 0;

  asm volatile("mtc0   %z0, $20"  :  : "Jr" ((INT32U)(cctl_val))); 
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
  cctl_val |= CP0_CCTL_IINVAL;
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
}

/*
 *****************************************************************************************
 * rlx_icache_lock()
 *
 * Description: Set locking in 2-way set associative instruction cache. Please reference 
 *              to the User Manual of RLX MPU for how they operate
 *
 * Arguments  : mode    -- 00 is normal     mode
 *                         10 is LockGather mode
 *                         11 is LockedDown mode
 *
 * Returns    : BOOLEAN -- OS_TRUE if paramter is valid, and OS_FALSE otherwise
 *****************************************************************************************
 */
BOOLEAN 
rlx_icache_lock(INT32U mode)
{
  INT32U  cctl_val;
  BOOLEAN mode_valid = OS_FALSE;  

  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));

  cctl_val &= ~CP0_CCTL_ILOCK_MASK;

  switch(mode)
    {
    case CP0_CCTL_ILOCK_NORMAL: 
    case CP0_CCTL_ILOCK_LOCKGATHER: 
    case CP0_CCTL_ILOCK_LOCKDOWN: 
      mode_valid = OS_TRUE;
      break;

    default:
      break;
    }

  /* valid setting, write to CP0_CCTL */    
  if (mode_valid == OS_TRUE)
    {
      cctl_val |= (mode << CP0_CCTL_ILOCK_OFFSET);
      asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
    }
  
  return (mode_valid);
}

/*
 *******************************************************************************
 * rlx_dmem_enable()
 *
 * Description: Perform data cache write back with a range of memory
 *
 * Arguments  : address -- start address of physical memory mapped 
 *              size    -- size of memory mapped 
 *
 * Returns    : None
 *******************************************************************************
 */

void
rlx_dmem_enable(INT32U base, INT32U top)
{
    INT32U cctl_dmem;
    asm volatile("mtc3   %z0, $4"  : : "Jr" ((INT32U)(base)));	
    asm volatile("mtc3   %z0, $5"  : : "Jr" ((INT32U)(top)));
    
    asm volatile("mfc0   %0, $20"   : "=r"(cctl_dmem));
    cctl_dmem |= CP0_CCTL_DMEMON;
    asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_dmem)));
}


/*
 *******************************************************************************
 * rlx_imem_refill()
 *
 * Description: Perform the imem refill 
 *
 * Arguments  : base  -- start address of physical memory mapped 
 *              top   -- size of memory mapped 
                mode  -- 0: only remap, 1: perform refill
 *
 * Returns    : None
 *******************************************************************************
 */
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode)
{
    INT32U cctl_imem;
    asm volatile("mtc3   %z0, $0"  : : "Jr" ((INT32U)(base)));	
    asm volatile("mtc3   %z0, $1"  : : "Jr" ((INT32U)(top)));

    if(mode == 1)
    {
        asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
        cctl_imem &=  ~CP0_CCTL_IMEMFILL;
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));

        cctl_imem |=  CP0_CCTL_IMEMFILL;
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
    }
}
