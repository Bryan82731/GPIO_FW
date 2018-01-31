#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pldm.h>
#include <smbus.h>
#include "bsp.h"
#include "lib.h"
#include "smbios.h"
#include "rmcp.h"

ROM_EXTERN  INT32U smbiosdatalen _ATTRIBUTE_ROM_BSS;
ROM_EXTERN2 INT8U smbios_flag;


static BiosAttr *attrhead=NULL,*attrtail=NULL;

ROM_EXTERN pldm_t pldmdata;
extern DPCONF *dpconf;
extern pldm_t *pldmdataptr;
ROM_EXTERN pldm_res_t *pldmresponse[7];
extern smbiosrmcpdata *smbiosrmcpdataptr;
extern INT8U *timestampdataptr;
extern INT8U *smbiosptr;
extern flash_data_t dirty[ENDTBL];
extern OS_EVENT *EVTQ;
extern sensor_t sensor[MAX_SENSOR_NUMS];
extern INT8U *AttrValBuf;
extern int AttrValOff;

char *_unicode(char *non_uni)
{
    char *retUni;
    unsigned int i;

    if (!non_uni)
    {
        return NULL;
    }

    retUni = (char *)malloc((strlen(non_uni)+1)*2);
    if (retUni == NULL)
    {
        return NULL;
    }
    memset(retUni, 0, ((strlen(non_uni)+1)*2));

    for (i=0; i<strlen(non_uni); i++)
    {
        retUni[(2*i)] = non_uni[i];
    }
    return retUni;
}
void AddItem(BiosAttr *NewItem)
{
    if (!attrhead)
        attrhead=NewItem;
    NewItem->next = NULL;
    if (attrtail)
        attrtail->next =NewItem;
    attrtail = NewItem;
}
void ReleasePldm()
{
    BiosAttr *mfreeptr,*mptr;

    mptr=attrhead;
    while (mptr)
    {
        mfreeptr = mptr;
        mptr = mptr->next;

        //these pointers are just assigned from tables
        ///free(mfreeptr->attr_possible_handle);
        ///free(mfreeptr->attr_data);
        ///free(mfreeptr->attr_val_index);
        free(mfreeptr);
    }
    attrhead=NULL;
    attrtail=NULL;

}

void SearchAttr(unsigned short tmp_handle,unsigned char *out)
{
    unsigned char *sptr;
    unsigned short handle,len;
    unsigned long i=0;

    //prevent null condition, but later would use strlen to count the name
    out[0] = 0;
    sptr=pldmdata.ptr[0];
    while (i < pldmdata.len[0])
    {
        handle=*(sptr+i) + (*(sptr+i+1) << 8);
        i=i+2;
        len=*(sptr+i) + (*(sptr+i+1) << 8);
        i=i+2;

        if (handle==tmp_handle)
        {
            strncpy(out,sptr+i,len);
            break;
        }
        i=i+len;
    }
    out=trimspace(out);
}

//INT32U SetValue(INT8U **buf,INT8U *str,INT8U *value)
INT32U SetValue(INT8U *str,INT8U *value)
{
    unsigned char *sptr,*tptr,*handle;
    unsigned short len;
    unsigned long i=0,j=0;
    unsigned char out[64],set=0;
    BiosAttr *Item;
    INT8U  attr_type = 0;
    INT16U  cur_val = 0,handle_val;    


    tptr = pldmdata.ptr[2];
    //prevent null condition, but later would use strlen to count the name

    for (Item = attrhead; Item; Item = Item->next)
    {
        memset(out,0,64);
        SearchAttr(Item->attr_name_handle,out);
        if (strcmp(str,out)==0)
            break;
    }
    j = 0;
    while (j < pldmdata.len[2]-4)
    {
        // handle = *(tptr + j)  + (*(tptr+j+1) << 8);
        handle=tptr+j;
        attr_type = *(tptr + j +2);

        if (attr_type == BIOSBootConfigSetting)
            j=j+5;
        else
            j=j+3;

        if (attr_type== BIOSString || attr_type == BIOSPassword)
        {
            cur_val =*(tptr+j) + (*(tptr+j+1) << 8);
            j=j+2;
            
        }
        else
        {
            cur_val=*(tptr+j);
            j++;
        }
        handle_val=*handle+(*(handle+1) <<8);
        if (handle_val == Item->attr_handle)
        {
            set=1;	
            break;
        }    

        j=j+cur_val*1;
    }
    if(set==1)
    {
      if (attr_type== BIOSString || attr_type == BIOSPassword)
      {
        if(cur_val <64)
        {	
          memset(out,0,64);	
          memcpy(out,_unicode(value),strlen(value)*2);
          memcpy(tptr+j,out,cur_val);
          if(AttrValOff+cur_val+5 < PLDM_ATT_TBL_SIZE)
          {
            memcpy(AttrValBuf+AttrValOff,tptr+j-5,cur_val+5);
            AttrValOff=AttrValOff+cur_val+5;
          }
          return 0;
        }
      }        	
     	
      j=0;
      for (j=0;j<Item->attr_possible_num;j++)
      {
        memset(out,0,64);
        SearchAttr(*(Item->attr_possible_handle+j),out);
        if(strcmp(out,value)==0 && attr_type==BIOSEnumeration)
        {
          *(Item->attr_val_index)=j;
          if(AttrValOff+5 < PLDM_ATT_TBL_SIZE)
          {
            memcpy(AttrValBuf+AttrValOff,Item->attr_val_index - 4,5);
            AttrValOff=AttrValOff+5;
          }
          return 0;
        }  
      }
    }  
    return 0;

}

