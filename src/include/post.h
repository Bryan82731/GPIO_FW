/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	post.h

*****************************************************************************
*/

#ifndef _POST_INCLUDED
#define _POST_INCLUDED

void netSet(struct tcp_pcb *pcb, char *p, int len);
void serviceSet(struct tcp_pcb *pcb, char *p, int len);
void remoteCtlSet(struct tcp_pcb *pcb, char *p, int len);
void tlsAuth(struct tcp_pcb *pcb, char *p, int len);
void EzShare(struct tcp_pcb *pcb, char *p, int len);
//ht 20110816
void smbSet(struct tcp_pcb *pcb, char *p, int len);
#endif

