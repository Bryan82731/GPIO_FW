
/****************************************************************************
 * File Description: This is a test program for 8168DP(F) UART.
 * 
 * The program reads your input text, and then shows a welcome message via
 * 8168DP UART interface.
 ****************************************************************************/

/****************************************************************************
 * Include List
 ****************************************************************************/
#include "bsp_cfg_fp_revA.h"
#include "uart.h"
#include "bsp.h"
#include <stdarg.h>

#if defined(CONFIG_BUILDROM)
void printk(const char *fmt, ...)
{
  int ret;
  char str[128];
  va_list ap;
  FILE f;

  f._flags = __SWR | __SSTR;
  f._bf._base = f._p = (unsigned char *) str;
  f._bf._size = f._w = 128;
  f._file = -1;  /* No file. */
#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = _svfprintf_r (_REENT, &f, fmt, ap);
  va_end (ap);
  *f._p = 0;
  uart_write_str(str);
  return (ret);
}

/****************************************************************************
 * Function Descriptions
 ****************************************************************************/
void uart_check_thre(void)
{
    INT32U status = 0;
    
    while(!status)
    {
        //$ check bit5 of LSR, that is THRE
        status = REG32(UART_IOBASE + UART_LSR) & 0x00000020;
    }
}

void uart_close(void)//disable UART
{
    //$ disable uart feature
    REG8(0xBB600004) &= ~0x10;
}
#if 0
void bsp_uart_init(unsigned int clk)
{
    //$ enable uart feature
    REG8(0xBB000004) |= 0x10;
    
    //$ disable uart interrupts
    REG32(UART_IOBASE + UART_IER) = 0x00;
	
    //$ enable to access DLL and DLH
    REG32(UART_IOBASE + UART_LCR) = 0x80;
    
    //$ set DLL and DLH
    //$ baud rate = uart clock / (16 * divide)
	REG32(UART_IOBASE + UART_DLL) = (INT8U)(clk/9600/16);//9600 baud
	REG32(UART_IOBASE + UART_DLH) = (INT8U)((clk/9600/16)>>8);
	

	//$ set data length to 8 bits, 1 stop bit, no parity
    REG32(UART_IOBASE + UART_LCR) = 0x03;
	
	// enable receive data avalable interrupt
	REG32(UART_IOBASE + UART_IER) = 0x01;
}
#endif
void bsp_uart_init(unsigned int clk)
{

	INT32U temp;

	//$ enable uart feature
	REG8(0xBB000004) |= 0x10;

	//$ disable uart interrupts
	REG32(UART_IOBASE + UART_IER) = 0x00;

	//$ enable to access DLL and DLH
	REG32(UART_IOBASE + UART_LCR) = 0x80;

	//$ set DLL and DLH
	//$ baud rate = uart clock / (16 * divide)
	REG32(UART_IOBASE + UART_DLL) = (INT8U)(clk/9600/16);//9600 baud
	REG32(UART_IOBASE + UART_DLH) = (INT8U)((clk/9600/16)>>8);


	//$ set data length to 8 bits, 1 stop bit, no parity
	REG32(UART_IOBASE + UART_LCR) = 0x03;

	//for B-cut tx/rx share pin with led
	temp = OOBMACReadIBReg(0xdc0c);
	temp |=0x40000 ;   //txd oe 1
	temp &=0xfffdffff; //rxd oe 0
	OOB2IB_W(0xdc0c, 0xF, temp);

	// enable receive data avalable interrupt
	REG32(UART_IOBASE + UART_IER) = 0x01;
}

INT32U uart_write_str(char *ptr)
{
    INT8U  ch;
    INT32U rt = 0;
    
    while(*ptr)
    {
        //$ check THRE bit
        uart_check_thre();
        
        //$ read 1-byte data from buffer
        ch = *(INT8U *)ptr ++;
        REG32(UART_IOBASE + UART_THR) = ch;
    }		
    
    return(rt);
}


//volatile INT8U Rx_data_valiable_flag = 0;
//volatile char rx_data= 0;

// UART RX interrupt handller function, interrupt will be cleared by read RBR
void bsp_uart_RX_handler(void)	
{
	//Rx_data_valiable_flag = 1;		// indicate that rx data avaliable
	//rx_data = REG8(UART_IOBASE+UART_RBR);
}
#endif
	
