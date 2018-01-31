/*
 * Copyright (c) 2007, Realtek Semiconductor Corp.
 *
 * rlx_irq_util.h:
 * Configuration about IRQ for processor, including IRQ handler and 
 * IRQ priority of IRQ[7:0]
 *
 * Viller Hsiao (villerhsiao@realtek.com.tw)
 * Dec. 17, 2007
 *
 */

#ifndef _RLX_IRQ_UTIL_H_
#define _RLX_IRQ_UTIL_H_

#include "bsp_cfg.h"


/* The handler address offset of IPx in bsp_irq_handler_table */
#define ISR_OFFSET_IP0     0 
#define ISR_OFFSET_IP1     4 
#define ISR_OFFSET_IP2     8 
#define ISR_OFFSET_IP3     12
#define ISR_OFFSET_IP4     16
#define ISR_OFFSET_IP5     20
#define ISR_OFFSET_IP6     24
#define ISR_OFFSET_IP7     28
#define ISR_OFFSET_IP8     32
#define ISR_OFFSET_IP9     36
#define ISR_OFFSET_IP10    40
#define ISR_OFFSET_IP11    44
#define ISR_OFFSET_IP12    48
#define ISR_OFFSET_IP13    52
#define ISR_OFFSET_IP14    56
#define ISR_OFFSET_IP15    60



/* IP_NONE is used for comparision only */
#define IP_NONE          0xff

/*
*****************************************************************************************
* Macros
*****************************************************************************************
*/
/* Macro to judge which interrupt of IP[7:0] occurs */
#define _CHECK_IP(ip_no)           \
  andi  t1, t0, STATUSF_## ip_no;  \
  bnez  t1, L1_CPU_## ip_no;       \
  nop;

#define CHECK_IP(ip_no)  _CHECK_IP(ip_no)


/* Macro to dispatch interrupt handler for IP[7:0] */
#define _LOAD_OFFSET(ip_no)             \
L1_CPU_## ip_no :                       \
    lw    t1, ISR_OFFSET_## ip_no(t3);  \
    j     IRQ_DISPATCH;                 \
    nop;


#define LOAD_OFFSET(ip_no)  _LOAD_OFFSET(ip_no)


#endif /* _RLX_IRQ_UTIL_H_ */ 
