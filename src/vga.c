//#include <rlx_types.h>
//#include <bsp_cfg.h>
//#include <rlx_cpu_regs.h>
#include "dma.h"
#include "dram.h"
#include "vga.h"
//#include "lib.h"
#include "bsp_cfg_fp_revA.h"
//#include "bsp_fp_revA.h"
#include "bsp.h"
//#include "rtskt.h"
#include "vnc.h"
//#include "lib.h"
//#include "PCIE_HOST.h"
#include "OOBMAC.h"

volatile INT32U debug_addr_start=0;
volatile INT32U*debug_pkt_dest=0;

//#include "GMAC.h"

//set frame buffer difference flag register
//fb0 : legacy frame buffer addr,  fb1: BAR frame buffer addr
//INT32U STD_counter;
//INT8U	VGA_flag;
//INT32U diff_timer_counter;
//INT32U black_screen_counter;
/*
extern volatile INT8U Timer_flag;//Timer interrupt flag
struct VgaInfo VGAInfo;
struct DMA_INFO dma_info1;
volatile INT8U	VGA_Reinit=0;
volatile INT32U VGAdiff_flag= NULL;
volatile INT8U	Timer_flag;
*/
#if 0
void set_frame_Block(INT16U vpixelnum,INT16U hpixelnum,INT8U byteperpixel,INT8U fb)
{

    INT8U vBlocknum,hBlocknum,vex_pixel,hex_pixel,i;
    INT32U block_byte_offset;
    INT16U FB_CTRL=0,FB_VaddrBase=0,FB_HaddrBase=0,FB_LaddrBase=0;
    if(fb==0)
    {
        FB_CTRL=FB0_CTRL;
        FB_VaddrBase=FB0_VaddrBase;
        FB_HaddrBase=FB0_HaddrBase;
        FB_LaddrBase=FB0_LaddrBase;
    }
    else if(fb==1)
    {
        FB_CTRL=FB1_CTRL;
        FB_VaddrBase=FB1_VaddrBase;
        FB_HaddrBase=FB1_HaddrBase;
        FB_LaddrBase=FB1_LaddrBase;
    }

    vBlocknum=vpixelnum/0x40;
    hBlocknum=hpixelnum/0x40;
    vex_pixel=vpixelnum%0x40;
    hex_pixel=hpixelnum%0x40;
    if(vex_pixel != 0)
        vBlocknum=vBlocknum+1;
    if(hex_pixel != 0)
        hBlocknum=hBlocknum+1;
    if(fb==0)
    {
        VGAInfo.FB0_HBnum=hBlocknum;
        VGAInfo.FB0_VBnum=vBlocknum;
    }
    else if(fb==1)
    {
        VGAInfo.FB1_HBnum=hBlocknum;
        VGAInfo.FB1_VBnum=vBlocknum;
    }
    // write FB1 VGA control register :
    //block extension pixel and number of pixel for vertical or horizontal
    //EVRI_Write(0x324, hBlocknum|(vBlocknum<<8)|(hex_pixel<<16)|(vex_pixel<<24));

    REG32(VGA_IOBASE+FB_CTRL)= hBlocknum|(vBlocknum<<8)|(hex_pixel<<16)|(vex_pixel<<24);

    //set vertical block address to HW -----
    block_byte_offset=(hpixelnum*byteperpixel*0x40);
    for(i=0; i<VBlock_maxnum; i++)
    {
        if(i<vBlocknum)
        {
            REG32(VGA_IOBASE+FB_VaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);
            //EVRI_Write(0x328+(i*0x4),0x00000000+(i*block_byte_offset));
        }
        else
        {
            REG32(VGA_IOBASE+FB_VaddrBase+(i*0x4))=0xFFFFFFFF;
            //EVRI_Write(0x328+(i*0x4),0xFFFFFFFF);
        }

    }

    //set Horizontal block address to HW
    block_byte_offset=(0x40*byteperpixel);
    REG32(VGA_IOBASE+FB_HaddrBase)=0x00000000;
    //EVRI_Write(0x36c,0x00000000);
    for(i=1; i<HBlock_maxnum; i++)
    {
        if(i<hBlocknum)
        {
            REG32(VGA_IOBASE+FB_HaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);
            //EVRI_Write(0x36c+(i*0x4),0x00000000+(i*block_byte_offset));
        }
        else
        {
            REG32(VGA_IOBASE+FB_HaddrBase+(i*0x4))=0xFFFFFFFF;
            //EVRI_Write(0x36c+(i*0x4),0xFFFFFFFF);
        }
    }

    //set in block Line address to HW
    block_byte_offset=(hpixelnum*byteperpixel);
    for(i=0; i<InBlock_maxnum; i++)
    {
        REG32(VGA_IOBASE+FB_LaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);
        //EVRI_Write((0x3e4+(i*0x4)),0x00000000+(i*block_byte_offset) );

    }



}

INT32U WIFIDASH_DBI_access(INT8U RW,INT32U addr, INT32U func_num, INT32U byte_en, INT32U data)
{
    if(RW == 1)
    {
        Read_Vt_PCIDword(addr, func_num, &data);
    }
    if(RW == 2)
    {
        Write_Vt_PCIDword(addr, func_num, data);
    }
}
/*
void VGA_test_ini()
{
int k;
STD_counter=0;
diff_timer_counter=0;
black_screen_counter=0;
VGA_flag=0;
// clear test dummy reg
REG8(VGA_IOBASE+VGA_dummy2)=0x00;

  // set BFN timer
  //FB0
  REG32(VGA_IOBASE+FB0_BFN_timer)=0x0000000F;

	//FB1
	REG32(VGA_IOBASE+FB1_BFN_timer)=0x0000000F;

	  //clear difference flag-----------------------------------------------
	  // clear fb0 diff flag
	  REG32(VGA_IOBASE+FB0_BGN)=0x0000FFFF;

		for(k=0;k<16;k++)
		{
		REG32(VGA_IOBASE+FB0_BFN_base+(k*4))=0xFFFFFFFF;
		//EVRI_Write((0x4E8+(k*4)), 0xFFFFFFFF);
		}

		  // clear fb1 diff flag
		  REG32(VGA_IOBASE+FB1_BGN)=0x0000FFFF;

			for(k=0;k<16;k++)
			{
			REG32(VGA_IOBASE+FB1_BFN_base+(k*4))=0xFFFFFFFF;
			//EVRI_Write((0x4E8+(k*4)), 0xFFFFFFFF);
			}
			//clear difference flag
			// clear fb0 DDR addr
			REG32(FB0_BGN_DDR)=0x00000000;

			  for(k=0;k<16;k++)
			  {
			  REG32(FB0_BFN_DDR+(k*4))=0x00000000;
			  }

				// clear fb1 DDR addr
				REG32(FB1_BGN_DDR)=0x00000000;

				  for(k=0;k<16;k++)
				  {
				  REG32(FB1_BFN_DDR+(k*4))=0x00000000;
				  }
				  }
*/
void VGA_pre_Initial()
{

    //hao dram_init();use old board, code use FP_4281_FPGA_20150618 (1).zip
    //Set ROM base addr
    REG32(VGA_IOBASE+0x568)=VGA_ROM_SPI_base; //VGA code 放在spi flash, 以後會放到bios, 0x82000000

    //set FB0 base addr
    REG32(VGA_IOBASE+0x558)=FB0_DDR_base; //legency, 目前暫不用到
    //set FB1 base addr
    REG32(VGA_IOBASE+0x55C)=FB1_DDR_base; //UEI, assign 8M memory(1920*1080*4), default 0x88000000


}



