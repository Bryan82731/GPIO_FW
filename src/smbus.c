#include <string.h>
#include <stdlib.h>
#include "smbus.h"
#include "rmcp.h"
#include "bsp.h"
#include "time.h"
#include "lib.h"
#include "rmcp.h"
#include "pldm.h"
#include "smbios.h"
#include "tcp.h"
#include "arp.h"

extern DPCONF *dpconf;
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern asf_config_t *asfconfig;
extern INT8U *timestampdataptr;
extern pldm_t *pldmdataptr;
extern pldm_res_t *pldmresponse[7];
extern INT8U smbios_flag;
extern INT8U *smbiosptr;

ROM_EXTERN2 OS_EVENT *PLDMSNRQ _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 void *PLDMSNRMsg[MAX_SENSOR_NUMS] _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U smbrxsize _ATTRIBUTE_ROM_BSS;
//ROM_EXTERN2 OS_EVENT *SMBUSLOGQ _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U smbrxdescnum _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U **smblogptr _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U smblogidx _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U *smbmtxbuffer _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U *smbtxbuffer _ATTRIBUTE_ROM_BSS;

ROM_EXTERN2 sensor_t sensor[MAX_SENSOR_NUMS];
extern OS_EVENT *SMBUSLOGQ;

#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
extern INT8U gIsLanWake;
#endif


#if (CONFIG_VERSION  >= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
//function pointers added in EP
#if (CONFIG_VERSION <= IC_VERSION_DP_RevF)
void (*handle_pldm)(INT8U *addr) _ATTRIBUTE_ROM_BSS;
void (*smbus_process_pldm)(INT8U *addr) _ATTRIBUTE_ROM_BSS;
#endif

extern INT8U RTKUDID[18];
extern flash_data_t dirty_ext[10];

//eventdata  *event_ptr;
ROM_EXTERN OS_EVENT *EVTQ _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U data_index _ATTRIBUTE_ROM_BSS;
ROM_EXTERN eventdata *eventdataptr _ATTRIBUTE_ROM_BSS;
ROM_EXTERN eventdata *event_head _ATTRIBUTE_ROM_BSS;
//ROM_EXTERN void *PLDMSNRMsg[MAX_SENSOR_NUMS] _ATTRIBUTE_ROM_BSS;
ROM_EXTERN OS_EVENT *SMBQ _ATTRIBUTE_ROM_BSS;
//set the maximal for block read case
ROM_EXTERN void *SMBUSMsg[64] _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U SMBRxDescIndex _ATTRIBUTE_ROM_BSS;
ROM_EXTERN INT8U smbuscount _ATTRIBUTE_ROM_BSS;


//#if CONFIG_VERSION  <= IC_VERSION_DP_RevF
#if (CONFIG_VERSION  >= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
void smbus_process_polling(INT8U *ptr)
{

    INT8U polledval, eventchanged = 0;
    eventdata evtdata;
    eventdata *evt_ptr;
    OS_CPU_SR  cpu_sr = 0;
    INT8U i, snrval;

    polledval = *ptr;
    //polledval = ptr->rtaddr;
    if (smbiosrmcpdataptr->PollType == POLL_LS)
    {
        DEBUGMSG(SENSOR_DEBUG, "Get Sensor index %d status %x, polled value %x\n", smbiosrmcpdataptr->SensorIndex, smbiosrmcpdataptr->LSensor[smbiosrmcpdataptr->SensorIndex], polledval);
        //detect state change
        if (smbiosrmcpdataptr->LSensor[smbiosrmcpdataptr->SensorIndex] !=polledval)
        {
            memset(&evtdata, 0, sizeof(evtdata));
            smbiosrmcpdataptr->LSensor[smbiosrmcpdataptr->SensorIndex] = polledval;
            for(i = 0 ; i < asfconfig->numofalerts; i++)
            {
                DEBUGMSG(SENSOR_DEBUG, "Get Sensor addr %x command %x, index %x\n", asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].addr, asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].command, i);
                if((asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].addr != asfconfig->asfalerts[i].address) || (asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].command != asfconfig->asfalerts[i].command))
                    continue;

                snrval = polledval & asfconfig->asfalerts[i].datamsk;

                //Assume 0 means normal state
                if (snrval == 0)
                    SetSnrState(0xff);

                DEBUGMSG(SENSOR_DEBUG, "Sensor Compared Value %x\n", snrval);
                if((snrval == asfconfig->asfalerts[i].cmpvalue))
                {
                    memcpy(&evtdata.Event_Sensor_Type, &asfconfig->asfalerts[i].evtsnrtype, 8);
                    SetSnrState(asfconfig->asfalerts[i].evtoffset);

                    evtdata.logtype = asfconfig->asfalerts[i].logtype;
                    evt_ptr = event_log(&evtdata.Event_Sensor_Type, 8);
                    evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
                    evt_ptr->timeout = 0;
                    evt_ptr->alertnum = dpconf->numsent;
                    asfconfig->asfalerts[i].status = EVENT_ASSERT;
                    OSQPost(EVTQ, evt_ptr);

                }
                else
                {
                    if(asfconfig->asfalerts[i].status == EVENT_ASSERT)
                    {
                        asfconfig->asfalerts[i].evtoffset |= 0x80;
                        memcpy(&evtdata.Event_Sensor_Type, &asfconfig->asfalerts[i].evtsnrtype, 8);
                        SetSnrState(asfconfig->asfalerts[i].evtoffset);

                        evtdata.logtype = asfconfig->asfalerts[i].logtype;
                        evt_ptr = event_log(&evtdata.Event_Sensor_Type, 8);
                        evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
                        evt_ptr->timeout = 0;
                        evt_ptr->alertnum = dpconf->numsent;
                        asfconfig->asfalerts[i].status = EVENT_DEASSERT;
                        OSQPost(EVTQ, evt_ptr);

                    }
                }

            }
            ///smbiosrmcpdataptr->LSPFlag |= polledval;
        }
        else
        {
            snrval = polledval & asfconfig->asfalerts[i].datamsk;

            //Assume 0 means normal state
            if (snrval == 0)
                SetSnrState(0xff);
        }
        OS_ENTER_CRITICAL();
        smbiosrmcpdataptr->PollType = POLL_NONE;
        OS_EXIT_CRITICAL();
    } //end of LS POLL
    else if (smbiosrmcpdataptr->PollType == POLL_ASF_STATUS)
    {

        smbiosrmcpdataptr->ASFFlag = 0;
        //eventchanged = 0;
        //reset the index for asf sensor
        for (i = 0 ; i < ifloor(*(ptr+1), 2); i++)
        {
            if ((*(ptr+i+2) & 0x0f)	!= (smbiosrmcpdataptr->Status[i] & 0x0f))
            {
                smbiosrmcpdataptr->Flag[smbiosrmcpdataptr->ASFFlag++] = 2*i;
                eventchanged = 1;
            }

            if ((*(ptr+i+2) & 0xf0)	!= (smbiosrmcpdataptr->Status[i] & 0xf0))
            {
                smbiosrmcpdataptr->Flag[smbiosrmcpdataptr->ASFFlag++] = 2*i+1;
                eventchanged = 1;
            }

        }
        if (eventchanged)
        {
            eventchanged = 0;
            memcpy(smbiosrmcpdataptr->Status, ptr+2 , ifloor(*(ptr+1), 2));
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_ASF_PEND;
            OS_EXIT_CRITICAL();
        }
        else
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_NONE;
            OS_EXIT_CRITICAL();
        }
    } //end of ASF sensor polling status
    else if (smbiosrmcpdataptr->PollType == POLL_ASF_DATA)
    {
        //evtdata.logtype = LOG_WARNING;
        evt_ptr = event_log(ptr+2, *ptr-1);
        setlogtype(evt_ptr);
        evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
        evt_ptr->timeout = 0;
        evt_ptr->alertnum = dpconf->numsent;
        if (--smbiosrmcpdataptr->ASFFlag == 0)
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_NONE;
            OS_EXIT_CRITICAL();
        }
        else
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_ASF_PEND;
            OS_EXIT_CRITICAL();
        }
        OSQPost(EVTQ, evt_ptr);
    }

}

void smbus_asfpoll()
{

    static INT8U asfidx = 0;
    static INT16U count =  3 ;


    if (((count % dpconf->polltime) == 0 ))
    {
        if (smbiosrmcpdataptr->PollType == POLL_NONE)
        {
            if (!SMBus_Prepare_GetEventStatus())
            {
                smbiosrmcpdataptr->PollType = POLL_ASF_STATUS;
                smbiosrmcpdataptr->PollTimeOut = 0;
            }
        }
    }

    if (smbiosrmcpdataptr->PollType == POLL_ASF_PEND)
    {
        if (smbiosrmcpdataptr->ASFFlag)
        {
            SMBus_Prepare_GetEventData(smbiosrmcpdataptr->Flag[asfidx++]);
            smbiosrmcpdataptr->PollType = POLL_ASF_DATA;
            smbiosrmcpdataptr->PollTimeOut = 0;
        }
    }

    count++;
}

void smbus_hook()
{
    static INT32U count= 0 ;

    if (REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x01)
        smbuscount++;
    if (smbuscount == 5)
    {

#ifdef CONFIG_GPIO_DEBUG
        GPIOSet(0);
#endif
        DEBUGMSG(CRITICAL_ERROR, "!!!!!!!!!!!!!! Reset SMBus # %06d!!!!!!!!!!!!!!\n", ++count);
        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0x00;
        REG8(SMBUS_BASE_ADDR+SEIO_Status) = 0x00;

        bsp_smbus_init(0);

        smbuscount = 0;

    }
}
/* SMBUS0 */

#define SMB0_TxdescStartAddr	(DMEM_V_BASE + 0x8000)//length=0x30
#define SMB0_MTxdescStartAddr	(DMEM_V_BASE + 0x8030)//length=0x30
#define SMB0_RxdescStartAddr	(DMEM_V_BASE + 0x8060)//length=0x200
#define SMB0_RxbuffStartAddr	(DMEM_V_BASE + 0x8300)//length=0x1300
#define SMB0_TxbuffStartAddr	(DMEM_V_BASE + 0x9600)//length=0x80
#define SMB0_MTxbuffStartAddr	(DMEM_V_BASE + 0x9680)//length=0x40

void bsp_smbus_init(INT8U reset)
{

    INT8U i, freq_div = 0x01, freq_slave_div=0x03;
    smbrxdesc_t *smbrxdesc = (smbrxdesc_t *)(SMBRxdescStartAddr);
    smbrxdesc_t *smbtxdesc = (smbrxdesc_t *)(SMBTxdescStartAddr);
    smbrxdesc_t *smbmtxdesc = (smbrxdesc_t *)(SMBMTxdescStartAddr);

    SMBRxDescIndex = 0;

    if(dpconf->pldmtype == PLDM_BLOCK_READ)
    {
        if(dpconf->bios == PHOENIXBIOS)
        {
            handle_pldm = handle_pldm_br_phoenix;
            smbus_process_pldm = smbus_process_pldm_br_phoenix;
        }
        else
        {
            handle_pldm = handle_pldm_br_ami;
            smbus_process_pldm = smbus_process_pldm_br_ami;
        }
        smbrxdescnum = 64;
        smbrxsize = 36;

    }
    else
    {
        smbus_process_pldm = smbus_process_pldm_bw;
        smbrxdescnum = 16;
        smbrxsize = 76;
        handle_pldm = handle_pldm_bw;
    }


    // SMBus Engine Firmware Reset and Random Number Reset
    // 04.17.08 tomadd -- To Modify Hardware Rx Fetch Error(Close Error)
#ifdef SMBUS_Loopback
    REG8(SMBUS_BASE_ADDR + SEIO_SMEn2) = 0xCF;
#endif
    if (!asfconfig->pec)
        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0xCD;
    else
        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0xCF;
    //for Intel => disable pec, others => enable

    REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) = 0xc0;
    REG32(SMBUS_BASE_ADDR + SEIO_SCTimeOut) = 0x08186a00;
    //03 (08) for retransmission
#if 0
    REG16(SMBUS_BASE_ADDR + SEIO_SampleTimingCtrl) = (0x03b4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_PutDataTimingCtrl) = (0x003e / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SMBFallingSetting) = (0x0030 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SMBRisingSetting) = (0x0090 / freq_div);
