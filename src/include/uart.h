#ifndef _UART_H_
#define _UART_H_

#include <unistd.h>
#include <stddef.h>
#include <sys/dpdef.h>
#include <sys/ucos_ii.h>    /* prototype, OS and CPU API             */
#include <sys/autoconf.h>
#include <sys/inet_types.h>


/****************************************************************************
 * Constant Definition
 ****************************************************************************/
#define UART_RBR    0x00
#define UART_THR    0x00
#define UART_DLL    0x00
#define UART_DLH    0x04
#define UART_IER    0x04
#define UART_IIR    0x08
#define UART_FCR    0x08
#define UART_LCR    0x0C
#define UART_MCR    0x10
#define UART_LSR    0x14
#define UART_MSR    0x18
#define UART_SCR    0x1C

void bsp_uart_init(unsigned int clk);
void uart_close(void);
void bsp_uart_RX_handler(void);
INT32U uart_write_str(char *ptr);

#ifndef CONFIG_UART_DEBUG
//#define  printk(a, ...) {}
#endif
#endif