BiosAttr *InitialPLDM()
{
    unsigned char *sptr,*tptr;
    unsigned char phase=0;
    BiosAttr *Item;
    unsigned long i=0,j=0,k=0;
    INT16U handle = 0;
    INT8U  attr_type = 0;
    INT16U  cur_val = 0;

    sptr = pldmdata.ptr[1];
    tptr = pldmdata.ptr[2];

    if (pldmdata.TBL2)
        // if(sptr)
    {
        ReleasePldm();
        while (i < pldmdata.len[1]-4)
        {
            phase=1;
            Item=malloc(sizeof(BiosAttr));
            memset(Item, 0 , sizeof(BiosAttr));           

            Item->attr_handle= *(sptr+i) + (*(sptr+i+1) << 8);
            i=i+2;
            Item->attr_type=*(sptr+i);
            i++;
            Item->attr_name_handle= *(sptr+i) + (*(sptr+i+1) << 8);
            //   SearchAttr(Item->attr_name_handle);
            i=i+2;
            if (Item->attr_type == BIOSString || Item->attr_type == BIOSPassword)
            {
                i=i+5;
                //get the default string length
                Item->attr_possible_num=*(sptr+i) + (*(sptr+i+1) << 8);
                i=i+2;
                Item->attr_data=sptr+i;
                i=i+(Item->attr_possible_num)*1;
            }
            else if (Item->attr_type == BIOSBootConfigSetting)
            {
                i=i+4;
                Item->attr_possible_num=*(sptr+i);
                i++;
                Item->attr_possible_handle= (INT16U *) (sptr+i);
                i=i+(Item->attr_possible_num)*2;
                i=i+2;;
            }
            else
            {
                Item->attr_possible_num=*(sptr+i);
                i++;
                Item->attr_possible_handle= (INT16U *) (sptr+i);
                i=i+(Item->attr_possible_num)*2;
                Item->attr_default_num=*(sptr+i);
                i++;
                Item->attr_default_index=sptr+i;
                i++;
            }
            AddItem(Item);
        }
    }

    //add the attribute value table into Item
    if (pldmdata.TBL3)
        //if(tptr)
    {
        for (Item = attrhead; Item; Item = Item->next)
        {
            j = 0;
            while (j < pldmdata.len[2]-4)
            {
                k=j;
                handle = *(tptr + j)  + (*(tptr+j+1) << 8);
                attr_type = *(tptr + j +2);

                if (attr_type == BIOSBootConfigSetting)
                    j=j+5;
                else
                    j=j+3;

                if (attr_type== BIOSString || attr_type == BIOSPassword)
                {
                    cur_val =*(tptr+j) + (*(tptr+j+1) << 8);
                    j=j+2;
                }
                else
                {
                    cur_val=*(tptr+j);
                    j++;
                }

                if (handle == Item->attr_handle)
                {
                    Item->attr_val_handle=tptr+k;
                    Item->attr_val_index=tptr+j;
                    Item->attr_val_possible_num=cur_val;
                    break;
                }
                j=j+cur_val*1;
            }
        }
    }
//   SearchHandler(pattern);
    return attrhead;
}
INT8U getPLDMElement(BiosAttr *addr,unsigned char ptype,unsigned char val,unsigned char *out)
{
    BiosAttr *ptr;
    //unsigned char val;
    unsigned short len;
    ptr=addr;

    if (ptype==0)
        SearchAttr(ptr->attr_name_handle,out);
    else
        SearchAttr(*(ptr->attr_possible_handle+val),out);

    return 0;

}


