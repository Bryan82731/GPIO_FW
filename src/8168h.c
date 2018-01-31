#include "bsp.h"
#include "lib.h"
#include "arp.h"
#include "ip.h"
#include "tcp.h"
#include "PCIE_HOST.h"
#include <string.h>
#include <stdlib.h>

#define R8168TxdescNumber	8
#define R8168RxdescNumber	8
//extern volatile INT32U pci_bus_scan_status;
//volatile INT32U io_addr;
//volatile INT32U rxdesctmp,txdesctmp;
extern struct Pci_Dev pci_dev;
struct Pg_status *pg_status;
int r8168hRdy = 0;
ROM_EXTERN OS_EVENT *RxQ;
extern _IPInfo IPInfo;
extern DebugMsg *debugmsg;
extern DPCONF *dpconf;
INT8U MLB_DATA=0;
INT8U MLB_RDATA=0;

void r8168h_init();
void DMA_Init();
void r8168h_tx();
void r8168h_ISR();
void ASPM_EN(INT8U type);
void ASPM_Dis();
INT32U ERIread(INT16U Addr, INT32U type);
void ERIwrite(INT16U Addr, INT32U data, INT32U type);
void Ocp_w(INT16U Addr,INT16U data);
INT32U Ocp_r(INT16U Addr);
void Gphy_w(INT16U Addr,INT16U data);
INT32U Gphy_r(INT16U Addr);
void GPHY_Ocp_w(INT16U Addr,INT16U data);
INT32U GPHY_Ocp_r(INT16U Addr);
INT8U CheckDataOK(INT8U *rx, INT8U desc, INT16U len, INT8U protocol,INT16U start);
INT8U MLB_CheckDataOK(INT8U *rx, INT8U desc, INT16U len, INT8U protocol,INT16U start);

INT32U _4281test();
void MLB_r8168h_tx();
void MLB_DMA_Init();
void MLB_r8168h_init();



void EnableISR()
{
	REG32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=REG32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)|0xF; //reset imr;	
}
void Getr8168MacAddr(char *addr)
{
	Read_EP_IODword(pci_dev.io_addr, 0,addr);
	Read_EP_IOWord(pci_dev.io_addr, 4,addr+4);
}

void r8168h_tx1(INT8U *bufaddr, INT32U length, INT8U options, void *freeptr)
{
	struct TxDesc *txdesc, *tmptx;	
	
    INT8U type = (options & 0x0f);
    INT8U v6flag = (options & OPT_IPV6) ;
    INT8U freed = (options & OPT_FREE);
    INT16U count = 0;

    OS_CPU_SR  cpu_sr = 0;


    OS_ENTER_CRITICAL();

	txdesc = (struct TxDesc*)pci_dev.txdesctmp +pci_dev.Txcount % R8168TxdescNumber;
    pci_dev.Txcount = ( pci_dev.Txcount + 1 ) % R8168TxdescNumber;
	

    while (txdesc->OWN == 1 && count < 20000)
    {
        bsp_wait(10);
        count++;
        //wait at most 200 m secs
    }

    if (count == 20000)
    {
		while(1);
    }

    txdesc->BufferAddress = (INT8U *) VA2PA(bufaddr);
    if (type != OPT_NOCS )
    {
        if (type == OPT_UDPCS)
        {
            txdesc->UDP4CS = 1;
            txdesc->TCPCS = 0;
        }
        else if (type == OPT_TCPCS)
        {
            txdesc->TCPCS = 1;
            txdesc->UDP4CS = 0;
        }
        else {
            txdesc->UDP4CS =  0;
            txdesc->TCPCS = 0;
        }
        if (v6flag)
            txdesc->IPV4CS = 0;
        else
            txdesc->IPV4CS = 1;
    }
    else
    {
        txdesc->UDP4CS = 0;
        txdesc->TCPCS = 0;
        txdesc->IPV4CS = 0;
    }

    if (v6flag == OPT_IPV6)
    {
        txdesc->IPV6CS = 1;
        txdesc->TCPHO = 54;
        //Ethernet header len(14) + IPv6 header len(40)
    }
    else
        txdesc->IPV6CS = 0;

    txdesc->FS  = 1;
    txdesc->LS  = 1;
    txdesc->Length = length;
    txdesc->OWN = 1;


    //TxPolling
   Write_EP_IOByte(pci_dev.io_addr,0x38,0x40);		//set polling bit

    OS_EXIT_CRITICAL();

    if (freed)
    {
        //wait until DMA complete, for non-TCP packets
        while (txdesc->OWN)
            bsp_wait(1);

        //may only need to wait, but reuse the buffer without free
        if (freeptr)
            free(freeptr);
    }

	return;
	
	txdesc = (struct TxDesc*)pci_dev.txdesctmp +pci_dev.Txcount % R8168TxdescNumber;
	if (txdesc->OWN==0){			
		txdesc->FS=1;
		txdesc->Length=64+pci_dev.Txcount%(1500-64);
		txdesc->LS=1;
		txdesc->OWN=1;
		Write_EP_IOByte(pci_dev.io_addr,0x38,0x40);		//set polling bit
		pci_dev.Txcount++;
	}
}

