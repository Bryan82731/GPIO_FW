#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC_COMMON.h"
#include "usb.h"


#define RST_TCA	GPO
#define SDA GPIO2                         //定义SDA所对应的GPIO接口编号 
#define SCL GPIO1                         //定义SCL所对应的GPIO接口编号 
#define OUTP OUTPUT                       //表示GPIO接口方向为输出 
#define INP  INPUT                        //表示GPIO接口方向为输入 
#define HALF_CLOCK 5

#define set_gpio_direction(gpio_num , dir ) OOBMAC_gpio_init(gpio_num,dir,1)
#define set_gpio_value(gpio_num, val) OOBMAC_gpio_output(gpio_num,val)
#define get_gpio_value(gpio_num)	OOBMAC_gpio_input(gpio_num)
#define delay() bsp_wait(HALF_CLOCK)


/* I2C起始条件 */ 
int i2c_start() 
{ 
//初始化GPIO口 
set_gpio_direction(SDA, OUTP);          //设置SDA方向为输出 
set_gpio_direction (SCL, OUTP);         //设置SCL方向为输出 
set_gpio_value(SDA, 1);                //设置SDA为高电平 
set_gpio_value(SCL, 1);                 //设置SCL为高电平 
delay();                            //延时 
//起始条件 
set_gpio_value(SDA, 0);                 //SCL为高电平时，SDA由高变低 
delay(); 
} 


/* I2C终止条件 */ 
void i2c_stop() 
{ 
set_gpio_value(SCL, 1); 
set_gpio_direction(SDA, OUTP); 
set_gpio_value(SDA, 0); 
delay(); 
set_gpio_value(SDA, 1);             //SCL高电平时，SDA由低变高 
}

 
/*  
I2C读取ACK信号(写数据时使用) 
返回值 ：0表示ACK信号有效；非0表示ACK信号无效 
*/ 
unsigned char i2c_read_ack() 
{ 
unsigned char r; 

set_gpio_value(SCL,0);              // SCL变低 
set_gpio_direction(SDA, INP);           //设置SDA方向为输入 
r = get_gpio_value(SDA);                //读取ACK信号 
delay(); 
set_gpio_value(SCL,1);              // SCL变高 
delay(); 
set_gpio_value(SCL,0);              // SCL变di  
return r; 
} 


/* I2C发出ACK信号(读数据时使用) */ 
int i2c_send_ack() 
{ 

set_gpio_value(SCL,0);              // SCL变低 
set_gpio_direction(SDA, OUTP);          //设置SDA方向为输出 
set_gpio_value(SDA, 0);             //发出ACK信号 
delay(); 
set_gpio_value(SCL,1);              // SCL变高 
delay(); 
} 

int i2c_send_nack() 
{ 

set_gpio_value(SCL,0);              // SCL变低 
set_gpio_direction(SDA, OUTP);          //设置SDA方向为输出 
set_gpio_value(SDA, 1);             //发出NACK信号 
delay(); 
set_gpio_value(SCL,1);              // SCL变高 
delay(); 
} 



/* I2C字节写 */ 
void i2c_write_byte(unsigned char b) 
{ 
int i;
INT8U ack_val ; 

// change out dir will change sda data , so place SCL = 0 ,the change dir 
set_gpio_value(SCL, 0);             // SCL变低 
set_gpio_direction(SDA, OUTP);          //设置SDA方向为输出 

for (i=7; i>=0; i--) { 
set_gpio_value(SCL, 0);             // SCL变低 
delay(); 
set_gpio_value(SDA, (b >> i)&0x01);        //从高位到低位依次准备数据进行发送 
set_gpio_value(SCL, 1);             // SCL变高 
delay(); 
} 
ack_val = i2c_read_ack();                 //检查目标设备的ACK信�
if (ack_val) printf ("ACK ERROR");
} 


/* I2C字节读 */ 
unsigned char i2c_read_byte(BOOLEAN end_of_nak ) 
{ 
int i; 
unsigned char r = 0; 
set_gpio_direction(SDA, INP);           //设置SDA方向为输入 
for (i=7; i>=0; i--) { 
set_gpio_value(SCL, 0);         // SCL变低 
delay(); 
r = (r <<1) | get_gpio_value(SDA);      //从高位到低位依次准备数据进行读取 
set_gpio_value(SCL, 1);         // SCL变高 
delay(); 
} 
if (end_of_nak){
	i2c_send_nack();				//向目标设备发送NACK信号 
}else{
	i2c_send_ack();                 //向目标设备发送ACK信号 
}
return r; 
} 


