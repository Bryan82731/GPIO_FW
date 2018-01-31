#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "lib.h"
#include "pldm.h"

extern DPCONF *dpconf;
extern flash_data_t dirty_ext[10];
extern pldm_res_t *pldmresponse[7];

extern const asf_ctldata_t asf_ctldata[4];
extern const INT8U RMCP_IANA[4];
extern const INT8U asf_legacy_sensor_num;
extern const legacy_snr_t snrinfo[16];
extern const asf_alert_t asfalerts[16];
extern const INT8U asf_alerts_num;

asf_config_t *asfconfig;


#if (CONFIG_VERSION >= IC_VERSION_EP_RevB) || (CONFIG_BUILDROM)
INT8U bsp_set_pldm_slave_address(INT8U address)
{
    INT8U i = 0;

    dpconf->pldmslaveaddr = address;

    for(i = 0 ; i < 7; i++)
    {
        if(dpconf->pldmtype == PLDM_BLOCK_READ)
            pldmresponse[i]->sslaveaddr = dpconf->pldmslaveaddr;
        else
            pldmresponse[i]->slaveaddr  = dpconf->pldmslaveaddr;
    }

}

INT8U bsp_set_asftbl(asf_header_t *ptr)
{
    INT8U *asftbl;

    if(memcmp(ptr, "ASF!", 4))
        return 1;

    asftbl = malloc (ptr->length);
    memcpy((void *) asftbl, (void *) ptr, ptr->length);
    dirty_ext[ASFTBL].addr = (INT8U *) asftbl;
    dirty_ext[ASFTBL].flashaddr = (INT8U *) SYSTEM_ASF_TABLE;
    dirty_ext[ASFTBL].length = ptr->length;

    return 0;

}

//void ParserASFTable(INT8U *asftbl)
void ParserASFTable()
{
    INT8U  *asftbl = dirty_ext[ASFTBL].addr;
    INT8U  type;
    INT8U  index, i, j;
    INT16U length;
    INT8U  checkpec=0;
    INT8U  snrindex=0;

    //get the start address of information
    asftbl += ASF_TABLE_OFFSET;
    asfconfig->numofalerts = 0;
    asfconfig->numofsnr = 0;
    dpconf->lspoll = 0;
    memset(asfconfig->legacysnr, 0, sizeof(asfconfig->legacysnr));


    while(asftbl < (dirty_ext[ASFTBL].addr + dirty_ext[ASFTBL].length))
    {
        type = (*asftbl & 0x7F);
        length = *(asftbl+2) + ( *(asftbl+3) << 8);

        switch(type)
        {
        case ASF_INFO:
            asfconfig->maxwdt = *(asftbl+4);
            asfconfig->minsnrpoll = *(asftbl+5);
            asfconfig->systemid = *(asftbl+6) + (*(asftbl+7) << 8);
            memcpy(asfconfig->IANA, asftbl+8, 4);
            break;

        case ASF_ALRT:
            index = *(asftbl+6);
            asfconfig->numofalerts += index;

            //support maximal 16 events
            if(asfconfig->numofalerts > 16)
                break;

            for(i = 0 ; i < index; i++)
            {
                memcpy(&asfconfig->asfalerts[snrindex].address, asftbl+8+12*i, 12);
                if(asfconfig->asfalerts[snrindex].evtseverity == 0)
                    asfconfig->asfalerts[snrindex].logtype = LOG_INFO;
                else if (asfconfig->asfalerts[snrindex].evtseverity <= 8)
                    asfconfig->asfalerts[snrindex].logtype = LOG_WARNING;
                else
                    asfconfig->asfalerts[snrindex].logtype = LOG_ERROR;

                if((asfconfig->asfalerts[snrindex].address & 0x01))
                {
                    asfconfig->asfalerts[snrindex].exactmatch = 1;
                    asfconfig->asfalerts[snrindex].address &= 0xFE;
                }
                else
                    asfconfig->asfalerts[snrindex].exactmatch = 0;


                for(j=0; j <= asfconfig->numofsnr; j++)
                {
                    if((asfconfig->legacysnr[j].addr == asfconfig->asfalerts[snrindex].address) && (asfconfig->legacysnr[j].command == asfconfig->asfalerts[snrindex].command))
                        break;
                }

                if(j > asfconfig->numofsnr)
                {
                    asfconfig->legacysnr[asfconfig->numofsnr].addr = asfconfig->asfalerts[snrindex].address;
                    asfconfig->legacysnr[asfconfig->numofsnr].command = asfconfig->asfalerts[snrindex].command;
                    asfconfig->legacysnr[asfconfig->numofsnr].addrplus = asfconfig->asfalerts[snrindex].address+1;
                    asfconfig->numofsnr++;
                }



                if((*(asftbl+4) >> i) & 0x01)
                    asfconfig->asfalerts[snrindex].assert = 0;
                else
                    asfconfig->asfalerts[snrindex].assert = 1;

                if((*(asftbl+5) >> i) & 0x01)
                    asfconfig->asfalerts[snrindex].deassert = 0;
                else
                    asfconfig->asfalerts[snrindex].deassert = 1;

                snrindex++;

            }
            break;

        case ASF_RCTL:
            for(i = 0; i < *(asftbl+4); i++)
            {
                index = *(asftbl+8+i*4);
                memcpy(&asfconfig->asf_rctl[index], asftbl+8+i*4, 4);

                if(!checkpec)
                {
                    if(asfconfig->asf_rctl[index].slaveaddr & 0x01)
                    {
                        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0xCF;
                        asfconfig->pec = 1;
                        //enable PEC
                    }
                    else
                    {
                        REG8(SMBUS_BASE_ADDR + SEIO_SMEn) = 0xCD;
                        asfconfig->pec = 0;
                    }

                    if(dpconf->pldmfromasf)
                        bsp_set_pldm_slave_address(asfconfig->asf_rctl[index].slaveaddr & 0xFE);
                    checkpec = 1;
                }
                asfconfig->asf_rctl[index].slaveaddr &= 0xFE;

            }
            break;

        case ASF_RMCP:
            memcpy(asfconfig->RMCPCap, asftbl+4, 7);
            asfconfig->RMCPCmpCode = *(asftbl+11);
            if(!asfconfig->RMCPCmpCode)
            {
                memcpy(asfconfig->RMCPIANA, asftbl+12, 4);
                asfconfig->RMCPSpecialCmd = *(asftbl+16);
                memcpy(asfconfig->RMCPSpecialCmdArgs, asftbl+17, 2);
                memcpy(asfconfig->RMCPBootOptions, asftbl+19, 2);
                memcpy(asfconfig->RMCPOEMArgs, asftbl+21, 2);
            }
            break;

        case ASF_ADDR:
            break;

        }

        asftbl += length;
    }

    if(asfconfig->numofsnr)
        dpconf->lspoll = 1;
}
#endif

