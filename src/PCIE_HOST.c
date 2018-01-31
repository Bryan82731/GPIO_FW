#include "bsp.h"
#include "PCIE_HOST.h"
#include <stdlib.h>
#include <string.h>

#include "OOBMAC_COMMON.h"
#include "OOBMAC.h"


extern void VGA_enable(void);

extern OS_EVENT *PCIQ;
//extern OS_EVENT *PCIBusSem;
#define  printk(a, ...) {}
#define OS_S3_FLAG 0
#define D3Cold_Switch 0
extern unsigned  char g_scan_ok_flag ;//add by ylb 20160106
extern unsigned  char g_scan_ap_cnt ;//add by ylb 20160106
extern unsigned  char g_is_rx_hang;//add by ylb 20160106

/*#define PCI_MSG  0x185
#define PCI_CMD 0x186*/
//modified by melody for conflict with CMAC
#define PCI_MSG  0x190
#define PCI_CMD  0x191

#define BYPASS_MODE_RDY 0x01
#define TO_4281_MODE_RDY 0x02

#define TO_BYPASS_MODE 0x11
#define TO_4281_MODE 0x12
#define TO_BYPASS_MODE_SYS 0x13
#define PCI_PWR_STATE 0x63
#define S3 0x03
#define S4 0x04
#define S5 0x05 


/*INT32U	Read_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile INT32U *value);
INT32U	Write_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile ULONG value);
INT32U	Read_RC_PCIDword(volatile USHORT addr,volatile INT32U *value);
INT32U	Write_RC_PCIDword(volatile USHORT addr,volatile ULONG value);

ULONG 	Read_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value);
ULONG   Read_EP_PCIWord(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value);
UCHAR 	Read_EP_PCIByte( UCHAR bus, UCHAR dev, UCHAR fun, USHORT reg);

INT8U   Write_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value);
UCHAR	Write_EP_PCIWord(UCHAR bus, UCHAR dev, UCHAR fun, USHORT reg, USHORT value);
UCHAR 	Write_EP_PCIByte(UCHAR bus, UCHAR dev, UCHAR fun, USHORT reg, UCHAR value);

UCHAR  PCIE_MEM8_READ(unsigned int addr);
void  PCIE_MEM8_WRITE(unsigned int addr, unsigned char val);
UCHAR  PCIE_MEM16_READ(unsigned int addr);
void  PCIE_MEM16_WRITE(unsigned int addr, unsigned char val);
UINT  PCIE_MEM32_READ(unsigned int addr);
void  PCIE_MEM32_WRITE(unsigned int addr, unsigned int val);
UINT PCIE_MMIO32_READ(unsigned int addr);
UINT PCIE_MMIO16_READ(unsigned int addr);
UINT PCIE_MMIO8_READ(unsigned int addr);
void  PCIE_MMIO32_WRITE(unsigned int addr, unsigned int val);
void  PCIE_MMIO16_WRITE(unsigned int addr, unsigned int val);
void  PCIE_MMIO8_WRITE(unsigned int addr, unsigned int val);
void bsp_PHost_handler(void);
INT32U EP_INDI_Initial();

INT32U PH_allocate_mem(volatile INT32U size);


void WIFI_DASH_Setting();
void SetISR();
void GetISR();
void SetIMR();
void GetIMR();

*/
//INT32U Host_Send_TLP();
struct Pg_status *pg_status;
struct Pci_Dev pci_dev;


extern int r8168hRdy;
void PCIDevDrvTask()
{
    volatile INT32U ph_result,flag;
    REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)=0x00;
    flag=1;
    ph_result=_4281test();
    //ph_result= 1;
    if (ph_result==0)
    {
        REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)=0x04;
    }
    else REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)=0x0A;

    while(1)
    {
        /*
            if(REGX8(OOBMAC_BASE_ADDR+0x180)==0x0F&&ph_result==1&&flag==1)
            {
                Write_Vt_PCIDword(0x3c,5,0x100);//set for interrupt
                Cfg_Bar_VD2VT(5);
                _2_Bypass_WO_cfg();
                pci_dev.Bypass_Driver_Status=1; //Driver loading
                _Write_Vt_PCIDword(4,5,0x00100407,0xF,0);
                pci_dev.Vendor_cfg[0][1]=0x00100407;
                pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
                flag=0;
                REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)=0x05;
            }*/

        OSTimeDly(OS_TICKS_PER_SEC/100);
    }

}

INT32U _2_4281()
{
    volatile INT32U ph_result,Rvalue;
    REGX8(0xBAFA0A00)=0x0;		//rg_bypass for 4281 mode
    REGX8(0xBAFA0A08)=REGX8(0xBAFA0A08)&0xFC;	//by pass mode cfg reg
    REGX8(0xBAFA0A14)=0x0;
    REGX8(0xBAFA0610)=REGX8(0xBAFA0610)&0xFE;	//set  LEBI_RTANS working at by 4281mode(0)
    REGX8(0xBAFA0710)=REGX8(0xBAFA0710)&0xFE;	//set CDM_MBOX working at 4281 mode(0)

    REGX8(0xBAFA0900)= 0xFF;  	//device MSG interrupt to 4281 enable;
    REGX8(0xBAFA0904)=0x00;		//MSG interrupt bypass to chipset disable;
    REGX8(0xBAFA0908)=0x00;		//other msg to 4281 mode

    REGX32(0xBAFA0B04)=0x3FFFFFF;	//set interrupt
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
    ph_result =ph_result| Write_RC_PCIDword(0x4,Rvalue|0x7);
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
    pci_dev.Bypass_mode_wocfg_flag = 0;

    //REGX8(0xBAF10040)=REGX8(0xBAF10040)&0xF0;	//just for asic,not need in FPGA
    return 1;
}


INT32U _2_Bypass()
{
    volatile INT32U ph_result,Rvalue;
    //printk("_2_bypass mode\r\n");
    REGX8(0xBAFA0A00)=0x1;		//rg_bypass
    //REGX8(0xBAFA0A08)=REGX8(0xBAFA0A08)|0x3;	//by pass mode cfg reg
    REGX8(0xBAFA0A08)=REGX8(0xBAFA0A08)|0x2;	//wifidash_cdm_rd_sel: PC host read cfg cpl from remote wifi-device cfg sapce
    REGX8(0xBAFA0A14)=2;		//wifidash_cdm_byp_en: cfg tlp bypass wifidash

    REGX8(0xBAFA0610)=REGX8(0xBAFA0610)|0x1;	//set  LEBI_RTANS working at by bypass mode(0)
    REGX8(0xBAFA0710)=REGX8(0xBAFA0710)|0x1;	//set CDM_MBOX working at by bypass mode(0)

    REGX8(0xBAFA0900)= 0x00;  	//device MSG interrupt to 4281 enable;
    REGX8(0xBAFA0904)=0xFF;		//MSG interrupt bypass to chipset disable;
    REGX8(0xBAFA0908)=0xFF;		//other msg to 4281 mode

    REGX32(0xBAFA0B04)=0x3FFFFFF;	//set interrupt
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
    ph_result =ph_result| Write_RC_PCIDword(0x4,Rvalue|0x7);
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);

    REGX8(0xBAFA0A10) = 0x3;		// set mem R/W as direcion patch for dma

    pci_dev.Bypass_mode_wocfg_flag = 0;
    //REGX8(0xBAF10040)=REGX8(0xBAF10040)&0xF0; 	//just for asic,not need in FPGA
    return 1;
}
INT32U _2_4281_cfg()
{
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    REGX8(0xBAFA0A00)=0x0;		//rg_bypass for 4281 mode
    REGX8(0xBAFA0A08)=0x3;	//by pass mode cfg reg
    REGX8(0xBAFA0A14)=0x2;	//wifidash_cdm_byp_en: cfg tlp bypass wifidash and cancel the ASPM binding between fp device and fp host

    REGX8(0xBAFA0610)=REGX8(0xBAFA0610)&0xFE;	//set  LEBI_RTANS working at by 4281mode(0)
    REGX8(0xBAFA0710)=REGX8(0xBAFA0710)&0xFE;	//set CDM_MBOX working at 4281 mode(0)

    REGX8(0xBAFA0900)= 0xFF;  	//device MSG interrupt to 4281 enable;
    REGX8(0xBAFA0904)=0x00;		//MSG interrupt bypass to chipset disable;
    REGX8(0xBAFA0908)=0x00;		//other msg to 4281 mode

    REGX32(0xBAFA0B04)=0x3FFFFFF;	//set interrupt

    REGX8(0xBAFA0A10) = 0x0;		// clear mem R/W as direcion patch for dma

//	pci_dev.Vendor_cfg[0][1]=0x00100400;
//	_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);

    pci_dev.Bypass_mode_wocfg_flag = 0;

    //REGX8(0xBAF10040)=REGX8(0xBAF10040)&0xF0;	//just for asic,not need in FPGA
    OS_EXIT_CRITICAL();
    return 1;
}

INT32U _2_Bypass_WO_cfg()
{
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();
    ////printk("_2_bypass mode\r\n");
    VDtable_2_wificfg();
    REGX8(0xBAFA0A00)=0x1;		//rg_bypass

    //wifidash_cdm_rd_sel: PC host read cfg cpl from remote wifi-device cfg sapce.
    //wifidash_cdm_wr_sel:func/func6 r/W cfg_space by 4281.
    REGX8(0xBAFA0A08)=REGX8(0xBAFA0A08)|0x3;
    REGX8(0xBAFA0A14)=REGX8(0xBAFA0A14)|0x3;	//wifidash_cdm_byp_en: cfg tlp bypass wifidash and ASPM bind with fp device


    REGX8(0xBAFA0610)=REGX8(0xBAFA0610)|0x1;	//set  LEBI_RTANS working at by bypass mode(0)
    REGX8(0xBAFA0710)=REGX8(0xBAFA0710)&0xFE;	//set CDM_MBOX working at 4281 mode(0)

    REGX8(0xBAFA0900)= 0x00;  	//device MSG interrupt to 4281 disable;
    REGX8(0xBAFA0904)=0xFF;		//MSG interrupt bypass to chipset enable;
    REGX8(0xBAFA0908)=0xFF;		//other msg to 4281 mode

    REGX32(0xBAFA0B04)=0x3FFFFFF;	//set interrupt
//	ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
//	ph_result =ph_result| Write_RC_PCIDword(0x4,Rvalue|0x7);
//	ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);

    REGX8(0xBAFA0A10) = 0x3;		// set mem R/W as direcion patch for dma
    //pci_dev.Vendor_cfg[0][1]=0x00100407;
    //_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
    pci_dev.Bypass_mode_wocfg_flag = 1;
    //REGX8(0xBAF10040)=REGX8(0xBAF10040)&0xF0; 	//just for asic,not need in FPGA
    OS_EXIT_CRITICAL();
    return 1;
}

//Virtual CFG Read for mutifunction
INT32U	_Read_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile INT32U *value,volatile USHORT cs2)
{
    volatile INT32U	tmp, rv = 0,EP_dbi_addr=0;
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if(pci_dev.EP_prst_status==0) return 0xFFFFFFFF;
    if(REGX8(0xBAFA0A08)==3&&(fun_num<5||fun_num>6))return 0xFFFFFFFF;
    EP_dbi_addr=EP_dbi_addr|addr;
    EP_dbi_addr=EP_dbi_addr|(fun_num<<16);
    EP_dbi_addr=EP_dbi_addr|(cs2<<19);
    REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET)=EP_dbi_addr;	//EP_DBI addr,fun mum and cs2
    REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0xC)=0x3; 	//write and execute

    for(tmp=0; tmp<TIME_OUT; tmp++)
    {
        if ((REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0xC)&BIT_0)!=BIT_0)
        {
            *value = REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0x8);
            rv ++;
            break;
        }
    }
    OS_EXIT_CRITICAL();
    return rv;
}