void VGA_initial()
{
    INT32U FB0_resolution,FB1_resolution;

    INT8U k;


#if 0
    //set pcie reset rising isr/imr
    //clear isr
    REG16(GMAC_IOBASE+0x38)=0x20;
    // set imr
    REG16(GMAC_IOBASE+0x3A)=(REG16(GMAC_IOBASE+0x3A)|0x20);
#endif
    //set vga imr & clear isr
    //REG32(VGA_IOBASE+VGA_ISR)=0x6;
    //REG32(VGA_IOBASE+VGA_IMR)=REG32(VGA_IOBASE+VGA_IMR)|0x6;

    //set STD reg imr & clear isr
    /*REG32(VGA_IOBASE+STD_reg_ISR0)=0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_ISR1)=0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_ISR2)=0x0000FFFF;
    REG32(VGA_IOBASE+STD_reg_IMR0)=REG32(VGA_IOBASE+STD_reg_IMR0)|0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_IMR1)=REG32(VGA_IOBASE+STD_reg_IMR1)|0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_IMR2)=REG32(VGA_IOBASE+STD_reg_IMR2)|0x0000FFFF;
    */
    //read the resolution and set frame block reg
    FB0_resolution=REG32(VGA_IOBASE+FB0_resol);
    FB1_resolution=REG32(VGA_IOBASE+FB1_resol);

    // set fb0 frame block reg
    if(FB0_resolution!=0)
    {
        VGAInfo.FB0_Hresol=FB0_resolution&0x00000FFF;
        VGAInfo.FB0_Vresol=(FB0_resolution&0x00FFF000)>>12;
        VGAInfo.FB0_BPP=(FB0_resolution&0xFF000000)>>24;

        //VGAInfo.FB0_HBnum=VGAInfo.FB0_Hresol/64/VGAInfo.FB0_BPP;
        //VGAInfo.FB0_VBnum=VGAInfo.FB0_Vresol/64;

        set_frame_Block(VGAInfo.FB0_Vresol, VGAInfo.FB0_Hresol,VGAInfo.FB0_BPP,0);
        REG32(VGA_IOBASE+FB0_HBN_BPP)=(VGAInfo.FB0_HBnum<<16|VGAInfo.FB0_BPP|0x00000000);

    }
    if(FB1_resolution!=0)
    {
        // set fb1 frame block reg
        VGAInfo.FB1_Hresol=FB1_resolution&0x00000FFF;
        VGAInfo.FB1_Vresol=(FB1_resolution&0x00FFF000)>>12;
        VGAInfo.FB1_BPP=(FB1_resolution&0xFF000000)>>24;
        set_frame_Block(VGAInfo.FB1_Vresol, VGAInfo.FB1_Hresol,VGAInfo.FB1_BPP,1);
        REG32(VGA_IOBASE+FB1_HBN_BPP)=(VGAInfo.FB1_HBnum<<16|VGAInfo.FB1_BPP|0x00000000);
    }
    //clear difference flag-----------------------------------------------

    // clear fb0 diff flag
    REG32(VGA_IOBASE+FB0_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB0_BFN_base+(k*4))=0xFFFFFFFF;
        //EVRI_Write((0x4E8+(k*4)), 0xFFFFFFFF);
    }

    // clear fb1 diff flag
    REG32(VGA_IOBASE+FB1_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB1_BFN_base+(k*4))=0xFFFFFFFF;
        //EVRI_Write((0x4E8+(k*4)), 0xFFFFFFFF);
    }
}



void VGA_enable(void)
{
    //hao
    //set VGA DID/VID/class code use WIFI_dash dbi channel
    WIFIDASH_DBI_access(0x02, 0x00, 0x03, 0xF, 0x816F10EF);
    //WIFIDASH_DBI_access(0x02, 0x08, 0x03, 0xF, 0x02000000); //ethernet class code for verification only

    //disable all function
#if 0
    //WIFIDASH_DBI_access(0x02, 0x00, 0x01, 0xF, 0xffffffff);
    WIFIDASH_DBI_access(0x02, 0x00, 0x02, 0xF, 0xffffffff);
    WIFIDASH_DBI_access(0x02, 0x00, 0x04, 0xF, 0xffffffff);
    //WIFIDASH_DBI_access(0x02, 0x00, 0x05, 0xF, 0xffffffff);
    WIFIDASH_DBI_access(0x02, 0x00, 0x06, 0xF, 0xffffffff);
    WIFIDASH_DBI_access(0x02, 0x00, 0x07, 0xF, 0xffffffff);
#endif

    while(1)
    {
        if(REG8(VGA_IOBASE+VGA_dummy3)==0x66)
            break;
        bsp_wait(1);
    }

    VGA_initial();
    REG8(VGA_IOBASE+VGA_dummy3+1) = 0x67;

    if(REG8(VGA_IOBASE+VGA_dummy3+2) != 1)
    {
        //tmp solution, must link at 10M
        REG32(0xbaf700a0)=0x00018f87;
        REG32(0xbaf700a4)=0x808fe0c0;
        REG8(VGA_IOBASE+VGA_dummy3+2)=1;
    }
}




void VGA_disable(void)
{
    //set 0xFFFFFFFF DID/VID/class code use WIFI_dash dbi channel
    WIFIDASH_DBI_access(0x02, 0x00, 0x03, 0xF, 0xFFFFFFFF);
    WIFIDASH_DBI_access(0x02, 0x08, 0x03, 0xF, 0x02000000); //ethernet class code for verification only
    //WIFIDASH_DBI_access(0x01, 0x08, 0x03, 0xF, 0x03000000); //VGA class code

}


/*
void update_frame_buffer_test(void)
{

  //INT32U frame_data;
  INT8U k;
  INT32U FB0_BGN_reg,FB0_BFN_reg[17], FB1_BGN_reg,FB1_BFN_reg[17];



		//frame_data = REG32(DDR_BASE);


			//check difference flag
			// set fb0 diff flag to DDR addr
			FB0_BGN_reg=REG32(VGA_IOBASE+FB0_BGN);
			REG32(FB0_BGN_DDR)=FB0_BGN_reg;
			for(k=0;k<16;k++)
			{
			FB0_BFN_reg[k]=REG32(VGA_IOBASE+FB0_BFN_base+(k*4));
			REG32(FB0_BFN_DDR+(k*4))=FB0_BFN_reg[k];
			}

			  // set fb1 diff flag to DDR addr
			  FB1_BGN_reg=REG32(VGA_IOBASE+FB1_BGN);
			  REG32(FB1_BGN_DDR)=FB1_BGN_reg;
			  for(k=0;k<16;k++)
			  {
			  FB1_BFN_reg[k]=REG32(VGA_IOBASE+FB1_BFN_base+(k*4));
			  REG32(FB1_BFN_DDR+(k*4))=FB1_BFN_reg[k];
			  }

				//clear difference flag-----------------------------------------------
				// clear fb0 diff flag
				REG32(VGA_IOBASE+FB0_BGN)=FB0_BGN_reg;

				  for(k=0;k<16;k++)
				  {
				  REG32(VGA_IOBASE+FB0_BFN_base+(k*4))=FB0_BFN_reg[k];

					}

					  // clear fb1 diff flag
					  REG32(VGA_IOBASE+FB1_BGN)=FB1_BGN_reg;

						for(k=0;k<16;k++)
						{
						REG32(VGA_IOBASE+FB1_BFN_base+(k*4))=FB1_BFN_reg[k];

						  }

							//set ok to fd diif flag test
							REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)|0x4;



								  }
*/
/*
void SendDiffBlock(INT32U startaddr,INT16U Hresol,INT16U BPP,INT32U blocknum)
{
INT16U len,packetlen;
INT32U Block_V_offset,pkt_blocknum,pkt_startaddr;
INT8U pkt_num,i;
len=64*64*BPP;
pkt_num=len/0x578;
if(len%0x578 !=0)
pkt_num++;

  Block_V_offset= Hresol*BPP;
  //normally using ZRLE engine, testing for none


	  // copy frame buffer to sequence memory buffer





				//oob send the block


					i=0;
					do{
					if((4096*BPP-pktnum*0x578)<0x578)
					txd->Length=4096*BPP-pktnum*0x578;
					else
					txd->Length=0x578;

					 for(k=0;k<(txd->Length);k++)
					 {
					 dataoffset=k+((pktnum*0x578)/(64*BPP)+Block_line)*Block_V_offset+(pktnum*0x578)%(64*BPP);
					 pktbuf[18+k] = REG8(DDR_start_addr+dataoffset);
					 if(k%(64*BPP)==((64*BPP)-1))
					 {
					 Block_line++;
					 }
					 }
					 bsp_gmac_send(2,startaddr, i,pkt_blocknum,Block_V_offset,BPP);
					 i++;
					 }while((pkt_num-i)!=0);
}*/
void update_frame_buffer_fb0(void)
{


    INT32U BGN,BFN,max_BGN;
    INT32U FB0_BGN_reg,FB0_BFN_reg[17];
    INT32U Groupnum=0;
    INT32U diff_block_start_addr,diff_block_num;
    INT32U V_Block_offset,H_Block_offset;

    V_Block_offset=VGAInfo.FB0_Hresol*64;
    H_Block_offset=64*VGAInfo.FB0_BPP;
    max_BGN=((VGAInfo.FB1_Hresol/0x40)*(VGAInfo.FB1_Vresol/0x40))/0x20;
    //check difference flag
    // set fb0 diff flag to DDR addr
    FB0_BGN_reg=REG32(VGA_IOBASE+FB0_BGN);
    for(BGN=0; BGN<16; BGN++)
    {
        if(((0x1<<BGN)&(FB0_BGN_reg))==(0x1<<BGN))
        {
            FB0_BFN_reg[BGN]=REG32(VGA_IOBASE+FB0_BFN_base+(BGN*4));
            //clear difference flag
            REG32(VGA_IOBASE+FB0_BFN_base+(BGN*4))=FB0_BFN_reg[BGN];
            Groupnum++;
        }
    }
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        for(BFN=0; BFN<0x20; BFN++)
        {
            if(((0x1<<BFN)&(FB0_BFN_reg[BGN]))==(0x1<<BFN))
            {
                diff_block_num=BGN*0x20+BFN;
                diff_block_start_addr=0x80000000+FB0_DDR_base+(diff_block_num/VGAInfo.FB0_HBnum)*V_Block_offset+(diff_block_num%VGAInfo.FB0_HBnum)*H_Block_offset;
                //SendDiffBlock(diff_block_start_addr,VGAInfo.FB0_Hresol,VGAInfo.FB0_BPP,diff_block_num);
            }
        }

    }

}