INT32U CreateAttrVal(INT8U *buf,INT8U *seq)
{
    //seq contain the start address of the 1st boot device
    BiosAttr *ptr =  attrhead;
    INT8U  i=0;
    INT8U  j=0;
    INT16U tmphandle = 0xFFFF;



    while (ptr)
    {
        if (ptr->attr_type == BIOSBootConfigSetting)
        {

#ifndef CONFIG_BLOCK_WRITE
            for (i = 0; i < ptr->attr_val_possible_num; i++)
            {

                if (dpconf->bios == AMIBIOS && dpconf->biostype == LEGACY)
                {
                    //AMI use the string table index as a boot index
                    if (*(ptr->attr_val_index+i) != *(seq + i))
                    {
                        if (tmphandle == 0xFFFF)
                        {
                            j = i;
                            tmphandle = *(ptr->attr_possible_handle + i);
                        }
                        else
                        {
                            *(ptr->attr_possible_handle+j) = *(ptr->attr_possible_handle+i);
                            *(ptr->attr_possible_handle+i) = tmphandle;

                        }
                    }
                }

                *(ptr->attr_val_index +i) = *(seq + i);
            }
#else
            for (i = 0; i < ptr->attr_val_possible_num; i++)
                *(ptr->attr_val_index +i) = *(seq + i);

#endif
            for (i=0,j=0; i< ptr->attr_val_possible_num+6; i++)
            {
                if (i>5)
                {
                    *(buf+i)=*(seq+j);
                    j++;
                }
                else
                    *(buf+i)=*(ptr->attr_val_index-6+i);
            }

            return (ptr->attr_val_possible_num + 6);

        }
        ptr = ptr->next;
    }

    return 0;
}
#if 1//(CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
void bsp_pldm_set(INT8U *buf, INT8U len)
{
    INT8U pad = 0;
    INT32U crc;
    INT8U *ptr ;
    INT8U *tmp;

    if (pldmdataptr->valid)
    {
        if (dpconf->bios == PHOENIXBIOS && dpconf->pldmtype == PLDM_BLOCK_READ)
        {
            //set the pointer to the starting point of middle packet
            //which is used to hold the pending value table
            ptr = ((INT8U *) pldmresponse[1]) + 8;
            pad  =  (len % 4) ? (4 - (len %4 )) : 0 ;
            memcpy(ptr, buf, len);
            if (pad != 0)
                memset(&ptr[len], 0, pad);
            crc = crc32(0, ptr, len+pad);
            memcpy(&ptr[len+pad], &crc, 4);
            pldmresponse[1]->length = len + pad + 4 + 5;
            //pldmdataptr->tag[0] = pldmdataptr->tag[0] ^ 0xFFFFFFFF;
            //pldmdataptr->tag[1] = pldmdataptr->tag[1] ^ 0xFFFFFFFF;
            //pldmdataptr->tag[2] = pldmdataptr->tag[2] ^ 0xFFFFFFFF;
        }
        else if(dpconf->pldmtype == PLDM_BLOCK_READ)
        {
            tmp = malloc(64);
            ptr = ((INT8U *) pldmresponse[0]) + 18;
            pad  =  (len % 4) ? (4 - (len %4 )) : 0 ;
            memcpy(tmp, buf, len);
            memset(tmp+len , 0 , pad);
            len += pad;
            crc = crc32(0, tmp, len);
            memcpy(tmp+len, &crc, 4);
            len += 4;
            if (len <= (32-15))
            {
                memcpy(ptr, tmp, len);
                pldmresponse[0]->length = len + 0x0f;
            }
            else
            {
                memcpy(ptr, tmp, 17);
                pldmresponse[0]->length = 0x20;
                memcpy(&pldmresponse[1]->sslaveaddr, tmp+17, len-17);
                pldmresponse[1]->length = len-17;
            }
            free(tmp);
        }
        else
        {
            ptr = &pldmresponse[0]->val[5];
            pad  =  (len % 4) ? (4 - (len %4 )) : 0 ;
            memcpy(ptr, buf, len);
            ///pldmresponse[0]->val[8]= 0x03;
            memset(ptr+len, 0 ,pad);
            len += pad;
            crc = crc32(0, ptr, len);
            memcpy(ptr+len, &crc, 4);
            len += 4;
            pldmresponse[0]->length = len + 0x0f;
        }
        //AMD need the tags to be the same for accepting pending value tables

        smbiosrmcpdataptr->bootopt[2] = 0x0;
        pldmdataptr->dirty = 1;

    }
    else
    {   //asf method
        smbiosrmcpdataptr->bootopt[4] = buf[6] + 1;
        smbiosrmcpdataptr->bootopt[2] = 0x11;
        //transform from 0x00 - 0x04 to 0x01 - 0x05
        //asf spec
    }

}

void handle_pldm_br_phoenix(INT8U *addr)
{
    if (*(addr+1) == 0x0f && *(addr+10) == 0x03 && (*(addr+4) & *(addr+7) & 0x80) == 0x80)
    {
        switch (*(addr+11))
        {
        case GetBIOSTableTags:
            pldmdataptr->pldmerror = 0;
            if (pldmdataptr->valid)
            {
                pldmdataptr->index = 5;
                memcpy(&pldmresponse[pldmdataptr->index]->val[0], &pldmdataptr->tag[0], 3*sizeof(pldmdataptr->tag[0]));
            }
            else
            {
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_TAG_UNAVAILABLE;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
            }
            break;

        case GetBIOSTable:
            if (addr[17] == 0x03 && pldmdataptr->dirty)
            {
                pldmdataptr->index = 0;
                //pldmdataptr->dirty = 0;
            }
            else
            {
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_UNAVAILABLE;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
                //may only end here
                smbiosrmcpdataptr->PollType = POLL_NONE;
            }
            break;

        case AcceptBIOSAttributesPendingValues:
            pldmdataptr->dirty = 0;

        default:
            //default length
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->length = 0x0A;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            break;
        }
        pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
        pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];

    }

}
extern int m_whocallSMBIOS;
void handle_pldm_br_ami(INT8U *addr)
{

    SMBIOS_Table_EP *smbiosmeta ;

    if (*(addr+1) == 0x0f && *(addr+10) == 0x03 && (*(addr+7) & 0x80) == 0x80 && *(addr+4) == 0x01)//add one item for fujitsu PLDM @20140515
    {
        switch (*(addr+11))
        {
        case GetBIOSTableTags:
            pldmdataptr->pldmerror = 0;
            if (pldmdataptr->valid)
            {
                pldmdataptr->index = 5;
                memcpy(&pldmresponse[pldmdataptr->index]->val[0], &pldmdataptr->tag[0], 3*sizeof(pldmdataptr->tag[0]));
            }
            else
            {
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_TAG_UNAVAILABLE;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
            }
            break;

        case GetBIOSTable:
            if (addr[17] == 0x03 && pldmdataptr->dirty)
            {
                pldmdataptr->index = 0;
                //pldmdataptr->dirty = 0;
            }
            else
            {
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_UNAVAILABLE;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
                //may only end here
                smbiosrmcpdataptr->PollType = POLL_NONE;
            }
            break;

        case AcceptBIOSAttributesPendingValues:
            pldmdataptr->dirty = 0;

        default:
            //default length
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->length = 0x0A;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            break;
        }
        pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
        pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];

    }

    //only AMI implements DSP0246
    else if (*(addr+1) == 0x0f && *(addr+10) == 0x01 && (*(addr+7) & 0x80) == 0x80 && *(addr+4) == 0x01)//add one item for fujitsu PLDM @20140515
    {
        switch (*(addr+11))
        {
        case GetSMBIOSStructureTableMetadata:
            pldmdataptr->pldmerror = 0;
            pldmdataptr->index = 5;
            smbiosmeta = (SMBIOS_Table_EP *) (timestampdataptr + SMBIOS_DATA_OFFSET);
            memcpy(&pldmresponse[pldmdataptr->index]->val[0], &smbiosmeta->majorVer , 4);
            memcpy(&pldmresponse[pldmdataptr->index]->val[4], &smbiosmeta->StTableLen , 2);
            memcpy(&pldmresponse[pldmdataptr->index]->val[6], &smbiosmeta->StNum , 2);
            memcpy(&pldmresponse[pldmdataptr->index]->val[8], smbiosmeta , 4);
	#ifdef CONFIG_CPU_SLEEP_ENABLED
		EnableCPUSleep();
	#endif
            break;

        case SetSMBIOSStructureTable:
            smbiosptr = (INT8U *) (SMBIOS_ROM_START_ADDR + SMBIOS_DATA_OFFSET);
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->length = 0x0A;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            break;

        case SetSMBIOSStructureTableMetadata:
            smbios_flag=1;
            smbiosptr = (INT8U *) (timestampdataptr + SMBIOS_DATA_OFFSET);
            smbiosmeta = (SMBIOS_Table_EP *) (timestampdataptr + SMBIOS_DATA_OFFSET);
            memcpy(smbiosmeta,addr+20 ,4);
            memcpy(&smbiosmeta->majorVer, addr+12, 4);
            memcpy(&smbiosmeta->StTableLen, addr+16, 2);
            memcpy(&smbiosmeta->StNum, addr+18, 2);
            dirty[SMBIOSTBL].length = *(INT16U *) (addr+16) + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;
            memset(timestampdataptr + dirty[SMBIOSTBL].length, 0 , MAX_SMBIOS_SIZE-dirty[SMBIOSTBL].length);
			m_whocallSMBIOS = 2;
            setdirty(SMBIOSTBL);
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->length = 0x0A;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            DEBUGMSG(PLDM_DEBUG, "Entering ParserSensor [%s:%d]: %s\n", __FILE__,__LINE__,__func__);
            #if 0//#ifndef CONFIG_VENDOR_FSC //remove parser to avoid no reply pldm request command @20141104
            ParserSensor();
            #endif
            break;

        default:
            //default length
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->length = 0x0A;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            break;
        }
        pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
        pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];
    }
}