//Virtual CFG Write for mutifunction
INT32U	_Write_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile ULONG value,volatile INT32U first_byte_en,volatile USHORT cs2)
{
    volatile INT32U	tmp=0, rv = 0,EP_dbi_addr=0,cmd=0;
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    if(pci_dev.EP_prst_status==0) return 0xFFFFFFFF;
    if(REGX8(0xBAFA0A08)==3&&(fun_num<5||fun_num>6))return 0xFFFFFFFF;
    EP_dbi_addr=EP_dbi_addr|addr;
    EP_dbi_addr=EP_dbi_addr|(fun_num<<16);
    EP_dbi_addr=EP_dbi_addr|(cs2<<20);
    REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET)=EP_dbi_addr;
    REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0x4)=value;	//EPDBI Write data
    cmd=cmd|(first_byte_en<<3);
    cmd=cmd|5;
    REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0xC)=cmd;	//EPDBI byte_enalbe+write_cmd+execute
    for(tmp=0; tmp<TIME_OUT; tmp++)
    {
        if ((REGX32(WIFI_DASH_BASE_ADDR + EP_DBI_CH_OFFSET+0xC)&BIT_0)!=BIT_0)
        {
            rv ++;
            break;
        }
    }
    OS_EXIT_CRITICAL();
    return 0;

}

INT32U	Read_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile INT32U *value)
{
    return _Read_Vt_PCIDword(addr,fun_num,value,0);
}

INT32U	Write_Vt_PCIDword(volatile USHORT addr,volatile USHORT fun_num,volatile ULONG value)
{
    return _Write_Vt_PCIDword(addr,fun_num,value,0xF,0);
}


//RC CFG RW
INT32U	Read_RC_PCIDword(volatile USHORT addr,volatile INT32U *value)
{
    volatile INT32U	tmp=0, rv = 0;
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET)=addr;	//RCDBI addr
    REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0xC)=0x03;	//RCDBI ctrl
    for(tmp=0; tmp<TIME_OUT; tmp++)
    {
        if ((REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0xC)&BIT_0)!=BIT_0)
        {
            *value = REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0x8);
            rv ++;
            break;
        }
    }
    OS_EXIT_CRITICAL();
    return rv;
}

INT32U	Write_RC_PCIDword(volatile USHORT addr,volatile ULONG value)
{
    volatile INT32U	tmp=0, rv = 0;
    OS_CPU_SR  cpu_sr = 0;
    OS_ENTER_CRITICAL();
    REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET)=addr;		//RCDBI addr
    REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0x4)=value;	//RCDBI Write data
    REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0xC)=0x7D;	//RCDBI byte_enalbe+write_cmd+execute
    for(tmp=0; tmp<TIME_OUT; tmp++)
    {
        if ((REGX32(WIFI_DASH_BASE_ADDR + RC_DBI_CH_OFFSET+0xC)&BIT_0)!=BIT_0)
        {
            rv ++;
            break;
        }
    }
    OS_EXIT_CRITICAL();
    return 0;
}

//calculate the byte enable bit by addr;
USHORT Addr2Byte_en(volatile USHORT addr,volatile USHORT byte_init)
{
    volatile USHORT byte_off,byte_en;
    byte_en=byte_init;
    byte_off=addr%4;
    byte_en=byte_en<<byte_off;
    return byte_en;
}
INT32U Byte_Enalbe_2_FF(volatile INT8U byte_enalbe)
{
    INT32U FFtype=0;
    if((byte_enalbe&BIT_0)==BIT_0)
    {
        FFtype=FFtype|0xFF;
    }
    if((byte_enalbe&BIT_1)==BIT_1)
    {
        FFtype=FFtype|0xFF00;
    }
    if((byte_enalbe&BIT_2)==BIT_2)
    {
        FFtype=FFtype|0xFF0000;
    }
    if((byte_enalbe&BIT_3)==BIT_3)
    {
        FFtype=FFtype|0xFF000000;
    }
    return FFtype;
}

//EP CFG RW
ULONG Send_TLP_Polling(
    volatile INT32U TLP_TPYE,		//TLP Type
    volatile INT32U RW,				//Read or Write
    volatile INT32U Base_addr_H,	//high Base addr
    volatile INT32U Base_addr_L,	//low base addr
    volatile INT32U offset,			//offset addr
    volatile INT32U first_byte_en,	//first byte enable
    volatile INT32U last_byte_en,	//last byte enalbe
    volatile INT32U LEN,			//length
    volatile INT32U Timeout,		//polling time out num
    volatile INT32U *value			//Read or write data ptr
)
{
    volatile struct RcPage *hostpage,hostpaget;
    volatile struct CFG_ADDR *cfg_addr;
    volatile INT32U tmp=0,*pg_value,addr , rv = PH_SUCCESS,pg_wr_ptr,Rhostpage,IMRvalue;
    OS_CPU_SR  cpu_sr = 0;

    OS_ENTER_CRITICAL();

//	IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
//	REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;

    hostpage = &hostpaget;

    /*check page full*/
    do
    {
        tmp++;
    }
    while(
        ((REGX32(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET)&BIT_7) == BIT_7)
        && tmp<10000
    );		//check page full
    if (tmp>=10000)
    {
        OOBMACWriteIBReg(0xE002,1,1,0);
        OOBMACWriteIBReg(0xE002,1,1,1);
        REGX32(WIFI_DASH_BASE_ADDR+0xA0C)=1;
        REGX32(WIFI_DASH_BASE_ADDR+0xA0C)=0;
        rv = PH_ERROR_PAGEFULL;
        goto Send_TLP_Polling_EXIT;
    }
    pg_wr_ptr = (REGX32(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET)&0x18)>>3;
    /*get the page should be using now*/
    Rhostpage =WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x10*(pg_wr_ptr+1);

    /*get the pg_value offset*/
    pg_value =(INT32U *)(WIFI_DASH_BASE_ADDR+0x150+0x80*pg_wr_ptr);

    /*rst the value of pg_value and page*/
    REGX32(Rhostpage)=0;
    REGX32(Rhostpage+0x4)=0;
    REGX32((INT32U)pg_value)=0;


    /*cfg  page*/
    hostpage->EP=0;
    hostpage->LEN=LEN;
    hostpage->LAST_BE=last_byte_en;
    hostpage->FIRST_BE=first_byte_en;
    hostpage->ADDRH=Base_addr_H;
    switch (TLP_TPYE)
    {
        case CFG_TYPE:
            cfg_addr =(struct CFG_ADDR *)((INT8U *)hostpage+8);
            addr=offset/4;
            hostpage->TLP_TYPE =CFG_TYPE;
            cfg_addr->bus=Base_addr_L>>8;
            cfg_addr->dev=(Base_addr_L<<8)>>11;
            cfg_addr->fun=(Base_addr_L<<13)>>13;
            cfg_addr->reg_num=addr;
            break;
        case IO_TYPE:
            hostpage->TLP_TYPE =IO_TYPE;
            addr=Base_addr_L+offset;
            hostpage->ADDRL = addr;
            break;
        case MEM_TYPE:
            hostpage->TLP_TYPE =MEM_TYPE;
            addr=Base_addr_L+offset;
            hostpage->ADDRL = addr;
            break;
        default:
            rv = 4;
            goto Send_TLP_Polling_EXIT;
            break;

    }

    REGX32(Rhostpage)=*((INT32U *)hostpage);			//PAGE CTRL REG0
    REGX32(Rhostpage+0x8)=hostpage->ADDRL;
    REGX32(Rhostpage+0xC)=hostpage->ADDRH;

    if(RW==R_CMD)
    {
        hostpage->CMD = R_CMD;
        hostpage->OWN = 1;
        REGX32(Rhostpage+4)=*((INT32U *)hostpage+1);		//PAGE CTRL REG1
        for(tmp=0; tmp<TIME_OUT; tmp++)
        {
            if ((REGX32(Rhostpage+4)& BIT_31)!=BIT_31)
            {
                *value = REGX32((INT32U)pg_value);
                REGX32(Rhostpage+4)=REGX32(Rhostpage+4)|BIT_28; //clear done bit
                rv = PH_SUCCESS;
                goto Send_TLP_Polling_EXIT;
            }
        }
    }
    else if(RW==W_CMD)
    {
        hostpage->CMD = W_CMD;
        hostpage->OWN = 1;
        REGX32((INT32U)pg_value) = *value;
        REGX32(Rhostpage+4)=*((INT32U *)hostpage+1);		//PAGE CTRL REG1
        for(tmp=0; tmp<TIME_OUT; tmp++)
        {
            WIFIDASH_DLY(10);
            if ((REGX32(Rhostpage+4)& BIT_31)!=BIT_31)
            {
                REGX32(Rhostpage+4)=REGX32(Rhostpage+4)|BIT_28; //clear done bit
                rv = PH_SUCCESS;
                goto Send_TLP_Polling_EXIT;
            }
        }
    }
    rv = PH_ERROR_TIMEOUT;

Send_TLP_Polling_EXIT:
//	REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    OS_EXIT_CRITICAL();
    return rv;
}
//ULONG Read_EP_IODword(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value)

