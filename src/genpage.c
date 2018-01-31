/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	smbios.c

Abstract:	According SMBIOS data to generate a web page

*****************************************************************************
*/

#include <stdio.h>
#include <stdlib.h>
#include "genpage.h"
#include "bsp.h"
#include "debug.h"

const static char htmlHead[] = {
"<link rel=\"stylesheet\" type=\"text/css\" href=\"rdcss.css\">\n"
#if CONFIG_ACER_WEB_UI
#else
"<STYLE type=text/css>\n\
th{FONT-FAMILY:Calibri, Arial;text-align:left;font-weight=normal}\n\
td{FONT-FAMILY:Calibri, Arial;background-color:#FF9900}\n\
</STYLE>\n"
#endif
"<html><head>\n\
<title>Welcome to use Realtek Gigabit</title>\n\
<body>\n"
};
const static char htmlTail[] = {"</body></html>"};

#if CONFIG_ACER_WEB_UI
const static char smbiosHtmlTBL[] = {"<TABLE cellSpacing=0 cellpadding=0><TBODY>\n"};
const static char smbiosHtmlTD[] = {"<TR><td class=\"list-label\"><img src=\"list-label.gif\"></td><TD>%s:</TD><TD></TD><TD class=\"t\">%s</TD>\n"};
const static char smbiosHtmlTH[] = {"<TR><TH></TH><TH>%s:</TH><TH width=10></TH><TH class=\"t\">%s</TH>"};
const static char smbiosHtmlTHNCOLON[] = {"<TR><TH></TH><TH>%s</TH><TH width=10></TH><TH class=\"t\">%s</TH>"};
#else
const static char smbiosHtmlTBL[] = {"<hr><table cellspacing=0>\n"};
const static char smbiosHtmlTD[] = {"<td>>%s:</td><td width=10></td><td>%s</td><tr>\n"};
const static char smbiosHtmlTH[] = {"<th>>%s:</th><th width=10></th><th>%s</th><tr>\n"};
const static char smbiosHtmlTHNCOLON[] = {"<th>%s</th><th width=10></th><th>%s</th><tr>\n"};
#endif

int biosPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN bios[] =
    {
        {"Vender", BIOS_VENDOR},
        {"BIOS Version", BIOS_VER},
        {"BIOS Starting Address Segment", BIOS_START_ADDR},
        {"BIOS Release Date", BIOS_RELEASE_DATE},
        {"BIOS ROM Size", BIOS_ROM_SIZE},
        {"BIOS Characteristics", BIOS_CHARS},
        {"BIOS Characteristics Extension Bytes", BIOS_CHARS_EXT_BYTE},
        {"System BIOS Major Release", BIOS_MAJOR},
        {"System BIOS Minor Release", BIOS_MINOR},
        {"Embedded Control Controller Firmware Major Release", BIOS_FIRMWARE_MAJOR},
        {"Embedded Control Controller Firmware Minor Release", BIOS__FIRMWARE_MINOR}
    };

    int lists = 0 , i = 0 , tmp = 0, t = -1;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    SBU64 u64;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>BIOS Information</H2></b></p>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);

    lists = sizeof(bios) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, bios[i].offset, out);
        if (bios[i].offset == BIOS_ROM_SIZE)
        {
            sscanf((char*)out, "%u", &tmp);
            sprintf(out, "%dKB", 64*(tmp+1));

            if (++t%2)
                current->len += sprintf(current->payLoadAddr + current->len, smbiosHtmlTH, bios[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, bios[i].token, out);

        }
        else if (bios[i].offset == BIOS_CHARS)
        {
            sscanf((char*)out, "%d %d", &u64.l, &u64.h);
            getBIOSChar(u64, sh, out);

            if (++t%2)
                current->len += sprintf(current->payLoadAddr + current->len, smbiosHtmlTH, bios[i].token,"");
            else
                current->len += sprintf(current->payLoadAddr + current->len, smbiosHtmlTD, bios[i].token,"");

            while (getBIOSChar(u64, sh, out))
            {
                if (t%2)
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTHNCOLON,"",out);
                else
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD,"",out);
                current = chkDataListBufSize(current);
            }
        }
        else if (bios[i].offset == BIOS_CHARS_EXT_BYTE)
        {
            //rv = sprintf(current->payLoadAddr + current->len,"<td>%s: </td><td></td><tr>2\n", bios[i].token);
            //current->len+=rv;
            while (getBIOSCharE1(sh, BIOS_CHARS_EXT_BYTE, addr[BIOS_CHARS_EXT_BYTE], out))
            {
                if (t%2)
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTHNCOLON,"",out);
                else
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD,"",out);
                current = chkDataListBufSize(current);
            }

            while (getBIOSCharE2(sh, BIOS_CHARS_EXT_BYTE + 1,addr[BIOS_CHARS_EXT_BYTE + 1], out))
            {
                if (t%2)
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTHNCOLON,"",out);
                else
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD,"", out);
                current = chkDataListBufSize(current);
            }
        }
        else
        {
            if (++t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, bios[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, bios[i].token, out);
        }

        current = chkDataListBufSize(current);
    }
    free(out);
    return current->len;
}

int sysInfoPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN sysInfo[] =
    {
        {"Manufacturer", SYSINFO_MAN},
        {"Product Name", SYSINFO_PN},
        {"Version", SYSINFO_VER},
        {"Serial Number", SYSINFO_SN},
        {"UUID", SYSINFO_UUID},
        {"Wake-up Type", SYSINFO_WAKEUP_TYPE},
        {"SKU Number", SYSINFO_SKU},
        {"Family", SYSINFO_FAMILY},
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>System Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);

    lists = sizeof(sysInfo) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, sysInfo[i].offset, out);

        if (i%2)
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, sysInfo[i].token, out);
        else
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, sysInfo[i].token, out);

        current = chkDataListBufSize(current);
    }

    free(out);
    return current->len;
}

int basedBoardPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN baseBoard[] =
    {
        {"Manufacturer", BBINFO_MAN},
        {"Product Name", BBINFO_PN},
        {"Version", BBINFO_VER},
        {"Serial Number", BBINFO_SN},
        {"Asset Tag", BBINFO_ASSET_TAG},
        {"Features", BBINFO_FEATURE_TAG},
        {"Location In Chassis", BBINFO_LOCATION_IN_CHASSIS},
        {"Chassis Handle", BBINFO_CHASSIS_HANDLE},
        {"Board Type", BBINFO_BOARD_TYPE},
        {"Contained Object Handles", BBINFO_OBJ_HANDLE}
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Based Board Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(baseBoard) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, baseBoard[i].offset, out);
        if (i%2)
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, baseBoard[i].token, out);
        else
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, baseBoard[i].token, out);

        current = chkDataListBufSize(current);
    }
    free(out);
    return current->len;
}

int chassisPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN chassis[] =
    {
        {"Manufacturer", CHASSIS_MAN},
        {"Type", CHASSIS_TYPE},
        {"Version", CHASSIS_VER},
        {"Serial Number", CHASSIS_SN},
        {"Asset Tag Number", CHASSIS_ASSET_TAG},
        {"Boot-up State", CHASSIS_BOOTUP},
        {"Pow Power Supply State", CHASSIS_POWER},
        {"Thermal State", CHASSIS_THERMAL},
        {"Security Status", CHASSIS_SECURITY},
        {"OEM-defined", CHASSIS_OEM},
        {"Height", CHASSIS_HEIGHT},
        {"Number of Power Cords", CHASSIS_POWER_NUM},
        {"Contained Element Count", CHASSIS_ELEMENT_COUNT},
        {"Contained Element Record Length", CHASSIS_ELEMENT_LEN},
        {"Contained Element", CHASSIS_ELEMENT}
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Chassis Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);

    lists = sizeof(chassis) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, chassis[i].offset, out);

        if (i%2)
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, chassis[i].token, out);
        else
            current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, chassis[i].token, out);

        current = chkDataListBufSize(current);
    }
    free(out);
    return current->len;
}

int processorPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN processor[] =
    {
        {"Socket Designation", PROCESSOR_SKT_DESIGN},
        {"Processor Type", PROCESSOR_TYPE},
        {"Processor Family", PROCESSOR_FAMILY},
        {"Processor Manufacturer", PROCESSOR_MAN},
        {"Processor ID", PROCESSOR_ID},
        {"Processor Version", PROCESSOR_VER},
        {"Voltage", PROCESSOR_VOLTAGE},
        {"External Clock", PROCESSOR_ECLK},
        {"Max Speed", PROCESSOR_MAX_SPEED},
        {"Current Speed", PROCESSOR_CURRENT_SPEED},
        {"Status", PROCESSOR_STATUS},
        {"Processor Upgrade", PROCESSOR_UPGRADE},
        {"L1 Cache Handle", PROCESSOR_L1_CACHE},
        {"L2 Cache Handle", PROCESSOR_L2_CACHE},
        {"L3 Cache Handle", PROCESSOR_L3_CACHE},
        {"Serial Number", PROCESSOR_SN},
        {"Asset Tag", PROCESSOR_ASSET_TAG},
        {"Part Number", PROCESSOR_PN},
        {"Core Count", PROCESSOR_CORE_COUNT},
        {"Core Enabled", PROCESSOR_CORE_ENABLED},
        {"Thread Count", PROCESSOR_THREAD_COUNT},
        {"Processor Characteristics", PROCESSOR_FAMILY2}
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Processor Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s", smbiosHtmlTBL);
    lists = sizeof(processor) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, processor[i].offset, out);
        if (processor[i].offset == PROCESSOR_ECLK || processor[i].offset == PROCESSOR_MAX_SPEED ||
                processor[i].offset == PROCESSOR_CURRENT_SPEED)
        {
        	strcat(out, " MHz");
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, processor[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, processor[i].token, out);
        }
        else
        {
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, processor[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, processor[i].token, out);
        }

        current = chkDataListBufSize(current);
    }
    free(out);
    return current->len;
}


