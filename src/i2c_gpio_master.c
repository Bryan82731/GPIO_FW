#include "i2c_gpio_master.h"


#define GPIO_BASE_ADDR      0xBAF70000
#define I2C_SDA_REG_OFFSET  508
#define I2C_SCL_REG_OFFSET  508



#define SDA GPIO13
#define SCL GPIO12

#define I2CSPEED 300


BOOLEAN started = 0; // global data
INT8U BitR=1;
INT8U BitW=0;
INT8U slaveaddr = 0x68<<1;
INT32U max_reg_retry_cnt=100;
INT32U i2c_retry_period=100;
INT32U i2c_retry_maxcnt=10;
INT8U read_data[128];






enum I2C_CMD 
{
	I2C_Write,
	I2C_Read,
};

void I2C_delay_S()
{
    wait_us_S((1000/I2CSPEED)/2);
}

void I2C_delay()
{
    wait_us((1000/I2CSPEED)/2);
}

void wait_us(INT32U usec)
{
    volatile INT32U i;
//#ifdef DYANMIC_CPU_SLOW
//    for (i = 0 ; i < usec; i++)
//#else
    for (i = 0 ; i < usec*4; i++)
//#endif
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

    }
}
void wait_us_S(INT32U usec)
{
    volatile INT32U i;
//#ifdef DYANMIC_CPU_SLOW
//    for (i = 0 ; i < usec; i++)
//#else
    for (i = 0 ; i < usec*4; i++)
//#endif
    {
        asm("nop");asm("nop");asm("nop");asm("nop");asm("nop");
    }
}


/*
INT8U read_SCL( void ) // Set SCL as input and return current level of line, 0 or 1
{
    REG32(GPIO_BASE_ADDR+I2C_SCL_REG_OFFSET)&=~((I2C_GPIO_BIT_IO)<<I2C_GPIO_SCL_OFFSET);
    return REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_SCL_OFFSET?1:0;
}
INT8U read_SDA( void ) // Set SDA as input and return current level of line, 0 or 1
{
    REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)&=~((I2C_GPIO_BIT_IO)<<I2C_GPIO_SDA_OFFSET);
    return REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_SDA_OFFSET?1:0;
}
void set_SCL( void ) // Actively drive SCL signal high
{
    REG32(GPIO_BASE_ADDR+I2C_SCL_REG_OFFSET)|=(I2C_GPIO_BIT_OUTPUT|I2C_GPIO_BIT_IO)<<I2C_GPIO_SCL_OFFSET;
}
void clear_SCL( void ) // Actively drive SCL signal low
{
    REG32(GPIO_BASE_ADDR+I2C_SCL_REG_OFFSET)|=((I2C_GPIO_BIT_IO)<<I2C_GPIO_SCL_OFFSET);
    REG32(GPIO_BASE_ADDR+I2C_SCL_REG_OFFSET)&=~((I2C_GPIO_BIT_OUTPUT)<<I2C_GPIO_SCL_OFFSET);
}
void set_SDA( void ) // Actively drive SDA signal high
{
    REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)|=(I2C_GPIO_BIT_OUTPUT|I2C_GPIO_BIT_IO)<<I2C_GPIO_SDA_OFFSET;
}
void clear_SDA( void ) // Actively drive SDA signal low
{
    REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)|=((I2C_GPIO_BIT_IO)<<I2C_GPIO_SDA_OFFSET);
    REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)&=~((I2C_GPIO_BIT_OUTPUT)<<I2C_GPIO_SDA_OFFSET);
}
*/


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
  else if(INPUT == dir) {
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


void i2c_start_cond( void ) 
{
    if( started ) 
    { 
        // if started, do a restart cond
        // set SDA to 1
        set_gpio_output(SDA,1);
        I2C_delay();
		printk("in started_SDA_H\r\n");
    }

    // SCL is high, set SDA from 1 to 0.
    set_gpio_output(SDA,0);
    //I2C_delay();
	I2C_delay();
    set_gpio_output(SCL,0);
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
    set_gpio_output(SDA,1);
    init_gpio_dir(SDA,INPUT,0);
    //get_gpio_input(SDA);

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay();

    // Set SCL high to indicate a new valid SDA value is available
    set_gpio_output(SCL,1);

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    
	I2C_delay();
    // SCL is high, read out bit
    bit = get_gpio_input(SDA);
	init_gpio_dir(SDA,OUTPUT,0);
    // Set SCL low in preparation for next operation
    set_gpio_output(SCL,0);
	I2C_delay();
	




    return bit;

}

// Write a byte to I2C bus. Return 0 if ack by the slave.
INT8U i2c_write_byte_M( BOOLEAN send_start, BOOLEAN send_stop, INT8U byte) 
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
		wait_us_S(1);
    }

    //I2C_delay();
	
    nack = i2c_read_bit();

    if (send_stop) 
    {
        i2c_stop_cond();
    }

    return nack;
}