#endif
    //REG8(SMBUS_BASE_ADDR + SEIO_MIS_ERR ) = 0x00;
    REG32(SMBUS_BASE_ADDR + SEIO_SlaveAddr) = 0xc8c8c8c8;
    //change the 2nd smbus arp address, since the first one is 0xC2
    ///REG8(SMBUS_BASE_ADDR + SEIO_SlaveAddr + 1) = dpconf->arpaddr;
    //C2 no response
    //04.06 tomadd master mode initial timing
    REG16(SMBUS_BASE_ADDR + SEIO_BusFree) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_1) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_DataHoldTime) = (0x03E / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterClkLow) = (0x0768 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterClkHigh) = (0x0768 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterRBControl) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_MasterTDControl) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveTimingCtrl) = (0x03B4 / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveLDTimingCtrl) = (0x003F / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBCFallTime) = (0x0030 / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBCRiseTime) = (0x0090 / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBDFallTime) = (0x0030 / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBDRiseTime) = (0x0090 / freq_slave_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_2) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_2) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_2) = (0x00FE / freq_div);
    REG8(SMBUS_BASE_ADDR + SEIO_SMBCFallTime) = 0x02;
    REG8(SMBUS_BASE_ADDR + SEIO_SMBCRiseTime) = 0x02;
    REG8(SMBUS_BASE_ADDR + SEIO_SMBDFallTime) = 0x02;
    REG8(SMBUS_BASE_ADDR + SEIO_SMBDRiseTime) = 0x02;

#ifdef SMBUS_Loopback
    REG16(SMBUS_BASE_ADDR + SEIO_SampleTimingCtrl2) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_PutDataTimingCtrl2) = (0x003E / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SMBFallingSetting2) = (0x0030 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SMBRisingSetting2) = (0x0090 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_BusFree2) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_1S) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_1S) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_1S) = (0x03E8 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_DataHoldTime_S) = (0x03E / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveTimingCtrl_S) = (0x03B4 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveLDTimingCtrl_S) = (0x003E / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBCFallTime_S) = (0x0030 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBCRiseTime_S) = (0x0090 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBDFallTime_S) = (0x0030 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_SlaveSMBDRiseTime_S) = (0x0090 / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_HoldTimeRStart_2S) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_RS_SetupTime_2S) = (0x00FE / freq_div);
    REG16(SMBUS_BASE_ADDR + SEIO_StopSetupTime_2S) = (0x00FE / freq_div);
#endif


    for (i = 0; i < smbrxdescnum; i++)
    {
        smbrxdesc->st.index  = i;
        smbrxdesc->st.length = smbrxsize;
        smbrxdesc->rx_buf_addr = (INT8U *) VA2PA(SMBRxbuffStartAddr) + i* (smbrxdesc->st.length);
        //prevent hardware overwrite each other
        if (i == smbrxdescnum-1)
            smbrxdesc->st.eor  = 1;
        else
            smbrxdesc->st.eor  = 0;
        smbrxdesc->st.own = 1;
        smbrxdesc++;
    }

    if(reset)
    {
        smbmtxbuffer = malloc(128);
        smbtxbuffer  = malloc(128);
    }

    //slave send buffer address
    smbtxdesc->rx_buf_addr = (INT8U *) VA2PA(smbtxbuffer);

    //master send buffer address
    smbmtxdesc->rx_buf_addr = (INT8U *) VA2PA(smbmtxbuffer);

    REG32(SMBUS_BASE_ADDR + SEIO_RxDesc) = VA2PA(SMBRxdescStartAddr);
    REG32(SMBUS_BASE_ADDR + SEIO_RxCurDesc) = VA2PA(SMBRxdescStartAddr);
    REG32(SMBUS_BASE_ADDR + SEIO_TxDesc) = VA2PA(SMBTxdescStartAddr);
    REG32(SMBUS_BASE_ADDR + SEIO_MTxDesc) = VA2PA(SMBMTxdescStartAddr);

#ifdef SMBUS_Loopback
    REG8(SMBUS_BASE_ADDR+SEIO_IMR2) = 0x11;
#else
#ifdef CONFIG_SMBUS_DEBUG
    REG8(SMBUS_BASE_ADDR+SEIO_IMR) = 0xFF;
#else
    REG8(SMBUS_BASE_ADDR+SEIO_IMR) = 0x13;
#endif
#endif

    if(reset)
    {
        if(dpconf->pldmdbg)
        {
            smblogptr = malloc(smbrxdescnum*sizeof (int *));
            for(i= 0 ; i < smbrxdescnum; i++)
                smblogptr[i] = malloc(smbrxsize*sizeof(char));
            smblogidx = 0;

        }
        //PLDMSNRSem = OSSemCreate(0);
        PLDMSNRQ = OSQCreate(PLDMSNRMsg,MAX_SENSOR_NUMS);
        SMBQ = OSQCreate(SMBUSMsg,smbrxdescnum);
        rlx_irq_register(BSP_SMBUS_IRQ, bsp_smbus_handler);
    }

}

void SMBUSFree(smbrxdesc_t *rxdesc)
{
#ifdef CONFIG_SMBUS_DEBUG
    smbrxdesc_t *smbtmp = (smbrxdesc_t *)(SMBRxdescStartAddr);
    INT32U val32_1 = 0, val32_2 = 0, valtmp;
    INT8U count;
#endif


    //val32 = (*((INT32U *)smbtmp) & 0x80000000) | (*((INT32U *)(smbtmp+1)) & 0x80000000) >> 4 | (*((INT32U *)(smbtmp+2)) & 0x80000000) >> 8 | (*((INT32U *)(smbtmp+3)) & 0x80000000) >> 12 | (*((INT32U *)(smbtmp+4)) & 0x80000000 ) >> 16 | (*((INT32U *)(smbtmp+5)) & 0x80000000) >> 20 | (*((INT32U *)(smbtmp+6)) & 0x80000000) >> 24 | (*((INT32U *)(smbtmp+7)) & 0x80000000) >> 28;

    //DEBUGMSG(SMBUS_DEBUG, "Before Set %08x(%d) ", val32, rxdesc->st.index);
    memset((void *) PA2VA(rxdesc->rx_buf_addr), 0, smbrxsize);
    //rxdesc->st.length = SMBRxSIZE;
    rxdesc->st.own = 1;
    //val32 = (*((INT32U *)smbtmp) & 0x80000000) | (*((INT32U *)(smbtmp+1)) & 0x80000000) >> 4 | (*((INT32U *)(smbtmp+2)) & 0x80000000) >> 8 | (*((INT32U *)(smbtmp+3)) & 0x80000000) >> 12 | (*((INT32U *)(smbtmp+4)) & 0x80000000 ) >> 16 | (*((INT32U *)(smbtmp+5)) & 0x80000000) >> 20 | (*((INT32U *)(smbtmp+6)) & 0x80000000) >> 24 | (*((INT32U *)(smbtmp+7)) & 0x80000000) >> 28;
#ifdef CONFIG_SMBUS_DEBUG
    for(count = 0; count < smbrxdescnum; count++)
    {
        valtmp = (*(INT32U *) smbtmp & 0x80000000);
        if(count <= 31)
            val32_1 |= (valtmp >> count);
        else
            val32_2 |= (valtmp >> (count-32));
        smbtmp++;
    }

    DEBUGMSG(SMBUS_DEBUG, "Recycling  %08x %08x (%d)\n", val32_1, val32_2, rxdesc->st.index);
#endif
}

void smbus_arp(INT8U *addr)
{
    static INT8U arpindex = 0;
    INT8U *bptr;


    //addr[1] is smbus comand
    if (addr[2] == 0xC3 && addr[1] == 0x03)
    {
        bptr = SMBus_Get_UDID();
        if(dpconf->pldmdbg)
        {
            memcpy((smblogptr[smblogidx]+5), bptr, *bptr+1);
            smblogptr[smblogidx][0] = 0x80;
            smblogptr[smblogidx][1] = *bptr+1;
        }
    }
    else if (addr[1] == 0x01) //prepare arp
    {
        arpindex = 0;
        if(dpconf->pldmdbg)
            smblogptr[smblogidx][1] = 3;
    }
    else if (addr[1] == 0x04)
    {
        //if (SMBus_Check_UDID(&addr[3]) ==0)
        {
            dpconf->arpaddr = addr[19];
            REG8(SMBUS_BASE_ADDR + SEIO_SlaveAddr + arpindex++) = addr[19];
        }
    }

    //assing value for easily handling in TASK
    addr[0] = dpconf->arpaddr;
    addr[2] = dpconf->arpaddr + 1;

}

