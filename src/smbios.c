/*
******************************* SOURCE FILE *******************************

Copyright (c) Realtek Semiconductor Corp. All rights reserved.

File Name:	smbios.c

Abstract:	SMBIOS parser

*****************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "smbios.h"
#include "debug.h"
#include "bsp.h"
#include "lib.h"

extern DPCONF *dpconf;
extern unsigned char *smbiosptr;

#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
void toSMBIOSHeader(SMBIOSHeader *h, unsigned char *data)
{
    h->type = data[0];
    h->len = data[1];
    h->handle = data[2] + (data[3]<<8);
    h->data = data;
}
#endif
int getSysUID(unsigned char *p, unsigned char* out)
{
    int only0xFF=1, only0x00=1;
    int i;

    for (i=0; i<16 && (only0x00 || only0xFF); i++)
    {
        if (p[i]!=0x00) only0x00=0;
        if (p[i]!=0xFF) only0xFF=0;
    }

    if (only0xFF)
    {
        sprintf(out, "Not Present");
        return 0;
    }
    if (only0x00)
    {
        sprintf(out, "Not Present");
        return 0;
    }
    sprintf(out,"%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X ",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
            p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

   // sprintf(out,"%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X ",
   //         p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
   //         p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);

    return 0;
}
int getBIOSChar(SBU64 code, SMBIOSHeader *sh, unsigned char* out)
{
    static const char *characteristics[]= {
        "BIOS characteristics not supported", /* 3 */
        "ISA is supported",
        "MCA is supported",
        "EISA is supported",
        "PCI is supported",
        "PC Card (PCMCIA) is supported",
        "PNP is supported",
        "APM is supported",
        "BIOS is upgradeable",
        "BIOS shadowing is allowed",
        "VLB is supported",
        "ESCD support is available",
        "Boot from CD is supported",
        "Selectable boot is supported",
        "BIOS ROM is socketed",
        "Boot from PC Card (PCMCIA) is supported",
        "EDD is supported",
        "Japanese floppy for NEC 9800 1.2 MB is supported (int 13h)",
        "Japanese floppy for Toshiba 1.2 MB is supported (int 13h)",
        "5.25\"/360 KB floppy services are supported (int 13h)",
        "5.25\"/1.2 MB floppy services are supported (int 13h)",
        "3.5\"/720 KB floppy services are supported (int 13h)",
        "3.5\"/2.88 MB floppy services are supported (int 13h)",
        "Print screen service is supported (int 5h)",
        "8042 keyboard services are supported (int 9h)",
        "Serial services are supported (int 14h)",
        "Printer services are supported (int 17h)",
        "CGA/mono video services are supported (int 10h)",
        "NEC PC-98" /* 31 */
    };

    static int i = 4;

    /*
    * This isn't very clear what this bit is supposed to mean
    */

    if (code.l&(1<<3))
    {
        sprintf(out,"%s ",	characteristics[i]);
        return 0;
    }

    for (; i <= 31; )
    {
        if (code.l&(1<<i))
        {
            sprintf(out,"%s ",	characteristics[i-3]);
            i++;
            return 1;
        }
        i++;
    }
    i = 4;
    return 0;
}
int getBIOSCharE1(SMBIOSHeader *sh,int e,unsigned char code, unsigned char* out)
{
    /* 3.3.1.2.1 */
    static const char *characteristics[]= {
        "ACPI is supported", /* 0 */
        "USB legacy is supported",
        "AGP is supported",
        "I2O boot is supported",
        "LS-120 boot is supported",
        "ATAPI Zip drive boot is supported",
        "IEEE 1394 boot is supported",
        "Smart battery is supported" /* 7 */
    };

    static int i = 0;

    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }

    for (; i <= 7;)
    {
        if (code&(1<<i))
        {
            sprintf(out,"%s ",	characteristics[i]);
            i++;
            return 1;
        }
        i++;
    }

    i = 0;
    return 0;
}

int getBIOSCharE2(SMBIOSHeader *sh,int e,unsigned char code,unsigned char* out)
{
    /* 3.3.1.2.2 */
    static const char *characteristics[]= {
        "BIOS boot specification is supported", /* 0 */
        "Function key-initiated network boot is supported",
        "Targeted content distribution is supported" /* 2 */
    };

    static int i = 0;

    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }

    for (; i <= 2;)
    {
        if (code&(1<<i))
        {
            sprintf(out,"%s ",	characteristics[i]);
            i++;
            return 1;
        }
        i++;
    }

    i = 0;
    return 0;
}

#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
unsigned char* getSMBIOSTypeAddr(enum SMBIOSType type)
{
    unsigned char* data =  (smbiosptr+32);
    int count = 0;
    int i = 0,len=0;
    //int aa=0;
    SMBIOS_Table_EP *SMBIOSTableEP = (SMBIOS_Table_EP *)smbiosptr;

    // while ( count < SMBIOSTableEP->StNum )
    while ( len < SMBIOSTableEP->StTableLen )
    {
        SMBIOSHeader sh = {0};
        toSMBIOSHeader(&sh, data);

        if ( sh.len < 4)
        {
            for (i =0; i < sh.len; i++ )
                DEBUGMSG(SMBIOS_DEBUG,"%x ",data[i]);
            DEBUGMSG(SMBIOS_DEBUG,"\nSMBIOS Header Error\n");
            return NULL;
        }

        if (sh.type == type)
            return data;

        len= len+ sh.len;
        //Find next SMBIOS structure
        data = data + sh.len;
        //Each structure is terminated by a double 0
        while ((data[0]!=0) || (data[1]!=0))
            data++;
        len +=2;
        data += 2;
        count++;
    }
    return 0;
}

char getSMBIOSTypeNext(unsigned char* p,enum SMBIOSType type, unsigned char** next)
{
    const char FOUND = 1;
    const char NONE = 0;

    unsigned char* data =  p;
    //int count = 0;
    //int i = 0;
    SMBIOSHeader sh = {0};
    toSMBIOSHeader(&sh, data);
    if ( sh.len < 4)
    {
        DEBUGMSG(SMBIOS_DEBUG, "SMBIOS Header Error\n");
        return NONE;
    }
    data = data + sh.len;
    while ((data[0]!=0) || (data[1]!=0))
        data++;
    data += 2;
    do
    {

        toSMBIOSHeader(&sh, data);
        if ( sh.len < 4)
        {
            DEBUGMSG(SMBIOS_DEBUG, "Next SMBIOS Header Error\n");
            return NONE;
        }
        if(sh.type == 0x7f)
            return NONE;

        //if(sh.type == SMBIOS_Inactive)
        //	continue;

        //sort issue
        if (sh.type == type)
        {
            *next = data;
            return FOUND;
        }

        data = data + sh.len;
        while ((data[0]!=0) || (data[1]!=0))
            data++;
        data += 2;

    } while (1);
}

void getBIOSElemnetByte(unsigned char *p,int e,unsigned char* out)
{
    sprintf(out, "%d ", p[e]);
}

void getBIOSElemnetWord(unsigned char *p,int e,unsigned char* out)
{
    int i = (int)p[e] + ((int)p[e + 1]<<8);
    sprintf(out, "%d ", i);
}

void getBIOSElemnetDWord(unsigned char *p,int e,unsigned char* out)
{
    unsigned int i = (unsigned int)p[e] + ((unsigned int)p[e + 1]<<8) +
                     ((unsigned int)p[e + 2]<<16) + ((unsigned int)p[e + 3]<<24);
    sprintf(out, "%u ", i);
}

void getBIOSElemnetQWord(unsigned char *p, int e,unsigned char* out)
{
    SBU64 tmp;
    int i;
    i = (int)p[e];
    i += (int)p[e + 1]<<8;
    i += (int)p[e + 2]<<16;
    i += (int)p[e + 3]<<24;
    tmp.l = i;
    i = (int)p[e + 4];
    i += (int)p[e + 5]<<8;
    i += (int)p[e + 6]<<16;
    i += (int)p[e + 7]<<24;
    tmp.h = i;
    sprintf(out,"%d %d ",tmp.l, tmp.h );
}

void getBIOSElemnetString(unsigned char *p,int e,unsigned char* out)
{
    unsigned char* tmp = p + (int)(p[1]);
    int offset = p[e];
    int len, i;

    if (offset == 0)
    {
        sprintf(out, "UnKnown ");
        return;
    }

    //Find string start adress
    while ((offset > 1) && *tmp)
    {
        tmp += strlen(tmp);
        tmp++;
        offset--;
    }

    len = strlen(tmp);
    //Use . to replace controll ascii code
    for (i=0; i < len; i++)
    {
        if (tmp[i]<32 || tmp[i]==127)
            out[i]='.';
        else
            out[i] = tmp[i];
    }
    out[len] = '\0';
}

int getBIOSElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case BIOS_ROM_SIZE:
    case BIOS_MAJOR:
    case BIOS_MINOR:
    case BIOS_FIRMWARE_MAJOR:
    case BIOS__FIRMWARE_MINOR:
        getBIOSElemnetByte(p, e, out);
        break;
    case BIOS_START_ADDR:
        getBIOSElemnetWord(p, e, out);
        break;
    case BIOS_CHARS:
        getBIOSElemnetQWord(p, e, out);
        break;
    case BIOS_VENDOR:
    case BIOS_VER:
    case BIOS_RELEASE_DATE:
        getBIOSElemnetString(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

#endif

#if 1

void getBIOSElemnetString_swpatch(unsigned char *p,int e,unsigned char* out)
{
    unsigned char* tmp = p + (int)(p[1]);
    int offset = p[e];
    int len, i;

	//printf("[RTK]+ getBIOSElemnetString_swpatch\n");

    if (offset == 0)
    {
        sprintf(out, "UnKnown ");
        return;
    }

    //Find string start adress
    while ((offset > 1) && *tmp)
    {
        tmp += strlen(tmp);
        tmp++;
        offset--;
    }

    len = strlen(tmp);
    //Use . to replace controll ascii code

	if(len > 1024){//newmem() allocate 1024
	 	//printf("[RTK]getBIOSElemnetString_swpatch: change length=%d to 1024\n",len);
		len = 1024;
	}
	
    for (i=0; i< len; i++)
    {
        if (tmp[i]<32 || tmp[i]==127)
            out[i]='.';
        else
            out[i] = tmp[i];
    }
    out[len] = '\0';
	
    if(out[0] == 0xFF){
		// printf("[RTK]getBIOSElemnetString_swpatch: change to unknown\n");
		 sprintf(out, "UnKnown ");
    }
    //printf("[RTK] e=%d , len=%d\n",e,len);
	
}



int getBIOSElemnetData_swpatch(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case BIOS_ROM_SIZE:
    case BIOS_MAJOR:
    case BIOS_MINOR:
    case BIOS_FIRMWARE_MAJOR:
    case BIOS__FIRMWARE_MINOR:
        getBIOSElemnetByte(p, e, out);
        break;
    case BIOS_START_ADDR:
        getBIOSElemnetWord(p, e, out);
        break;
    case BIOS_CHARS:
        getBIOSElemnetQWord(p, e, out);
        break;
    case BIOS_VENDOR:
    case BIOS_VER:
    case BIOS_RELEASE_DATE:
        //getBIOSElemnetString(p, e, out);
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}



#endif

int getSysWakeUPType(unsigned char code, unsigned char* out)
{
    static const char *type[]= {
        "Reserved", /* 0x00 */
        "Other",
        "Unknown",
        "APM Timer",
        "Modem Ring",
        "LAN Remote",
        "Power Switch",
        "PCI PME#",
        "AC Power Restored" /* 0x08 */
    };

    if (code<=0x08)
    {
        sprintf(out,"%s ",type[code]);
        return 0;
    }
    return -1;
}
int getSystemInfoElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case SYSINFO_MAN:
    case SYSINFO_PN:
    case SYSINFO_VER:
    case SYSINFO_SN:
    case SYSINFO_SKU:
    case SYSINFO_FAMILY:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case SYSINFO_UUID:
        getSysUID(p+e, out);
        break;
    case SYSINFO_WAKEUP_TYPE:
        getSysWakeUPType(p[e], out);
        break;
    default:
        sprintf(out," ");
    }

    return 0;
}

int getBaseBoardFeatures(SMBIOSHeader *sh,int e, unsigned char code,unsigned char* out)
{
    /* 3.3.3.1 */
    static const char *features[]= {
        "Board is a hosting board", /* 0 */
        "Board requires at least one daughter board",
        "Board is removable",
        "Board is replaceable",
        "Board is hot swappable" /* 4 */
    };

    static int i = 0;


    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }

    if ((code&0x1F)==0)
    {
        sprintf(out," None\n");
        return 0;
    }

    for (; i <= 4;)
    {
        if (code&(1<<i))
        {
            sprintf(out,"%s ",	features[i]);
            i++;
            return 1;
        }
        i++;
    }

    i = 0;
    return 0;
}

