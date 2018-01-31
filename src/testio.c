#include "bsp.h"
#include "lib.h"
#include "testio.h"

//enable interrupt if needed
#if 0
void bsp_testio_inten(void){
	rlx_irq_set_handler(BSP_GMAC_IRQ, IB_Access_OOB_Handler);
	rlx_irq_set_mask(BSP_GMAC_IRQ, OS_FALSE);
	REG32(Interrupt_IMSR) = (REG32(Interrupt_IMSR) &0x0000ffff)|IMR_ACCRISC_bit ;
}
#endif
void IB_Access_OOB_Handler (void){

	void * acc_address;

	if((REG32(Interrupt_IMSR) & ISR_ACCRISC_bit)){

		//clear the Access RISC register Interrup status W1C
		REG32(Interrupt_IMSR) = REG32(Interrupt_IMSR)&0x0000ffff|ISR_ACCRISC_bit ;

		acc_address = REG32(RISC_ACC_ADDR);
		//RW operation 
		if ((REG32(RISC_ACC_SET) & RISC_RW_SEL_bit) ==RISC_WRITE){
			switch (REG32(RISC_ACC_SET)& RISC_ACC_BYTEEN_bits){
				case RISC_ACC_1BYTEEN_0 : 
					REG8(acc_address)=REG8(RISC_ACC_DATA);
					break;
				case RISC_ACC_1BYTEEN_1 : 
					REG8(acc_address+1)=REG8(RISC_ACC_DATA+1);
					break;

				case RISC_ACC_1BYTEEN_2 : 
					REG8(acc_address+2)=REG8(RISC_ACC_DATA+2);
					break;
				case RISC_ACC_1BYTEEN_3 : 
					REG8(acc_address+3)=REG8(RISC_ACC_DATA+3);
					break;
				case RISC_ACC_2BYTEEN_01:
					REG16(acc_address)=REG16(RISC_ACC_DATA);
					break;	
				case RISC_ACC_2BYTEEN_23:
					REG16(acc_address+2)=REG16(RISC_ACC_DATA+2);
					break;	
				case RISC_ACC_3BYTEEN:
					REG16(acc_address)=REG16(RISC_ACC_DATA);
					REG8(acc_address+2)=REG8(RISC_ACC_DATA+2);
					break;	
				case RISC_ACC_4BYTEEN:
					REG32(acc_address)=REG32(RISC_ACC_DATA);
					break;	
				default : break;
			}			
		}
		else {
			REG32(RISC_ACC_DATA) = REG32(acc_address);
		}

		// clear the RISC_ACC_FLAG
		REG32(RISC_ACC_SET) &= (~RISC_ACC_FLAG_bit) ;
		
	}
}