void bsp_smbus_handler(void)
{

    INT8U val8, v2;
#ifdef CONFIG_CRITICAL_ERROR
    INT8U loopcount = 0;
#endif

#ifdef CONFIG_SMBUS_DEBUG
    INT8U v1;
    INT32U val32_1 = 0, val32_2 = 0, valtmp;
    smbrxdesc_t *smbtmp = (smbrxdesc_t *)(SMBRxdescStartAddr);
    static INT32U rducount = 0;
#endif

    INT8U err = 0;
    //INT8U *addr;
    INT8U *bptr;
    //SMBIOS_Table_EP *smbiosmeta ;
    //static INT8U arpindex = 0;
    INT8U count = 0 ;

    smbrxdesc_t *smbrxdesc = (smbrxdesc_t *) SMBRxdescStartAddr + SMBRxDescIndex;

    //smbioshdr *ptr = (smbioshdr *) PA2VA(smbrxdesc->rx_buf_addr) ;
    //addr = (INT8U *) ptr;
    INT8U *addr = (INT8U *) PA2VA(smbrxdesc->rx_buf_addr);

    smbuscount = 0;
    REG8(SMBUS_BASE_ADDR+SEIO_IMR) = 0x00;
    val8 = REG8(SMBUS_BASE_ADDR+SEIO_ISR);
    REG8(SMBUS_BASE_ADDR+SEIO_ISR) = ( val8 & 0xEF);
    v2 = REG8(SMBUS_BASE_ADDR + 0x01);

#ifdef CONFIG_SMBUS_DEBUG
    v1 = REG8(SMBUS_BASE_ADDR);

    DEBUGMSG(SMBUS_DEBUG, "In ISR %08x(%02d), En(%02x), SR(%02x), ISR(%02x) => Handling ISR ", *((INT32U *)smbrxdesc), smbrxdesc->st.index, v1, v2 ,val8 );
#endif

    if(val8 & 0x10)
    {
#ifdef CONFIG_GPIO_DEBUG
        GPIOSet(0);
#endif
        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0x00;
        REG8(SMBUS_BASE_ADDR+SEIO_Status) = 0x00;
        DEBUGMSG(CRITICAL_ERROR, "SMBUS RDU Happened # %06d\n",++rducount);
        bsp_smbus_init(0);
        REG8(SMBUS_BASE_ADDR+SEIO_ISR) = 0x10;
        DEBUGMSG(CRITICAL_ERROR, "Clear RDU\n");
#ifdef CONFIG_GPIO_DEBUG
        GPIOSet(1);
#endif
    }

    count = 0;

    if(dpconf->pldmdbg)
    {
        memcpy((smblogptr[smblogidx] + 2), addr, smbrxsize - 1);
        smblogptr[smblogidx][0] = 0x00;
        smblogptr[smblogidx][1] = ((addr[2] + 2) <= (smbrxsize)) ? (addr[2]+2) :(smbrxsize);
        smblogptr[smblogidx][smbrxsize -1 ] = val8;
    }

    while (!smbrxdesc->st.own)
    {
#ifdef CONFIG_CRITICAL_ERROR
        loopcount++;
        if(loopcount > 32)
            DEBUGMSG(CRITICAL_ERROR, "Handling SMBus %02d \n", loopcount);
#endif
        DEBUGMSG(SMBUS_DEBUG, "%02d ", ++count );
        //broadcast address +1, Get UDID
        if (addr[0] == 0xC2)
        {
            smbus_arp(addr);
            err = OSQPost(SMBQ, (void *) smbrxdesc);

            if (err == OS_ERR_Q_FULL)
                SMBUSFree(smbrxdesc);

        }
        //byte read
        else if ((addr[0] != dpconf->arpaddr) && (smbiosrmcpdataptr->PollType == POLL_SENSOR))
        {
            err = OSQPost(PLDMSNRQ, (void *) smbrxdesc);

            if (err == OS_ERR_Q_FULL)
                SMBUSFree(smbrxdesc);

        }
        else if ((addr[0] != dpconf->arpaddr) && (smbiosrmcpdataptr->PollType == POLL_LS))
        {

            err = OSQPost(SMBQ, (void *) smbrxdesc);

            if (err == OS_ERR_Q_FULL)
                SMBUSFree(smbrxdesc);
        }
        //check not a block read command, means a block write or byte read
        //else if (addr[2] != (dpconf->arpaddr + 1) )
        else if ((addr[0] == dpconf->arpaddr) && (addr[2] != (dpconf->arpaddr + 1) ))
        {
            //set block read type in advance for BIOS management only
            //try to get the PLDM command at the first
            handle_pldm(addr);

            if (addr[1] == 0x0F && addr[10] == 0x02 && addr[11] == 0x11)
                err = OSQPost(PLDMSNRQ, (void *) smbrxdesc);
            else
                err = OSQPost(SMBQ, (void *) smbrxdesc);

            if (err == OS_ERR_Q_FULL)
                SMBUSFree(smbrxdesc);

        }
        else //sould be block read
        {
            if (addr[1] == 0x03)
            {
                bptr = BootOptions_F();//SMBus_Prepare_BootOptions();
                if(dpconf->pldmdbg)
                {
                    memcpy((smblogptr[smblogidx]+5), bptr, *bptr+1);
                    smblogptr[smblogidx][0] = 0x80;
                    smblogptr[smblogidx][1] = *bptr+1;
                }
            }
            // 03/31 for ASF Message Function
#if 0
            else if (ptr->cmd==0x01 && ptr->bytecount==0xC9)
            {
                SMBus_DTP_Message();
            }
#endif

            //handling PLDM response using block read
            else if (addr[1] == 0x0f && dpconf->pldmtype == PLDM_BLOCK_READ)
            {
                //DEBUGMSG(CRITICAL_ERROR, "Response # %d\n",pldmdataptr->index);
                if(pldmdataptr->index > 5)
                {
                    pldmdataptr->index = 4;
                }
                smbus_send_F(&pldmresponse[pldmdataptr->index]->length, pldmresponse[pldmdataptr->index]->length + 1);
                if(dpconf->pldmdbg)
                {
                    smblogptr[smblogidx][0] = 0x80;
                    smblogptr[smblogidx][1] = pldmresponse[pldmdataptr->index]->length+3;
                    memcpy(smblogptr[smblogidx]+5, &pldmresponse[pldmdataptr->index]->length, pldmresponse[pldmdataptr->index]->length+3);

                }
                //for handling multi-part transfers
                if(!(v2 & 0x20))
                    pldmdataptr->index++;
            }

            err = OSQPost(SMBQ, (void *) smbrxdesc);

            if (err == OS_ERR_Q_FULL)
                SMBUSFree(smbrxdesc);
        }
        SMBRxDescIndex = ( SMBRxDescIndex + 1 )% (smbrxdescnum) ;
        smbrxdesc = (smbrxdesc_t *) SMBRxdescStartAddr + SMBRxDescIndex;
    }

#ifdef CONFIG_SMBUS_DEBUG
    if(val8 & 0x10)
        DEBUGMSG(SMBUS_DEBUG, "\nSMBus RDU\n");

    if(v2 & 0x20)
        DEBUGMSG(SMBUS_DEBUG, "\nSMBus Slave Address Collision\n");
#endif

    if(dpconf->pldmdbg && (val8 & 0x03))
    {
        err = OSQPost(SMBUSLOGQ, (void *) smblogptr[smblogidx]);
        smblogidx = (smblogidx + 1) % smbrxdescnum;
    }

#ifdef CONFIG_SMBUS_DEBUG
    REG8(SMBUS_BASE_ADDR+SEIO_IMR) = 0xFF;

    v1 = REG8(SMBUS_BASE_ADDR);
    v2 = REG8(SMBUS_BASE_ADDR + 0x01);
    val8 = REG8(SMBUS_BASE_ADDR+SEIO_ISR);

    for(count = 0; count < smbrxdescnum ; count++)
    {
        valtmp = (*(INT32U *) smbtmp & 0x80000000);
        if(count <= 31)
            val32_1 |= (valtmp >> count);
        else
            val32_2 |= (valtmp >> (count-32));
        smbtmp++;
    }

    DEBUGMSG(SMBUS_DEBUG, "\nLeave ISR %08x %08x, En(%x), SR(%x), ISR(%x)\n", val32_1, val32_2,  v1, v2, val8 );
#else
    REG8(SMBUS_BASE_ADDR+SEIO_IMR) = 0x13;
#endif

}
#endif

//eventdata  *event_ptr;
ROM_EXTERN eventdata *eventdataptr _ATTRIBUTE_ROM_BSS;
ROM_EXTERN eventdata *event_head _ATTRIBUTE_ROM_BSS;
extern smbiosrmcpdata *smbiosrmcpdataptr;
//event_list *event_head = NULL;
//static event_list event_list_entry[MAX_EVENT_ENTRY];
ROM_EXTERN INT8U data_index _ATTRIBUTE_ROM_BSS;
extern DPCONF *dpconf;
extern flash_data_t dirty[ENDTBL];
#if 0
INT8U rmttbl[16][2] _ATTRIBUTE_ROMDATA = {DASH_PWR_On, DASH_Reset, DASH_Shutdown, DASH_PWR_CR, "0"} ;
#endif

# if 0
extern INT32S SMBus_Check(INT32S *smbstartaddr, INT32S smbus_length);
static INT8U choose_rtaddr(INT8U *pool);
extern void smbus_recycle();
void SMBus_Prepare_RTudid(INT32S* Buffer_address, INT32S length);

INT32S SMBDescIndex, SMBRCBIndex, TxDescIndex, TxRCBIndex, CurRCBIndex;
INT32S RxTotal, RxOK, RxErr;
INT32U random_num;
// smbus host pool data

static INT8U choose_rtaddr(INT8U *pool)
{
    INT32S i;
    for (i=0; i<0x7f; i++)
    {
        if (pool[i] == ARP_FREE)
        {
            return ((INT8U) i);
        }
    }
    return 0xff;
}

void SMBus_Prepare_RTudid(INT32S* Buffer_address, INT32S length)
{
    INT32S *ptr;
    INT32S i;
    INT8S *bptr;
    INT8S ByteCount;

    struct arp_rtdevice *arp_rtptr;
    //arp_rtptr = RTLMalloc(sizeof(struct arp_rtdevice));
    arp_rtptr->dev_cap = ((Address_Type&0x40)|(Dev_CapRsvd&0x00)|(PEC_support&0x01));
    arp_rtptr->dev_ver = ((Rsvd&0x00)|(UDID_Ver&0x08)|(Silicon_RevsionID&0x00));
    arp_rtptr->dev_vid =  VendorID;
    arp_rtptr->dev_did = DeviceID;
    arp_rtptr->dev_INT32S =  (ASF_Support|SMBus_Version);
    arp_rtptr->dev_svid = SubVerdorID;
    arp_rtptr->dev_sdid = SubDeviceID;
    // note1. semaphore algorithm
    // Vendor Specific ID
    arp_rtptr->dev_vsid = random_num;
    ptr= (INT32S*)(Buffer_address);
    ByteCount = UDID_LENGTH;
    *ptr = ((ByteCount&0x000000ff)<<24)+(((arp_rtptr->dev_cap)&0x000000ff)<<16);


}

INT32S SMBus_Check(INT32S *smbstartaddr, INT32S smbus_length)
{
    INT8S *addr;
    INT8S val8;
    INT32S i;
    INT32S val32;
    INT8U  *SMBDescBuf;

    INT32S errorcode=0;
    INT32S cnt = 0;
    struct	SMBus_ARP *SMBusptr;
    //SMBusptr = RTLMalloc(sizeof(struct SMBus_ARP));

    //SMBDescBufSize = (Adapter->SMBDesc* sizeof(struct SMBDesc))+256;
    //arp_rtdevice=RTLMalloc(SMBDescBufSize);
    //SMBDescBuf=RTMalloc(SMBDescBufSize);
    //setmem(SMBDescBuf, SMBDescBufSize, 0x00);

    addr = (INT8S*)(smbstartaddr); //smbus start memory poINT32Ser
    addr = addr+0x80100000; // smbus descriptor memory address ?

    // 1.Check ASD slave address
    val8 = *(addr+0);
    if (val8 == ARP_ADDRESS)
        return 1; //right slave address
    else
        return 0; //wrong slave address
    val8 = *(addr+1);
    if (val8 == ARP_PREPARE)
    {
        return 1; //prepare to arp
        //val8 = *(addr+2);
        //if(val8 == PEC)
        //	return 1;
        //else
        //	return 0;

    }
    else if (val8 == ARP_RESET_DEV)
    {
        return DASH_FAIL; //reset ASD
        //val8 = *(addr+2);
        //if(val8 == PEC)
        //	return 1;
        //else
        //	return 0;

    }
    else if (val8 == ARP_GET_UDID_GEN)
    {
        //return 3; //get realtek udid
        val8 = *(addr+2);
        if (val8 == (ARP_ADDRESS+1))
        {
            //(1)prepare realtek udid value.

            return 3;
        }
        else
            return 0;//wrong slave address.
    }
    else if (val8 == ARP_ASSIGN_ADDR)
    {
        return 4; //assign slave address
        val8 = *(addr+2);
        if (val8 == UDID_LENGTH)
        {
            return 1;
            for (i=0; i<UDID_LENGTH; i++)
            {
                val8 = *(addr+3+i);
                //SMBusptr -> RTUDIDptr = (struct arp_rtdevice *)(((INT32U)SMBDescBuf));
                //if(val8 == SMBusptr->)
            }
        }
        else
            return 0;

    }
    else if (val8 == SMBIOS_Command)
    {
        return 5; //SMBIOS_Command.
    }
    else
        return 0; //Wrong SMBIOS Command.


    //return 0;
}

#endif
INT8U master_check()
{
    INT8U loop_count = 0;
    INT8U ret = 0;

    //0 means idle
    while (((REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x07)) && loop_count < 100)
    {
        OSTimeDly(1);
        loop_count++;
    }

    if (loop_count == 100)
        ret = 1;
    return ret;
}

INT8U master_send(msendtype type, INT8U* bufaddr, INT8U length, int cmd)
{
    //no need to copy from bufaddr, since bufaddr is the smbtx buf addr
    INT8U val8, ret;
    INT8U count = 0;
    OS_CPU_SR cpu_sr = 0;
    smbrxdesc_t *txdesc = (smbrxdesc_t *) (SMBMTxdescStartAddr);
    static INT8U enter = 0;
    static INT8U *smbuslog = NULL;
    INT8U err;

    INT8U mFlag;
    INT16U macspeed=0;
	INT32U eee=0;	
	
// using if(dpconf->pldmdbg) instead
// so that we can do the debug using ROM
    if(dpconf->pldmdbg)
    {
        if(smbuslog == NULL)
            smbuslog = malloc(128);
        memcpy(&smbuslog[2], bufaddr, length);
        if(cmd == 0x06 || cmd == 0x0b)
            smbuslog[0] = 0x40;
        else
            smbuslog[0] = 0x20;
        smbuslog[1] = length;
        err = OSQPost(SMBUSLOGQ, (void *) smbuslog);
    }

    if (enter)
        return DASH_FAIL;

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

//	mFlag = CheckEEE(&macspeed,&eee);

    val8 = REG8(SMBUS_BASE_ADDR+SEIO_ISR);
    while ((val8 & 0x02))
    {
        OSTimeDly(1);
        if (count++ > 100){

		if(mFlag){
			//restoreEEE(macspeed,eee);
		}
			
            return 1;
    }
    }

    ret = master_check();

    if (ret == 0)
    {
        if((type == PLDM_RESPONSE) && !dpconf->pldmfromasf)
        {

            if(dpconf->pldmpec)
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x02;
            else
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0xFD;
        }
        //using global pec setting
        else
        {
            if(asfconfig->pec)
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x02;
            else
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0xFD;

        }

        ///OS_ENTER_CRITICAL();
        //check again the status to avoid slave send happened at the same time
        if (!(REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x07))
        {
            if (type == BMC_POLL)
                bsp_bits_set(MAC_BIOSREG, 1, BIT_SYSSH, 1);
            //bsp_bits_set(MAC_BIOSREG, 1, BIT_SYSSH, 1);
            if (type == BMC_POLL && (bsp_get_sstate() != S_S0 || smbiosrmcpdataptr->PollType == POLL_STOP) )
            {
                bsp_bits_set(MAC_BIOSREG, 0, BIT_SYSSH, 1);
                //bsp_bits_set(MAC_BIOSREG, 0, BIT_SYSSH, 1);
                ret = 1;
            }
            else
            {
                txdesc->rx_buf_addr = (INT8U *) VA2PA(bufaddr);
                txdesc->st.length = length;
                txdesc->st.own = 1;
                REG8(SMBUS_BASE_ADDR+SEIO_ModeCmd) = cmd;
                REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) = 0x10;
            }
        }

        ///OS_EXIT_CRITICAL();
    }

    OS_ENTER_CRITICAL();
    enter = 0;
    OS_EXIT_CRITICAL();


	
		if(mFlag){
			//restoreEEE(macspeed,eee);
		}

    return ret;
}


