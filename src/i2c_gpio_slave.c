#include "i2c_gpio_slave.h"

#define SDA GPIO13
#define SCL GPIO12
#define Clear_SDA_interrupt REG32(OOBMAC_IOBASE + 0x518)|=(BIT13); // write 1 clear
#define ACK(sda)	init_gpio_dir(SDA, OUTPUT, 0);//OOBMAC_gpio_init(sda,OUTPUT,0);
#define SET_SDA_OUTPUT(val) set_gpio_output(SDA,val);
#define Clear_interrupt REG32(OOBMAC_IOBASE + 0x510)|=(BIT4); // write 1 clear
				
#define Wrong_Address 99
#define SLAVE_ADDRESS 0x57




enum I2C_CMD {
	I2C_Write,
	I2C_Read,
};

void init_gpio_dir(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val)
{
	INT32U temp, offset;
	offset =(gpio_num - GPIO12)*4;
	
	temp = REG32(OOBMAC_IOBASE + MAC_GPIOCTL3);
 	temp &= ~(0xF << offset);  /*first unmask the responding gpio regs */
 	
  if(OUTPUT == dir) {
        temp |= (0x03 | (val << 2)) << offset;  // enable oobmac controller and direction
 		REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) = temp;
   	}
  else if(INPUT== dir) {
        temp |= (0x01)<< offset;
		REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) = temp;
   	}
}

void set_gpio_output(OOBMAC_gpio_no gpio_num,INT8U val)
{
	if (val >= 1) 
	REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) |= (1<<((gpio_num-GPIO12)*4+2));
	else
	REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) &= ~((1<<((gpio_num-GPIO12)*4+2)));	
}

INT8U get_gpio_input(OOBMAC_gpio_no gpio_num)
{
	INT32U temp;
	temp = REG32(OOBMAC_IOBASE + MAC_GPIOCTL3);
	temp &= (1<<((gpio_num-GPIO12)*4+3));
	return ((temp!=0)?1:0);
}

INT8U I2C_check_address(INT8U addr)
{
	INT8U Rec_addr = 0;
	INT8U idx = 0;
	bsp_wait_1us();
	
	while(1) {
	/* high valid */ 
	if(get_gpio_input(SCL)) {
		Rec_addr |= get_gpio_input(SDA);
		idx++;
		if(idx == 8)
			break;	
		Rec_addr <<= 1;
		}
	}
	if((Rec_addr >> 1)== addr) {
			ACK(SDA);
		    return Rec_addr & BIT0;
		}
	else
		return Wrong_Address;
}

void I2C_Get_Multi_Byte(INT8U **buffer,INT8U Length)
{
	INT8U i = 0, idx = 0;
	*buffer = (INT8U*)malloc(Length*sizeof(INT8U));
	
	while(1) {
	/* high valid */ 
	if(get_gpio_input(SCL)) {
		*buffer[i] |= get_gpio_input(SDA);
		idx++;
		
		if(idx == 8) {
			if(++i == Length) 
				break;
			else {
				ACK(SDA);
				bsp_wait_1us();
				init_gpio_dir(SDA, INPUT, 1);
				idx = 0;
				continue;
				}
			
		*buffer[i] <<= 1;
			}
		}
	}
}

void I2C_Send_Multi_Byte(INT8U buffer[],INT8U Length)
{
	INT8U i = 0;
	INT8S idx = 7;
	BOOLEAN SCL_Flag = 1;
	
	while(1) {
	/* change value at SCL Low */
	if(SCL_Flag && !get_gpio_input(SCL)) {
		SET_SDA_OUTPUT((buffer[i]>>idx & 0x01));
		
		idx--;
		SCL_Flag = 0;
		
		if(idx < 0) {
			if(++i == Length) {
				OOBMAC_gpio_init(SDA,INPUT,1); //wait Master Ack
				break;
				}
			else {
				idx = 7;
					
				OOBMAC_gpio_init(SDA,INPUT,1); //wait Master Ack
				}
		}
	

	}
	if(get_gpio_input(SCL))
		SCL_Flag = 1;
	
	
			
		}
	
		
			
		
}

void Start_Condition(void)
{
	
	INT8U CMD = 0;
	INT8U *Data_buf = NULL;
	INT8U Send_Data[5]={0x55,0x22,0x33,0x44,0x55};
	
	CMD = I2C_check_address(SLAVE_ADDRESS);
	
	bsp_wait_1us(); // keep ACK until next clock
	
	switch(CMD) {
	case I2C_Write:
		init_gpio_dir(SDA,INPUT,1);
		//OOBMAC_gpio_init(SDA,INPUT,1);
		I2C_Get_Multi_Byte(&Data_buf,1);
		printk("OUT\r\n");
		printk("CMD and Data is %d %x \r\n",CMD,Data_buf[0]);
		//printk("CMD and Data is %d %x \r\n",CMD,Data_buf[1]);
		free(Data_buf);
		break;
		
	case I2C_Read:
		I2C_Send_Multi_Byte(Send_Data,1);
		printk("OUT\r\n");
		break;
		
	case Wrong_Address:
		printk("Receive Wrong Slave Address \r\n");
		break;
		
	default:
		break;
		
		}

	Clear_SDA_interrupt
}

void I2C_gpio_init(void)
{
	printk("1 \r\n");
	//OOBMAC_gpio_init(SCL,INPUT,1);
	//OOBMAC_gpio_init(SDA,INPUT,1);
	init_gpio_dir(SDA,OUTPUT,1);
	init_gpio_dir(SCL,OUTPUT,1);
	set_gpio_output(SDA,1);
	set_gpio_output(SCL,1);

	
	REG32(OOBMAC_IOBASE + 0x0518)&=(~BIT29);
	REG32(OOBMAC_IOBASE + 0x0518)|=BIT30;
	rlx_irq_register(BSP_ISO_IRQ, Start_Condition);
	
	
}