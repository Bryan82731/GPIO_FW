/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	telnet.h

*****************************************************************************
*/

#ifndef _TELNET_INCLUDED
#define _TELNET_INCLUDED
#include "rtskt.h"
#include "ssh.h"
/*--------------------------Include Files-----------------------------------*/

/*--------------------------Definitions------------------------------------*/

enum{SSH_TCR, TEL_TCR};

void telnetSrv(void *data);
void udpSrvTest(void *data);
enum {DISABLED= 0, DISCONNECTED, LISTENED, ACCEPTED, ENABLED, CONNECTED};
enum {SERVER_MODE = 0, CLIENT_MODE};


void bsp_tcr_init(void) ;
void bsp_tcr_handler(void);
void tcrrx();
void bsp_kcs_handler(void);
void bsp_kcs_init(void) ;
//void tcr_received(INT8U *ptr, INT8U len) _ATTRIBUTE_SRAM;
//void bsp_tcr2_init(void) _ATTRIBUTE_IROM ;
//void bsp_tcr2_handler(void) _ATTRIBUTE_IROM;
void kcs_rx_recycle();
#endif