int cachePage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN cache[] =
    {
        {"Socket Designation", CACHE_SKT_DSEIGN},
        {"Cache Configuration", CACHE_CFG},
        {"Maximum Cache Size", CACHE_MAX_SIZE},
        {"Installed Size", CACHE_INSTALL_SIZE},
        {"Supported SRAM Type", CACHE_SUPPORT_SRAM_TYPE},
        {"Current SRAM Type", CACHE_CURRENT_SRAM_TYPE},
        {"Cache Speed", CACHE_SPEED},
        {"Error Correction Type", CACHE_ERR_CORRECT_TYPE},
        {"System Cache Type", CACHE_SYS_TYPE},
        {"Associativity", CACHE_SUPPORT_ASSOC},
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    unsigned char *curLoc;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Cache Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s", smbiosHtmlTBL);
    lists = sizeof(cache) /sizeof(WEBTOKEN);

    
    do
    {
        for (i=0; i<lists; i++)
        {
            getSMBIOSElement(addr, sh,  type, cache[i].offset, out);
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, cache[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len, smbiosHtmlTD, cache[i].token, out);
            current = chkDataListBufSize(current);
        }
        current->len += sprintf(current->payLoadAddr + current->len,"%s","<tr><tr>");
        curLoc = addr;
    }while (getSMBIOSTypeNext(curLoc, type, &addr));

    free(out);
    return current->len;
}

int portConnectorPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN portConnector[] =
    {
        {"Internal Reference Designator", PCON_INTER_REF_DESIGN},
        {"Internal Connector Type", PCON_INTER_TYPE},
        {"External Reference Designator", PCON_EXTER_REF_DESIGN},
        {"External Connector Type", PCON_EXTER_TYPE},
        {"Port Type", PCON_PORT_TYPE}
    };

    int lists = 0 , i = 0, t= 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    unsigned char *curLoc;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Port Connector Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s", smbiosHtmlTBL);
    lists = sizeof(portConnector) /sizeof(WEBTOKEN);

    do{
        for (i=0; i<lists; i++)
        {
            getSMBIOSElement(addr, sh,  type, portConnector[i].offset, out);

            if (t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, portConnector[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, portConnector[i].token, out);

            current = chkDataListBufSize(current);
            t++;
        }

        current->len += sprintf(current->payLoadAddr + current->len,"%s","<tr><tr>");
        curLoc = addr;
    }while (getSMBIOSTypeNext(curLoc, type, &addr));

    free(out);
    return current->len;
}

int sysSlotPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN sysSlot[] =
    {
        {"Slot Designation", SLOT_DESIGN},
        {"Slot Type", SLOT_TYPE},
        {"Slot Data Bus Width", SLOT_BUS_WIDTH},
        {"Current Usage", SLOT_USAGE},
        {"Slot Length", SLOT_LEN},
        {"Slot ID", SLOT_ID},
        {"Slot Characterist Characteristics 1", SLOT_CHAR1},
        {"Slot Characterist Characteristics 2", SLOT_CHAR2},
        {"Segment Group Number", SLOT_SEG_GN},
        {"Bus Number", SLOT_BUS_NUM},
        {"Device Number", SLOT_DEV_NUM}
    };

    int lists = 0 , i = 0, t = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    unsigned char *curLoc;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>System Slot Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(sysSlot) /sizeof(WEBTOKEN);
    do{
        for (i=0; i<lists; i++)
        {
            getSMBIOSElement(addr, sh,  type, sysSlot[i].offset, out);
            if (t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, sysSlot[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, sysSlot[i].token, out);
            current = chkDataListBufSize(current);
            t++;
        }
        current->len += sprintf(current->payLoadAddr + current->len,"%s","<tr><tr>");
        curLoc = addr;
    }while (getSMBIOSTypeNext(curLoc, type, &addr));

    free(out);
    return current->len;
}

int onBoardDevPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    int i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    int devNum;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>On Board Devices Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);

    devNum = (sh->len - 0x04) /2;
    sh->data+=4;

    for (i = 0; i<devNum; i++)
    {
        getOnBoardDevType((unsigned char)(sh->data[2*i]&0x7F), out);
        current->len += sprintf(current->payLoadAddr + current->len, "<td>Type:</td><td> %s</td><tr>\n", out);
        current->len += sprintf(current->payLoadAddr + current->len, "<td>Status:</td><td> %s<td><tr>\n",(sh->data[2*i]&0x80) ?"Enabled":"Disabled");
        getBIOSElemnetString_swpatch(sh->data, 2*i+1, out);
        current->len += sprintf(current->payLoadAddr + current->len, "<td>Descripition:<td><td> %s</td><tr>",out);
        current = chkDataListBufSize(current);
    }

    free(out);
    return current->len;
}

int sysEventLogPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    WEBTOKEN sysEventLog[] =
    {
        {"Access Method", SYSEVTLOG_ACCESS_METHOD},
        {"Log Status", SYSEVTLOG_STATUS},
        {"Number of Supported Event Log Type", SYSEVTLOG_DESCRIPTOR_NUM},
        {"Supported Event Log Type", SYSEVTLOG_DESCRIPTOR_LIST}
    };

    int lists = 0 , i = 0, count = 0, t = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    INT8U *tmp = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>System Event Log Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(sysEventLog) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        if (sysEventLog[i].offset == SYSEVTLOG_DESCRIPTOR_LIST)
        {        	      
            while (getSysEventLogDescriptor(addr[SYSEVTLOG_DESCRIPTOR_NUM],
                                            addr[SYSEVTLOG_DESCRIPTOR_TYPE],(addr + SYSEVTLOG_DESCRIPTOR_LIST), out))
            {
            	sprintf(tmp,"%s %d", sysEventLog[i].token, ++count);
                if (t%2)
                    current->len += sprintf(current->payLoadAddr + current->len, smbiosHtmlTH, tmp, out);
                else
                    current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, tmp, out);

                current = chkDataListBufSize(current);
                t++;
            }
        }
        else if (sysEventLog[i].offset == SYSEVTLOG_DESCRIPTOR_NUM)
        {
        
        sprintf(tmp,"%d", addr[SYSEVTLOG_DESCRIPTOR_NUM]);
            if (t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, sysEventLog[i].token, tmp);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, sysEventLog[i].token, tmp);
            t++;
        }
        else
        {
            getSMBIOSElement(addr, sh,  type, sysEventLog[i].offset, out);
            if (t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, sysEventLog[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, sysEventLog[i].token, out);
            t++;
        }
        current = chkDataListBufSize(current);
    }

    free(out);
    return current->len;
}

int phyMemArrayPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN phyMemArray[] =
    {
        {"Location", PHYMEMARY_LOCATION},
        {"Use", PHYMEMARY_USE},
        {"Memory Error Correction", PHYMEMARY_ERR_CORRECT},
        {"Maximum Capacity", PHYMEMARY_MAX_CAP},
        {"Memory Error Information Handle", PHYMEMARY_ERR_INFO_HANDLE},
        {"Number of Memory Devices", PHYMEMARY_DEV_NUM},
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Physical Memory Array Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(phyMemArray) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, phyMemArray[i].offset, out);
        if (phyMemArray[i].offset == PHYMEMARY_MAX_CAP)
        {
        	strcat(out,"KBytes");        
        
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, phyMemArray[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, phyMemArray[i].token, out);
        }
        else
        {
            if (i%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, phyMemArray[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, phyMemArray[i].token, out);
        }
        current = chkDataListBufSize(current);
    }

    free(out);
    return current->len;
}

int memDevPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    static const WEBTOKEN memDev[] =
    {
        {"Memory Error Information Handle", MEMDEV_ERR_INFO_HANDLE},
        {"Total Width", MEMDEV_WIDTH},
        {"Data Width", MEMDEV_DATA_WIDTH},
        {"Size", MEMDEV_SIZE},
        {"Form Factor", MEMDEV_FACTOR},
        {"Device Set", MEMDEV_SET},
        {"Device Locator", MEMDEV_DEV_LOCATOR},
        {"Bank Locator", MEMDEV_BANK_LOCATOR},
        {"Memory Type", MEMDEV_TYPE},
        {"Type Detail", MEMDEV_DEV_DETAIL},
        {"Speed", MEMDEV_SPEED},
        {"Manufacturer", MEMDEV_MAN},
        {"Serial Number", MEMDEV_SN},
        {"Asset Tag", MEMDEV_TAG},
        {"Part Number", MEMDEV_PN}
    };

    int lists = 0 , i = 0, t = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    unsigned char *curLoc;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Memory Device Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(memDev) /sizeof(WEBTOKEN);
    do{
        for (i=0; i<lists; i++)
        {
            getSMBIOSElement(addr, sh,  type, memDev[i].offset, out);
            if (t%2)
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTH, memDev[i].token, out);
            else
                current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, memDev[i].token, out);
            current = chkDataListBufSize(current);
            t++;
        }
        curLoc = addr;
    }while (getSMBIOSTypeNext(curLoc, type, &addr));

    free(out);
    return current->len;
}

int memArrayMapAddrPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    WEBTOKEN memArrayMapAddrPage[] =
    {
        {"Starting Address", MEMARYMAP_START_ADR},
        {"Ending Address", MEMARYMAP_END_ADR},
        {"Memory Array Handle", MEMARYMAP_HANDLE},
        {"Partition Width", MEMARYMAP_PWIDTH}
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);
    unsigned char *curLoc;

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>Memory Array Mapped address Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(memArrayMapAddrPage) /sizeof(WEBTOKEN);

    do{
        for (i=0; i<lists; i++)
        {
            getSMBIOSElement(addr, sh,  type, memArrayMapAddrPage[i].offset, out);
            current->len += sprintf(current->payLoadAddr + current->len,"<td>%s: </td><td>%s</td><tr>\n", memArrayMapAddrPage[i].token, out);
            current = chkDataListBufSize(current);
        }
        current->len += sprintf(current->payLoadAddr + current->len,"%s","<tr><tr>");
        curLoc = addr;
    }while (getSMBIOSTypeNext(curLoc, type, &addr));

    free(out);
    return current->len;
}


int sysBootPage(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)
{
    WEBTOKEN sysBoot[] =
    {
        {"Boot Status", SYSBOOTSTATUS_STATUS},
    };

    int lists = 0 , i = 0;
    //unsigned char out[255];
    INT8U *out = malloc(255);

    current->len += sprintf(current->payLoadAddr + current->len,"%s","<H2>System Boot Information</H2>\n");
    current->len += sprintf(current->payLoadAddr + current->len,"%s",smbiosHtmlTBL);
    lists = sizeof(sysBoot) /sizeof(WEBTOKEN);

    for (i=0; i<lists; i++)
    {
        getSMBIOSElement(addr, sh,  type, sysBoot[i].offset, out);
        current->len += sprintf(current->payLoadAddr + current->len,smbiosHtmlTD, sysBoot[i].token, out);
        current = chkDataListBufSize(current);
    }
    free(out);
    return current->len;
}



int (*pageFun[])(DataList *current, unsigned char *addr, SMBIOSHeader *sh, enum SMBIOSType type)=
{
    biosPage,
    sysInfoPage,
    basedBoardPage,
    chassisPage,
    processorPage, /*4*/
    0,
    0,
    cachePage,/*7*/
    portConnectorPage,
    sysSlotPage,
    onBoardDevPage,/*10*/
    0,
    0,
    0,
    0,
    sysEventLogPage,/*15*/
    phyMemArrayPage,
    memDevPage,
    0,
    memArrayMapAddrPage,/*19*/
    0,
    0,
    0,
    0,
    0,
    0,/*25*/
    0,
    0,
    0,
    0,
    0,
    0,
    sysBootPage
};

/**
* Description:	Do basic authentication
*
* Parameters:
*				type ->	SMBIOS type
*				buf  ->	output data
*
* Returns:		size
*
*/
int genSMBIOSWebPage(int type, DataList *current)
{
    DataList *tmp = current;

    //enum SMBIOSType type = SYS_BOOT_STATUS;
    unsigned char* addr;
    SMBIOSHeader sh ={0};

    addr = getSMBIOSTypeAddr(type);
    if (addr == 0)
    {
        DEBUGMSG(SMBIOS_DEBUG, "Can not find specified type\n");
        return -1;
    }

    toSMBIOSHeader(&sh, addr);
    current->len += sprintf(current->payLoadAddr + current->len, "%s ",htmlHead);

    pageFun[type](current, addr, &sh, type);

    while (current->next)
    {
        tmp = current->next;
        current = current->next;
    }

    tmp->len += sprintf(tmp->payLoadAddr + tmp->len,"%s\n%s","</table><hr>",htmlTail);
    return 0;
}

