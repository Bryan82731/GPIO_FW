#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC.h"
#include "OOBMAC_COMMON.h"

typedef unsigned  int            BaseAddr; 

//#define OOBMAC_IOBASE        	  0xBAF70000
//#define MAC_IB_ACC_DATA           0x00A0
//#define MAC_IB_ACC_SET            0x00A4

//#define NCSI_IOBASE			0xBAFC0000 
//#define IO_IB_ACC_DATA           0x00A0
//#define IO_IB_ACC_SET            0x00A4


OS_CPU_SR  cpu_sr;
/* for OOB MAC access IB REG channel*/

INT32U OOB_access_IB_channel(INT32U baseAddr,ReadWrite_type ReadWrite,INT8U ByteEn,INT16U regAddr)/* regAddr should be OCP addr */
{
    REG32(baseAddr + IO_IB_ACC_SET) = (0x80000000 | (ReadWrite<<23) | ((ByteEn & 0x0f)<<16) | regAddr);
    while((REG32(baseAddr + IO_IB_ACC_SET) & 0x80000000));
}

/* regAddr should be OCP addr */
void OOB_READ_IB(INT32U baseAddr,INT8U ByteEn,INT16U regAddr,INT32U *data)
{
	OS_CPU_SR  cpu_sr = 0;

	OS_ENTER_CRITICAL();
	
	OOB_access_IB_channel(baseAddr,READ,ByteEn,regAddr);
	*data = REG32(baseAddr + IO_IB_ACC_DATA);

	OS_EXIT_CRITICAL();
	
}

/* regAddr should be OCP addr */
void OOB_WRITE_IB(INT32U baseAddr,INT8U ByteEn,INT16U regAddr,INT32U data)
{
	OS_CPU_SR  cpu_sr = 0;
		
	OS_ENTER_CRITICAL();

	REG32(baseAddr + IO_IB_ACC_DATA) = data;
	OOB_access_IB_channel(baseAddr,WRITE,ByteEn,regAddr);
	
	OS_EXIT_CRITICAL();
}

/*
function: OOBMAC access IBMAC by bit
baseAddr: eg OOBMAC_IOBASE or NCSI_IOBASE
reg     : IBMAC reg, fill ocp address (DWORD alignment)
highBit : bit 31-0
lowBit  : bit 31-0
value   : relative to value of lowBit to highBit
*/
void OOB_WRITE_IB_BIT(INT32U baseAddr, INT16U reg, INT8U highBit, INT8U lowBit, INT32U value)
{
	INT8U ByteEn=0x0000;
	INT32U	oriValue = 0x00000000; 
	INT32U	maskValue = 0x00000000;
	INT32U	inputValue = 0x00000000;
	INT8U   i=0;
	OS_CPU_SR  cpu_sr = 0;

	if(lowBit>=0 && lowBit<=7)
	{
		ByteEn |= 0x01;
		if(highBit>=8 && highBit<=15)
		{
			ByteEn |= 0x02;
		}
		if(highBit>=16 && highBit<=23)
		{
			ByteEn |= 0x04 | 0x02;	
		}
		if(highBit>=24 && highBit<=31)
		{
			ByteEn |= 0x08 | 0x04 | 0x02;	
		}
	}
	else if(lowBit>=8 && lowBit<=15)
	{
		ByteEn |= 0x02;
		if(highBit>=16 && highBit<=23)
		{
			ByteEn |= 0x04;	
		}
		if(highBit>=24 && highBit<=31)
		{
			ByteEn |= 0x08 | 0x4;	
		}
	}
	else if(lowBit>=16 && lowBit<=23)
	{
		ByteEn |= 0x04;
		if(highBit>=24 && highBit<=31)
		{
			ByteEn |= 0x08;	
		}
	}
	else if(lowBit>=24 && lowBit<=31)
	{
		ByteEn |= 0x08;
	}

	OS_ENTER_CRITICAL();

	OOB_access_IB_channel(baseAddr,READ,ByteEn,reg);
	oriValue = REG32(baseAddr + IO_IB_ACC_DATA);

	for(i=lowBit;i<=highBit;i++){
		maskValue=maskValue|(1<<i);
	}
	maskValue = ~maskValue;
	inputValue = (oriValue & maskValue)|(value<<lowBit);

	REG32(baseAddr + IO_IB_ACC_DATA) = inputValue;
	OOB_access_IB_channel(baseAddr,WRITE,ByteEn,reg);
	
	OS_EXIT_CRITICAL();
}