//#if CONFIG_VERSION  < IC_VERSION_DP_RevF
#if (CONFIG_VERSION  >= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
void smbus_send(INT8U *bufaddr, INT8U length)
{
    //no need to copy from bufaddr, since bufaddr is the smbtx buf addr
    smbrxdesc_t *txdesc = (smbrxdesc_t *) (SMBTxdescStartAddr);

    if (bufaddr != NULL)
        txdesc->rx_buf_addr = (INT8U *) VA2PA(bufaddr);
    else
        txdesc->rx_buf_addr = (INT8U*) VA2PA(smbtxbuffer);//VA2PA(SMBTxbuffStartAddr);

    txdesc->st.length = length;
    txdesc->st.own = 1;

    REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) = 0x01;
    //DEBUGMSG(MESSAGE_DEBUG, "Buff Addr %x\n", bufaddr);

}
#endif

#ifdef CONFIG_VENDOR_FSC

void Get_FSC_Sensor(INT8U snrindex, INT8U polledval)
{
    bsp_bits_set(IO_CONFIG2, 0, BIT_SYSSH, 1);

    //fault = 5 means OK, 10 means warning, 20 means major failure
    //please refer to DASH spec.

    //status query
    DEBUGMSG(SENSOR_DEBUG, "Get %d status %x\n", snrindex, polledval);
    if (polledval == 0x80)
    {
        //fan not installed (available in the system)
        sensor[snrindex].exist = SNR_INVALID;
        sensor[snrindex].state = STATE_FATAL;
    }
    else if (sensor[snrindex].type == SNR_TACHOMETER)
    {
        if (((polledval & 0x04) == 0x04) )
        {
            //if it fautls, still need to read its value
            //since it exists before

            //should assert event
            sensor[snrindex].exist = SNR_READ;
            sensor[snrindex].state = STATE_CRITICAL;
            sensor[snrindex].fault = 20;

        }
        else if ((polledval & 0x0C) == 0x00)
        {
            sensor[snrindex].exist = SNR_READ;
            sensor[snrindex].state = STATE_NORMAL;
            sensor[snrindex].fault = 5;
            //should deassert event
        }
        else
        {
            sensor[snrindex].exist = SNR_FAULT;
            sensor[snrindex].state = STATE_CRITICAL;
        }
    }
    else
    {
        if ((polledval & 0x01) == 0x01)
        {
            //no fault, but may have temperature over problem
            //should assert event

            sensor[snrindex].exist = SNR_READ;
            if((polledval & 0x03) == 0x03)
            {
                sensor[snrindex].state = STATE_WARNING;
                sensor[snrindex].fault = 10;
            }
            else
            {
                sensor[snrindex].state = STATE_NORMAL;
                sensor[snrindex].fault = 5;
            }
        }
        else if ((polledval & 0x01) == 0x00)
        {

            //should assert event, set event = 1
            sensor[snrindex].exist = SNR_READ;
            sensor[snrindex].state = STATE_CRITICAL;
            sensor[snrindex].fault = 20;
        }
        else
        {
            sensor[snrindex].exist = SNR_FAULT;
            sensor[snrindex].state = STATE_CRITICAL;
        }

    }
}
INT8U Fan_Poll(INT8U fanindex, INT8U index)
{

    ///static INT8U fanindex = 0;
    INT8U ret = 0;
    INT8U *bptr = (INT8U*)(SMBMTxbuffStartAddr);

#if 0
    if (sensor[fanindex].pollindex == 2 )
    {
        //if sensor does not exist, go to next sensor
        sensor[fanindex].pollindex = 0;
        fanindex = (fanindex + 1) % (MAX_SENSOR_NUMS);
    }

    while (sensor[fanindex].exist == SNR_INVALID)
        fanindex = (fanindex + 1) % (MAX_SENSOR_NUMS);

#endif
    *bptr = 0xE6;

    *(bptr+1) = sensor[fanindex].offset[index];
    smbiosrmcpdataptr->sensor = &sensor[fanindex];

    *(bptr+2) = *bptr + 1;

    ret = master_send(BMC_POLL, bptr, 3, 0x06);

    return ret;

}
#endif
//Prepare Legacy Sensor Polling Message

INT8U SMBus_Prepare_LSPMsg()
{
    static INT8U tmp = 0;
    //INT8U *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U *bptr = smbmtxbuffer;//(INT8U*)(SMBMTxbuffStartAddr);
    INT8U maxnumofsnr;

    // need write cmd register tell SMBus Engine
    // push smbus receive byte protocol

#ifndef CONFIG_SENSOR_AGGREGATE
    memcpy(bptr, &asfconfig->asfalerts[tmp].address, 2);
    *(bptr+2) = asfconfig->asfalerts[tmp].address+1;
    maxnumofsnr = asfconfig->numofalerts;
#else
    memcpy(bptr, &asfconfig->legacysnr[tmp], 3);
    maxnumofsnr = asfconfig->numofsnr;
#endif

    if(maxnumofsnr == 0)
        return 1;

#if 0
    if (dpconf->chipset != INTEL && dpconf->chipset != LENOVO && dpconf->chipset != AMDSOC)
        //have no data about Nvidia
        return 1;

    //lenovo use different sensor addresses for different sensor types
    *bptr = smbus_param[dpconf->chipset][RCA_LS_ADDR0 + tmp *2];
    *(bptr+1) = smbus_param[dpconf->chipset][RCA_LS_CMD0 + tmp*2];
    *(bptr+2) = *bptr + 1;
#endif
    smbiosrmcpdataptr->SensorIndex = tmp;
    tmp = (tmp + 1) % maxnumofsnr;
    //tmp = (tmp+1) % 2;
    return master_send(LS_POLL, bptr, 3, 0x06);
}

#if 0
INT8U SMBus_dummy_send()
{
    INT8U *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U ret;

    *bptr++ = 0xFF;
    *bptr++ = 0xFF;
    ret = master_send(0xFF,bptr, 3,0xFF);
}
#endif

// Remote control Waveform to Chipset
INT8U SMBus_Prepare_RmtCtrl(INT8U MsgType, INT8U force)
{
    //INT8U *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U *bptr = smbmtxbuffer;
    INT8U ret = 0;
    static INT8U enter = 0;
    OS_CPU_SR cpu_sr = 0;
    INT8U state;

    //prevent re-entry, possibly caused by OSTimeDly
    if (enter)
        return DASH_FAIL;

    state = bsp_get_sstate();

    if (dpconf->vendor == FSC)
    {
        if ((MsgType == RMCP_PWR_Off || MsgType == RMCP_Reset || MsgType == RMCP_PWR_CR) && (state == S_S3 || state == S_S4 || state == S_S5))
            return DASH_FAIL;

        else if (MsgType == RMCP_PWR_On && state == S_S0)
            return DASH_FAIL;

        else if (state == S_UNKNOWN)
            return DASH_FAIL;
    }

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

    if (dpconf->vendor == EVERLIGHT)
    {
        INT8U gpio;
        if (MsgType == RMCP_PWR_On && state == S_S5)
        {
            if (!GPIOGet(&gpio))
                GPIOEnable();
            if (gpio)
            {
                GPIOSet(0);
                bsp_wait(1000);
                GPIOSet(1);
            }
            OS_ENTER_CRITICAL();
            enter = 0;
            OS_EXIT_CRITICAL();
            return ret;
        }
    }

    //in band only exists in S0
    //graceful shutdown through client tool
    if (bsp_IODrvAP_exist() && !force)
    {
        DEBUGMSG(MESSAGE_DEBUG, "IB Exist\n");
        DEBUGMSG(MESSAGE_DEBUG, "0x11 is %x\n", REG8(IOREG_IOBASE + 0x11));
        if (MsgType == RMCP_PWR_Off)
            bsp_bits_set_mutex(MAC_SYNC1, IB_Shutdown, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_HIBER) //hibernate
            bsp_bits_set_mutex(MAC_SYNC1, IB_Hibernate, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_STANDBY)
            bsp_bits_set_mutex(MAC_SYNC1, IB_Sleep, BIT_RMTCTL, 3);
        else
            bsp_bits_set_mutex(MAC_SYNC1, IB_Reset, BIT_RMTCTL, 3);

        //timeout is 10 secs
        //OSTimeDly(10*OS_TICKS_PER_SEC);
        smbiosrmcpdataptr->IBRmtl = 1;
        smbiosrmcpdataptr->IBRmtlCmd = MsgType;
        OS_ENTER_CRITICAL();
        enter = 0;
        OS_EXIT_CRITICAL();
        return ret;

        /*
            if (!bsp_bits_get(IO_CONFIG2, BIT_RMTCTL, 3))
            {
                OS_ENTER_CRITICAL();
                enter = 0;
                OS_EXIT_CRITICAL();
                return ret;
            }
        */

    }

    //in-band does not exist, but issue a hibernate or standby
    switch(MsgType)
    {
    case RMCP_Reset:
        MsgType = ASF_RESET;
        break;

    case RMCP_PWR_On:
        MsgType = ASF_POWER_ON;
        break;

    case RMCP_PWR_Off:
    case RMCP_HIBER:
    case RMCP_STANDBY:
        MsgType = ASF_POWER_OFF;
        break;

    case RMCP_PWR_CR:
        MsgType = ASF_POWER_CR;
        break;
    }
    memcpy(bptr, &asfconfig->asf_rctl[MsgType].slaveaddr, 3);

    ret = master_send(REMOTE_CONTROL,bptr,3,0x04);

    //alwyas clear the bits used to inform IB
    //bsp_bits_set(IO_CONFIG2, 0x0, BIT_RMTCTL, 3);
    bsp_bits_set_mutex(MAC_SYNC1, 0x0, BIT_RMTCTL, 3);

    OS_ENTER_CRITICAL();
    enter = 0;
    OS_EXIT_CRITICAL();

    if (ret) ret = DASH_FAIL;
    return ret;

}

// Return Boot Options Response and No Boot Options Response
#if (CONFIG_VERSION  > IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
INT8U *SMBus_Prepare_BootOptions()
{
    //INT8U *bptr = (INT8U*)(SMBTxbuffStartAddr);
    INT8U *bptr = smbtxbuffer;//(INT8U*)(SMBTxbuffStartAddr);

    if (smbiosrmcpdataptr->bootopt[2] == 0x11)
    {
        *bptr++ = sizeof(smbiosrmcpdataptr->bootopt) + 2;
        *bptr++ = 0x16; //subcmd-return boot option
    }
    else
    {
        *bptr++ = 0x02;
        *bptr++ = 0x17; //subcmd-no return boot option
    }

    *bptr++ = 0x10; //VersionNumner
    if (smbiosrmcpdataptr->bootopt[2] == 0x11)
    {
        memcpy(bptr, smbiosrmcpdataptr->bootopt, sizeof(smbiosrmcpdataptr->bootopt));
        //*(bptr+sizeof(data->bootopt)) = 0x38;
        smbus_send(NULL, sizeof(smbiosrmcpdataptr->bootopt)+3);
    }
    else
    {
        smbus_send(NULL, 3);
    }

    //return (INT8U*)(SMBTxbuffStartAddr);
    return (INT8U*) smbtxbuffer;

}
#endif

INT8U SMBus_Check_UDID(INT8U *udid)
{
    return memcmp(RTKUDID+1, udid, 16);
}

