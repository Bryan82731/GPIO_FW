/*
******************************* C HEADER FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	telnet.h

*****************************************************************************
*/

#ifndef _VNC_INCLUDED
#define _VNC_INCLUDED

/*--------------------------Include Files-----------------------------------*/

/*--------------------------Definitions------------------------------------*/
struct VgaInfo
{
	INT16U FB0_Vresol, FB0_Hresol, FB0_BPP, FB0_VBnum, FB0_HBnum, FB1_Vresol, FB1_Hresol, FB1_BPP, FB1_VBnum, FB1_HBnum;
};

void VNCSrv(void *data);
typedef struct _VNCPointEvent
{
	unsigned char msg_type;
	unsigned char button_mask;
	unsigned short  x;
	unsigned short	y;	
}VNCPointEvent;

typedef struct _VNCKeyEvent
{
	unsigned char msg_type;
	unsigned char down_flag;
	unsigned char padding[2];
	unsigned int key;	
}VNCKeyEvent;

//rtk tom report format
#if 0
typedef struct _HIDMouse_Report
{
	char rID;
	char buttons;	
	char x;
	char y;
	char wheel;		
}HIDMouse_Report;
#endif

#if 0
typedef struct _HIDMouse_Report
{
	char rID;
	char buttons;			
	char x;
	char y;	
	char wheel;		
}HIDMouse_Report;
#endif

typedef struct _HIDMouse_Report
{
	char rID;
	char buttons;			
	unsigned short x;
	unsigned short y;	
	char wheel;		
}HIDMouse_Report;

#define VNC_Left_Shift 0xffe1
#define VNC_Right_Shift 0xffe2
#define VNC_Left_Ctl 0xffe3
#define VNC_Right_Ctl 0xffe4
#define VNC_Left_Alt 0xffe9
#define VNC_Right_Alt 0xffea

#define KEY_MODIFIER_LEFT_CTRL      0x01
#define KEY_MODIFIER_LEFT_SHIFT      0x02
#define KEY_MODIFIER_LEFT_ALT      0x04
#define KEY_MODIFIER_LEFT_GUI      0x08
#define KEY_MODIFIER_RIGHT_CTRL   0x010
#define KEY_MODIFIER_RIGHT_SHIFT   0x020
#define KEY_MODIFIER_RIGHT_ALT      0x040
#define KEY_MODIFIER_RIGHT_GUI      0x080

#define HID_Left_Shift 0xffe1
#define HID_Right_Shift 0xffe2
#define HID_Left_Ctl 0xffe3
#define HID_Right_Ctl 0xffe4
#define HID_Left_Alt 0xffe9
#define HID_Right_Alt 0xffea

#endif