ULONG Read_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value)
{
    volatile INT32U Base_addr_L=0,ph_result;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;
    ph_result=Send_TLP_Polling(CFG_TYPE,R_CMD,0,Base_addr_L,addr,0b1111,0,1,TIME_OUT,value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

ULONG Read_EP_PCIWord(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value)
{

    volatile INT32U Base_addr_L=0,byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(addr,0x03);
    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;
    ph_result = Send_TLP_Polling(CFG_TYPE,R_CMD,0,Base_addr_L,addr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x3:
            *value=(INT16U)value_temp;
            break;
        case 0x6:
            *value=(INT16U)(value_temp>>8);
            break;
        case 0xc:
            *value=(INT16U)(value_temp>>16);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
ULONG 	Read_EP_PCIByte(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U *value)
{
    volatile INT32U Base_addr_L=0,byte_en,value_temp,ph_result;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(addr,0x01);
    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;
    ph_result = Send_TLP_Polling(CFG_TYPE,R_CMD,0,Base_addr_L,addr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x1:
            *value=(INT8U)value_temp;
            break;
        case 0x2:
            *value=(INT8U)(value_temp>>8);
            break;
        case 0x4:
            *value=(INT8U)(value_temp>>16);
            break;
        case 0x8:
            *value=(INT8U)(value_temp>>24);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

ULONG Write_EP_PCIDword(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value)
{

    volatile INT32U Base_addr_L=0,ph_result;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;


    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;


    ph_result=Send_TLP_Polling(CFG_TYPE,W_CMD,0,Base_addr_L,addr,0b1111,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return 	ph_result;
}

ULONG	Write_EP_PCIWord(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value)
{
    volatile INT32U Base_addr_L=0,byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(addr,0x03);
    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;
    value_temp = value;
    switch(byte_en)
    {
        case 0x3:
            value=value_temp;
            break;
        case 0x6:
            value=value_temp<<8;
            break;
        case 0xc:
            value=value_temp<<16;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result = Send_TLP_Polling(CFG_TYPE,W_CMD,0,Base_addr_L,addr,byte_en,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
ULONG Write_EP_PCIByte(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun,volatile USHORT addr,volatile INT32U value)
{
    volatile INT32U Base_addr_L=0,byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;

    byte_en=Addr2Byte_en(addr,0x01);
    Base_addr_L = bus;
    Base_addr_L=(Base_addr_L<<5)+dev;
    Base_addr_L =(Base_addr_L<<3)+fun;
    value_temp = value;
    switch(byte_en)
    {
        case 0x1:
            value=value_temp;
            break;
        case 0x2:
            value=value_temp<<8;
            break;
        case 0x4:
            value=value_temp<<16;
            break;
        case 0x8:
            value=value_temp<<24;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result = Send_TLP_Polling(CFG_TYPE,W_CMD,0,Base_addr_L,addr,byte_en,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

//IO Read Dword
ULONG Read_EP_IODword(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value)
{
    volatile INT32U IMRvalue,ph_result;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    ph_result=Send_TLP_Polling(IO_TYPE,R_CMD,0,io_base,io_addr,0b1111,0,1,TIME_OUT,value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;

}
//
//IO Read word
ULONG Read_EP_IOWord(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value)
{
    volatile INT32U byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(io_addr,0x03);
    ph_result=Send_TLP_Polling(IO_TYPE,R_CMD,0,io_base,io_addr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x3:
            *value=(INT16U)value_temp;
            break;
        case 0x6:
            *value=(INT16U)(value_temp>>8);
            break;
        case 0xc:
            *value=(INT16U)(value_temp>>16);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

//IO Read byte
ULONG Read_EP_IOByte(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U *value)
{
    volatile INT32U byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(io_addr,0x01);
    ph_result=Send_TLP_Polling(IO_TYPE,R_CMD,0,io_base,io_addr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x1:
            *value=(INT8U)value_temp;
            break;
        case 0x2:
            *value=(INT8U)(value_temp>>8);
            break;
        case 0x4:
            *value=(INT8U)(value_temp>>16);
            break;
        case 0x8:
            *value=(INT8U)(value_temp>>24);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

//IO write Dword
ULONG Write_EP_IODword(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value)
{
    volatile INT32U IMRvalue,ph_result;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    ph_result=Send_TLP_Polling(IO_TYPE,W_CMD,0,io_base,io_addr,0b1111,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

//IO write word
ULONG Write_EP_IOWord(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value)
{
    volatile INT32U byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;

    byte_en=Addr2Byte_en(io_addr,0x03);
    value_temp = value;
    switch(byte_en)
    {
        case 0x3:
            value=value_temp;
            break;
        case 0x6:
            value=value_temp<<8;
            break;
        case 0xc:
            value=value_temp<<16;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result=ph_result|Send_TLP_Polling(IO_TYPE,W_CMD,0,io_base,io_addr,byte_en,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;

}

ULONG Write_EP_IOByte(volatile INT32U io_base,volatile USHORT io_addr,volatile INT32U value)
{
    volatile INT32U byte_en,value_temp,ph_result=0;
    volatile INT32U IMRvalue;
    /*close imr of msg interrupt from device*/
    IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(io_addr,0x01);
    value_temp = value;
    switch(byte_en)
    {
        case 0x1:
            value=value_temp;
            break;
        case 0x2:
            value=value_temp<<8;
            break;
        case 0x4:
            value=value_temp<<16;
            break;
        case 0x8:
            value=value_temp<<24;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result=ph_result|Send_TLP_Polling(IO_TYPE,W_CMD,0,io_base,io_addr,byte_en,0,1,TIME_OUT,&value);
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
void bsp_PHost_handler(void)
{
    PH_INT PH_ISR;
    INT32U ph_result=1,Rvalue,FFType,PH_ISRT;
    INT32U RCdm_cfg_info,rg_cdm_data,rg_cdm_addr,rg_func_num,rg_cdm_rd,rg_cdm_wr;
    WIFIDASH_DLY(10);
    pci_dev.IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);	//back up wifi dash imr
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=0x00000000;		//clear wifi dash imr
    //PH_ISR=(struct PH_INT *)REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET);
    PH_ISRT=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET);
    memcpy(&PH_ISR,(struct PH_INT *)(&PH_ISRT),sizeof(struct PH_INT));
    //memcpy(&PH_ISR,(struct PH_INT *)(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET),sizeof(struct PH_INT));//get wifi dash isr
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET)=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET)&pci_dev.IMRvalue;	 //clear wifi dash isr
    //printk("bsp_PHost_handler\r\n");
    //device interrupt
    if(PH_ISR.sii_rc_inta || PH_ISR.sii_rc_intb || PH_ISR.sii_rc_intc || PH_ISR.sii_rc_intd)
    {
        pci_interrupt_handler_callback();
    }
    if(PH_ISR.sii_ep_obff_msg||
            PH_ISR.sii_ep_unlock_msg||
            PH_ISR.sii_ep_ven_msg||
            PH_ISR.sii_rc_err_msg||
            PH_ISR.sii_rc_ltr_msg||
            PH_ISR.sii_rc_pme_msg||
            PH_ISR.sii_rc_ven_msg)
    {
        ph_result=1;

    }
    if(PH_ISR.cdm_rw) 				//cfg Read/write from OS interrupt
    {
        RCdm_cfg_info=WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET;
        rg_cdm_data=REGX32(RCdm_cfg_info);
        rg_cdm_addr=REGX32(RCdm_cfg_info+4);
        rg_func_num=(REGX32(RCdm_cfg_info+8)&0x60000000)>>29;
        rg_cdm_rd=(REGX32(RCdm_cfg_info+8)&0xF0000)>>16;
        rg_cdm_wr=(REGX32(RCdm_cfg_info+8)&0xF00000)>>20;

        if((REGX32(RCdm_cfg_info+8)&BIT_31)==BIT_31)
        {

            /*Deal the EP fun 5/fun0 cfg r/w */
            if(rg_func_num==0)
            {
                /*Deal the cfg R/W in bypass mode*/
                if(pci_dev.Bypass_mode_wocfg_flag==1)
                {
                    /*Deal the cfg read*/
                    if(rg_cdm_rd!=0)
                    {
                        if(rg_cdm_addr>=0x700 && rg_cdm_addr<0x800)
                        {
                            rg_cdm_data=pci_dev.Vendor_cfg[rg_func_num][(rg_cdm_addr-0x500)/4];
                        }
                        else if(rg_cdm_addr ==0x30 ||rg_cdm_addr>=0x200)
                        {
                            rg_cdm_data=0x0;
                        }
                        else
                        {
                            rg_cdm_data=pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4];
                        }
                    }

                    /*Deal the cfg write in bypass mode*/
                    if(rg_cdm_wr!=0 )
                    {

                        /*Deal cfg write 0ffset 0x00 and 0x30*/
                        if(rg_cdm_addr ==0x00 ||rg_cdm_addr ==0x30)
                        {

                        }
                        else if(
                            //rg_cdm_addr==pci_dev.PM_status_offset ||
                            rg_cdm_addr==pci_dev.ASPM_status_offset)
                        {

                        }

                        /*Deal cfg write 0x4~0x200*/
                        else if(rg_cdm_addr >=0x4 && rg_cdm_addr <0x200)
                        {
                            /*write the reg to the offset of wifi device*/
                            Send_TLP_Polling(
                                CFG_TYPE,
                                W_CMD,
                                0,
                                rg_func_num,
                                rg_cdm_addr,
                                rg_cdm_wr,
                                0,
                                1,
                                TIME_OUT,
                                &rg_cdm_data);
                            /*Read back the reg from the same offset of wifi device*/
                            Read_EP_PCIDword(0,0,rg_func_num,rg_cdm_addr,&Rvalue);
                            /*update the table*/
                            pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4]=Rvalue;

                            /*write to the EP fun 5*/
                            /*cfg base addr reg*/
                            if(rg_cdm_addr >=0x10 && rg_cdm_addr <0x28)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              rg_cdm_addr,
                                              rg_func_num+5,
                                              rg_cdm_data|pci_dev.Vendor_cfg_bar[(rg_cdm_addr-0x10)/4][0],
                                              rg_cdm_wr,
                                              0);
                            }
                            /*cfg reg <0x40*/
                            else if (rg_cdm_addr<0x40)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              rg_cdm_addr,
                                              rg_func_num+5,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
                            }
                            /*PM status cfg write*/
                            else if(rg_cdm_addr==pci_dev.PM_status_offset)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              0x44,								//PM status at 0x44
                                              rg_func_num+5,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
                            }
                            /*aspm cfg write*/
                            /*else if(rg_cdm_addr==pci_dev.ASPM_status_offset){
                            	ph_result=ph_result|_Write_Vt_PCIDword(
                            		0x80,								//PM status at 0x44
                            		rg_func_num+5,
                            		rg_cdm_data,
                            		rg_cdm_wr,
                            		0);
                            }*/
                        }
                        else if(rg_cdm_addr>=0x700 && rg_cdm_addr<0x800)
                        {
                            /*write the reg to the offset of wifi device*/
                            Send_TLP_Polling(
                                CFG_TYPE,
                                W_CMD,
                                0,
                                rg_func_num,
                                rg_cdm_addr,
                                rg_cdm_wr,
                                0,
                                1,
                                TIME_OUT,
                                &rg_cdm_data);
                            /*Read back the reg from the same offset of wifi device*/
                            Read_EP_PCIDword(0,0,rg_func_num,rg_cdm_addr,&Rvalue);
                            /*update the table*/
                            pci_dev.Vendor_cfg[rg_func_num][(rg_cdm_addr-0x500)/4]=Rvalue;
                        }
                        else {}
                        //printk("Attention!\n cfg write over 0x200:0x%8X\n",rg_cdm_addr);
                    }// end off cfg write flow
                }

                /*Deal the cfg R/W in 4281 mode*/
                else if(pci_dev.Bypass_mode_wocfg_flag==0)
                {

                    /*Deal the cfg read*/
                    /*Not support expansion rom(0x30) ,offset 0x200~0x6FC, larger than 0x800	*/
                    if(rg_cdm_rd!=0)
                    {
                        if(rg_cdm_addr>=0x700 && rg_cdm_addr<0x800)
                        {
                            rg_cdm_data=pci_dev.Vendor_cfg[rg_func_num][(rg_cdm_addr-0x500)/4];
                        }
                        else if(rg_cdm_addr ==0x30 ||rg_cdm_addr>=0x200)
                        {
                            rg_cdm_data=0x0;
                        }
                        else
                        {
                            rg_cdm_data=pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4];
                        }
                    }

                    /*Deal the cfg write in 4281 mode*/
                    if(rg_cdm_wr!=0)
                    {
                        /*Deal cfg write 0ffset 0x00 and 0x30*/
                        if(rg_cdm_addr ==0x00 ||rg_cdm_addr ==0x30)
                        {
                        }
                        /*cfg base addr reg*/
                        else if(rg_cdm_addr >=0x10 && rg_cdm_addr <0x28)
                        {
                            /*update the table*/
                            pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4]
                                =(rg_cdm_data
                                  &(~(pci_dev.Vendor_cfg_bar[(rg_cdm_addr-0x10)/4][1]-1)))
                                 |pci_dev.Vendor_cfg_bar[(rg_cdm_addr-0x10)/4][0];
                            ph_result=ph_result|_Write_Vt_PCIDword(
                                          rg_cdm_addr,
                                          rg_func_num+5,
                                          rg_cdm_data|pci_dev.Vendor_cfg_bar[(rg_cdm_addr-0x10)/4][0],
                                          rg_cdm_wr,
                                          0);
                        }
                        /*mask the BME enable when in 4281 mode */
                        /*else if(rg_cdm_addr==0x4){
                        	pci_dev.Vendor_cfg[0][1]=pci_dev.Vendor_cfg[0][1] & 0xFFFFFFF8;
                        	_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
                        }*/
                        //else if(
                            //rg_cdm_addr==pci_dev.PM_status_offset ||
                            //rg_cdm_addr==pci_dev.ASPM_status_offset)
                        //{

                        //}
                        /*Deal cfg write  0x8~0x200 except bar*/
                        else if(rg_cdm_addr >=0x8 && rg_cdm_addr <0x200)
                        {
                            FFType=Byte_Enalbe_2_FF(rg_cdm_wr);
                            /*update the table*/
                            pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4]
                                =(pci_dev.Vendor_cfg[rg_func_num][rg_cdm_addr/4]
                                  |FFType
                                 )
                                 &
                                 (rg_cdm_data |(~FFType));
                            /*cfg reg <0x40*/
                            if (rg_cdm_addr<0x40)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              rg_cdm_addr,
                                              rg_func_num+5,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
                            }
                            /*PM status cfg write*/
                            else if(rg_cdm_addr==pci_dev.PM_status_offset)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              0x44,								//fun 5's PM status at 0x44
                                              rg_func_num+5,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
								
								 ph_result=ph_result|_Write_Vt_PCIDword(
                                              0x44,								//fun 5's PM status at 0x44
                                              rg_func_num+6,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
                            }
                            /*aspm cfg write*/
                            else if(rg_cdm_addr==pci_dev.ASPM_status_offset)
                            {
                                ph_result=ph_result|_Write_Vt_PCIDword(
                                              0x80,								//fun 5's aspm status at 0x44
                                              rg_func_num+5,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
								ph_result=ph_result|_Write_Vt_PCIDword(
                                              0x80,								//fun 5's aspm status at 0x44
                                              rg_func_num+6,
                                              rg_cdm_data,
                                              rg_cdm_wr,
                                              0);
                            }
                        }
                        else if(rg_cdm_addr>=0x700 && rg_cdm_addr<0x800)
                        {
                            FFType=Byte_Enalbe_2_FF(rg_cdm_wr);
                            /*update the table*/
                            pci_dev.Vendor_cfg[rg_func_num][(rg_cdm_addr-0x500)/4]
                                =(pci_dev.Vendor_cfg[rg_func_num][(rg_cdm_addr-0x500)/4]
                                  |FFType
                                 )
                                 &
                                 (rg_cdm_data |(~FFType));

                        }
                        else
                        {
                            //printk("Attention!\n cfg write over 0x200:0x%8X\n",rg_cdm_addr);
                        }
                    }// end off cfg write flow

                }

            }	//end the flow of deal the fun num 0
            /*Deal the EP fun 6/fun1 cfg r/w. Just return all 0xFF for temp*/
            else if(rg_func_num==1)
            {
                rg_cdm_data=0xFFFFFFFF;
            }
            REGX32(RCdm_cfg_info)=rg_cdm_data;
            REGX32(RCdm_cfg_info+8)=REGX32(RCdm_cfg_info+8)|BIT_31;
        }// end off cfg deal while
    }
    //REGX16(WIFI_DASH_BASE_ADDR+0x70E)=CFG_imr;
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=pci_dev.IMRvalue & 0x3FFFFFF; //reset imr;
//to be disabe
}

//FUN0 interrupt handler
void bsp_Fun0_handler(void)
{
    volatile FUN0_INT *fun0_ISR;
    volatile INT32U IMRvalue,ISRvalue,ph_result,Rvalue,i;

    IMRvalue=REGX16(0xBAF1003A);
    REGX16(0xBAF1003a)=0x0000;
    ISRvalue=REGX16(0xBAF10038);
    REGX16(0xBAF10038)=REGX16(0xBAF10038);	//clean ISR
    fun0_ISR = (struct FUN0_INT*)&ISRvalue;
	#ifdef CONFIG_VGA_TEST_ENABLED
    if(fun0_ISR->perstb_r_sts)
    {
        //printk("prst rising interrupt\r\n");
        //Write_Vt_PCIDword(0,5,0x816510ec);
        pci_dev.EP_prst_status=1;
        REGX8(0xBAFA0A08)=0;
        //Write_Vt_PCIDword(0,7,0xFFFFFFFF);
        VGA_enable();
        Write_Vt_PCIDword(0,4,0xFFFFFFFF);
        //Write_Vt_PCIDword(0,2,0xFFFFFFFF);
        //Write_Vt_PCIDword(0,1,0xFFFFFFFF);
        REGX8(0xBAFA0A08)=0x3;
        //ph_result=ph_result|Write_Vt_PCIDword(0x3c,5,0x100);//set for interrupt
        //Cfg_Bar_VD2VT(5);
        //OOB2IB_W(0xD3EC,0xF,0x1010001);
    }
	#endif
    if(fun0_ISR->perstb_f_sts)
    {
        pci_dev.EP_prst_status=0;
        pci_dev.Bypass_Driver_Status=0; 	//shut down flag no driver
        //printk("prst falling interrupt\r\n");
//		pci_dev.Bypass_Driver_Status=0;		//shut down flag no driver

        /*back up wifi base addr for s3 s4*/
        if(pci_dev.S3_S4_Flag==1)
        {
            for(i=0; i<0x10; i++)
            {
                //pci_dev.S3_Vendor_cfg_cmd=pci_dev.Vendor_cfg[0][1];
                pci_dev.S3_Vendor_cfg_bar[i]=pci_dev.Vendor_cfg[0][i];
                pci_dev.S3_S4_Resume=1;
            }
        }

        /*rset the vendor cfg talbe to default*/
//		memcpy(pci_dev.Vendor_cfg,pci_dev.Default_cfg,sizeof(pci_dev.Default_cfg));

        //r8168h_init();
        //DMA_Init();
        /*when prst to low,if is in bypass mode ,change to 4281 mode ,else just reset vendor cfg*/
        //PH_HW_Reset(0x7);
        if(pci_dev.Bypass_mode_wocfg_flag == 1)
        {
            Inital_Adapter_cfg();
            //ucosii_wifi_hw_open();
        }
        else memcpy(pci_dev.Vendor_cfg,pci_dev.Default_cfg,sizeof(pci_dev.Default_cfg));
        pci_dev.Vendor_cfg[0][0]=0x816510EC;
        REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=0;
        REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;

    }
    //REGX16(0xBAF10038)=REGX16(0xBAF10038);

    REGX16(0xBAF1003a)=IMRvalue;
}


//FUN0 interrupt handler
void bsp_Fun0_subHandler(INT16U isr_tmp)
{
    volatile FUN0_INT *fun0_ISR;
    volatile INT32U ph_result,Rvalue,i;

    fun0_ISR = (struct FUN0_INT*)&isr_tmp;
    if(fun0_ISR->perstb_r_sts)
    {

#ifdef CONFIG_PCIE_ENABLED
        //printk("prst rising interrupt\r\n");
        //Write_Vt_PCIDword(0,5,0x816510ec);
        pci_dev.EP_prst_status=1;
        REGX8(0xBAFA0A08)=0;
        //Write_Vt_PCIDword(0,7,0xFFFFFFFF);
#endif

#ifdef CONFIG_VGA_TEST_ENABLED

        VGA_enable();
#endif

#ifdef CONFIG_PCIE_ENABLED
        //Write_Vt_PCIDword(0,4,0xFFFFFFFF);  //it affects EHCI not to be found
        //Write_Vt_PCIDword(0,2,0xFFFFFFFF);
        //Write_Vt_PCIDword(0,1,0xFFFFFFFF);
        REGX8(0xBAFA0A08)=0x3;
        ph_result=ph_result|Write_Vt_PCIDword(0x3c,5,0x100);//set for interrupt
        //Cfg_Bar_VD2VT(5);
        //OOB2IB_W(0xD3EC,0xF,0x1010001);
#endif
    }


    if(fun0_ISR->perstb_f_sts)
    {
#ifdef CONFIG_PCIE_ENABLED
        pci_dev.EP_prst_status=0;
        pci_dev.Bypass_Driver_Status=0; 	//shut down flag no driver
        //printk("prst falling interrupt\r\n");
//		pci_dev.Bypass_Driver_Status=0;		//shut down flag no driver

        /*back up wifi base addr for s3 s4*/
        if(pci_dev.S3_S4_Flag==1)
        {
            for(i=0; i<0x10; i++)
            {
                //pci_dev.S3_Vendor_cfg_cmd=pci_dev.Vendor_cfg[0][1];
                pci_dev.S3_Vendor_cfg_bar[i]=pci_dev.Vendor_cfg[0][i];
                pci_dev.S3_S4_Resume=1;
            }
        }

        /*rset the vendor cfg talbe to default*/
//		memcpy(pci_dev.Vendor_cfg,pci_dev.Default_cfg,sizeof(pci_dev.Default_cfg));

        //r8168h_init();
        //DMA_Init();
        /*when prst to low,if is in bypass mode ,change to 4281 mode ,else just reset vendor cfg*/
        //PH_HW_Reset(0x7);
        if(pci_dev.Bypass_mode_wocfg_flag == 1)
        {
            Inital_Adapter_cfg();
            //ucosii_wifi_hw_open();
        }
        else memcpy(pci_dev.Vendor_cfg,pci_dev.Default_cfg,sizeof(pci_dev.Default_cfg));
        pci_dev.Vendor_cfg[0][0]=0x816510EC;

#endif

    }

}

void bsp_pcie_sw_handler()
{
    volatile OOBMAC_INT *OOBMAC_ISR;
    volatile INT32U IMRvalue,ISRvalue,i,Rvalue;


    //printk("OOB mac interrupt\r\n");
    //if(OOBMAC_ISR->sw_int){
    //old method of switch to bypass mode
//			if(REG8(OOBMAC_BASE_ADDR+0x0180) == 0x60){
    /*resume wifi bar reg for s3 s4*/
    /*			if(pci_dev.S3_Flag==1){
    				for(i=1;i<0x10;i++){
    					//pci_dev.Vendor_cfg[0][1]=0x00100400;
    					//_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
    					//pci_dev.Vendor_cfg[0][0x10/4+i]=pci_dev.S3_Vendor_cfg_bar[i];
    					pci_dev.Vendor_cfg[0][i]=pci_dev.S3_Vendor_cfg_bar[i];
    					_Write_Vt_PCIDword(i*4,5,pci_dev.Vendor_cfg[0][i],0xF,0);
    					//pci_dev.Vendor_cfg[0][0x3C/4]=0x00000100;
    					//_Write_Vt_PCIDword(0x3C,5,pci_dev.Vendor_cfg[0][0x3C/4],0xF,0);
    				}
    				pci_dev.Vendor_cfg[0][1]=0x00100407;
    				_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);

    			}
    */
#if WIFI_INIT
    ucosii_wifi_hw_close();
#endif
    //VTcfg_2_VDcfg(5,0,0,0);
    /*			_2_Bypass_WO_cfg();
    			pci_dev.S3_Flag=OS_S3_FLAG;
    			pci_dev.Bypass_Driver_Status=1; //Driver loading
    			pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
    			//Write_Vt_PCIDword(0,5,0x816810ec);
    			REG8(OOBMAC_BASE_ADDR+0x0180)= 0x61;
    			REG8(OOBMAC_BASE_ADDR +0x108)=0x1;
    		}*/

    /*for switch bypass/4281 mode by system resume/boot*/
    if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)== TO_BYPASS_MODE_SYS)
    {
        /*resume wifi bar reg for s3 s4*/
        if(pci_dev.S3_S4_Resume==1)
        {
            for(i=1; i<0x10; i++)
            {
                //pci_dev.Vendor_cfg[0][1]=0x00100400;
                //_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
                //pci_dev.Vendor_cfg[0][0x10/4+i]=pci_dev.S3_Vendor_cfg_bar[i];
                pci_dev.Vendor_cfg[0][i]=pci_dev.S3_Vendor_cfg_bar[i];
                _Write_Vt_PCIDword(i*4,5,pci_dev.Vendor_cfg[0][i],0xF,0);
                pci_dev.S3_S4_Resume=0;
                //pci_dev.Vendor_cfg[0][0x3C/4]=0x00000100;
                //_Write_Vt_PCIDword(0x3C,5,pci_dev.Vendor_cfg[0][0x3C/4],0xF,0);
            }
        }

#if WIFI_INIT
        ucosii_wifi_hw_close();
#endif
        //VTcfg_2_VDcfg(5,0,0,0);
        //if(pci_dev.Bypass_mode_wocfg_flag==0){			//check if it is not in bypass mode, switch to bypass mode
        _2_Bypass_WO_cfg();
        //pci_dev.S3_S4_Flag=OS_S3_FLAG;
        pci_dev.Bypass_Driver_Status=1; //Driver loading
        //}
        _Write_Vt_PCIDword(4,5,0x00100407,0xF,0);
        pci_dev.Vendor_cfg[0][1]=0x00100407;
        pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
        //Write_Vt_PCIDword(0,5,0x816810ec);
        REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=BYPASS_MODE_RDY;
        REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
    }

    /*switch to 4281 mode by client*/
    if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)==TO_4281_MODE)
    {
        /*Disalbe BME*/
        _Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1]&0xFFFFFFF8,0xF,0);
        pci_dev.Vendor_cfg[0][1]=0x00100400;
        pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
        pci_dev.Bypass_Driver_Status=1; 	//shut down flag no driver
        Inital_Adapter_indriver();
        REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=TO_4281_MODE_RDY;
        pci_dev.goto4281mode=0;

#if NIC_INIT
        r8168h_init();
        Write_EP_IODword(pci_dev.io_addr,0xE4,VA2PA((INT32U)pci_dev.rxdesctmp));
        Write_EP_IODword(pci_dev.io_addr,0xE8,0x0);
        Write_EP_IODword(pci_dev.io_addr,0x20,VA2PA((INT32U)pci_dev.txdesctmp));
        Write_EP_IODword(pci_dev.io_addr,0x24,0x0);
        Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
        Rvalue =Rvalue|0x0C;
        Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx enalbe
        pci_dev.Rxcount=0;
        pci_dev.Txcount=0;
        Write_EP_IODword(pci_dev.io_addr,0x60,0x80009200);
#endif

        REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
    }
    /* switch to bypass mode by client tool*/
    if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)==TO_BYPASS_MODE)
    {
        //pci_dev.Vendor_cfg[0][pci_dev.PM_status_offset/4]=pci_dev.Vendor_cfg[0][pci_dev.PM_status_offset/4]&0xFFFFFFFC;
        _2_Bypass_WO_cfg();
        pci_dev.Bypass_Driver_Status=1; //Driver loading
        pci_dev.Vendor_cfg[0][1]=0x00100407;
        _Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
        pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
        REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=BYPASS_MODE_RDY;
        REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
    }
    /*get system power state(S3/S4/S5)*/
    if(REGX8(OOBMAC_BASE_ADDR+MAC_DMEMSTA)==PCI_PWR_STATE)
    {
        if(REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)==S3||REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)==S4)
        {
            pci_dev.S3_S4_Flag=1;
        }
        else pci_dev.S3_S4_Flag=0;
        REGX8(OOBMAC_BASE_ADDR+MAC_DMEMSTA)=0;
    }
    //}
}

//OOB mac interrupt handler
void bsp_OOBMAC_handler(void)
{
    volatile OOBMAC_INT *OOBMAC_ISR;
    volatile INT32U IMRvalue,ISRvalue,i,Rvalue;
    WIFIDASH_DLY(10);
    IMRvalue=REGX16(0xBAF7002C);
    REGX16(0xBAF7002C)=0x0000;
    ISRvalue=REGX16(0xBAF7002E);
    REGX16(0xBAF7002E)=REGX16(0xBAF7002E);
    OOBMAC_ISR=(struct OOBMAC_INT *)&ISRvalue;

    //printk("OOB mac interrupt\r\n");
    if(OOBMAC_ISR->sw_int)
    {
        //old method of switch to bypass mode
//			if(REG8(OOBMAC_BASE_ADDR+0x0180) == 0x60){
        /*resume wifi bar reg for s3 s4*/
        /*			if(pci_dev.S3_Flag==1){
        				for(i=1;i<0x10;i++){
        					//pci_dev.Vendor_cfg[0][1]=0x00100400;
        					//_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
        					//pci_dev.Vendor_cfg[0][0x10/4+i]=pci_dev.S3_Vendor_cfg_bar[i];
        					pci_dev.Vendor_cfg[0][i]=pci_dev.S3_Vendor_cfg_bar[i];
        					_Write_Vt_PCIDword(i*4,5,pci_dev.Vendor_cfg[0][i],0xF,0);
        					//pci_dev.Vendor_cfg[0][0x3C/4]=0x00000100;
        					//_Write_Vt_PCIDword(0x3C,5,pci_dev.Vendor_cfg[0][0x3C/4],0xF,0);
        				}
        				pci_dev.Vendor_cfg[0][1]=0x00100407;
        				_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);

        			}
        */
#if WIFI_INIT
        ucosii_wifi_hw_close();
#endif
        //VTcfg_2_VDcfg(5,0,0,0);
        /*			_2_Bypass_WO_cfg();
        			pci_dev.S3_Flag=OS_S3_FLAG;
        			pci_dev.Bypass_Driver_Status=1; //Driver loading
        			pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
        			//Write_Vt_PCIDword(0,5,0x816810ec);
        			REG8(OOBMAC_BASE_ADDR+0x0180)= 0x61;
        			REG8(OOBMAC_BASE_ADDR +0x108)=0x1;
        		}*/

        /*for switch bypass/4281 mode by system resume/boot*/
        if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)== TO_BYPASS_MODE_SYS)
        {
            /*resume wifi bar reg for s3 s4*/
            if(pci_dev.S3_S4_Resume==1)
            {
                for(i=1; i<0x10; i++)
                {
                    //pci_dev.Vendor_cfg[0][1]=0x00100400;
                    //_Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
                    //pci_dev.Vendor_cfg[0][0x10/4+i]=pci_dev.S3_Vendor_cfg_bar[i];
                    pci_dev.Vendor_cfg[0][i]=pci_dev.S3_Vendor_cfg_bar[i];
                    _Write_Vt_PCIDword(i*4,5,pci_dev.Vendor_cfg[0][i],0xF,0);
                    pci_dev.S3_S4_Resume=0;
                    //pci_dev.Vendor_cfg[0][0x3C/4]=0x00000100;
                    //_Write_Vt_PCIDword(0x3C,5,pci_dev.Vendor_cfg[0][0x3C/4],0xF,0);
                }
            }

#if WIFI_INIT
            ucosii_wifi_hw_close();
#endif
            //VTcfg_2_VDcfg(5,0,0,0);
            //if(pci_dev.Bypass_mode_wocfg_flag==0){			//check if it is not in bypass mode, switch to bypass mode
            _2_Bypass_WO_cfg();
            //pci_dev.S3_S4_Flag=OS_S3_FLAG;
            pci_dev.Bypass_Driver_Status=1; //Driver loading
            //}
            _Write_Vt_PCIDword(4,5,0x00100407,0xF,0);
            pci_dev.Vendor_cfg[0][1]=0x00100407;
            pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
            //Write_Vt_PCIDword(0,5,0x816810ec);
            REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=BYPASS_MODE_RDY;
            REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
        }

        /*switch to 4281 mode by client*/
        if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)==TO_4281_MODE)
        {
            /*Disalbe BME*/
            _Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1]&0xFFFFFFF8,0xF,0);
            pci_dev.Vendor_cfg[0][1]=0x00100400;
            pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
            pci_dev.Bypass_Driver_Status=1; 	//shut down flag no driver
            Inital_Adapter_indriver();
            REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=TO_4281_MODE_RDY;
            pci_dev.goto4281mode=0;