#define R8168TxdescNumber	8
#define R8168RxdescNumber	8
extern struct Pci_Dev pci_dev;
void rx_recyclewifi(void)
{

    INT8U err;
    //INT8U cnt = 0;
	struct RxDesc *rxdesc68H;
    volatile rxdesc_t *rxdesc;	
    volatile INT8U *ptr, *newptr, *tmpptr;
    EthHdr *ethdr;
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
    RxQMsg *tmpmsg;
#endif

	
    rxdesc68H = (struct RxDesc *) pci_dev.rxdesctmp +pci_dev.Rxcount%R8168RxdescNumber;
	rxdesc = (rxdesc_t *) rxdesc68H;
		
    while (!rxdesc->offset0.st.own)
    {
		//printf("rx_recyclewifi 1\n");

        ptr = (INT8U *) PA2VA(rxdesc->rx_buf_addr_low-PKT_ALIGN_SIZE);
        ethdr = (EthHdr *) ptr;
		
        debugmsg->RxTotal++;
		
		//8168h will not enable rxdesc->offset0.st.v4f
        if (ethdr->ethType == htons(ETHERNET_TYPE_IPv4) && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *ptr = V4_PKT;
        else if (ethdr->ethType == htons(ETHERNET_TYPE_IPv6) && dpconf->IPv6Enable && !IPInfo.IPv6Reset)
            *ptr = V6_PKT;
        else if (ethdr->ethType == htons(ARP_Protocol) && dpconf->IPv4Enable && !IPInfo.IPv4Reset)
            *ptr = ARP_PKT;
        else if (ethdr->ethType == htons(EAP8021x_Protocol))
            *ptr = EAP_PKT;
        else
        {
            debugmsg->DrvDrop++;
            *ptr = INVALID;	
printf("rx_recyclewifi INVALID\n");			
        }

        *(ptr + rxdesc->offset0.st.length -2) = '\0';

        if (*ptr == INVALID)
            DEBUGMSG(DRIVER_DEBUG, "packet with %d invalid\n",rxdesc->offset0.st.length);

        //if invalid => drop the packet
        if (*ptr != INVALID)
        {
            if(rxdesc->offset0.st.length <= 128)
            {
                newptr = malloc(rxdesc->offset0.st.length);
                memcpy(newptr, ptr, rxdesc->offset0.st.length);
                tmpptr = ptr;
                ptr = newptr;
                newptr = tmpptr;

            }
            else
            {
                newptr = malloc(MAX_PKT_SIZE);
            }
            if (newptr != NULL)
            {
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                tmpmsg = malloc(sizeof(RxQMsg));
                tmpmsg->ptr = ptr;
                tmpmsg->length = rxdesc->offset0.st.length;
#endif
                DEBUGMSG(DRIVER_DEBUG, "packet with %d received\n",rxdesc->offset0.st.length);
                rxdesc->rx_buf_addr_low = (INT8U *) VA2PA(newptr) + PKT_ALIGN_SIZE;
#ifdef CONFIG_PROFILING
                mgtcycles[1] = rlx_cp3_get_counter_lo(0) ;
#endif

#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                err = OSQPost(RxQ, tmpmsg);
#else
				*(ptr + 1) = wlan0;
                err = OSQPost(RxQ, ptr);
#endif

                //if queue is full
                if (err == OS_ERR_Q_FULL)
                {
                    DEBUGMSG(DRIVER_DEBUG, "packet droped %d with QFULL\n",rxdesc->offset0.st.length);
                    free(ptr);
#ifdef CONFIG_WAKE_UP_PATTERN_ENABLED
                    free(tmpmsg);
#endif
                }

            }
            else
            {
                debugmsg->DrvBusy++;
                DEBUGMSG(DRIVER_DEBUG,"packet droped %d with Memory empty\n",rxdesc->offset0.st.length);
            }
            //if full, drop the packet
        }
		rxdesc->offset0.cmd.buffer_size = BUFF_SIZE;
        rxdesc->offset0.cmd.own = 1; 

		pci_dev.Rxcount = (pci_dev.Rxcount + 1)%R8168RxdescNumber;
        
		rxdesc68H = (struct RxDesc *) pci_dev.rxdesctmp +pci_dev.Rxcount;
		rxdesc = (rxdesc_t *) rxdesc68H;

    }

}


void r8168h_init()
{
	volatile INT32U Rvalue,ph_result;
	//volatile INT32U memio_addr;
	volatile USHORT bus=0,dev=0,fun=0;
	char addr[6];


	ph_result=0;
	//memio_addr=0;
	//Device_Initial();
	//pci_dev.pci_bus_scan_status=Inital_Adapter_cfg();
	//pci_dev.Vendor_cfg[0][0]=0x816510EC;
	//pci_dev.pci_bus_scan_status=Inital_Adapter_cfg();

	//get MAC IO baseaddr
	Read_EP_PCIDword(bus,dev,fun,0x10,&Rvalue);
	pci_dev.io_addr=Rvalue-1; 


	ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,0x18,&Rvalue);
	//memio_addr=Rvalue & 0xFFFFFFF0;

	Read_EP_IOByte(pci_dev.io_addr,0xD3,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0xD3,Rvalue&0x7F);	//turn off 0xD3 bit7 now_is_oob
	//cfg devcie mac io
	Write_EP_IODword(pci_dev.io_addr,0x60,0x80000000|BIT_12);
	do{
		Rvalue=GPHY_Ocp_r(0xA420)&0x7;
	}while(Rvalue!=0x3);
	Write_EP_IODword(pci_dev.io_addr,0x60,0x80000000|BIT_15|BIT_12);
	while((Gphy_r(0)&BIT_15)==BIT_15);
	//WIFIDASH_DLY(0x500000);	
	Write_EP_IOByte(pci_dev.io_addr,0x47,0);
	Read_EP_IOByte(pci_dev.io_addr,0x50,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0x50,Rvalue|0xC0);
	Read_EP_IOByte(pci_dev.io_addr,0x52,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0x52,Rvalue|0x20);
	Write_EP_IOWord(pci_dev.io_addr,0xDA,0x2401);
	Write_EP_IOByte(pci_dev.io_addr,0x37,0x10);
	//Write_EP_IODword(io_addr,0x40,0x3f000380);   //max request size 32DW 
	Write_EP_IODword(pci_dev.io_addr,0x40,0x3f000780);
	//Write_EP_IODword(pci_dev.io_addr,0x44,0x01008f0e);
	Write_EP_IODword(pci_dev.io_addr,0x44,0x0100cf0e);//must enable single fetch
	Write_EP_IOByte(pci_dev.io_addr,0xEC,0x27);
	Write_EP_IOWord(pci_dev.io_addr,0x3E,0xFFFF);
	Write_EP_IOWord(pci_dev.io_addr,0x3C,0x1bf);
}
void MLB_r8168h_init()
{
	volatile INT32U Rvalue,ph_result;
	//volatile INT32U memio_addr;
	volatile USHORT bus=0,dev=0,fun=0;
	char addr[6];


	ph_result=0;

	Read_EP_PCIDword(bus,dev,fun,0x10,&Rvalue);
	pci_dev.io_addr=Rvalue-1; 


	ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,0x18,&Rvalue);

	Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
	Rvalue =Rvalue & 0xF3;
	Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx disalbe

	Read_EP_IOByte(pci_dev.io_addr,0xD3,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0xD3,Rvalue&0x7F);	//turn off 0xD3 bit7 now_is_oob
	//cfg devcie mac io	
	Write_EP_IOByte(pci_dev.io_addr,0x47,0);
	Read_EP_IOByte(pci_dev.io_addr,0x50,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0x50,Rvalue|0xC0);
	Read_EP_IOByte(pci_dev.io_addr,0x52,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0x52,Rvalue|0x20);
	Write_EP_IOWord(pci_dev.io_addr,0xDA,0x2401);
	Write_EP_IOByte(pci_dev.io_addr,0x37,0x10);
	//Write_EP_IODword(io_addr,0x40,0x3f000380);   //max request size 32DW 
	Write_EP_IODword(pci_dev.io_addr,0x40,0x3f000780);
	//Write_EP_IODword(pci_dev.io_addr,0x44,0x01008f0e);
	Write_EP_IODword(pci_dev.io_addr,0x44,0x0100cf0e);//must enable single fetch
	Write_EP_IOByte(pci_dev.io_addr,0xEC,0x27);
	Read_EP_IOByte(pci_dev.io_addr,0x42,&Rvalue);
	Write_EP_IOByte(pci_dev.io_addr,0x42,Rvalue|0x2);
	Read_EP_IOByte(pci_dev.io_addr,0x6c,&Rvalue);
	
	Write_EP_IOWord(pci_dev.io_addr,0x3E,0xFFFF);
	Write_EP_IOWord(pci_dev.io_addr,0x3C,0x1bf);
}
void MLB_DMA_Init(){
	volatile INT32U Rvalue,i,diff;
	INT8U tx_packet[14]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x4c, 0x68, 0x00, 0x05, 0x02, 0x4b};
	//volatile INT32U *rxdesctmp,*txdesctmp;
	struct RxDesc *rxdesc;
	struct TxDesc *txdesc;
	char *tmp;
	
	//init rx DMA;
	if(!pci_dev.rxdesctmp)
	{
		pci_dev.rxdesctmp=malloc(R8168RxdescNumber*sizeof(struct RxDesc)+256);
		memset((void*)pci_dev.rxdesctmp,0,R8168RxdescNumber*sizeof(struct RxDesc)+256);
		diff=((pci_dev.rxdesctmp+0xFF)&0xFFFFFF00)-pci_dev.rxdesctmp;
		pci_dev.rxdesctmp=pci_dev.rxdesctmp+diff;
	}
	
	Write_EP_IODword(pci_dev.io_addr,0xE4,VA2PA((INT32U)pci_dev.rxdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0xE8,0x0);	
	for(i=0;i<R8168RxdescNumber;i++){
		rxdesc=(struct RxDesc*)pci_dev.rxdesctmp +i;
		tmp = malloc(1520);
		memset(tmp, 0 ,1520);
		rxdesc->BufferAddress = VA2PA(tmp)+PKT_ALIGN_SIZE;
		rxdesc->BufferAddressHigh = 0;
		rxdesc->Length=1520;
		if(i==7)rxdesc->EOR=1;
		else rxdesc->EOR=0;
		rxdesc->OWN=1;
		rxdesc->HDF=0;
		rxdesc->BufferAddressHigh_Header =0;
		rxdesc->BufferAddress_Header=VA2PA(malloc(50));
		rxdesc->Length_Header=50;
	}


	//test tx DMA Init;
	if(!pci_dev.txdesctmp)
	{
		pci_dev.txdesctmp =(INT32U) malloc(R8168TxdescNumber*sizeof(struct TxDesc)+256);
		memset((void*)pci_dev.txdesctmp,0,R8168TxdescNumber*sizeof(struct TxDesc)+256);
		diff=((pci_dev.txdesctmp+0xFF)&0xFFFFFF00)-pci_dev.txdesctmp;
		pci_dev.txdesctmp=pci_dev.txdesctmp+diff;

	}
	Write_EP_IODword(pci_dev.io_addr,0x20,VA2PA((INT32U)pci_dev.txdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0x24,0x0);
	Read_EP_IOByte(pci_dev.io_addr,0x20,&Rvalue);
	for(i=0;i<R8168TxdescNumber;i++){
		txdesc = (struct TxDesc*)pci_dev.txdesctmp +i;
		//txdesc->BufferAddress=PH_allocate_mem_DMRAM(2000)-0x80000000;
		txdesc->BufferAddress=VA2PA((INT32U)malloc(1520));
		if (i==7)	txdesc->EOR=1;
		else txdesc->EOR = 0;
		txdesc->BufferAddressHigh=0;
		memcpy((void*)(PA2VA(txdesc->BufferAddress)),(void*)tx_packet,14);
	}
	pci_dev.Txcount=0;
	pci_dev.Rxcount=0;
	Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
	Rvalue =Rvalue|0x0C;
	Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx enalbe
	r8168hRdy = 1;
}

void MLB_DMA_Init2(){
	volatile INT32U Rvalue,i,diff;
	INT8U tx_packet[14]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x4c, 0x68, 0x00, 0x05, 0x02, 0x4b};
	//volatile INT32U *rxdesctmp,*txdesctmp;
	struct RxDesc *rxdesc;
	struct TxDesc *txdesc;
	char *tmp;

	Write_EP_IODword(pci_dev.io_addr,0xE4,VA2PA((INT32U)pci_dev.rxdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0xE8,0x0);	
	for(i=0;i<R8168RxdescNumber;i++){
		rxdesc=(struct RxDesc*)pci_dev.rxdesctmp +i;
		rxdesc->Length=1520;
		if(i==7)rxdesc->EOR=1;
		else rxdesc->EOR=0;
		rxdesc->OWN=1;
		rxdesc->HDF=0;
		rxdesc->Length_Header=50;
	}

	Write_EP_IODword(pci_dev.io_addr,0x20,VA2PA((INT32U)pci_dev.txdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0x24,0x0);
	Read_EP_IOByte(pci_dev.io_addr,0x20,&Rvalue);
	for(i=0;i<R8168TxdescNumber;i++){
		txdesc = (struct TxDesc*)pci_dev.txdesctmp +i;
		txdesc->OWN=0;
		if (i==7)	txdesc->EOR=1;
		else txdesc->EOR = 0;

	}
	pci_dev.Txcount=0;
	pci_dev.Rxcount=0;
	pci_dev.WrongCount=0;
	Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
	Rvalue =Rvalue|0x0C;
	Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx enalbe
	r8168hRdy = 1;
}

void DMA_Init(){
	volatile INT32U Rvalue,i,diff;
	INT8U tx_packet[14]={0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0xe0, 0x4c, 0x68, 0x00, 0x05, 0x02, 0x4b};
	//volatile INT32U *rxdesctmp,*txdesctmp;
	struct RxDesc *rxdesc;
	struct TxDesc *txdesc;
	char *tmp;
	
	//init r8168h hook function
	GetPCIMacAddr = Getr8168MacAddr;
	pci_tx = r8168h_tx1;


	//init rx DMA;
	pci_dev.rxdesctmp=malloc(R8168RxdescNumber*sizeof(struct RxDesc)+256);
	memset((void*)pci_dev.rxdesctmp,0,R8168RxdescNumber*sizeof(struct RxDesc)+256);
	diff=((pci_dev.rxdesctmp+0xFF)&0xFFFFFF00)-pci_dev.rxdesctmp;
	pci_dev.rxdesctmp=pci_dev.rxdesctmp+diff;
	
	Write_EP_IODword(pci_dev.io_addr,0xE4,VA2PA((INT32U)pci_dev.rxdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0xE8,0x0);	
	for(i=0;i<R8168RxdescNumber;i++){
		rxdesc=(struct RxDesc*)pci_dev.rxdesctmp +i;
		tmp = malloc(1520);
		memset(tmp, 0 ,1520);
		rxdesc->BufferAddress = VA2PA(tmp)+PKT_ALIGN_SIZE;
		rxdesc->BufferAddressHigh = 0;
		rxdesc->Length=1520;
		if(i==7)rxdesc->EOR=1;
		else rxdesc->EOR=0;
		rxdesc->OWN=1;
		rxdesc->HDF=0;
		rxdesc->BufferAddressHigh_Header =0;
		rxdesc->BufferAddress_Header=VA2PA(malloc(50));
		rxdesc->Length_Header=50;
	}


	//test tx DMA Init;
	pci_dev.txdesctmp =(INT32U) malloc(R8168TxdescNumber*sizeof(struct TxDesc)+256);
	memset((void*)pci_dev.txdesctmp,0,R8168TxdescNumber*sizeof(struct TxDesc)+256);
	diff=((pci_dev.txdesctmp+0xFF)&0xFFFFFF00)-pci_dev.txdesctmp;
	pci_dev.txdesctmp=pci_dev.txdesctmp+diff;
	Write_EP_IODword(pci_dev.io_addr,0x20,VA2PA((INT32U)pci_dev.txdesctmp));
	Write_EP_IODword(pci_dev.io_addr,0x24,0x0);
	Read_EP_IOByte(pci_dev.io_addr,0x20,&Rvalue);
	for(i=0;i<R8168TxdescNumber;i++){
		txdesc = (struct TxDesc*)pci_dev.txdesctmp +i;
		//txdesc->BufferAddress=PH_allocate_mem_DMRAM(2000)-0x80000000;
		txdesc->BufferAddress=VA2PA((INT32U)malloc(1520));
		if (i==7)	txdesc->EOR=1;
		else txdesc->EOR = 0;
		txdesc->BufferAddressHigh=0;
		memcpy((void*)(PA2VA(txdesc->BufferAddress)),(void*)tx_packet,14);
	}
	pci_dev.Txcount=0;
	pci_dev.Rxcount=0;
	Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
	Rvalue =Rvalue|0x0C;
	Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx enalbe
	Write_EP_IODword(pci_dev.io_addr,0x60,0x80009200);
	r8168hRdy = 1;
}
int rxflag = 0;
void MLB_r8168h_tx()
{
	struct TxDesc *txdesc;
	volatile INT32U i;
	txdesc = (struct TxDesc*)pci_dev.txdesctmp +pci_dev.Txcount % R8168TxdescNumber;
	if (txdesc->OWN==0){			
		txdesc->FS=1;
		txdesc->Length=1500;
		for(i=0;i<1500-14;i++){
			*((INT8U *)(PA2VA(txdesc->BufferAddress))+14+i)=MLB_DATA;
			MLB_DATA++;
		}
		txdesc->LS=1;
		txdesc->OWN=1;
		Write_EP_IOByte(pci_dev.io_addr,0x38,0x40);		//set polling bit
		pci_dev.Txcount++;
	}
}

void r8168h_tx()
{
	struct TxDesc *txdesc;
	if(rxflag == 0)
		return;
	else
		rxflag = 0;
	txdesc = (struct TxDesc*)pci_dev.txdesctmp +pci_dev.Txcount % R8168TxdescNumber;
	if (txdesc->OWN==0){			
		txdesc->FS=1;
		txdesc->Length=64+pci_dev.Txcount%(1500-64);
		txdesc->LS=1;
		txdesc->OWN=1;
		Write_EP_IOByte(pci_dev.io_addr,0x38,0x40);		//set polling bit
		pci_dev.Txcount++;
	}
}
INT32U _4281test(){
	volatile INT32U i;
	//8111h mac loop back init
	MLB_r8168h_init();
	//DMA init
	//if(pcie_rst_f_risc_mode==0){
		MLB_DMA_Init();
	//}
	//else{
	//	MLB_DMA_Init2();
	//}
	//tx
	for(i=0;i<500;i++){
		MLB_r8168h_tx();
	}
	for(i=0;i<100000;i++){
		if(pci_dev.Rxcount==500&&pci_dev.WrongCount==0)	return 1;
		else if(pci_dev.WrongCount!=0) return 0;
	}
	return 0;
}

void r8168h_ISR(){
	volatile INT32U ISR,IMR,phresult;
	INT8U *rxdescVA;
	struct RxDesc *rxdesc;
	Read_EP_IOWord(pci_dev.io_addr,0x3c,&IMR);
	Write_EP_IOWord(pci_dev.io_addr,0x3c,0x0);
	Read_EP_IOWord(pci_dev.io_addr,0x3E,&ISR);
	Write_EP_IOWord(pci_dev.io_addr,0x3E,ISR&IMR);
	//tmp=ISR & (BIT_0|BIT_4);
	//r8168h rx
	if((ISR & (BIT_0|BIT_4))!=0){
		//rx_recyclewifi();
		#if 1
		rxdesc=(struct RxDesc*)pci_dev.rxdesctmp +pci_dev.Rxcount%R8168RxdescNumber;
		while(rxdesc->OWN==0){
		//printk("**********Rx!**********\r\n");
			rxdescVA=(INT8U *)PA2VA(rxdesc->BufferAddress); 
			phresult=MLB_CheckDataOK(rxdescVA,pci_dev.Rxcount%8,rxdesc->Length-4,0,14);
			if (phresult == 0) {
				pci_dev.WrongCount++;
				//printk("rx check data error!\n");
				//REG32(0xbafa0b04)=REG32(0xbafa0b04)|0x80000000;
			}
			rxdesc->Length=1520;
			rxdesc->OWN=1;
			pci_dev.Rxcount++;
			rxdesc=(struct RxDesc*)pci_dev.rxdesctmp +pci_dev.Rxcount%R8168RxdescNumber;
			//rxflag = 1;
			//printk("**********Tx!**********\r\n");		
			//r8168h_tx();
		}
		#endif		
	}
	Write_EP_IOWord(pci_dev.io_addr,0x3c,IMR);
	//printf("r8168h_ISR<<\n");

}
INT8U MLB_CheckDataOK(INT8U *rx, INT8U desc, INT16U len, INT8U protocol,INT16U start)
{
	INT16U i;
	INT8U val=rx[start];		//Mark-14
	for(i=start; i<len; i++){
		if(rx[i]!=MLB_RDATA){			
			pci_dev.WrongData=i;			
	  	 	pci_dev.WrongDesc=desc;
	   		return 0;
		}
		else MLB_RDATA++;
	}
  return 1;
}


INT8U CheckDataOK(INT8U *rx, INT8U desc, INT16U len, INT8U protocol,INT16U start)
{
	INT16U i;
	INT8U val=rx[start];		//Mark-14
	/*for(i=0;i<12;i++){
		if ((i<6 && rx[i]!=0xFF)|| (i>5 && rx[i]!=0x00)){
			pci_dev.WrongData=i;
			pci_dev.WrongDesc=desc;
			return 0;
		}

	}*/
	for(i=start; i<len; i++){
		if(rx[i]!=val++){
			if((rx[i]!=0x20 )&&(rx[i]!=0x0)){
				switch(protocol){
					case 0:		//normal packets
						pci_dev.WrongData=i;
						i=i+14;
						break;
				
					case 1:		//TCP/IP packets
						//Adapter->WrongData=i+54;
						pci_dev.WrongData=i;
						break;
								
					case 2:		//UDP/IP packets
						//Adapter->WrongData=i+42;
						pci_dev.WrongData=i;
						break;
	
					case 3:		//IP packets
						pci_dev.WrongData=i;
						//Adapter->WrongData=i+34;
						break;						
				}
		  	}
			else
				pci_dev.WrongData=i;
			
	  	 	pci_dev.WrongDesc=desc;
	   		return 0;
		}	 		
	}
  return 1;
}

void bypass_test(){
	pg_status=(struct Pg_status *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET);
	//REG8(0xBAF10040)=0x03;
	Fun0_Initial();
	do{
	}while(!(REG8(0xBAF1004C)& BIT_0));
	//Write_Vt_PCIDword(0,5,0x816710ec);
	//Cfg_Bar_VD2VT(0,0,0,5);

	_2_Bypass();
}

void bypass_test_cfg(){
	pg_status=(struct Pg_status *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET);
	pci_dev.Cdm_cfg_info=(struct CDM_MBOX_CFG*)(WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET);
	REG8(0xBAF10040)=0x03;
	Fun0_Initial();
	do{
	}while(!(REG8(0xBAF1004C)& BIT_0));
	
	RC_INDI_Initial_cfg();
	//ASPM_EN(2);
	//_2_4281_cfg();
	_2_Bypass_WO_cfg();
	//Write_Vt_PCIDword(0,5,0x816710ec);
	//Cfg_Bar_VD2VT(0,0,0,5);
	//_2_Bypass();

}

void Bypass_mode3(){
	volatile INT32U tmd,ph_result;
	ph_result=0;
	Inital_Adapter();	
	Write_Vt_PCIDword(0,2,0xFFFFFFFF);
	Write_Vt_PCIDword(0,3,0xFFFFFFFF);
	Write_Vt_PCIDword(0,4,0xFFFFFFFF);
	Write_Vt_PCIDword(0,7,0xFFFFFFFF);	
	//Write_Vt_PCIDword(0,1,0x816810ec);
	Write_Vt_PCIDword(0,1,0xFFFFFFFF);	
	Read_Vt_PCIDword(4,5,&tmd);
	Write_Vt_PCIDword(4,5,tmd&0xFFFFFFF8);
	Read_Vt_PCIDword(4,5,&tmd);
	Write_Vt_PCIDword(4,5,tmd|0x3);
	ph_result=ph_result|VTcfg_2_VDcfg(0,0,0,5);
	_2_Bypass();
	//move virtual cfg to vendor cfg
}
//Type 0:disable;	1:L0s;	2:L1;	3:L0s+L1
void ASPM_EN(INT8U type){
	volatile INT32U ph_result=0,tmd;
	switch (type){
	case 0:
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd&0xFE);
		
		ph_result =ph_result| Read_RC_PCIDword(0x80,&tmd);
		ph_result =ph_result| Write_RC_PCIDword(0x80,tmd&0xFFFFFFFC);
		
		ph_result =ph_result| Read_EP_PCIByte(0,0,0,0x80,&tmd);
		ph_result =ph_result| Write_EP_PCIByte(0,0,0,0x80,tmd&0xFC);

		break;
	case 1: 
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd|0x1);
		
		ph_result =ph_result| Read_RC_PCIDword(0x80,&tmd);
		tmd=tmd|0x1;
		tmd=tmd&0xFFFFFFFD;
		ph_result =ph_result| Write_RC_PCIDword(0x80,tmd);

		ph_result =ph_result| Read_EP_PCIByte(0,0,0,0x80,&tmd);
		tmd=tmd|0x1;
		tmd=tmd&0xFD;
		ph_result =ph_result| Write_EP_PCIByte(0,0,0,0x80,tmd);		

		break;
	case 2:	
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd|0x1);
		
		ph_result =ph_result| Read_RC_PCIDword(0x80,&tmd);
		tmd=tmd|0x2;
		tmd=tmd&0xFFFFFFFE;
		ph_result =ph_result| Write_RC_PCIDword(0x80,tmd);

		ph_result =ph_result| Read_EP_PCIByte(0,0,0,0x80,&tmd);
		tmd=tmd|0x2;
		tmd=tmd&0xFE;
		ph_result =ph_result| Write_EP_PCIByte(0,0,0,0x80,tmd);
		
		break;
	case 3:
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd|0x1);
		
		ph_result =ph_result| Read_RC_PCIDword(0x80,&tmd);
		ph_result =ph_result| Write_RC_PCIDword(0x80,tmd|0x3);
		
		ph_result =ph_result| Read_EP_PCIByte(0,0,0,0x80,&tmd);
		ph_result =ph_result| Write_EP_PCIByte(0,0,0,0x80,tmd|0x3);

		break;
	case 4:
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd|0x1);
		
		ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x53,&tmd);
		ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x53,tmd|0x80);
		
		ph_result =ph_result| Write_EP_IODword(pci_dev.io_addr,0x70,0x01);
		ph_result =ph_result| Write_EP_IODword(pci_dev.io_addr,0x70,0x8000F2FC);
		
		ph_result =ph_result| Read_RC_PCIDword(0x80,&tmd);
		ph_result =ph_result| Write_RC_PCIDword(0x80,tmd|0x42);
		
		ph_result =ph_result| Read_EP_PCIWord(0,0,0,0x80,&tmd);
		ph_result =ph_result| Write_EP_PCIWord(0,0,0,0x80,tmd|0x0142);		
		break;
	default: break;
	
	}

}

