#include "i2c_gpio_master.h"
#include "bsp.h"

#define SDA GPIO13
#define SCL GPIO12

#define I2CSPEED 100


enum I2C_CMD 
{
	I2C_Write,
	I2C_Read,
};


void I2C_delay()
{
    wait_us((1000/I2CSPEED)/2);
}

static void wait_us(INT32U usec)
{
    volatile INT32U i;
#ifdef DYANMIC_CPU_SLOW
    for (i = 0 ; i < usec; i++)
#else
    for (i = 0 ; i < usec*4; i++)
#endif
    {
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");//50 NOP
        asm("nop");
    }
}



void init_gpio_dir(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val)
{
	INT32U temp, offset;
	offset =(gpio_num - GPIO12)*4;
	printk("ENter GPIO_SDA_H1 \r\n");
	temp = REG32(OOBMAC_IOBASE + MAC_GPIOCTL3);
 	temp &= ~(0xF << offset);  /*first unmask the responding gpio regs */
 	printk("ENter GPIO_SDA_H2 \r\n");
  if(OUTPUT == dir) {
  		printk("ENter GPIO_SDA_H3 \r\n");
        temp |= (0x03 | (val << 2)) << offset;  // enable oobmac controller and direction
 		REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) = temp;
		printk("ENter GPIO_SDA_H4 \r\n");
   	}
  else if(INPUT== dir) {
  	printk("ENter GPIO_SDA_H5 \r\n");
        temp |= (0x01)<< offset;
		REG32(OOBMAC_IOBASE + MAC_GPIOCTL3) = temp;
		printk("ENter GPIO_SDA_H6 \r\n");
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


void i2c_start_cond( void ) 
{
    if( started ) 
    { 
        // if started, do a restart cond
        // set SDA to 1
        set_gpio_output(SDA,1);
        I2C_delay();
    }

    // SCL is high, set SDA from 1 to 0.
    set_gpio_output(SDA,0);
    I2C_delay();
    I2C_delay();
    set_gpio_output(SCL,1);
    started = true;
}

void i2c_stop_cond( void )
{
    // set SDA to 0
    set_gpio_output(SDA,0);
    I2C_delay();

    I2C_delay();
    set_gpio_output(SCL,1);
    I2C_delay();

    // Stop bit setup time, minimum 4us
    I2C_delay();

    // SCL is high, set SDA from 0 to 1
    set_gpio_output(SDA,1);
    I2C_delay();

    I2C_delay();
    started = false;
}

// Write a bit to I2C bus
void i2c_write_bit( INT8U bit ) 
{ 
    if( bit ) 
    {
        set_gpio_output(SDA,1);
    } 
    else 
    {
        set_gpio_output(SDA,0);
    }

    // SDA change propagation delay
    I2C_delay();  

    // Set SCL high to indicate a new valid SDA value is available
    set_gpio_output(SCL,1);

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay();

    // Clear the SCL to low in preparation for next change
    set_gpio_output(SCL,0);
}

// Read a bit from I2C bus
INT8U i2c_read_bit( void ) 
{
    INT8U bit;

    // bossino, release clk, necessary for APPLE ACP
    // read_SDA();

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay();

    // Set SCL high to indicate a new valid SDA value is available
    set_gpio_output(SCL,1);

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay();
	
    // SCL is high, read out bit
    init_gpio_dir(SDA,INPUT,1);
    bit = get_gpio_input(SDA);
	init_gpio_dir(SDA,OUTPUT,1);
	
    // Set SCL low in preparation for next operation
    set_gpio_output(SCL,0);

    return bit;

}

// Write a byte to I2C bus. Return 0 if ack by the slave.
INT8U i2c_write_byte( BOOLEAN send_start, BOOLEAN send_stop, INT8U byte) 
{
    INT8U     bit;
    INT8U     nack;

    if( send_start ) 
    {
        i2c_start_cond();
    }

    for( bit = 0; bit < 8; bit++ ) 
    {
        i2c_write_bit( ( byte & 0x80 ) != 0 );
        byte <<= 1;
    }

    I2C_delay();

    nack = i2c_read_bit();

    if (send_stop) 
    {
        i2c_stop_cond();
    }

    return nack;
}

// Read a byte from I2C bus
INT8U i2c_read_byte( BOOLEAN nack , BOOLEAN send_stop ) 
{
    INT8U byte = 0;
    INT8U bit;

    for( bit = 0; bit < 8; bit++ ) 
    {
        byte = ( byte << 1 ) | i2c_read_bit();
    }

    i2c_write_bit( nack );

    if( send_stop ) 
    {
        i2c_stop_cond();
    }

    return byte;
}






void I2C_gpio_init(void)
{
	printk("ENter GPIO_init \r\n");
	init_gpio_dir(SDA,OUTPUT,1);
	printk("ENter GPIO_SDA_H \r\n");
	init_gpio_dir(SCL,OUTPUT,1);
	printk("ENter GPIO_SCL_H \r\n");

	
	printk("ENter GPIO_Write \r\n");
	i2c_write_byte(true,true,0x22);
	printk("ENter GPIO_Write_END \r\n");
}