#if NIC_INIT
            r8168h_init();
            Write_EP_IODword(pci_dev.io_addr,0xE4,VA2PA((INT32U)pci_dev.rxdesctmp));
            Write_EP_IODword(pci_dev.io_addr,0xE8,0x0);
            Write_EP_IODword(pci_dev.io_addr,0x20,VA2PA((INT32U)pci_dev.txdesctmp));
            Write_EP_IODword(pci_dev.io_addr,0x24,0x0);
            Read_EP_IOByte(pci_dev.io_addr,0x37,&Rvalue);
            Rvalue =Rvalue|0x0C;
            Write_EP_IOByte(pci_dev.io_addr,0x37,Rvalue);	//tx rx enalbe
            pci_dev.Rxcount=0;
            pci_dev.Txcount=0;
            Write_EP_IODword(pci_dev.io_addr,0x60,0x80009200);
#endif

            REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
        }
        /* switch to bypass mode by client tool*/
        if(REGX8(OOBMAC_BASE_ADDR+PCI_CMD)==TO_BYPASS_MODE)
        {
            //pci_dev.Vendor_cfg[0][pci_dev.PM_status_offset/4]=pci_dev.Vendor_cfg[0][pci_dev.PM_status_offset/4]&0xFFFFFFFC;
            _2_Bypass_WO_cfg();
            pci_dev.Bypass_Driver_Status=1; //Driver loading
            pci_dev.Vendor_cfg[0][1]=0x00100407;
            _Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
            pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
            REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=BYPASS_MODE_RDY;
            REGX8(OOBMAC_BASE_ADDR+PCI_CMD)=0;
        }
        /*get system power state(S3/S4/S5)*/
        if(REGX8(OOBMAC_BASE_ADDR+MAC_DMEMSTA)==PCI_PWR_STATE)
        {
            if(REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)==S3||REGX8(OOBMAC_BASE_ADDR+MAC_DMEMENDA)==S4)
            {
                pci_dev.S3_S4_Flag=1;
            }
            else pci_dev.S3_S4_Flag=0;
            REGX8(OOBMAC_BASE_ADDR+MAC_DMEMSTA)=0;
        }
    }
    REGX16(0xBAF7002C)=IMRvalue;
}