void handle_pldm_bw(INT8U *addr)
{
    return ;
}

void setPLDMEndPoint(INT8U srcept, INT8U dstept)
{
    int i = 0;

    if (srcept == 0 && dstept == 0)
        return;

    for (i=0 ; i < 7 ; i++)
    {
        pldmresponse[i]->srcept = srcept;
        pldmresponse[i]->dstept = dstept;
    }

}

//#if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)

void smbus_process_pldm_br_ami(INT8U *addr)
{

    INT8U pldmaddr = dpconf->pldmslaveaddr + 1;
    INT8U command;
    OS_CPU_SR cpu_sr = 0;
    static INT8U *pldmptr = NULL;
    static INT32U *pldmlen = NULL;
    static INT8U length = 0;

    if (*(addr+2) > 0x20 || *(addr+2) == 0)
    {
        DEBUGMSG(CRITICAL_ERROR, "Get Block Write Data : %02x %02x %02x %02x %02x %02x %02x %02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7] );

    }
    else if (*(addr+3) == pldmaddr &&  *(addr+4) == 0x01 && *(addr+7) == 0xC8 && *(addr+11) != 0x02  && *(addr+8) == 0x01 && *(addr+10) == 0x03)
    {
        command = *(addr+11);
        switch (command)
        {

        case 0x05: //Set bios table tag
            handle_pldm_tags(addr);
            break;

        case 0x0d: //Set datetime
            //set datatime is the first entry for PLDM
            pldm_handle_setgettime(addr);
            pldmptr = pldmdataptr->xferptr[0];
            pldmlen = &pldmdataptr->xferlen[0];
            break;

        }

    }
    else if (*(addr+3) == pldmaddr &&  *(addr+4) == 0x01 && (*(addr+7) & 0x88) == 0x88  &&  (*(addr+10) == 0x03 || *(addr+10) == 0x01))
    {
        if (*(addr+11) == 0x02)
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_STOP;
            ///smbiosrmcpdataptr->BIOSBoot = 1;
            OS_EXIT_CRITICAL();
            //stop again, since the table is processing

            //length = *(addr+2);
            //tbltype = *(addr+17);
            pldmptr = pldmdataptr->xferptr[*(addr+17)];
            pldmlen = &pldmdataptr->xferlen[*(addr+17)];

            //since it is sequential updating
            //pldmxferptr->ptr[tbltype] = pldmptr;
            //pldmlen = &pldmxferptr->len[tbltype];

            length = *(addr+2);
            memcpy(pldmptr, addr+18, length-15);
            pldmptr += length - 15;
            *pldmlen += length - 15;
        }
        //smbios data
        else if (*(addr+11) == 0x04)
        {////block read : pldm cmd=0x04 , 1st packet of first transfer
            pldmptr = timestampdataptr + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;
            pldmlen = &smbiosdatalen;
            *pldmlen = 0;

            length = *(addr+2);
            memcpy(pldmptr, addr+17, length-14);
            pldmptr += length - 14;
            *pldmlen += length - 14;

        }


    }
    else
    {
        if (*(addr+3) == pldmaddr &&  *(addr+4) == 0x01 && (*(addr+7) & 0x08) == 0x08 && *(addr+8) == 0x80 && ((*(addr+9) == 0x03 && *(addr+10) == 0x02) || (*(addr+9) == 0x01 && *(addr+10) == 0x04)))
        {
            if (*(addr+9) == 0x03)
            {
                length = *(addr+2);
                memcpy(pldmptr, addr+17, length-14);
                pldmptr += length -14;
                *pldmlen += length - 14;
            }
            else if (*(addr+9) == 0x01)
            {
                length = *(addr+2);
                memcpy(pldmptr, addr+16, length-13);
                pldmptr += length -13;
                *pldmlen += length - 13;
            }
        }
        else
        {//block read : pldm cmd=0x04 , 2nd packet
            length = *(addr+2);
            memcpy(pldmptr, addr+3, length);
            pldmptr += length;
            *pldmlen += length;
        }
    }
}

