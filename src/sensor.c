#include <stdio.h>
#include <string.h>
#include "sensor.h"
#include "bsp.h"
#include "lib.h"

extern INT8U PLDMSNR[15];
extern DPCONF *dpconf;
extern asf_config_t *asfconfig;
extern sensor_t sensor[MAX_SENSOR_NUMS];

static INT8U *pldmsnrsendbuf = NULL;

extern snr_name_tbl_t snr_name_tbl[16];

static void GetASFSensor(INT32S i)
{
    INT8U index, j = 0;

    for(index= 0 ; index < asfconfig->numofsnr; index++)
    {
        i++;
        sensor[i].pollindex=0;
        sensor[i].exist=SNR_READ;
        sensor[i].event=0;
        sensor[i].fault=5;
        sensor[i].value=0;
        sensor[i].state=0;
        sensor[i].prestate=0;
        sensor[i].index=i;

        //estabilish a link
        sensor[i].offset[0]= index;
        asfconfig->legacysnr[index].sensorindex = i;

#if defined(CONFIG_CHIPSET_AMDSOC)
        for(j = 0 ; j < 16; j++)
        {
            if(asfconfig->legacysnr[index].addr == snr_name_tbl[j].address)
                strcpy(sensor[i].name, snr_name_tbl[j].name);

        }
        sensor[i].type = asfconfig->legacysnr[index].type ;
#else
        strcpy(sensor[i].name, "Legacy Sensor");
        sensor[i].type = 0x01 ;

#endif
        dpconf->numofsnr++;
    }


}

int ParserSensor()
{
    unsigned char *addr=NULL,*addr1=NULL,*curLoc,*curLoc1;
    unsigned short component,threshold,tmp;
    int ret,i=-1;
    int cflag = 0, sflag = 0;

    dpconf->numofsnr = 0;

    addr  = getSMBIOSTypeAddr(0x99);
    addr1 = getSMBIOSTypeAddr(0xff);

    //DEBUGMSG(PLDM_DEBUG, "Entering ParserSensor [%s:%d]: %s\n", __FILE__,__LINE__,__func__);

    if(addr1)
    {
        DEBUGMSG(PLDM_DEBUG, "Type 0xFF\n");
        dpconf->pldmsnr = 0xff;

        //using EC for different slave address
        if((*(addr1+6) & 0x08))
        {
            DEBUGMSG(PLDM_DEBUG, "Using EC as PLDM Sensor Controller\n");
            set_pldm_snr_slave_address((*(addr1+10) & 0xFE));
            if(*(addr1+10) & 0x01)
                dpconf->snrpec = 1;
            else
                dpconf->snrpec = 0;
        }
        else
        {
            DEBUGMSG(PLDM_DEBUG, "Using BIOS as PLDM Sensor Controller\n");
            set_pldm_snr_slave_address(dpconf->pldmslaveaddr);
            dpconf->snrpec = dpconf->pldmpec;

        }
    }
    else if(addr)
    {
        DEBUGMSG(PLDM_DEBUG, "Type 0x99\n");
        dpconf->pldmsnr = 0x99;
    }
    else
    {
        DEBUGMSG(PLDM_DEBUG, "ASF Type\n");
        GetASFSensor(i);
        return 1;
    }

    addr =getSMBIOSTypeAddr(0x23);
    while(addr)
    {
        cflag = 0;
        sflag = 0;
        component=*(addr+7)+(*(addr+8) <<8);
        threshold=*(addr+9)+(*(addr+10) <<8);
        DEBUGMSG(PLDM_DEBUG, "Component handle %d, %d\n", component, threshold);
        addr1 =getSMBIOSTypeAddr(dpconf->pldmsnr);


        if(!addr1)
        {
            DEBUGMSG(PLDM_DEBUG, "Can not find 0xFF\n");
            return 0;
        }

        while(addr1)
        {
            if(dpconf->pldmsnr == 0xff)
                tmp=*(addr1+4)+(*(addr1+5) <<8);
            else
                tmp=*(addr1+11)+(*(addr1+12) <<8);

            if(tmp==component)
            {
                DEBUGMSG(PLDM_DEBUG, "Get Sensor %d, %d\n", i+1, tmp);
                i++;
                sensor[i].pollindex=0;
                sensor[i].exist=SNR_READ;
                sensor[i].event=0;
                sensor[i].fault=5;
                sensor[i].value=0;
                sensor[i].state=0;
                sensor[i].prestate=0;
                sensor[i].index=i;
                sensor[i].offset[0]=*(addr1+9);
                if(*(addr1+1) >= 0x0C)
                    sensor[i].offset[1]=*(addr1+13);
                else
                    sensor[i].offset[1]= 0;
                strcpy(sensor[i].name,addr+11);
                dpconf->numofsnr++;
                break;
            }
            curLoc1 = addr1;
            ret =getSMBIOSTypeNext(curLoc1, dpconf->pldmsnr, &addr1);

            if(!ret)
            {
                DEBUGMSG(PLDM_DEBUG, "Can not find matched sensor %d\n", component);
                //return 0;
                sflag = 1;
                break;
            }
        }
        if(sflag == 0)
        {
            addr1 =getSMBIOSTypeAddr(0x1a);

            while(addr1)
            {
                tmp=*(addr1+2)+(*(addr1+3) <<8);
                if(tmp==component)
                {
                    sensor[i].type = SNR_VOLTAGE;
                    cflag = 1;
                    DEBUGMSG(PLDM_DEBUG, "Get Voltage Type %d\n", tmp);
                    break;
                }
                curLoc1 = addr1;
                ret=getSMBIOSTypeNext(curLoc1, 0x1a, &addr1);
                if(!ret)
                    break;
            }

            if(cflag == 0)
            {
                addr1 =getSMBIOSTypeAddr(0x1b);
                while(addr1)
                {
                    tmp=*(addr1+2)+(*(addr1+3) <<8);
                    if(tmp==component)
                    {
                        sensor[i].type = SNR_TACHOMETER;
                        cflag = 1;
                        DEBUGMSG(PLDM_DEBUG, "Get Tachometer %d\n", tmp);
                        break;
                    }
                    curLoc1 = addr1;
                    ret=getSMBIOSTypeNext(curLoc1, 0x1b, &addr1);
                    if(!ret)
                        break;
                }
            }

            if(cflag == 0)
            {
                addr1 =getSMBIOSTypeAddr(0x1c);
                while(addr1)
                {
                    tmp=*(addr1+2)+(*(addr1+3) <<8);
                    if(tmp==component)
                    {
                        sensor[i].type = SNR_TEMPERATURE;
                        DEBUGMSG(PLDM_DEBUG, "Get Temperature sensor %d\n", tmp);
                        break;
                    }
                    curLoc1 = addr1;
                    ret=getSMBIOSTypeNext(curLoc1, 0x1c, &addr1);
                    if(!ret)
                        break;
                }
            }


            addr1 =getSMBIOSTypeAddr(0x24);
            while(addr1)
            {
                tmp=*(addr1+2)+(*(addr1+3) <<8);
                if(tmp==threshold)
                {
                    DEBUGMSG(PLDM_DEBUG, "Setting Threshold %d\n", tmp);
                    sensor[i].LNC=*(addr1+4)+(*(addr1+5) <<8);
                    sensor[i].UNC=*(addr1+6)+(*(addr1+7) <<8);
                    sensor[i].LC=*(addr1+8)+(*(addr1+9) <<8);
                    sensor[i].UC=*(addr1+10)+(*(addr1+11) <<8);
                    sensor[i].LF=*(addr1+12)+(*(addr1+13) <<8);
                    sensor[i].UF=*(addr1+14)+(*(addr1+15) <<8);
                    break;
                }
                curLoc1 = addr1;
                ret=getSMBIOSTypeNext(curLoc1, 0x24, &addr1);
                if(!ret)
                    break;
            }
        }

        curLoc = addr;
        ret=getSMBIOSTypeNext(curLoc, 0x23, &addr);
        if(!ret)
        {
            DEBUGMSG(PLDM_DEBUG, "No more 0x23\n");
            break;
        }
    }
    GetASFSensor(i);
    return 1;
}

