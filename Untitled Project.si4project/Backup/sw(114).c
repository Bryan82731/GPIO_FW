#include "bsp.h"

#include "lib.h"
#include "usb.h"
#include "sw.h"

extern DPCONF *dpconf;
extern USBCB usbcb;
INT8U *rdptr;
#ifdef CONFIG_POWER_SAVING_ENABLED
#define DYANMIC_CPU_SLOW
#endif

extern CONST INT8U   cert_accessory[1280];

// Hardware-specific support functions that MUST be customized:
#define GPIO_BASE_ADDR      0xB2000500
#define I2C_SDA_REG_OFFSET  0
#define I2C_SCL_REG_OFFSET  0
//for redpart gpio
#define I2C_GPIOa_REG_OFFSET  4
#define I2C_GPIOb_REG_OFFSET  4
#define I2C_GPIOc_REG_OFFSET  4
#define I2C_GPIOd_REG_OFFSET  4


#define I2C_GPIO_SDA_OFFSET 8//UART TX
#define I2C_GPIO_SCL_OFFSET 12//UART RX

#define I2C_GPIO_GPIOa_OFFSET 4
#define I2C_GPIO_GPIOb_OFFSET 0
#define I2C_GPIO_GPIOc_OFFSET 12
#define I2C_GPIO_GPIOd_OFFSET 8


//bit 0:gpio en, bit 1:O/I, bit2:output, bit3:input
#define I2C_GPIO_BIT_EN     (1<<0)
#define I2C_GPIO_BIT_IO     (1<<1)//HIGH is output
#define I2C_GPIO_BIT_OUTPUT (1<<2)
#define I2C_GPIO_BIT_INTPUT (1<<3)

#define I2CSPEED 100
INT8U slaveaddr;
INT8U BitR=1;
INT8U BitW=0;
INT32U max_reg_retry_cnt=100;
INT32U i2c_retry_period=100;
INT32U i2c_retry_maxcnt=10;
BOOLEAN started = false; // global data
INT8U cd_data[20]={
              0xE6,0x0C,0xA9,0x0A,0xD9,0xBE,0x3D,0x1D,
              0x4A,0xBD,0x92,0x29,0x71,0x1A,0x33,0xF0,
              0x94,0xF8,0xD2,0x9F};
INT8U read_data[128], /*challenge_resp_data[128], challenge_data[128],*/ cert_data[128], apple_cert_data[128], serial_data[31];
extern CONST INT8U   challenge_data[128];
extern CONST INT8U   challenge_resp_data[128];
INT8U write_data[128]={
                        0xE6,0x0C,0xA9,0x0A,0xD9,0xBE,0x3D,0x1D,
                        0x4A,0xBD,0x92,0x29,0x71,0x1A,0x33,0xF0,
                        0x94,0xF8,0xD2,0x9F,0x15,0x16,0x17,0x18,
                        0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,
                        0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,
                        0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,
                        0x51,0x52,0x53,0x54,0x55,0x56,0x57,0x58,
                        0x61,0x62,0x63,0x64,0x65,0x66,0x67,0x68,
                        0x71,0x72,0x73,0x74,0x75,0x76,0x77,0x78,
                        0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,
                        0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,
                        0xa1,0xa2,0xa3,0xa4,0xa5,0xa6,0xa7,0xa8,
                        0xb1,0xb2,0xb3,0xb4,0xb5,0xb6,0xb7,0xb8,
                        0xc1,0xc2,0xc3,0xc4,0xc5,0xc6,0xc7,0xc8,
                        0xd1,0xd2,0xd3,0xd4,0xd5,0xd6,0xd7,0xd8,
                        0xe1,0xe2,0xe3,0xe4,0xe5,0xe6,0xe7,0xe8};
INT8U reg=0x0;
INT16U data_length=1;
INT16U us_time=20;

void I2C_delay( void );
//wait 1 us in cpu slow
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

void i2c_start_cond( void ) 
{
    if( started ) 
    { 
        // if started, do a restart cond
        // set SDA to 1
        set_SDA();
        I2C_delay();
    }

    // SCL is high, set SDA from 1 to 0.
    clear_SDA();
    I2C_delay();
    I2C_delay();
    clear_SCL();
    started = true;
}
void i2c_stop_cond( void )
{
    // set SDA to 0
    clear_SDA();
    I2C_delay();

    I2C_delay();
    set_SCL();
    I2C_delay();

    // Stop bit setup time, minimum 4us
    I2C_delay();

    // SCL is high, set SDA from 0 to 1
    set_SDA();
    I2C_delay();

    I2C_delay();
    started = false;
}