void smbus_process_pldm_br_phoenix(INT8U *addr)
{
    INT8U pldmaddr = dpconf->pldmslaveaddr + 1;
    INT8U command;
    OS_CPU_SR cpu_sr = 0;
    static INT8U *pldmptr = NULL;
    static INT32U *pldmlen = NULL;
    static INT8U length = 0;

    if (*(addr+2) > 0x20 || *(addr+2) == 0)
    {
        DEBUGMSG(CRITICAL_ERROR, "Get Block Write Data : %02x %02x %02x %02x %02x %02x %02x %02x\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7] );

    }
    else if (*(addr+3) == pldmaddr && *(addr+4) == 0xc1 && *(addr+7) == 0xc0 && *(addr+8) == 0x01 && *(addr+10) == 0x03)
    {
        command = *(addr+11);
        switch (command)
        {

        case 0x05: //Set bios table tag
            handle_pldm_tags(addr);
            break;

        case 0x0d: //Set datetime
            //set datatime is the first entry for PLDM
            pldm_handle_setgettime(addr);
            pldmptr = pldmdataptr->xferptr[0];
            pldmlen = &pldmdataptr->xferlen[0];
            break;

        }

    }
    else if ( *(addr+4) == 0x81 && (*(addr+7)&0x80)==0x80 && *(addr+8)==0x01 && *(addr+10)==0x03 && *(addr+11)==0x02)
    {
        if (*(addr+11) == 0x02)
        {
            OS_ENTER_CRITICAL();
            smbiosrmcpdataptr->PollType = POLL_STOP;
            ///smbiosrmcpdataptr->BIOSBoot = 1;
            OS_EXIT_CRITICAL();
            //stop again, since the table is processing

            //length = *(addr+2);
            //tbltype = *(addr+17);
            pldmptr = pldmdataptr->xferptr[*(addr+17)];
            pldmlen = &pldmdataptr->xferlen[*(addr+17)];

            //since it is sequential updating
            //pldmxferptr->ptr[tbltype] = pldmptr;
            //pldmlen = &pldmxferptr->len[tbltype];

            length = *(addr+2);
            memcpy(pldmptr, addr+18, length-15);
            pldmptr += length - 15;
            *pldmlen += length - 15;
        }
    }
    else
    {
        length = *(addr+2);
        memcpy(pldmptr, addr+8, length-5);
        pldmptr += length-5;
        *pldmlen += length - 5;
    }
}
void smbus_process_pldm_bw(INT8U *addr)
{
    INT8U pldmaddr = dpconf->pldmslaveaddr + 1;
    INT8U command;

    OS_CPU_SR cpu_sr = 0;
    static INT8U *pldmptr;
    static INT32U *pldmlen;
    static INT8U length = 0;
    static INT8U pldmres = 1;
    static INT32U pldmhandler = 0;
    SMBIOS_Table_EP *smbiosmeta ;
    INT32U crc;
    INT8U snrnum;
    eventdata evtdata;
    eventdata *evt_ptr;
    INT8U pldmcode;

    if (*(addr+3) == pldmaddr && (*(addr+4) == 0x01) && ((*(addr+7) & 0xC8) == 0xC8))
    {
        if (*(addr+8) == 0x01 && *(addr+10) == 0x03)
        {
            //offset 11 is the command code for PLDM
            command = *(addr+11);
            switch (command)
            {
            case GetBIOSTableTags:
                pldmdataptr->pldmerror = 0;
                if (pldmdataptr->valid)
                {
                    pldmdataptr->index = 5;
                    pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                    memcpy(&pldmresponse[pldmdataptr->index]->val[0], &pldmdataptr->tag[0], 3*sizeof(pldmdataptr->tag[0]));
                }
                else
                {
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0A;
                    pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_TAG_UNAVAILABLE;
                    pldmptr = pldmdataptr->xferptr[0];
                    pldmlen = &pldmdataptr->xferlen[0];
                }
                break;

            case GetBIOSTable:
                if (addr[17] == 0x03 && pldmdataptr->dirty)
                {
                    pldmdataptr->index = 0;
                    //pldmdataptr->dirty = 0;
                }
                else
                {
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0A;
                    pldmresponse[pldmdataptr->index]->pldmcode = PLDM_BIOS_TABLE_UNAVAILABLE;
                    //may only end here
                    smbiosrmcpdataptr->PollType = POLL_NONE;
                }
                break;

            case 0x02: //Set bios table
                pldmres = 0;
                OS_ENTER_CRITICAL();
                smbiosrmcpdataptr->PollType = POLL_STOP;
                ///smbiosrmcpdataptr->BIOSBoot = 1;
                OS_EXIT_CRITICAL();
                //stop again, since the table is processing

                //length = *(addr+2);
                //tbltype = *(addr+17);
                //only response when start bit is set
                if ((*(addr+16) & 0x01)== 0x01)
                {
                    memcpy(&pldmhandler, addr+12, 4);
                    pldmptr = pldmdataptr->xferptr[*(addr+17)];
                    pldmlen = &pldmdataptr->xferlen[*(addr+17)];
                }

                //since it is sequential updating
                //pldmxferptr->ptr[tbltype] = pldmptr;
                //pldmlen = &pldmxferptr->len[tbltype];

                length = *(addr+2);
                pldmhandler += (length-15);
                if(((*(addr+17) == 0) && (pldmhandler >= PLDM_STR_TBL_SIZE)) || (((*(addr+17) == 1) || (*(addr+17) == 2)) && (pldmhandler >= PLDM_ATT_TBL_SIZE)))
                {
                    pldmdataptr->pldmerror = 1;
                    pldmcode = PLDM_ERROR;

                }
                else
                {
                    pldmcode = PLDM_SUCCESS;
                    memcpy(pldmptr, addr+18, length-15);
                    pldmptr += length - 15;
                    *pldmlen += length - 15;
                }

                //only response when end bit is set
                if ((addr[16] & 0x04) == 0x04)
                {
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0A;
                    pldmresponse[pldmdataptr->index]->pldmcode = pldmcode;
                    pldmres = 1;

                }
                else if(dpconf->pldmmultitx)
                {
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0E;
                    pldmresponse[pldmdataptr->index]->pldmcode = pldmcode;
                    memcpy(pldmresponse[pldmdataptr->index]->val, &pldmhandler, 4);
                    pldmres = 1;

                }
                break;
            case 0x05: //Set bios table tag

                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                handle_pldm_tags(addr);

                break;

            case 0x0d: //Set datetime
                //set datatime is the first entry for PLDM
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                pldmresponse[pldmdataptr->index]->length = 0x0A;

                pldm_handle_setgettime(addr);

                break;

            case AcceptBIOSAttributesPendingValues:
                pldmdataptr->dirty = 0;

            default:
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                break;

            }
            if (pldmres)
            {
                pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
                pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];
                master_send(PLDM_RESPONSE, (INT8U *)pldmresponse[pldmdataptr->index], pldmresponse[pldmdataptr->index]->length + 3, 0x09);

                //patching the change case for DELL
                if (dpconf->pldmreset && (command == AcceptBIOSAttributesPendingValues))
                    SMBus_Prepare_RmtCtrl(RMCP_Reset,1);
            }
        }
        else if (*(addr+1) == 0x0F && *(addr+8) == 0x00 )
        {
            //MCTP Version
            pldmresponse[6]->pldmtype = *(addr+10);
            if (*(addr+10) == 0x02) //get endpoint ID
            {
                pldmresponse[6]->dstept = *(addr+6);
                pldmresponse[6]->srcept = 0x09;
                pldmresponse[6]->length = 0x0C;
                pldmresponse[6]->pldmcode = 0x09;
                pldmresponse[6]->val[0] = 0x01;
                pldmresponse[6]->val[1] = 0x00;

            }
            //Get MCTP version command
            else if (*(addr+10) == 0x04 && *(addr+11) == 0x00)
            {
                setPLDMEndPoint(*(addr+5), *(addr+6));
                pldmresponse[6]->length = 0x0E;
                pldmresponse[6]->pldmcode = 0x01;
                pldmresponse[6]->val[0] = 0xF1;
                pldmresponse[6]->val[1] = 0xF0;
            }
            else if (*(addr+10) == 0x05)
            {
                pldmresponse[6]->length = 0x0C;
                pldmresponse[6]->pldmcode = 0x02;
                pldmresponse[6]->val[0] = 0x00;
                pldmresponse[6]->val[1] = 0x01;
            }
            master_send(PLDM_RESPONSE, (INT8U *) pldmresponse[6], pldmresponse[6]->length + 3, 0x09);

        }
//PLDM 0240 PLDM Messaging Control and Discovery
        else if (*(addr+8) == 0x01 && *(addr+10) == 0x00)
        {
            pldmdataptr->index = 4;
            pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
            if (*(addr+11) == GetPLDMVersion && *(addr+17) == 0x00)
            {
                pldmresponse[pldmdataptr->index]->length = 0x17;
                pldmresponse[pldmdataptr->index]->val[0] = 0x08;
                pldmresponse[pldmdataptr->index]->val[4] = 0x05;
                pldmresponse[pldmdataptr->index]->val[5] = 0xF1;
                pldmresponse[pldmdataptr->index]->val[6] = 0xF0;
                pldmresponse[pldmdataptr->index]->val[7] = 0xF0;
                pldmresponse[pldmdataptr->index]->val[8] = 0x00;
                //iid is different ?
                crc = crc32(0, &pldmresponse[pldmdataptr->index]->val[5],4);
                memcpy(&pldmresponse[pldmdataptr->index]->val[9], (const char *)crc, 4);

            }
            else if (*(addr+11) == GetPLDMTypes)
            {
                pldmresponse[pldmdataptr->index]->length = 0x12;
                pldmresponse[pldmdataptr->index]->val[0] = 0x0F;
                memset(&pldmresponse[pldmdataptr->index]->val[1], 0 , 7);

            }
            else if (*(addr+11) == GetPLDMCommands)
            {
                pldmresponse[pldmdataptr->index]->length = 0x2A;
                pldmresponse[pldmdataptr->index]->val[0] = 0x1F;
                memset(&pldmresponse[pldmdataptr->index]->val[1], 0 , 31);

            }
            else if (*(addr+11) == GetTID)
            {
                pldmresponse[pldmdataptr->index]->length = 0x0B;
                pldmresponse[pldmdataptr->index]->val[0] = 0x00;
            }
            else
            {

                pldmresponse[pldmdataptr->index]->length = 0x0A;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_ERROR_UNSUPPORTED_PLDM_CMD;
            }
            pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
            pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];
            master_send(PLDM_RESPONSE, (INT8U *)pldmresponse[4], pldmresponse[4]->length + 3, 0x09);

        }
        else if (*(addr+8) == 0x01 && *(addr+10) == 0x01)
        {

            command = *(addr+11);
            switch (command)
            {
            case GetSMBIOSStructureTableMetadata:
                pldmdataptr->pldmerror = 0;
                pldmdataptr->index = 5;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                smbiosmeta = (SMBIOS_Table_EP *) (timestampdataptr + SMBIOS_DATA_OFFSET);
                memcpy(&pldmresponse[pldmdataptr->index]->val[0], &smbiosmeta->majorVer , 4);
                memcpy(&pldmresponse[pldmdataptr->index]->val[4], &smbiosmeta->StTableLen , 2);
                memcpy(&pldmresponse[pldmdataptr->index]->val[6], &smbiosmeta->StNum , 2);
                memcpy(&pldmresponse[pldmdataptr->index]->val[8], smbiosmeta , 4);
                break;

            case SetSMBIOSStructureTableMetadata:
                if(pldmdataptr->pldmerror)
                {

                    pldmdataptr->pldmerror = 0;
                    break;
                }
                smbios_flag=1;
                smbiosptr = (INT8U *) (timestampdataptr + SMBIOS_DATA_OFFSET);
                smbiosmeta = (SMBIOS_Table_EP *) (timestampdataptr + SMBIOS_DATA_OFFSET);
                memcpy(smbiosmeta,addr+20 ,4);
                memcpy(&smbiosmeta->majorVer, addr+12, 4);
                memcpy(&smbiosmeta->StTableLen, addr+16, 2);
                memcpy(&smbiosmeta->StNum, addr+18, 2);
                dirty[SMBIOSTBL].length = *(INT16U *) (addr+16) + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;
                memset(timestampdataptr + dirty[SMBIOSTBL].length, 0 , MAX_SMBIOS_SIZE-dirty[SMBIOSTBL].length);
                setdirty(SMBIOSTBL);
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                ParserSensor();
                break;

            case SetSMBIOSStructureTable:
                //change to the ROM one
                smbiosptr = (INT8U *) (SMBIOS_ROM_START_ADDR + SMBIOS_DATA_OFFSET);
                pldmres = 0;

                if ((*(addr+16) & 0x01)== 0x01)
                {
                    pldmptr = timestampdataptr + SMBIOS_DATA_OFFSET + SMBIOS_HEADER_SIZE;
                    pldmlen = &smbiosdatalen;
                    *pldmlen = 0;
                    memcpy(&pldmhandler, addr+12, 4);
                }

                pldmhandler += (length - 14);
                length = *(addr+2);

                //maximal SMBIOS table length should be 8192
                if(pldmhandler >= 8000)
                {
                    pldmdataptr->pldmerror = 1;
                    pldmcode = PLDM_ERROR;

                }
                else
                {
                    pldmcode = PLDM_SUCCESS;
                    memcpy(pldmptr, addr+17, length-14);
                    pldmptr += length - 14;
                    *pldmlen += length - 14;
                }


                if ((*(addr+16) & 0x04)== 0x04)
                {
                    pldmres = 1;
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0A;
                    pldmresponse[pldmdataptr->index]->pldmcode = pldmcode;
                }
                else if (dpconf->pldmmultitx)
                {
                    pldmdataptr->index = 4;
                    pldmresponse[pldmdataptr->index]->length = 0x0E;
                    pldmresponse[pldmdataptr->index]->pldmcode = pldmcode;
                    memcpy(pldmresponse[pldmdataptr->index]->val, &pldmhandler, 4);
                    pldmres = 1;

                }

                /*
                        length = *(addr+2);
                        memcpy(pldmptr, addr+17, length-14);
                        pldmptr += length - 14;
                        *pldmlen += length - 14;
                */
                break;

            default:
                pldmdataptr->index = 4;
                pldmresponse[pldmdataptr->index]->length = 0x0A;
                pldmresponse[pldmdataptr->index]->pldmcode = PLDM_SUCCESS;
                break;

            }
            if (pldmres)
            {
                pldmresponse[pldmdataptr->index]->pldmtype = addr[10];
                pldmresponse[pldmdataptr->index]->pldmcmd = addr[11];
                master_send(PLDM_RESPONSE, (INT8U *)pldmresponse[pldmdataptr->index], pldmresponse[pldmdataptr->index]->length + 3, 0x09);
            }
        }
    }
    //PlatformEventMessage
    else if (*(addr+3) == 0xDB && *(addr+10) == 0x02 && *(addr+11) == 0x0A )
    {
        for (snrnum = 0; snrnum < MAX_SENSOR_NUMS; snrnum++)
            if (sensor[snrnum].offset[0] == *(addr+15))
                break;
        //find it

        sensor[snrnum].state = *(addr+18);
        //only generate assert alerts
        if ((sensor[snrnum].state != 0x01) && (sensor[snrnum].state != sensor[snrnum].prestate))
        {
            sensor[snrnum].prestate = sensor[snrnum].state;
            memset(&evtdata, 0, sizeof(evtdata));

            evtdata.Sensor_Number = snrnum;
            evtdata.Entity = 0x13;
            evtdata.Sensor_Device = sensor[snrnum].offset[0];
            strcpy((char *)evtdata.EventData, "PLDM");

            if (sensor[snrnum].state == 6)
                evtdata.Event_Offset = 2;
            else if (sensor[snrnum].state == 5)
                evtdata.Event_Offset = 0;
            else if (sensor[snrnum].state == 8)
                evtdata.Event_Offset = 7;
            else if (sensor[snrnum].state == 9)
                evtdata.Event_Offset = 9;
            else
                evtdata.Event_Offset = 11;

            if (sensor[snrnum].type == SNR_TACHOMETER)
            {
                evtdata.Event_Sensor_Type = 0x04;

                if ((*(addr+21) == 0xFF) && (*(addr+22) == 0xFF))
                {
                    evtdata.Event_Offset = 2;
                    evtdata.Event_Type = 0x07;
                }
                else
                {
                    evtdata.Event_Type = 0x01;
                    evtdata.Event_Offset = (evtdata.Event_Offset >= 7) ? (evtdata.Event_Offset -7): (evtdata.Event_Offset +7);
                }
            }
            else
            {
                evtdata.Event_Sensor_Type = 0x01;
                evtdata.Event_Type = 0x01;

            }

            evtdata.Event_Severity = 0x10;
            evtdata.logtype = LOG_WARNING;
            evt_ptr = event_log(&evtdata.Event_Sensor_Type, 16);
            evt_ptr->interval = (ALERT_INTERVAL/(dpconf->numsent-1));
            evt_ptr->timeout = 0;
            evt_ptr->alertnum = dpconf->numsent;
            OSQPost(EVTQ, evt_ptr);

        }

    }

}
void handle_pldm_tags(INT8U *addr)
{
    INT8U tagerr = 0;
    INT8U i;
    INT8U tblindex;
    INT32U tmptag;
    OS_CPU_SR cpu_sr = 0;
    INT8U *tmpptr;

    if(pldmdataptr->pldmerror)
    {
        pldmdataptr->pldmerror = 0;
        return;
    }

    for (i = 0; i < *(addr+12); i++)
    {
        tblindex = *(addr+13+i*5);

        memcpy(&tmptag, addr+14+i*5, 4);
        if (tmptag == pldmdataptr->tag[tblindex])
        {
            pldmdataptr->xfertag[tblindex] = 0;
            continue;
        }

        pldmdataptr->xfertag[tblindex] = tmptag;
        //memcpy(&pldmdataptr->xfertag[tblindex], addr+14+i*5, 4);

        if (pldmdataptr->xferlen[tblindex] < 4)
        {
            tagerr = 1;
            DEBUGMSG(PLDM_DEBUG, "Get Table %d length zero\n", tblindex);
            break;
        }
        pldmdataptr->chksum[tblindex] = crc32(0 , pldmdataptr->xferptr[tblindex], pldmdataptr->xferlen[tblindex] -4 );
        DEBUGMSG(PLDM_DEBUG, "Table %d chksum 0x%04x(BIOS)/0x%04x(LAN)\n", tblindex, pldmdataptr->xfertag[tblindex], pldmdataptr->chksum[tblindex]);
        if (pldmdataptr->chksum[tblindex] != pldmdataptr->xfertag[tblindex])
        {
            tagerr = 1;
            DEBUGMSG(PLDM_DEBUG,"checksum error\n");
            break;
        }
    }

    if (tagerr)
        return ;


    OS_ENTER_CRITICAL();
    for (i = 0; i < *(addr+12); i++)
    {
        tblindex = *(addr+13+i*5);

        if (pldmdataptr->xfertag[tblindex])
        {
            //4 bytes for CRC32
            pldmdataptr->len[tblindex] = pldmdataptr->xferlen[tblindex]-4 ;
            pldmdataptr->tag[tblindex] = pldmdataptr->xfertag[tblindex];

            tmpptr = pldmdataptr->xferptr[tblindex];
            pldmdataptr->xferptr[tblindex] = pldmdataptr->ptr[tblindex];
            pldmdataptr->ptr[tblindex] = tmpptr;

            pldmdataptr->numwrite[tblindex+1] = pldmdataptr->numwrite[tblindex] + (pldmdataptr->len[tblindex] - 1 + FLASH_WRITE_SIZE) / FLASH_WRITE_SIZE ;

            if (tblindex == 0)
                pldmdataptr->TBL1 = 1;
            else if (tblindex == 1)
                pldmdataptr->TBL2 = 1;
            else if (tblindex == 2)
                pldmdataptr->TBL3 = 1;
        }


    }

    pldmdataptr->valid = 1;
    smbiosrmcpdataptr->PollType = POLL_NONE;
    dirty[PLDMTBL].length = pldmdataptr->numwrite[3];

    setdirty(PLDMTBL);
    OS_EXIT_CRITICAL();
}

