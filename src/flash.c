#include <lib.h>
#include <aes.h>
#include <smbus.h>
#include <string.h>
#include <smbios.h>
#include <pldm.h>
#include <stdlib.h>
#include <bsp.h>

extern flash_data_t dirty[ENDTBL];
extern flash_data_t dirty_ext[10];
extern INT8U *timestampdataptr;
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern DPCONF *dpconf;
extern RoleInfo *rolehead;
extern SubEvent *subhead;
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
char *idata;
#else
extern char *idata;
#endif

#define FLASH_DEBUG 0

//#if CONFIG_VERSION  < 6
#define COMPARE_SAME 1
#define COMPARE_DIFF 0

int m_whocallSMBIOS; // 0 : init value , 1 : CMAC  , 2 : PLDM

INT8U CompareSMBIOS(INT8U *psmbiosram,INT8U *psmbiosrom , int length){
	INT8U flag = 0;
	INT32U idx = 0;
	SMBIOS_Table_EP *smbiosrom = psmbiosram;
	SMBIOS_Table_EP *smbiosram = psmbiosrom;

	
	if(smbiosram->StTableLen == smbiosrom->StTableLen && smbiosram->chkSum == smbiosrom->chkSum && smbiosram->ichkSum == smbiosrom->ichkSum && smbiosram->StNum == smbiosrom->StNum){

		


		if(memcmp(psmbiosram,psmbiosrom,48) == 0){
				//printf("[RTK] Header same\n");
			}else{
				//printf("[RTK] Header NOT\n");
			}
			
			if(memcmp(&psmbiosram[48],&psmbiosrom[48],length-48) == 0){
				//printf("[RTK] content same\n");
			}else{
				//printf("[RTK] content NOT\n");
			}
			
	
		if(memcmp(psmbiosram,psmbiosrom,length) == 0){			

			//printf("[RTK] compare Same\n");

			
			return COMPARE_SAME; // means the same
		}else{
			return COMPARE_DIFF; // means different
		}
		
	}else{
		return COMPARE_DIFF; // means different
	}
	
}

#ifdef CONFIG_SIMPLE_BACKUP

void RecoveryDpconfFromDefault(){

	INT8U *ptrFrom = NULL;	
	struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
	INT8U wrsize = FLASH_WRITE_SIZE ;
	INT8U *buffer = NULL;
	INT32U flashaddr=0,length=0,idx=0,numWrite=0;
	INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;

	INT32U idxDebug=0;
	

	buffer =malloc(wrsize);
	if(buffer == NULL){
		return;
	}
	
	flashaddr = (INT32U )((INT8U *) DPCONF_ROM_START_ADDR_NOCACHE)&0x00FFFFFF; //<---------------
	
	if (rdid == 0x1f){
       	spi_se_unprotect(ssi_map, flashaddr);
	}
       spi_se_erase(ssi_map, flashaddr);	
	   


	ptrFrom = (INT8U *)DPCONF_BACKUP_DEFAULT_ADDR;
	length = sizeof(DPCONF);
	numWrite = (length - 1)/wrsize + 1;
	for(idx=0;idx<numWrite;idx++){		
		memcpy(buffer , ptrFrom ,wrsize);					
		spi_write(ssi_map, flashaddr , buffer , wrsize);
		
		 ptrFrom += wrsize;
              flashaddr += wrsize;
							
	}
	

	
	if(buffer){
		free(buffer);
	}	 	
	memcpy((void *) dpconf, (void *) DPCONF_ROM_START_ADDR_NOCACHE, sizeof(DPCONF));

	
}

