#include <stdlib.h>
#include <string.h>
#include "lib.h"
#include "http.h"
#include "bsp_cfg.h"
#include "bsp.h"
#include "wcom.h"
#include "mDNS.h"

#define MaxLen 1536
extern INT8U smbios_flag;
extern sensor_t sensor[MAX_SENSOR_NUMS];
extern flash_data_t dirty_ext[10];
extern unsigned char *smbiosptr;
extern char *idata;
extern DPCONF *dpconf;
extern unsigned char* DASH_OS_PSH_Buf;
extern OS_EVENT *DASH_OS_Push_Event;
extern INT8U *timestampdataptr;

int m_isUpdateSMBIOS = -1; //used for debug : -1(init or Got Same SMBIOS , clear the flag to initial value)
                                                            //-2(Got new SMBIOS start)  1(got all smbios form CMAC) 0(checksum error)

#if CONFIG_mDNS_OFFLOAD_ENABLED
extern struct _mDNSOffloadCmd mDNSOffloadCmd;
extern _IPInfo IPInfo[];
#endif
extern asf_config_t *asfconfig;
#if CONFIG_PCI_Bridge_Test
static int OOBSendTestEnable = 0;
//Stress test use
void ComWinTask(void *p_arg)
{
    int  i = 0;
    unsigned char *data;
    unsigned char *data1;
    unsigned char v1;
    unsigned char v2;
    unsigned char hdrSize = sizeof(OSOOBHdr);
    OSOOBHdr *hdr;
    static int pass = 0;
    static int fail = 0;
    static int HWFail = 0;
    data = malloc(1024);
    data1 = malloc(1024);

    while (1)
    {
        if (!OOBSendTestEnable)
            OSTaskSuspend(OS_PRIO_SELF);

        //Must refill
        hdr = (OSOOBHdr *)data;
        hdr->type = OOB_SEND_TEST;
        hdr->len = 1024 - hdrSize;

        for (i = hdrSize; i < 1024; i++)
            data[i] = data1[i] = (rand() % 256);

        if (bsp_get_inband_data(data, 1024) != PCI_DMA_OK)
        {
            HWFail++;
            OSTaskSuspend(OS_PRIO_SELF);
            continue;
        }

        for (i = hdrSize; i < 1024; i++)
        {
            v1 = data[i];
            v2 = data1[i] + 0x20;
            if (v1 != v2)
            {
                for (i = hdrSize; i < 1024; i++)
                    data[i] = data1[i] = 0;

                fail++;
                OSTaskSuspend(OS_PRIO_SELF);
            }
        }
        pass++;
        OSTimeDly(10);
    }
}
#endif
int read_hm_sensor(char *str,int htype,int index)
{
	    char *xptr,*xptr_end;
              
            xptr=DASH_OS_PSH_Buf + sizeof(OSOOBHdr);  
            xptr=strstr(xptr,str);
            while(xptr)
            {
              if(index >MAX_SENSOR_NUMS-1)
                break;	
              if(htype==SNR_TEMPERATURE)  
                sensor[index].offset[1]=-1;  
              else if(htype==SNR_VOLTAGE)  
                sensor[index].offset[1]=-3; 
              else   
                sensor[index].offset[1]=0;      
              xptr_end=strback(xptr,',');
              xptr=xptr+strlen(str);
              strncpy(sensor[index].name,xptr_end+1,xptr-xptr_end-2);	              
              xptr_end=strchr(xptr,',');
              if(xptr_end)
              {	
                *xptr_end='\0';	
                sscanf(xptr,"%u",&sensor[index].value);
                sensor[index].type=htype;
                index++;
                *xptr_end=',';
                xptr=xptr_end+1;
              }
              xptr=strstr(xptr,str);
            }
            return index;
}            

#if 1
extern int m_whocallSMBIOS;
int m_ShouldChksum , m_CalChksum;

#define SMBIOS_CMAC_SINGLE 1024 //For client tool sending SMBIOS table 
#define SMBIOS_LEN_MAX 8192