INT8U OOBMAC_sharepin_mode_set(SharePin_type sharepin,SharePin_mode mode)
{
	INT32U	oriValue = 0x00000000;   
	
	switch(sharepin)
	{
		case PERSTB_ISOLATEB_LANWAKE_CLKREQB:
			if(FUNCTION_MODE == mode){
				

				}
			else if(GPIO_MODE== mode){


				}
			break;
		case LEDPIN0TO2:
			switch(mode)
			{
				case GPIO_MODE:
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xDC00, 2, 2, 0);  //test_io_act = 0
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xDC00, 10, 10, 0);  //dbg_act = 0
					break;
				case DEBUG_MODE: 
					#ifdef DYNAMIC_CPU_SLOW    //risc_clock
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xDC00, 10, 10, 1);  //dbg_act = 1
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 12, 12, 1);   //risc_dbg_oe=1
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 23, 20, 0x0F); //macdbg_sel=0x1111
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 26, 25, 0);    //macdbg_shift[1:0]
					REG32(CPU1_IOBASE+0x2C)|=DWBIT31|DWBIT30;
					#endif
					#ifdef DEBUG_LTSSM
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 19, 17, 1);     //macdbg_pg_sel=0x1
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 23, 20, 0x02); //macdbg_sel=0x1111
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE850, 26, 25, 0);	//macdbg_shift[1:0]
					#endif
					break;
				
			}
			break;
		case EJTAG_SMBALERT0:
			switch(mode){
				case GPIO_MODE:
					//1.ocp_gpio_en:1   controlled in GPIO Pin Control settings
					//2.dbg_chg_pin:0
					oriValue=REG32(OOBMAC_IOBASE + MAC_DBG_SEL);   
					REG32(OOBMAC_IOBASE + MAC_DBG_SEL)=oriValue&(~DWBIT15);  
				    //REG32(OOBMAC_IOBASE + MAC_DBG_SEL) &= (~DWBIT15);
					//3.dis_jtag_mode[1:0]:11
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xDC00, 8, 7, 3);
					break;
				
				case DEBUG_MODE:

					break;
				case FUNCTION_MODE:
					
					break;
				case BOUNDARY_SCAN_MODE:
					
					break;
				case NOT_USED_MODE:
					
					break;




				}


			break;
		case SMBCLK0_SMBDATA0:



			break;
		case SPISCK_SPISI_SPISO0_2_3:



			break;
		case NF_ALE_NF_DD1_3_5_7:



			break;
		case NF_DD0_6_RDYWR_N_RD_N_NF_CLE:



			break;
		case UARTTX_UARTRX:



			break;
		case SMBCLK1_SMBDATA1:



			break;
		case NCSI_GPIO:



			break;
		case GPIPIN:
			switch(mode)
			{
				case GPIO_MODE:
					//1.ldps_en.linkok_gpio_en,ocp_gpi_en,gpi_oe_reg=4'b0010
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE85A, 9, 9, 0);     //linkok_gpio_en  
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xE90A, 0, 0, 0);		//ldps_en
					OOB_WRITE_IB_BIT(OOBMAC_IOBASE, 0xDC06, 13, 13, 0);	//gpi_oe_reg
					break;
				
			}
			break;
	
	}
	return 1;
}