INT8U *SMBus_Get_UDID()
{
    INT8U *bptr = smbtxbuffer;//(INT8U*)(SMBTxbuffStartAddr);

    memcpy(bptr, RTKUDID, 18);

#if CONFIG_VERSION  > 1
    bptr[2] += 1;
#endif

#if CONFIG_VERSION  > 4
    bptr[2] += 1;
#endif

    bptr[17] =  dpconf->arpaddr + 1;

    smbus_send(NULL, bptr[0]+3);

    //return (INT8U*)(SMBTxbuffStartAddr);
    return smbtxbuffer;//(INT8U*)(SMBTxbuffStartAddr);

}

#if 0
inline void SMBus_Prepare_Ack(INT8U cmdcode)
{
    INT8U *bptr = (INT8U*)(SMBTxbuffStartAddr);

    if (cmdcode == 5)
    {
        *bptr = ReturnValue_Yes;
        //*(bptr+1) = 0x2E;
    }
    else if (cmdcode == 0)
    {
        *bptr = ReturnValue_No;
        //*(bptr+1) = 0x4B;
    }

    smbus_send(NULL, 1);

}
#endif
#if 0
inline void SMBus_DTP_Message()
{
    INT8S *bptr = (INT8U*)(SMBTxbuffStartAddr);

    *bptr = 0x03;
    *(bptr+1) = DeviceTypePoll;
    *(bptr+2) = 0x10;

#if 0
    //only valid in revision F
    if (REG16(IOREG_IOBASE+MAC_IMR) & 0x0400)
        *(bptr+3) = 0x1d;
    else
        *(bptr+3) = 0x15;
#endif
    if (bsp_get_link_status())
        *(bptr+3) = 0x1d;
    else
        *(bptr+3) = 0x15;

    smbus_send(NULL, *bptr+1);
}
#endif
//Notify ARP Master Message
void SMBus_Prepare_NotifyARP(INT8S* Buffer_address, INT32S length)
{
    //INT32S *ptr;
    //INT32S i;
    INT8S *bptr;
    INT8S DATALow,DATAHigh;
    //smbrxdesc_t *txdesc;
    bptr = (INT8S*)(Buffer_address);

    DATALow = 0x00;
    DATAHigh = 0x00;

    // need write cmd register tell SMBus Engine push smbus receive byte protocol
    *bptr = SMBHost_ADDRESS;
    *(bptr+1) = ARP_ADDRESS;
    *(bptr+2) = DATALow;
    *(bptr+3) = DATAHigh;
    // PEC Value
    *(bptr+4) = 0x3c;
    smbus_send(NULL, 5);
}

#if (CONFIG_VERSION  > IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
void event_log_load()
{
    eventdata  *romptr, *event_ptr, *prev = NULL;
    //event_list *elist, *prev = NULL;
    INT8U ecount = 0;

    eventdataptr = malloc(2048);
    romptr =  (eventdata *) SYSTEM_EVENT_LOG_ROM;

    while (romptr->timestamp != 0xFFFFFFFF && romptr->timestamp != 0)
        //only check if a flash is empty, since PLDM may not be implemented
        //which would cause timestamp to zero
        //while (romptr->timestamp != 0xFFFFFFFF)
    {
        ecount++;
        romptr++;
    }

    //prevent overrun condition, when the configuration is not updated
    if (ecount > MAX_EVENT_ENTRY)
        ecount = MAX_EVENT_ENTRY;

    while (ecount--)
    {
        romptr--;

        event_ptr = eventdataptr + data_index;
        //eptr = event_ptr;
        data_index = (data_index + 1) % MAX_EVENT_ENTRY;

        memcpy(event_ptr, romptr, sizeof(*romptr));

        if (romptr == (eventdata *) SYSTEM_EVENT_LOG_ROM)
            event_head = event_ptr;

        event_ptr->next = prev;
        prev = event_ptr;

#if 0
        elist = &event_list_entry[event_index];
        event_index = (event_index + 1) % MAX_EVENT_ENTRY;

        memset(elist, 0, sizeof(event_list));
        elist->addr = event_ptr;

        if (romptr == (eventdata *) SYSTEM_EVENT_LOG_ROM)
            event_head = elist;

        //backward link
        elist->next = prev;
        prev = elist;
#endif

        if (dirty[EVENTTBL].length < MAX_EVENT_ENTRY)
            dirty[EVENTTBL].length++;
        else
            dirty[EVENTTBL].length = MAX_EVENT_ENTRY;
    }

    dirty[EVENTTBL].flashaddr = (INT8U *) SYSTEM_EVENT_LOG_ROM;

}
#endif

eventdata *event_log(INT8U *addr, INT8U byte)
{
    static INT8U recall = 1;
    eventdata *eptr, *recalllist, *nextlist;
    ///event_list *elist,*recalllist;
    ///event_list *nextelist;
    INT8U i = 1;

    //if(*addr == 0) //means event sensor type == 0, which is impossible
    //    return NULL;
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    eptr = eventdataptr + data_index;
    data_index = (data_index + 1) % MAX_EVENT_ENTRY;
    nextlist = eventdataptr + data_index;
    OS_EXIT_CRITICAL();

    memset(eptr, 0, sizeof(eventdata));
    eptr->timestamp = time(0);
    memcpy(&eptr->Event_Sensor_Type, addr, byte);

    if (eptr->timestamp == 0)
        recall = 1;

    if (nextlist->next == eptr)
        nextlist->next = NULL;

    OS_ENTER_CRITICAL();
    eptr->next = event_head;
    event_head = eptr;
    OS_EXIT_CRITICAL();
#if 0
    OS_ENTER_CRITICAL();
    elist = &event_list_entry[event_index];
    event_index = (event_index + 1) % MAX_EVENT_ENTRY;
    nextelist = &event_list_entry[event_index];
    OS_EXIT_CRITICAL();
    memset(elist, 0, sizeof(event_list));

    //prevent cyclic link
    if (nextelist->next == elist)
        nextelist->next = NULL;

    OS_ENTER_CRITICAL();
    elist->next = event_head;
    elist->addr = eptr;
    event_head = elist;
    OS_EXIT_CRITICAL();
#endif

    //recall these events, happened before timestamp
    //no need to recall
    if (recall && eptr->timestamp != 0)
    {
        recalllist = eptr->next;
        while (recalllist && recalllist->timestamp == 0)
        {
            recalllist->timestamp = eptr->timestamp - i++;
            recalllist = recalllist->next;
            dirty[EVENTTBL].length++;
            //recall == 0 would check the boundary again
        }
        recall = 0;
    }

    //not write back timestamp 0 to flash
    if (!recall)
    {
        if (dirty[EVENTTBL].length < MAX_EVENT_ENTRY)
            dirty[EVENTTBL].length++;
        else
            dirty[EVENTTBL].length = MAX_EVENT_ENTRY;

        setdirty(EVENTTBL);
    }

#if 0
    //determine its log type
    if (eptr->Event_Sensor_Type == 0x25)
        eptr->logtype = LOG_INFO;
    else if (eptr->Event_Sensor_Type == 0x0F)
    {
        if (eptr->EventData[1] == 0x07 || eptr->EventData[1] == 0x09 || eptr->EventData[1] == 0x01 || eptr->EventData[1] == 0x02)
            eptr->logtype = LOG_HW;
        else if (evt_ptr->EventData[1] == 0x04)
            eptr->logtype = LOG_AUTH;
        else
            eptr->logtype = LOG_INFO;

    }
#endif

    return eptr;

}

// 2008.04.01 tomadd
// ASF Sensor Polling Message: Get Event Status
INT8U SMBus_Prepare_GetEventStatus()//INT8S* Buffer_address, INT32S length)
{
    if (dpconf->chipset != NVIDIA)
        return 1;
    //must check the difference of data->status
    //if found at i, then set in *(bptr+5)
    //INT8S *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U *bptr = smbmtxbuffer;///(INT8U*)(SMBMTxbuffStartAddr);

    // need write cmd register tell SMBus Engine push smbus receive byte protocol
    //*bptr = smbus_param[dpconf->chipset][RCA_ADDR];//nVidia_RCA_Addr;
    *bptr = asfconfig->asf_rctl[0].slaveaddr;
    *(bptr+1) = SnsrSystemState;
    *(bptr+2) = 0x03; //WrByteCount
    *(bptr+3) = GetEventStatus;
    *(bptr+4) = 0x10;
    *(bptr+5) = 0x00;
    //*(bptr+6) = (smbus_param[dpconf->chipset][RCA_ADDR]+1);
    *(bptr+6) = asfconfig->asf_rctl[0].slaveaddr + 1;

    return master_send(ASF_SENSOR_POLL, bptr,7, 0x0b);
}

// ASF Sensor Polling Message: Get Event Data
INT8U SMBus_Prepare_GetEventData(INT8U index)
{
    //INT8S *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U *bptr = smbmtxbuffer;//(INT8U*)(SMBMTxbuffStartAddr);

    // need write cmd register tell SMBus Engine push smbus receive byte protocol
    //*bptr = smbus_param[dpconf->chipset][RCA_ADDR];//nVidia_RCA_Addr;
    *bptr = asfconfig->asf_rctl[0].slaveaddr;
    *(bptr+1) = SnsrSystemState;
    *(bptr+2) = 0x04; //WrByteCount
    *(bptr+3) = GetEventData;
    *(bptr+4) = 0x10;
    //from get event status return index value!
    //Event Status Index Value come from Get Event Status!!
    smbiosrmcpdataptr->Flag[index] = 0;
    *(bptr+5) = index;
    *(bptr+6) = 0x00;
    *(bptr+7) = asfconfig->asf_rctl[0].slaveaddr + 1;
    ///*(bptr+7) = (smbus_param[dpconf->chipset][RCA_ADDR]+1);

    return master_send(ASF_SENSOR_POLL, bptr, 8, 0x0b);
}


#if (CONFIG_VERSION  > IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
eventdata *eventGet()
{
    return event_head;
}
#endif

#if 0
void fill_rmttbl(INT8U state)
{
    if (state == S_S0)
    {
        strcpy(rmttbl[0], DASH_Reset);
        strcpy(rmttbl[1], DASH_Shutdown);
        if (bsp_AP_exist())
        {
            strcpy(rmttbl[2], DASH_Sleep);
            strcpy(rmttbl[3], DASH_Hibernate);
            strcpy(rmttbl[4], "0");
        }
        else
        {
            strcpy(rmttbl[2], DASH_PWR_CR);
            strcpy(rmttbl[3], "0");
        }
    }
    else if (state == S_S3 || state == S_S4 || state == S_S5)
    {
        strcpy(rmttbl[0], DASH_PWR_On);
        strcpy(rmttbl[1], "0");
    }
    else
    {
        strcpy(rmttbl[0], DASH_PWR_On);
        strcpy(rmttbl[1], DASH_Reset);
        strcpy(rmttbl[2], DASH_Shutdown);
        strcpy(rmttbl[3], DASH_PWR_CR);
        strcpy(rmttbl[4], "0");

    }


}
#endif

void setlogtype(eventdata *eptr)
{
    //determine its log type
    if (eptr->Event_Sensor_Type == 0x20)
        eptr->logtype = LOG_ERROR;
    else if (eptr->Event_Sensor_Type == 0x05)
        eptr->logtype = LOG_WARNING;
    else if (eptr->Event_Sensor_Type == 0x01 || eptr->Event_Sensor_Type == 0x02 || eptr->Event_Sensor_Type == 0x04)
    {
        if (eptr->Event_Severity == 0x01)
            eptr->logtype = LOG_WARNING;
        else
            eptr->logtype = LOG_ERROR;
    }
    else if (eptr->Event_Sensor_Type == 0x25)
        eptr->logtype = LOG_INFO;
    else if (eptr->Event_Sensor_Type == 0x0F)
    {
        if (eptr->Event_Offset == 0x00 || eptr->Event_Offset == 0x01)
            eptr->logtype = LOG_ERROR;
        else if ((eptr->EventData[1] >= 1 && eptr->EventData[1] <= 3) || (eptr->EventData[1] >= 6 && eptr->EventData[1] <= 17))
            eptr->logtype = LOG_HW;
        else if (eptr->EventData[1] == 0x04 || eptr->EventData[1] == 0x05)
            eptr->logtype = LOG_AUTH;
        else
            eptr->logtype = LOG_INFO;

    }
}
#ifdef CONFIG_CHIPSET_NVIDIA
void ASFsnrpolling()
{
    //1.5 secs timeout
    static INT16U count =  3 ;
    static INT8U asfidx = 0;

    if (fwsig->SIG == NORMAL_SIG)
    {
        smbiosrmcpdataptr->PollTimeOut++;
        if (smbiosrmcpdataptr->PollTimeOut ==  150)
            smbiosrmcpdataptr->PollType = POLL_NONE;

        if (dpconf->asfpoll)
        {

            if (((count % dpconf->polltime) == 0 ))
            {
                if (smbiosrmcpdataptr->PollType == POLL_NONE)
                {
                    if (!SMBus_Prepare_GetEventStatus())
                    {
                        smbiosrmcpdataptr->PollType = POLL_ASF_STATUS;
                        smbiosrmcpdataptr->PollTimeOut = 0;
                        asfidx = 0;
                    }
                }
            }

            if (smbiosrmcpdataptr->PollType == POLL_ASF_PEND)
            {
                if (smbiosrmcpdataptr->ASFFlag)
                {
                    SMBus_Prepare_GetEventData(smbiosrmcpdataptr->Flag[asfidx++]);
                    smbiosrmcpdataptr->PollType = POLL_ASF_DATA;
                    smbiosrmcpdataptr->PollTimeOut = 0;
                }
            }

            count++;
        }
    }
}
#endif

