#include "i2c_gpio_master.h"

#define SDA GPIO13
#define SCL GPIO12



enum I2C_CMD 
{
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










void I2C_gpio_init(void)
{
	printk("ENter GPIO_init \r\n");
	init_gpio_dir(SDA,OUTPUT,1);
	init_gpio_dir(SCL,OUTPUT,1);
	
	printk("ENter GPIO_SDAL_SCLL \r\n");
	set_gpio_output(SDA,0);
	set_gpio_output(SCL,0);
	
	printk("ENter GPIO_SDAH_SCLH \r\n");
	set_gpio_output(SDA,1);
	set_gpio_output(SCL,1);
}