void pldm_handle_setgettime(INT8U *addr)
{

    static INT32U rebootnum = 0;

    OS_CPU_SR cpu_sr = 0;
    //set datatime is the first entry for PLDM
    OS_ENTER_CRITICAL();
    smbiosrmcpdataptr->PollType = POLL_STOP;
    ///smbiosrmcpdataptr->BIOSBoot = 1;
    OS_EXIT_CRITICAL();

    //clear the corresponding fields
    memset(&pldmdataptr->xferlen, 0 , 48);
    DEBUGMSG(MESSAGE_DEBUG, "# %u Reboot\n",++rebootnum);
    DEBUGMSG(PLDM_DEBUG,  "Reset PLDM table len %d %d %d\n", pldmdataptr->xferlen[0],pldmdataptr->xferlen[1],pldmdataptr->xferlen[2]);

    *timestampdataptr = *(addr+17);
    *(timestampdataptr + 1) = *(addr+16);
    *(timestampdataptr + 2) = *(addr+15);
    *(timestampdataptr + 3) = *(addr+14);
    *(timestampdataptr + 4) = *(addr+13);
    *(timestampdataptr + 5) = *(addr+12);
    set_time(timestampdataptr);

    //should only use in Revision A
    if (dpconf->vendor != FSC)
        bsp_set_sstate(S_S0);

}
#endif
