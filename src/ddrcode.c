#include <unistd.h>
#include <stddef.h>
#include <sys/dpdef.h>
#include <sys/ucos_ii.h>    /* prototype, OS and CPU API             */
#include <sys/autoconf.h>
#include <sys/inet_types.h>
#include <rlx/rlx_cpu.h>
#include <app_cfg.h>
#include "dram.h"
#include "uart.h"

#if 0
int ddrData[100];
void ddrcode()
{
	const int size = 100;
	int a[size];
	int i = 0;
	for(;i < size; i++)
	{
		ddrData[i]= i;
	}
	
	//printf("  Run code in ddr %x \r\n", $pc);	
	#if 0
		uart_write_str("  Run code in ddr \r\n");	
				uart_write_str("  Run code in ddr 1\r\n");	
						uart_write_str("  Run code in ddr 2\r\n");	
								uart_write_str("  Run code in ddr 3\r\n");	
										uart_write_str("  Run code in ddr 4\r\n");	
										#endif
}
#endif