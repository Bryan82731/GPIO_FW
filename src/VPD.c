/*#include <rlx/rlx_types.h>
#include <rlx/rlx_cpu_regs.h>
#include "bsp_cfg.h"
#include "bsp_fp_revA.h"
#include "rlx_types.h"*/

#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>


#include "VPD.h"


//for test ,VPD do not use flash as memory access 
//here will use a malloc buffer 
INT8U* vpd_buffer;

void VPD_Inital (void){

/*
	//unprotect spi flash
	WRSR_Flash_one_two_channel();

	//erase block,the block is special block for vpd,here it's just for test with block 0;
	//we should modify flash's opera for different application 
	Flash_erase_block(0);
*/

//use while loop task

	//REG16(BMC_IMR)=0x0000;
	REG16(BMC_ISR)=REG16(BMC_ISR);
	REG16(BMC_IMR) |= VPD_MASK;

	//malloc virtual memory 
	vpd_buffer=(INT8U*)malloc(512);

}


//VPD rw function 
//called by VPD interrupt handler 
//prerequest : we  must unprotect spi flash before write flash  
void VPD_RW_subHandler (void)
{

	volatile INT32U temp;
	volatile INT32U addr;

	temp = REG32(VPD_CTRL);   //same as ISR
	addr = (temp & VPD_ADDR_MASK)>>VPD_ADDR_BIT;
	
//	REG16(BMC_IMR)&=~VPD_MASK;  //close interrupt
//	REG16(BMC_ISR)=VPD_STS;   //clear ISR
	
	if ( (temp & VPD_RW_MASK ) == VPD_CMD_WRITE)
	{
		//call the flash write function 
		//Flash_write_one_channel(4,addr, (INT32U *)VPD_DATA);
		*(vpd_buffer+addr)=REG8(VPD_DATA);
		*(vpd_buffer+addr+1)=REG16(VPD_DATA)>>8;
		*(vpd_buffer+addr+2)=REG32(VPD_DATA)>>16;
		*(vpd_buffer+addr+3)=REG32(VPD_DATA)>>24;
	}
	else if ( (temp & VPD_RW_MASK ) == VPD_CMD_RD)
	{
		// data can directly read from flash 
		//REG32(VPD_DATA) = REG32(FLASH_DATA_ADDR + addr) ;	
		if(addr%4){
			if(addr%2){
				REG32(VPD_DATA)= REG8(vpd_buffer+addr)|(((INT32U)REG8(vpd_buffer+addr+1))<<8)|\
									(((INT32U)REG8(vpd_buffer+addr+2))<<16)|\
									(((INT32U)REG8(vpd_buffer+addr+3))<<24);
			}else{
				REG32(VPD_DATA)= (((INT32U)REG16(vpd_buffer+addr+1))<<16) |\
								  REG16(vpd_buffer+addr);
			}
		}else{
			REG32(VPD_DATA)=REG32(vpd_buffer+addr);
		}
	}

	REG16(BMC_ISR) = VPD_STS;   //clear ISR
//	REG16(BMC_IMR)|=VPD_MASK;   //open IMR
	
}


void VPD_Test_Task(void)
{
//	VPD_Inital();
	
	while (1){
		/*
		if (REG32(BMC_ISR_IMR) & VPD_STS){
			VPD_RW_Handler();
			//W1C
			REG32(BMC_ISR_IMR)|=VPD_STS;   //clear ISR
		}
		*/   //in a polling way
		OSTimeDly(OS_TICKS_PER_SEC);
	}
}