void set_pldm_snr_slave_address(INT8U addr)
{
    if (pldmsnrsendbuf == NULL)
    {
        pldmsnrsendbuf = malloc(16);
        memcpy(pldmsnrsendbuf, PLDMSNR, 15);
    }
    pldmsnrsendbuf[0] = addr;
    pldmsnrsendbuf[5] = 0x0A;
    pldmsnrsendbuf[6] = 0x09;
}

void pldm_snr_read(INT8U snrnum)
{
    static INT8U msgtag = 0xC8;
    static INT8U instid = 0x80;
    INT8U i = 0;

    if (pldmsnrsendbuf == NULL)
    {
        pldmsnrsendbuf = malloc(16);
        memcpy(pldmsnrsendbuf, PLDMSNR, 15);
    }

    msgtag = (msgtag == 0xCF) ? (0xC8) : (++msgtag);
    instid = (instid == 0x9F) ? (0x80) : (++instid);
    pldmsnrsendbuf[7] = msgtag;
    pldmsnrsendbuf[9] = instid;
    pldmsnrsendbuf[12] = sensor[snrnum].offset[0];
    DEBUGMSG(PLDM_DEBUG, "NIC Sends: ");
#ifdef CONFIG_PLDM_DEBUG
    for(i= 0 ; i < pldmsnrsendbuf[2]+3 ; i++)
        DEBUGMSG(PLDM_DEBUG, "%02x ", pldmsnrsendbuf[i]);
    DEBUGMSG(PLDM_DEBUG, "\n");
#endif
	master_send(PLDM_REQUEST, pldmsnrsendbuf, pldmsnrsendbuf[2] + 3, 0x09);
#if 0	
    master_send(PLDM_RESPONSE, pldmsnrsendbuf, pldmsnrsendbuf[2] + 3, 0x09);
#endif

}