//EP Indirect engine initial
INT32U EP_INDI_Initial()
{
    //EP_DMA_DSC
    return 1;
}

INT32U RC_INDI_Initial()
{
    //volatile INT32U *PH_ISR,*PH_IMR;
    volatile INT32U Rvalue,ph_result,Regdata;
    volatile USHORT ph_addr;
    ph_result=1;
    rlx_irq_register(BSP_PHOST_IRQ,bsp_PHost_handler); 		//register wifi dash interrupt
    //PH_ISR=WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET;
    //PH_IMR=WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4;
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=0x3FFFFFF;	//set IMR of rc_indr_int_msk
    //REGX16(WIFI_DASH_BASE_ADDR+WIFI_DASH_ELBI_TRAN_IMR_OFFSET)=0x7;			//set IMR of ELBI_TRAN_IMR
    //REGX8(WIFI_DASH_BASE_ADDR+0x0610)=REGX8(WIFI_DASH_BASE_ADDR+0x0610)&0xFE;	//rg_bypass_mode setting IO for 4281 mode

    //REGX16(WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET+0xE)=0x7;//set IMR of CMD MBOX CFG
    //REGX8(WIFI_DASH_BASE_ADDR+0x0710)=REGX8(WIFI_DASH_BASE_ADDR+0x0710)&0xFE;	//rg_bypass_mode setting CFG for 4281 mode

    //REGX8(WIFI_DASH_BASE_ADDR+0xA08)=0x3;

    //REGX8(WIFI_DASH_BASE_ADDR+0x710)=REGX8(WIFI_DASH_BASE_ADDR+0x710)|0x6;

    //enalbe device message interrupt
    Regdata = REGX32(WIFI_DASH_BASE_ADDR+0x900);
    Regdata = Regdata | BIT_0;
    REGX32(WIFI_DASH_BASE_ADDR+0x900)=Regdata;

    //set rc IO base addr
    ph_addr=0x1c;
    ph_result =ph_result|Read_RC_PCIDword(ph_addr,&Rvalue);
    Rvalue=Rvalue>>16;
    Rvalue=Rvalue<<16;
    Rvalue=Rvalue|0xD0D0;
    ph_result =ph_result|Write_RC_PCIDword(ph_addr,Rvalue);
    ph_result =ph_result|Read_RC_PCIDword(ph_addr,&Rvalue);
    //set RC IO enable memory eable bit
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
    ph_result =ph_result| Write_RC_PCIDword(0x4,Rvalue|0x7);
    return ph_result;
}

