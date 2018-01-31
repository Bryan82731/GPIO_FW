/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	genpage.h

*****************************************************************************
*/

#ifndef __GENPAGE_H__
#define __GENPAGE_H__
#include "smbios.h"
#include "lib.h"
/*--------------------------Definitions------------------------------------*/

/*-----------------------------Structures------------------------------------*/

typedef struct webToken
{
    char* token;
    char offset;
}WEBTOKEN;
int genSMBIOSWebPage(int type, DataList *current);

#endif