/* 
I2C读操作 
addr：目标设备地址 
buf：读缓冲区 
len：读入字节的长度 
*/ 
void i2c_read(unsigned char addr, unsigned char* buf, int len, BOOLEAN end_of_nak) 
{ 
int i; 
unsigned char t; 
i2c_start();                        //起始条件，开始数据通信 
//发送地址和数据读写方向 
t = (addr << 1) | 1;                    //低位为1，表示读数据 
i2c_write_byte(t); 
//读入数据 
for (i=0; i<len; i++){ 
	buf[i] = i2c_read_byte((i==(len-1))?end_of_nak : 0); 
}
i2c_stop();                     //终止条件，结束数据通信 
}

 
/* 
I2C写操作 
addr：目标设备地址 
buf：写缓冲区 
len：写入字节的长度 
*/ 
void i2c_write (unsigned char addr, unsigned char* buf, int len,BOOLEAN need_stop) 
{ 
int i; 
unsigned char t; 
i2c_start();                        //起始条件，开始数据通信 
//发送地址和数据读写方向 
t = (addr << 1) | 0;                    //低位为0，表示写数据 
i2c_write_byte(t); 
//写入数据 
for (i=0; i<len; i++) 
i2c_write_byte(buf[i]); 
if (need_stop){
	i2c_stop();                     //终止条件，结束数据通信 
}
} 


//--------------------------------------------------------------

#define SLAVE_ADDR 0x74   

enum _port_dev {
	DEV_0,
	DEV_1,
	DEV_2,
	DEV_3
} ;

enum _port_dir{
	PORT_OUT,
	PORT_IN
} ;

#define INPORT0 	0
#define INPORT1		1
#define OUTPORT0	2
#define OUTPORT1	3
#define POLARITY_INV0	4
#define POLARITY_INV1	5
#define CONFIGPORT0	6
#define CONFIGPORT1	7




void write_reg (INT8U dev_addr,INT8U reg_sel,INT8U data ){
	INT8U buf[2] ;

	buf[0] = reg_sel ; //reg addr
	buf[1] = data ; 	//reg data 
	i2c_write (dev_addr, buf , 2 ,1); 

}

INT8U  read_reg (INT8U dev_addr,INT8U reg_sel){
	INT8U buf[2] ;

	//--1. write the reg address you want to access 
	buf[0] =  reg_sel ; //reg addr
	i2c_write (dev_addr,buf,1, 0) ;

	//--2. read reg data 
	i2c_read (dev_addr,buf,1,1) ;

	return buf[0] ;
	
}


void set_port_dir (INT8U port_num, INT8U dir){

	INT8U dev_num = ((port_num >> 4) & 0x03 ) ;
	INT8U dev_addr = SLAVE_ADDR | dev_num ;

	INT8U reg_sel = ( port_num& 0xf0 )? CONFIGPORT1 : CONFIGPORT0 ;
	INT8U pin_sel = (port_num & 0x0f);

	//1.read the orignal config info 
	INT8U config_info =read_reg (dev_addr,reg_sel) ;

	//2.update config info 
	config_info = config_info & (~(1<<pin_sel )) | (dir << pin_sel) ;

	//3.write to dev  reg
	write_reg(dev_addr,reg_sel,config_info );
	
		
}


 // before call this function , need set_port_dir
INT8U get_port_val (INT8U port_num){
	//set_port_dir (port_num , PORT_IN) ;

	INT8U dev_num = ((port_num >> 4) & 0x03 ) ;
	INT8U dev_addr = SLAVE_ADDR | dev_num ;

	
	INT8U reg_sel = ( port_num & 0xf0 )? INPORT1 : INPORT0 ;
	INT8U pin_sel = (port_num & 0x0f);

	return (read_reg (dev_addr,reg_sel)>>pin_sel )& 0x01;
	
}