void OOBResponseTask(void *p_arg)
{    
    unsigned short raw_len;
    int send_len=0;
	int left_len=0;
    unsigned char *tmpBuf = 0,*tmpBuf1=0 , *tmpBuf_2 = NULL;
    OS_CPU_SR cpu_sr = 0;
#if CONFIG_PCI_Bridge_Test
    unsigned char *testTx;
    unsigned char *testRx;
    OSOOBHdr *txHdr;
#endif
    unsigned int i = 0, t = 0;
    OSOOBHdr *hdr;
    unsigned char *rsBuf;
    int len;
    int tmphdrtype = 0;
    int *ip;
    IPv4ChgInfo* ipv4ChgInfo;
    IPv6ChgInfo* ipv6ChgInfo;
#ifdef CONFIG_PROVISIONING
    ProvRecord *pr;
#endif
    OpaqueData *opaq;
    OPAQDATA* opaqcontent;
    unsigned char *tmpopaqdata = NULL;
    OpaqueData *opaq2;
    unsigned char block = 0;
    unsigned char *ret;
    unsigned short offset = 0;
    unsigned short opaq_len = 0;
    unsigned char err = 0;
    OOBSrvSetting* srv;
    DPCONF *tmpdpconf;
    INT8U provstate;
    INT8U dhcpstatus;
    asf_header_t *asftbl;
    asf_header_t *oldtbl;
#ifdef CONFIG_PROVISIONING
    PortList* portPtr;
    RRList*		RRListPtr;
    unsigned char* tmpPtr;
#endif
    unsigned char *xptr,*xptr_end; 
    unsigned char check_set=0; 
	unsigned char *fwversion;
unsigned int checksum_A , checksum_B;

    if(idata)
      free(idata);
    idata=malloc(MaxLen);
    memset(idata, 0, MaxLen);
    memcpy(idata,(char *) SYSTEM_SW_INFO,MaxLen-1);
    
#if CONFIG_VERSION == IC_VERSION_DP_RevF
    bsp_bits_set(IO_CONFIG1, 1, BIT_FIRMWARERDY, 1);
    //Do firmware update in inband, must call get ip to get correct ip address(Single IP mode)
    if (bsp_in_band_exist() && bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP, 1))
    {
        len = sizeof(OSOOBHdr);
        rsBuf = malloc(len);
        hdr = (OSOOBHdr *)rsBuf;
        hdr->type = OOB_GET_IB_IP;
        hdr->len = 0;
        bsp_get_inband_data(rsBuf, len);
        free(rsBuf);
    }
#elif CONFIG_VERSION >= IC_VERSION_EP_RevA
    bsp_bits_set(MAC_OOBREG, 1, BIT_FIRMWARERDY, 1);
    //Do firmware update in inband, must call get ip to get correct ip address(Single IP mode)
    if (bsp_in_band_exist() && bsp_bits_get(MAC_OOBREG, BIT_SINGLE_IP, 1))
    {
        len = sizeof(OSOOBHdr);
        rsBuf = malloc(len);
        hdr = (OSOOBHdr *)rsBuf;
        hdr->type = OOB_GET_IB_IP;
        hdr->len = 0;
        bsp_get_inband_data(rsBuf, len);
        free(rsBuf);
    }