#endif
extern int m_isUpdateSMBIOS;
extern int m_ShouldChksum , m_CalChksum;
void FlashTask(void *pdata)
{
    OS_CPU_SR  cpu_sr = 0;
    aes_context *ctx = malloc(sizeof(aes_context));
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
    INT32U flashaddr,tmpsize=0;
    INT8U i, type;
    INT32U numwrite;
    INT8U *ptr = NULL, *tmpptr = NULL, *cbcptr;
    INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;
    flash_data_t *dirtyptr;
    INT8U reset;
    //ptr is the actual pointer for write into flash
    //cbcptr is used for decryption
    int newChksum ,smbioslen;
    INT8U realCopy;
	INT8U *tmp_smbiosptr;

    INT8U wrsize = FLASH_WRITE_SIZE ;
    WAKE_UP_t *wake;
    SMBIOS_Table_EP *smbiosrom = (SMBIOS_Table_EP *) (SMBIOS_ROM_START_ADDR + SMBIOS_DATA_OFFSET);//flash
    SMBIOS_Table_EP *smbiosram = (SMBIOS_Table_EP *) (timestampdataptr + SMBIOS_DATA_OFFSET);//sram



//only key auto loaded from efuse is needed
#if 0
#if CONFIG_VERSION >= 6
    INT32U sig;
    INT8U keylen;
#endif
#endif
	m_whocallSMBIOS = 0;

    cbcptr =  malloc(64);

    while (OS_TRUE)
    {
        //if POLL STOP, means firmware need to response with PLDM
        //in that condition, we should avoid update flash
        //while (smbiosrmcpdataptr->BIOSBoot)
        //    OSTimeDly(10);

        dirtyptr = &dirty[0];
        reset = 0;

        for (type = 0; type < ENDTBL; type++)
        {
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
            if(type == SUBTBL)
                dirtyptr = &dirty_ext[type];


            if(dirty[type].dirty && (type < SUBTBL))
            {
                dirty_ext[type].dirty=dirty[type].dirty;
                dirty_ext[type].length=dirty[type].length;
                dirty_ext[type].flashaddr=dirty[type].flashaddr;

            }
#endif
            if (dirtyptr->dirty)
            {
                flashaddr = (INT32U ) (dirtyptr->flashaddr) & 0x00FFFFFF;

                OS_ENTER_CRITICAL();
                switch (type)
                {

                case USERTBL:
                    numwrite = dirtyptr->length;
                    ptr = cbcptr;
                    tmpptr = (INT8U *) (getuserinfo()->next);
                    if(tmpptr != NULL)
                    {
                        memset(ctx, 0, sizeof(aes_context));
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
                        aes_crypt_sw(ctx, tmpptr, ptr, AES_ENCRYPT);
#else
                        aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#endif
                        wrsize = FLASH_WRITE_SIZE;
                    }
                    break;

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                case WAKETBL:
                    wake = GetWakePtnHead();
                    ptr = (INT8U *) wake;
                    numwrite = dirty[type].length*3;
                    wrsize = FLASH_WRITE_SIZE;
                    break;
#endif

#ifdef CONFIG_DASH_ENABLED
                case ROLETBL:
                    numwrite = dirtyptr->length;
                    ptr = (INT8U *) rolehead;
                    wrsize = 32;
                    break;

                case SUBTBL:
                    wrsize = FLASH_WRITE_SIZE;
                    numwrite =  dirtyptr->length / wrsize;
                    ptr = (INT8U *) subhead;
                    break;
                case SWTBL:
                    wrsize = FLASH_WRITE_SIZE;
                    numwrite =  (dirtyptr->length - 1 + wrsize)/ wrsize;
                    ptr = (INT8U *)idata;
                    break;

                case EVENTTBL:
                    numwrite = dirtyptr->length;
                    ptr = (INT8U *) eventGet();
                    wrsize = 32;
                    break;

                case PLDMTBL:
                    numwrite = dirtyptr->length;
                    ptr = dirtyptr->addr;
                    wrsize = FLASH_WRITE_SIZE;
                    break;

                case SMBIOSTBL:
                    //if (smbiosram->StTableLen == smbiosrom->StTableLen && smbiosram->chkSum == smbiosrom->chkSum && smbiosram->ichkSum == smbiosrom->ichkSum && smbiosram->StNum == smbiosrom->StNum)

			//printf("[RTK] (flash)dirtyptr->length = %d\n",dirtyptr->length);
					

		smbioslen = smbiosram->StTableLen;
		printf("[RTK]smbioslen=(%d)\n", smbioslen);

		#if 0
		//simple protect mechanism
		if((smbioslen ==0)||(smbioslen >= 4096)){
			 printf("[RTK] (flash SMBIOSTBL)do not copy , something wrong : len=%d \n",smbioslen);	
			dirtyptr->dirty = 0;
	                    numwrite = 0;
	                    ptr = NULL;
			dirtyptr->dirty = 0;
			break;			
		}
		#endif

		tmp_smbiosptr = (INT8U *)smbiosram;		
		dirtyptr->length = 0;//+briank.rtk : is it ok?
                    if(CompareSMBIOS(smbiosram , smbiosrom , dirtyptr->length - 1) == COMPARE_SAME)
                    {
                        dirtyptr->dirty = 0;
                        numwrite = 0;
                        ptr = NULL;
		    dirtyptr->length = 0;
			//newChksum
			//memcpy(&smbioslen ,smbiosram+16 , 2 );
			newChksum = chksum(tmp_smbiosptr +SMBIOS_HEADER_SIZE ,smbioslen);

			//printf("[RTK] (flash SMBIOSTBL)do not copy , len=%d , smbiosram address=0x%x\n",smbioslen,smbiosram);		
			//printf("[RTK] (A) m_isUpdateSMBIOS=(%d) , m_whocallSMBIOS=%d, is_flagA = 0x%x\n",m_isUpdateSMBIOS , m_whocallSMBIOS, is_flagA);
			    //	printf("m_ShouldChksum = 0x%x , m_CalChksum=0x%x , newChksum=0x%x\n" , m_ShouldChksum , m_CalChksum,newChksum);
				m_whocallSMBIOS = 0;
				m_isUpdateSMBIOS = -1;
                    }
                    else
                    {
			realCopy = 0;
			
			if((smbioslen ==0)||(smbioslen >= 8192)){
				printf("[RTK] do copy but fail: numwrite=%d , dirtyptr->length=%d , smbioslen=%d\n",numwrite,dirtyptr->length , smbioslen);
				dirtyptr->dirty = 0;
			          numwrite = 0;
			          ptr = NULL;
				 dirtyptr->length = 0;
				break;
			}

			newChksum = chksum(tmp_smbiosptr +SMBIOS_HEADER_SIZE ,smbioslen);				
				
			if(m_whocallSMBIOS == 1){//WCOM (CMAC) // If the new smbio is from CMAC , check the checksum before flash
				if((m_CalChksum == newChksum)&&(m_ShouldChksum ==newChksum ) &&(newChksum != 0)){						 
					// m_ShouldChksum is the checksum in the smbios table send from CMAC
					// m_CalChksum is calculated by fw( at  CMAC after reveive whole smbios table)
					// newChksum is calaulated by Flash task for double confirm
					realCopy = 1;
				}
			}else{
				realCopy = 1;
			}
			//printf("[RTK] (B) m_isUpdateSMBIOS=(%d) , m_whocallSMBIOS=%d  , is_flagA = 0x%x , realCopy=%d\n",m_isUpdateSMBIOS , m_whocallSMBIOS , is_flagA,realCopy);
			//printf("m_ShouldChksum = 0x%x , m_CalChksum=0x%x , newChksum=0x%x,  smbioslen =%d\n" , m_ShouldChksum , m_CalChksum,newChksum,smbioslen);

			 //Set to initial value for all variable ------------------
			m_whocallSMBIOS =0; 
			m_isUpdateSMBIOS = -1;
			m_ShouldChksum= 0;
			m_CalChksum = 0;						

			if(realCopy==1){
				dirtyptr->length = smbioslen + 32;
			          wrsize = FLASH_WRITE_SIZE;
			          numwrite =  (dirtyptr->length - 1 + wrsize)/ wrsize;
			          ptr = dirtyptr->addr;
				//printf("[RTK] (flash SMBIOSTBL)do copy , len=%d  \n" , smbioslen);												
			}else{
				//printf("[RTK] (flash SMBIOSTBL)do copy but fail, len=%d  \n" , smbioslen);
				dirtyptr->length = 0;
				dirtyptr->dirty = 0;
			          numwrite = 0;
			          ptr = NULL;
			}
			 //printf("[RTK] (flash SMBIOSTBL)do copy \n");			
                    }

                    if(numwrite == 0)
                        dirtyptr->dirty = 0; 
			
                    break;
#endif

                case DPCONFIGTBL:
                    //change the key handshaking mechanism
#if 0
#if CONFIG_VERSION >= 6
                    sig = *((INT32U *) (dpconf->aesmsg + 16)) & 0x00FFFFFF;
                    keylen = (*((INT32U *) (dpconf->aesmsg +16)) & 0xFF000000) >> 24;
                    if (dpconf->efusekey != VALID_KEY_SIG && sig == VALID_KEY_SIG && keylen >=4 && keylen <=16)
                    {
                        dpconf->efusekey = VALID_KEY_SIG;
                        updateCert(dpconf->aesmsg, keylen);
                        setdirty(USERTBL);
                    }
#endif
#endif

                    ptr = cbcptr;
                    tmpptr = (INT8U *) getuserinfo();

                    memset(ctx, 0, sizeof(aes_context));
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
                    //aes_crypt_sw(ctx, tmpptr, ptr, AES_ENCRYPT);
                    aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#else
                    aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#endif
                    wrsize = FLASH_WRITE_SIZE;
                    numwrite =  (dirtyptr->length - 1 + wrsize)/ wrsize;
                    break;


                default:
                    wrsize = FLASH_WRITE_SIZE;
                    numwrite =  (dirtyptr->length - 1 + wrsize)/ wrsize;
                    ptr = dirtyptr->addr;
                    break;
                }


                DEBUGMSG(FLASH_DEBUG, "Get table index %d : %d, %x\n", type, dirtyptr->length, numwrite, ptr);

                if(dirtyptr->dirty && (ptr || type == SUBTBL))
                    //if((dirty_ext[type].dirty && ptr) ||(dirty_ext[type].dirty==1 && type==SUBTBL))
                    //subscription may contian zero entry
                {
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
					if((REG8(MAC_BASE_ADDR + MAC_CONFIG0) & 0x07))
					{
						while(bsp_bits_get(MAC_FLASH_LOCK, BIT_FLASH_LOCK, 1))
							OSTimeDly(3);
					}
#endif

                    if (rdid == 0x1f)
                        spi_se_unprotect(ssi_map, flashaddr);
                    spi_se_erase(ssi_map, flashaddr);
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                    if (type == WAKETBL)
#else
                    if (type == SMBIOSTBL)
#endif
                    {
                        if (rdid == 0x1f)
                            spi_se_unprotect(ssi_map, flashaddr);
                        spi_se_erase(ssi_map, flashaddr + 0x1000);
                    }
                    tmpsize=0;
                    DEBUGMSG(FLASH_DEBUG, "Erase table index %d, flash %x\n", type, flashaddr);

                    for (i = 0 ; i < numwrite ; i++)
                    {
                        spi_write(ssi_map, flashaddr, ptr, wrsize);

                        switch (type)
                        {
                        case USERTBL:
                            ptr = cbcptr;
                            tmpptr = (INT8U *) ((UserInfo *) tmpptr)->next;
                            if(tmpptr != NULL)
                            {
                                memset(ctx, 0, sizeof(aes_context));
#if CONFIG_VERSION  >= IC_VERSION_EP_RevA
                                aes_crypt_sw(ctx, tmpptr, ptr, AES_ENCRYPT);
#else
                                aes_crypt(ctx, tmpptr, ptr, AES_ENCRYPT);
#endif

                                flashaddr += wrsize;
                            }
                            break;

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                        case WAKETBL:
                            if ((i+1) %3 == 0)
                            {
                                if(wake)
                                {
                                    wake = wake->next;
                                    ptr = wake;
                                }
                            }
                            else
                                ptr += wrsize;
                            flashaddr += wrsize;
                            break;
#endif

#ifdef CONFIG_DASH_ENABLED
                        case ROLETBL:
                            ptr = (INT8U *)(((RoleInfo *) ptr)->next);
                            flashaddr += wrsize;
                            break;

                        case SUBTBL:
                            if(tmpsize==0)
                                tmpptr=ptr;
                            tmpsize += wrsize;
                            if(tmpsize >=256)
                            {
                                ptr = (INT8U *)(((SubEvent *) tmpptr)->next);
                                tmpsize=0;
                            }
                            else
                                ptr += wrsize;

                            flashaddr += wrsize;
                            break;

                        case EVENTTBL:
                            ptr = (INT8U *) (((eventdata *)ptr)->next);
                            flashaddr += wrsize;
                            break;
#endif

                        case DPCONFIGTBL:
                            ptr = dirtyptr->addr + wrsize*(i+1);
                            flashaddr += wrsize;
                            break;


                        case PLDMTBL:
                            if (((pldm_t *) dirtyptr->addr)->numwrite[0] == i+1)
                            {
                                ptr = ((pldm_t *)dirtyptr->addr)->ptr[0];
                                //flashaddr = (PLDM_STR_TBL - DPCONF_ADDR_CACHED);
                                flashaddr = (PLDM_STR_TBL & 0x00FFFFFF);
                            }
                            else if (((pldm_t *) dirtyptr->addr)->numwrite[1] == i+1)
                            {
                                ptr = ((pldm_t *)dirtyptr->addr)->ptr[1];
                                //flashaddr = (PLDM_ATR_TBL - DPCONF_ADDR_CACHED);
                                flashaddr = (PLDM_ATR_TBL & 0x00FFFFFF);
                            }
                            else if (((pldm_t *) dirtyptr->addr)->numwrite[2] == i+1)
                            {
                                ptr = ((pldm_t *)dirtyptr->addr)->ptr[2];
                                //flashaddr = (PLDM_VAL_TBL - DPCONF_ADDR_CACHED);
                                flashaddr = (PLDM_VAL_TBL & 0x00FFFFFF);
                            }
                            else
                            {
                                ptr += wrsize;
                                flashaddr += wrsize;
                            }
                            break;


                        default:
                            ptr += wrsize;
                            flashaddr += wrsize;
                            DEBUGMSG(FLASH_DEBUG, "Write Flash %x, %x\n", ptr, flashaddr);
                            break;
                        }

                    }
                    dirtyptr->dirty = 0;

                    if (dirtyptr->wait)
                    {
                        DEBUGMSG(FLASH_DEBUG, "Enter Wait\n");
                        dirtyptr->wait = 0;
                        bsp_bits_set(MAC_OOBREG, 1, BIT_DASHEN, 1);

                    }
                }
                OS_EXIT_CRITICAL();

                if(dirtyptr->reset)
                    reset = dirtyptr->reset;
            }
            if (!dirty[DPCONFIGTBL].wait && !dirty[DPCONFIGTBL].reset)
                OSTimeDly(OS_TICKS_PER_SEC);

            dirtyptr++;
        }

        if(reset)
        {

#if CONFIG_VERSION < 6
            rlx_icache_invalidate_all();
            rlx_dcache_flush_all();
#endif
            DEBUGMSG(MESSAGE_DEBUG, "Before CPU Reset\n");
            disable_master_engine();
            reset_isr_table();
            cpu_reset();
        }

        if (!dirty[DPCONFIGTBL].wait && !dirty[DPCONFIGTBL].reset)
            OSTimeDly(10*OS_TICKS_PER_SEC);
    }
}
//#endif