// Read a byte from I2C bus
INT8U i2c_read_byte_M( BOOLEAN nack , BOOLEAN send_stop ) 
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
void i2c_cp_write_byte(INT8U reg_addr, INT8U reg_data)
{
    INT8U timeout_cnt=0, w_nack;
    do{
        w_nack=i2c_write_byte_M(true, false, slaveaddr|BitW);
        if(w_nack==0)
        {
            i2c_write_byte_M(false, false, reg_addr);
            i2c_write_byte_M(false, true, reg_data);
        }
        else
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(w_nack&&(timeout_cnt<i2c_retry_maxcnt));
}
INT8U i2c_write_byte_retry( BOOLEAN send_start, BOOLEAN send_stop, INT8U byte)
{
    INT8U timeout_cnt=0, w_nack;
	OS_CPU_SR  cpu_sr = 0;
	
    do{
	    //OS_ENTER_CRITICAL();
        w_nack=i2c_write_byte_M(send_start, send_stop, byte);
		//OS_EXIT_CRITICAL();
        if(w_nack!=0)
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(w_nack&&(timeout_cnt<i2c_retry_maxcnt));

    return w_nack;
}

void i2c_cp_write_multibyte(INT8U reg_addr, INT16U length, INT8U *data_ptr)
{
    INT8U w_nack;
    INT16U i;

    w_nack=i2c_write_byte_retry(true, false, slaveaddr|BitW);
    if(w_nack==0)
    {
        w_nack=i2c_write_byte_retry(false, false, reg_addr);
        if(w_nack==0)
        {
            for(i=0;i<length;i++)
            {
                if(i==(length-1))
                    i2c_write_byte_retry(false, true, *data_ptr++);//last data
                else
                    i2c_write_byte_retry(false, false, *data_ptr++);
            }
        }
    }
}

void i2c_cp_read_multibyte(INT8U reg_addr, INT16U length, INT8U *data_ptr)
{
    INT8U timeout_cnt=0, w_nack, r_nack;
    INT16U i;
	OS_CPU_SR  cpu_sr = 0;
	
   // OS_ENTER_CRITICAL();
    do{
        w_nack=i2c_write_byte_M(true, false, slaveaddr|BitW);
        if(w_nack==0)
        {
            i2c_write_byte_M(false, true, reg_addr);
        }
        else
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(w_nack&&(timeout_cnt<i2c_retry_maxcnt));
	//OS_EXIT_CRITICAL();

    // if get timeout, abort this read operation
    if(timeout_cnt==i2c_retry_maxcnt)
        return;
	
    wait_us(500);
	
    //OS_ENTER_CRITICAL();
    timeout_cnt=0;
    do{
        r_nack=i2c_write_byte_M(true, false, slaveaddr|BitR);
        if(r_nack==0)
        {
            for(i=0;i<length;i++)
            {
                if(i==(length-1))
                    *data_ptr++=i2c_read_byte_M(true, true);//last data
                else
                    *data_ptr++=i2c_read_byte_M(false, false);
            }
        }
        else
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(r_nack&&(timeout_cnt<i2c_retry_maxcnt));
	//OS_EXIT_CRITICAL();
}

void  i2c_chargerIC_USB3750_reset()
{ 
	INT8U  reg_data;
	static BOOLEAN i2c_inited=0;

	//i2c_set_slave_addr(0x68<<1);
	i2c_cp_read_multibyte(0x01, 1, &reg_data);
	printk("%d\r\n",reg_data);
	reg_data=0x80;	  
	i2c_cp_write_multibyte(0x01, 1, &reg_data);
	OSTimeDly(30);//need to wait more than 20ms after reset
}





void I2C_gpio_init(void)
{
	init_gpio_dir(SDA,OUTPUT,1);
	init_gpio_dir(SCL,OUTPUT,1);
	int success = 1;
	int answer;
	printk("ENter GPIO_Write \r\n");

	
	OSTimeDly(900);


	

	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x21);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x00);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}
	OSTimeDly(200);



	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x21);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x02);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}

	OSTimeDly(200);



	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x21);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x04);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x43);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}