void update_frame_buffer_fb1(void)
{


    INT32U BGN,BFN,max_BGN;
    INT32U  FB1_BGN_reg,FB1_BFN_reg[17];
    INT32U Groupnum=0;
    INT32U diff_block_start_addr,diff_block_num;
    INT32U V_Block_offset,H_Block_offset;
    INT32U V_DDR_offset,H_DDR_offset;

    V_Block_offset=VGAInfo.FB1_Hresol*64*VGAInfo.FB1_BPP;
    H_Block_offset=64*VGAInfo.FB1_BPP;
    max_BGN=((VGAInfo.FB1_Hresol/0x40)*(VGAInfo.FB1_Vresol/0x40))/0x20;



    //check difference flag
    // set fb0 diff flag to DDR addr
    FB1_BGN_reg=REG32(VGA_IOBASE+FB1_BGN);
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        if(((0x1<<BGN)&(FB1_BGN_reg))==(0x1<<BGN))
        {
            FB1_BFN_reg[BGN]=REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4));
            //clear difference flag
            REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4))=FB1_BFN_reg[BGN];
            Groupnum++;
        }
        else
        {
            FB1_BFN_reg[BGN]=0x0;
        }
    }
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        for(BFN=0; BFN<0x20; BFN++)
        {
            if(((0x1<<BFN)&(FB1_BFN_reg[BGN]))==(0x1<<BFN))
            {
                diff_block_num=BGN*0x20+BFN;
                V_DDR_offset = (diff_block_num/VGAInfo.FB1_HBnum)*V_Block_offset;
                H_DDR_offset= (diff_block_num%VGAInfo.FB1_HBnum)*H_Block_offset;
                diff_block_start_addr=0x80000000+FB1_DDR_base+V_DDR_offset+H_DDR_offset;
                //hao vga_send(diff_block_start_addr,)
                //REG32(VGA_IOBASE+VGA_dummy2)=diff_block_start_addr;
                //SendDiffBlock(diff_block_start_addr,VGAInfo.FB1_Hresol,VGAInfo.FB1_BPP,diff_block_num);
            }
        }

    }


}
/*
void SendDiffBlock(INT32U startaddr,INT16U Hresol,INT16U BPP,INT32U blocknum)

{
	INT16U len,packetlen;
	INT32U Block_V_offset,pkt_blocknum,pkt_startaddr;
	INT32U*pkt_dest;
	INT32U*tmp;
	INT8U pkt_num,i,j;




	pkt_blocknum=blocknum;
	len=HerPixel*Line*BPP;					//each block len
	pkt_num=(0x400*BPP)/pck_Dlen;				//0x578=1400
	if((0x400*BPP)%pck_Dlen !=0)
		pkt_num++;


	Block_V_offset= Hresol*BPP;
//normally using ZRLE engine, testing for none

//initial send packet
		bsp_gmac_send(1,0, 0,0,0,0); // initial packet

for (j=0;j<4;j++)
	{
		pkt_dest=malloc(BPP*0x400);				//size cannot large  than 4K
		tmp=(INT32U*)pkt_dest;
		memset(tmp,0,BPP*0x400);

		dmac_test_gather(startaddr+BPP*0x400*j,tmp,BPP,&dma_info0); // DMA DDR TO DMEM

		i=0;
		do
		{
			bsp_gmac_send(2,tmp+i*pck_Dlen, i,pkt_blocknum,Block_V_offset,BPP);		//oob send the block
			i++;
		}while((pkt_num-i)!=0);
	}
free(pkt_dest);
}
*/

static struct _framebufferUpdate
{
    unsigned char msg_type;
    unsigned char padding;
    unsigned short numRect;
    unsigned short x;
    unsigned short y;
    unsigned short width;
    unsigned short height;
    unsigned int encoding_type;
} framebufferUpdate;

void vga_send_black(RTSkt* cs, char* addr, int x ,int y)
{
    PKT* txPkt;
    int rfblen = 64*64*4;
    int t;
    char *data;


    txPkt = (PKT*)allocPkt(sizeof(framebufferUpdate));
    memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
    framebufferUpdate.msg_type = 0;
    framebufferUpdate.numRect = htons(1);

    framebufferUpdate.x = htons(x);
    framebufferUpdate.y = htons(y);
    framebufferUpdate.height = htons(64);
    framebufferUpdate.width = htons(64);

    framebufferUpdate.encoding_type = 0;
    memcpy(txPkt->wp, &framebufferUpdate, sizeof(framebufferUpdate));
    rtSktSend(cs, txPkt, sizeof(framebufferUpdate));

    while(rfblen > 0)
    {
        if(rfblen >= ETH_PAYLOAD_LEN)
        {
            txPkt = (PKT*)allocPkt(ETH_PAYLOAD_LEN);
            t = ETH_PAYLOAD_LEN;
            memset(txPkt->wp, 0xff, ETH_PAYLOAD_LEN );
        }
        else
        {
            txPkt = (PKT*)allocPkt(rfblen);
            t = rfblen;
            memset(txPkt->wp, 0xff, t );
        }

        data = txPkt->wp;

        rtSktSend(cs, txPkt, ETH_PAYLOAD_LEN);
        rfblen = rfblen - ETH_PAYLOAD_LEN;
    }
}

void vga_send_dump(RTSkt* cs, char* addr)
{
#define VGA_MAX_TX_PAYLOAD 64*4*5
    PKT* txPkt;
    unsigned int rfblen = 800*600*4;
    int t;
    char *data;
    PKT* rxPkt;
    int status = 0;

    txPkt = (PKT*)allocPkt(sizeof(framebufferUpdate));
    memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
    framebufferUpdate.msg_type = 0;
    framebufferUpdate.numRect = htons(1);

    framebufferUpdate.x = htons(0);
    framebufferUpdate.y = htons(0);
    framebufferUpdate.height = htons(600);
    framebufferUpdate.width = htons(800);

    framebufferUpdate.encoding_type = 0;
    memcpy(txPkt->wp, &framebufferUpdate, sizeof(framebufferUpdate));
    rtSktSend(cs, txPkt, sizeof(framebufferUpdate));


    while(rfblen > 0)
    {
        rxPkt = rtSktRx(cs, 1, &status);
        if(rxPkt)
        {
            freePkt(rxPkt);
        }
        if(rfblen >= VGA_MAX_TX_PAYLOAD)
        {
            txPkt = (PKT*)allocPkt(VGA_MAX_TX_PAYLOAD);
            t = VGA_MAX_TX_PAYLOAD;
        }
        else
        {
            txPkt = (PKT*)allocPkt(rfblen);
            t = rfblen;
        }

        data = txPkt->wp;


        if(rfblen >= VGA_MAX_TX_PAYLOAD)
        {
            memcpy(data, addr, VGA_MAX_TX_PAYLOAD);
            addr = addr + VGA_MAX_TX_PAYLOAD;
            rtSktSend(cs, txPkt, VGA_MAX_TX_PAYLOAD);
        }
        else
        {
            memcpy(data, addr, rfblen);
            rtSktSend(cs, txPkt, rfblen);
        }

        rfblen = rfblen - VGA_MAX_TX_PAYLOAD;
    }
}

void vga_send(RTSkt* cs, char* addr, int x ,int y)
{
#define VGA_MAX_TX_PAYLOAD 64*4*5
    PKT* txPkt;
    int rfblen = 64*64*4;
    int  i = 0;
    int t;
    volatile char *data;


    if(VGAInfo.FB1_Hresol == 1024)
    {

        if( x > (VGAInfo.FB1_Hresol - 64))
            //if( x > (800 - 64))
        {
            x = 0;
            return;
        }

        if( y > (VGAInfo.FB1_Vresol - 64))
            //if( y > (600 - 64))
        {
            y= 0;
            return;
        }
    }
    else
    {
        if( x > (1024 - 64))
        {
            x = 0;
            return;
        }

        if( y > (768 - 64))
        {
            y= 0;
            return;
        }

        if((x>=800) || (y >= 600))
            vga_send_black(cs, 0, x, y);

        if((x==768) || (y == 576))
            return;
    }
    txPkt = (PKT*)allocPkt(sizeof(framebufferUpdate));
    memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
    framebufferUpdate.msg_type = 0;
    framebufferUpdate.numRect = htons(1);

    framebufferUpdate.x = htons(x);
    framebufferUpdate.y = htons(y);
    framebufferUpdate.height = htons(64);
    framebufferUpdate.width = htons(64);

    framebufferUpdate.encoding_type = 0;
    memcpy(txPkt->wp, &framebufferUpdate, sizeof(framebufferUpdate));
    rtSktSend(cs, txPkt, sizeof(framebufferUpdate));

    i = 0;
    while(rfblen > 0)
    {
        if(rfblen >= VGA_MAX_TX_PAYLOAD)
        {
            txPkt = (PKT*)allocPkt(VGA_MAX_TX_PAYLOAD);
            t = VGA_MAX_TX_PAYLOAD;
        }
        else
        {
            txPkt = (PKT*)allocPkt(rfblen);
            t = rfblen;
        }

        data = txPkt->wp;

        while(t)
        {

            if(t >= 64*4)
            {
                memcpy(data, addr, 64*4);
                t = t - 64*4;
                data = data + 64*4;
                addr = addr + (VGAInfo.FB1_Hresol*4);

            }
            else
            {
                memcpy(data, addr, t);
                addr = addr + t;
                t = 0;
            }
        }

        if(rfblen >= VGA_MAX_TX_PAYLOAD)
        {
            rtSktSend(cs, txPkt, VGA_MAX_TX_PAYLOAD);
        }
        else
            rtSktSend(cs, txPkt, rfblen);

        rfblen = rfblen - VGA_MAX_TX_PAYLOAD;
    }
}