INT32U RC_INDI_Initial_cfg()
{
    //volatile INT32U *PH_ISR,*PH_IMR;
    volatile INT32U Rvalue,ph_result,Regdata;
    volatile USHORT ph_addr;
    ph_result=1;

    //PH_ISR=WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET;
    //PH_IMR=WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4;
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=0x3FFFFFF;	//set IMR of rc_indr_int_msk

    //REGX16(WIFI_DASH_BASE_ADDR+WIFI_DASH_ELBI_TRAN_IMR_OFFSET)=0x7;			//set IMR of ELBI_TRAN_IMR
    //REGX8(WIFI_DASH_BASE_ADDR+0x0610)=REGX8(WIFI_DASH_BASE_ADDR+0x0610)&0xFE;	//rg_bypass_mode setting IO for 4281 mode

    //REGX16(WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET+0xE)=0x7;//set IMR of CMD MBOX CFG
    //REGX8(WIFI_DASH_BASE_ADDR+0x0710)=REGX8(WIFI_DASH_BASE_ADDR+0x0710)&0xFE;	//rg_bypass_mode setting CFG for 4281 mode

    //REGX8(WIFI_DASH_BASE_ADDR+0xA08)=0x3;

    //REGX8(WIFI_DASH_BASE_ADDR+0x710)=REGX8(WIFI_DASH_BASE_ADDR+0x710)|0x6;

    //enalbe device message interrupt
    Regdata = REGX32(WIFI_DASH_BASE_ADDR+0x900);
    Regdata = Regdata | BIT_0;
    REGX32(WIFI_DASH_BASE_ADDR+0x900)=Regdata;

    //set rc IO base addr
    ph_addr=0x1c;
    ph_result =ph_result|Read_RC_PCIDword(ph_addr,&Rvalue);
    Rvalue=Rvalue>>16;
    Rvalue=Rvalue<<16;
    Rvalue=Rvalue|0xD0D0;
    ph_result =ph_result|Write_RC_PCIDword(ph_addr,Rvalue);
    ph_result =ph_result|Read_RC_PCIDword(ph_addr,&Rvalue);
    //set RC IO enable memory eable bit
    ph_result =ph_result| Read_RC_PCIDword(0x4,&Rvalue);
    ph_result =ph_result| Write_RC_PCIDword(0x4,Rvalue|0x7);

    return ph_result;
}

INT32U CFG_Initial(volatile USHORT bus, volatile USHORT dev,volatile USHORT fun)
{
    volatile INT32U tmpd,Wvalue,reg,config_addr,smask, omask, amask, size, reloc, min_align,base,ph_result;
    //volatile INT32U config_data;
    static volatile INT32U io_base,mem_base;
    io_base=IO_BASE_ADDR;
    mem_base=MEM_BASE_ADDR;
    ph_result=1;
    //cfg cmd reg
    ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,CMD_STATUS_REG,&tmpd);
    tmpd=tmpd | 0x7;
    ph_result=ph_result|Write_EP_PCIDword(bus,dev,fun,CMD_STATUS_REG,0x00100007);

    //cfg interrupt
    //Read_RC_PCIDword(INT_REG,&tmpd);
    ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,INT_REG,&tmpd);
    Wvalue = ((tmpd>>8)<<8)|BSP_Device_IRQ;
    ph_result=ph_result|Write_EP_PCIDword(bus,dev,fun,INT_REG,Wvalue);
    ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,INT_REG,&tmpd);

    //cfg 0x0C
    //Write_EP_PCIDword(bus,dev,fun,0xC,0x10);
    //cfg 0x98
    //Write_EP_PCIDword(bus,dev,fun,0x98,0x0400);
    //cfg 0x174
    //Write_EP_PCIDword(bus,dev,fun,0x174,0x08460846);

    pci_dev.PM_status_offset=SearchPID(0,0,0,0x01,0x04);
    pci_dev.ASPM_status_offset=SearchPID(0,0,0,0x10,0x10);
    /*set D0 status*/
    ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,pci_dev.PM_status_offset,&tmpd);
    ph_result=ph_result|Write_EP_PCIByte(bus,dev,fun,pci_dev.PM_status_offset,tmpd & 0xFC);

    /*Close ASPM*/
    ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,pci_dev.ASPM_status_offset,&tmpd);
    tmpd=tmpd&0xFC;
    tmpd=tmpd|BIT_6;
    ph_result=ph_result|Write_EP_PCIByte(bus,dev,fun,pci_dev.ASPM_status_offset,tmpd);

    ph_result=ph_result|Write_EP_PCIByte(bus,dev,fun,0x79,0x00);	//cfg max request size

    //cfg base addr
    omask = 0x00000000;
    for(reg=0; reg<6; reg++)
    {
        config_addr = BADDR0 + reg * 4;
        /* get region size */
        ph_result=ph_result|Write_EP_PCIDword(bus,dev,fun,config_addr,0xffffffff);
        ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,config_addr,&smask);
        if (smask == 0x00000000 || smask == 0xffffffff)
        {
            pci_dev.Vendor_cfg_bar[reg][0]=0;
            pci_dev.Vendor_cfg_bar[reg][1]=0;
            ph_result=ph_result|Write_EP_PCIDword(bus,dev,fun,config_addr,0x0);
            continue;
        }

        if (smask & 0x00000001)
        {
            /* I/O space */
            min_align = 1 << 7;
            amask = 0x00000001;
            base=io_base;
            pci_dev.Vendor_cfg_bar[reg][0]=0x1;
        }
        else
        {
            /* Memory Space */
            min_align = 1 << 16;
            amask = 0x0000000F;
            pci_dev.Vendor_cfg_bar[reg][0]=smask &0xEF;
            base=mem_base;
        }

        omask = smask & amask;
        smask &= ~amask;
        size = (~smask) + 1;
        pci_dev.Vendor_cfg_bar[reg][1]=size;
        reloc = base;
        if (size < min_align)
            size = min_align;
        reloc = (reloc + size -1) & ~(size - 1);
        if (io_base == base)
        {
            io_base = reloc + size;
        }
        else
        {
            mem_base = reloc + size;
        }
        ph_result=ph_result|Write_EP_PCIDword(bus,dev,fun,config_addr,reloc|omask);

    }
    return ph_result;

}
INT32U Device_Initial()
{
    volatile INT32U i,j,k,tmpd,ph_result;
    ph_result=1;
    for(i=0; i<BUS_NUM_T; i++)
    {
        for(j=0; j<DEV_NUM_T; j++)
        {
            for(k=0; k<FUN_NUM_T; k++)
            {
                if (Read_EP_PCIDword(i,j,k,0,&tmpd) && tmpd != 0xFFFFFFFF)
                {
                    pci_dev.FunNo++;
                    //BackUp_Vendor_Cfg(i,j,k,pci_dev.Default_cfg[k]);
                    ph_result=ph_result|CFG_Initial(i,j,k);
                }
            }
        }
    }
    return ph_result;
}

//Inital Fun0
void Fun0_Initial()
{
    prst_control(0);

    REGX32(0xBAF10038)|=(BIT_21|BIT_22);					//enable prst rising and falling interrupt in pcie_fun0
    REGX8(0xBAF10040)=0x00;		//set pcie host's prst/ioslate spilt with pc's prst/ioslate
//	prst_control(0);
//	Wake_control(0);
//	Wake_control(1);
    //Iso_control(0);
    Iso_control(1);				//put the isolate pin high
    WIFIDASH_DLY(100);
    prst_control(1);			//rising the prst pin
}

//Back up 256byte of  the Vendor configuration space to memory
INT32U BackUp_Vendor_Cfg(volatile USHORT bus,volatile USHORT dev,volatile USHORT fun,volatile INT32U * Memory)
{
    volatile INT32U i,ph_result=0;

    for(i=0; i<CFG_SPCACE_SIZE/4; i++)
    {
        /*back up 0x0~0x1FC*/
        if (i<0x200/4)
            ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,i*4,Memory+i);	//Back up Vendor cfg to memory

        /*back up 0x700~0x7FC*/
        else if(i>=0x200/4)
            ph_result=ph_result|Read_EP_PCIDword(bus,dev,fun,(i+0x140)*4,Memory+i);	//Back up Vendor cfg to memory

    }
    return ph_result;
}

//Back up 256byte of  the Virtual configuration space to memory
INT32U BackUp_Virtual_Cfg(volatile USHORT fun,volatile INT32U * Memory)
{
    volatile INT32U i,ph_result=0;
    for(i=0; i<CFG_SPCACE_SIZE/4; i++)
    {
        /*back up 0x0~0x1FC*/
        if (i<0x200/4)
            ph_result=ph_result|Read_Vt_PCIDword(i*4,fun,Memory+i);
        /*back up 0x700~0x7FC*/
        else if(i>=0x200/4)
            ph_result=ph_result|Read_Vt_PCIDword((i+0x140)*4,fun,Memory+i);
    }
    return ph_result;
}

//cfg the device bar type and size to fun5;
INT32U Cfg_Bar_VD2VT(volatile USHORT VDfun)
{
    volatile INT32U i,ph_result=0,addr;
    //printk("bar seting\r\n");
    for(i=0; i<6; i++)
    {
        addr=0x10+i*4;
        if(pci_dev.Vendor_cfg_bar[i][1]!=0)
        {
            /*cfg type*/
            ph_result=ph_result|Write_Vt_PCIDword(addr,VDfun,pci_dev.Vendor_cfg_bar[i][0]);
            /*cfg size*/
            ph_result=ph_result|_Write_Vt_PCIDword(addr,VDfun,pci_dev.Vendor_cfg_bar[i][1]-1,0xF,1);
        }
        else ph_result=ph_result|_Write_Vt_PCIDword(addr,VDfun,0,0xF,1);
    }
    return ph_result;
}

//copy the  vendor cfg table to wifi's  cfg space
INT32U VDtable_2_wificfg()
{
    volatile INT32U i,ph_result=0;
    for(i=1; i<CFG_SPCACE_SIZE/4; i++)
    {
        /*resume 0x0~0x1FC*/
        if (i<0x200/4)
        {
            ph_result=ph_result|Write_EP_PCIDword(0,0,0,i*4,pci_dev.Vendor_cfg[0][i]);
            ph_result=ph_result|Read_EP_PCIDword(0,0,0,i*4,&pci_dev.Vendor_cfg[0][i]);
        }
        /*resume 0x700~0x7FC*/
        else if(i>=0x200/4)
        {
            ph_result=ph_result|Write_EP_PCIDword(0,0,0,(i+0x140)*4,pci_dev.Vendor_cfg[0][i]);
            ph_result=ph_result|Read_EP_PCIDword(0,0,0,(i+0x140)*4,&pci_dev.Vendor_cfg[0][i]);
        }
    }
    return 1;
}

//copy the  Default cfg table to wifi's  cfg space
INT32U Default_table_2_wificfg()
{
    volatile INT32U i,ph_result=0;
    for(i=1; i<CFG_SPCACE_SIZE/4; i++)
    {
        /*resume 0x0~0x1FC*/
        if (i<0x200/4)
            ph_result=ph_result|Write_EP_PCIDword(0,0,0,i*4,pci_dev.Default_cfg[0][i]);
        /*resume 0x700~0x7FC*/
        else if(i>=0x200/4)
            ph_result=ph_result|Write_EP_PCIDword(0,0,0,(i+0x140)*4,pci_dev.Default_cfg[0][i]);
    }
    return 1;
}

//copy the  virtual cfg to vendor cfg
INT32U VTcfg_2_VDcfg(volatile USHORT vtfun,volatile USHORT bus,volatile USHORT dev,volatile USHORT vdfun)
{
    volatile INT32U i,ph_result=0,Rvalue;
    for(i=1; i<CFG_SPCACE_SIZE/4; i++)
    {
        /*resume 0x0~0x1FC*/
        if (i<0x200/4)
        {
            ph_result=ph_result|Read_Vt_PCIDword(i*4,vtfun,&Rvalue);
            ph_result=ph_result|Write_EP_PCIDword(bus,dev,dev,i*4,Rvalue);
        }
        /*resume 0x700~0x7FC*/
        else if(i>=0x200/4)
        {
            ph_result=ph_result|Read_Vt_PCIDword((i+0x140)*4,vtfun,&Rvalue);
            ph_result=ph_result|Write_EP_PCIDword(bus,dev,dev,(i+0x140)*4,Rvalue);
        }
    }
    return 1;
}

INT32U Inital_Adapter()
{
    volatile INT32U ph_result;
    //printk("Inital Adapter start\r\n");
    ph_result=1;
    REGX8(0xBAF10040)=0x0;	//for pcie host's prst/ioslate spilt with pc's prst/ioslate
    /*init some struct*/
    pg_status=(struct Pg_status *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET);	//Init pg_status struct for RC indirect channel
    //pci_dev.Elbi_io_info=(struct ELBI_TRAN_IO*)(WIFI_DASH_BASE_ADDR+ELBI_TRAN_IO_OFFSET);//ELBI IO info get init
    //pci_dev.Cdm_cfg_info=(struct CDM_MBOX_CFG*)(WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET);	 //CDM cfg info get init

    //rlx_irq_register(BSP_GMAC_IRQ,bsp_OOBMAC_handler);	//register OOB mac interrupt
    //REGX16(0xBAF7002C)=REGX16(0xBAF7002C)|BIT_6;		//enalbe oob mac sw interrupt

    //REGX16(0xBAF70180)=0x60;		//just for test temp


    Fun0_Initial();			//some fun0's init
    do
    {
        //}while(!(REGX8(0xBAF1004C)& BIT_0));
    }
    while(REGX8(0xBAF1004C)==0);

    //Cfg_Bar_VD2VT(0,0,0,5);
    ph_result=ph_result|RC_INDI_Initial();
    ph_result=ph_result|Device_Initial();

    //VTcfg_2_VDcfg(5,0,0,0);
    //DefCfg_2_VTCfg();
    return ph_result;

}