// Write a bit to I2C bus
void i2c_write_bit( INT8U bit ) 
{ 
    if( bit ) 
    {
        set_SDA();
    } 
    else 
    {
        clear_SDA();
    }

    // SDA change propagation delay
    I2C_delay();  

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL();

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay();

    // Clear the SCL to low in preparation for next change
    clear_SCL();
}

// Read a bit from I2C bus
INT8U i2c_read_bit( void ) 
{
    INT8U bit;

    // bossino, release clk, necessary for APPLE ACP
    read_SDA();

    // Wait for SDA value to be written by slave, minimum of 4us for standard mode
    I2C_delay();

    // Set SCL high to indicate a new valid SDA value is available
    set_SCL();

    // Wait for SDA value to be read by slave, minimum of 4us for standard mode
    I2C_delay();
    // SCL is high, read out bit
    bit = read_SDA();

    // Set SCL low in preparation for next operation
    clear_SCL();

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

//delay 1 clk_cycle
void I2C_delay()
{
    wait_us((1000/I2CSPEED)/2);
}

void i2c_init()
{
    INT32U value=0;

    value=(I2C_GPIO_BIT_EN|I2C_GPIO_BIT_IO|I2C_GPIO_BIT_OUTPUT)<<I2C_GPIO_SDA_OFFSET;
    //set SDA gpio output high
    REG32(GPIO_BASE_ADDR+I2C_SDA_REG_OFFSET)|=value;

    value=(I2C_GPIO_BIT_EN|I2C_GPIO_BIT_IO|I2C_GPIO_BIT_OUTPUT)<<I2C_GPIO_SCL_OFFSET;
    //set SCL gpio output high
    REG32(GPIO_BASE_ADDR+I2C_SCL_REG_OFFSET)|=value;
    slaveaddr=0x11<<1;
}
static Charger_type det_type=charger_type_noconnect;

void redpark_gpio_init()
{
    INT32U value=0;
	
    //set GPIOa input
    value=REG32(GPIO_BASE_ADDR+I2C_GPIOa_REG_OFFSET);
    value|=I2C_GPIO_BIT_EN<<I2C_GPIO_GPIOa_OFFSET;//set gpio enable
	value&=~(I2C_GPIO_BIT_IO<<I2C_GPIO_GPIOa_OFFSET);//set input
    REG32(GPIO_BASE_ADDR+I2C_GPIOa_REG_OFFSET)=value;

    //set GPIOb output
    value=REG32(GPIO_BASE_ADDR+I2C_GPIOb_REG_OFFSET);
    value|=I2C_GPIO_BIT_EN<<I2C_GPIO_GPIOb_OFFSET;//set gpio enable
	value|=(I2C_GPIO_BIT_IO<<I2C_GPIO_GPIOb_OFFSET);//set output
	value&=~(I2C_GPIO_BIT_OUTPUT<<I2C_GPIO_GPIOb_OFFSET);//set output low
    REG32(GPIO_BASE_ADDR+I2C_GPIOb_REG_OFFSET)=value;

	//set GPIOc input
	value=REG32(GPIO_BASE_ADDR+I2C_GPIOc_REG_OFFSET);
	value|=I2C_GPIO_BIT_EN<<I2C_GPIO_GPIOc_OFFSET;//set gpio enable
	value&=~(I2C_GPIO_BIT_IO<<I2C_GPIO_GPIOc_OFFSET);//set input
	REG32(GPIO_BASE_ADDR+I2C_GPIOc_REG_OFFSET)=value;

	//set GPIOd input
	value=REG32(GPIO_BASE_ADDR+I2C_GPIOd_REG_OFFSET);
	value|=I2C_GPIO_BIT_EN<<I2C_GPIO_GPIOd_OFFSET;//set gpio enable
	value&=~(I2C_GPIO_BIT_IO<<I2C_GPIO_GPIOd_OFFSET);//set input
	REG32(GPIO_BASE_ADDR+I2C_GPIOd_REG_OFFSET)=value;
}

Dongle_type redpark_product_type_det()
{
    volatile INT32U gpioa_value, gpioc_value, gpiod_value;
    
	//get GPIOa
	gpioa_value=REG32(GPIO_BASE_ADDR+I2C_GPIOa_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_GPIOa_OFFSET?1:0;
	//get GPIOc
	gpioc_value=REG32(GPIO_BASE_ADDR+I2C_GPIOc_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_GPIOc_OFFSET?1:0;
	//get GPIOd
	gpiod_value=REG32(GPIO_BASE_ADDR+I2C_GPIOd_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_GPIOd_OFFSET?1:0;

	if(gpioa_value == 0x00)
		return dongle_redpark_L6_NETPOE;
	else if(gpioa_value == 0x01 && gpioc_value == 0x00 && gpiod_value == 0x00)
		return dongle_redpark_L5_NET;
	else
		return dongle_redpark_L6_NETAC;
}

Charger_type redpark_charger_det()
{
    INT32U gpioc_value, gpiod_value;

	//get GPIOc
	gpioc_value=REG32(GPIO_BASE_ADDR+I2C_GPIOc_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_GPIOc_OFFSET?1:0;
	//get GPIOd
	gpiod_value=REG32(GPIO_BASE_ADDR+I2C_GPIOd_REG_OFFSET)&(I2C_GPIO_BIT_INTPUT)<<I2C_GPIO_GPIOd_OFFSET?1:0;

	if((gpioc_value == 0x00)&&(gpiod_value == 0x00))
		det_type=charger_type_noconnect;
	else if((gpioc_value == 0x00)&&(gpiod_value == 0x01))
		det_type=charger_type_apple_1a;
	else if((gpioc_value == 0x01)&&(gpiod_value == 0x00))
		det_type=charger_type_apple_2d1a;
	else if((gpioc_value == 0x01)&&(gpiod_value == 0x01))
		det_type=charger_type_apple_2d4a;

	return det_type;
}

void redpark_enum_done()
{
    INT32U value=0;
	
    //set GPIOb output
    value=REG32(GPIO_BASE_ADDR+I2C_GPIOb_REG_OFFSET);
	value|=(I2C_GPIO_BIT_OUTPUT<<I2C_GPIO_GPIOb_OFFSET);//set output high
    REG32(GPIO_BASE_ADDR+I2C_GPIOb_REG_OFFSET)=value;
}

void redpark_test()
{
	Dongle_type tmp_dongle=dongle_default;
	Charger_type tmp_type;
	
	redpark_gpio_init();
	tmp_dongle=redpark_product_type_det();

	if(tmp_dongle==dongle_redpark_L6_NETPOE)
	{
		tmp_type=redpark_charger_det();
		if(tmp_type==charger_type_apple_2d4a)
			redpark_enum_done();
	}
	else if(tmp_dongle==dongle_redpark_L6_NETAC)
	{
		tmp_type=redpark_charger_det();
		if(tmp_type==charger_type_apple_1a)
			redpark_enum_done();
	}
}

void i2c_cp_write_byte(INT8U reg_addr, INT8U reg_data)
{
    INT8U timeout_cnt=0, w_nack;
    do{
        w_nack=i2c_write_byte(true, false, slaveaddr|BitW);
        if(w_nack==0)
        {
            i2c_write_byte(false, false, reg_addr);
            i2c_write_byte(false, true, reg_data);
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
	    OS_ENTER_CRITICAL();
        w_nack=i2c_write_byte(send_start, send_stop, byte);
		OS_EXIT_CRITICAL();
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
	
    OS_ENTER_CRITICAL();
    do{
        w_nack=i2c_write_byte(true, false, slaveaddr|BitW);
        if(w_nack==0)
        {
            i2c_write_byte(false, true, reg_addr);
        }
        else
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(w_nack&&(timeout_cnt<i2c_retry_maxcnt));
	OS_EXIT_CRITICAL();

    // if get timeout, abort this read operation
    if(timeout_cnt==i2c_retry_maxcnt)
        return;
	
    wait_us(500);
	
    OS_ENTER_CRITICAL();
    timeout_cnt=0;
    do{
        r_nack=i2c_write_byte(true, false, slaveaddr|BitR);
        if(r_nack==0)
        {
            for(i=0;i<length;i++)
            {
                if(i==(length-1))
                    *data_ptr++=i2c_read_byte(true, true);//last data
                else
                    *data_ptr++=i2c_read_byte(false, false);
            }
        }
        else
        {
            timeout_cnt++;
            i2c_stop_cond();
            wait_us(i2c_retry_period);
        }
    }while(r_nack&&(timeout_cnt<i2c_retry_maxcnt));
	OS_EXIT_CRITICAL();
}

void i2c_test_read()
{
    static BOOLEAN i2c_inited=0;
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    } 
    i2c_cp_read_multibyte(reg, data_length, read_data);
}
void i2c_test_write()
{
    static BOOLEAN i2c_inited=0;
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    }

    i2c_cp_write_multibyte(reg, data_length, write_data);
}

void i2c_test_writeread()
{
    static BOOLEAN i2c_inited=0;
    INT8U i;
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    }

    for(i=0;i<data_length;i++)
        read_data[i]=0x00;//clear 
    
    i2c_cp_write_multibyte(reg, data_length, write_data);
    i2c_cp_read_multibyte(reg, data_length, read_data);

    for(i=0;i<data_length;i++)
    {
        if(read_data[i]!=write_data[i])
            return;//error
    }
}

void i2c_test_all()
{   
    static BOOLEAN i2c_inited=0;
    INT8U *data_ptr=read_data;
    INT16U cert_length, challenge_resp_length, challenge_length, apple_cert_length;
    
    rdptr = malloc(1536);	
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    } 
    reg=0x0;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x1;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x2;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x3;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x4;
    data_length=4;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;
    
    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x10;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    //read challenge response data length & test W/R data
    reg=0x11;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    challenge_resp_length=(*data_ptr);
    challenge_resp_length<<=8;
    challenge_resp_length|=(*(data_ptr+1));
    data_ptr+=data_length;

    reg=0x12;
    data_length=challenge_resp_length;
    i2c_cp_write_multibyte(reg, data_length, write_data);
    i2c_cp_read_multibyte(reg, data_length, challenge_resp_data);

    //read challenge data length & test W/R data
    reg=0x20;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    challenge_length=(*data_ptr);
    challenge_length<<=8;
    challenge_length|=(*(data_ptr+1));
    data_ptr+=data_length;

    reg=0x21;
    data_length=challenge_length;
    i2c_cp_write_multibyte(reg, data_length, write_data);
    i2c_cp_read_multibyte(reg, data_length, challenge_data);

    //read certificate data
    reg=0x30;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    cert_length=(*data_ptr);
    cert_length<<=8;
    cert_length|=(*(data_ptr+1));
    data_ptr+=data_length;

    if(cert_length)
    {
        //read first 128 byte
        reg=0x31;
        data_length=128;
        i2c_cp_read_multibyte(reg, data_length, cert_data);
    }

    //read serial number
    reg=0x4E;
    data_length=31;
    i2c_cp_read_multibyte(reg, data_length, serial_data);

    //read apple device certificate data length & test W/R data
    reg=0x50;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    apple_cert_length=(*data_ptr);
    apple_cert_length<<=8;
    apple_cert_length|=(*(data_ptr+1));
    data_ptr+=data_length;

    if(apple_cert_length)
    {
        //test first 128 byte
        reg=0x51;
        data_length=128;
        i2c_cp_write_multibyte(reg, data_length, write_data);
        i2c_cp_read_multibyte(reg, data_length, apple_cert_data);
    }
}

void i2c_cert_data()
{   
    static BOOLEAN i2c_inited=0;
    INT8U *data_ptr=read_data;
    INT16U cert_length, challenge_resp_length, challenge_length, apple_cert_length;
    
    rdptr = malloc(1536);	
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    } 
i2c_set_slave_addr(0x11<<1);
    
    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    //read certificate data
    reg=0x30;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    cert_length=(*data_ptr);
    cert_length<<=8;
    cert_length|=(*(data_ptr+1));
    data_ptr+=data_length;
    
    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;
    
    if(cert_length)
    {
        //read first 128 byte
        reg=0x31;
        data_length=128;
        //i2c_cp_read_multibyte(reg, data_length, cert_data);
        dpconf->certlen = cert_length;
        memset(rdptr, 0 , sizeof(rdptr));
        do{
        		
        	i2c_cp_read_multibyte(reg, data_length, cert_data);
        	memcpy(rdptr, cert_data, data_length);	
        	reg++;
        	cert_length -= 128;
        	rdptr += 128; 

        }while( cert_length > 128);
        
        if(cert_length<128){
        		data_length=cert_length;
        		i2c_cp_read_multibyte(reg, data_length, cert_data);
        		memcpy(rdptr, cert_data, data_length);
        }
        memcpy(cert_accessory, rdptr - (dpconf->certlen-cert_length), dpconf->certlen);
        
        data_length=1;
        i2c_cp_read_multibyte(reg, data_length, data_ptr);
        data_ptr+=data_length;
    }
    
    //read first 128 byte: offset 0
    reg=0x31;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;
}

void i2c_authentication()
{   
    static BOOLEAN i2c_inited=0;
    
    INT8U  *data_ptr=read_data;
    INT8U  proc_results, loop_cnt=0;
    INT16U challenge_resp_length, challenge_length;
    
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    }
    
    //read challenge data length
    reg=0x20;
    data_length=2;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    challenge_length=(*data_ptr);
    challenge_length<<=8;
    challenge_length|=(*(data_ptr+1));
    data_ptr+=data_length;
    dpconf->cdlen = challenge_length;

    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    //write challenge data length
    reg=0x20;
    data_length=2;
    (*data_ptr) = (INT8U)challenge_length>>8;//(*data_ptr) = dpconf->cdlen;
    (*(data_ptr+1)) = (INT8U)challenge_length;//((*(data_ptr+1)) = dpconf->cdlen>>8;
    i2c_cp_write_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;
    
    //write challenge data
    reg=0x21;
    data_length=challenge_length;//dpconf->cdlen;
    i2c_cp_write_multibyte(reg, data_length, challenge_data);//write_data);
    
    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;
    
    //write authentication control
    reg=0x10;
    data_length=1;
    (*data_ptr)=1;
    i2c_cp_write_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length;

    
    //read authentication control    
    do
    {
        reg=0x10;
        data_length=1;
        i2c_cp_read_multibyte(reg, data_length, data_ptr);
        proc_results=*data_ptr;
        data_ptr+=data_length;
        wait_us(1000);
        loop_cnt++;
    }while((proc_results==0)&&loop_cnt<max_reg_retry_cnt);
    
   if((proc_results&0x70) == 0x10){
    	data_ptr+=data_length;	  	
    	//read challenge response read back data length / data
    	reg=0x11;
    	data_length=2;
    	i2c_cp_read_multibyte(reg, data_length, data_ptr);
    	challenge_resp_length=(*data_ptr);
    	challenge_resp_length<<=8;
    	challenge_resp_length|=(*(data_ptr+1));
    	data_ptr+=data_length;
    	dpconf->crlen=challenge_resp_length;

    	reg=0x12;
    	data_length=challenge_resp_length;
    	//i2c_cp_write_multibyte(reg, data_length, write_data);
    	i2c_cp_read_multibyte(reg, data_length, challenge_resp_data);
  	}
   else if((proc_results&0x70) == 0x20){
    	data_ptr+=data_length;	  	
    	//ToDo:Challenge successfully generated
    	reg=0x0;
  	}
   else if((proc_results&0x70) == 0x30){
    	data_ptr+=data_length;	  	
    	//ToDo:Apple device challenge response successfully verified
    	reg=0x0;
  	}
   else if((proc_results&0x70) == 0x40){
    	data_ptr+=data_length;	  	
    	//ToDo:Apple device certificate successfully validated
    	reg=0x0;
  	}
  	//clear error status
   else if(proc_results&0x80){
    	data_ptr+=data_length;
    	reg=0x5;
    	data_length=1;
    	i2c_cp_read_multibyte(reg, data_length, data_ptr);
    	data_ptr+=data_length; 	
  	}
    
    reg=0x5;
    data_length=1;
    i2c_cp_read_multibyte(reg, data_length, data_ptr);
    data_ptr+=data_length; 	
}

void i2c_set_slave_addr(INT8U i2c_slaveaddr)
{
    slaveaddr=i2c_slaveaddr;
}

void i2c_adc_get(INT16U *adc_value, INT8U adc_port)
{
    INT8U data[2];
    INT16U adc_ctl_reg;

    i2c_set_slave_addr(0x48<<1);

    //For ads1015, data[0] is high byte, data[1] is low byte
    i2c_cp_read_multibyte(1, 2, &data);
    adc_ctl_reg=(data[0]<<8)+data[1];
    //set detect range
    adc_ctl_reg&=(~(0x0E00));//6.144V
    adc_ctl_reg|=(0x0200);//4.09V
    
    //set detect channel
    adc_ctl_reg&=(~(0x7000));
    adc_ctl_reg|=(INT16U)((adc_port|0x4)&0x7)<<12;///GND=AINn
    //adc_ctl_reg|=(INT16U)((adc_port)&0x7)<<12;//AIN3=AINn
    adc_ctl_reg|=0x8000;// Begin a single conversion 
    data[0]=(INT8U)(adc_ctl_reg>>8);
    data[1]=(INT8U)(adc_ctl_reg);
    i2c_cp_write_multibyte(1, 2, data);
	
	//wait until conversion complete
	do{
		OSTimeDly(1);
		i2c_cp_read_multibyte(1, 2, &data);
		*adc_value=(data[0]<<8)+data[1];
	}while((*adc_value&0x8000)!=0x8000);
	//get conversion data
    i2c_cp_read_multibyte(0, 2, &data);
    *adc_value=(data[0]<<8)+data[1];
}
void i2c_adc_get_continuous(INT16U *adc_value, INT8U adc_port)
{
    INT8U data[2];
    INT16U adc_ctl_reg, channel;

    i2c_set_slave_addr(0x48<<1);

    //For ads1015, data[0] is high byte, data[1] is low byte
    i2c_cp_read_multibyte(1, 2, &data);
    adc_ctl_reg=(data[0]<<8)+data[1];
    //set detect range
    adc_ctl_reg&=(~(0x0E00));//6.144V
    adc_ctl_reg|=(0x0200);//4.09V    
    //set detect channel
    adc_ctl_reg&=(~(0x7000));
    channel=(INT16U)((adc_port|0x4)&0x7)<<12;///GND=AINn
    adc_ctl_reg|=channel;
    // set conversion method:continuous 
    adc_ctl_reg&=(~(0x0100));
    // set data rate:3300SPS 
    adc_ctl_reg|=(0x00E0);	
    data[0]=(INT8U)(adc_ctl_reg>>8);
    data[1]=(INT8U)(adc_ctl_reg);
    i2c_cp_write_multibyte(1, 2, data);
	
	//wait until conversion complete
	do{
		OSTimeDly(1);
		i2c_cp_read_multibyte(1, 2, &data);
		*adc_value=(data[0]<<8)+data[1];
	}while((*adc_value&0x7000)!=channel);
	//get conversion data
    i2c_cp_read_multibyte(0, 2, &data);
    *adc_value=(data[0]<<8)+data[1];
}

void i2c_adc_get_simple(INT16U *adc_value, INT8U adc_port)
{
    INT8U data[2], timeout=0;
    INT16U adc_ctl_reg;

    i2c_set_slave_addr(0x48<<1);

    //For ads1015, data[0] is high byte, data[1] is low byte
    i2c_cp_read_multibyte(1, 2, &data);
    adc_ctl_reg=(data[0]<<8)+data[1];
    
    //set detect channel
    adc_ctl_reg&=(~(0x7000));
    adc_ctl_reg|=(INT16U)((adc_port|0x4)&0x7)<<12;///GND=AINn
    data[0]=(INT8U)(adc_ctl_reg>>8);
    data[1]=(INT8U)(adc_ctl_reg);
    i2c_cp_write_multibyte(1, 2, data);
	
	//wait until conversion complete
	do{
		OSTimeDly(1);
		i2c_cp_read_multibyte(1, 2, &data);
		*adc_value=(data[0]<<8)+data[1];
	}while(((*adc_value&0x8000)!=0x8000)&&(timeout++<100));
	//get conversion data
    i2c_cp_read_multibyte(0, 2, &data);
    *adc_value=(data[0]<<8)+data[1];
}
void i2c_adc_init(INT8U adc_port)
{
    INT8U data[2];
    INT16U adc_ctl_reg;

    i2c_set_slave_addr(0x48<<1);

    //For ads1015, data[0] is high byte, data[1] is low byte
    i2c_cp_read_multibyte(1, 2, &data);
    adc_ctl_reg=(data[0]<<8)+data[1];
    //set detect range
    adc_ctl_reg&=(~(0x0E00));//6.144V
    adc_ctl_reg|=(0x0200);//4.09V
    
    //set detect channel
    adc_ctl_reg&=(~(0x7000));
    adc_ctl_reg|=(INT16U)((adc_port|0x4)&0x7)<<12;///GND=AINn
    //adc_ctl_reg|=(INT16U)((adc_port)&0x7)<<12;//AIN3=AINn
    adc_ctl_reg&=~(0x0100);// set continue conversion
    
    adc_ctl_reg&=~(0x00e0);//set data rate 128sps
    //adc_ctl_reg|=(0x00e0);//set data rate 3300sps
    data[0]=(INT8U)(adc_ctl_reg>>8);
    data[1]=(INT8U)(adc_ctl_reg);
    i2c_cp_write_multibyte(1, 2, data);
}

// after calling i2c_charger_det_reset, should delay 200ms
void  i2c_charger_det_reset()
{ 
    INT8U  reg_data;
    static BOOLEAN i2c_inited=0;
	
    if(i2c_inited==0)
    {
        i2c_init();
        i2c_inited=1;
    }    

    i2c_set_slave_addr(0x25<<1);
    reg_data=0x01;    
    i2c_cp_write_multibyte(0x1b, 1, &reg_data);
    OSTimeDly(1);
    reg_data=0x00;    
    i2c_cp_write_multibyte(0x1b, 1, &reg_data);
    OSTimeDly(20);
   // wait_us(200000);

}

Charger_type  i2c_charger_det()
{ 
    INT8U  reg_data_0x0A, reg_data_0x0E, reg_data_0x03, reg_data_0x05=0x03;
    INT16U adc_dp_value = 0;
    INT16U adc_dm_value = 0;
    static BOOLEAN i2c_chargerIC_reset=0;    
//OS_CPU_SR  cpu_sr = 0;            
    i2c_set_slave_addr(0x25<<1);

    if(i2c_chargerIC_reset==0)
    {
        i2c_init();

        //toggle software reset
        #if 0
        reg_data_0x1B=0x01;    
        i2c_cp_write_multibyte(0x1b, 1, &reg_data_0x1B);
        OSTimeDly(1); 
        reg_data_0x1B=0x00;    
        i2c_cp_write_multibyte(0x1b, 1, &reg_data_0x1B);
        OSTimeDly(20);//OSTimeDlyHMSM(0,0,0,200);//delay 200ms
        #else
        i2c_charger_det_reset();
        #endif
        i2c_cp_write_multibyte(0x05, 1, &reg_data_0x05);
        i2c_chargerIC_reset=1;
    }
    //bossino set always true for power on detection read reg_03 get zero issue.
//OS_ENTER_CRITICAL();
    //i2c_cp_read_multibyte(0x03, 1, &reg_data_0x03);
    //if(reg_data_0x03&0x03)
    if(1)
    {
        i2c_cp_read_multibyte(0x0a, 1, &reg_data_0x0A);
        i2c_cp_read_multibyte(0x0e, 1, &reg_data_0x0E);

        if(reg_data_0x0A & 0x04)
            det_type=charger_type_usb_SDP;
        if(reg_data_0x0A & 0x20)
            det_type=charger_type_usb_CDP;
        if(reg_data_0x0A & 0x40)
            det_type=charger_type_usb_DCP;
        
        if(reg_data_0x0E & 0x04)
            det_type=charger_type_apple_1a;
        if(reg_data_0x0E & 0x08)
            det_type=charger_type_apple_2d1a;
        if(reg_data_0x0E & 0x10)
            det_type=charger_type_apple_2d4a;
        
        if((reg_data_0x0A==0)&&(reg_data_0x0E==0 ))
            det_type=charger_type_noconnect;

        i2c_adc_get(&adc_dp_value, 0);
        i2c_adc_get(&adc_dm_value, 1);
        //OS_EXIT_CRITICAL();
        //fix the apple chager type error while dp=dm>1.8V
        if(det_type==charger_type_usb_DCP)
        {
            if((adc_dp_value>0x3000)||(adc_dm_value>0x3000))//0x2580 for FS=6.144V, 0x3840 for FS=4.096V
                det_type=charger_type_apple_0d5a;
        }

        //fix the apple chager type error while dp or dm is over 3V
        if((det_type==charger_type_apple_2d1a)|| \
               (det_type==charger_type_apple_2d4a))
        {
            if((adc_dp_value>0x5d00)||(adc_dm_value>0x5d00))//0x3e80 for FS=6.144V, 0x5d00 for FS=4.096V
                det_type=charger_type_apple_1a;
        }
    }
    return det_type;
}

void  i2c_chargerIC_USB3750_reset()
{ 
	INT8U  reg_data;
	static BOOLEAN i2c_inited=0;
	if(i2c_inited==0)
	{
		i2c_init();
		i2c_inited=1;
	}	 
	i2c_set_slave_addr(0x68<<1);
	i2c_cp_read_multibyte(0x01, 1, &reg_data);
	reg_data=0x80;	  
	i2c_cp_write_multibyte(0x01, 1, &reg_data);
	OSTimeDly(3);//need to wait more than 20ms after reset
}
void  i2c_chargerIC_USB3750_setmux(INT8U mux_num)
{ 
	INT8U  reg_data;
	static BOOLEAN i2c_inited=0;
	if(i2c_inited==0)
	{
		i2c_init();
		i2c_inited=1;
	}	 
	i2c_set_slave_addr(0x68<<1);
	i2c_cp_read_multibyte(0x01, 1, &reg_data);
	if(reg_data!=(mux_num<<5))
	{
		reg_data&=~(0x60);
		reg_data|=mux_num<<5;	  
		i2c_cp_write_multibyte(0x01, 1, &reg_data);
	}
}
Charger_type  i2c_chargerIC_USB3750_det()
{ 
    INT8U  reg_data_0x00 = 0;
    INT16U adc_dp_value = 0;
    INT16U adc_dm_value = 0;
    static BOOLEAN i2c_chargerIC_reset=0;
	static Charger_type lasttime_det_type=charger_type_noconnect;
	
    if(i2c_chargerIC_reset==0)
    {
        i2c_init();
        i2c_chargerIC_USB3750_reset();
		i2c_chargerIC_reset=1;

		//i2c_adc_init(0);
		//i2c_adc_init(1);
    }
	
    i2c_set_slave_addr(0x68<<1);
    i2c_cp_read_multibyte(0x00, 1, &reg_data_0x00);

	if(reg_data_0x00&0x10)//det complete
	{
		switch (reg_data_0x00&0xE0)
		{
			case 0x20:
				det_type=charger_type_usb_DCP;
				break;
			case 0x40:
				det_type=charger_type_usb_CDP;
				break;
			case 0x60:
				det_type=charger_type_usb_SDP;
				break;
			case 0x80:
				det_type=charger_type_apple_0d5a;
				break;
			case 0xA0:
				det_type=charger_type_apple_1a;
				break;
			case 0xC0:
				det_type=charger_type_apple_2d1a;
				break;
			default://0xe0, 0x00
				det_type=charger_type_noconnect;
				break;
		}
		
		if((det_type==charger_type_apple_0d5a)||
			(det_type==charger_type_apple_1a)||
			(det_type==charger_type_apple_2d1a))
		{
			i2c_adc_get_continuous(&adc_dp_value, 0);
			i2c_adc_get_continuous(&adc_dm_value, 1);				
			
			if(det_type==charger_type_apple_1a)
			{
				if(((adc_dp_value>0x47E0)&&(adc_dp_value<0x5DC0))//2.3(2.7)<DP<3(3.3)V
				   &&((adc_dm_value>0x47E0)&&(adc_dm_value<0x5DC0)))//2.3(2.7)<DM<3(3.3)V
					det_type=charger_type_apple_2d4a;
			}
			else if(det_type==charger_type_apple_2d1a)
			{
				if(adc_dp_value>0x5AA0)//2.9(3.3)V<DP
					det_type=charger_type_apple_1a;
			}
		}
		
		if(det_type==lasttime_det_type)
		{
		}
		else
		{
			if(det_type==charger_type_usb_SDP)
			{
				i2c_chargerIC_USB3750_setmux(2);//Connected to PC or Mac
				OSTimeDly(10);
			}
			else
			{
				i2c_chargerIC_USB3750_setmux(1);//Connected to ADC detect
			}
		}
	}
	else
	{
        det_type=charger_type_noconnect;
	}

	lasttime_det_type=det_type;
    return det_type;
}