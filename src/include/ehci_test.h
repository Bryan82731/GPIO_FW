/**************************************/
/*  Date: 2010.09.10                                    */
/*  Editor: Tom                                            */
/*  Reason: RTK EHCI Design                       */
/*  Firmware version: Draft version               */
/**************************************/
#ifndef  _RTK_EHCI_H_
#define  _RTK_EHCI_H_
#include "bsp.h"

//tomadd 2011.08.24
//reason for anybyte alignment test
//#define ANYBYTE

//reason for composite hid test
#define COMPOSITE_HID

//reason for stall  ,this stall will tenimate qtd transfer 
#define EHCISTALL 

//reason for 	Keyboard/mouse  IN Request Received When TxFIFO Empty 
//#define HID_TIMEOUT


/** RISC register utilition format **/
#define REG32(reg)	(*(volatile INT32U *)(reg))
#define REG16(reg)      (*(volatile INT16U *)(reg))
#define REG8(reg)	(*(volatile INT8U  *)(reg))
#define cpu_to_le32(x)  (x)//((x>>24) | ((x>>8)&0xff00) | ((x<<8)&0xff0000) | (x<<24))
#define cpu_to_le16(x)	(x)//((x>>8) | ((x&0x00ff)<<8))
//#define Uncache_Mask	0x80000000

/** DMEM Configuration **/
//          
#define DMEM_BASE			0x00040000

//#define DMEM_TOP             0x0901FFFF
//#define DMEM_ON              0x00000400
//#define DMEM_V_BASE          0x80000000

/** Physical address to virtual address **/
//#define PA2VA(vaddr)		((INT32U) (vaddr) | DMEM_V_BASE)
/** Virtual address to physical address **/
//#define VA2PA(vaddr)		((INT32U) (vaddr) & ~DMEM_V_BASE)

// **** assign memory allocation ****/
/** Descriptors assign to EHCI engine **/
//#define DMEMStartAddr          0x89000000
// tom mark 2010.12.14
// for control out descriptor start address
//#define CTLOUTdescStartAddr      (DMEMStartAddr + 0x1900)//0x0EC0) 
// for control in descriptor start address
//#define CTLINdescStartAddr         (DMEMStartAddr + 0x1920)//0x0EE0)
// for bulk out descriptor start address
//#define OUTdescStartAddr            (DMEMStartAddr + 0x1940)//0x0F00) 
// for bulk in descriptor start address
//#define INdescStartAddr               (DMEMStartAddr + 0x1960)//0x0F20)

// tom mark 2010.12.14
// reason for control out/in descriptor maximal number
#define BULKOUTdescNumber                      4 // Maximal Bulk OUT descriptor number 
#define BULKINdescNumber                         1 // Maximal Bulk IN descripotr number
#define CTLOUTdescNumber                4 // Maximal Control OUT descriptor number
#define CTLINdescNumber                   1 // Maximal Control IN descriptor number
#define INTIN1desNumber					1	//Maximal INT IN 1descriptor number
#define INTIN2desNumber					1	//Maximal INT IN 2 descriptor number

// tom mark 2010.12.14
// reason for control out/in buffer start address
/** EHCI engine out transaction start buffer address **/
//#define CTLOUT_BUFSTARTADDR     (DMEMStartAddr + 0x2000) // Control out buffer start address
//#define OUT_BUFSTARTADDR           (DMEMStartAddr + 0x2800) // Bulk out buffer start address
/** EHCI engine in transaction start buffer address  **/
//#define CTLIN_BUFSTARTADDR        (DMEMStartAddr + 0x3200) // Control in buffer start address 
//#define IN_BUFSTARTADDR              (DMEMStartAddr + 0x3400) //Bulk in buffer start address 
//#define BulkOUTADDR                 (DMEMStartAddr + 0x4000)
//tom modifed 2011/10/21
//for EP phase II debug
//#define BulkINADDR                  (DMEMStartAddr + 0x18000)
//#define BulkINADDR                  (DMEMStartAddr + 0x3800)
// tom modified 2010.12.14
// descriptor final version
/** EHCI control/bulk out and control/bulk in descriptor data structure **/
typedef struct
{
    struct
    {
        INT32U length  : 16; //total bytes to transfer, max len = 0x5000(20 kbytes)
	 INT32U rsvd   : 11;  // reserved
	 	//tomadd 2011.09.18
	 	//add stall bit
	 	INT32U stall    :1;
	 // tom modified 2010.12.14
	 // descriptor final version
	 INT32U ls        :1;    // last segment descriptor
	 INT32U fs        :1;    // first segment descriptor
	 INT32U eor     :1;    // end of ring
        INT32U own     : 1;  // own bit [descriptor]
    }stoi;
	
    INT8U* outin_buf_lowaddr;
}volatile outindesc_r;

//Sophia : Control OUT descriptor new format
typedef struct
{
  struct
    {
        INT32U length 		: 16; //total bytes to transfer, max len = 0x5000(20 kbytes)
        INT32U rsvd   		: 2; //reserved
        INT32U port_num 	: 2;
		INT32U device_addr  : 7; //device address
		INT32U stall	  	: 1;
		INT32U ls        	: 1;   // last segment descriptor
		INT32U fs        	: 1;    // first segment descriptor
		INT32U eor     		: 1;    // end of ring
	    INT32U own     		: 1;  // own bit [descriptor]
    }stoi;
	
    INT8U* outin_buf_lowaddr;

}volatile controloutdesc_r;

// tom modified 2010.12.14
// Hardware register interface final version
/** RTK EHCI Engine Register Definition in ./src/include/bsp_cfg_fp_revA.h**/

//tomadd 2011.04.28
//EHCI connection bit
#define PORTSC1                                       0x064
#define PORTSC2										  0x068
#define PORTSC3										  0x06c
#define HCSPARAMS                                    0x004
#define HCCPARAMS									 0x008
//tomadd 2011.09.22
//EHCI IO register
#define CONFIGFLAG                                   0x060
//tomadd 2011.09.30
//debug for WIN7 remove virtual device issue
#define USBCMD                                       0x020
//tomadd 2011.10.26
//debug for EP phase II
#define IBACTADDR                                    0x044

//extern INT8U descriptor_prepare_in1; 
//extern INT8U descriptor_prepare_in2;
extern INT16U  bulk_counter;
extern INT8U setaddress_counter;
extern INT8U PORT1_address;
extern INT8U PORT2_address;	
extern INT8U PORT3_address;
extern INT8U HID_ep;
extern INT8U is_out;

extern INT8U * DESC_ADDR;
extern INT8U * CSW_ADDR;







#endif /* _RTK_EHCI_H_ */