void smbus_process_watchdog(INT8U *ptr)
{

    eventdata wdtdata;

    if (ptr[2] == 0xc9) //PEC = 0xc9
        ;//Rese Alert Sending Device(NIC)
    else if (ptr[2] >0x0c && ptr[2] < 0x13)
    {
        if ( ptr[3] == StartWatchDog)
        {
            if ( ptr[4] ==0x10)
            {
                memset(&wdtdata, 0,  sizeof(eventdata));
                memcpy(&wdtdata.Event_Sensor_Type, ptr+7, ptr[2] - 4);
                wdtdata.timeout = (*(ptr+6) << 8 ) + *(ptr+5) + dpconf->hbtime;
                //need to add the hbtime, since the START TASK would
                //be wake up immediately
                wdtdata.interval = (ALERT_INTERVAL/(dpconf->numsent-1));
                wdtdata.alertnum = dpconf->numsent;
                wdtdata.watchdog = 1;
                OSQPost(EVTQ, &wdtdata);

                //default set to non-expired
                smbiosrmcpdataptr->expired = 0;
                //smbiosrmcpdataptr->alert = 3;
#if 0
                evt_ptr = event_log(addr+7, rmcp->slaveaddr-4);
                evt_ptr->timeout = (*(addr+6) << 8 ) + *(addr+5);
                evt_ptr->starttimer = 1;
                //evt_ptr->watchdog = 0x01; //watchdog timer event
                wdt_ptr = evt_ptr;
                OSQPost(EVTQ, evt_ptr);
                OSTimeDlyResume(TASK_START_PRIO);
#endif
            }
        }
    }
    else if (ptr[2] == 0x02)
    {
        if (ptr[3] == StopWatchDog)
        {
            if ( ptr[4] == 0x10)
            {
                //force the queue to stop push again
                wdtdata.alertnum = 0;
                wdtdata.watchdog = 0;
                // time unexpired and to fill
                // RMCP Msg System_State_Reponse.(Data2 = 0x01)
                // smbios_addr = (INT8U *) SMBIOS_START_ADDR;
                ///smbiosrmcpdataptr->stoptimer = 0x01;
                ///smbiosrmcpdataptr->expired = 0x00;
                ///if (wdt_ptr != NULL)
                ///    wdt_ptr->watchdog = 0x01; //not to send
            }
        }
    }
}

void smbus_process_alerts(INT8U *ptr)
{
    eventdata *evt_ptr;

    if (ptr[2] == UDID_LENGTH)
    {
        if (!memcmp(RTKUDID, ptr+3, UDID_LENGTH))
        {
            //SMBus ARP Protocol: SMBus Host Assign SMBus Slave Address
            //RTSAddr = *(addr+19);
            dpconf->arpaddr = *(ptr+19);
        }
    }
    else if (ptr[2] > 0x0a && ptr[2] < 0x11)
    {
        if (ptr[3] == PushAlertMsgWR)
        {
            if (ptr[4] == 0x10)
            {
                evt_ptr =  event_log(ptr+5, ptr[2]-2);
                setlogtype(evt_ptr);
                evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
                evt_ptr->alertnum = dpconf->numsent ;

                evt_ptr->timeout  =  0;
                OSQPost(EVTQ, evt_ptr);
            }
        }
        else if (ptr[3] == PushAlertMsgWoR)
        {
            if (ptr[4] == 0x10)
            {
                evt_ptr =  event_log(ptr+5, ptr[2]-2);
                setlogtype(evt_ptr);
                evt_ptr->timeout = 0;
                evt_ptr->alertnum = 1;
                OSQPost(EVTQ, evt_ptr);
            }
        }
    }
}

#endif

void SetSnrState(INT8U evtoffset)
{

    switch(evtoffset)
    {
    case 0x00:
        sensor[asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].sensorindex].state = STATE_LOWERWARNING;
        break;

    case 0x02:
        sensor[asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].sensorindex].state = STATE_LOWERCRITICAL;
        break;

    case 0x07:
        sensor[asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].sensorindex].state = STATE_UPPERWARNING;
        break;

    case 0x09:
        sensor[asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].sensorindex].state = STATE_UPPERCRITICAL;
        break;

    case 0xff:
        sensor[asfconfig->legacysnr[smbiosrmcpdataptr->SensorIndex].sensorindex].state = STATE_NORMAL;
        break;

    }
}

#if 0
//target is (0xE052) bit 0 , but 4-bytes alignment isuue. , must use 0xe050 bit16
INT8U CheckEEE(INT16U *macspeed,INT32U *eee)
{
	INT8U flag = 0;	
	INT16U ret16A,ret16A2;
	INT32U ret32B,ret32B2;
	#if 1	

	//AcquireIbOobMutex();

	
	flag = 0;
	ret16A = ReadIBReg(0xE0C0);	
	ret32B = ReadIBReg(0xE050);	

	printf("[RTK] 0x%x , 0x%x\n",ret16A,ret32B);

	*macspeed = ret16A;
	*eee = ret32B;
	
	if((ret16A & 0x0F07) != 0x00){
		ret16A2 = ret16A&0xF0F8;
		WriteIBReg(0xE0C0,ret16A2);
		flag += 1;
	}
	if((ret32B & 0x00010000  ) != 0x00){
		ret32B2 = ret32B & 0xFFFEFFFF;
		WriteIBReg(0xE052,ret32B2);
		flag += 1;
	}
	
	#endif

 	//ReleaseIbOobMutex();

	return flag;
	
}
#endif


void restoreEEE(INT16U macspeed,INT32U eee){
	INT16U a;
	INT32U b;

	//AcquireIbOobMutex();
	
	a= ReadIBReg(0xE0C0);
	b= ReadIBReg(0xE050);
	printf("[RTK] restore EEE , macspeed is =%x , eee is =%x\n",a,b);
	
	
	printf("[RTK] restore EEE , macspeed=%x , eee=%x\n",macspeed,eee);
	
	#if 1	
	WriteIBReg(0xE0C0,macspeed);
	WriteIBReg(0xE050,eee);
	#endif	

	//ReleaseIbOobMutex();

	
}

#if (CONFIG_VERSION >= IC_VERSION_EP_RevB) && defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)
INT8U master_check_patch()
{
    INT16U loop_count = 0;
    INT8U ret = 0;

    //0 means idle
    while (((REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x07)) && loop_count < 1000)
    {
        //OSTimeDly(1);//10ms
        bsp_wait(1000);//1000us if running at EP freq = 250Mhz (inner loop~0.5us)
        loop_count++;
    }

    if (loop_count == 1000)
        ret = 1;
    return ret;
}
#endif

#if 0
INT8U master_send(msendtype type, INT8U* bufaddr, INT8U length, int cmd)
{
    //no need to copy from bufaddr, since bufaddr is the smbtx buf addr
    INT8U val8, ret;
    INT8U count = 0;
    OS_CPU_SR cpu_sr = 0;
    smbrxdesc_t *txdesc = (smbrxdesc_t *) (SMBMTxdescStartAddr);
    static INT8U enter = 0;
    static INT8U *smbuslog = NULL;
    INT8U err;
	
    INT8U mFlag;
    INT16U macspeed=0;	
    INT32U eee=0;
	INT8U timeout;

printf("[RTK]+ master_send\n");


AcquireIbOobMutex();

mFlag = CheckEEE(&macspeed,&eee);
	
// using if(dpconf->pldmdbg) instead
// so that we can do the debug using ROM
    if(dpconf->pldmdbg)
    {
        if(smbuslog == NULL)
            smbuslog = malloc(128);
        memcpy(&smbuslog[2], bufaddr, length);
        if(cmd == 0x06 || cmd == 0x0b)
            smbuslog[0] = 0x40;
        else
            smbuslog[0] = 0x20;
        smbuslog[1] = length;
        err = OSQPost(SMBUSLOGQ, (void *) smbuslog);
    }

    if (enter){
	if(mFlag){
		restoreEEE(macspeed,eee);
	}
	
	 ReleaseIbOobMutex();
        return DASH_FAIL;
    }

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

	

    val8 = REG8(SMBUS_BASE_ADDR+SEIO_ISR);
    while ((val8 & 0x02))
    {
        OSTimeDly(1);
        if (count++ > 100){

		if(mFlag){
			restoreEEE(macspeed,eee);
		}

	     ReleaseIbOobMutex();
            return 1;
        }
    }

#if (CONFIG_VERSION >= IC_VERSION_EP_RevB) && defined(CONFIG_ENHANCE_SMB_SAMPLE_RATE)
    ret = master_check_patch();
#else
    ret = master_check();
#endif

    if (ret == 0)
    {
        if((type == PLDM_RESPONSE) && !dpconf->pldmfromasf)
        {

            if(dpconf->pldmpec)
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x02;
            else
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0xFD;
        }
        //using global pec setting,//for ASROCK intel platform @20140219
        else if (type == PLDM_REQUEST)
        {
            if(dpconf->snrpec)
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x02;
            else
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0xFD;

        }
        //using sensor related settings
        else
        {
            if(asfconfig->pec)
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= 0x02;
            else
                REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0xFD;

        }

        ///OS_ENTER_CRITICAL();
        //check again the status to avoid slave send happened at the same time
        if (!(REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x07))
        {
            if (type == BMC_POLL)
                bsp_bits_set(MAC_BIOSREG, 1, BIT_SYSSH, 1);
            //bsp_bits_set(MAC_BIOSREG, 1, BIT_SYSSH, 1);
            if (type == BMC_POLL && (bsp_get_sstate() != S_S0 || smbiosrmcpdataptr->PollType == POLL_STOP) )
            {
                bsp_bits_set(MAC_BIOSREG, 0, BIT_SYSSH, 1);
                //bsp_bits_set(MAC_BIOSREG, 0, BIT_SYSSH, 1);
                ret = 1;
            }
            else
            {
                txdesc->rx_buf_addr = (INT8U *) VA2PA(bufaddr);
                txdesc->st.length = length;
                txdesc->st.own = 1;
                REG8(SMBUS_BASE_ADDR+SEIO_ModeCmd) = cmd;
                REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) = 0x10;
            }
        }

        ///OS_EXIT_CRITICAL();
    }

    OS_ENTER_CRITICAL();
    enter = 0;
    OS_EXIT_CRITICAL();


	//OSTimeDly(200);
	//try REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn)  == 0x10 , not working , change to 0x00 immediately
	//try txdesc->st.own = 1 , not working , change to 0x00 immediately
	//Have to use delay 1s simply.
	
	//OSTimeDly(100);

	timeout = 50;		
	while((txdesc->st.length == length)  &&(timeout>0)){
		OSTimeDly(1);
		timeout--;
	}

	printf("[RTK]  SEIO_SMPollEn:(0x%x) ,txdesc->st.own=0x%x , len =%d\n",REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) ,txdesc->st.own,txdesc->st.length);

	
	if(timeout == 0){
		printf("[RTK] timeout  timeout timeout timeout \n");
	}else{
		printf("[RTK] timeout  [%d]\n",timeout);
	}
	
	if(mFlag){
		restoreEEE(macspeed,eee);
	}

    ReleaseIbOobMutex();
    return ret;
}
#endif