#endif   

    while (1)
    {
        rsBuf = 0;
        len = 0;
        OSSemPend(DASH_OS_Push_Event, 0, &err);

        #ifdef CONFIG_CPU_SLEEP_ENABLED
	wakeCPU();
	#endif

        hdr = (OSOOBHdr*)DASH_OS_PSH_Buf;
        #if CONFIG_VERSION >= IC_VERSION_EP_RevA
        switch(hdr->hostReqV)
        {
        case 0x02:
            bsp_cmac_send(dpconf, sizeof(*dpconf));
            continue;
        case 0x06:
            bsp_cmac_send(asfconfig, sizeof(*asfconfig));
            continue;
            /*
            case 0x85:
            bsp_bits_set(IO_CONFIG1, 0, BIT_DASHEN, 1);
            len = sizeof(OSOOBHdr);
            rsBuf = malloc(len);

            hdr = (OSOOBHdr *) DASH_OS_PSH_Buf;
            hdr->type = OOB_SET_DPCONF;
            hdr->flag = PCIE_NO_ACK;
            bsp_cmac_send(hdr, sizeof(OSOOBHdr));

            free(rsBuf);
            continue;*/
        default:
            break;
        }
#endif

        //DEBUGMSG(WCOM_DEBUG, "HDR Type: %d %d\n", hdr->type, bsp_bits_get(IO_CONFIG, BIT_SINGLE_IP,1 ));


        switch (hdr->type)
        {
        case OOB_SET_DPCONF:
            provstate = dpconf->ProvisioningState;
            dhcpstatus = dpconf->DHCPv4Enable;

            tmpdpconf = (DPCONF *) (DASH_OS_PSH_Buf +sizeof(OSOOBHdr));
            if (!isprint(tmpdpconf->admin.name[0]))
            {
                memcpy((void *) tmpdpconf->admin.name, "Administrator", 13);
                memcpy((void *) tmpdpconf->admin.passwd, "Realtek", 7);
            }
            tmpdpconf->admin.opt   = 1;
            tmpdpconf->admin.role  = 0x7F;
            tmpdpconf->admin.crc16 = inet_chksum((void *) &tmpdpconf->admin,34);

            if (tmpdpconf->HostIP[eth0].addr != dpconf->HostIP[eth0].addr)
            {
                setIPAddress_F(htonl(tmpdpconf->HostIP[eth0].addr), eth0);
                setSubnetMask_F(htonl(tmpdpconf->SubnetMask[eth0].addr), eth0);
            }

            memcpy(dpconf, tmpdpconf, sizeof(DPCONF));

            setHostName((void *) dpconf->HostName);

            if ((dhcpstatus == 0)  && dpconf->DHCPv4Enable)
            {
                //dpconf->DHCPv4State = InitState;
                setIPAddress_F(0, eth0);
                OSTaskResume(TASK_DHCPv4_PRIO);
            }

#ifdef CONFIG_PROVISIONING
            doprovision(provstate);
#endif

            setwait(DPCONFIGTBL);
            setdirty(DPCONFIGTBL);
            OSTimeDlyResume(TASK_FLASH_PRIO);
            break;

        case OOB_SET_ASFTBL:
            oldtbl = (asf_header_t *) dirty_ext[ASFTBL].addr;
            asftbl = (asf_header_t *) (DASH_OS_PSH_Buf +sizeof(OSOOBHdr));
            if((memcmp(asftbl, "ASF!", 4) == 0) &&  (asftbl->chksum != oldtbl->chksum))
            {
                free(dirty_ext[ASFTBL].addr);
                bsp_set_asftbl(asftbl);
                ParserASFTable();
                dirty_ext[ASFTBL].dirty = 1;
            }
            break;

#ifdef CONFIG_PROVISIONING
        case OOB_SET_PROV:
            pr = (ProvRecord*)( DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            tmphdrtype = OOB_SET_PROV;
            memcpy(dpconf->DomainName, pr->DomainName, MAX_DN_LEN);
            dpconf->ProvisioningSetup = pr->ProvisioningSetup;
            dpconf->ProvisioningMode = pr->ProvisioningMode;
            dpconf->ProvisioningState = pr->ProvisioningState;
            dpconf->PKIEnabled = pr->PKIEnabled;
            dpconf->PSKEnabled = pr->PSKEnabled;
            dpconf->ProvisionServerPort = pr->ProvisionServerPort;
            dpconf->ProvisionServerIP = pr->ProvisionServerIP;
            memcpy(dpconf->ProvisionServerName, pr->ProvisionServerName, MAX_PS_LEN);
            memcpy(&dpconf->pkidata, &pr->pkidata, sizeof(PKIDATA));
            memcpy(&dpconf->pskdata, &pr->pskdata, sizeof(PSKDATA));
            memcpy(dpconf->OTP, pr->OTP, MAX_OTP_LEN);
            break;
        case OOB_REQ_PROV:
            len = sizeof(ProvRecord);
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_REQ_PROV;
            hdr->len = len;
            pr = (ProvRecord *)( rsBuf + sizeof(OSOOBHdr));
            memcpy(pr->DomainName, dpconf->DomainName, MAX_DN_LEN);
            pr->ProvisioningSetup = dpconf->ProvisioningSetup;
            pr->ProvisioningMode = dpconf->ProvisioningMode;
            pr->ProvisioningState = dpconf->ProvisioningState;
            pr->PKIEnabled = dpconf->PKIEnabled;
            pr->PSKEnabled = dpconf->PSKEnabled;
            pr->ProvisionServerPort = dpconf->ProvisionServerPort;
            pr->ProvisionServerIP = dpconf->ProvisionServerIP;
            memcpy((void*)pr->ProvisionServerName, (void*)dpconf->ProvisionServerName, MAX_PS_LEN);
            memcpy(&pr->pkidata, &dpconf->pkidata, sizeof(PKIDATA));
            memcpy(&pr->pskdata, &dpconf->pskdata, sizeof(PSKDATA));
            memcpy(pr->OTP, dpconf->OTP, MAX_OTP_LEN);
            break;
#endif
        case OOB_GET_IP:
            len = 4;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_GET_IP;
            hdr->len = 4;
            ip = (int*)( rsBuf + sizeof(OSOOBHdr));
            *ip = getIPAddress_F(eth0);
            break;
	case OOB_GET_FW_VERSION:
            len = 20;//INT32U*5
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
	     memset(rsBuf,0,sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_GET_FW_VERSION;
            hdr->len = 20;
            fwversion = (char *)( rsBuf + sizeof(OSOOBHdr));
            memcpy(fwversion,&(dpconf->fwMajorVer),4);
	      memcpy(fwversion+4,&(dpconf->fwMinorVer),4);
	      memcpy(fwversion+8,&(dpconf->fwExtraVer),4);
	      memcpy(fwversion+12,&(dpconf->fwBuildVer),4);
		memcpy(fwversion+16,&(dpconf->builddate),4);  	
            break;		

        case OOB_SET_IP:
            ipv4ChgInfo = (IPv4ChgInfo*)( DASH_OS_PSH_Buf + sizeof(OSOOBHdr));

            if (ipv4ChgInfo->IPver == IPv4)
            {
                dpconf->DHCPv4Enable = ipv4ChgInfo->isDHCP;

                memcpy(&i, ipv4ChgInfo->IPv4addr, IPv4_ADR_LEN);
                if (i != getIPAddress_F(eth0))
                {
                    i = ntohl(i);
                    setIPAddress_F(i, eth0);
                }

                memcpy(&i, ipv4ChgInfo->IPv4mask, IPv4_ADR_LEN);
                i = ntohl(i);
                setSubnetMask_F(i, eth0);

                memcpy(&i, ipv4ChgInfo->IPv4Gateway, IPv4_ADR_LEN);
                i = ntohl(i);
                setGateWayIP_F(i, eth0);

                memcpy(&i, ipv4ChgInfo->IPv4DNS, IPv4_ADR_LEN);
                i = ntohl(i);
                setDNSIP_F(i, eth0);
            }
            else
            {
                ipv6ChgInfo = (IPv6ChgInfo*) ipv4ChgInfo;

                if (memcmp(getIPv6Address_F(eth0), ipv6ChgInfo->IPv6addr, IPv6_ADR_LEN) != 0)
                {
                    dpconf->IPv6PrefixLen[eth0] = ipv6ChgInfo->PrefixLen;
                    setIPv6Address_F(ipv6ChgInfo->IPv6addr, eth0);
                    setIPv6DNS_F(ipv6ChgInfo->IPv6DNS, eth0);
                }
            }
            break;
        case OOB_GET_SRV:
            tmphdrtype = OOB_GET_SRV;
            len = sizeof(OOBSrvSetting);
            len  = 4;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_GET_SRV;
            hdr->len = len;
            srv = (OOBSrvSetting*)( rsBuf + sizeof(OSOOBHdr));

            srv->EchoService = dpconf->EchoService;
            srv->httpService = dpconf->httpService;
            srv->httpsService = dpconf->httpsService;
            srv->wsmanService = dpconf->wsmanService;
            DEBUGMSG(WCOM_DEBUG, "Get Srv  ES: %x, HS: %x, HSS: %x, WS: %x\n", srv->EchoService, srv->httpService, srv->httpsService, srv->wsmanService);

            break;
        case OOB_SET_SRV:
            srv = (OOBSrvSetting*)( DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            dpconf->httpService = srv->httpService;
            dpconf->httpsService = srv->httpsService;
            dpconf->wsmanService = srv->wsmanService ;
            dpconf->EchoService = srv->EchoService;
            DEBUGMSG(WCOM_DEBUG,"Set Srv\n");
            break;
#if CONFIG_PCI_Bridge_Test
        case IB_SEND_TEST:
            rsBuf = malloc(sizeof(OSOOBHdr) + hdr->len);
            txHdr = (OSOOBHdr*)rsBuf;
            txHdr->type = IB_SEND_TEST;
            txHdr->len = hdr->len;
            testTx = rsBuf + sizeof(OSOOBHdr);
            testRx = DASH_OS_PSH_Buf + sizeof(OSOOBHdr);
            for ( i = 0; i < hdr->len; i++)
            {
                testTx[i] = testRx[i] + 0x10;
            }
            hdr = (OSOOBHdr*)rsBuf;
            break;
        case OOB_SEND_TEST_START:
            OOBSendTestEnable = 1;
            OSTaskResume(TASK_OOBTest_PRIO);
            break;
        case OOB_SEND_TEST_STOP:
            OOBSendTestEnable = 0;
            break;
#endif

#if CONFIG_mDNS_OFFLOAD_ENABLED
        case IB_mDNSOFFLOAD_RR:
            RRListPtr = (RRList*)(hdr + 1);

            if (RRListPtr->firstFrag)
            {
                mDNSOffloadCmd.numRRRecords = 0;
                mDNSOffloadCmd.curRROffset = 0;

                if (mDNSOffloadCmd.rrRecords)
                {
                    free(mDNSOffloadCmd.rrRecords);
                }

                mDNSOffloadCmd.rrBufferSize = hdr->len - sizeof(RRList);
                mDNSOffloadCmd.rrRecords = (unsigned char*)malloc(mDNSOffloadCmd.rrBufferSize);
            }

            tmpPtr = (unsigned char*)mDNSOffloadCmd.rrRecords;

            memcpy(tmpPtr + mDNSOffloadCmd.curRROffset, RRListPtr + 1, RRListPtr->fragLen);
            mDNSOffloadCmd.curRROffset += RRListPtr->fragLen;

            if (RRListPtr->lastFrag)
            {
                mDNSOffloadCmd.numRRRecords = RRListPtr->numRRRecords;
                //dumpmDNSRR();
                setmDNSOffloadAddr();
            }

            break;
        case IB_mDNSOFFLOAD_UDP_PORT:
            portPtr = (PortList*)(hdr + 1);
            mDNSOffloadCmd.numUDPPorts = portPtr->num;
            if (mDNSOffloadCmd.udpPorts)
            {
                free(mDNSOffloadCmd.udpPorts);
                mDNSOffloadCmd.udpPorts = 0;
            }

            if (portPtr->num)
            {
                mDNSOffloadCmd.udpPorts = (unsigned short*)malloc(portPtr->num * 2);
                memcpy(mDNSOffloadCmd.udpPorts, portPtr->ports, portPtr->num * 2);
            }
            break;
        case IB_mDNSOFFLOAD_TCP_PORT:
            portPtr = (PortList*)(hdr + 1);
            mDNSOffloadCmd.numTCPPorts = portPtr->num;
            if (mDNSOffloadCmd.tcpPorts)
            {
                free(mDNSOffloadCmd.tcpPorts);
                mDNSOffloadCmd.tcpPorts = 0;
            }

            if (portPtr->num)
            {
                mDNSOffloadCmd.tcpPorts = (unsigned short*)malloc(portPtr->num * 2);
                memcpy(mDNSOffloadCmd.tcpPorts, portPtr->ports, portPtr->num * 2);
            }
            mDNSOffloadCmd.linkChangeCount = 0;
            break;
        case IB_mDNSOFFLOAD_MAC_ADDR:
            //Mac Addresss will be restored when wakeup
            ip = (int*)IPInfo[eth0].OOBMAR;
            *ip = REG32(IOREG_IOBASE+IO_IDR0);
            *(ip + 1) = REG32(IOREG_IOBASE+IO_IDR4);

            ip = (int*)(hdr + 1);
            REG32(IOREG_IOBASE+IO_IDR0) = *ip;
            REG32(IOREG_IOBASE+IO_IDR4) = *((unsigned short*)(ip + 1));
            break;
#endif

#ifdef CONFIG_DASH_ENABLED
	case OOB_WMI: 	
	    tmpBuf1=malloc(MaxLen);
	    memset(tmpBuf1,0,MaxLen);
	    OS_ENTER_CRITICAL();
	    memcpy(tmpBuf1,DASH_OS_PSH_Buf + sizeof(OSOOBHdr),1200);
            OS_EXIT_CRITICAL();
	    
	    if(strstr(tmpBuf1,"$$$$"))
	      xptr=tmpBuf1+10;
	    else
	      xptr=tmpBuf1;    
            if(strstr(xptr,"CIM_OperatingSystem="))
            {              
              if(strlen(xptr)>=1190)
              {
                xptr_end=strback(xptr+1190,'#');
                *xptr_end='\0';
              } 
              memset(idata,0,MaxLen);
              memcpy(idata,xptr,strlen(xptr)); 
              if(strlen(idata))
              {  
#if CONFIG_VERSION < IC_VERSION_EP_RevA            	
                dirty_ext[SWTBL].flashaddr = (INT8U*) SYSTEM_SW_INFO;
                dirty_ext[SWTBL].length=strlen(idata);
                dirty_ext[SWTBL].dirty=1;
#else                
                setdirtyaddr(SWTBL, SYSTEM_SW_INFO);
                setdirtylen(SWTBL, strlen(idata), DIRTY_SET);
                setdirty(SWTBL);
#endif               
              }
              dpconf->restart=1;
            }
            free(tmpBuf1);
            break;

        case SMBIOS_WMI:
            if(tmpBuf==NULL)
            {
              tmpBuf = malloc(4096);
              memset(tmpBuf,0,4096);
            }  
			
	 if(tmpBuf_2==NULL)
            {
            	tmpBuf_2 = malloc(4096);
		if(tmpBuf_2){
			 memset(tmpBuf_2,0,4096);	
		}else{
			printf("[RTK] no memory\n");
		}
            }
		
            xptr=DASH_OS_PSH_Buf + sizeof(OSOOBHdr);           
            if(strncmp(xptr,"$$$$",4)==0) 
            { 
               	check_set=1;	
               	send_len=0;	
               	xptr=xptr+8; 	                        
               	raw_len= *xptr+ ((*(xptr+1)) << 8);
		m_isUpdateSMBIOS = -2;
            }  
                        
            OS_ENTER_CRITICAL();


            if((send_len+SMBIOS_CMAC_SINGLE) <= 4096){  
              memcpy(tmpBuf+send_len , DASH_OS_PSH_Buf + sizeof(OSOOBHdr) , SMBIOS_CMAC_SINGLE);

		//printf("[RTK] copy SMBIOS total = %d , copy length = 1200 \n", send_len);
			  
            }
	else if((send_len >= 4096) && ((send_len + SMBIOS_CMAC_SINGLE) < SMBIOS_LEN_MAX)){
            	memcpy(tmpBuf_2 , DASH_OS_PSH_Buf + sizeof(OSOOBHdr) , SMBIOS_CMAC_SINGLE);
	  }
	  else{
	  	//? something wrong
	  }


		
            OS_EXIT_CRITICAL();
            if(!check_set){//? this should not be execuated?
              raw_len=*((unsigned short *)tmpBuf);          
            }
          
           send_len=send_len+SMBIOS_CMAC_SINGLE;   //remove at 20141201

	//m_smbiosLen = raw_len;//3167 , 3167 should include the 10 ( header lengh)
	//m_smbiossendLen =send_len; //4096
			
	if (send_len  >= raw_len && raw_len < SMBIOS_LEN_MAX)		           
            {
		
			
            	smbiosptr = (INT8U *) (timestampdataptr + SMBIOS_DATA_OFFSET);
            	if(check_set)
            	{  
            		
				
	            	m_ShouldChksum = *((unsigned int *)(tmpBuf+4)); //this checksum is calculated by ClientTool
			

			if(raw_len>4086){
				//checksum_A  =  chksum(tmpBuf+10 , 4086) ;
				//checksum_B = chksum(tmpBuf_2 , raw_len - 4086);
				//printf("[RTK] The checksum_A = 0x%x , checksum_b = 0x%x\n",checksum_A,checksum_B);
				m_CalChksum =  chksum(tmpBuf+10 , 4086) + chksum(tmpBuf_2 , raw_len - 4086);
				
			}else{
		m_CalChksum = chksum(tmpBuf+10,raw_len);
			}
				
			
			
				
	            	
	            	 if(m_ShouldChksum != m_CalChksum)
            	  {
				printf("[RTK] The Checksum not matched\n");
					  
	            	  	
            	    free(tmpBuf);
            	    tmpBuf=NULL;
            	    check_set = 0;
            	    send_len=0;
		    m_isUpdateSMBIOS = 0;
            	    break;
            	  }
            	  else
					  	
            	  {
	            	  	
            	    OS_ENTER_CRITICAL();	

				if(raw_len < (4096-10)){			
            	    memcpy(smbiosptr+SMBIOS_DATA_OFFSET+6,tmpBuf+8,2);
                    memcpy(smbiosptr+SMBIOS_HEADER_SIZE,tmpBuf+10,raw_len); 
				}else{

					printf("[RTK] Copy length = %d\n",raw_len );
						
					memcpy(smbiosptr+SMBIOS_DATA_OFFSET+6,tmpBuf+8,2);
		                    	memcpy(smbiosptr+SMBIOS_HEADER_SIZE,tmpBuf+10, 4086);
					memcpy(smbiosptr+SMBIOS_HEADER_SIZE +4086 ,tmpBuf_2, raw_len-4086);			
				}
							
				//m_smbiosLen = raw_len;
				
                    OS_EXIT_CRITICAL();               
                  }             	  
            	}
                else
                {
			
			
                  OS_ENTER_CRITICAL();	
                  memcpy(smbiosptr+SMBIOS_DATA_OFFSET+6,tmpBuf,2);
                  memcpy(smbiosptr+SMBIOS_HEADER_SIZE,tmpBuf+2,raw_len);
                  OS_EXIT_CRITICAL();
                }  
          //      DEBUGMSG(PLDM_DEBUG, "Entering ParserSensor [%s:%d]: %s\n", __FILE__,__LINE__,__func__);

		  
                ParserSensor();
		m_isUpdateSMBIOS = 1;
		m_whocallSMBIOS = 1;
                setdirty(SMBIOSTBL);
		
	
	      if(tmpBuf){
                free(tmpBuf);
                tmpBuf=NULL;
	      }
		  

	    if(tmpBuf_2){
		 free(tmpBuf_2);
	 	tmpBuf_2 = NULL;
	    }
	  
                check_set = 0;
            	send_len=0;                                               
            }
            break;

        case SMBIOS_CHECK:
            if(tmpBuf)
              free(tmpBuf);
            tmpBuf=NULL;
            check_set=0;
            send_len=0;
            
            len = 1;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = SMBIOS_CHECK;
            hdr->len = 1;
	#ifdef DISABLE_SMBIOS_SYNC
	  *(rsBuf + sizeof(OSOOBHdr))=0x01; //tell the client tool , do not send SMBIOS table
	#else
            *(rsBuf + sizeof(OSOOBHdr))=smbios_flag;
	#endif
            break;
            
        case SENSOR_INBAND: 
            len = 1;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = SENSOR_INBAND;
            hdr->len = 1;
 #if CONFIG_SENSOR_BY_AGENT 
            *(rsBuf + sizeof(OSOOBHdr))=1;
 #else
            *(rsBuf + sizeof(OSOOBHdr))=2;
 #endif           
            break;    

        case OOB_REQ_OPAQ_BLO:
            len = 1024;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            memset(rsBuf, 0, sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_REQ_OPAQ_BLO;
            hdr->len = len;
            ret = (unsigned char *)( rsBuf + sizeof(OSOOBHdr));
            *ret = 0;
            //tmpopaqdata = (unsigned char*)malloc(sizeof(OPAQDATA));
            tmpopaqdata = (unsigned char*)malloc(128);
            t = 0;
            for (i = 0; i < 4; i++)
            {
                //read_opaque(i, 0, sizeof(OPAQDATA), tmpopaqdata);
                read_opaque(i, 0, 128, tmpopaqdata);
                if (((OPAQDATA*)tmpopaqdata)->Status == 1)
                {
                    memcpy(ret + sizeof(OPAQDATA)*t, tmpopaqdata, sizeof(OPAQDATA));
                    t++;
                }
            }
            free(tmpopaqdata);
            tmpopaqdata = NULL;
            tmphdrtype = OOB_REQ_OPAQ_BLO;
            break;
        case OOB_OPAQ_CRE_BLO:
            opaqcontent = (OPAQDATA*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            len = 1;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_OPAQ_CRE_BLO;
            hdr->len = len;
            ret = (unsigned char *)( rsBuf + sizeof(OSOOBHdr));
            //tmpopaqdata = (unsigned char*)malloc(sizeof(OPAQDATA));
            tmpopaqdata = (unsigned char*)malloc(128);
            *ret = 1;
            for (i = 0; i < 4; i++)
            {
                //read_opaque(i, 0, sizeof(OPAQDATA), tmpopaqdata);
                read_opaque(i, 0, 128, tmpopaqdata);
                if (((OPAQDATA*)tmpopaqdata)->Status == 0)
                {
                    if (create_opaque(i, 4096, "text", (INT8U*)opaqcontent->ElementName, "Administrator") == 0)
                    {
                        *ret = 0;
                        break;
                    }
                }
            }
            tmphdrtype = OOB_OPAQ_CRE_BLO;
            break;
        case OOB_OPAQ_DEL_BLO:
            opaqcontent = (OPAQDATA*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            len = 1;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_OPAQ_DEL_BLO;
            hdr->len = len;
            ret = (unsigned char *)( rsBuf + sizeof(OSOOBHdr));
            //tmpopaqdata = (unsigned char*)malloc(sizeof(OPAQDATA));
            tmpopaqdata = (unsigned char*)malloc(128);
            *ret = 1;
            for (i = 0; i < 4; i++)
            {
                //read_opaque(i, 0, sizeof(OPAQDATA), tmpopaqdata);
                read_opaque(i, 0, 128, tmpopaqdata);
                if (((OPAQDATA*)tmpopaqdata)->Status == 1 &&
                        strcmp(((OPAQDATA*)tmpopaqdata)->ElementName, opaqcontent->ElementName) == 0)
                {
                    if (delete_opaque(i)  == 0)
                    {
                        *ret = 0;
                        break;
                    }
                }
            }
            tmphdrtype = OOB_OPAQ_DEL_BLO;
            free(tmpopaqdata);
            tmpopaqdata = NULL;
            break;
        case OOB_SET_BLO_NUM:
            opaqcontent = (OPAQDATA*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            //tmpopaqdata = (unsigned char*)malloc(sizeof(OPAQDATA));
            tmpopaqdata = (unsigned char*)malloc(128);
            for (i=0; i<4; i++)
            {
                //read_opaque(i, 0, sizeof(OPAQDATA), tmpopaqdata);
                read_opaque(i, 0, 128, tmpopaqdata);
                if (((OPAQDATA*)tmpopaqdata)->Status == 1 &&
                        strcmp(((OPAQDATA*)tmpopaqdata)->ElementName, opaqcontent->ElementName) == 0)
                {
                    block = i;
                    if (opaqcontent->rw == 1)
                    {
                        ((OPAQDATA*)tmpopaqdata)->dirty = 1;
                        //write_opaque(block, 0, sizeof(OPAQDATA), tmpopaqdata);
                        write_opaque(block, 0, 128, tmpopaqdata);
                    }
                    break;
                }
            }
            free(tmpopaqdata);
            tmpopaqdata = NULL;
            tmphdrtype = OOB_SET_BLO_NUM;
            tmpopaqdata = (unsigned char*)malloc(4096); //for write or read opaque data using
            break;
        case OOB_SET_OPAQ_S:
            opaq = (OpaqueData*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            opaq_len += opaq->len;
            memcpy(tmpopaqdata, opaq->buf, opaq->len);
            tmphdrtype = OOB_SET_OPAQ_S;
            break;
        case OOB_SET_OPAQ_E:
            opaq = (OpaqueData*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            offset = opaq->offset;
            tmphdrtype = OOB_SET_OPAQ_E;
            break;
        case OOB_REQ_OPAQ_S:
            opaq = (OpaqueData*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            tmphdrtype = OOB_REQ_OPAQ_S;
            break;
        case OOB_REQ_OPAQ_E:
            opaq = (OpaqueData*)(DASH_OS_PSH_Buf + sizeof(OSOOBHdr));
            len = sizeof(OpaqueData);
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = OOB_REQ_OPAQ_E;
            hdr->len = len;
            opaq2 = (OpaqueData *)( rsBuf + sizeof(OSOOBHdr));
            memcpy(opaq2->buf, tmpopaqdata+opaq->offset, opaq->len);
            opaq2->len = opaq->len;
            opaq2->offset = opaq->offset;
            opaq2->end = 0;
            if (opaq->end == 1 && tmpopaqdata != NULL)
            {
                opaq2->end = 1;
                free(tmpopaqdata);
                tmpopaqdata = NULL;
            }
            tmphdrtype = OOB_REQ_OPAQ_E;
            break;
        case Get_HM_Sensor:
            len = 1;
            rsBuf = malloc(sizeof(OSOOBHdr) + len);
            hdr = (OSOOBHdr*)rsBuf;
            hdr->type = Get_HM_Sensor;
            hdr->len = 1;
            *(rsBuf + sizeof(OSOOBHdr))=Get_HM_Sensor;
            break;
        case Set_HM_Sensor:
            i=0;
            OS_ENTER_CRITICAL();
            xptr=DASH_OS_PSH_Buf + sizeof(OSOOBHdr);
            xptr=strstr(xptr,"sensor_count=");
            if(xptr)
            {
              xptr=xptr+strlen("sensor_count=");
              xptr_end=strchr(xptr,',');
              if(xptr_end)
              {	
                *xptr_end='\0';	
                sscanf(xptr,"%d",&i);
                dpconf->numofsnr=i;
                *xptr_end=',';
                xptr=xptr_end+1;
              } 
            }  
            else
              break;
              
              
            for(i=0; i<dpconf->numofsnr; i++)
            {
                sensor[i].pollindex=0;
                sensor[i].exist=SNR_EXIST;
                sensor[i].event=0;
                sensor[i].fault=5;
                sensor[i].state=0;
                sensor[i].prestate=0;
                sensor[i].index=i; 
                
                sensor[i].LNC=0;
                sensor[i].UNC=0;
                sensor[i].LC=0;            
                sensor[i].UC=0;
                sensor[i].LF=0;           
                sensor[i].UF=0;               
            }   
            i=0;
            i=read_hm_sensor("temperature=",SNR_TEMPERATURE,i);
            i=read_hm_sensor("FAN=",SNR_TACHOMETER,i);
            i=read_hm_sensor("voltage=",SNR_VOLTAGE,i);
            OS_EXIT_CRITICAL();
            break;    
#endif
        }

        if(hdr->flag == PCIE_NO_ACK)
        {
            //handle non Windows PCI-E bridge condition
            hdr->flag = 0;
            continue;
        }

        for (i = 0; i < DASHOSPSHLEN ; i++)
            DASH_OS_PSH_Buf[i] = 0;

        i = bsp_inband_push_ack();
        if (tmphdrtype == OOB_SET_OPAQ_E)
        {
            //if(write_opaque(block, sizeof(OPAQDATA), opaq_len, tmpopaqdata) != 0)
            if (write_opaque(block, 128+offset, opaq_len, tmpopaqdata) != 0)
            {
            }
            opaq_len = 0;
            free(tmpopaqdata);
            tmpopaqdata = NULL;
        }
        else if (tmphdrtype == OOB_REQ_OPAQ_S)
        {
            //if(read_opaque(block, sizeof(OPAQDATA), 4096-sizeof(OPAQDATA), tmpopaqdata) != 0)
            if (read_opaque(block, 128, 4096-128, tmpopaqdata) != 0)
            {
            }
        }

        //Reponse OOB Request
        if (rsBuf && i == PCI_DMA_OK)
        {
            i = bsp_get_inband_data(rsBuf, sizeof(OSOOBHdr) + hdr->len);
        }

        if (rsBuf)
            free(rsBuf);
        err = 0;

        DEBUGMSG(WCOM_DEBUG, "Complete WCOM\n" );

#ifdef CONFIG_PROVISIONING
        if (tmphdrtype == OOB_SET_PROV)
        {
            if (dpconf->ProvisioningState == FULL_UNPROVISIONED)
            {
                full_unprovision();
                setdirty(DPCONFIGTBL);
                setdirty(USERTBL);
                setdirty(EVENTTBL);
                setreset(EVENTTBL);
                OSTimeDlyResume(TASK_FLASH_PRIO);
            }
            else if (dpconf->ProvisioningState == PARTIAL_UNPROVISIONED)
            {
                partial_unprovision();
                setdirty(DPCONFIGTBL);
                setreset(DPCONFIGTBL);
                OSTimeDlyResume(TASK_FLASH_PRIO);
            }
        }
#endif
    }
}

#endif