int getBaseBoardType(unsigned char code,unsigned char* out)
{
    static const char *type[]= {
        "Unknown", /* 0x01 */
        "Other",
        "Server Blade",
        "Connectivity Switch",
        "System Management Module",
        "Processor Module",
        "I/O Module",
        "Memory Module",
        "Daughter Board",
        "Motherboard",
        "Processor+Memory Module",
        "Processor+I/O Module",
        "Interconnect Board" /* 0x0D */
    };

    if (code>=0x01 && code<=0x0D)
    {
        sprintf(out,"%s ",	type[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;

}

int getBaseBoardInfoElemnetData(unsigned char *p,SMBIOSHeader *sh,int e,unsigned char* out)
{
    /*
    	BBINFO_OBJ_HANDLE = 0x0F*/
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }

    switch (e)
    {
    case BBINFO_OBJ_NUM:
        getBIOSElemnetByte(p, e, out);
        break;
    case BBINFO_CHASSIS_HANDLE:
        getBIOSElemnetWord(p, e, out);
        break;
    case BBINFO_MAN:
    case BBINFO_PN:
    case BBINFO_VER:
    case BBINFO_SN:
    case BBINFO_ASSET_TAG:
    case BBINFO_LOCATION_IN_CHASSIS:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case BBINFO_BOARD_TYPE:
        getBaseBoardType(p[e], out);
        break;
    default:
        sprintf(out," ");
    }

    return 0;
}

int getChassisType(unsigned char code,unsigned char* out)
{
    /* 3.3.4.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Desktop",
        "Low Profile Desktop",
        "Pizza Box",
        "Mini Tower",
        "Tower",
        "Portable",
        "Laptop",
        "Notebook",
        "Hand Held",
        "Docking Station",
        "All In One",
        "Sub Notebook",
        "Space-saving",
        "Lunch Box",
        "Main Server Chassis", /* master.mif says System */
        "Expansion Chassis",
        "Sub Chassis",
        "Bus Expansion Chassis",
        "Peripheral Chassis",
        "RAID Chassis",
        "Rack Mount Chassis",
        "Sealed-case PC",
        "Multi-system",
        "CompactPCI",
        "AdvancedTCA" /* 0x1B */
    };

    if (code>=0x01 && code<=0x1B)
    {
        sprintf(out,"%s ",	type[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}

int getChassisState(unsigned char code, unsigned char* out)
{
    /* 3.3.4.2 */
    static const char *state[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Safe", /* master.mif says OK */
        "Warning",
        "Critical",
        "Non-recoverable" /* 0x06 */
    };

    if (code>=0x01 && code<=0x06)
    {
        sprintf(out,"%s ",	state[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}

int getChassisSecurityStatus(unsigned char code, unsigned char* out)
{
    /* 3.3.4.3 */
    static const char *status[]= {
        "Other", /* 0x01 */
        "Unknown",
        "None",
        "External Interface Locked Out",
        "External Interface Enabled" /* 0x05 */
    };

    if (code>=0x01 && code<=0x05)
    {
        sprintf(out,"%s ",	status[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}
int getChassisElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    /*
    CHASSIS_ELEMENT_COUNT = 0x13,
    CHASSIS_ELEMENT_LEN = 0x14,
    CHASSIS_ELEMENT = 0x15
    	*/
    int c = 0;
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case CHASSIS_HEIGHT:
    case CHASSIS_POWER_NUM:
        getBIOSElemnetByte(p, e, out);
        sscanf(out,"%d",&c);
        if (!c)
            sprintf(out, "unspecified ");
        break;
    case CHASSIS_OEM:
        getBIOSElemnetWord(p, e, out);
        break;
    case CHASSIS_MAN:
    case CHASSIS_VER:
    case CHASSIS_SN:
    case CHASSIS_ASSET_TAG:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case CHASSIS_TYPE:
        getChassisType((unsigned char)(p[e]&0x7F) ,out);
        break;
    case CHASSIS_BOOTUP:
    case CHASSIS_POWER:
    case CHASSIS_THERMAL:
        getChassisState(p[e], out);
        break;
    case CHASSIS_SECURITY:
        getChassisSecurityStatus(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getProcessorType(unsigned char code, unsigned char* out)
{
    /* 3.3.5.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Central Processor",
        "Math Processor",
        "DSP Processor",
        "Video Processor" /* 0x06 */
    };

    if (code>=0x01 && code<=0x06)
    {
        sprintf(out,"%s ",	type[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}

int getProcessorFamily(unsigned char code, unsigned char* out)
{
    /* 3.3.5.2 */
    static const char *family[256]= {
        0, /* 0x00 */
        "Other",
        "Unknown",
        "8086",
        "80286",
        "80386",
        "80486",
        "8087",
        "80287",
        "80387",
        "80487",
        "Pentium",
        "Pentium Pro",
        "Pentium II",
        "Pentium MMX",
        "Celeron",
        "Pentium II Xeon",
        "Pentium III",
        "M1",
        "M2",
        0, /* 0x14 */
        0,
        0,
        0, /* 0x17 */
        "Duron",
        "K5",
        "K6",
        "K6-2",
        "K6-3",
        "Athlon",
        "AMD2900",
        "K6-2+",
        "Power PC",
        "Power PC 601",
        "Power PC 603",
        "Power PC 603+",
        "Power PC 604",
        "Power PC 620",
        "Power PC x704",
        "Power PC 750",
        0, /* 0x28 */
        0,
        0,
        0,
        0,
        0,
        0,
        0,/* 0x2F */
        "Alpha",
        "Alpha 21064",
        "Alpha 21066",
        "Alpha 21164",
        "Alpha 21164PC",
        "Alpha 21164a",
        "Alpha 21264",
        "Alpha 21364",
        0, /* 0x38 */
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0x3F */
        "MIPS",
        "MIPS R4000",
        "MIPS R4200",
        "MIPS R4400",
        "MIPS R4600",
        "MIPS R10000",
        0, /* 0x46 */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0x4F */
        "SPARC",
        "SuperSPARC",
        "MicroSPARC II",
        "MicroSPARC IIep",
        "UltraSPARC",
        "UltraSPARC II",
        "UltraSPARC IIi",
        "UltraSPARC III",
        "UltraSPARC IIIi",
        0, /* 0x59 */
        0,
        0,
        0,
        0,
        0,
        0, /* 0x5F */
        "68040",
        "68xxx",
        "68000",
        "68010",
        "68020",
        "68030",
        0, /* 0x66 */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0x6F */
        "Hobbit",
        0, /* 0x71 */
        0,
        0,
        0,
        0,
        0,
        0, /* 0x77 */
        "Crusoe TM5000",
        "Crusoe TM3000",
        "Efficeon TM8000",
        0, /* 0x7B */
        0,
        0,
        0,
        0, /* 0x7F */
        "Weitek",
        0, /* 0x81 */
        "Itanium",
        "Athlon 64",
        "Opteron",
        "Sempron",
        "Turion 64",
        "Dual-Core Opteron",
        "Athlon 64 X2",
        0, /* 0x89 */
        0,
        0,
        0,
        0,
        0,
        0, /* 0x8F */
        "PA-RISC",
        "PA-RISC 8500",
        "PA-RISC 8000",
        "PA-RISC 7300LC",
        "PA-RISC 7200",
        "PA-RISC 7100LC",
        "PA-RISC 7100",
        0, /* 0x97 */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0x9F */
        "V30",
        0, /* 0xA1 */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0xAF */
        "Pentium III Xeon",
        "Pentium III Speedstep",
        "Pentium 4",
        "Xeon",
        "AS400",
        "Xeon MP",
        "Athlon XP",
        "Athlon MP",
        "Itanium 2",
        "Pentium M",
        "Celeron D",
        "Pentium D",
        "Pentium EE",
        0, /* 0xBD */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0xC7 */
        "IBM390",
        "G4",
        "G5",
        0, /* 0xCB */
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0, /* 0xF9 */
        "i860",
        "i960",
        0, /* 0xFC */
        0,
        0,
        0 /* 0xFF */
        /* master.mif has values beyond that, but they can't be used for DMI */
    };

    if (family[code]!=0)
    {
        sprintf(out,"%s ", family[code]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}

#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
int grtProcessorID(unsigned char *p,SMBIOSHeader *sh, int e, unsigned char* out)
{
    if (sh->len < e + 7)
    {
        sprintf(out," ");
    }
    else
    {
        sprintf(out,"%02X %02X %02X %02X %02X %02X %02X %02X ", p[e], p[e+1], p[e+2], p[e+3],
                p[e+4], p[e+5], p[e+6], p[e+7] );
    }
    return 0;

}
#endif

int getProcessorVoltage(unsigned char code, unsigned char* out)
{
    /* 3.3.5.4 */
    static const char *voltage[]= {
        "5.0 V", /* 0 */
        "3.3 V",
        "2.9 V" /* 2 */
    };

    int i = 0;

    if (code&0x80)
        sprintf(out,"%d.%d V ", (code&0x7f)/10, (code&0x7f) - (10*((code&0x7f)/10)) );
    else
    {
        for (i=0; i<=2; i++)
            if (code&(1<<i))
                sprintf(out, " %s ", voltage[i]);
        if (code==0x00)
            sprintf(out, " Unknown ");
    }

    return 0;
}

int getProcessorStatus(unsigned char code, unsigned char* out)
{
    static const char *status[]= {
        "Unknown", /* 0x00 */
        "Enabled",
        "Disabled By User",
        "Disabled By BIOS",
        "Idle", /* 0x04 */
        "Other" /* 0x07 */
    };
    if (!(code&(1<<6)))
    {
        sprintf(out,"Unpopulated ");
        return 0;
    }
    code = code & 0x07;

#if 1
	//return a int value;
	sprintf(out,"%d ", code);
	return 0;
#else		

    if (code<=0x04)
    {
       sprintf(out,"%s ", status[code]);
       
        return 0;
    }
    if (code==0x07)
    {
        sprintf(out,"%s ", status[code]);
	
        return 0;
    }
    sprintf(out," ");
    return 0;
#endif
	
}

int getProcessorUpgrade(unsigned char code, unsigned char* out)
{
    /* 3.3.5.5 */
    static const char *upgrade[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Daughter Board",
        "ZIF Socket",
        "Replaceable Piggy Back",
        "None",
        "LIF Socket",
        "Slot 1",
        "Slot 2",
        "370-pin Socket",
        "Slot A",
        "Slot M",
        "Socket 423",
        "Socket A (Socket 462)",
        "Socket 478",
        "Socket 754",
        "Socket 940",
        "Socket 939",
        "Socket mPGA604",
        "Socket LGA771",
        "Socket LGA775" /* 0x15 */
    };

    if (code>=0x01 && code<=0x15)
        sprintf(out,"%s ", upgrade[code - 1]);
    else
        sprintf(out," ");
    return 0;
}

#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
int getProcessorCache(unsigned short code, unsigned char* out)
{
    //???? tmp solution
    if (code==0xFFFF)
    {
        //if(ver>=0x0203)
        sprintf(out,"Not Provided ");
    }
    else
        sprintf(out, "0x%04X ", code);
    return 0;
}
#endif

int getProcessorCharacteristics(unsigned short code, unsigned char* out)
{
    /* 3.3.5.9 */
    static const char *characteristics[]= {
        "unKnown",/*Bit 1 */
        "64-bit capable" /*Bit 2 */
    };

    if ((code&0x0002)==0)
        sprintf(out, "%s ", characteristics[0]);
    else if ((code&0x0004)==0)
        sprintf(out, "%s ", characteristics[1]);
    else
        sprintf(out, " ");

    return 0;
}

int getProcessorElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int us = 0;
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case PROCESSOR_CORE_COUNT:
    case PROCESSOR_CORE_ENABLED:
    case PROCESSOR_THREAD_COUNT:
        getBIOSElemnetByte(p, e, out);
        break;
    case PROCESSOR_SKT_DESIGN:
    case PROCESSOR_MAN:
    case PROCESSOR_VER:
    case PROCESSOR_SN:
    case PROCESSOR_ASSET_TAG:
    case PROCESSOR_PN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case PROCESSOR_ECLK:
    case PROCESSOR_MAX_SPEED:
    case PROCESSOR_CURRENT_SPEED:
        getBIOSElemnetWord(p, e, out);
        break;
    case PROCESSOR_TYPE:
        getProcessorType(p[e], out);
        break;
    case PROCESSOR_FAMILY:
        getProcessorFamily(p[e], out);
        break;
    case PROCESSOR_ID:
        grtProcessorID(p, sh, e, out);
        break;
    case PROCESSOR_VOLTAGE:
        getProcessorVoltage(p[e], out);
        break;
    case PROCESSOR_STATUS:
        getProcessorStatus(p[e], out);
        break;
    case PROCESSOR_UPGRADE:
        getProcessorUpgrade(p[e], out);
        break;
    case PROCESSOR_L1_CACHE:
    case PROCESSOR_L2_CACHE:
    case PROCESSOR_L3_CACHE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &us);
        getProcessorCache((unsigned short)us, out);
        break;
    case PROCESSOR_CHARS:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &us);
        getProcessorCharacteristics((unsigned short)us, out);
        break;
    case PROCESSOR_FAMILY2:
        /*Equal to PROCESSOR_FAMILY*/
        sprintf(out," ");
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getMemCtlErrDetectMethod(unsigned char code, unsigned char* out)
{
    /* 3.3.6.1 */
    static const char *method[]= {
        "Other", /* 0x01 */
        "Unknown",
        "None",
        "8-bit Parity",
        "32-bit ECC",
        "64-bit ECC",
        "128-bit ECC",
        "CRC" /* 0x08 */
    };

    if (code>=0x01 && code<=0x08)
    {
        sprintf(out,"%s ",method[code-0x01]);
    }
    else
    {
        sprintf(out," ");
    }
    return 0;
}

int getMemCtlErrCorrectCap(unsigned char code, unsigned char* out)
{
    /* 3.3.6.2 */
    static const char *capabilities[]= {
        "Other", /* 0 */
        "Unknown",
        "None",
        "Single-bit Error Correcting",
        "Double-bit Error Correcting",
        "Error Scrubbing" /* 5 */
    };

    if ((code&0x3F)==0)
        sprintf(out, " ");
    else
    {
        int i;
        int rs = 0;
        for (i=0; i<=5; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", capabilities[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }
    return 0;
}

int getMemCtlInterleave(unsigned char code, unsigned char* out)
{
    /* 3.3.6.3 */
    static const char *interleave[]= {
        "Other", /* 0x01 */
        "Unknown",
        "One-way Interleave",
        "Two-way Interleave",
        "Four-way Interleave",
        "Eight-way Interleave",
        "Sixteen-way Interleave" /* 0x07 */
    };

    if (code>=0x01 && code<=0x07)
        sprintf(out, "%s ", interleave[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getMemCtlSpeed(unsigned short code, unsigned char* out)
{
    /* 3.3.6.4 */
    const char *speeds[]= {
        "Other", /* 0 */
        "Unknown",
        "70 ns",
        "60 ns",
        "50 ns" /* 4 */
    };

    if ((code&0x001F)==0)
        sprintf(out, " ");
    else
    {
        int i;
        int rs = 0;
        for (i=0; i<=4; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", speeds[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }
    return 0;
}

int getMemCtlType(unsigned short code, unsigned char* out)
{
    /* 3.3.7.1 */
    static const char *types[]= {
        "Other", /* 0 */
        "Unknown",
        "Standard",
        "FPM",
        "EDO",
        "Parity",
        "ECC",
        "SIMM",
        "DIMM",
        "Burst EDO",
        "SDRAM" /* 10 */
    };

    if ((code&0x07FF)==0)
        sprintf(out, " ");
    else
    {
        int i;
        int rs = 0;
        for (i=0; i<=10; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", types[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }
    return 0;
}


#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
int getMemCtlCfgHandle(unsigned char* p, unsigned char slots, unsigned char code, unsigned char* out)
{
    int i;
    int rs = 0;

    for (i=0; i<slots; i++)
    {
        rs = sprintf(out, "0x%04X ",((int)p[sizeof(unsigned short)*i]) +
                     ((int)p[sizeof(unsigned short)*i + 1]<< 8));
        out = out + rs;
    }
    sprintf(out, " ");
    return 0;
}
#endif

int getMemErrCorectCap(unsigned char code, unsigned char* out)
{
    /* 3.3.6.2 */
    static const char *capabilities[]= {
        "Other", /* 0 */
        "Unknown",
        "None",
        "Single-bit Error Correcting",
        "Double-bit Error Correcting",
        "Error Scrubbing" /* 5 */
    };

    if ((code&0x3F)==0)
        sprintf(out, " ");
    else
    {
        int i;
        int rs = 0;
        for (i=0; i<=5; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", capabilities[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }

    return 0;
}

int getMemoryControlElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int us = 0;
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case MCTL_SLOTS:
        getBIOSElemnetByte(p, e, out);
        break;
    case MCTL_ERR_DETECT_METHOD:
        getMemCtlErrDetectMethod(p[e], out);
        break;
    case MCTL_ERR_CORRECT_CAP:
        getMemCtlErrCorrectCap(p[e], out);
        break;
    case MCTL_INTERLEAVE:
    case MCTL_CUR_INTERLEAVE:
        getMemCtlInterleave(p[e], out);
        break;
    case MCTL_MAX_SIZE:
        sprintf(out, "%d ", (1<<p[e]));
        break;
    case MCTL_SPEED:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &us);
        getMemCtlSpeed((unsigned short)us, out);
        break;
    case MCTL_TYPE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &us);
        getMemCtlType((unsigned short)us, out);
        break;
    case MCTL_VOLTAGE:
        getProcessorVoltage(p[e], out);
        break;
    case MCTL_HANDLE:
        if (sh->len < (0x0F + p[MCTL_SLOTS]*sizeof(unsigned short)))
        {
            sprintf(out, " ");
            return 0;
        }
        getMemCtlCfgHandle(&p[e], p[e-1], p[e] ,out);
        break;
    case MCTL_ERR_CAP_ENABLED:
        if (sh->len < 0x10 + p[MCTL_SLOTS]*sizeof(unsigned short))
        {
            sprintf(out, " ");
            return 0;
        }
        getMemErrCorectCap(p[0x0F+p[0x0E]*sizeof(unsigned short)], out);
        break;
    default:
        sprintf(out," ");
    }

    return 0;
}

#if (CONFIG_VERSION  < IC_VERSION_EP_RevA) || defined(CONFIG_BUILDROM)
int getMemoryModuleBank(unsigned char code, unsigned char* out)
{
    int rs = 0;
    if (code==0xFF)
        sprintf(out, " ");
    else
    {
        if ((code&0xF0)!=0xF0)
            rs = sprintf(out,"%u ", code>>4);
        out = out + rs;
        rs = 0;
        if ((code&0x0F)!=0x0F)
            rs = sprintf(out,"%u ", code&0x0F);
        out = out + rs;
        sprintf(out," ");
    }
    return 0;
}
#endif

int getMemoryModuleType(unsigned char *p,SMBIOSHeader *sh, int e, unsigned char* out)
{
    /* 3.3.7.1 */
    static const char *types[]= {
        "Other", /* 0 */
        "Unknown",
        "Standard",
        "FPM",
        "EDO",
        "Parity",
        "ECC",
        "SIMM",
        "DIMM",
        "Burst EDO",
        "SDRAM" /* 10 */
    };

    unsigned short code = 0;

    if (sh->len <= (e+1))
    {
        sprintf(out," ");
        return 0;
    }

    code = (unsigned short)p[e] + ((unsigned short)p[e + 1]<<8);

    if ((code&0x07FF)==0)
        sprintf(out, " ");
    else
    {
        int i;
        int rs = 0;
        for (i=0; i<=10; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", types[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }
    return 0;
}

int getMemoryModuleSize(unsigned char code, unsigned char* out)
{
    int rs = 0;
    /* 3.3.7.2 */
    switch (code&0x7F)
    {
    case 0x7D:
        rs = sprintf(out,"%s ","Not Determinable");
        break;
    case 0x7E:
        rs = sprintf(out,"%s ","Disabled");
        break;
    case 0x7F:
        rs = sprintf(out,"%s ","Not Installed");
        return 0;
    default:
        rs = sprintf(out,"%u MB ", 1<<(code&0x7F));
    }
    out = out + rs;
    rs = 0;
    if (code&0x80)
        rs = sprintf(out,"%s "," (Double-bank Connection)");
    else
        rs = sprintf(out,"%s "," (Single-bank Connection)");

    out = out + rs;
    sprintf(out," ");
    return 0;
}

int getMemoryModuleErrorStatus(unsigned char code, unsigned char* out)
{
    int rs = 0;

    if (code&(1<<2))
        sprintf(out, "See Event Log ");
    else
    {
        if ((code&0x03)==0)
            rs = sprintf(out, "OK ");
        out = out + rs;
        rs = 0;

        if (code&(1<<0))
            sprintf(out, "Uncorrectable Errors ");
        out = out + rs;
        rs = 0;

        if (code&(1<<1))
            sprintf(out, "Correctable Errors ");
        out = out + rs;
        rs = 0;
        sprintf(out, " ");
    }

    return 0;
}

int getMemoryModuleElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case MM_SPEED:
        getBIOSElemnetByte(p, e, out);
        break;
    case MM_SKT_DSEIGN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case MM_BANK:
        getMemoryModuleBank(p[e], out);
        break;
    case MM_TYPE:
        getMemoryModuleType(p, sh, e, out);
        break;
    case MM_INSTALLED_SIZE:
    case MM_ENABLED_SIZE:
        getMemoryModuleSize(p[e], out);
        break;
    case MM_ERR_STATUS:
        getMemoryModuleErrorStatus(p[e], out);
        break;
    }
    return 0;
}


int getCacheCfg(unsigned short code, unsigned char* out)
{
    int rs = 0;

    switch (code&0x0300)
    {
    case 0x0000:
        rs = sprintf(out, "%s ","Write Through, ");
        break;
    case 0x0100:
        rs = sprintf(out, "%s ","Write Back, ");
        break;
    case 0x0200:
        rs = sprintf(out, "%s ","Varies With Memory Address, ");
        break;
    case 0x0300:
        rs = sprintf(out, "%s ","Unknown, ");
        break;
    }
    out = out + rs;
    rs = 0;

    if (code&0x80)
        rs = sprintf(out, "%s ","Enabled, ");
    else
        rs = sprintf(out, "%s ","Disabled, ");

    out = out + rs;
    rs = 0;

    switch (code&0x60)
    {
    case 0x00:
        rs = sprintf(out, "%s ","Internal, ");
        break;
    case 0x20:
        rs = sprintf(out, "%s ","External, ");
        break;
    case 0x40:
        rs = sprintf(out, "%s ","Reserved, ");
        break;
    case 0x60:
        rs = sprintf(out, "%s ","Unknown, ");
        break;
    }
    out = out + rs;
    rs = 0;

    switch (code&0x08)
    {
    case 0x08:
        rs = sprintf(out, "%s ","Socketed, ");
        break;
    case 0x00:
        rs = sprintf(out, "%s ","No Socketed,");
        break;
    }
    out = out + rs;
    rs = 0;

    rs = sprintf(out, "Cache Level:<%d>",(code&0x7) + 1);
    out = out + rs;
    sprintf(out, " ");

    return 0;
}

int getCacheTypes(unsigned short code, unsigned char* out)
{
    /* 3.3.8.2 */
    static const char *types[]= {
        "Other", /* 0 */
        "Unknown",
        "Non-burst",
        "Burst",
        "Pipeline Burst",
        "Synchronous",
        "Asynchronous" /* 6 */
    };
    int rs = 0;
    if ((code&0x007F)==0)
        sprintf(out, " ");
    else
    {
        int i;
        for (i=0; i<=6; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out, "%s ", types[i]);
                out = out + rs;
                rs = 0;
            }
        }
        sprintf(out, " ");
    }

    return 0;
}

int getCacheErrCorrecttype(unsigned char code, unsigned char* out)
{
    /* 3.3.8.3 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "None",
        "Parity",
        "Single-bit ECC",
        "Multi-bit ECC" /* 0x06 */
    };

    if (code>=0x01 && code<=0x06)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getCacheType(unsigned char code, unsigned char* out)
{
    /* 3.3.8.4 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Instruction",
        "Data",
        "Unified" /* 0x05 */
    };

    if (code>=0x01 && code<=0x05)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getCacheAssociativity(unsigned char code, unsigned char* out)
{
    /* 3.3.8.5 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Direct Mapped",
        "2-way Set-associative",
        "4-way Set-associative",
        "Fully Associative",
        "8-way Set-associative",
        "16-way Set-associative" /* 0x08 */
    };

    if (code>=0x01 && code<=0x08)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getCacheElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int us = 0;

    switch (e)
    {
    case CACHE_SPEED:
        getBIOSElemnetByte(p, e, out);
        break;
    case CACHE_SKT_DSEIGN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case CACHE_CFG:
        getBIOSElemnetWord(p, e, out);
        /*Note sscanf can not read a short integer */
        sscanf(out,"%u", &us);
        getCacheCfg((unsigned short)us, out);
        break;
    case CACHE_MAX_SIZE:
    case CACHE_INSTALL_SIZE:
        getBIOSElemnetWord(p, e, out);
        /*Note sscanf can not read a short integer */
        sscanf(out,"%u", &us);
        if (us&0x8000)
            sprintf(out, "%u ", (us&0x7FFF)<<6);
        else
            sprintf(out,"%u ", us);
        break;
    case CACHE_SUPPORT_SRAM_TYPE:
    case CACHE_CURRENT_SRAM_TYPE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &us);
        getCacheTypes((unsigned short)us, out);
        break;
    case CACHE_ERR_CORRECT_TYPE:
        getCacheErrCorrecttype(p[e], out);
        break;
    case CACHE_SYS_TYPE:
        getCacheType(p[e], out);
        break;
    case CACHE_SUPPORT_ASSOC:
        getCacheAssociativity(p[e], out);
        break;
    default:
        sprintf(out," ");
    }

    return 0;
}

int getPortConnectorType(unsigned char code, unsigned char* out)
{
    /* 3.3.9.2 */
    static const char *type[]= {
        "None", /* 0x00 */
        "Centronics",
        "Mini Centronics",
        "Proprietary",
        "DB-25 male",
        "DB-25 female",
        "DB-15 male",
        "DB-15 female",
        "DB-9 male",
        "DB-9 female",
        "RJ-11",
        "RJ-45",
        "50 Pin MiniSCSI",
        "Mini DIN",
        "Micro DIN",
        "PS/2",
        "Infrared",
        "HP-HIL",
        "Access Bus (USB)",
        "SSA SCSI",
        "Circular DIN-8 male",
        "Circular DIN-8 female",
        "On Board IDE",
        "On Board Floppy",
        "9 Pin Dual Inline (pin 10 cut)",
        "25 Pin Dual Inline (pin 26 cut)",
        "50 Pin Dual Inline",
        "68 Pin Dual Inline",
        "On Board Sound Input From CD-ROM",
        "Mini Centronics Type-14",
        "Mini Centronics Type-26",
        "Mini Jack (headphones)",
        "BNC",
        "IEEE 1394",
        "SAS/SATA Plug Receptacle" /* 0x22 */
    };
    static const char *type_0xA0[]= {
        "PC-98", /* 0xA0 */
        "PC-98 Hireso",
        "PC-H98",
        "PC-98 Note",
        "PC-98 Full" /* 0xA4 */
    };

    if (code<=0x22)
        sprintf(out, "%s ", type[code]);
    else if (code>=0xA0 && code<=0xA4)
        sprintf(out, "%s ", type_0xA0[code-0xA0]);
    else if (code==0xFF)
        sprintf(out, "%s ", "Other");
    else
        sprintf(out, " ");
    return 0;
}

int getPortType(unsigned char code, unsigned char* out)
{
    /* 3.3.9.3 */
    static const char *type[]= {
        "None", /* 0x00 */
        "Parallel Port XT/AT Compatible",
        "Parallel Port PS/2",
        "Parallel Port ECP",
        "Parallel Port EPP",
        "Parallel Port ECP/EPP",
        "Serial Port XT/AT Compatible",
        "Serial Port 16450 Compatible",
        "Serial Port 16550 Compatible",
        "Serial Port 16550A Compatible",
        "SCSI Port",
        "MIDI Port",
        "Joystick Port",
        "Keyboard Port",
        "Mouse Port",
        "SSA SCSI",
        "USB",
        "Firewire (IEEE P1394)",
        "PCMCIA Type I",
        "PCMCIA Type II",
        "PCMCIA Type III",
        "Cardbus",
        "Access Bus Port",
        "SCSI II",
        "SCSI Wide",
        "PC-98",
        "PC-98 Hireso",
        "PC-H98",
        "Video Port",
        "Audio Port",
        "Modem Port",
        "Network Port",
        "SATA",
        "SAS" /* 0x21 */
    };
    static const char *type_0xA0[]= {
        "8251 Compatible", /* 0xA0 */
        "8251 FIFO Compatible" /* 0xA1 */
    };

    if (code<=0x21)
        sprintf(out, "%s ", type[code]);
    else if (code>=0xA0 && code<=0xA1)
        sprintf(out, "%s ", type_0xA0[code-0xA0]);
    else if (code==0xFF)
        sprintf(out, "%s ", "Other");
    else
        sprintf(out, " ");
    return 0;
}
int getPConnectorElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    switch (e)
    {
    case PCON_INTER_REF_DESIGN:
    case PCON_EXTER_REF_DESIGN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case PCON_INTER_TYPE:
    case PCON_EXTER_TYPE:
        getPortConnectorType(p[e], out);
        break;
    case PCON_PORT_TYPE:
        getPortType(p[e], out);
        break;
    }
    return 0;
}

int getSlotType(unsigned char code, unsigned char* out)
{
    /* 3.3.10.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "ISA",
        "MCA",
        "EISA",
        "PCI",
        "PC Card (PCMCIA)",
        "VLB",
        "Proprietary",
        "Processor Card",
        "Proprietary Memory Card",
        "I/O Riser Card",
        "NuBus",
        "PCI-66",
        "AGP",
        "AGP 2x",
        "AGP 4x",
        "PCI-X",
        "AGP 8x" /* 0x13 */
    };
    static const char *type_0xA0[]= {
        "PC-98/C20", /* 0xA0 */
        "PC-98/C24",
        "PC-98/E",
        "PC-98/Local Bus",
        "PC-98/Card",
        "PCI Express" /* 0xA5 */
    };

    if (code>=0x01 && code<=0x13)
        sprintf(out, "%s ", type[code]);
    else if (code>=0xA0 && code<=0xA5)
        sprintf(out, "%s ", type_0xA0[code-0xA0]);
    else
        sprintf(out, " ");
    return 0;
}

int getSlotBusWidth(unsigned char code, unsigned char* out)
{
    /* 3.3.10.2 */
    static const char *width[]= {
        "", /* 0x01, "Other" */
        "", /* "Unknown" */
        "8-bit ",
        "16-bit ",
        "32-bit ",
        "64-bit ",
        "128-bit ",
        "x1 ",
        "x2 ",
        "x4 ",
        "x8 ",
        "x12 ",
        "x16 ",
        "x32 " /* 0x0E */
    };

    if (code>=0x01 && code<=0x0E)
        sprintf(out, "%s ",width[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getSlotCurrentUsage(unsigned char code, unsigned char* out)
{
    /* 3.3.10.3 */
    static const char *usage[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Available",
        "In Use" /* 0x04 */
    };

    if (code>=0x01 && code<=0x04)
        sprintf(out, "%s ",usage[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getSlotLength(unsigned char code, unsigned char* out)
{
    /* 3.3.1O.4 */
    static const char *length[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Short",
        "Long" /* 0x04 */
    };
    if (code>=0x01 && code<=0x04)
        sprintf(out, "%s ",length[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getSlotID(unsigned char code1, unsigned char code2, unsigned char type, unsigned char* out)
{
    /* 3.3.10.5 */
    switch (type)
    {
    case 0x04: /* MCA */
        sprintf(out, "%u ", code1);
        break;
    case 0x05: /* EISA */
        sprintf(out, "%u ", code1);
        break;
    case 0x06: /* PCI */
    case 0x0E: /* PCI */
    case 0x0F: /* AGP */
    case 0x10: /* AGP */
    case 0x11: /* AGP */
    case 0x12: /* PCI-X */
    case 0x13: /* AGP */
    case 0xA5: /* PCI Express */
        sprintf(out, "%u ", code1);
        break;
    case 0x07: /* PCMCIA */
        sprintf(out, "%u %u ", code1, code2);
        break;
    default:
        sprintf(out, " ");
    }
    return 0;
}

int getSlotCharacters(unsigned char code1, unsigned char code2, unsigned char* out)
{
    int rs = 0;
    /* 3.3.10.6 */
    static const char *characteristics1[]= {
        "5.0 V is provided", /* 1 */
        "3.3 V is provided",
        "Opening is shared",
        "PC Card-16 is supported",
        "Cardbus is supported",
        "Zoom Video is supported",
        "Modem ring resume is supported" /* 7 */
    };
    /* 3.3.10.7 */
    static const char *characteristics2[]= {
        "PME signal is supported", /* 0 */
        "Hot-plug devices are supported",
        "SMBus signal is supported" /* 2 */
    };

    if (code1&(1<<0))
        sprintf(out," Unknown ");
    else if ((code1&0xFE)==0 && (code2&0x07)==0)
        sprintf(out, " None ");
    else
    {
        int i;
        for (i=1; i<=7; i++)
        {
            if (code1&(1<<i))
            {
                rs = sprintf(out, "%s ",characteristics1[i-1]);
                out = out + rs;
            }
        }

        for (i=0; i<=2; i++)
        {
            if (code2&(1<<i))
            {
                rs = sprintf(out, "%s ", characteristics2[i]);
                out = out + rs;
            }
        }
        sprintf(out, " ");
    }

    return 0;
}

int getSlotElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    /*SLOT_SEG_GN = 0x0D,
    SLOT_BUS_NUM = 0x0F,
    	SLOT_DEV_NUM = 0x10*/

    switch (e)
    {
    case SLOT_DESIGN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case SLOT_TYPE:
        getSlotType(p[e], out);
        break;
    case SLOT_BUS_WIDTH:
        getSlotBusWidth(p[e], out);
        break;
    case SLOT_USAGE:
        getSlotCurrentUsage(p[e], out);
        break;
    case SLOT_LEN:
        getSlotLength(p[e], out);
        break;
    case SLOT_ID:
        getSlotID(p[SLOT_ID], p[SLOT_ID + 1], p[SLOT_TYPE], out);
        break;
    case SLOT_CHAR1:
        getSlotCharacters(p[e], 0x00, out);
        break;
    case SLOT_CHAR2:
        getSlotCharacters(p[SLOT_CHAR1], p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getOnBoardDevType(unsigned char code, unsigned char* out)
{
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Video",
        "SCSI Controller",
        "Ethernet",
        "Token Ring",
        "Sound",
        "PATA Controller",
        "SATA Controller",
        "SAS Controller" /* 0x0A */
    };

    if (code>=0x01 && code<=0x0A)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out," ");
    return 0;
}

int getOEMStringElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case OEMSTR_COUNT:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;

}

int getSystemCfgOptElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }

    switch (e)
    {
    case SYSCFGOPT_COUNT:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getBIOSLanguageElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case BIOSLANG_INSTALL_LANG:
    case BIOSLANG_FLAGS:
        getBIOSElemnetByte(p, e, out);
        break;
    case BIOSLANG_CURRENT_LANG:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;

}

int getGroupAssociationElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case GA_ITEM_TYPE:
        getBIOSElemnetByte(p, e, out);
        break;
    case GA_NAME:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case GA_ITEM_HANDLE:
        getBIOSElemnetWord(p, e, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}



int getSysEventLogMethod(unsigned char code, unsigned char* out)
{
    static const char *method[]= {
        "Indexed I/O, one 8-bit index port, one 8-bit data port", /* 0x00 */
        "Indexed I/O, two 8-bit index ports, one 8-bit data port",
        "Indexed I/O, one 16-bit index port, one 8-bit data port",
        "Memory-mapped physical 32-bit address",
        "General-purpose non-volatile data functions" /* 0x04 */
    };

    if (code<=0x04)
        sprintf(out, "%s ",method[code]);
    else if (code>=0x80)
        sprintf(out, "%s ","OEM-specific");
    else
        sprintf(out, " ");

    return 0;
}

int getSysEventLogStatus(unsigned char code, unsigned char* out)
{
    static const char *valid[]= {
        "Invalid", /* 0 */
        "Valid" /* 1 */
    };
    static const char *full[]= {
        "Not Full", /* 0 */
        "Full" /* 1 */
    };

    sprintf(out," %s, %s ",valid[(code>>0)&1], full[(code>>1)&1]);
    return 0;
}

int getSysEventLogAddress(unsigned char*p, unsigned char* out)
{
    unsigned char method = p[SYSEVTLOG_ACCESS_METHOD];
    unsigned char tmp[8] = {0};
    unsigned int us = 0;
    int rs = 0;
    /* 3.3.16.3 */
    switch (method)
    {
    case 0x00:
    case 0x01:
    case 0x02:
        getBIOSElemnetWord(p, SYSEVTLOG_METHOD_ADDRESS, tmp);
        sscanf(tmp, "%u", &us);
        rs = sprintf(out, "Index 0x%04X, ", us);
        getBIOSElemnetWord(p, SYSEVTLOG_METHOD_ADDRESS, tmp);
        sscanf(tmp, "%u", &us);
        out = out + rs;
        sprintf(out, "Data 0x%04X ", us);
        break;
    case 0x03:
        getBIOSElemnetDWord(p, SYSEVTLOG_METHOD_ADDRESS, out);
        break;
    case 0x04:
        getBIOSElemnetWord(p, SYSEVTLOG_METHOD_ADDRESS, out);
        break;
    default:
        sprintf(out, " Unknown ");
    }
    return 0;
}

int getSyseventLogHeaderFormat(unsigned char code, unsigned char* out)
{
    static const char *type[]= {
        "No Header", /* 0x00 */
        "Type 1" /* 0x01 */
    };

    if (code<=0x01)
        sprintf(out, "%s ", type[code]);
    else if (code>=0x80)
        sprintf(out," %s ","OEM-specific");
    else
        sprintf(out," ");
    return 0;
}

int getSysEventLogDescriptorType(unsigned char code, unsigned char* out)
{
    int rs = 0;
    /* 3.3.16.6.1 */
    static const char *type[]= {
        0, /* 0x00 */
        "Single-bit ECC memory error",
        "Multi-bit ECC memory error",
        "Parity memory error",
        "Bus timeout",
        "I/O channel block",
        "Software NMI",
        "POST memory resize",
        "POST error",
        "PCI parity error",
        "PCI system error",
        "CPU failure",
        "EISA failsafe timer timeout",
        "Correctable memory log disabled",
        "Logging disabled",
        0, /* 0x0F */
        "System limit exceeded",
        "Asynchronous hardware timer expired",
        "System configuration information",
        "Hard disk information",
        "System reconfigured",
        "Uncorrectable CPU-complex error",
        "Log area reset/cleared",
        "System boot" /* 0x17 */
    };

    if (code<=0x17 && type[code]!=0)
        rs = sprintf(out, "%s ",type[code]);
    else if (code>=0x80 && code<=0xFE)
        rs = sprintf(out, "%s ","OEM-specific");
    else if (code==0xFF)
        rs = sprintf(out, "%s ","End of log");
    else sprintf(out, " ");
    return rs;
}

int getSysEventLogDescriptorFormat(unsigned char code, unsigned char* out)
{
    /* 3.3.16.6.2 */
    static const char *format[]= {
        "None", /* 0x00 */
        "Handle",
        "Multiple-event",
        "Multiple-event handle",
        "POST results bitmap",
        "System management",
        "Multiple-event system management" /* 0x06 */
    };
    int rs = 0;

    if (code<=0x06)
        rs = sprintf(out, "%s ", format[code]);
    else if (code>=0x80)
        rs = sprintf(out, "%s ", "OEM-specific");
    else
        sprintf(out, " ");
    return rs;
}

int getSysEventLogDescriptor(unsigned char count, unsigned char len,
                             unsigned char*p, unsigned char* out)
{
    static int i = 0;
    int rs = 0;

    for (; i<count; )
    {
        i++;
        if (len>=0x02)
        {
            rs = getSysEventLogDescriptorType(p[(i-1)*len], out);
            out = out + rs;
            getSysEventLogDescriptorFormat(p[(i-1)*len+1], out);
            return 1;
        }
    }
    i = 0;
    return 0;
}

int getSysEventLogElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case SYSEVTLOG_LEN:
    case SYSEVTLOG_HEADER_OFFSET:
    case SYSEVTLOG_DATA_OFFSET:
    case SYSEVTLOG_TOKEN:
        getBIOSElemnetWord(p, e, out);
        break;
    case SYSEVTLOG_ACCESS_METHOD:
        getSysEventLogMethod(p[e], out);
        break;
    case SYSEVTLOG_STATUS:
        getSysEventLogStatus(p[e], out);
        break;
    case SYSEVTLOG_METHOD_ADDRESS:
        getSysEventLogAddress(p, out);
        break;
    case SYSEVTLOG_HEADER_FORMAT:
        getSyseventLogHeaderFormat(p[e], out);
    default:
        sprintf(out," ");
    }
    return 0;
}


int getMemoryArrayLocation(unsigned char code, unsigned char* out)
{
    /* 3.3.17.1 */
    static const char *location[]= {
        "Other", /* 0x01 */
        "Unknown",
        "System Board Or Motherboard",
        "ISA Add-on Card",
        "EISA Add-on Card",
        "PCI Add-on Card",
        "MCA Add-on Card",
        "PCMCIA Add-on Card",
        "Proprietary Add-on Card",
        "NuBus" /* 0x0A, master.mif says 16 */
    };
    static const char *location_0xA0[]= {
        "PC-98/C20 Add-on Card", /* 0xA0 */
        "PC-98/C24 Add-on Card",
        "PC-98/E Add-on Card",
        "PC-98/Local Bus Add-on Card",
        "PC-98/Card Slot Add-on Card" /* 0xA4, from master.mif */
    };

    if (code>=0x01 && code<=0x0A)
        sprintf(out, "%s ", location[code-0x01]);
    else if (code>=0xA0 && code<=0xA4)
        sprintf(out, "%s ", location_0xA0[code-0xA0]);
    else
        sprintf(out, " ");
    return 0;
}

int getMemoryArrayUse(unsigned char code, unsigned char* out)
{
    /* 3.3.17.2 */
    static const char *use[]= {
        "Other", /* 0x01 */
        "Unknown",
        "System Memory",
        "Video Memory",
        "Flash Memory",
        "Non-volatile RAM",
        "Cache Memory" /* 0x07 */
    };

    if (code>=0x01 && code<=0x07)
        sprintf(out, "%s ",use[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getMemoryArrayErrorCorrection(unsigned char code, unsigned char* out)
{
    /* 3.3.17.3 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "None",
        "Parity",
        "Single-bit ECC",
        "Multi-bit ECC",
        "CRC" /* 0x07 */
    };

    if (code>=0x01 && code<=0x07)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getPhyMemArrayElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case PHYMEMARY_DEV_NUM:
        getBIOSElemnetWord(p, e, out);
        break;
    case PHYMEMARY_MAX_CAP:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0x80000000)
            sprintf(out,"%d", 0);
        break;
    case PHYMEMARY_ERR_INFO_HANDLE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui==0xFFFE)
            sprintf(out, "Not Provided ");
        else if (ui==0xFFFF)
            sprintf(out, "No Error ");
        else
            sprintf(out, "0x%04X ", ui);
        break;
    case PHYMEMARY_LOCATION:
        getMemoryArrayLocation(p[e], out);
        break;
    case PHYMEMARY_USE:
        getMemoryArrayUse(p[e], out);
        break;
    case PHYMEMARY_ERR_CORRECT:
        getMemoryArrayErrorCorrection(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getMemDevFormFactor(unsigned char code, unsigned char* out)
{
    /* 3.3.18.1 */
    static const char *form_factor[]= {
        "Other", /* 0x01 */
        "Unknown",
        "SIMM",
        "SIP",
        "Chip",
        "DIP",
        "ZIP",
        "Proprietary Card",
        "DIMM",
        "TSOP",
        "Row Of Chips",
        "RIMM",
        "SODIMM",
        "SRIMM",
        "FB-DIMM" /* 0x0F */
    };

    if (code>=0x01 && code<=0x0F)
        sprintf(out, "%s ", form_factor[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getMemDevType(unsigned char code, unsigned char* out)
{
    /* 3.3.18.2 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "DRAM",
        "EDRAM",
        "VRAM",
        "SRAM",
        "RAM",
        "ROM",
        "Flash",
        "EEPROM",
        "FEPROM",
        "EPROM",
        "CDRAM",
        "3DRAM",
        "SDRAM",
        "SGRAM",
        "RDRAM",
        "DDR",
        "DDR2",
        "DDR2 FB-DIMM" /* 0x14 */
    };

    if (code>=0x01 && code<=0x14)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getMemDevDetail(unsigned short code, unsigned char* out)
{
    /* 3.3.18.3 */
    static const char *detail[]= {
        "Other", /* 1 */
        "Unknown",
        "Fast-paged",
        "Static Column",
        "Pseudo-static",
        "RAMBus",
        "Synchronous",
        "CMOS",
        "EDO",
        "Window DRAM",
        "Cache DRAM",
        "Non-Volatile" /* 12 */
    };

    int rs = 0;

    if ((code&0x1FFE)==0)
        sprintf(out,"None ");
    else
    {
        int i;
        for (i=1; i<=12; i++)
        {
            if (code&(1<<i))
            {
                rs = sprintf(out,"%s", detail[i-1]);
                out = out + rs;
            }
        }
        sprintf(out," ");
    }
    return 0;
}

int getMemDeviceElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    /*
    case MEMDEV_ATTRIBUTE
    	*/
    unsigned int ui = 0;
    switch (e)
    {
    case MEMDEV_LOCATION:
    case MEMDEV_SPEED:
        getBIOSElemnetWord(p, e, out);
        break;
    case MEMDEV_DEV_LOCATOR:
    case MEMDEV_BANK_LOCATOR:
    case MEMDEV_MAN:
    case MEMDEV_SN:
    case MEMDEV_TAG:
    case MEMDEV_PN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case MEMDEV_ERR_INFO_HANDLE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u ", &ui);
        if (ui==0xFFFE)
            sprintf(out, "Not Provided ");
        else if (ui==0xFFFF)
            sprintf(out, "No Error ");
        else
            sprintf(out, "0x%04X ", ui);
        break;
    case MEMDEV_WIDTH:
    case MEMDEV_DATA_WIDTH:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui==0xFFFF || ui == 0x0)
            sprintf(out, "0");
        break;
    case MEMDEV_SIZE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0)
            sprintf(out, "No Module Installed ");
        else if (ui == 0xFFFF)
            sprintf(out, "Unknown ");
        else
        {
            if (ui&0x8000)
                sprintf(out,"%u kB ", ui&0x7FFF);
            else
                sprintf(out,"%u MB ", ui);
        }
        break;
    case MEMDEV_FACTOR:
        getMemDevFormFactor(p[e], out);
        break;
    case MEMDEV_SET:
        if (p[e]==0)
            sprintf(out, "None ");
        else if (p[e] == 0xFF)
            sprintf(out, "Unknown ");
        else
            sprintf(out, "%u", p[e]);
        break;

    case MEMDEV_TYPE:
        getMemDevType(p[e], out);
        break;
    case MEMDEV_DEV_DETAIL:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u ", &ui);
        getMemDevDetail((unsigned short)ui, out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getMemoryErrorInfoType(unsigned char code, unsigned char* out)
{
    /* 3.3.19.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "OK",
        "Bad Read",
        "Parity Error",
        "Single-bit Error",
        "Double-bit Error",
        "Multi-bit Error",
        "Nibble Error",
        "Checksum Error",
        "CRC Error",
        "Corrected Single-bit Error",
        "Corrected Error",
        "Uncorrectable Error" /* 0x0E */
    };

    if (code>=0x01 && code<=0x0E)
        sprintf(out,"%s ", type[code-0x01]);
    else
        sprintf(out," ");
    return 0;
}

int getMemoryErrorInfoGranularity(unsigned char code, unsigned char* out)
{
    /* 3.3.19.2 */
    static const char *granularity[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Device Level",
        "Memory Partition Level" /* 0x04 */
    };

    if (code>=0x01 && code<=0x04)
        sprintf(out,"%s ", granularity[code-0x01]);
    else
        sprintf(out," ");
    return 0;
}

int getMemoryErrorInfoOperation(unsigned char code, unsigned char* out)
{
    /* 3.3.19.3 */
    static const char *operation[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Read",
        "Write",
        "Partial Write" /* 0x05 */
    };

    if (code>=0x01 && code<=0x05)
        sprintf(out,"%s ",operation[code-0x01]);
    else
        sprintf(out," ");
    return 0;
}

int getMemErrorInfoElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case MEMEINFO_VENDOR_SYNDROME:
        getBIOSElemnetWord(p, e, out);
        break;
    case MEMEINFO_TYPE:
        getMemoryErrorInfoType(p[e], out);
        break;
    case MEMEINFO_GRANULARITY:
        getMemoryErrorInfoGranularity(p[e], out);
        break;
    case MEMEINFO_OP:
        getMemoryErrorInfoOperation(p[e], out);
        break;
    case MEMEINFO_ARR_ERROR_ADDRESS:
    case MEMEINFO_DEV_ERROR_ADDRESS:
    case MEMEINFO_ERROR_RES:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui==0x80000000)
            sprintf(out, "Unknown ");
        break;
    default:
        sprintf(out," ");
    }

    return 0;
}

int getMemArpMapAddressElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case MEMARYMAP_PWIDTH:
        getBIOSElemnetByte(p, e, out);
        break;
    case MEMARYMAP_HANDLE:
        getBIOSElemnetWord(p, e, out);
        break;
    case MEMARYMAP_START_ADR:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        sprintf(out, "0x%08X%03X ", ui>>2, (ui&0x03)<<10);
        break;
    case MEMARYMAP_END_ADR:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        sprintf(out, "0x%08X%03X ", ui>>2, ((ui&0x03)<<10) + 0x3FF);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}


int getMemDevMapAddressElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case MEMDEVMAP_HANDLE:
    case MEMDEVMAP_ARY_HANDLE:
        getBIOSElemnetWord(p, e, out);
        break;
    case MEMDEVMAP_START_ADR:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        sprintf(out, "0x%08X%03X ", ui>>2, (ui&0x03)<<10);
        break;
    case MEMDEVMAP_END_ADR:
        getBIOSElemnetDWord(p, e, out);
        sscanf(out, "%u", &ui);
        /*???????????*/
        sprintf(out, "0x%08X%03X ", ui>>2, ((ui&0x03)<<10) + 0x3FF);
        break;
    case MEMDEVMAP_ROW_POS:
    case MEMDEVMAP_INTERLEAVE_POS:
    case MEMDEVMAP_INTERLEAVE_DEPTH:
        if (p[e]== 0xFF)
            sprintf(out, " ");
        else
            sprintf(out, "%d ",p[e]);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getBuiltInPointingDevType(unsigned char code, unsigned char* out)
{
    /* 3.3.22.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Mouse",
        "Track Ball",
        "Track Point",
        "Glide Point",
        "Touch Pad",
        "Touch Screen",
        "Optical Sensor" /* 0x09 */
    };

    if (code>=0x01 && code<=0x09)
        sprintf(out, "%s ", type[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getBuiltInPointingDevIF(unsigned char code, unsigned char* out)
{
    /* 3.3.22.2 */
    static const char *interface[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Serial",
        "PS/2",
        "Infrared",
        "HIP-HIL",
        "Bus Mouse",
        "ADB (Apple Desktop Bus)" /* 0x08 */
    };
    static const char *interface_0xA0[]= {
        "Bus Mouse DB-9", /* 0xA0 */
        "Bus Mouse Micro DIN",
        "USB" /* 0xA2 */
    };

    if (code>=0x01 && code<=0x08)
        sprintf(out, "%s ", interface[code-0x01]);
    else if (code>=0xA0 && code<=0xA2)
        sprintf(out, "%s ", interface_0xA0[code-0xA0]);
    else
        sprintf(out, " ");
    return 0;
}

int getBuiltInPointDevElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case BPTDEV_BTN_NUM:
        getBIOSElemnetByte(p, e, out);
        break;
    case BPTDEV_TYPE:
        getBuiltInPointingDevType(p[e], out);
        break;
    case BPTDEV_IF:
        getBuiltInPointingDevIF(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getBatteryChemistry(unsigned char code, unsigned char* out)
{
    /* 3.3.23.1 */
    static const char *chemistry[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Lead Acid",
        "Nickel Cadmium",
        "Nickel Metal Hydride",
        "Lithium Ion",
        "Zinc Air",
        "Lithium Polymer" /* 0x08 */
    };

    if (code>=0x01 && code<=0x08)
        sprintf(out, "%s ", chemistry[code-0x01]);
    else
        sprintf(out, " ");
    return 0;
}

int getBatteryElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case BATTERY_DEGIGN_VOLTAGE:
    case BATTERY_SBDS_SN:
    case BATTERY_OEM_SPEC:
        getBIOSElemnetWord(p, e, out);
        break;
    case BATTERY_LOC:
    case BATTERY_MAN:
    case BATTERY_DATE:
    case BATTERY_SN:
    case BATTERY_DN:
    case BATTERY_SBDS_VER:
    case BATTERY_SBDS_DEV_CHEMISTRY:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case BATTERY_DEV_CHEMISTRY:
        getBatteryChemistry(p[e], out);
        break;
    case BATTERY_DESIGN_CAP:
    case BATTERY_CAP_MUL:
        if (e == BATTERY_DESIGN_CAP)
            getBIOSElemnetWord(p, e, out);
        else
            getBIOSElemnetWord(p, BATTERY_DESIGN_CAP, out);

        if (!(sh->len <= BATTERY_CAP_MUL))
        {
            sscanf(out, "%u", &ui);
            sprintf(out, "%d ", ui*(p[BATTERY_CAP_MUL]));
        }
        break;
    case BATTERY_MAX_ERR:
        getBIOSElemnetByte(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0xFF)
            sprintf(out, "Unknown ");
        break;
    case BATTERY_SBDS_DATE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        sprintf(out, "%u-%02u-%02u ", 1980+(ui>>9), (ui>>5)&0x0F, ui&0x1F);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getSysResetCap(unsigned char code, unsigned char* out)
{
    int rs = 0;

    static const char *option[]= {
        "Operating System", /* 0x1 */
        "System Utilities",
        "Do Not Reboot" /* 0x3 */
    };

    rs = sprintf(out, "Status<%s> ",(code&(1<<0))?"Enabled":"Disabled");
    out = out + rs;
    rs = sprintf(out, "Watchdog Timer:<%s> ",(code&(1<<5))?"Present":"Not Present");
    out = out + rs;
    rs = sprintf(out, "Boot Option:<%s> ",((code>>1)&0x03)?option[((code>>1)&0x03)-1]:" ");
    out = out + rs;
    rs = sprintf(out, "Boot Option On Limit:<%s> ",((code>>3)&0x03)?option[((code>>1)&0x03)-1]:" ");
    out = out + rs;
    sprintf(out," ");
    return 0;
}
int getSysRestElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case SYSRES_CAP:
        getSysResetCap(p[e], out);
        break;
    case SYSRES_CONUT:
    case SYSRES_LIMIT:
    case SYSRES_INTERVAL:
    case SYSRES_TIMEOUT:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0xFFFF)
            sprintf(out,"Unknown ");
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getHWSecuritySetting(unsigned char code, unsigned char* out)
{
    unsigned char tmp = 0;
    int rs = 0;
    static const char *status[]= {
        "Disabled", /* 0x00 */
        "Enabled",
        "Not Implemented",
        "Unknown" /* 0x03 */
    };

    tmp = code>>6;
    rs = sprintf(out, "Power-On Password Status<%s> ", status[tmp]);
    out = out + rs;
    tmp = (code>>4)&0x3;
    rs = sprintf(out, "Keyboard Password Status<%s> ", status[tmp]);
    out = out + rs;
    tmp = (code>>2)&0x3;
    rs = sprintf(out, "Administrator Password Status<%s> ", status[tmp]);
    out = out + rs;
    tmp = code&0x3;
    rs = sprintf(out, "Front Panel Reset Status<%s> ", status[tmp]);
    return 0;
}

int getHWSecurityElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case HWSEC_SETTING:
        getHWSecuritySetting(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int chkBCDRange(unsigned char code,unsigned char l,unsigned char h)
{
    if (code>0x99 || (code&0x0F)>0x09)
        return 0;
    if (code<l || code>h)
        return 0;
    return 1;
}

int getSysPowerControlElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    int r = 0;

    switch (e)
    {
    case SYSPWRCTL_MONTH:
        r = chkBCDRange(p[e], 0x01, 0x12);
        break;
    case SYSPWRCTL_DAY:
        r = chkBCDRange(p[e], 0x01, 0x31);
        break;
    case SYSPWRCTL_HOUR:
        r = chkBCDRange(p[e], 0x00, 0x23);
        break;
    case SYSPWRCTL_MIN:
    case SYSPWRCTL_SEC:
        r = chkBCDRange(p[e], 0x00, 0x59);
        break;
    default:
        sprintf(out," ");
    }

    if (r)
        sprintf(out,"%d ", p[e]);
    else
        sprintf(out," ");
    return 0;
}

int getVoltageProbeLocationStatus(unsigned char code, unsigned char* out)
{
    unsigned char tmp = 0;
    int rs = 0;
    static const char *location[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Processor",
        "Disk",
        "Peripheral Bay",
        "System Management Module",
        "Motherboard",
        "Memory Module",
        "Processor Module",
        "Power Unit",
        "Add-in Card" /* 0x0B */
    };

    static const char *status[]= {
        "Other", /* 0x01 */
        "Unknown",
        "OK",
        "Non-critical",
        "Critical",
        "Non-recoverable" /* 0x06 */
    };
    tmp = code & 0x1f;
    if (tmp>=0x01 && tmp<=0x0B)
        rs = sprintf(out,"Location<%s> " ,location[tmp-0x01]);

    out = out + rs;

    tmp = code >>5;
    if (tmp>=0x01 && tmp<=0x06)
        rs = sprintf(out,"Status<%s>" ,status[tmp-0x01]);

    out = out + rs;
    sprintf(out," ");
    return 0;
}
int getVoltageProbeElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui;

    switch (e)
    {
    case VOLPB_OEM_DEFINED:
        getBIOSElemnetDWord(p, e, out);
        break;
    case VOLPB_DESCRIPTION:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case VOLPB_LOC_STATUS:
        getVoltageProbeLocationStatus(p[e], out);
        break;
    case VOLPB_MAX:
    case VOLPB_MIN:
    case VOLPB_RES:
    case VOLPB_TOLERANCE:
    case VOLPB_ACCURACY:
    case VOLPB_NOMINAL:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0x8000)
            sprintf(out, "Unknown ");
        else if (e == VOLPB_RES)
            sprintf(out, "%d.%d ", ui/10, ui-((ui/10)*10));
        else if (e == VOLPB_ACCURACY)
            sprintf(out, "%d.%d ", ui/100, ui-((ui/100)*100));
        else
            sprintf(out, "%d.%d ", ui/1000, ui-((ui/1000)*1000));
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getCoolingDevTypeStatus(unsigned char code, unsigned char* out)
{
    unsigned char tmp = 0;
    int rs = 0;
    /* 3.3.28.1 */
    static const char *type[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Fan",
        "Centrifugal Blower",
        "Chip Fan",
        "Cabinet Fan",
        "Power Supply Fan",
        "Heat Pipe",
        "Integrated Refrigeration" /* 0x09 */
    };
    static const char *type_0x10[]= {
        "Active Cooling", /* 0x10, master.mif says 32 */
        "Passive Cooling" /* 0x11, master.mif says 33 */
    };

    static const char *status[]= {
        "Other", /* 0x01 */
        "Unknown",
        "OK",
        "Non-critical",
        "Critical",
        "Non-recoverable" /* 0x06 */
    };


    tmp = code & 0x1F;
    if (tmp >= 0x01 && tmp <=0x09)
        rs = sprintf(out, "Type<%s> ", type[tmp-0x01]);
    else if (tmp >= 0x10 && tmp <= 0x11)
        rs = sprintf(out, "Type<%s> ", type_0x10[tmp-0x10]);

    out = out + rs;
    tmp = code >> 5;

    if (tmp >= 0x01 && tmp <= 0x06)
        rs = sprintf(out, "Status<%s> ", status[tmp - 0x01]);

    out = out + rs;
    sprintf(out, " ");
    return 0;
}

int getCoolingDevElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui;

    switch (e)
    {
    case COOLDEV_UINT:
        getBIOSElemnetByte(p, e, out);
        break;
    case COOLDEV_NOMINAL_SPEED:
        getBIOSElemnetWord(p, e, out);
        break;
    case COOLDEV_OEM_DEF:
        getBIOSElemnetDWord(p, e, out);
        break;
    case COOLDEV_TEMP_PBOBE:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0xFFFF)
            sprintf(out, " ");
        break;
    case COOLDEV_TYPE_STATUS:
        getCoolingDevTypeStatus(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getTemperatureProbeLocationStatus(unsigned char code, unsigned char* out)
{
    int rs = 0;
    unsigned char tmp;
    /* 3.3.29.1 */
    static const char *location[]= {
        "Other", /* 0x01 */
        "Unknown",
        "Processor",
        "Disk",
        "Peripheral Bay",
        "System Management Module", /* master.mif says SMB Master */
        "Motherboard",
        "Memory Module",
        "Processor Module",
        "Power Unit",
        "Add-in Card",
        "Front Panel Board",
        "Back Panel Board",
        "Power System Board",
        "Drive Back Plane" /* 0x0F */
    };
    /* 3.3.27.1 */
    static const char *status[]= {
        "Other", /* 0x01 */
        "Unknown",
        "OK",
        "Non-critical",
        "Critical",
        "Non-recoverable" /* 0x06 */
    };

    tmp = code&0x1F;
    if (tmp >= 0x01 && tmp <= 0x0F)
        rs = sprintf(out, "<%s>", location[tmp-0x01]);
    else
        rs = sprintf(out, " ");

    out = out + rs;
    tmp = code >> 5;

    if (tmp>= 0x01 && tmp <= 0x06)
        sprintf(out," <%s> ", status[tmp-0x01]);
    else
        sprintf(out, " ");

    return 0;
}

int getTempatureProbeElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case TMPPB_OEM_DEFINED:
        getBIOSElemnetDWord(p, e, out);
        break;
    case TMPPB_DESCRIPTION:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case TMPPB_LOC_STATUS:
        getTemperatureProbeLocationStatus(p[e], out);
        break;
    case TMPPB_MAX:
    case TMPPB_MIN:
    case TMPPB_RES:
    case TMPPB_TOLERANCE:
    case TMPPB_ACCURACY:
    case TMPPB_NOMINAL:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0x8000)
            sprintf(out, "Unknown ");
        else if (e == TMPPB_NOMINAL)
            sprintf(out, "%d.%d ", ui/10, ui-((ui/10)*10));
        else if (e == TMPPB_ACCURACY)
            sprintf(out, "%d.%d ", ui/100, ui-((ui/100)*100));
        else
            sprintf(out, "%d.%d ", ui/1000, ui-((ui/1000)*1000));
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getElectricalCurrentProbeElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    unsigned int ui = 0;
    switch (e)
    {
    case ECPB_OEM_DEFINED:
        getBIOSElemnetDWord(p, e, out);
        break;
    case ECPB_DESCRIPTION:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case ECPB_LOC_STATUS:
        getVoltageProbeLocationStatus(p[e], out);
        break;
    case ECPB_MAX:
    case ECPB_MIN:
    case ECPB_RES:
    case ECPB_TOLERANCE:
    case ECPB_ACCURACY:
    case ECPB_NOMINAL:
        getBIOSElemnetWord(p, e, out);
        sscanf(out, "%u", &ui);
        if (ui == 0x8000)
            sprintf(out, "Unknown ");
        else if (e == ECPB_RES )
            sprintf(out, "%d.%d ", ui/10, ui-((ui/10)*10));
        else if (e == ECPB_ACCURACY)
            sprintf(out, "%d.%d ", ui/100, ui-((ui/100)*100));
        else
            sprintf(out, "%d.%d ", ui/1000, ui-((ui/1000)*1000));
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getOOBRemoteAccessElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case OOBRA_MAN:
        getBIOSElemnetString_swpatch(p, e, out);
        break;
    case OOBRA_CONN:
        sprintf(out, "Inbound Connection<%s> Outbound Connection<%s> ",
                p[e]&(1<<0)?"Enabled":"Disabled",p[e]&(1<<1)?"Enabled":"Disabled");
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}

int getSystemBootStatus(unsigned char code, unsigned char* out)
{
    static const char *status[]= {
        "No errors detected", /* 0 */
        "No bootable media",
        "The \"normal\" operating system failed to load",
        "Firmware-detected hardware failure, including \"unknown\" failure types",
        "Operating system-detected hardware failure",
        "User-requested boot",
        "System security violation",
        "Previously-requested image",
        "System watchdog timer expired" /* 8 */
    };

    if (code<=8)
        sprintf(out, "%s ", status[code]);
    else if (code>=128 && code<=191)
        sprintf(out, "OEM-specific ");
    else if (code>=192)
        sprintf(out, "Product-specific ");
    else
        sprintf(out,"Unknown ");
    return 0;
}


int getSysBootStatusElemnetData(unsigned char *p,SMBIOSHeader *sh, int e,unsigned char* out)
{
    switch (e)
    {
    case SYSBOOTSTATUS_STATUS:
        getSystemBootStatus(p[e], out);
        break;
    default:
        sprintf(out," ");
    }
    return 0;
}



int getSMBIOSElement(unsigned char *p,SMBIOSHeader *sh, enum SMBIOSType type, int e,unsigned char* out)
{
    static int(*SMBIOSfun[])(unsigned char*, SMBIOSHeader*, int e, unsigned char* )
    = {
        getBIOSElemnetData_swpatch, getSystemInfoElemnetData,getBaseBoardInfoElemnetData,
        getChassisElemnetData, getProcessorElemnetData, getMemoryControlElemnetData,
        getMemoryModuleElemnetData, getCacheElemnetData, getPConnectorElemnetData,/*8*/
        getSlotElemnetData,0 /*on board dev = 10*/, getOEMStringElemnetData,
        getSystemCfgOptElemnetData, getBIOSLanguageElemnetData, getGroupAssociationElemnetData,
        getSysEventLogElemnetData, getPhyMemArrayElemnetData, getMemDeviceElemnetData,/*17*/
        getMemErrorInfoElemnetData, getMemArpMapAddressElemnetData, getMemDevMapAddressElemnetData,
        getBuiltInPointDevElemnetData, getBatteryElemnetData, getSysRestElemnetData,
        getHWSecurityElemnetData, getSysPowerControlElemnetData, getVoltageProbeElemnetData,/*26*/
        getCoolingDevElemnetData, getTempatureProbeElemnetData, getElectricalCurrentProbeElemnetData,
        getOOBRemoteAccessElemnetData,0 /*BIS = 31*/, getSysBootStatusElemnetData
    };

    if (sh->len <= e)
    {
        sprintf(out," ");
        return 0;
    }
    if (type < 0 || type == 31 || type == 10 || type > 32)
    {
        sprintf(out," ");
        return 0;
    }
    else
        return (*SMBIOSfun[type])(p, sh, e, out);

    return 0;
}