#if 0
INT8U SMBus_Prepare_RmtCtrl(INT8U MsgType, INT8U force)
{
    //INT8U *bptr = (INT8U*)(SMBMTxbuffStartAddr);
    INT8U *bptr = smbmtxbuffer;
    INT8U ret = 0;
    static INT8U enter = 0;
    OS_CPU_SR cpu_sr = 0;
    INT8U state;

#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
	gIsLanWake = 0;
#endif

    //prevent re-entry, possibly caused by OSTimeDly
    if (enter)
        return DASH_FAIL;

    state = bsp_get_sstate();

    if (dpconf->vendor == FSC)
    {
        if ((MsgType == RMCP_PWR_Off || MsgType == RMCP_Reset || MsgType == RMCP_PWR_CR) && (state == S_S3 || state == S_S4 || state == S_S5))
            return DASH_FAIL;

        else if (MsgType == RMCP_PWR_On && state == S_S0)
            return DASH_FAIL;

        else if (state == S_UNKNOWN)
            return DASH_FAIL;
    }

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

    if (dpconf->vendor == EVERLIGHT)
    {
        INT8U gpio;
        if (MsgType == RMCP_PWR_On && state == S_S5)
        {
            if (!GPIOGet(&gpio))
                GPIOEnable();
            if (gpio)
            {
                GPIOSet(0);
                bsp_wait(1000);
                GPIOSet(1);
            }
            OS_ENTER_CRITICAL();
            enter = 0;
            OS_EXIT_CRITICAL();
            return ret;
        }
    }

    //in band only exists in S0
    //graceful shutdown through client tool
    if (bsp_IODrvAP_exist() && !force)
    {
        DEBUGMSG(MESSAGE_DEBUG, "IB Exist\n");
        DEBUGMSG(MESSAGE_DEBUG, "0x11 is %x\n", REG8(IOREG_IOBASE + 0x11));

        ret = DASH_SUCCESS;

		
        if (MsgType == RMCP_PWR_Off)
            bsp_bits_set_mutex(MAC_SYNC1, IB_Shutdown, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_HIBER) //hibernate
            bsp_bits_set_mutex(MAC_SYNC1, IB_Hibernate, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_STANDBY)
            bsp_bits_set_mutex(MAC_SYNC1, IB_Sleep, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_Reset)
            bsp_bits_set_mutex(MAC_SYNC1, IB_Reset, BIT_RMTCTL, 3);
        else if (MsgType == RMCP_PWR_CR)	
	  bsp_bits_set_mutex(MAC_SYNC1, IB_Reset, BIT_RMTCTL, 3);			
        else{
	ret = DASH_FAIL;		
        }
        	

        //timeout is 10 secs
        //OSTimeDly(10*OS_TICKS_PER_SEC);
        smbiosrmcpdataptr->IBRmtl = 1;
        smbiosrmcpdataptr->IBRmtlCmd = MsgType;
        OS_ENTER_CRITICAL();
        enter = 0;
        OS_EXIT_CRITICAL();
        return ret;

        /*
            if (!bsp_bits_get(IO_CONFIG2, BIT_RMTCTL, 3))
            {
                OS_ENTER_CRITICAL();
                enter = 0;
                OS_EXIT_CRITICAL();
                return ret;
            }
        */

    }

    //in-band does not exist, but issue a hibernate or standby
    switch(MsgType)
    {
    case RMCP_Reset:
        MsgType = ASF_RESET;
        break;

    case RMCP_PWR_On:
        MsgType = ASF_POWER_ON;
        break;

    case RMCP_PWR_Off:
    case RMCP_HIBER:
    case RMCP_STANDBY:
        MsgType = ASF_POWER_OFF;
        break;

    case RMCP_PWR_CR:
        MsgType = ASF_POWER_CR;
        break;
    }
    memcpy(bptr, &asfconfig->asf_rctl[MsgType].slaveaddr, 3);



	#if CONFIG_LANWAKEUP_ENABLED //This is special for HP NoteBook Only	
	if(MsgType == ASF_POWER_ON){ 
			
		bsp_bits_set(MAC_LAN_WAKE , 1,BIT_LANWAKE_IB_DIS,1);//set DIS_IB_WAKE to 1
		bsp_bits_set(MAC_LAN_WAKE , 1,BIT_OOB_LANWAKE,1);//set OOB_PINLANWAKE 1
		bsp_bits_set(MAC_LAN_WAKE , 0,BIT_OOB_LANWAKE,1);//set OOB_PINLANWAKE 0
		bsp_bits_set(MAC_LAN_WAKE , 0,BIT_LANWAKE_IB_DIS,1);//set DIS_IB_WAKE to 0

		bsp_bits_set(MAC_OOB_LANWAKE , 1,BIT_OOBLANWAKE,1);//set BIT_OOBLANWAKE to 1
#ifdef CONFIG_LANWAKE_NOTIFY_PCIE
		gIsLanWake = 1;
	}else{
		gIsLanWake = 0;
#endif
	}
	#endif

if(bsp_get_sstate() == S_S0){
	if(MsgType == ASF_POWER_ON){	
		ret = DASH_FAIL; 
	}else{
		ret = master_send(REMOTE_CONTROL,bptr,3,0x04);
	}
}else{
	if((MsgType == ASF_POWER_ON) ||(MsgType == ASF_POWER_CR) ){	
		ret = master_send(REMOTE_CONTROL,bptr,3,0x04);
	}else{
		ret = DASH_FAIL; 
	}
}

    //alwyas clear the bits used to inform IB
    //bsp_bits_set(IO_CONFIG2, 0x0, BIT_RMTCTL, 3);
    bsp_bits_set_mutex(MAC_SYNC1, 0x0, BIT_RMTCTL, 3);

    OS_ENTER_CRITICAL();
    enter = 0;
    OS_EXIT_CRITICAL();
	

    if (ret) ret = DASH_FAIL;
    return ret;

}
#endif

smbus_config_t *smbusconfig[2];
void slaveMode_PLDM_ASFdataFill(INT8U smbus_port)
{
	INT8U cnt=0;
	volatile INT32U BASE_ADDR = 0;
	
	BASE_ADDR=(smbus_port==0)?SMBUS0_IOBASE:SMBUS1_IOBASE;

	if(smbus_port==0)// smbus0
	{
		for(cnt=0;cnt<8*4;cnt++)// 4bytes per data
		{
			REG8(BASE_ADDR + SEIO_PLDMdata1 + cnt)=cnt+1;
			REG8(BASE_ADDR + SEIO_PLDMdata2 + cnt)=32-cnt;
		}
		cnt=0;
		for(cnt=0;cnt<13;cnt++)
			REG8(BASE_ADDR + SEIO_ASF_bootOptData + cnt)=128-cnt;
	}
	else// smbus1
	{
		for(cnt=0;cnt<8*4;cnt++)// 4bytes per data
		{
			REG8(BASE_ADDR + SEIO_PLDMdata1 + cnt)=32-cnt;
			REG8(BASE_ADDR + SEIO_PLDMdata2 + cnt)=cnt+1;
		}
		cnt=0;
		for(;cnt<13;cnt++)
			REG8(BASE_ADDR + SEIO_ASF_bootOptData + cnt)=255-cnt;
	}
}

inline void smbus_slave_send(INT8U *bufaddr, INT8U length, INT8U smbus_port)
{
	//INT32U BASE_ADDR = (0==smbus_port) ? SMBUS0_IOBASE:SMBUS1_IOBASE;
	INT32U BASE_ADDR = SMBUS0_IOBASE;
	smbrxdesc_t *smbtxdesc;
	
	smbtxdesc = (smbrxdesc_t *)PA2VA(REG32(BASE_ADDR+SEIO_TxDesc));

	//if (bufaddr != NULL)
		smbtxdesc->rx_buf_addr = (INT8U *) VA2PA(bufaddr);
	smbtxdesc->st.own = 1;
	smbtxdesc->st.length = length;

	REG8(BASE_ADDR+SEIO_SMPollEn) = SMEN_SMB_LSN_RST|SMEN_SMB_STRANS_PS;
}

INT8U smbus_master_send(msendtype type, INT8U* bufaddr, INT8U length, INT8U cmd, INT8U smbus_port)
{
    INT8U ret, count = 0;
    OS_CPU_SR cpu_sr = 0;
    smbrxdesc_t *txdesc = (smbrxdesc_t *) (SMBMTxdescStartAddr);
    static INT8U enter = 0;

    if (enter)
        return DASH_FAIL;

    OS_ENTER_CRITICAL();
    enter = 1;
    OS_EXIT_CRITICAL();

    while (REG8(SMBUS_BASE_ADDR+SEIO_ISR) & ISR_TX_INT)
    {
		OSTimeDly(1);
		if (count++ > 100){
	    	return 1;
		}
    }

    ret = master_check();

    if (ret == 0)
    {
        if(smbusconfig[smbus_port]->master_pec_enable)
            REG8(SMBUS_BASE_ADDR + SEIO_SMEn) |= SMEN_SMBUS_M_PEC_EN;
        else
            REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= ~(SMEN_SMBUS_M_PEC_EN);

        if (!(REG8(SMBUS_BASE_ADDR+SEIO_Status)&0x07))
        {
            txdesc->rx_buf_addr = (INT8U *) VA2PA(bufaddr);
            txdesc->st.length = length;
            txdesc->st.own = 1;
            REG8(SMBUS_BASE_ADDR+SEIO_ModeCmd) = cmd;
            REG8(SMBUS_BASE_ADDR+SEIO_SMPollEn) = 0x10;
        }
    }

    OS_ENTER_CRITICAL();
    enter = 0;
    OS_EXIT_CRITICAL();

    return ret;
}


void SMBUSRXFree(smbrxdesc_t *rxdesc, INT8U smbus_port)
{
    rxdesc->st.length = smbusconfig[smbus_port]->desc_rxsize;
    rxdesc->st.own = 1;
}
INT32U SMBDAT_Fail=0;
INT32U SMBUSINT=0;		//record INT times
INT8U SMBRxDescIdx[2]={0,0};

void bsp_smbus_isr(void)
{
	volatile INT8U cnt=0;
	volatile INT8U  smbus_port;
	INT32U BASE_ADDR = 0, SendLen;
	volatile INT8U  *Rx_Descidx_ptr;
	volatile INT8U reg_isr,  reg_imr;
	volatile INT8U *rx_buf, *tx_buf;
	volatile smbrxdesc_t *smbrxdesc, *smbtxdesc;
	
	smbus_port=0;
	BASE_ADDR=SMBUS0_IOBASE;

	for(smbus_port=0;smbus_port<2;smbus_port++)
	{
		BASE_ADDR=(smbus_port==0)?SMBUS0_IOBASE:SMBUS1_IOBASE;

		reg_imr=REG8(BASE_ADDR+SEIO_IMR);//SEIO_IMR
		//turn off IMR
		REG8(BASE_ADDR+SEIO_IMR) = 0x00;	

		//read ISR
		reg_isr= REG8(BASE_ADDR+SEIO_ISR);
		//clear ISR
		REG8(BASE_ADDR+SEIO_ISR) = reg_isr;		

		if(reg_isr & (ISR_RX_RB_INT|ISR_RX_NS_INT))// for smbus0 INT
		{			
			Rx_Descidx_ptr=&SMBRxDescIdx[smbus_port];
		}
		else{
			REG8(BASE_ADDR+SEIO_IMR) = reg_imr;
			return;//break;
		}
		//-------------------------------------------------------
		//=======================================================
		 
		SMBUSINT++;
		//smbrxdesc = (smbrxdesc_t *) (PA2VA(REG32(BASE_ADDR+SEIO_RxDesc)) + *Rx_Descidx_ptr);
		smbrxdesc = (smbrxdesc_t *) (smbusconfig[smbus_port]->rxdesc + *Rx_Descidx_ptr);
		
		smbtxdesc = (smbrxdesc_t *) PA2VA(REG32(BASE_ADDR+SEIO_TxDesc));
		tx_buf = (INT8U *)PA2VA(smbtxdesc->rx_buf_addr);

		while((smbrxdesc->st.own==0)&&(smbrxdesc->st.length!=0))
		{
			rx_buf = (INT8U *)PA2VA(smbrxdesc->rx_buf_addr);

			if(rx_buf[0]==(smbusconfig[smbus_port]->slave_addr<<1))
			{		
				if(rx_buf[1] == 0xF1)//pldm
				{
					REG8(BASE_ADDR+SEIO_SMB_BRautoReply_EN)=0x01;//Enable auto reply
					for(cnt=0;cnt<8*4;cnt++)// 4bytes per data
					{
						REG8(BASE_ADDR + SEIO_PLDMdata1 + cnt)=rx_buf[cnt+3];
					}
					REG8(BASE_ADDR+SEIO_SMB_BR_DataRdy) = 0x20 | 0x01;// default
					REG8(BASE_ADDR+SEIO_PLDMdata1_ByteCnt) = rx_buf[2];
				}
				else if(rx_buf[1] == 0xF2)//UGMAC & FT test write
	        	{
					REG8(BASE_ADDR+SEIO_SMB_BRautoReply_EN)=0x00;//Disable auto reply
    				tx_buf[0] = rx_buf[2];//copy bytecount to slavetxaddr
    				SendLen=tx_buf[0];
					tx_buf[1]=rx_buf[3];
					for(cnt=1;cnt<SendLen;cnt++)
					{
						*(tx_buf+cnt+1)=*(rx_buf+cnt+3)^rx_buf[3];
					}
	        	}
				else if(rx_buf[1] == 0xf3)//UGMAC & FT test read
	        	{
					if(rx_buf[2]==(smbusconfig[smbus_port]->slave_addr<<1|1))
					{
						smbus_slave_send(tx_buf, SendLen+1,smbus_port);
					}
	        	}
				else{
					REG8(BASE_ADDR+SEIO_SMB_BRautoReply_EN)=0x0;//Disable auto reply

					if(rx_buf[2]==(smbusconfig[smbus_port]->slave_addr<<1|1))
					{
						SMBus_Prepare_BootOptions();		
					}
					else
					{
						SMBDAT_Fail++;
					}		 
				}
			}		 
			else
			{
				SMBDAT_Fail++;
			}	
			SMBUSRXFree(smbrxdesc, smbus_port);
			*Rx_Descidx_ptr = ( *Rx_Descidx_ptr + 1 )% (smbusconfig[smbus_port]->desc_num);
			smbrxdesc++;
		};
		REG8(BASE_ADDR+SEIO_IMR) = reg_imr;//Enable INT
	}
}