void update_frame_buffer_hao(RTSkt* cs)
{
    INT32U BGN,BFN,max_BGN,VblockNum,HblockNum;
    INT32U  FB1_BGN_reg,FB1_BFN_reg[17];
    INT32U Groupnum=0;
    volatile INT32U diff_block_start_addr,diff_block_num;
    INT32U V_Block_offset,H_Block_offset;
    INT32U V_DDR_offset,H_DDR_offset;
    int cnt;
    int status = 0;
    PKT* rxPkt;

    if(REG8(VGA_IOBASE+VGA_dummy3)==0x66)
    {
        REG8(VGA_IOBASE+VGA_dummy3+1) = 0x67;
#if 1
        VGA_initial();
#if 0
        if((VGAInfo.FB1_Hresol != 800) || (VGAInfo.FB1_Vresol!=600))
        {
            //while(1)
            //	;
        }
#endif
#endif
        REG8(VGA_IOBASE+VGA_dummy3)=0x7;
    }

    V_Block_offset=VGAInfo.FB1_Hresol*64*VGAInfo.FB1_BPP;
    H_Block_offset=64*VGAInfo.FB1_BPP;
    HblockNum=VGAInfo.FB1_Hresol/0x40;
    VblockNum=VGAInfo.FB1_Vresol/0x40;
    if(VGAInfo.FB1_Hresol%0x40!=0)
        HblockNum++;
    if(VGAInfo.FB1_Vresol%0x40!=0)
        VblockNum++;

    max_BGN=(HblockNum*VblockNum)/0x20;

    //check difference flag
    // set fb0 diff flag to DDR addr
    FB1_BGN_reg=REG32(VGA_IOBASE+FB1_BGN);
    //1 group = 32 block
    //Group, number of group is (H*V)/(64*64*32) ==>If 1024*768, group is 6
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        if(((0x1<<BGN)&(FB1_BGN_reg))==(0x1<<BGN))
        {
            //Read block change bit from reg and restore to memory
            FB1_BFN_reg[BGN]=REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4));
            //clear difference flag
            REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4))=FB1_BFN_reg[BGN];
            Groupnum++;
        }
        else
        {
            FB1_BFN_reg[BGN]=0x0;
        }
    }
    //group block, 1 group=32 blocks
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        for(BFN=0; BFN<0x20; BFN++)
        {
            if(((0x1<<BFN)&(FB1_BFN_reg[BGN]))==(0x1<<BFN))
            {
                diff_block_num=BGN*0x20+BFN;
                V_DDR_offset = (diff_block_num/VGAInfo.FB1_HBnum)*V_Block_offset;
                H_DDR_offset= (diff_block_num%VGAInfo.FB1_HBnum)*H_Block_offset;
                diff_block_start_addr=0xa0000000+FB1_DDR_base+V_DDR_offset+H_DDR_offset;//must use uncached address
                printf("vga_send Addr:%x V:%x H:%x\n",diff_block_start_addr, (diff_block_num/VGAInfo.FB1_HBnum)*64, (diff_block_num%VGAInfo.FB1_HBnum)*64);
                vga_send(cs, diff_block_start_addr, (diff_block_num%VGAInfo.FB1_HBnum)*64,(diff_block_num/VGAInfo.FB1_HBnum)*64);
                printf("vga_send fin\n");
                //Prevent target window size zero
                //OS_TICKS_PER_SEC is 100
                rxPkt = rtSktRx(cs, OS_TICKS_PER_SEC/100, &status);

                if (status == SKTDISCONNECT)
                {
                    return;
                }
                if(rxPkt)
                    freePkt(rxPkt);
            }
        }

    }


}


/*
void STD_reg(INT32U STD_ISR0,INT32U STD_ISR1,INT32U STD_ISR2)
{

  INT32U STD0,STD1,STD2;
  if(STD_ISR0!=0x0)
  {
  STD0=REG32(VGA_IOBASE+0x00);
  STD_counter++;
  }

	if(STD_ISR1!=0x0)
	{

	  STD1=REG32(VGA_IOBASE+0x32);
	  STD_counter++;

		}

		  if(STD_ISR2!=0x0)
		  {

			STD2=REG32(VGA_IOBASE+0x32);
			STD_counter++;

			  }



}*/



/*

  void bsp_VGA_handler()
  {
  INT32U VGA_timer_ISR,VGA_timer_IMR,STD_ISR0,STD_IMR0,STD_ISR1,STD_IMR1,STD_ISR2,STD_IMR2;


	  //flag Timer interrupt handler
	  //check VGA timer interrupt
	  //VGA_timer_ISR=REG32(VGA_IOBASE+VGA_ISR)&REG32(VGA_IOBASE+VGA_IMR);
	  //VGA_timer_IMR=REG32(VGA_IOBASE+VGA_IMR);
	  //clear VGA interrupt
	  //REG32(VGA_IOBASE+VGA_ISR)=VGA_timer_ISR;
	  //REG32(VGA_IOBASE+VGA_IMR)=0x00000000;

		//store ISR
		STD_ISR0=REG32(VGA_IOBASE+STD_reg_IMR0)&REG32(VGA_IOBASE+STD_reg_ISR0);
		STD_ISR1=REG32(VGA_IOBASE+STD_reg_IMR1)&REG32(VGA_IOBASE+STD_reg_ISR1);
		STD_ISR2=REG32(VGA_IOBASE+STD_reg_IMR2)&REG32(VGA_IOBASE+STD_reg_ISR2);
		//Store IMR
		STD_IMR0=REG32(VGA_IOBASE+STD_reg_IMR0);
		STD_IMR1=REG32(VGA_IOBASE+STD_reg_IMR1);
		STD_IMR2=REG32(VGA_IOBASE+STD_reg_IMR2);
		//clear ISR and IMR

		  REG32(VGA_IOBASE+STD_reg_ISR0)=STD_ISR0;
		  REG32(VGA_IOBASE+STD_reg_IMR0)=0x00000000;
		  REG32(VGA_IOBASE+STD_reg_ISR1)=STD_ISR1;
		  REG32(VGA_IOBASE+STD_reg_IMR1)=0x00000000;
		  REG32(VGA_IOBASE+STD_reg_ISR2)=STD_ISR2;
		  REG32(VGA_IOBASE+STD_reg_IMR2)=0x00000000;



				if((VGA_timer_ISR|0x2)==0x2) //fb0/fb1 block flag time out
				{
				//set diff timer for check
				//REG32(VGA_IOBASE+VGA_dummy3)=diff_timer_counter<<16;

				  update_frame_buffer_fb0();
				  }
				  else if((VGA_timer_ISR|0x4)==0x4)
				  {
				  update_frame_buffer_fb1();

					}
					//clear diff flag




							if(STD_ISR0!=0x0 || STD_ISR2!=0x0 || STD_ISR2!=0x0) //fb0/fb1 block flag time out
							STD_reg(STD_ISR0,STD_ISR1,STD_ISR2);





									  //enable IMR
									  REG32(VGA_IOBASE+VGA_IMR)=VGA_timer_IMR;

										REG32(VGA_IOBASE+STD_reg_IMR0)=STD_IMR0;
										REG32(VGA_IOBASE+STD_reg_IMR1)=STD_IMR1;
										REG32(VGA_IOBASE+STD_reg_IMR2)=STD_IMR2;

										  }
*/

void bsp_VGA_PCIE_handler(int enVGA)
{
    if(enVGA==1)
        VGA_enable();
    else if(enVGA==0)
        VGA_disable();
}

void bsp_gmac_VGA_handler(void)
{

}