/*4281 inital function
1,rising the prst
2,init some struct
3,set as 4281 mode and the cfg from pc with the 4281 mode also.
4,register the oob interrupt,the PCIE host interrupt and the fun0 interrupt
5,waiting for the L0 states
6,back up the wifi devcie cfg space pci_dev.Vendor_cfg
*/
INT32U Inital_Adapter_cfg()
{
    volatile INT32U ph_result,i=0,j=0;
    //printk("Inital Adapter cfg start\r\n");
    //OOBMACWriteIBReg(0xDC32,17,16,0x3);
    REGX8(OOBMAC_BASE_ADDR+0x185)=0x00;
    //OOB2IB_W(0xD3EC,0xF,0x1000000);		//enable risc_sync_ctrl_reg:the bit set EP can not pcie link until risc_initial_done
    _2_4281_cfg();	//set as 4281 mode and the cfg from pc with the 4281 mode also.
    //PH_HW_Reset();
    Fun0_Initial();				//the fun0 interrupt handle setting
    ph_result=1;
    pg_status=(struct Pg_status *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET);	//Init pg_status struct for RC indirect channel
    //pci_dev.Elbi_io_info=(struct ELBI_TRAN_IO*)(WIFI_DASH_BASE_ADDR+ELBI_TRAN_IO_OFFSET);//ELBI IO info get init
    //pci_dev.Cdm_cfg_info=(struct CDM_MBOX_CFG*)(WIFI_DASH_BASE_ADDR+CDM_MBOX_OFFSET);	 //CDM cfg info get init

    //rlx_irq_register(BSP_GMAC_IRQ,bsp_OOBMAC_handler);	//register OOB mac interrupt
    //pci_sw_isr = bsp_pcie_sw_handler;
    REGX16(0xBAF7002C)=REGX16(0xBAF7002C)|BIT_6;			//enalbe oob mac sw interrupt
    //REGX16(0xBAF70180)=0x60;		//just for test temp

    /*polling the L0 states	*/
    do
    {
        //i++;
        //}while(!(REGX8(0xBAF1004C)& BIT_0)&&i<=100000);
    }
    while(!(REGX8(0xBAF1004C)& BIT_0));

    if(i>100000)
    {
        ph_result=ph_result|PH_ERROR_PCIELINK_FAIL;
    }
    else
    {
        /*back up the wifi device cfg space in vendor cfg table*/
        BackUp_Vendor_Cfg(0,0,0,pci_dev.Default_cfg[0]);
        memcpy(pci_dev.Vendor_cfg,pci_dev.Default_cfg,sizeof(pci_dev.Default_cfg));

        ph_result=ph_result|RC_INDI_Initial_cfg();
        ph_result=ph_result|Device_Initial();
    }
    pci_dev.Vendor_cfg[0][0]=0x816510EC;

    REGX8(OOBMAC_BASE_ADDR+0x185)=0x0E;

    return ph_result;
}

INT32U Inital_Adapter_indriver()
{
    volatile INT32U ph_result,i=0;
    ph_result=1;
    _2_4281_cfg();	//set as 4281 mode and the cfg from pc with the 4281 mode also.
    //prst_control(0);
    //Iso_control(1); 			//put the isolate pin high
    //WIFIDASH_DLY(10000);
    //prst_control(1);			//rising the prst pin

    /*polling the L0 states*/
    do
    {
        //}while(!(REGX8(0xBAF1004C)& BIT_0));
    }
    while(REGX8(0xBAF1004C)==0);


    /*Resume default cfg to wifi device*/
    Default_table_2_wificfg();
    /*back up the wifi device cfg space in vendor cfg table*/
    if(i>100000)
    {
        ph_result=ph_result|PH_ERROR_PCIELINK_FAIL;
    }
    else
    {
        ph_result=ph_result|RC_INDI_Initial_cfg();
        ph_result=ph_result|Device_Initial();
    }
    //PH_HW_Reset(0x7);
    return ph_result;
}

//Mem RW
ULONG  PCIE_MEM8_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value)
{
    volatile INT32U byte_en,value_temp,ph_result=0,IMRvalue;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(Laddr,0x01);
    ph_result=Send_TLP_Polling(MEM_TYPE,R_CMD,Haddr,0,Laddr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x1:
            *value=(INT8U)value_temp;
            break;
        case 0x2:
            *value=(INT8U)(value_temp>>8);
            break;
        case 0x4:
            *value=(INT8U)(value_temp>>16);
            break;
        case 0x8:
            *value=(INT8U)(value_temp>>24);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

ULONG  PCIE_MEM8_WRITE(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U value)
{
    volatile INT32U byte_en,value_temp,ph_result=0,IMRvalue;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(Laddr,0x01);
    value_temp = value;
    switch(byte_en)
    {
        case 0x1:
            value=value_temp;
            break;
        case 0x2:
            value=value_temp<<8;
            break;
        case 0x4:
            value=value_temp<<16;
            break;
        case 0x8:
            value=value_temp<<24;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result=ph_result|Send_TLP_Polling(MEM_TYPE,W_CMD,Haddr,0,Laddr,byte_en,0,1,TIME_OUT,&value);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
//------------------------------------------------------------------

//IO Read word
ULONG PCIE_MEM16_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value)
{
    volatile INT32U byte_en,value_temp,ph_result=0,IMRvalue;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(Laddr,0x03);
    ph_result=Send_TLP_Polling(MEM_TYPE,R_CMD,Haddr,0,Laddr,byte_en,0,1,TIME_OUT,&value_temp);
    switch(byte_en)
    {
        case 0x3:
            *value=(INT16U)value_temp;
            break;
        case 0x6:
            *value=(INT16U)(value_temp>>8);
            break;
        case 0xc:
            *value=(INT16U)(value_temp>>16);
            break;
        default:
            ph_result=ph_result|PH_ERROR_WRONGVALUE;
            break;
    }
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

ULONG  PCIE_MEM16_WRITE(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U value)
{
    volatile INT32U byte_en,value_temp,ph_result=0,IMRvalue;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    byte_en=Addr2Byte_en(Laddr,0x03);
    value_temp = value;
    switch(byte_en)
    {
        case 0x3:
            value=value_temp;
            break;
        case 0x6:
            value=value_temp<<8;
            break;
        case 0xc:
            value=value_temp<<16;
            break;
        default:
            return PH_ERROR_WRONGVALUE;
    }
    ph_result=ph_result|Send_TLP_Polling(MEM_TYPE,W_CMD,Haddr,0,Laddr,byte_en,0,1,TIME_OUT,&value);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
//------------------------------------------------------------------

//MEM Read Dword
ULONG PCIE_MEM32_READ(volatile INT32U Haddr,volatile INT32U Laddr,volatile INT32U *value)
{
    volatile INT32U IMRvalue,ph_result;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    ph_result= Send_TLP_Polling(MEM_TYPE,R_CMD,Haddr,0,Laddr,0b1111,0,1,TIME_OUT,value);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}

ULONG  PCIE_MEM32_WRITE(volatile INT32U Haddr,unsigned int Laddr, unsigned int value)
{
    volatile INT32U IMRvalue,ph_result;
    /*close imr of msg interrupt from device*/
    //IMRvalue=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue & 0xFFFFFFF0;
    ph_result=Send_TLP_Polling(MEM_TYPE,W_CMD,Haddr,0,Laddr,0b1111,0,1,TIME_OUT,&value);
    //REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=IMRvalue;
    return ph_result;
}
INT32U	SearchPID(volatile INT32U bus,volatile INT32U dev,volatile INT32U fun,volatile INT32U SPID,volatile INT32U offset)
{
    volatile INT32U ph_result,Roffset,PID;

    ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,0x34,&Roffset);
    ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,Roffset,&PID);
    while(PID!=SPID)
    {
        ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,Roffset+1,&Roffset);
        ph_result=ph_result|Read_EP_PCIByte(bus,dev,fun,Roffset,&PID);
        if (Roffset>=200)
        {
            break;
        }
    }
    return Roffset+offset;
}
void Before_shut_down()
{
    REGX8(0xBAFA090D)=REGX8(0xBAFA090D)|0x08;
    REGX8(0xBAFA090D)=REGX8(0xBAFA090D)&0xF7;
}
void Shut_down()
{
    //Write_EP_PCIByte(0,0,0,pci_dev.PM_status_offset,0xB);
    REGX8(0xBAFA090D)=REGX8(0xBAFA090D)|0x08;
    REGX8(0xBAFA090D)=REGX8(0xBAFA090D)&0xF7;
    WIFIDASH_DLY(1000);
    prst_control(0);
    OSTimeDly(OS_TICKS_PER_SEC/10);
    Iso_control(0);
}

/*
control the PCIE host's pcie rst pin by 4281
0:pcie rst falling down;
1:pcie rst rising up;
*/
void prst_control(INT8U value)
{
    if(value==0)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)&0xFE;
    }
    else if(value==1)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)|BIT_0;
    }
}
/*
control the PCIE host's pcie isolateb pin by 4281
0:isolateb falling down;
1:isolateb rising up;
*/
void Iso_control(INT8U value)
{
    if(value==0)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)&0xFD;
    }
    else if(value==1)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)|BIT_1;
    }
}

/*
control the PCIE host's pcie Wake pin by 4281
0:Wake falling down;
1:Wakerising up;
*/
void Wake_control(INT8U value)
{
    if(value==0)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)&0xFB;
    }
    else if(value==1)
    {
        REGX8(0xBAF1003C)=REGX8(0xBAF1003C)|BIT_2;
    }
}

/*When the PCIE HOST HW error, try to use the function to reset HW
   value:
   bit0:WIFIDASH_SOFT_RESET
   bit1:MCU reset ephy rc
   bit2:MCU reset pcie rc
*/
void PH_HW_Reset(INT8U value)
{
    //bit0:WIFIDASH_SOFT_RESET
    if((value & BIT_0)==BIT_0)
    {
        REGX8(WIFI_DASH_BASE_ADDR + 0xA0C)=1;
        REGX8(WIFI_DASH_BASE_ADDR + 0xA0C)=0;
    }
    //bit1:MCU reset ephy rc
    if((value & BIT_1)==BIT_1)
    {
        OOB2IB_W(0xE000,0x4,OOBMACReadIBReg(0xE000)&(~BIT_18));
        OOB2IB_W(0xE000,0x4,OOBMACReadIBReg(0xE000)|BIT_18);
    }
    //bit2:MCU reset pcie rc
    if((value & BIT_2)==BIT_2)
    {
        OOB2IB_W(0xE000,0x4,OOBMACReadIBReg(0xE000)&(~BIT_17));
        OOB2IB_W(0xE000,0x4,OOBMACReadIBReg(0xE000)|BIT_17);
    }
}

void pci_interrupt_handler_callback(void)
{

    if(r8168hRdy)
        r8168h_ISR();

#if WIFI_INIT
    ucosii_wifi_interrupt_entry();
#endif
}

void Show_Rc_cfg(INT16U size)
{
    int i = 0;
    INT32U tmpd;
    for(i=0; i<size/4; i++)
    {
        if(i%4==0)printk("\n0x%-8X",i*4);
        Read_RC_PCIDword(i*4,&tmpd);
        printk("%08X ",tmpd);

    }
}
void Show_table_cfg()
{
    int i = 0;
    for(i=0; i<CFG_SPCACE_SIZE/4; i++)
    {
        if(i%4==0)printk("\n0x%-8X",i*4);
        printk("%08X ",pci_dev.Vendor_cfg[0][i]);
    }
}