void bsp_load_asf_default(void)
{

    INT8U i = 0;
    //copy the default configuration into asfconfig
    dpconf = malloc(sizeof(DPCONF));
    memcpy((void *) dpconf, (void *) DPCONF_ROM_START_ADDR, sizeof(DPCONF));

    #ifdef CONFIG_SIMPLE_BACKUP //Simple Dpconf Backup Mechansim
	if(verifyUserChksum() == 0){
		RecoveryDpconfFromDefault();
    }
   #endif
   
    dpconf->restart = 0;

    asfconfig = malloc(512);

    memset(asfconfig,0,512);

    memcpy(asfconfig->asf_rctl, asf_ctldata, sizeof(asf_ctldata));
    for(i=0; i < 4; i++)
    {
        if(asfconfig->asf_rctl[i].slaveaddr & 0x01)
            asfconfig->pec = 1;
        asfconfig->asf_rctl[i].slaveaddr &= 0xFE;
    }

    //Set defualt slave address of PLDM Responder
    if (dpconf->pldmfromasf)
    {
        dpconf->pldmslaveaddr = asfconfig->asf_rctl[0].slaveaddr;
        dpconf->pldmpec = asfconfig->pec;
    }

    memcpy(asfconfig->IANA, RMCP_IANA, sizeof(RMCP_IANA));

    asfconfig->RMCPCap[0] = 0x67;
    asfconfig->RMCPCap[1] = 0xF8;
    asfconfig->RMCPCap[4] = 0x00;
    asfconfig->RMCPCap[5] = 0x1F;
    asfconfig->RMCPCap[6] = 0xFF;

    if(asf_legacy_sensor_num)
    {
        memcpy(asfconfig->legacysnr, snrinfo, sizeof(snrinfo));
        asfconfig->numofsnr = asf_legacy_sensor_num;
        memcpy(asfconfig->asfalerts, asfalerts, sizeof(asfalerts));
        asfconfig->numofalerts = asf_alerts_num;
        dpconf->lspoll = 1;
    }

    if (dpconf->chipset == INTEL || dpconf->chipset == AMDSOC)
        asfconfig->pec = 0;
    else if (dpconf->chipset == AMD){
	#if CONFIG_ENABLED_DEFAULT_PEC
		asfconfig->pec = 1;
	#else
		asfconfig->pec = 0;
	#endif
		
    }else{
	 
	 asfconfig->pec = 1;
    	 
    }
}