/*	OSTimeDly(200);


	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x41);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x00);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x32);
					success = i2c_write_byte_M(false,false,0x01);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}
	OSTimeDly(200);



	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x41);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x02);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x33);
					success = i2c_write_byte_M(false,false,0x33);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}

	OSTimeDly(200);



	success = i2c_write_byte_M(true,false,(0x68<<1 | 0) ); //0x6d<<1 | 0
	if(success == 0)
		{
		//printk("1\r\n");
		success = i2c_write_byte_M(false,false,0x41);
		if(success == 0)
			{
			//printk("2\r\n");
			success = i2c_write_byte_M(false,false,0xC0);
			if(success == 0)
				{
				//printk("3\r\n");
				success = i2c_write_byte_M(false,false,0x04);
				if(success == 0)
					{
					//printk("4\r\n");
					success = i2c_write_byte_M(false,false,0x35);
					success = i2c_write_byte_M(false,false,0x35);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,false,0x00);
					success = i2c_write_byte_M(false,true,0x00);
					//success = i2c_write_byte_M(false,true,0x00);
					}
				}
			}
		}
*/
/*
	success = i2c_write_byte_M(true,false, ((0x68<<1) | 0) );
	if(success == 0)
		{
		success = i2c_write_byte_M(false,false,0x01);
		if(success == 0)
			{
			success = i2c_write_byte_M(false,true,0x01);
			if(success == 0)
				printk("success_write");
			}
		}
*/
/*
		INT8U i;
		INT8U  reg_data;
		INT8U  reg_data_0x00 = 0;
		
		i2c_chargerIC_USB3750_reset();


		i2c_cp_read_multibyte(0x00, 1, &reg_data_0x00);
		printk("%d\r\n",reg_data_0x00);
		printk("%d\r\n",reg_data_0x00&0x10);
		if(reg_data_0x00&0x10)//det complete
		{
			switch (reg_data_0x00&0xE0)
			{
				case 0x20:
					printk("20");
					break;
				case 0x40:
					printk("40");
					break;
				case 0x60:
					printk("60\r\n");
					break;
				case 0x80:
					printk("80");
					break;
				case 0xA0:
					printk("A0");
					break;
				case 0xC0:
					printk("C0");
					break;
				default://0xe0, 0x00
					printk("00");
					break;
			}
		}
		i2c_cp_read_multibyte(0x01, 1, &reg_data_0x00);
		printk("%d\r\n",reg_data_0x00);
		i2c_cp_read_multibyte(0x02, 1, &reg_data_0x00);
		printk("%d\r\n",reg_data_0x00);
		i2c_cp_read_multibyte(0x03, 1, &reg_data_0x00);
		printk("%d\r\n",reg_data_0x00);
*/
/*
	success = i2c_write_byte_M(true,false, ((0x68<<1) | 0) );
	if(success == 0)
		{
		
		//printk("%d\r\n",success);
		success = i2c_write_byte_M(false,true,0x01);
		if(success == 0)
			{
			printk("%d\r\n",success);
			wait_us(500);
			success = i2c_write_byte_M(true,false,((0x68<<1) | 1) );
			if(success == 0)
				{
				success = i2c_read_byte_M(true,true);
				printk("%d\r\n",success);
				}
			}
		else
			{
			printk("1'\r\n");
			}
		}
	else
		{
		printk("1\r\n");
		}
*/


/*	if(success == 0)
		{
		printk("ACK address \r\n");
		success = i2c_write_byte_M(false,false,0x44);
		}
	if(success == 0)
		{
		printk("ACK 1 \r\n");
		success = i2c_write_byte_M(false,false,0xc0);	
		}
	if(success == 0)
		{
		printk("ACK 2 \r\n");
		success = i2c_write_byte_M(false,false,0x00);	
		}
	if(success == 0)
		{
		printk("ACK 3 \r\n");
		success = i2c_write_byte_M(false,false,0xFF);	
		}
	if(success == 0)
		{
		printk("ACK 4 \r\n");
		success = i2c_write_byte_M(false,false,0xFF);	
		}
	if(success == 0)
		{
		printk("ACK 5 \r\n");
		success = i2c_write_byte_M(false,false,0xFF);	
		}
	if(success == 0)
		{
		printk("ACK 6 \r\n");
		success = i2c_write_byte_M(false,false,0xFF);	
		}
	if(success == 0)
		{
		printk("ACK YA \r\n");
		success = i2c_write_byte_M(false,false,0xFF);	
		}
*/

	printk("ENter GPIO_Write_END \r\n");
}
