/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	eventlog.c

Abstract:	Event log parser

*****************************************************************************
*/

#include <stdio.h>
#include "sys/autoconf.h"
#include "sys/dpdef.h"
#include "smbus.h"
#include "bsp_cfg.h"

extern sensor_t sensor[MAX_SENSOR_NUMS];

#if (CONFIG_VERSION <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
int assertStr(eventdata *ed, char *buf, int offset)
{

    int rv = 0;

    if (ed->Event_Offset & 0x80)
        rv = sprintf(buf + offset, "Completed ");
    else
        rv = sprintf(buf + offset, "Starting ");

    offset += rv;
    return offset;
}

int thermalSrc(eventdata *ed, char *buf, int offset)
{
    int rv = 0;

    if (ed->Entity == 0x07 || ed->Entity == 0x30)// && ed->Entity_Instance == 0x01)
        rv = sprintf(buf + offset, "System Board ");
    else if (ed->Entity == 0x03)// && ed->Entity_Instance == 0x01)
        rv = sprintf(buf + offset, "Processor ");
    else if (ed->Entity == 0x13) //FTS and PLDM Sensor, it is reserved
        rv = sprintf(buf + offset, "%s: ", sensor[ed->Sensor_Number].name);
    else if (ed->Entity == 0x34)
        rv = sprintf(buf + offset, "System Management Software ");
    else if (ed->Entity <= 26 || ed->Entity >= 0x90 )
        rv = 0;   //PET Spec.
    else
        return -1;

    offset += rv;
    return offset;
}
#endif