INT8U module_inited[2]={0,0};

void smbus_variable_init(INT8U smbus_port)
{	

	if(module_inited[smbus_port]==1)
		return;
	
	smbusconfig[smbus_port] = malloc(sizeof(smbus_config_t));
	smbusconfig[smbus_port]->freq_div=1;
	smbusconfig[smbus_port]->freq_slave_div=2;
	smbusconfig[smbus_port]->master_pec_enable=0;
	smbusconfig[smbus_port]->slave_pec_enable=0;
	smbusconfig[smbus_port]->desc_num=16;
	smbusconfig[smbus_port]->desc_rxsize=76;
	if(smbus_port==0)
		smbusconfig[smbus_port]->slave_addr=0x64;
	else//smbus_port==1
		smbusconfig[smbus_port]->slave_addr=0x62;
	#if 0
	smbusconfig[smbus_port]->rxdesc = malloc(16*(smbusconfig[smbus_port]->desc_num));
	smbusconfig[smbus_port]->txdesc = malloc(16);
	smbusconfig[smbus_port]->mtxdesc = malloc(16);
	smbusconfig[smbus_port]->rxdesc->rx_buf_addr= (INT8U *) VA2PA(malloc(smbusconfig[smbus_port]->desc_rxsize*(smbusconfig[smbus_port]->desc_num)));
	smbusconfig[smbus_port]->txdesc->rx_buf_addr = (INT8U *) VA2PA(malloc(smbusconfig[smbus_port]->desc_rxsize));
	smbusconfig[smbus_port]->mtxdesc->rx_buf_addr = (INT8U *) VA2PA(malloc(smbusconfig[smbus_port]->desc_rxsize));
	#else
	smbusconfig[smbus_port]->txdesc = SMB0_TxdescStartAddr;
	smbusconfig[smbus_port]->mtxdesc = SMB0_MTxdescStartAddr; 	
	smbusconfig[smbus_port]->rxdesc= SMB0_RxdescStartAddr;
	smbusconfig[smbus_port]->rxdesc->rx_buf_addr= (INT8U *) VA2PA(SMB0_RxbuffStartAddr);
	smbusconfig[smbus_port]->txdesc->rx_buf_addr = (INT8U *) VA2PA(SMB0_TxbuffStartAddr);
	smbusconfig[smbus_port]->mtxdesc->rx_buf_addr = (INT8U *) VA2PA(SMB0_MTxbuffStartAddr);
	#endif

	module_inited[smbus_port]=1;
}
void smbus_variable_deinit(INT8U smbus_port)
{
	if(module_inited[smbus_port]==1)
	{
		if(smbusconfig[smbus_port]->mtxdesc->rx_buf_addr)
			free(smbusconfig[smbus_port]->mtxdesc->rx_buf_addr);
		if(smbusconfig[smbus_port]->txdesc->rx_buf_addr)
			free(smbusconfig[smbus_port]->txdesc->rx_buf_addr);
		if(smbusconfig[smbus_port]->rxdesc->rx_buf_addr)
			free(smbusconfig[smbus_port]->rxdesc->rx_buf_addr);
		module_inited[smbus_port]=0;
	}
}

void smbus_engine_reset(INT8U smbus_port)
{
	volatile INT32U BASE_ADDR = 0;
	volatile INT8U reg_val;
	
	BASE_ADDR=(smbus_port==0)?SMBUS0_IOBASE:SMBUS1_IOBASE;

	REG8(BASE_ADDR + SEIO_Status) = 0x00;
	REG8(BASE_ADDR + SEIO_SMEn) = 0x00;

	REG8(BASE_ADDR+SEIO_SMPollEn) = SMEN_SMB_FRST|SMEN_SMB_RAND_RST|SMEN_SMB_LSN_RST;

	do
	{
	    bsp_wait(500);
		reg_val = REG8(BASE_ADDR+SEIO_SMPollEn);
	}while(reg_val!=0);
}

void smbus_engine_init(INT8U smbus_port)
{
	volatile INT32U BASE_ADDR = 0, i;
	
	BASE_ADDR=(smbus_port==0)?SMBUS0_IOBASE:SMBUS1_IOBASE;
	
	smbrxdesc_t *smbrxdesc, *smbtxdesc, *smbmtxdesc;
	
	smbus_engine_reset(smbus_port);

	REG8(BASE_ADDR + SEIO_SMEn) = SMEN_MASTER_EN|SMEN_SLAVE_EN|\
										SMEN_SMBUSALERT_N|\
										(smbusconfig[smbus_port]->master_pec_enable<<2)|\
										(smbusconfig[smbus_port]->slave_pec_enable<<1)|\
										SMEN_SMBUS_EN;
	
	REG32(BASE_ADDR + SEIO_SCTimeOut) = 0x03186a00;

	#if 0
	REG32(BASE_ADDR + SEIO_SlaveAddr)=(INT32U)(smbusconfig[smbus_port]->slave_addr<<1);
	REG32(BASE_ADDR + SEIO_SlaveAddr1)=0;
	#else
	for(i=0;i<8;i++)
	{
		REG8(BASE_ADDR + SEIO_SlaveAddr+i)=(INT32U)(smbusconfig[smbus_port]->slave_addr<<1);
	}
	#endif

	REG16(BASE_ADDR + SEIO_BusFree) = (0x03E8 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_HoldTimeRStart_1) = (0x03E8 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_RS_SetupTime_1) = (0x03E8 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_StopSetupTime_1) = (0x03E8 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_DataHoldTime) = (0x03E / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_MasterClkLow) = (0x0768 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_MasterClkHigh) = (0x0768 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_MasterRBControl) = (0x03B4 / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_MasterTDControl) = (0x03B4 / smbusconfig[smbus_port]->freq_div);

    REG16(BASE_ADDR + SEIO_SlaveTimingCtrl) = (0x03B4 / smbusconfig[smbus_port]->freq_slave_div);
    REG16(BASE_ADDR + SEIO_SlaveLDTimingCtrl) = (0x003F / smbusconfig[smbus_port]->freq_slave_div);
    REG16(BASE_ADDR + SEIO_SlaveSMBCFallTime) = (0x0030 / smbusconfig[smbus_port]->freq_slave_div);
    REG16(BASE_ADDR + SEIO_SlaveSMBCRiseTime) = (0x0090 / smbusconfig[smbus_port]->freq_slave_div);
    REG16(BASE_ADDR + SEIO_SlaveSMBDFallTime) = (0x0030 / smbusconfig[smbus_port]->freq_slave_div);
    REG16(BASE_ADDR + SEIO_SlaveSMBDRiseTime) = (0x0090 / smbusconfig[smbus_port]->freq_slave_div);
	
	REG16(BASE_ADDR + SEIO_HoldTimeRStart_2) = (0x00FE / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_RS_SetupTime_2) = (0x00FE / smbusconfig[smbus_port]->freq_div);
	REG16(BASE_ADDR + SEIO_StopSetupTime_2) = (0x00FE / smbusconfig[smbus_port]->freq_div);
	REG8(BASE_ADDR + SEIO_SMBCFallTime) = 0x02;
	REG8(BASE_ADDR + SEIO_SMBCRiseTime) = 0x02;
	REG8(BASE_ADDR + SEIO_SMBDFallTime) = 0x02;
	REG8(BASE_ADDR + SEIO_SMBDRiseTime) = 0x02;
	

	//slave rx
	smbrxdesc = smbusconfig[smbus_port]->rxdesc;
	for (i = 0; i < smbusconfig[smbus_port]->desc_num; i++)
	{
		smbrxdesc->st.length = smbusconfig[smbus_port]->desc_rxsize;
		//total desc_num of slave rx buffer
		smbrxdesc->rx_buf_addr = (INT8U *) (VA2PA(smbusconfig[smbus_port]->rxdesc->rx_buf_addr) + i* (smbrxdesc->st.length));
		if (i == smbusconfig[smbus_port]->desc_num-1)
			smbrxdesc->st.eor  = 1;
		else
			smbrxdesc->st.eor  = 0;
		smbrxdesc->st.own = 1;//ready for rx
		smbrxdesc++;
	}
        
	REG8(BASE_ADDR+SEIO_SMB_BRautoReply_EN)=0x00;//disable auto reply

	//only one slave tx buffer
	smbtxdesc = smbusconfig[smbus_port]->txdesc;
	//smbtxdesc->st.length = smbusconfig[smbus_port]->desc_rxsize;
	//smbtxdesc->st.eor  = 1;
	//smbtxdesc->st.own = 0;// not ready for tx

	//only one master tx buffer
	smbmtxdesc = smbusconfig[smbus_port]->mtxdesc;
	//smbmtxdesc->st.length = smbusconfig[smbus_port]->desc_rxsize;
	//smbmtxdesc->st.eor  = 1;
	//smbmtxdesc->st.own = 0;// not ready for tx

	REG32(BASE_ADDR + SEIO_RxDesc) = VA2PA(smbusconfig[smbus_port]->rxdesc);
	REG32(BASE_ADDR + SEIO_TxDesc) = VA2PA(smbusconfig[smbus_port]->txdesc);
	REG32(BASE_ADDR + SEIO_MTxDesc) = VA2PA(smbusconfig[smbus_port]->mtxdesc);
	REG32(BASE_ADDR + SEIO_RxDescOnListenM) = VA2PA(smbusconfig[smbus_port]->rxdesc);

	slaveMode_PLDM_ASFdataFill(0);
	REG8(BASE_ADDR+SEIO_SMB_BR_DataRdy) = 0x20;// default
	REG8(BASE_ADDR+SEIO_PLDMdata1_ByteCnt) = 0x00;
	REG8(BASE_ADDR+SEIO_PLDMdata2_ByteCnt) = 0x00;
	REG8(BASE_ADDR+SEIO_ASF_bootOpt_ByteCnt) = 0x02;// default

	REG16(BASE_ADDR+SEIO_SMB_norm_TxD) = 0x4321;

	//REG8(BASE_ADDR+SEIO_IMR) = ISR_RX_RDU_INT|ISR_RX_RB_INT|ISR_RX_NS_INT;
	REG8(BASE_ADDR+SEIO_IMR) = ISR_RX_RB_INT|ISR_RX_NS_INT;

	rlx_irq_register(BSP_SMBUS_IRQ, bsp_smbus_isr);//smbus0, smbus1 use same isr
}
void SMBUS_Test_Task(void)
{


	while(OS_TRUE)
	{
			OSTimeDly(OS_TICKS_PER_SEC);
	}
}

