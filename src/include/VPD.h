#ifndef _VPD_H
#define _VPD_H

	
	#include "bsp_cfg.h"
	#include <rlx/rlx_types.h>

	#define VPD_CTRL	  FUNCTION0_BASE_ADDR+0x300
	#define VPD_RW_MASK   0x03
	#define VPD_CMD_WRITE 0x01
	#define VPD_CMD_RD	  0x02
	#define VPD_ADDR_MASK 0x7fff0000 
	#define VPD_ADDR_BIT  16
	
	#define VPD_MASK	0x0010

	#define VPD_DATA	(FUNCTION0_BASE_ADDR+0x304)

	#define VPD_STS			0x0010
//	#define PERSTB_R_STS 	0x0020
//	#define PERSTB_F_STS	0x0040


	
	void VPD_Inital (void);
	void VPD_RW_subHandler (void);
	void VPD_Test_Task(void);

	

#endif