void ASPM_Dis(){
	volatile INT32U ph_result=0,tmd;
	ph_result = Read_RC_PCIDword(0x80,&tmd);
	ph_result = Write_RC_PCIDword(0x80,tmd&0xFC);
	ph_result = Read_EP_PCIByte(0,0,0,0x80,&tmd);
	ph_result = Write_EP_PCIByte(0,0,0,0x80,tmd&0xFC);
	ph_result =ph_result| Read_EP_IOByte(pci_dev.io_addr,0x56,&tmd);
	ph_result =ph_result| Write_EP_IOByte(pci_dev.io_addr,0x56,tmd&0xFE);
}

INT32U ERIread(INT16U Addr, INT32U type)
{
	INT32U i;
	type=type<<16;
	Write_EP_IODword(pci_dev.io_addr,0x74,Addr|0x0000f000|type);	
	do{
		Read_EP_IODword(pci_dev.io_addr,0x74,&i);
	}while ((i & 0x80000000)==0);
	Read_EP_IODword(pci_dev.io_addr,0x70,&i);		
	return i;
}

void ERIwrite(INT16U Addr, INT32U data, INT32U type)
{
	INT32U tmp,Rvalue;
	
	type=type<<16;
	Write_EP_IODword(pci_dev.io_addr,0x70,data);	
	Write_EP_IODword(pci_dev.io_addr,0x74,Addr|0x8000f000|type);
	do
	{
		tmp++;
		Read_EP_IODword(pci_dev.io_addr,0x74,&Rvalue);
	}while (((Rvalue & 0x80000000)==0) && (tmp<100));
			
}