void VGA_Test_Task(void)
{
    ddr_asic_phy_init();
    ddr_asic_controllor_init();
    rlx_irq_set_handler(BSP_DMA_IRQ, bsp_DMA_handler);
    rlx_irq_set_handler(BSP_TIMER1_IRQ, bsp_ostimer_handler);
    rlx_irq_set_handler(BSP_GMAC_IRQ, bsp_gmac_VGA_handler);		//IB Write OOB Reset AND START (VGA REINIT)
    //rlx_irq_set_handler(BSP_FUN0_IRQ, bsp_Fun0_handler);		// register fun0 interrupt
    //REG32(FUN0+BMC_ISR)|=(BIT21|BIT22);					//enable prst rising and falling interrupt in pcie_fun0

}

#endif


#ifdef MAC_DEBUG_COUNT
INT32U RERCnt;
INT32U RFOVCnt;
INT32U RDUCnt;
#endif

extern volatile INT8U dma_flag;
extern RXdesc	*rxd;
extern TXdesc 	*txd;

//extern volatile INT8U * RXBUFFER = NULL;
//extern volatile INT8U * TXBUFFER = NULL;

extern INT8U	hwtxptr;
extern  INT8U	hwrxptr;
extern INT32U	TxTotal;
extern volatile INT8U	TxMixed;

extern INT32U CPU_CLK_FREQ;
/*
extern volatile FWSIG *fwsig;
extern  DPCONF *dpconf;
*/
struct VgaInfo VGAInfo;
extern volatile INT8U	VGA_Reinit;
extern volatile INT8U 	VGA_CombineTest;

volatile INT32U VGAdiff_flag= NULL;
//static INT8U times_TEST=NULL;
INT8U	Timer_flag_VGA;

extern struct DMA_INFO dma_info0;
extern void dmac_test_gather_VGA(INT32U startaddr,INT32U destaddr,INT8U BPP,struct DMA_INFO *dma_p);
extern void dmac_test_nomal(INT32U dma_size,INT32U src, INT32U des, struct DMA_INFO *dma_p);
//extern void bsp_ostimer_handler(void) ;


#if 0

//#define NEED_PATCH 9
#define BSP_TIMER_FRE  250000000

bsp_ostimer_handler_VGA(void)
{
    //static INT8U asfidx = 0;
    //static INT16U count =  3 ;
    static INT8U tcrcnt = 0;
#ifdef CONFIG_DEBUG
    static INT32U count = 0;
#endif
    volatile INT8S eoi;
    static INT8U times_TEST = 0;

    //INT8U err = 0;
    eoi = REG32(TIMER_IOBASE+TIMER_EOI);



    times_TEST++;

    if (times_TEST==3)
    {
        Timer_flag=1;
        times_TEST=0;
    }
}

void bsp_timer_init_VGA(void)
{
    INT8U divide;

    divide = (REG8(CPU_REG) & 0x07); //only 3 bits

    CPU_CLK_FREQ = (BSP_TIMER_FRE >> divide);

    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000000;             //disable first
    REG32(TIMER_IOBASE+TIMER_LC) = CPU_CLK_FREQ / HZ;      //set counter
    REG32(TIMER_IOBASE+TIMER_CR) = 0x00000003;             //enable

    rlx_irq_register(BSP_TIMER1_IRQ,bsp_ostimer_handler_VGA);
}
void bsp_setup_VGA(void)
{
    bsp_timer_init_VGA();		    /* Initialize timer */


}

void bsp_oobmac_init_VGA(void)
{
    INT16U i;
    INT32U value;
    TXdesc *txdesc;
    RXdesc *rxdesc;
    INT8U *tmp;
    INT8U RCRvalue;
    INT16U entry_number;
    INT16U test_num;
    INT16U DataBit=0;
    INT8U Valid=0;
    INT8U loop=1;
    TCAM_Entry_Setting_st stTCAM_Table= {0,};
    TCAM_Entry_Setting_st *pstTCAM_Table=NULL;
#ifdef MAC_DEBUG_COUNT
    RERCnt = 0;
    RFOVCnt = 0;
    RDUCnt = 0;
#endif
    /*Disable tx rx*/
    REG8(OOBMAC_IOBASE+MAC_CMD)=0x0;
    /*DASH Enable*/
    REG8(OOBMAC_IOBASE+MAC_OOBREG)=REG8(OOBMAC_IOBASE+MAC_OOBREG)|0x01;           //mask at moment    dash not use

    TxdescStartAddr_VGA= malloc(TxdescNumber_VGA*sizeof(struct _TxDesc));
    if(TxdescStartAddr_VGA==NULL)
    {
        bsp_4everloop(0);
    }
    RxdescStartAddr_VGA= malloc(RxdescNumber_VGA*sizeof(struct _RxDesc));
    if(RxdescStartAddr_VGA==NULL)
    {
        bsp_4everloop(0);
    }
    TallyCounterAddr = malloc(sizeof(struct _TallyCnt));
    if(TallyCounterAddr==NULL)
    {
        bsp_4everloop(0);
    }

    tmp = (INT8U*) TxdescStartAddr_VGA;
    memset(tmp, 0, sizeof(TXdesc)*TxdescNumber_VGA);
    tmp = (INT8U*) RxdescStartAddr_VGA;
    memset(tmp, 0, sizeof(RXdesc)*RxdescNumber_VGA);
    tmp = (INT8U*) TallyCounterAddr;
    memset(tmp, 0, sizeof(TallyCnt));

    /* Tx desciptor setup */
    for(i=0; i<TxdescNumber_VGA; i++)
    {
        txdesc = ((TXdesc *)TxdescStartAddr_VGA)+i;
        txdesc->FS = 1;
        txdesc->LS = 1;
        txdesc->BufferAddress = VA2PA((INT32U)malloc(TXBUFFERSTEP));
        if(i ==(TxdescNumber_VGA-1))
        {
            txdesc->EOR = 1;
        }
    }

    /* Rx descriptor setup */
    for(i=0; i<RxdescNumber_VGA; i++)
    {
        rxdesc = ((RXdesc *)RxdescStartAddr_VGA)+i;
        rxdesc->Length = 0x600;
#ifdef MAC_RX_ANY_BYTE
        rxdesc->BufferAddress = VA2PA((INT32U)malloc(RXBUFFERSTEP)+rand()%4);   //modify by lisa  rx any byte assignment
#else
        rxdesc->BufferAddress = VA2PA((INT32U)malloc(RXBUFFERSTEP));
#endif
        if(i == (RxdescNumber_VGA- 1))
        {
            rxdesc->EOR = 1;
        }
        rxdesc->OWN = 1;
    }

//	bsp_oobmac_init_packet();
//	srand((unsigned)TXBUFFER);

    REG32(OOBMAC_IOBASE+MAC_MAR0)=0x0;
    REG32(OOBMAC_IOBASE+MAC_MAR4)=0x0;
    /* Assign Tx and Rx descriptor address */
    REG32(OOBMAC_IOBASE+MAC_RDSAR)=VA2PA(RxdescStartAddr_VGA);
    REG32(OOBMAC_IOBASE+MAC_TNPDS)=VA2PA(TxdescStartAddr_VGA);

#if 1
    /* enable hw checksum & set interrupt timer unit */
    REG16(OOBMAC_IOBASE+MAC_CPCR)=0x0021;
#else
    /*No checksum offload. Rx descriptor will not record real packet type*/
    REG16(OOBMAC_IOBASE+MAC_CPCR)=0x0001;
#endif

    REG8(OOBMAC_IOBASE+MAC_RxCR) = REG8(OOBMAC_IOBASE+MAC_RxCR) & 0x0f;
    RCRvalue= REG8(OOBMAC_IOBASE+MAC_RxCR);
#ifdef MAC_TEST_VLAN_RXDETAGING
    RCRvalue |= RCR_VLANDETAGGING;
    /*Accept Bro,Multi, and physical match packets. Reject flow control, error, runt packets*/
    REG8(OOBMAC_IOBASE+MAC_RxCR)   = RCRvalue;
#endif

#ifdef MAC_ACCEPT_FLOW_CTL
    RCRvalue |= RCR_AFL;
    REG8(OOBMAC_IOBASE+MAC_RxCR)   = RCRvalue;
#endif


#ifdef MAC_FLOW_CONTROL_TEST
    //REG8(OOBMAC_IOBASE+MAC_RxCR) |= (1<<6);
    /*Set RxConf Rx_FIFO_Cond_SEL(bit 10, 9) to (0, 1)*/
    REG8(OOBMAC_IOBASE+MAC_RxCR+1) = 0x2; //OOB rx fifo nearly full/empty condition for flow control
#endif

#if 1//Setup RMS
    REG16(OOBMAC_IOBASE+MAC_RxCR+2) = 0x600;
#endif

    //Interrupt Configuration
    REG16(OOBMAC_IOBASE+MAC_IMR)=0x0000;
    REG16(OOBMAC_IOBASE+MAC_ISR)=0xFFFF;
    REG16(OOBMAC_IOBASE+MAC_IMR)=DEFAULT_IMR;
#ifdef TXUNICAST
    REG32(OOBMAC_IOBASE+MAC_IDR0)=0x00054ce0;
    REG32(OOBMAC_IOBASE+MAC_IDR4)=0x00000001;
#else
    /*Write MAC Addr*/
    REG32(OOBMAC_IOBASE+MAC_IDR0)=0x00000067;  //0x00000067
    REG32(OOBMAC_IOBASE+MAC_IDR4)=0x00000000;
#endif
#if 1//From Han
    PacketFillDefault();
#endif

#ifdef TXUNICAST
    /*Write TCAM macid1-0*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x000000e0;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010003;
#else
    /*Write TCAM macid1-0*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00006700;   //0x00000067
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010003;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

#ifdef TXUNICAST
    /*Write TCAM macid1-1*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00004c05;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010004;
#else
    /*Write TCAM macid1-1*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000000;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010004;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

#ifdef TXUNICAST
    /*Write TCAM macid1-2*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000001;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010005;
#else
    /*Write TCAM macid1-2*/
    REG32(OOBMAC_IOBASE+MAC_TCAM_DATA)=0x00000000;
    REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)=0xC0010005;