// before call this function , need set_port_dir
void set_port_val (INT8U port_num, INT8U val){
	//set_port_dir (port_num , PORT_OUT) ;

	INT8U dev_num = ((port_num >> 4) & 0x03 ) ;
	INT8U dev_addr = SLAVE_ADDR | dev_num ;

	INT8U reg_sel = ( port_num & 0xf0 )? OUTPORT1 : OUTPORT0 ;
	INT8U pin_sel = (port_num & 0x0f);

	INT8U port_val = get_port_val (port_num) ;
	port_val = port_val & (~(1<<pin_sel )) | (val << pin_sel) ;
	
	write_reg (dev_addr,reg_sel,port_val) ;
	
}



//-----------------------------------------------------------------------------------------




typedef struct _TYPE_GPIO {	
	INT8U setnum ;
	INT16U	delay 	;

}__attribute__ ((__packed__)) TYPE_GPIO,*pTYPE_GPIO;



typedef struct __GPIO_CMD {
	
	INT8U pin_num	;
	INT8U pin_dir	: 1;
	INT8U pin_level	: 1 ;
	INT8U rvd 		: 6 ;
	INT8U * pin_info_ack ; 
}__attribute__ ((__packed__))GPIO_CMD,*pGPIO_CMD;




void GPIO_IIC_Init (){

	//GPIO init 	
	OOBMAC_gpio_init(RST_TCA,OUTPUT,1) ; //realse rst pin to high  


}



INT8U GPIO_Processing ( pTYPE_GPIO ptype_gpio ,pGPIO_CMD pgpio_cmd  ){

	INT8U retval=0;
	INT8U port_val ;

	

	INT16U i=0  ;
	INT32U read_date ;


	//output ,during ptype_gpio->delay.
	for ( i=0 ;i < ptype_gpio->setnum;i++){
		if ((&pgpio_cmd[i])->pin_dir == PORT_OUT){
			set_port_dir ((&pgpio_cmd[i])->pin_num, PORT_OUT) ;
			set_port_val ((&pgpio_cmd[i])->pin_num, (&pgpio_cmd[i])->pin_level) ;

		
		}

	}


	// just delay once time, this delay will inpact to bulk in ack and accept the next command 	
	bsp_wait(ptype_gpio->delay) ;
	
	//after delay time , get the input value 
	for ( i=0 ;i < ptype_gpio->setnum;i++){
		if((&pgpio_cmd[i])->pin_dir == PORT_IN){
			set_port_dir ((&pgpio_cmd[i])->pin_num, PORT_IN) ;
			port_val = get_port_val((&pgpio_cmd[i])->pin_num) ;
			*((&pgpio_cmd[i])->pin_info_ack) = port_val ;
		}
	
	}
	
	return  0 ;

}



INT8U GPIO_Parsing (struct command_block_wrapper  *CMD , INT8U * data_addr , INT16U *pdata_num){
	
	INT16U i=0;
	INT16U offset = sizeof(TYPE_GPIO) ; //gpio cmd offset 
	pTYPE_GPIO p  = (pTYPE_GPIO)(CMD->data);
	pGPIO_CMD pgpio_cmd ;
	INT8U retval=0;
	*pdata_num =0 ; 
	
	pgpio_cmd = (pGPIO_CMD)malloc (sizeof(GPIO_CMD)*(p->setnum));

	for (i=0; i< p->setnum;i++){
		(&pgpio_cmd[i])->pin_num = CMD->data[offset+0] ;
		(&pgpio_cmd[i])->pin_dir = CMD->data[offset+1] & 0x1 ;
		(&pgpio_cmd[i])->pin_level = (CMD->data[offset+1]>>1) & 0x01 ;
		//(&pgpio_cmd[i])->pin_delay = CMD->data[offset+1] >> 1 ;
		(&pgpio_cmd[i])->pin_info_ack =  data_addr +  (*pdata_num)  ;

		offset += 2 ; 
		if ((&pgpio_cmd[i])->pin_dir == PORT_IN ) {
			*pdata_num += 1 ;
		}
		
	}			
			

	//call gpio function to processing CMD	
	retval = GPIO_Processing(p,pgpio_cmd);
	free (pgpio_cmd);

	return retval ;

}


 

