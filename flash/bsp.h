/*
 * Realtek Semiconductor Corp.
 *
 * Board Support Package header file
 *
 * Tony Wu (tonywu@realtek.com.tw)
 * Dec. 07, 2007
 */


#ifndef  _BSP_H_
#define  _BSP_H_

#include "bsp_cfg.h"    /* address mapping and BSP configuration */

#ifndef  OS_FALSE
#define  OS_FALSE                     0u
#endif

#ifndef  OS_TRUE
#define  OS_TRUE                      1u
#endif

typedef unsigned  char           BOOLEAN;
typedef unsigned  char           INT8U;        /* Unsigned  8-bit quantity */
typedef signed    char           INT8S;        /* Signed    8-bit quantity */
typedef unsigned  short          INT16U;       /* Unsigned 16-bit quantity */
typedef signed    short          INT16S;       /* Signed   16-bit quantity */
typedef unsigned  int            INT32U;       /* Unsigned 32-bit quantity */
typedef signed    int            INT32S;       /* Signed   32-bit quantity */

typedef struct 
{
    union 
    {
       INT32U word;

       struct 
       {
          INT32U buffer_size : 14;
          INT32U rsvd	   : 16;
          INT32U eor	   :  1;
	  INT32U own	   :  1;
        }cmd;

       struct
       {
          INT32U length  : 14;
	  INT32U tcpf    :  1;
	  INT32U udpf    :  1;
	  INT32U ipf     :  1;
 	  INT32U tcpt    :  1;
	  INT32U udpt    :  1;
          INT32U crc     :  1;
          INT32U runt    :  1;
          INT32U res     :  1;
	  INT32U rwt     :  1;
	  INT32U rsvd    :  2;
	  INT32U bar     :  1;
	  INT32U pam     :  1;
	  INT32U mar     :  1;
	  INT32U ls      :  1;
          INT32U fs      :  1;
          INT32U eor     :  1;
	  INT32U own     :  1;
	}st;

    }offset0;	

    union
    {
       INT32U word;
       struct
       {
          INT32U vlan_tag    : 16;
	  INT32U tava        :  1;
	  INT32U ripv4       :  1;
	  INT32U ripv6       :  1;
	  INT32U rtcp        :  1;
          INT32U rsvd        :  8;
	  INT32U rmcp        :  1;
          INT32U mobf        :  1;
          INT32U v4f         :  1;
          INT32U v6f         :  1;
       }st; 
    }offset1;

    INT8U *rx_buf_addr_low;
    INT8U *rx_buf_addr_high;

}volatile rxdesc_t; 

typedef struct 
{
    union 
    {
       INT32U word;

       struct 
       {
          INT32U length      : 16;
          INT32U rsvd	   :  9;
	  INT32U gtsenv6     :  1;
          INT32U gtsenv4     :  1;
          INT32U lgsen       :  1;
	  INT32U ls          :  1;
          INT32U fs          :  1;
          INT32U eor	   :  1;
	  INT32U own	   :  1;
        }cmd;

       struct
       {
          INT32U rsvd    : 28;
	  INT32U ls      :  1;
          INT32U fs      :  1;
          INT32U eor     :  1;
	  INT32U own     :  1;
	}st;

    }offset0;	

    union
    {
       INT32U word;
       struct
       {
          INT32U vlan_tag    : 16;
	  INT32U rsvd        :  1;
          INT32U tagc        :  1;
          INT32U tcpho       : 10;
          INT32U v6f         :  1;
          INT32U ipv4cs      :  1;
          INT32U tcpcs       :  1;
          INT32U udpcs       :  1;
       }cmd; 
    }offset1;

    INT8U  *tx_buf_addr_low;
    INT8U  *tx_buf_addr_high;

}volatile txdesc_t; 

typedef struct
{
    INT8U   NDF;
	  INT16U  *buf;
}flash_t; 

typedef struct
{
    union
    {
       INT32U word;
       struct
       {
          INT32U length      : 11;
	  INT32U rsvd        : 16;
          INT32U alsel       :  2;
          INT32U valid       :  1;
          INT32U eor         :  1;
          INT32U own         :  1;
       }cmd; 
    }offset1;

    INT32U RSVD;
    INT8U  *key_addr;
    INT8U  *payload_addr;

}volatile tlsdesc_t;

/*
*****************************************************************************************
* BSP definition
*****************************************************************************************
*/
/* Register Macro */
#define REG32(reg)		(*(volatile INT32U *)(reg))
#define REG16(reg)		(*(volatile INT16U *)(reg))
#define REG8(reg)		  (*(volatile INT8U  *)(reg))

#define PA2VA(vaddr)		((vaddr) + DMEM_V_BASE)
#define VA2PA(vaddr)		((vaddr) - DMEM_V_BASE)


/*
*****************************************************************************************
* FUNCTION  PROTOTYPES
*****************************************************************************************
*/
/* Initialization */
void     bsp_setup (void);
void	 bsp_irq_disable(void);

/* GMAC functions */
INT32U send(INT8U *txbufferaddress, INT32U length);
void GetMacAddr(INT8U* addr);
inline void ReleaseBuf(rxdesc_t *desc);

#endif /*  _BSP_H_ */