#endif
    while(((value=REG32(OOBMAC_IOBASE+MAC_TCAM_PORT)) & 0x80000000) == 0x80000000);

    /*Write Packet Rule 1*/
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE1)=0x00000002;
    /*Write Rule Act*/
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_ACT0)=0x00000201;
    /*Write packet rule enable*/
    REG32(OOBMAC_IOBASE+MAC_PKT_RULE_EN)=0x80000002;
    REG32(OOBMAC_IOBASE+0x254)=0x00000000;

    hwtxptr = 0x00;
    hwrxptr = 0x00;
    //Enable TX/RX
#if 1//For phase 2
    i = 0;
    /*Check SYNC_RXEN and SYNC_TXEN for 255 times*/
    while((REG8(OOBMAC_IOBASE+MAC_CMD+1) & 0x3) && i++ < 0xff)
    {
        bsp_wait(1);
    }
    if(i < 0xff)
    {
        REG8(OOBMAC_IOBASE+MAC_CMD)=0x0C;
    }
    else
    {
        //	bsp_4everloop(15);
    }
#else
    REG8(OOBMAC_IOBASE+MAC_CMD)=0x0C;
#endif
}
#endif

//set frame buffer difference flag register
//fb0 : legacy frame buffer addr,  fb1: BAR frame buffer addr
void set_frame_Block(INT16U vpixelnum,INT16U hpixelnum,INT8U byteperpixel,INT8U fb)
{

    INT8U vBlocknum,hBlocknum,vex_pixel,hex_pixel,i;
    INT32U block_byte_offset;
    INT16U FB_CTRL=0,FB_VaddrBase=0,FB_HaddrBase=0,FB_LaddrBase=0;
    if(fb==0)
    {
        FB_CTRL=FB0_CTRL;
        FB_VaddrBase=FB0_VaddrBase;
        FB_HaddrBase=FB0_HaddrBase;
        FB_LaddrBase=FB0_LaddrBase;
    }
    else if(fb==1)
    {
        FB_CTRL=FB1_CTRL;
        FB_VaddrBase=FB1_VaddrBase;
        FB_HaddrBase=FB1_HaddrBase;
        FB_LaddrBase=FB1_LaddrBase;
    }

    vBlocknum=vpixelnum/0x40;
    //hBlocknum=hpixelnum/(0x40*byteperpixel);
    hBlocknum=hpixelnum/0x40;
    vex_pixel=vpixelnum%0x40;
    //hex_pixel=hpixelnum%(0x40*byteperpixel);
    hex_pixel=hpixelnum%0x40;

    if(vex_pixel != 0)
        vBlocknum=vBlocknum+1;
    if(hex_pixel != 0)
        hBlocknum=hBlocknum+1;
    if(fb==0)
    {
        VGAInfo.FB0_HBnum=hBlocknum;
        VGAInfo.FB0_VBnum=vBlocknum;
    }
    else if(fb==1)
    {
        VGAInfo.FB1_HBnum=hBlocknum;
        VGAInfo.FB1_VBnum=vBlocknum;
    }
    // write FB1 VGA control register :
    //block extension pixel and number of pixel for vertical or horizontal

    REG32(VGA_IOBASE+FB_CTRL)= hBlocknum|(vBlocknum<<8)|(hex_pixel<<16)|(vex_pixel<<24);

    //set vertical block address to HW -----
    block_byte_offset=(hpixelnum*byteperpixel*0x40);						//1920*4*64
    for(i=0; i<VBlock_maxnum; i++)
    {
        if(i<vBlocknum)
        {
            REG32(VGA_IOBASE+FB_VaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);		//set VFB addr
        }
        else
        {
            REG32(VGA_IOBASE+FB_VaddrBase+(i*0x4))=0xFFFFFFFF;					//ext
        }

    }

    //set Horizontal block address to HW
    block_byte_offset=(0x40*byteperpixel);									//each block=64*4
    REG32(VGA_IOBASE+FB_HaddrBase)=0x00000000;							//first addr
    for(i=1; i<HBlock_maxnum; i++)
    {
        if(i<hBlocknum)
        {
            REG32(VGA_IOBASE+FB_HaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);		//set HFB addr
        }
        else
        {
            REG32(VGA_IOBASE+FB_HaddrBase+(i*0x4))=0xFFFFFFFF;					//ext
        }
    }

    //set in block Line address to HW
    block_byte_offset=(hpixelnum*byteperpixel);							//1920*4
    for(i=0; i<InBlock_maxnum; i++)
    {
        REG32(VGA_IOBASE+FB_LaddrBase+(i*0x4))=0x00000000+(i*block_byte_offset);		//set line addr

    }



}


/*
		//Read : 0x01, Write : 0x02
		//addr is DWORD Register Address
	INT32U WIFIDASH_DBI_access(INT8U RW,INT32U addr, INT32U func_num, INT32U byte_en, INT32U data)
	{

		REG32(WIFI_DASH_BASE_ADDR+0x400)=(addr<<2|func_num<<16);
		if(RW==0x02)
			REG32(WIFI_DASH_BASE_ADDR+0x404)=data;

		REG32(WIFI_DASH_BASE_ADDR+0x40C)=(0x00000001|byte_en<<3|RW<<1);


		do
		{}while((REG32(WIFI_DASH_BASE_ADDR+0x40C)&0x00000001)==0x00000001);

		if(RW==0x01)
			data=REG32(WIFI_DASH_BASE_ADDR+0x408);

	return data;
	}
*/
void VGA_pre_ini()
{
    volatile int k;
    STD_counter=0;
    diff_timer_counter=0;
    black_screen_counter=0;
    VGA_flag=0;
    // clear test dummy reg
    REG32(VGA_IOBASE+VGA_dummy2)=0x00000000;
    REG32(VGA_IOBASE+VGA_dummy3)=0x00000000;

    // set BFN timer
    //FB0
    REG32(VGA_IOBASE+FB0_BFN_timer)=0x0000000F;

    //FB1
    REG32(VGA_IOBASE+FB1_BFN_timer)=0x0000000F;		//30ms

    //Set ROM base addr
    REG32(VGA_IOBASE+ROM_Base_Addr)=VGA_ROM_SPI_base;

    //set FB0 base addr
    REG32(VGA_IOBASE+FB0_Base_Addr)=FB0_DDR_P_Addrbase;
    //set FB1 base addr
    REG32(VGA_IOBASE+FB1_Base_Addr)=FB1_DDR_P_Addrbase;

    //set FB0 base addr DMEM
    //REG32(VGA_IOBASE+FB0_Base_Addr)=FB0_DMEM_P_Addrbase;
    //set FB1 base addr DMEM
    //REG32(VGA_IOBASE+FB1_Base_Addr)=FB1_DMEM_P_Addrbase;

    //memset((void *)0x80071A60,0,0x4000);

    //clear difference flag-----------------------------------------------
    // clear fb0 diff flag
    REG32(VGA_IOBASE+FB0_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB0_BFN_base+(k*4))=0xFFFFFFFF;
    }
    // clear fb1 diff flag
    REG32(VGA_IOBASE+FB1_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB1_BFN_base+(k*4))=0xFFFFFFFF;
    }


    REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)|0x00000001;		//FW ok bit=1

    REG16(BMC_ISR)=REG16(BMC_ISR);
    REG16(BMC_IMR) |= PERSTB_R_STS;
}