void Ocp_w(INT16U Addr,INT16U data){
	INT32U Ddata=0, tmp=0;
	Addr=Addr/2;
	Ddata=Addr|0x8000;
	Ddata=Ddata<<16;
	Ddata=Ddata+data;
	Write_EP_IODword(pci_dev.io_addr,0xB0,Ddata);
}
INT32U Ocp_r(INT16U Addr){
	INT32U Rvalue;
	Addr=Addr/2;
	Write_EP_IODword(pci_dev.io_addr,0xB0,(INT32U)Addr<<16);
	WIFIDASH_DLY(1000);
	Read_EP_IOWord(pci_dev.io_addr,0xB0,&Rvalue);
	return Rvalue;
}
void GPHY_Ocp_w(INT16U Addr,INT16U data){
	INT32U Ddata=0,Rvalue;
	Addr=Addr/2;
	Ddata=Addr|0x8000;
	Ddata=Ddata<<16;
	Ddata=Ddata+data;
	Write_EP_IODword(pci_dev.io_addr,0xB8,Ddata);
	do{
		Read_EP_IODword(pci_dev.io_addr,0xB8,&Rvalue);		
	}while((Rvalue&BIT_31)==BIT_31);

}
INT32U GPHY_Ocp_r(INT16U Addr){
	INT32U Rvalue,Ddata;
	Addr=Addr/2;
	Ddata=Addr;
	Ddata=Ddata<<16;
	Write_EP_IODword(pci_dev.io_addr,0xB8,Ddata);
	do{
		Read_EP_IODword(pci_dev.io_addr,0xB8,&Rvalue);		
	}while((Rvalue&BIT_31)==0);
	Read_EP_IOWord(pci_dev.io_addr,0xB8,&Rvalue);
	return Rvalue;
}

void Gphy_w(INT16U Addr,INT16U data){
	INT32U Ddata=0,tmp=0,Rvalue;
	Ddata=Addr|0x8000;
	Ddata=Ddata<<16;
	Ddata=Ddata+data;
	Write_EP_IODword(pci_dev.io_addr,0x60,Ddata);
	do{
		Read_EP_IODword(pci_dev.io_addr,0x60,&Rvalue);
	}while((Rvalue&BIT_31)==BIT_31);	
}
INT32U Gphy_r(INT16U Addr){
	INT32U Rvalue,Ddata=0;
	Ddata=Addr;
	Write_EP_IODword(pci_dev.io_addr,0x60,Ddata<<16);
	do{
		Read_EP_IODword(pci_dev.io_addr,0x60,&Rvalue);
	}while((Rvalue&BIT_31)==0);
	Rvalue=Rvalue&0xFFFF;
	return Rvalue;
}
MSIX_Setting_device(){

}