/*
OOBMAC access IBMAC : DWORD alignment bit 31-0   OCP reg
value: relative to value of lowBit to highBit
*/
INT32U OOBMACReadIBReg(INT16U reg)
{
    INT32U reg_data;
    OOB_READ_IB(MAC_BASE_ADDR,0xFF,reg,&reg_data);
    return reg_data;
}
void OOBMACWriteIBReg(INT16U reg, INT8U highBit, INT8U lowBit, INT32U value)
{
    OOB_WRITE_IB_BIT(MAC_BASE_ADDR,reg,highBit,lowBit,value);
}

void OOB2IB_W(INT16U reg,INT8U byte_en, INT32U value)
{
    OOB_WRITE_IB(MAC_BASE_ADDR,byte_en,reg,value);
}

/*Read PCIE  HOST ephy*/

volatile INT32U Rc_ephy_R(INT32U reg)
{
    volatile INT32U cmd=0,data=0;
    cmd=cmd|reg<<16;
    OOB2IB_W(0xDE28,0xF,cmd);
    WIFIDASH_DLY(10);
    while((OOBMACReadIBReg(0xDE28)&BIT_31)==0);
    data=OOBMACReadIBReg(0xDE28)&0xFFFF;
    return data;
}
volatile INT32U Rc_ephy_W(INT32U reg,INT32U data)
{
    volatile INT32U cmd=0x80000000;
    cmd=cmd|reg<<16;
    cmd=cmd|data;
    OOB2IB_W(0xDE28,0xF,cmd);
    WIFIDASH_DLY(10);
    while((OOBMACReadIBReg(0xDE28)&BIT_31)==BIT_31);
    return 0;
}
void rc_ephy_init(void)
{      
	//RC ephy parameter
	Rc_ephy_W(0x0, 0x584E);
	Rc_ephy_W(0x6, 0xF0F0);
	Rc_ephy_W(0xC, 0x219);
	Rc_ephy_W(0xD, 0xF64);
	Rc_ephy_W(0x1E, 0x08F5);
}

INT32U Wifi_driver_loading()
{
    INT32U ret = 1;
    /*	ucosii_wifi_hw_open();
    	g_scan_ok_flag = 0;
    	ucosii_wifi_scan();
    	while(g_scan_ok_flag == 0)
    	{
    		OSTimeDly(100);		//100ms
    	}
    	if(g_scan_ap_cnt == 0 && g_is_rx_hang != 1)//not rx hang, and not scan ap
    		ret = 0;//0 is error
    	ucosii_wifi_hw_close();*/
    return ret;
}
void Enable_wifidevcie_ISR()
{
    REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)=REGX32(WIFI_DASH_BASE_ADDR+WIFI_DASH_ISR_OFFSET+4)|0xF; //reset imr;
}

INT32U WIFI_Reg32(INT32U reg)
{
    WIFIDASH_DLY(3);
    return reg;
}
void WIFIDASH_DLY(INT32U i)
{
    volatile INT32U t,m;
    for (t=0; t<=i; t++)
    {
        m++;
        m++;
    }
}

/*INT8U OOBMAC_sharepin_mode_set(SharePin_type sharepin,SharePin_mode mode)
{
	INT32U	oriValue = 0x00000000;

	switch(sharepin)
	{
		case PERSTB_ISOLATEB_LANWAKE_CLKREQB:
			if(FUNCTION_MODE == mode){


				}
			else if(GPIO_MODE== mode){


				}


			break;
		case LEDPIN0TO2:
			switch(mode)
			{
				case GPIO_MODE:
					OOBMACWriteIBReg(0xDC00, 2, 2, 0);    //test_io_act = 0
					OOBMACWriteIBReg(0xDC00, 10, 10, 0);  //dbg_act = 0
					break;
			}


			break;
		case EJTAG_SMBALERT0:
			switch(mode){
				case GPIO_MODE:
					//1.ocp_gpio_en:1   controlled in GPIO Pin Control settings
					//2.dbg_chg_pin:0
					oriValue=REG32(MAC_BASE_ADDR + MAC_DBG_SEL);
					REG32(MAC_BASE_ADDR + MAC_DBG_SEL)=oriValue&(~BIT_15);
				    //REG32(MAC_BASE_ADDR + MAC_DBG_SEL) &= (~DWBIT15);
					//3.dis_jtag_mode[1:0]:11
					OOBMACWriteIBReg(0xDC00, 8, 7, 3);
					break;

				case DEBUG_MODE:

					break;
				case FUNCTION_MODE:

					break;
				case BOUNDARY_SCAN_MODE:

					break;
				case NOT_USED_MODE:

					break;




				}


			break;
		case SMBCLK0_SMBDATA0:



			break;
		case SPISCK_SPISI_SPISO0_2_3:



			break;
		case NF_ALE_NF_DD1_3_5_7:



			break;
		case NF_DD0_6_RDYWR_N_RD_N_NF_CLE:



			break;
		case UARTTX_UARTRX:



			break;
		case SMBCLK1_SMBDATA1:



			break;
		case NCSI_GPIO:



			break;
		case GPIPIN:
			switch(mode)
			{
				case GPIO_MODE:
					//1.ldps_en.linkok_gpio_en,ocp_gpi_en,gpi_oe_reg=4'b0010
					OOBMACWriteIBReg(0xE85A, 9, 9, 0);     //linkok_gpio_en
					OOBMACWriteIBReg(0xE90A, 0, 0, 0);		//ldps_en
					OOBMACWriteIBReg(0xDC06, 13,13,0);	//gpi_oe_reg
					break;

			}
			break;



	}
	return 1;

}*/

/*
function: OOBMAC gpio initial setting
parameter:
gpio_num: GPIO pinname
dir:      directiong setting :INPUT or OUTPUT
val:		 if dir=INPUT,val is ignored,but you should set related interrupt
		 if dir=OUTPUT,val is the initial output value
*/
/*void OOBMAC_gpio_init(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val)//if dir is GPIO_INPUT, then val is useless
{
     INT32U temp,temp_gpioType,temp_gpioCtrl;

	//1.sharepin mode setting
	//NOTE:GPO/GPIO1-GPIO3 only set ocp_gpio_en(the next step will do it);GPIO7:sharepin with SJTCLKPIN of Jtag,it will be test by upper layer(UGMAC ERI channel)
	switch(gpio_num)
	{
		case GPI:
			OOBMAC_sharepin_mode_set(GPIPIN,GPIO_MODE);
			break;
		case GPIO4:
		case GPIO5:
		case GPIO6:
			OOBMAC_sharepin_mode_set(LEDPIN0TO2,GPIO_MODE);
			break;
	//	case GPIO7:
	//		OOBMAC_sharepin_mode_set(EJTAG_SMBALERT0,GPIO_MODE);
	//		break;
	}
	//2.gpio configure:direction and controller right setting
	if(gpio_num>=GPIO4)
    {
		temp_gpioType = GPIO4;
		temp_gpioCtrl = MAC_GPIOCTL2;
	}
	else
	{
		temp_gpioType = GPI;
		temp_gpioCtrl = MAC_GPIOCTL;
	}

	temp = REG32(MAC_BASE_ADDR+temp_gpioCtrl);
 	temp &= ~(0xf<<((gpio_num-temp_gpioType)*4));  //first unmask the responding gpio regs

    if(OUTPUT == dir)  //2.1 if as output,you should set a initial output value
   	{
        temp |= (0x03 | (val<<2))<<((gpio_num-temp_gpioType)*4);  // enable oobmac controller and direction
		temp &= ~(1<<((gpio_num-temp_gpioType)*4+2));  //clear ori bit value
 		temp |= (val<<((gpio_num-temp_gpioType)*4+2));  //set new value for this bit
 		REG32(MAC_BASE_ADDR+temp_gpioCtrl) = temp;
   	}
    else if(INPUT== dir)  //2.2 if as input,you should enable its interrupter
   	{
        temp |= (0x01)<<((gpio_num-temp_gpioType)*4);//second, set the responding regs
		REG32(MAC_BASE_ADDR+temp_gpioCtrl) = temp;
		temp = REG32(MAC_BASE_ADDR+MAC_EXTR_INT);
		temp |= (1<<(gpio_num+2))|(1<<(gpio_num+18));//enable gpio interrupt, clear gpio interrupt states
		REG32(MAC_BASE_ADDR + MAC_EXTR_INT) = temp;
   	}

}


void OOBMAC_gpio_output(OOBMAC_gpio_no gpio_num,INT8U val)
{
	INT32U temp = 0;

	if(gpio_num>=GPIO4)
     {
     	temp = REG32(MAC_BASE_ADDR+MAC_GPIOCTL2);
     	temp &= ~(1<<((gpio_num-GPIO4)*4+2));  //clear ori bit value
     	temp |= (val<<((gpio_num-GPIO4)*4+2));  //set new value for this bit
     	REG32(MAC_BASE_ADDR+MAC_GPIOCTL2) = temp;
     }
     else
     {
        temp = REG32(MAC_BASE_ADDR+MAC_GPIOCTL);
		temp &= ~(1<<((gpio_num-GPI)*4+2));  //clear ori bit value
     	temp |= (val<<((gpio_num-GPI)*4+2));  //set new value for this bit
     	REG32(MAC_BASE_ADDR+MAC_GPIOCTL) = temp;
     }
}

INT8U OOBMAC_gpio_input(OOBMAC_gpio_no gpio_num)
{
	INT32U temp;
	temp = REG32(MAC_BASE_ADDR + MAC_STATUS);
	temp &= (1<<(gpio_num+18));
	return ((temp!=0)?1:0);
}*/

void GoTo_4281_Mode()
{
    pci_dev.Vendor_cfg[0][1]=0x00100400;
    _Write_Vt_PCIDword(4,5,pci_dev.Vendor_cfg[0][1],0xF,0);
    pci_dev.Vendor_cfg[0][0]=pci_dev.Default_cfg[0][0];
    pci_dev.Bypass_Driver_Status=1; 	//shut down flag no driver
    Inital_Adapter_indriver();
    REGX8(OOBMAC_BASE_ADDR+PCI_MSG)=TO_4281_MODE_RDY;
    pci_dev.goto4281mode=0;
}

/*INT32U Sent_TLP_R(volatile INT32U addr,volatile INT32U *value){
	volatile struct RcPage *hostpage;
	volatile INT32U *pg_value;
	volatile INT32U tmp=0;
	do{
		tmp++;
	}while(pg_status->pg_full && tmp<10000);
	if (tmp>=10000) return 2;
	hostpage =(struct RcPage *) (WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x10*(pg_status->pg_wr_ptr+1));
	pg_value =(INT32U *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x150+0x80*pg_status->pg_wr_ptr);
	//hostpage =(struct RcPage *) (WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x10*(pg_write%4+1));	//temp for pg_wr_ptr issue
	//pg_value =(INT32U *)(WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x150+0x80*(pg_write%4));		//temp for pg_wr_ptr issue
	value=pg_value;
	memset((void *)hostpage,0x0,sizeof(struct RcPage));
	hostpage->TLP_TYPE =IO_TYPE;
	hostpage->CMD = R_CMD;
	hostpage->LEN = 1;
	hostpage->FIRST_BE = 0b1111;
	hostpage->LAST_BE = 0b0;
	hostpage->ADDRL = addr;
	hostpage->INTEN = 0b1;
	hostpage->OWN = 0b1;
	return 1;
}

void rc_indr_int()
{
	volatile struct RcPage *hostpage;
	volatile INT32U tmp=0;

	for(tmp=0;tmp<4;tmp++){
		hostpage =(struct RcPage *) (WIFI_DASH_BASE_ADDR+RC_INDIRECT_CH_OFFSET+0x10*((pg_status->pg_rd_ptr+tmp)%4+1));
		if (hostpage->DONE==0b1) hostpage->DONE=0b1;
	}
	pci_dev.rc_indr_int = 0;
}*/