void VGA_initial()
{
    volatile INT32U FB0_resolution,FB1_resolution;

    INT8U k;

#if 0
    //set pcie reset rising isr/imr
    //clear isr
    REG16(GMAC_IOBASE+0x38)=0x20;
    // set imr
    REG16(GMAC_IOBASE+0x3A)=(REG16(GMAC_IOBASE+0x3A)|0x20);
#endif
    //set vga imr & clear isr
    REG32(VGA_IOBASE+VGA_ISR)=0x6;				//clear flag timeout interrupt
    //REG32(VGA_IOBASE+VGA_IMR)=REG32(VGA_IOBASE+VGA_IMR)|0x6;

    //set STD reg imr & clear isr
    REG32(VGA_IOBASE+STD_reg_ISR0)=0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_ISR1)=0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_ISR2)=0x0000FFFF;
    REG32(VGA_IOBASE+STD_reg_IMR0)=REG32(VGA_IOBASE+STD_reg_IMR0)|0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_IMR1)=REG32(VGA_IOBASE+STD_reg_IMR1)|0xFFFFFFFF;
    REG32(VGA_IOBASE+STD_reg_IMR2)=REG32(VGA_IOBASE+STD_reg_IMR2)|0x0000FFFF;

    //read the resolution and set frame block reg
    FB0_resolution=REG32(VGA_IOBASE+FB0_resol);
    FB1_resolution=REG32(VGA_IOBASE+FB1_resol);

    // set fb0 frame block reg
    VGAInfo.FB0_Hresol=FB0_resolution&0x00000FFF;
    VGAInfo.FB0_Vresol=(FB0_resolution&0x00FFF000)>>12;
    VGAInfo.FB0_BPP=(FB0_resolution&0xFF000000)>>24;
    set_frame_Block(VGAInfo.FB0_Vresol, VGAInfo.FB0_Hresol,VGAInfo.FB0_BPP,0);


    // set fb1 frame block reg
    VGAInfo.FB1_Hresol=FB1_resolution&0x00000FFF;
    VGAInfo.FB1_Vresol=(FB1_resolution&0x00FFF000)>>12;
    VGAInfo.FB1_BPP=(FB1_resolution&0xFF000000)>>24;
    set_frame_Block(VGAInfo.FB1_Vresol, VGAInfo.FB1_Hresol,VGAInfo.FB1_BPP,1);		//CRTL, Vaddr, Haddr, Lineaddr

    //clear difference flag-----------------------------------------------

// clear fb0 diff flag
    REG32(VGA_IOBASE+FB0_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB0_BFN_base+(k*4))=0xFFFFFFFF;
    }

// clear fb1 diff flag
    REG32(VGA_IOBASE+FB1_BGN)=0x0000FFFF;

    for(k=0; k<16; k++)
    {
        REG32(VGA_IOBASE+FB1_BFN_base+(k*4))=0xFFFFFFFF;
    }

    //set VGA initial ok2 flag
    REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)|0x00000002;

}


//Read : 0x01, Write : 0x02
//addr is DWORD Register Address
INT32U WIFIDASH_DBI_access(INT8U RW,INT32U addr, INT32U func_num, INT32U byte_en, INT32U data)
{
    if(RW == 1)
    {
        Read_Vt_PCIDword(addr, func_num, &data);
    }
    if(RW == 2)
    {
        Write_Vt_PCIDword(addr, func_num, data);
    }
}

void VGA_enable(void)
{
    //set VGA DID/VID/class code use WIFI_dash dbi channel
    WIFIDASH_DBI_access(0x02, 0x00, 0x03, 0xF, 0x816F10EC);
    WIFIDASH_DBI_access(0x02, 0x08, 0x03, 0xF, 0x02000000); //ethernet class code for verification only

    //WIFIDASH_DBI_access(0x01, 0x08, 0x03, 0xF, 0x03000000); //VGA class code
}



void VGA_disable(void)
{
    //set 0xFFFFFFFF DID/VID/class code use WIFI_dash dbi channel
    WIFIDASH_DBI_access(0x02, 0x00, 0x03, 0xF, 0xFFFFFFFF);
    WIFIDASH_DBI_access(0x02, 0x08, 0x03, 0xF, 0x02000000); //ethernet class code for verification only
    //WIFIDASH_DBI_access(0x01, 0x08, 0x03, 0xF, 0x03000000); //VGA class code
}



void SendDiffBlock(INT32U startaddr,INT16U Hresol,INT8U BPP,INT32U blocknum)
{
    volatile INT16U len,packetlen;
    volatile INT32U Block_V_offset,pkt_blocknum,pkt_startaddr;
    volatile INT32U*pkt_dest;
    volatile INT32U*tmp;
    volatile INT8U pkt_num,i,j;


    pkt_dest=malloc(BPP*0x400);				//size cannot large  than 4K
    debug_pkt_dest = pkt_dest;
    if(pkt_dest==NULL)
    {
        bsp_4everloop(0);
    }

//		tmp=(INT32U*)pkt_dest;
    //	memset(tmp,0,BPP*0x400);

    pkt_blocknum=blocknum;
//	len=HerPixel*Line*BPP;					//each block len
    pkt_num=(0x400*BPP)/pck_Dlen;				//0x578=1400
    if((0x400*BPP)%pck_Dlen !=0)
        pkt_num++;


//	Block_V_offset= Hresol*BPP;
//normally using ZRLE engine, testing for none

    //initial send packet
    bsp_oobmac_send_VGA(1,0, 0,0,0); // initial packet

    for (j=0; j<4; j++)
    {

        debug_addr_start=startaddr+BPP*0x400*j;
        dmac_test_gather_VGA(debug_addr_start,pkt_dest,BPP,&dma_info0); // DMA DDR TO DMEM
        //	dmac_test_gather_VGA(startaddr+BPP*0x400*j,pkt_dest,BPP,&dma_info0); // DMA DDR TO DMEM

        i=0;
        do
        {
            bsp_oobmac_send_VGA(2,pkt_dest+i*pck_Dlen, i,pkt_blocknum,BPP);		//oob send the block
#ifdef MAC_LOOPBACK
            bsp_wait(30);   //30us
            //	bsp_wait(1000);   //10us
#endif
            i++;
        }
        while((pkt_num-i)!=0);
    }
    free(pkt_dest);
}


void update_frame_buffer_fb0(void)
{


    INT8U BGN,BFN,max_BGN,ex_BGN,hBlocknum,vBlocknum,hex_pixel,vex_pixel;
    INT32U FB0_BGN_reg=0,FB0_BFN_reg[17]= {0};
    INT32U Groupnum=0;
    INT32U diff_block_start_addr,diff_block_num;
    INT32U V_Block_offset,H_Block_offset;
    INT32U V_DDR_offset,H_DDR_offset;

    V_Block_offset=VGAInfo.FB0_Hresol*VGAInfo.FB0_BPP*64;
    H_Block_offset=64*VGAInfo.FB0_BPP;
    hBlocknum=VGAInfo.FB0_Hresol/0x40;
    vBlocknum=VGAInfo.FB0_Vresol/0x40;
    hex_pixel=VGAInfo.FB0_Hresol%0x40;
    vex_pixel=VGAInfo.FB0_Vresol%0x40;
    if(hex_pixel != 0)
        hBlocknum=hBlocknum+1;
    if(vex_pixel != 0)
        vBlocknum=vBlocknum+1;

    max_BGN=(hBlocknum*vBlocknum)/0x20;
    ex_BGN=(hBlocknum*vBlocknum)%0x20;
    if(ex_BGN!=0)
        max_BGN++;


//check difference flag
// set fb0 diff flag to DDR addr
    FB0_BGN_reg=REG32(VGA_IOBASE+FB0_BGN);
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        if(((0x1<<BGN)&(FB0_BGN_reg))==(0x1<<BGN))						//check which group
        {
            FB0_BFN_reg[BGN]=REG32(VGA_IOBASE+FB0_BFN_base+(BGN*4));	 //copy
            //clear difference flag
            REG32(VGA_IOBASE+FB0_BFN_base+(BGN*4))=FB0_BFN_reg[BGN];	//  W1C
            Groupnum++;
        }
        else
        {
            FB0_BFN_reg[BGN]=0x0;
        }
    }
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        for(BFN=0; BFN<0x20; BFN++)
        {
            if(((0x1<<BFN)&(FB0_BFN_reg[BGN]))==(0x1<<BFN))
            {
                diff_block_num=BGN*0x20+BFN;
                V_DDR_offset =(diff_block_num/VGAInfo.FB0_HBnum)*V_Block_offset;
                H_DDR_offset=(diff_block_num%VGAInfo.FB0_HBnum)*H_Block_offset;
                diff_block_start_addr=FB0_DDR_V_Addrbase+V_DDR_offset+H_DDR_offset;
                SendDiffBlock(diff_block_start_addr,VGAInfo.FB0_Hresol,VGAInfo.FB0_BPP,diff_block_num);

            }
        }

    }
    bsp_oobmac_send_VGA(3,0,0,0,0);

}