int getEventLogParser(eventdata *ed, unsigned char* buf)
{
    int rv = 0;
    int offset = 0;


    sprintf(buf, "%c", '\0');

    //environmental events
    if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Critical Temperature Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x00)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Critical Temperature Problem Cleared\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x03)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Temperature Warning\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x09)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Over-Temperature Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x07)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Over-Temperature Warning\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Under-Temperature Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x01 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x00)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Under-Temperature Warning\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x02 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Critical Voltage Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x02 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x09)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Over-Voltage Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x02 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Under-Voltage Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x04 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Critical Fan failure\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x04 && ed->Event_Type == 0x07 && ed->Event_Offset == 0x00)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic Critical Fan failure Cleared\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x04 && ed->Event_Type == 0x03 && ed->Event_Offset == 0x01)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Generic predictive Fan failure\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x04 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x02)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Fan Speed Problem\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x04 && ed->Event_Type == 0x01 && ed->Event_Offset == 0x00)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Fan Speed Warning\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x05 && ed->Event_Type == 0x6F && ed->Event_Offset == 0x00)
    {
        rv = thermalSrc(ed, buf, offset);
        if ( rv != -1)
        {
            offset = rv;
            rv = sprintf(buf + offset, "Chasis Intrusion\n");
        }
    }
    else if (ed->Event_Sensor_Type == 0x06 && ed->Event_Type == 0x6F )
    {
        rv = sprintf(buf , "BIOS Password Error\n");
    }

    //System Firmware Related
    else if (ed->Event_Sensor_Type == 0x0F && ed->EventData[0] == 0x40)
    {
        //Standard System Firmware Error
        if (ed->Event_Offset == 0x00)
        {
            if (ed->EventData[1] == 0x01 && ed->Entity == 32)
                rv = sprintf(buf + offset, "No system memory; memory missing\n");
            else if (ed->EventData[1] == 0x02 && ed->Entity == 32)
                rv = sprintf(buf + offset, "No system memory;unrecoverable failure\n");
            else if (ed->EventData[1] == 0x03 && ed->Entity == 4)
                rv = sprintf(buf + offset, "Unrecoverable hard-disk failure\n");
            else if (ed->EventData[1] == 0x04 && ed->Entity == 7)
                rv = sprintf(buf + offset, "Unrecoverable system board failure\n");
            else if (ed->EventData[1] == 0x05 && ed->Entity == 4)
                rv = sprintf(buf + offset, "Unrecoverable diskette subsystem failure\n");
            else if (ed->EventData[1] == 0x06 && ed->Entity == 4)
                rv = sprintf(buf + offset, "Unrecoverable hard-disk controller failure\n");
            else if (ed->EventData[1] == 0x07)
                rv = sprintf(buf + offset, "Unrecoverable PS/2 or USB keyboard failure\n");
            else if (ed->EventData[1] == 0x08)
                rv = sprintf(buf + offset, "Removable boot media not found\n");
            else if (ed->EventData[1] == 0x09)
                rv = sprintf(buf + offset, "Unrecoverable video controller failure\n");
            else if (ed->EventData[1] == 0x0A)
                rv = sprintf(buf + offset, "No video device detected\n");
            else if (ed->EventData[1] == 0x0B)
            {
                if (ed->Entity == 3)
                    rv = sprintf(buf + offset, "Unrecoverable multi-processor  configuration mismatch\n");
                else
                    rv = sprintf(buf + offset, "Firmware ROM corruption detected\n");
            }
            else if (ed->EventData[1] == 0x0C)
                rv = sprintf(buf + offset, "CPU VID Mismatch. One or more processors sharing the same voltage supply have mismatched voltage requirements\n");
            else if (ed->EventData[1] == 0x0D)
                rv = sprintf(buf + offset, "CPU speed-matching failure\n");
            //0x0E - 0xFF
            else
                rv = sprintf(buf + offset, "Reserved for future definition by this specification\n");

        }
        //System Firmware Hang
        else if (ed->Event_Offset == 0x01)
        {
            if (ed->Entity == 0x0B && ed->EventData[1] == 0x08)
                rv = sprintf(buf + offset, "Hang during option ROM initialization\n");
        }
        //System Firmware Progress Events
        else if ((ed->Event_Offset & 0x7F) == 0x02)
        {
            offset += assertStr(ed, buf, offset);
            if (ed->EventData[1] == 0x00 && ed->Entity == 0x04)
                rv = sprintf(buf + offset, "system firmware progress\n");
            if (ed->EventData[1] == 0x01 && ed->Entity == 32)
                rv = sprintf(buf + offset, "memory initialization and test\n");
            else if (ed->EventData[1] == 0x02 && ed->Entity == 4)
                rv = sprintf(buf + offset, "hard-disk initialization and test\n");
            else if (ed->EventData[1] == 0x03 && ed->Entity == 3)
                rv = sprintf(buf + offset, "secondary processor\n");
            else if (ed->EventData[1] == 0x04 && ed->Entity == 34)
                rv = sprintf(buf + offset, "waiting for user-password entry\n");
            else if (ed->EventData[1] == 0x05 && ed->Entity == 34)
                rv = sprintf(buf + offset, "BIOS setup\n");
            else if (ed->EventData[1] == 0x06 && ed->Entity == 34)
                rv = sprintf(buf + offset, "USB resource configuration\n");
            else if (ed->EventData[1] == 0x07 && ed->Entity == 34)
                rv = sprintf(buf + offset, "PCI resource configuration\n");
            else if (ed->EventData[1] == 0x08 && ed->Entity == 11)
                rv = sprintf(buf + offset, "option ROM initialization\n");
            else if (ed->EventData[1] == 0x09 && (ed->Entity == 0x01 || ed->Entity == 0x0B))
                rv = sprintf(buf + offset, "video initialization\n");
            else if (ed->EventData[1] == 0x0A && (ed->Entity == 0x00 || ed->Entity == 0x03) )
                rv = sprintf(buf + offset, "cache initialization\n");
            else if (ed->EventData[1] == 0x0B)
                rv = sprintf(buf + offset, "SM Bus initialization\n");
            else if (ed->EventData[1] == 0x0C && ed->Entity == 0x00)
                rv = sprintf(buf + offset, "keyboard initialization\n");
            else if (ed->EventData[1] == 0x0C && ed->Entity == 0x07)
                rv = sprintf(buf + offset, "keyboard controller initialization\n");
            else if (ed->EventData[1] == 0x0D)
                rv = sprintf(buf + offset, "embedded controller/management controller initialization\n");
            else if (ed->EventData[1] == 0x0E)
                rv = sprintf(buf + offset, "docking station attachment\n");
            else if (ed->EventData[1] == 0x12 && ed->Entity == 0)
                rv = sprintf(buf + offset, "calling operating system wake-up vector\n");
            else if (ed->EventData[1] == 0x13 && ed->Entity == 0)
                rv = sprintf(buf + offset, "OS boot process\n");
            else if (ed->EventData[1] == 0x14 && ((ed->Entity == 0) || (ed->Entity == 0x07)))
                rv = sprintf(buf + offset, "baseboard or motherboard initialization\n");
            else if (ed->EventData[1] == 0x16)
                rv = sprintf(buf + offset, "floppy initialization\n");
            else if (ed->EventData[1] == 0x17)
                rv = sprintf(buf + offset, "keyboard test\n");
            else if (ed->EventData[1] == 0x18)
                rv = sprintf(buf + offset, "pointing device test\n");
            else if (ed->EventData[1] == 0x19 && ed->Entity == 3)
                rv = sprintf(buf + offset, "primary processor initialization\n");
        }
        else
        {


        }
    }
    else if (ed->Event_Sensor_Type == 0x0F && ed->Event_Offset == 0x00 && ed->EventData[0] == 0xa0 && ed->EventData[1] == 0x81)
    {
        //TOM added
        rv = sprintf(buf + offset, "Unrecoverable PS/2 or USB keyboard failure\n");
    }
    else if (ed->Event_Sensor_Type == 0x1E && ed->EventData[0] == 0x40)
    {
        if (ed->Event_Offset == 0x00 && ed->Entity == 0x00 && ed->EventData[1] == 0x00)
            rv = sprintf(buf + offset, "No bootable media\n");


    }
    else if (ed->Event_Sensor_Type == 0x25 && ed->Event_Offset == 0x00 && ed->EventData[1] == 0x00 && ed->Entity == 34)
    {
        rv = sprintf(buf + offset, "System firmware started.\n");
    }
    //OS Events
    else if (ed->Event_Sensor_Type == 0x23 && ed->Event_Type == 0x6F && ed->Event_Offset == 0x00 /*&& ed->Entity == 0x23*/)
    {
        rv = sprintf(buf + offset, "OS Boot Failure\n");
    }
    else if (ed->Event_Sensor_Type == 0x20 && ed->Event_Type == 0x6F && ed->Event_Offset == 0x01 /*&& ed->Entity == 0x23*/)
    {
        rv = sprintf(buf + offset, "OS Hung\n");
    }

    else if (ed->Event_Type == 0x6F && ed->Entity == 0x03)
    {
        rv = sprintf(buf + offset, "CPU Missing\n");
    }

    if (rv != -1)
        offset += rv;

    if (offset == 0 )
        sprintf(buf, "Reserved for future definition by this specification\n");

    return offset;
}