/*
function: OOBMAC gpio initial setting
parameter:
gpio_num: GPIO pinname
dir:      directiong setting :INPUT or OUTPUT
val:		 if dir=INPUT,val is ignored,but you should set related interrupt 
		 if dir=OUTPUT,val is the initial output value
*/
void OOBMAC_gpio_init(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val)//if dir is GPIO_INPUT, then val is useless
{
     INT32U temp,temp_gpioType,temp_gpioCtrl;
	
	//1.sharepin mode setting  
	//NOTE:GPO/GPIO1-GPIO3 only set ocp_gpio_en(the next step will do it);GPIO7:sharepin with SJTCLKPIN of Jtag,it will be test by upper layer(UGMAC ERI channel)
	switch(gpio_num)
	{
		case GPI:
			OOBMAC_sharepin_mode_set(GPIPIN,GPIO_MODE);
			break;
		case GPIO4:
		case GPIO5:
		case GPIO6:
			OOBMAC_sharepin_mode_set(LEDPIN0TO2,GPIO_MODE);
			break;
	/*	case GPIO7:
			OOBMAC_sharepin_mode_set(EJTAG_SMBALERT0,GPIO_MODE);
			break;*/
	}
	//2.gpio configure:direction and controller right setting
	if(gpio_num>=GPIO25)
	{
		return;
		//T.B.D
	}
	
	if(gpio_num>=GPIO20)
    {
		temp_gpioType = GPIO20;
		temp_gpioCtrl = MAC_GPIOCTL4;
	}
	else if(gpio_num>=GPIO12)
    {
		temp_gpioType = GPIO12;
		temp_gpioCtrl = MAC_GPIOCTL3;
	}
	else if(gpio_num>=GPIO4)
    {
		temp_gpioType = GPIO4;
		temp_gpioCtrl = MAC_GPIOCTL2;
	}
	else
	{
		temp_gpioType = GPI;
		temp_gpioCtrl = MAC_GPIOCTL;
	}
	
	temp = REG32(OOBMAC_IOBASE+temp_gpioCtrl);
 	temp &= ~(0xf<<((gpio_num-temp_gpioType)*4));  //first unmask the responding gpio regs
 	
    if(OUTPUT == dir)  //2.1 if as output,you should set a initial output value
   	{
        temp |= (0x03 | (val<<2))<<((gpio_num-temp_gpioType)*4);  // enable oobmac controller and direction
		temp &= ~(1<<((gpio_num-temp_gpioType)*4+2));  //clear ori bit value
 		temp |= (val<<((gpio_num-temp_gpioType)*4+2));  //set new value for this bit
 		REG32(OOBMAC_IOBASE+temp_gpioCtrl) = temp;
   	}
    else if(INPUT== dir)  //2.2 if as input,you should enable its interrupter 
   	{
        temp |= (0x01)<<((gpio_num-temp_gpioType)*4);//second, set the responding regs
		REG32(OOBMAC_IOBASE+temp_gpioCtrl) = temp;
		temp = REG32(OOBMAC_IOBASE+MAC_EXTR_INT);
		temp |= (1<<(gpio_num+2))|(1<<(gpio_num+18));//enable gpio interrupt, clear gpio interrupt states
		REG32(OOBMAC_IOBASE + MAC_EXTR_INT) = temp;
		REG32(OOBMAC_IOBASE + 0x518) |= (1<<(gpio_num+15));  //enable positive gpio interrupt
   	}
}

/*

*/
void OOBMAC_gpio_output(OOBMAC_gpio_no gpio_num,INT8U val)
{
	INT32U temp = 0;
	INT32U reg_addr, gpio_offset;
	if(gpio_num>=GPIO25)
	{
		//T.B.D
	}
	else
	{
		if(gpio_num>=GPIO20)//GPIO20~GPIO24
		{
			reg_addr=MAC_GPIOCTL4;
			gpio_offset=gpio_num-GPIO20;
		}
		else if(gpio_num>=GPIO12)//GPIO12~GPIO19
		{
			reg_addr=MAC_GPIOCTL3;
			gpio_offset=gpio_num-GPIO12;
		}
		else if(gpio_num>=GPIO4)//GPIO4~GPIO11
		{
			reg_addr=MAC_GPIOCTL2;
			gpio_offset=gpio_num-GPIO4;
		}
		else//GPI, GPO, GPIO0~GPIO3
		{
			reg_addr=MAC_GPIOCTL;
			gpio_offset=gpio_num-GPI;
		}
		
		temp = REG32(OOBMAC_IOBASE+reg_addr);
		temp &= ~(1<<(gpio_offset*4+2));  //clear ori bit value
		temp |= (val<<(gpio_offset*4+2));  //set new value for this bit
		REG32(OOBMAC_IOBASE+reg_addr) = temp;
	}
}

INT8U OOBMAC_gpio_input(OOBMAC_gpio_no gpio_num)
{
	INT32U temp;
	temp = REG32(OOBMAC_IOBASE + MAC_STATUS);
	temp &= (1<<(gpio_num+18));
	return ((temp!=0)?1:0);
}

INT8U OOBMAC_gpio_input_general(OOBMAC_gpio_no gpio_num)
{
	INT32U temp = 0;
	INT32U reg_addr, gpio_offset;
	if(gpio_num>=GPIO25)
	{
		//T.B.D
	}
	else
	{
		if(gpio_num>=GPIO20)//GPIO20~GPIO24
		{
			reg_addr=MAC_GPIOCTL4;
			gpio_offset=gpio_num-GPIO20;
		}
		else if(gpio_num>=GPIO12)//GPIO12~GPIO19
		{
			reg_addr=MAC_GPIOCTL3;
			gpio_offset=gpio_num-GPIO12;
		}
		else if(gpio_num>=GPIO4)//GPIO4~GPIO11
		{
			reg_addr=MAC_GPIOCTL2;
			gpio_offset=gpio_num-GPIO4;
		}
		else//GPI, GPO, GPIO0~GPIO3
		{
			reg_addr=MAC_GPIOCTL;
			gpio_offset=gpio_num-GPI;
		}
		
		temp = REG32(OOBMAC_IOBASE+reg_addr);
		
		return ((temp & (1<<(gpio_offset*4+3)))?1:0);
	}
}