void update_frame_buffer_fb1(void)
{


    volatile INT8U BGN,BFN,max_BGN,ex_BGN,vBlocknum,hBlocknum,vex_pixel,hex_pixel;
    volatile INT32U  FB1_BGN_reg=0,FB1_BFN_reg[17]= {0};
    volatile INT32U Groupnum=0;
    volatile INT32U diff_block_start_addr,diff_block_num;
    volatile INT32U V_Block_offset,H_Block_offset;
    volatile INT32U V_DDR_offset,H_DDR_offset;

    V_Block_offset=VGAInfo.FB1_Hresol*VGAInfo.FB1_BPP*64;			//resolu*64(line)
    H_Block_offset=64*VGAInfo.FB1_BPP;
    hBlocknum=VGAInfo.FB1_Hresol/0x40;
    vBlocknum=VGAInfo.FB1_Vresol/0x40;
    hex_pixel=VGAInfo.FB1_Hresol%0x40;
    vex_pixel=VGAInfo.FB1_Vresol%0x40;
    if(hex_pixel != 0)
        hBlocknum=hBlocknum+1;
    if(vex_pixel != 0)
        vBlocknum=vBlocknum+1;

    max_BGN=(hBlocknum*vBlocknum)/0x20;
    ex_BGN=(hBlocknum*vBlocknum)%0x20;
    if(ex_BGN!=0)
        max_BGN++;

//check difference flag

// set fb1 diff flag to DDR addr
    FB1_BGN_reg=REG32(VGA_IOBASE+FB1_BGN);
    //delay_ms(1);

    for(BGN=0; BGN<max_BGN; BGN++)
    {
        if(((0x1<<BGN)&(FB1_BGN_reg))==(0x1<<BGN))							//Group, FB1_BGN bit=1,get BFN_reg
        {
            FB1_BFN_reg[BGN]=REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4));		//FB1_BFN_reg=flag reg
            //clear difference flag
            REG32(VGA_IOBASE+FB1_BFN_base+(BGN*4))=FB1_BFN_reg[BGN];		//w1c flag reg
            Groupnum++;
        }
        else
        {
            FB1_BFN_reg[BGN]=0x0;
        }
    }
    for(BGN=0; BGN<max_BGN; BGN++)
    {
        for(BFN=0; BFN<0x20; BFN++)
        {
            if(((0x1<<BFN)&(FB1_BFN_reg[BGN]))==(0x1<<BFN))					//Block, find flag=1
            {
                diff_block_num=BGN*0x20+BFN;
                V_DDR_offset = (diff_block_num/VGAInfo.FB1_HBnum)*V_Block_offset;
                H_DDR_offset= (diff_block_num%VGAInfo.FB1_HBnum)*H_Block_offset;
                diff_block_start_addr=FB1_DDR_V_Addrbase+V_DDR_offset+H_DDR_offset;		//block offset

                SendDiffBlock(diff_block_start_addr,VGAInfo.FB1_Hresol,VGAInfo.FB1_BPP,diff_block_num);

            }
        }

    }
    bsp_oobmac_send_VGA(3,0,0,0,0);


}




void bsp_VGA_PCIE_handler(int enVGA)
{
    if(enVGA==1)
        VGA_enable();

    else if(enVGA==0)
        VGA_disable();

}

/*
//FUN0 interrupt handler
void bsp_Fun0_handler(void){
	volatile FUN0_INT *fun0_ISR;
	volatile INT32U IMRvalue,ISRvalue;
	int enVGA=1;
	IMRvalue=REG16(FUN0+BMC_IMR);
	REG16(FUN0+BMC_IMR)=0x0000;
	ISRvalue=REG16(FUN0+BMC_ISR);
	REG16(FUN0+BMC_ISR)=REG16(FUN0+BMC_ISR);	//clean ISR
	fun0_ISR = (struct FUN0_INT*)&ISRvalue;
	if(fun0_ISR->perstb_r_sts){
		VGA_enable();
	}
	if(fun0_ISR->perstb_f_sts){
		//uart_write_str("prst falling interrupt\r\n");
		//bsp_VGA_PCIE_handler(!enVGA);
	}

	REG16(FUN0+BMC_IMR)=IMRvalue;
}

*/
void vga_dma(void)
{
    volatile INT32U VGA_timer_ISR;
    VGA_timer_ISR=REG32(VGA_IOBASE+VGA_ISR);
#if 1


    if((VGA_timer_ISR|0x4)==0x4) //fb0/fb1 block flag time out
    {
        //REG32(VGA_IOBASE+VGA_dummy3)=diff_timer_counter<<16;
        REG8(VGA_IOBASE+VGA_dummy2)=REG8(VGA_IOBASE+VGA_dummy2)&0xFE;			//clear FW OK bit
        update_frame_buffer_fb1();
        REG8(VGA_IOBASE+VGA_dummy2)=REG8(VGA_IOBASE+VGA_dummy2)|0x01;				//clear VGA initial bit to wait  VGA_initial
        //REG8(VGA_IOBASE+VGA_dummy3)=0x00000000;
        REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)&0x7FFFFFFF;
    }
    else if((VGA_timer_ISR|0x2)==0x2)
    {
        REG8(VGA_IOBASE+VGA_dummy2)=REG8(VGA_IOBASE+VGA_dummy2)&0xFE;			//clear OK bit
        update_frame_buffer_fb0();
        REG8(VGA_IOBASE+VGA_dummy2)=REG8(VGA_IOBASE+VGA_dummy2)|0x01;				//clear VGA initial bit to wait  VGA_initial
        //REG8(VGA_IOBASE+VGA_dummy3)=0x00000000;
        REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)&0x7FFFFFFF;
    }



#endif
}


void DDR_VGA_test()
{
    INT32U r0=0xA9000000,i=0;

    volatile INT32U Dtemp,Dtemp1,Dtemp2;

    while(1)
    {
        Dtemp=rand();
        *(INT32U *)(r0+i) = Dtemp;
        Dtemp1 = *(INT32U *)(r0+i);

        if(Dtemp1 != Dtemp)
            while(1);

        *(INT32U *)(r0+i)=0;
        Dtemp2 = *(INT32U *)(r0+i);
        if(Dtemp2 != 0)
            while(1);

        i+=4;
        if(i>=0x10000)
            i=0;


    }
}


void test_flow_VGA(void)
{
    INT32U tmp0,tmp1;

    tmp0=REG32(VGA_IOBASE+VGA_dummy2)&0xFFFF00;
    tmp1=REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB);   //inform IB DDR testing result

    REG8(VGA_IOBASE+VGA_dummy3+0x03)= tmp1;//DDR result

    if(tmp1==0x01)
    {
        if(tmp0==0x123400)
        {
            //VGA_Reinit=1;
            REG32(VGA_IOBASE+VGA_dummy2)=REG32(VGA_IOBASE+VGA_dummy2)&0xFF0000FF;  //FW code finish ReInitial
            REG8(VGA_IOBASE+VGA_dummy3+0x03)=tmp1;
            //VGA_Reinit=0;

            VGA_initial();

            //REG8(VGA_IOBASE+VGA_dummy2)|=0x04;  //FW code finish ReInitial
            REG32(VGA_IOBASE+VGA_dummy2)&=0xFF0000FF;
            REG32(OOBMAC_IOBASE + MAC_DUMMY_INFORM_IB)=0;
        }
    }



    //if(VGA_Reinit==1)
    //{
    //VGA_Reinit=0;
    //VGA_initial();
    //REG8(VGA_IOBASE+VGA_dummy2)=REG8(VGA_IOBASE+VGA_dummy2)|0x00000004;  //FW code finish ReInitial

    //}

}


void VGA_Test_Task(void)
{

    //ddr_asic_phy_init();
    //ddr_asic_controllor_init();
    /*	rlx_irq_register(BSP_DMA_IRQ, bsp_DMA_handler);
    	//rlx_irq_register(BSP_TIMER1_IRQ, bsp_timer_handler);
    //	rlx_irq_register(BSP_GMAC_IRQ, bsp_oobmac_handler);
    	rlx_irq_register(BSP_FUN0_IRQ, bsp_Fun0_handler);		// register fun0 interrupt
    	REG32(FUN0+BMC_ISR)|=(BIT21|BIT22);					//enable prst rising and falling interrupt in pcie_fun0
    	//bsp_gmac_init();
    	bsp_oobmac_init_VGA();
    	VGA_pre_ini();			//set 30ms, clear dummy reg OKbit ,clear diff flag ,DDR address*/
    //Timer_initial(test_VGA); //called in startTask
    while(1)
    {
        test_flow_VGA();
        OSTimeDly(OS_TICKS_PER_SEC/10);

    }
}

/*
void bsp_gmac_VGA_handler(void)
{
	volatile INT16U val;
	REG16(MAC_BASE_ADDR + MAC_IMR) = 0x0000;
	//--------------------
	val = REG16(MAC_BASE_ADDR + MAC_ISR);
	REG16(MAC_BASE_ADDR + MAC_ISR) = val;
	VGA_Reinit=1;

	REG16(MAC_BASE_ADDR + MAC_IMR) = DEFAULT_IMR;
}
*/
