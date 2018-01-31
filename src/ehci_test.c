/***************************************/
/*  Date: 2010.09.10                                      */
/*  Editor: Tom                                              */
/*  Reason: RTK EHCI Design                         */
/*  Firmware version: Draft                            */
/***************************************/
#include "ehci_test.h"
#include "usb_test.h"
//#include <rlx/rlx_funs.h>

// tomadd 2010.11.05
// reason for USB2.0 chapter9 packet
struct usb_ctrlrequest_test *setup_pkt;
int device_addr;                          //Sophia 
extern struct command_block_wrapper *CBW;
extern struct command_status_wrapper *CSW;
INT8U CTRLDescOUTIndex=0,CTRLDescINIndex=0,BulkDescOUTIndex=0,BulkDescINIndex=0;
INT8U INT1DescINIndex=0,INT2DescINIndex=0;
INT8U ADDROffset=0;
INT8U counter=0;
//tomadd 20111020
//debug for EP phase II
INT8U EHCINT=0;
//INT32U RLength=0;
#ifdef bulk_in_test
INT32U OUTBUFLEN=0x200;//512!!!! 
#else
INT32U OUTBUFLEN=0x200;//512bytes
#endif
INT32U TotalLen=0;
//tomadd 2011.05.31
extern unsigned int inLen ;
INT32U rlen=0;
//tomadd 2011.08.23
extern unsigned int setup_count;
#ifdef EHCISTALL
//tomadd 2011.09.18
extern unsigned int ep_stall;
#endif

//INT8U descriptor_prepare_in1; 
//INT8U descriptor_prepare_in2;
INT16U bulk_counter;

INT8U setaddress_counter=0;
INT8U PORT1_address=0;
INT8U PORT2_address=0;	
INT8U PORT3_address=0;
INT8U HID_ep;
INT8U is_out;



//USB 
// USB DESCRIPTOR ADDRESS
INT8U * DESC_ADDR;

// USB CSW ADDRESS
INT8U * CSW_ADDR;





// READ FROM DMEM ADDRESS

 

//EHCI
// for control out descriptor start address
controloutdesc_r * CTLOUTdescStartAddr_EHCI;


// for control in descriptor start address
outindesc_r * CTLINdescStartAddr_EHCI;


// for bulk out descriptor start address
outindesc_r * BULKOUTdescStartAddr;


// for bulk in descriptor start address
outindesc_r * BULKINdescStartAddr;


//for interrupt 1 in descriptor address neil
outindesc_r * INTIN1descStartAddr_EHCI;


//for interrupt 1 in descriptor address neil
outindesc_r * INTIN2descStartAddr_EHCI;




/** EHCI engine out transaction start buffer address **/
INT8U * CTLOUT_BUFSTARTADDR;
INT8U *	OUT_BUFSTARTADDR;
INT8U * BulkINADDR;
INT8U * INTIN1ADDR;
INT8U * INTIN2ADDR;



#ifdef	__HIGH_SPEED__
#define ep_mps 	512	//high speed
#define ep0_mps 64	//high speed
#else
#define ep_mps 	64	//full speed
#define ep0_mps 64	//full speed
#endif



static struct usb_device_descriptor
MassStorage_device_desc = {
	.bLength =		sizeof MassStorage_device_desc,
	.bDescriptorType =	USB_DT_DEVICE,
#ifdef	__HIGH_SPEED__	
	//.bcdUSB =		0x0002,
	.bcdUSB =		0x0200,
#else
	.bcdUSB =		0x1001,
#endif	
	.bDeviceClass =		USB_CLASS_PER_INTERFACE,
	.bDeviceSubClass =      0,
	.bDeviceProtocol =      0,	
	.bMaxPacketSize0 =      ep0_mps,
	.idVendor =		0x10EC,//0xDA0B,
	.idProduct =		0x8168,//0x7286,dostool will use idproduct  to act different test flow , this place is mass storage 
	.bcdDevice =            0x0100,//0x0001,
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
};

/*Config descriptor*/
static unsigned char MassStorage_descriptors[] = { 0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xC0, 0x32,
			      0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x06, 0x50, 0x00,
#ifdef	__HIGH_SPEED__			      
			      0x07, 0x05, 0x81, 0x02, 0x00, 0x02, 0x00,//0x01,
			    0x07, 0x05, 0x02, 0x02, 0x00, 0x02, 0x00 };//0x01
#else
			      0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 0x01,
			      0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x01 };
#endif			      
			      
static unsigned char str0[] = { 0x04, 0x03, 0x09, 0x04 };		
static unsigned char str1[] = { 0x10 , 0x03, 'R', 0x00, 'e', 0x00, 'a', 0x00, 'l', 0x00, 't', 0x00,
			 'e', 0x00, 'k', 0x00 }; 		 
static unsigned char str2[] = { 0x16 , 0x03, 'A', 0x00, 'T', 0x00, 'A', 0x00, ' ', 0x00, 'B', 0x00,
			 'r', 0x00, 'i', 0x00, 'd', 0x00, 'g', 0x00, 'e', 0x00 }; 
#if 0			 
unsigned char str3[] = { 0x06 , 0x03, '0', 0x00, '0', 0x00 }; 	
#else
static unsigned char str3[] = { 0x1C, 0x03, 0x30, 0x00, 0x33, 0x00, 0x35, 0x00,
	                       0x37, 0x00, 0x41, 0x00, 0x30, 0x00, 0x32, 0x00,
	                       0x31, 0x00, 0x31, 0x00, 0x43, 0x00, 0x30, 0x00,
	                       0x41, 0x00, 0x30, 0x00};//0x0A, 0x03, 0x30, 0x00, 0x38, 0x00, 0x37, 0x00, 0x35, 0x00}; 
#endif



#ifndef COMPOSITE_HID

/******************keyboard************************************/  
static struct usb_device_descriptor
KeyBoard_device_desc = {
	.bLength =		sizeof KeyBoard_device_desc,
	.bDescriptorType =	USB_DT_DEVICE,
#ifdef	__HIGH_SPEED__	
	//.bcdUSB =		0x0002,
	.bcdUSB =		0x0200,
#else 
	.bcdUSB =		0x1001,
#endif	
	.bDeviceClass =		USB_CLASS_PER_INTERFACE, //Note that the HID class is defined in the Interface descriptor. 
	.bDeviceSubClass =      0,
	.bDeviceProtocol =      0,	
	.bMaxPacketSize0 =      ep0_mps,
	.idVendor =		0x10EC,//0xDA0B,
	.idProduct =		0x0001,//0x7286, this place will be match to dos tool
	.bcdDevice =            0x0100,//0x0001,
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
};

/*Config descriptor*/
// When a Get_Descriptor(Configuration)request is issued, it 
//returns the Configuration descriptor, all Interfacedescriptors, all Endpoint
//descriptors, and the HIDdescriptor for each interface. It shall not return the 
//Stringdescriptor, HID Report descriptor or any of the optional HIDclass 
//descriptors. The HIDdescriptor shall be interleaved between the Interfaceand 
//Endpointdescriptors for HID Interfaces. That is, the order shall be: 
//Configuration descriptor 
//	Interface descriptor (specifying HID Class) 
// 		HID descriptor (associated with above Interface) 
//  			Endpoint descriptor (for HID Interrupt In Endpoint) 
// 			 Optional Endpoint descriptor (for HID Interrupt Out Endpoint) 

static unsigned char KeyBoard_descriptors[] = { 0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, //Configuration Descriptor
			      0x09, 0x04, 0x00, 0x00, 0x01, 0xff, 0x01, 0x01, 0x00, 			//Interface Descriptor			      
				  0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x3F,0x00,	//HID Descriptor
#ifdef	__HIGH_SPEED__			      //Endpoint Descriptor
			      0x07, 0x05, 0x81, 0x03, 0x00, 0x02, 0x0A,//0x01,
#else
			      0x07, 0x05, 0x81, 0x03, 0x40, 0x00, 0x0A, 
#endif			      



};

/*
unsigned char str0[] = { 0x04, 0x03, 0x09, 0x04 };		
unsigned char str1[] = { 0x10 , 0x03, 'R', 0x00, 'e', 0x00, 'a', 0x00, 'l', 0x00, 't', 0x00,
			 'e', 0x00, 'k', 0x00 }; 		 
unsigned char str2[] = { 0x16 , 0x03, 'A', 0x00, 'T', 0x00, 'A', 0x00, ' ', 0x00, 'B', 0x00,
			 'r', 0x00, 'i', 0x00, 'd', 0x00, 'g', 0x00, 'e', 0x00 }; 
#if 0			 
unsigned char str3[] = { 0x06 , 0x03, '0', 0x00, '0', 0x00 }; 	
#else
unsigned char str3[] = { 0x1C, 0x03, 0x30, 0x00, 0x33, 0x00, 0x35, 0x00,
	                       0x37, 0x00, 0x41, 0x00, 0x30, 0x00, 0x32, 0x00,
	                       0x31, 0x00, 0x31, 0x00, 0x43, 0x00, 0x30, 0x00,
	                       0x41, 0x00, 0x30, 0x00};//0x0A, 0x03, 0x30, 0x00, 0x38, 0x00, 0x37, 0x00, 0x35, 0x00}; 
#endif

*/
/********************************************************************************/

/******************Mouse************************************/  
static struct usb_device_descriptor
Mouse_device_desc = {
	.bLength =		sizeof Mouse_device_desc,
	.bDescriptorType =	USB_DT_DEVICE,
#ifdef	__HIGH_SPEED__	
	//.bcdUSB =		0x0002,
	.bcdUSB =		0x0200,
#else 
	.bcdUSB =		0x1001,
#endif	
	.bDeviceClass =		USB_CLASS_PER_INTERFACE, //Note that the HID class is defined in the Interface descriptor. 
	.bDeviceSubClass =      0,
	.bDeviceProtocol =      0,	
	.bMaxPacketSize0 =      ep0_mps,
	.idVendor =		0x10EC,//0xDA0B,
	.idProduct =		0x0002,//0x7286, this place will be match to dos tool
	.bcdDevice =            0x0100,//0x0001,
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
};

/*Config descriptor*/
static unsigned char Mouse_descriptors[] = { 0x09, 0x02, 0x22, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, //Configuration Descriptor
			      0x09, 0x04, 0x00, 0x00, 0x01, 0x03, 0x01, 0x02, 0x00, 			//Interface Descriptor			      
				  0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x32,0x00,	//HID Descriptor
#ifdef	__HIGH_SPEED__			      //Endpoint Descriptor
			      0x07, 0x05, 0x81, 0x03, 0x00, 0x02, 0x0A,//0x01,
#else
			      0x07, 0x05, 0x81, 0x03, 0x40, 0x00, 0x0A, 
#endif			      



};


#else
	/******************composite hid************************************/  
	static struct usb_device_descriptor
	composite_hid_desc = {
		.bLength =		sizeof composite_hid_desc,
		.bDescriptorType =	USB_DT_DEVICE,
	#ifdef	__HIGH_SPEED__	
		//.bcdUSB =		0x0002,
		.bcdUSB =		0x0200,
	#else 
		.bcdUSB =		0x1001,
	#endif	
	.bDeviceClass =		USB_CLASS_PER_INTERFACE, //Note that the HID class is defined in the Interface descriptor. 
	.bDeviceSubClass =      0,
	.bDeviceProtocol =      0,	
	.bMaxPacketSize0 =      ep0_mps,
	.idVendor =		0x10EC,//0xDA0B,
	.idProduct =		0x0003,//0x7286, this place will be match to dos tool
	.bcdDevice =            0x0100,//0x0001,
	.iManufacturer =	STRING_MANUFACTURER,
	.iProduct =		STRING_PRODUCT,
	.iSerialNumber =	STRING_SERIAL,
	.bNumConfigurations =	1,
	};

	static unsigned char composite_hid_descriptors[] = { 0x09, 0x02, 0x3b, 0x00, 0x01, 0x01, 0x00, 0x80, 0x32, //Configuration Descriptor
				      0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x01, 0x01, 0x00, 			//Interface Descriptor			      
					  0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x3F,0x00,	//HID Descriptor
					  0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x32,0x00,	//HID Descriptor	 
					  0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x3F,0x00,	//HID Descriptor just test acceed 64 bytes payload
					  //0x09,0x21,0x01,0x01,0x00,0x01,0x22,0x3F,0x00,	//HID Descriptor just test acceed 64 bytes payload
					  #ifdef	__HIGH_SPEED__			      //Endpoint Descriptor
				      0x07, 0x05, 0x81, 0x03, 0x00, 0x02, 0x0A,//0x01,
				      0x07, 0x05, 0x82, 0x03, 0x00, 0x02, 0x0A,//0x01,
					  #else
				      0x07, 0x05, 0x81, 0x03, 0x40, 0x00, 0x0A, 
				      0x07, 0x05, 0x82, 0x03, 0x40, 0x00, 0x0A, 
					  #endif			      
					 };

#endif 

static ep0state_e ep0state[Max_USBD_TPYE] = {EP0_DISCONNECT,EP0_DISCONNECT};
static ep_state_e epstate[Max_USBD_TPYE] = {EP_CBW,EP_CBW};
static unsigned char clearfeature = 0;
static unsigned int totalLen = 0;
static volatile unsigned int *bufaddr;
static unsigned char LUN = 0;







void recycle_outdesc_copy_data(outindesc_r *outdesc, INT8U* buf);
void recycle_outdesc(outindesc_r *outdesc);
void rtkehci_ep_start_transfer_test(unsigned int len, unsigned int*addr, unsigned char is_in);
void rtkehci_ep_start_transfer_mps(INT32U len, INT8U*addr, unsigned char is_in);

void bulk_out_ufi_write(INT32U len)
{

}

//tomadd 2011.07.28
//for bulk out bulk in data compare test
int bulk_out_in_test(INT32U len)
{
	int i=0;
	INT8U *addr;
	INT32U *bufaddr,*startaddr;
	outindesc_r *outdesc;

	outdesc = (outindesc_r *) (BULKOUTdescStartAddr)+BulkDescOUTIndex;  
	if(counter==16)
		counter=0;
	
  	addr=(INT8U *)(BulkINADDR);		
  	bufaddr=(INT8U *)PA2VA(outdesc->outin_buf_lowaddr);
	
  	TotalLen=TotalLen+len;
  	inLen=*bufaddr;

	bulk_counter++; //counter bulk out 
  	
  	if(len==inLen)
  	{
		#ifdef ANYBYTE
	  	counter++;
	  	if(counter==1)
	  		addr=addr+1;
	  	else if(counter==2)
	  		addr=addr+2;
	  	else if(counter==3)
	  		addr=addr+3;
	  	else if(counter==4)
	  		addr=addr+4;
	  	else if(counter==5)
	  		addr=addr+5;
	  	else if(counter==6)
	  		addr=addr+6;
	  	else if(counter==7)
	  		addr=addr+7;
	  	else if(counter==8)
	  		addr=addr+8;
	  	else if(counter==9)
	  		addr=addr+9;
	  	else if(counter==10)
	  		addr=addr+10;
	  	else if(counter==11)
	  		addr=addr+11;
	  	else if(counter==12)
	  		addr=addr+12;
	  	else if(counter==13)
	  		addr=addr+13;
	  	else if(counter==14)
	  		addr=addr+14;
	  	else if(counter==15)
	  		addr=addr+15;
		#endif	
		
  		memcpy( addr, bufaddr, len);//copy data to bulk in buffer address

		#if 1
		rtkehci_ep_start_transfer_test(TotalLen,addr,is_IN);
		TotalLen=0;		  	
		#endif
  	}
  	else
	{
		#ifdef ANYBYTE	  	
	  	if(counter==1)
	  		addr=addr+1;
	  	else if(counter==2)
	  		addr=addr+2;
	  	else if(counter==3)
	  		addr=addr+3;
	  	else if(counter==4)
	  		addr=addr+4;
	  	else if(counter==5)
	  		addr=addr+5;
	  	else if(counter==6)
	  		addr=addr+6;
	  	else if(counter==7)
	  		addr=addr+7;
	  	else if(counter==8)
	  		addr=addr+8;
	  	else if(counter==9)
	  		addr=addr+9;
	  	else if(counter==10)
	  		addr=addr+10;
	  	else if(counter==11)
	  		addr=addr+11;
	  	else if(counter==12)
	  		addr=addr+12;
	  	else if(counter==13)
	  		addr=addr+13;
	  	else if(counter==14)
	  		addr=addr+14;
	  	else if(counter==15)
	  		addr=addr+15;
		#endif	
		
  		memcpy( addr+rlen, bufaddr, len);//copy data to bulk in buffer address

		if((rlen)>0x4000)
		{
	  	  	#if 1
	  	  	rtkehci_ep_start_transfer_test(TotalLen,addr,is_IN);		  	
	  	  	#endif
	  	  	rlen=0;
	  	  	TotalLen=0;
	  	  	counter++;
		}
		else
		{
			rlen=len;
		}
  	}		  	
}

void rtkehci_ep_start_transfer_mps(INT32U len, INT8U*addr, unsigned char is_in)
{
	INT32U length=len;
	INT8U*address=addr;
	
	do{
		if(length<512)
		{
			rtkehci_ep_start_transfer_test(length, address, is_in);
		}
		else
		{
			rtkehci_ep_start_transfer_test(512, address, is_in);
		}
		
		length-=512;
		address+=512;
		bsp_wait(2000);
	}while(length);
}

//tomadd 2011.05.02
//EHCI Bulk IN transfer
void rtkehci_ep_start_transfer_test(unsigned int len, unsigned int*addr, unsigned char is_in)
{
    INT8U *outaddr,*inaddr;
    int i,j;
    INT32U tmp,tcounter=0;
    outindesc_r *indesc = (outindesc_r *)(BULKINdescStartAddr)+BulkDescINIndex;
    outindesc_r *outdesc;
	
    BulkDescINIndex = ( BulkDescINIndex ) % BULKINdescNumber;    
    outdesc = (outindesc_r *) (BULKOUTdescStartAddr)+BulkDescOUTIndex;	
    inaddr = (unsigned int)addr & ~Uncache_Mask;

    j=1;
    if (is_in)
    {
	while(((REG32(CMDSTS)&0x00000080)!=0)/*&& tcounter<1000*/)
	{
		tcounter++;
	}

		  #ifdef EHCISTALL
          if(ep_stall==1)
          	indesc->stoi.stall = 1;//set stall
          else
          	indesc->stoi.stall = 0;//clear stall
          #endif
			
		  //for (i = 0; i < j; i++)
          //{
	           //prevent hardware overwrite each other
	          // if (i == (j-1))
	           //{
		             //indesc->stoi.length = len-(i*OUTBUFLEN);
		             indesc->stoi.length = len;//-(i*0x4600);
		             indesc->stoi.ls  = 1;//last segment
			     	indesc->stoi.fs  = 1;
		             indesc->outin_buf_lowaddr = (INT8U *) VA2PA(inaddr);//+(0x4600*i);
	           //}
	           //else
	          // {
		   //          indesc->stoi.length = len-0x0A00; // buffer size = 4kbytes
		   //          indesc->stoi.ls  = 0;
		   //          indesc->stoi.fs  = 1;//first segement
		   //          indesc->outin_buf_lowaddr = (INT8U *) VA2PA(inaddr);
	           //}
		    indesc->stoi.own = 1;
	            indesc++;
            
          //}
          //while(((REG32(CMDSTS)&0x00000080)!=0)/*&& tcounter<1000*/)
          //{
          //	tcounter++;
         //}
          REG32(CMDSTS) |= 0x00000080;
    }
    else
    {
    	recycle_outdesc_copy_data(outdesc,addr);
    }
}



void rtkehci_ep0_start_transfer_test(unsigned short len, unsigned int*addr, unsigned char is_in)
{
    volatile INT8U *ctloutaddr,*ctlinaddr;
    outindesc_r *ctlindesc = (outindesc_r *)(CTLINdescStartAddr_EHCI)+CTRLDescINIndex;
    CTRLDescINIndex = ( CTRLDescINIndex + 1 ) % CTLINdescNumber;
	  INT32U tcounter=0;
    ctlinaddr = (unsigned int)addr & ~Uncache_Mask;
    ctlindesc->outin_buf_lowaddr=(INT8U *) VA2PA(ctlinaddr);
	
	while((REG32(CMDSTS)&0x00000001)!=0); //neil modify !!!
	//while(((REG32(CMDSTS)&0x00000001)!=0) && tcounter<1000)
    //{
    //    tcounter++;
    //}
    if (is_in)
    {
          #ifdef EHCISTALL
          if(ep_stall==1)
          	ctlindesc->stoi.stall = 1;//set stall
          else
          	ctlindesc->stoi.stall = 0;//clear stall
          #endif
		  ctlindesc->stoi.fs = 1;//first segment neil modify!!!
		  ctlindesc->stoi.ls = 1;//last segment
          ctlindesc->stoi.length = len;
          ctlindesc->stoi.own= 1; 
          //command status register
          //bit0: control in descriptor polling queue
          //while(((REG32(CMDSTS)&0x00000001)!=0) && tcounter<1000)
          //{
          //	tcounter++;
          //}
          REG32(CMDSTS) |= 0x00000001;
		  while((REG32(CMDSTS)&0x00000001)!=0);
    }
}


//neil add 2015.05.19
//EHCI interrupt IN transfer
//call function need poll the Key_polling_bit or Mouse_polling_bit
//port num is 1 or 2
int rtkehci_INTIN_start_transfer_test(INT8U portnum ,unsigned short len, unsigned int*addr, unsigned char is_in)
{
    INT8U *intinaddr;
    outindesc_r *intindesc ; 
	INT32U tcounter=0;

	if (portnum == 1)
	{
		intindesc = (outindesc_r *)(INTIN1descStartAddr_EHCI)+INT1DescINIndex;
		INT1DescINIndex = ( INT1DescINIndex + 1 ) % INTIN1desNumber;
	}
	else if (portnum == 2)
	{
		intindesc = (outindesc_r *)(INTIN2descStartAddr_EHCI)+INT2DescINIndex;
		INT2DescINIndex = ( INT2DescINIndex + 1 ) % INTIN2desNumber;
	}
	
	//descriptor is full  and not be moved to IB(not used) ,return !
	if((intindesc->stoi.own == 1) || ((REG32(CMDSTS)&(0x00000001<<portnum))!=0)){
		return -1;
	}

	//Transfer DMEM physical address to FW virtual address
    intindesc->outin_buf_lowaddr=(INT8U *) VA2PA(addr);

    if (is_in)
    {
          #ifdef EHCISTALL
          if(ep_stall==1)
          	intindesc->stoi.stall = 1;//set stall
          else
          	intindesc->stoi.stall = 0;//clear stall
          #endif
          intindesc->stoi.ls = 1;//last segment
          intindesc->stoi.length = len;
          intindesc->stoi.own= 1; 
          //command status register
          //bit0: control in descriptor polling queue
          while(((REG32(CMDSTS)&(0x00000001<<portnum))!=0) && tcounter<1000)
          {
          	tcounter++;
          }
          REG32(CMDSTS) |= (0x00000001<<portnum);
    }

	return 0;
}






//tomadd 2011.05.02
//recycle control out descriptor
void recycle_ctloutdesc_test(controloutdesc_r *ctloutdesc)
{
    INT8U i = 0;
    INT8U err;
    if (!ctloutdesc->stoi.own)
    {
		ctloutdesc->stoi.length = 0x40;
    	ctloutdesc->stoi.own = 1;
        CTRLDescOUTIndex= ( CTRLDescOUTIndex+ 1 )% (CTLOUTdescNumber) ;
        ctloutdesc = (outindesc_r *) CTLOUTdescStartAddr_EHCI + CTRLDescOUTIndex;
    }
	
}

void recycle_outdesc_copy_data(outindesc_r *outdesc, INT8U* buf)
{
	INT8U i = 0;
	INT8U err;
	INT8U *addr;

	if (!outdesc->stoi.own)
	{
		while (i++ < BULKOUTdescNumber)
		{              
			memcpy(buf+(i*512), (INT8U *)PA2VA(outdesc->outin_buf_lowaddr),512);
			//recycle out descriptor
    		        outdesc->stoi.length = OUTBUFLEN;
    		        outdesc->stoi.ls = 0;
    		        outdesc->stoi.fs = 0;
    		        outdesc->stoi.own = 1;
			BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (BULKOUTdescNumber) ;
			outdesc = (outindesc_r *) BULKOUTdescStartAddr + BulkDescOUTIndex;
		}
		BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (BULKOUTdescNumber) ;
		outdesc = (outindesc_r *) BULKOUTdescStartAddr + BulkDescOUTIndex;
	}
}
//tomadd 2011.05.02
//recycle bulk out descriptor
void recycle_outdesc(outindesc_r *outdesc)
{
	INT8U i = 0;
	INT8U err;
	INT8U *addr;

	if (!outdesc->stoi.own)
	{
	  /*
		while (i++ < BULKOUTdescNumber)
		{              
			//recycle out descriptor
    		        outdesc->stoi.length = OUTBUFLEN;
    		        outdesc->stoi.ls = 0;
    		        outdesc->stoi.fs = 0;
    		        outdesc->stoi.own = 1;
			BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (BULKOUTdescNumber) ;
			outdesc = (outindesc_r *) BULKOUTdescStartAddr + BulkDescOUTIndex;
		}
		*/

	  	//recycle out descriptor
    	outdesc->stoi.length = OUTBUFLEN;
    	outdesc->stoi.ls = 0;
    	outdesc->stoi.fs = 0;
    	outdesc->stoi.own = 1;
		BulkDescOUTIndex= ( BulkDescOUTIndex+ 1 )% (BULKOUTdescNumber) ;
		outdesc = (outindesc_r *) BULKOUTdescStartAddr + BulkDescOUTIndex;
	}
}
	   
//***************************************************//
//  FW ACCESS IB Channel IO or Configuration Space   //
//  Build Date: 2011.04.28                           //
//  Editor: Tom                                      //
//***************************************************//

// tomadd 2011.04.28
// OOB reads IB IO
int Rd_IBIO_test(INT8U phyaddr)
{    
 	INT32U  temp_count;    
	INT8U  offset;    
	INT32U  Rd_Data;
	offset = phyaddr;   

	REG32(OOBACTADDR) = 0x0000F000|offset; 
	temp_count=0;
	//Rd_Data=REG32(OOBACTDATA);
	//while((REG32(OOBACTADDR)>>31)==0 && temp_count<1000)
	do{
		Rd_Data=REG32(OOBACTDATA);
		temp_count++;

	}while((REG32(OOBACTADDR)>>31)==0 && temp_count<1000);
	//}while((REG32(OOBACTADDR)>>31)==0);
	
	return Rd_Data;
}
//tomadd 2011.04.28
//OOB writes IB IO
void Wt_IBIO_test(INT8U phyaddr,INT32U Wt_Data)
{   
	
	INT32U  temp_count;    
	INT8U offset;
	offset = phyaddr;
	REG32(OOBACTDATA)=Wt_Data;
	REG32(OOBACTADDR)=0x8000F000|offset;
	temp_count=0;
	//while((REG32(OOBACTADDR)>>31)==1 && temp_count<1000)
	do
	{
		temp_count++;
	}while((REG32(OOBACTADDR)>>31)==1 && temp_count<1000);
	//}while((REG32(OOBACTADDR)>>31)==1);

}
// tomadd 2011.04.28
// OOB reads IB PCIE configuration space
int Rd_PCIECS(INT8U phyaddr)
{    
 	INT8U  temp_count;    
	INT8U  offset;    
	INT32U  Rd_Data;
	offset = phyaddr;   

	REG32(DBIACTADDR) = 0x0000F000|offset; 
	temp_count=0;
	while((REG32(DBIACTADDR)>>31)==0&&temp_count<1000)
	{
		Rd_Data=REG32(DBIACTDATA);
		temp_count++;

	};

	return Rd_Data;
}
//tomadd 2011.04.28
//OOB writes IB PCIE configuration space
void Wt_PCIECS_test(INT8U phyaddr,INT32U Wt_Data)
{   
	
	INT8U  temp_count;    
	INT8U offset;
	offset = phyaddr;
	REG32(DBIACTDATA)=Wt_Data;
	REG32(DBIACTADDR)=0x8000F000|offset;
	temp_count=0;
	while((REG32(DBIACTADDR)>>31)==1&&temp_count<1000)
	{
		temp_count++;
	};

}


//*************************************************************//
//  Introduce RTKEHCI_INT_HANDLER(),                           //
//  1. OUT transaction done                                    //
//       [1] Parse USB2.0 Chapter 9 packet                     //
//            (1) Call IN transaction response data            //
//       [2] Parse CBW/UFI command                             //
//            (1) Call IN transaction response data or CSW     //
//            (2) Response CSW                                 //
//  2. OUT tranfer done                                        //
//  Build Date: 2010.09.10                                     //
//  Editor: Tom                                                //
//  Modified Date: 2010.11.05                                  //
//*************************************************************//

volatile INT32U test[5];

extern INT32U Timer_flag;
int rtkehci_int_handler_test()
{
	volatile INT32U OUTSTS;
	volatile INT8U err = 0;
	volatile INT32U packetlen;
	volatile INT32U D_tmp;

	//bulk out descriptor address
	outindesc_r *bulkoutdesc;
	bulkoutdesc = (outindesc_r *) (BULKOUTdescStartAddr)+BulkDescOUTIndex;
	//control out descriptor address
	controloutdesc_r *ctloutdesc;
	ctloutdesc = (controloutdesc_r *)(CTLOUTdescStartAddr_EHCI)+CTRLDescOUTIndex;
	//bulk in descriptor address
	outindesc_r *bulkindesc;
	bulkindesc = (outindesc_r *) (BULKINdescStartAddr);
	//control in descriptor address
	outindesc_r *ctlindesc;
	ctlindesc = (outindesc_r *)(CTLINdescStartAddr_EHCI);
	//interrupt in 1 des address
	outindesc_r *intin1desc ; 
	intin1desc= (outindesc_r *)(INTIN1descStartAddr_EHCI);
	//interrupt in 2 des address 
  	outindesc_r *intin2desc ;
	intin2desc = (outindesc_r *)(INTIN2descStartAddr_EHCI);
  
	//disable IMR
	REG32(EHCI_IMR) = 0x00000000;
	OUTSTS = REG32(EHCI_ISR);
	//clear ISR
	REG32(EHCI_ISR)=OUTSTS;
	//tomadd 2011.04.28
	//host controller reset done interrupt
	//this reset will  reset the portsc's port connect status ,so set this bit with handler

	if(OUTSTS & 0x00000080)
	{

//OK
/*
		D_tmp=Rd_IBIO(PORTSC1);
		Wt_IBIO(PORTSC1,D_tmp|0x00000001);
		D_tmp=Rd_IBIO(PORTSC2);
		Wt_IBIO(PORTSC2,D_tmp|0x00000001);
*/

#if 1 	
	
		//tomadd 2011.08.23
		//reset setup phase address counter
		setup_count=0;

		EHCI_RST_test();
		rtkehci_init_test();

		//tomadd 2011.09.30
		//this read-modify-write may cause error when 
		//IB is accessing this IB register ,so anote!!
		//debug for remove virtual device issue
		//disable ASE &PSE
		//D_tmp=Rd_IBIO(USBCMD);
		//Wt_IBIO(USBCMD,D_tmp& 0xFFFFFFCF);

		//port disconnection
		D_tmp=Rd_IBIO_test(PORTSC1);
		Wt_IBIO_test(PORTSC1,D_tmp & 0xFFFFFFFE);
		D_tmp=Rd_IBIO_test(PORTSC2);
		Wt_IBIO_test(PORTSC2,D_tmp & 0xFFFFFFFE);
		#ifndef COMPOSITE_HID
		D_tmp=Rd_IBIO_test(PORTSC3);
		Wt_IBIO_test(PORTSC3,D_tmp & 0xFFFFFFFE);
		#endif

#endif
		D_tmp=Rd_IBIO_test(PORTSC1);
		Wt_IBIO_test(PORTSC1,D_tmp|0x00000001);
		D_tmp=Rd_IBIO_test(PORTSC2);
		Wt_IBIO_test(PORTSC2,D_tmp|0x00000001);
		#ifndef COMPOSITE_HID
		D_tmp=Rd_IBIO_test(PORTSC3);
		Wt_IBIO_test(PORTSC3,D_tmp|0x00000001);
		#endif

	}

	//tomadd 2011.04.28
	//host port reset interrupt
	if(OUTSTS & 0x00000200)
	{
		
		D_tmp=REG32(EHCICONFIG);
		D_tmp=Rd_IBIO_test(PORTSC1);
		test[0]=D_tmp;
		//Wt_IBIO(PORTSC1,D_tmp&0xfffffEff);
		
	}
  
	//tomadd 2011.04.28
	//host port reset done interrupt
	if(OUTSTS & 0x00000100)
	{

		
		D_tmp=REG32(EHCICONFIG);
		D_tmp=Rd_IBIO_test(PORTSC1);
		test[1]=D_tmp;
		//Wt_IBIO(PORTSC1,D_tmp|0x00000004);
		//D_tmp=Rd_IBIO(PORTSC1);
		
	}

		//tomadd 2011.04.28
	//host port reset interrupt
	if(OUTSTS & 0x00000800)
	{
		D_tmp=REG32(EHCICONFIG);
		D_tmp=Rd_IBIO_test(PORTSC2);
		//Wt_IBIO(PORTSC2,D_tmp&0xfffffEff);
		//D_tmp=Rd_IBIO(PORTSC2);
	}
  
	//tomadd 2011.04.28
	//host port reset done interrupt
	if(OUTSTS & 0x00000400)
	{
		D_tmp=REG32(EHCICONFIG);
		D_tmp=Rd_IBIO_test(PORTSC2);
		//Wt_IBIO(PORTSC2,D_tmp|0x00000004);
	}
  
	//tom modified 2010.12.14
	//Bit1 =1,  control out  ,it presents the control transfers' data stage 
	if((OUTSTS & 0x00000002)&& (is_out==1))
	{
		is_out=0;
		rtkehci_ep0_start_transfer_test(0,PA2VA(ctloutdesc->outin_buf_lowaddr),is_IN);
		
		if(!(ctloutdesc->stoi.own))
		{
			#ifdef COMPOSITE_HID
				#ifdef HID_TIMEOUT
				//wait for timeout and 
				//check 0xbaf6000c bit 17:16 's status 
				while(1);
				#endif
			 	rtkehci_INTIN_start_transfer_test( HID_ep,8, PA2VA(ctloutdesc->outin_buf_lowaddr),is_IN);
			#else 
				rtkehci_INTIN_start_transfer_test( ctloutdesc->stoi.device_addr,8, PA2VA(ctloutdesc->outin_buf_lowaddr),is_IN);
			#endif

			recycle_ctloutdesc_test(ctloutdesc);
		}

		//because data phase  will also put in control out descriptor ,modify setup_pkt address in setup_phase();
		setup_count++;
		setup_pkt=(unsigned int)setup_pkt + 0x40;
		if(setup_count==4)
		{
			 setup_pkt=(unsigned int)setup_pkt -setup_count*0x40;
			 setup_count=0;
		}	

	}

	//tomadd 2011.05.02
	//Bit0 =1, setup token
	//this interrupt is not same as control out,it presents the control transfers' setup stage
	if((OUTSTS & 0x00000001) )
	{
		if(!(ctloutdesc->stoi.own))
		{
			setup_phase_test(EHCI_USBD , ctloutdesc->stoi.device_addr );//parsing usb2.0 chap 9 pattern...	
			//setup_phase(EHCI_USBD , ctloutdesc->stoi.port_num );//parsing usb2.0 chap 9 pattern..
			//recycle outdescriptorss
			recycle_ctloutdesc_test(ctloutdesc);	
			
		}
	
	}



	//tom modified 2010.12.14
	//Bit4 =1, bulk out transaction 
	if(OUTSTS & 0x00000010)
	{
        D_tmp=Rd_IBIO(USBCMD);
        Wt_IBIO(USBCMD,(D_tmp&0x00ffff)|0x80000);
		while (!(bulkoutdesc->stoi.own))
		{
			packetlen = OUTBUFLEN-bulkoutdesc->stoi.length; // ???
			//CBW 
			if(packetlen == CBWLength){
				CBW = PA2VA(bulkoutdesc->outin_buf_lowaddr);  //modify neil 
//				ata_cbw(EHCI_USBD);
			}

			//DATA out & copy the DATA to bulk in
			if(packetlen!=CBWLength)
			{
				#ifdef bulk_in_test
				bulk_out_in_test(packetlen);
				#else
				bulk_out_ufi_write(packetlen);
				#endif
			}
			recycle_outdesc(bulkoutdesc);
		}    
	}

	//tom modified 2010.12.15
	//Bit2: control out descriptor unavailable
	//Bit3: bulk out descriptor unavailable
	//Bit5: bulk in descriptor unavailable
	//Bit6: control in descriptor unavailable
	if(OUTSTS & 0x00000004)
	{

	}
	if(OUTSTS & 0x00000008)
	{
		recycle_outdesc(bulkoutdesc);
	}
	if(OUTSTS & 0x00000020)
	{

	}
	if(OUTSTS & 0x00000040)
	{

	}

	//	 Keyboard IN Request Received When TxFIFO Empty  
	if(OUTSTS & 0x00010000)
	{

	}
	//	Mouse IN Request Received When TxFIFO Empty Mask 
	if(OUTSTS & 0x00020000)
	{

	}
	
	//tom modified 2011.09.18
	//Bit31 IB: IN transfer timeout 

	//enable IMR	
	REG32(EHCI_IMR) = 0xE00300FF;//0x600003FF;
}

//********************************************************//
//  Introduce RTKEHCI_INIT(),                             //
//  1. Tune buffer size : 128bytes ~ 20kbytes             //
//  2. Prepare out and in descriptor                      //
//  3. Enable Out In transaction state mechine            //
//  Build Date: 2010.09.10                                //
//  Editor: Tom                                           //

//********************************************************//
int rtkehci_init_test()
{
  INT8U i;
  INT32U *ptr;
  INT8U *ctloutaddr,*ctlinaddr,*outaddr,*inaddr;
  INT32U ctladdr,cbwaddr;
  INT32U dval;
  
  CTRLDescOUTIndex=0;
  CTRLDescINIndex=0;
  BulkDescOUTIndex=0;
  BulkDescINIndex=0;
  INT1DescINIndex=0;
  INT2DescINIndex=0;
  // tom modified 2010.12.14
  // assign control out/in and bulk out/in descriptor pointer
  controloutdesc_r *ctloutdesc = (controloutdesc_r *)(CTLOUTdescStartAddr_EHCI); // control out new descriptor 
  outindesc_r *ctlindesc = (outindesc_r *)(CTLINdescStartAddr_EHCI); //?? 
  outindesc_r *bulkoutdesc = (outindesc_r *)(BULKOUTdescStartAddr);
  outindesc_r *bulkindesc = (outindesc_r *)(BULKINdescStartAddr);
  outindesc_r *intin1desc = (outindesc_r *)(INTIN1descStartAddr_EHCI);
  outindesc_r *intin2desc = (outindesc_r *)(INTIN2descStartAddr_EHCI);

  //tomadd 2011.08.07
  // FW writes IB EHCI port connection bit=0
  //dval=Rd_IBIO(PORTSC);
	//Wt_IBIO(PORTSC,dval& 0xFFFFFFFE);
  //tomadd 2011.07.22
  //Reset EHCI OOB engine
  //EHCI_RST();
  
  //tomadd 2011.05.07
  // FW writes IB EHCI port connection bit=1
  //dval=Rd_IBIO(PORTSC);
	//Wt_IBIO(PORTSC,dval|0x00000001);
	//tomadd 2011.07.06
	//for FPGA test(patch hardware bug)
 #ifndef COMPOSITE_HID
  Wt_IBIO_test(HCSPARAMS,0x00000003);  //there are 3 downstream port  ,for 3 port modify
 #else
  Wt_IBIO_test(HCSPARAMS,0x00000002); 
 #endif

 dval=Rd_IBIO_test(HCSPARAMS);

 
  //Wt_IBIO(PORTSC,D_tmp|0x00000001);
  REG32(EHCICONFIG)=0x00000000;
  /* step1. tune out and in buffer size */
  REG32(EHCICONFIG) = 0x0106001B;
  
  /* step2. */
  //(1)prepare control/bulk out/bulk in descriptors
  // tomadd 2010.11.18
  //  [1]  for control out descriptor, CTLOUTdescNumber=4
  for(i=0; i< CTLOUTdescNumber; i++)
  {
  	ctloutdesc->stoi.length = 0x40; // buffer size = 64bytes
  	// tomadd 2010.11.18
	  // reason for control setup phase
	  //this address maybe need modify to CTLOUT_BUFSTARTADDR@ neil 
	  //setup_pkt = (INT32U *)USB_ADDR;
	  setup_pkt = (INT32U*)CTLOUT_BUFSTARTADDR;
	  ctloutaddr=(unsigned int)setup_pkt & ~Uncache_Mask;
	  
	  // tom modified 2010.11.18
  	ctloutdesc->outin_buf_lowaddr = (INT8U *) VA2PA(ctloutaddr)+ i* (ctloutdesc->stoi.length);

	if(i==CTLOUTdescNumber-1)
		  ctloutdesc ->stoi.eor = 1;//last descriptor
	  else
		  ctloutdesc ->stoi.eor = 0;

	ctloutdesc->stoi.device_addr=0;
	ctloutdesc->stoi.own = 1;
	
	ctloutdesc++;
  }
  // tomadd 2010.12.14
  //  [2]  for control in descriptor, CTLINdescNumber=1
  for(i=0; i< CTLINdescNumber; i++)
  {
  	ctlindesc->stoi.length = 0x40; // buffer size = 64bytes
  	//ctlindesc->outin_buf_lowaddr = (INT8U *) VA2PA(CTLIN_BUFSTARTADDR) + i* (ctlindesc->stoi.length);
        //prevent hardware overwrite each other
  	if(i==CTLINdescNumber-1)
		 ctlindesc ->stoi.eor = 1;//last descriptor
	  else
		 ctlindesc ->stoi.eor = 0;
    //ctlindesc->stoi.own = 1;
	  ctlindesc++;
  }
  //  [3] for bulk out descriptors, OUTdescNumber=4
  for(i=0; i< BULKOUTdescNumber; i++)
  {
		bulkoutdesc->stoi.length = OUTBUFLEN; // buffer size = 2kbytes
		//CBW  = (INT32U *)(DATA_ADDR);
		  //this address maybe need modify to OUT_BUFSTARTADDR @ neil
		//cbwaddr=(unsigned int)CBW & ~Uncache_Mask;
		cbwaddr=(unsigned int)OUT_BUFSTARTADDR & ~Uncache_Mask;
		//cbwaddr=(INT8U *)(BulkOUTADDR);
		#ifdef bulk_in_test
		bulkoutdesc->outin_buf_lowaddr = (INT8U *) VA2PA(cbwaddr)+i*bulkoutdesc->stoi.length;
		#else
		bulkoutdesc->outin_buf_lowaddr = (INT8U *) VA2PA(cbwaddr);
		//outdesc->outin_buf_lowaddr = (INT8U *) VA2PA(OUT_BUFSTARTADDR)+i*outdesc->stoi.length;
		#endif
		if(i==BULKOUTdescNumber-1)
			bulkoutdesc ->stoi.eor = 1;//last descriptor
		else
			bulkoutdesc ->stoi.eor = 0;
		  
		bulkoutdesc->stoi.own = 1;
		bulkoutdesc++;
  } 
  //  [4] for bulk IN descriptors, INdescNumber=1
  for (i = 0; i < BULKINdescNumber; i++)
  {
        bulkindesc->stoi.length = OUTBUFLEN; // buffer size = 4kbytes
        //indesc->outin_buf_lowaddr = (INT8U *) VA2PA(IN_BUFSTARTADDR) + i* (indesc->stoi.length);
        bulkindesc->outin_buf_lowaddr = (INT8U *) VA2PA(BulkINADDR) + i* (bulkindesc->stoi.length);
        //prevent hardware overwrite each other
        if (i == BULKINdescNumber-1)
            bulkindesc->stoi.eor  = 1;//last descriptor
        else
            bulkindesc->stoi.eor  = 0;
	      //indesc->stoi.own = 1;
        bulkindesc++;
  } 
  // [5] for interrupt IN 1 descriptor ,
   for (i = 0; i < INTIN1desNumber; i++)
  {
        intin1desc->stoi.length = 0x08; // buffer size = 8
        //indesc->outin_buf_lowaddr = (INT8U *) VA2PA(IN_BUFSTARTADDR) + i* (indesc->stoi.length);
        intin1desc->outin_buf_lowaddr = (INT8U *) VA2PA(INTIN1ADDR) + i* (intin1desc->stoi.length);
        //prevent hardware overwrite each other
        if (i == INTIN1desNumber-1)
            intin1desc->stoi.eor  = 1;//last descriptor
        else
            intin1desc->stoi.eor  = 0;
		intin1desc->stoi.own  = 0;
        intin1desc++;
  } 
  // [6] for interrupt IN 2 descriptor ,
   for (i = 0; i < INTIN2desNumber; i++)
  {
        intin2desc->stoi.length = 0x8; // buffer size = 8
        //indesc->outin_buf_lowaddr = (INT8U *) VA2PA(IN_BUFSTARTADDR) + i* (indesc->stoi.length);
        intin2desc->outin_buf_lowaddr = (INT8U *) VA2PA(INTIN2ADDR) + i* (intin2desc->stoi.length);
        //prevent hardware overwrite each other
        if (i == INTIN2desNumber-1)
            intin2desc->stoi.eor  = 1;//last descriptor
        else
            intin2desc->stoi.eor  = 0;
	      //indesc->stoi.own = 1;
		intin2desc->stoi.own  = 0;
		intin2desc++;
  } 
   
  //(2)assign out descriptor address to register
  REG32(OUTDesc_Addr) = VA2PA(BULKOUTdescStartAddr);
  //(3)assign in descriptor address to register
  REG32(INDesc_Addr) = VA2PA(BULKINdescStartAddr);
  //(4)assign control out descriptor address to register
  REG32(CTLOUTDesc_Addr) = VA2PA(CTLOUTdescStartAddr_EHCI);
  //(5)assign control in descriptor address to register
  REG32(CTLINDesc_Addr) = VA2PA(CTLINdescStartAddr_EHCI);
  //(6)assign interrupt in 1 descriptor address to register
  REG32(INTINDESCADDR1) = VA2PA(INTIN1descStartAddr_EHCI);
  //(7)assign interrupt in 2 in descriptor address to register
  REG32(INTINDESCADDR2) = VA2PA(INTIN2descStartAddr_EHCI);

  // modify neil 
  ptr = (INT32U *)(CSW_ADDR);
  CSW = ptr;
	
  
  /* step3. */
  //enable OUT interrupt mask register
  //tom modified 2011.09.18
  //Bit31 IB: IN transfer timeout 
  //??
  //REG32(EHCI_IMR) = 0xE00003FF;//0x600003FF;
  //for hid time out interrupt 
  REG32(EHCI_IMR) = 0xE00300FF;//0x600003FF;
  
  /* step4. */
  // tom modified 2010.12.24
  // enable OUT transaction state mechine
  REG32(EHCICONFIG) = REG32(EHCICONFIG)|0x00010000;

  //initial keyboard & mouse endpoint num for composite HID
  #ifdef COMPOSITE_HID
  	REG32(ENDPOINT_REG)= (0x02<<4)| 0x01 ; // keyboad ep num is 0x01, mouse ep mun is 0x02
  #endif
  
  //tomadd 2011.11.14
  //EP phase II test
  //REG32(DBIACTDATA)|= 0x20000000;

  //clear  globla vaule with reboot
  setaddress_counter=0;
  PORT1_address=0;
  PORT2_address=0;	
  PORT3_address=0;
  
  /* step5. */
  // tomadd 2010.11.11
  // register IRQ with MIPS 

  //share with function 0 handler 
  //rlx_irq_set_handler(BSP_EHCI_IRQ, rtkehci_int_handler);
  //rlx_irq_set_mask(BSP_EHCI_IRQ, OS_FALSE);
}

// tomadd 2010.11.15
// ehci engine reset function
int EHCI_RST_test()
{
	INT32U ehcirst,count;
	
	count=0;
	ehcirst = 0x00800000;
	REG32(EHCICONFIG) = ehcirst;
	do 
	{
		REG32(EHCICONFIG)=ehcirst;
		ehcirst = REG32(EHCICONFIG);

		if (++count > 1000)
		{
			return;
		}
	} 
	while ( (ehcirst & 0x00800000) );	/* ehci firmware reset */
}

void bsp_ehci_init()
{
	INT32U 	j;
	
	//j=Rd_IBIO(HCSPARAMS);


	
	//issue
	//this rst will deassert HCSPARAMS to 0,
	//if bios check this register before set command of resert to EHCI,
	//bios will not do the emulatation
	 EHCI_RST_test();
	//j=Rd_IBIO(HCSPARAMS);

	 #ifndef COMPOSITE_HID
  		Wt_IBIO_test(HCSPARAMS,0x00000003);  //there are 3 downstream port  ,for 3 port modify
 	 #else
  		Wt_IBIO_test(HCSPARAMS,0x00000002); 
 	 #endif
				
	//port disconnection, for 3 port modify
	j=Rd_IBIO_test(PORTSC1);
	Wt_IBIO_test(PORTSC1,j& 0xFFFFFFFE);
	j=Rd_IBIO_test(PORTSC2);
	Wt_IBIO_test(PORTSC2,j& 0xFFFFFFFE);
	#ifndef COMPOSITE_HID
	j=Rd_IBIO_test(PORTSC3);
	Wt_IBIO_test(PORTSC3,j& 0xFFFFFFFE);
	#endif
	bsp_wait(1000);
	//port connection ,for 3 port modify 
	j=Rd_IBIO_test(PORTSC1);
	Wt_IBIO_test(PORTSC1,j|0x00001803);
	j=Rd_IBIO_test(PORTSC2);
	Wt_IBIO_test(PORTSC2,j|0x00001803);
	#ifndef COMPOSITE_HID
	j=Rd_IBIO_test(PORTSC3);
	Wt_IBIO_test(PORTSC3,j|0x00000001);
	#endif
		//j=Rd_IBIO(PORTSC1);

	CTLOUTdescStartAddr_EHCI = malloc(sizeof(controloutdesc_r)*CTLOUTdescNumber);
  	if(!CTLOUTdescStartAddr_EHCI){
		for(;;);
  	}

	CTLOUT_BUFSTARTADDR = malloc(0x40*CTLOUTdescNumber);
  	if(!CTLOUT_BUFSTARTADDR) for(;;);

	CTLINdescStartAddr_EHCI = malloc(sizeof(outindesc_r)*CTLINdescNumber);
  	if(!CTLINdescStartAddr_EHCI){
		for(;;);
  	}

	BULKOUTdescStartAddr = malloc(sizeof(outindesc_r)*BULKOUTdescNumber);
   	if(!BULKOUTdescStartAddr){
		for(;;);
  	}

	OUT_BUFSTARTADDR = malloc(OUTBUFLEN*BULKOUTdescNumber);
  	if(!OUT_BUFSTARTADDR) for(;;);

	BULKINdescStartAddr = malloc(sizeof(outindesc_r)*BULKINdescNumber);
   	if(!BULKINdescStartAddr){
		for(;;);
  	}

	 BulkINADDR = malloc(OUTBUFLEN*BULKINdescNumber);
  	if(!BulkINADDR) for(;;);

	
  	INTIN1descStartAddr_EHCI = malloc(sizeof(outindesc_r)*INTIN1desNumber);
  	if(!INTIN1descStartAddr_EHCI){
		for(;;);
  	}

	INTIN1ADDR = malloc(0x08*INTIN1desNumber);
  	if(!INTIN1ADDR) for(;;);

	INTIN2descStartAddr_EHCI = malloc(sizeof(outindesc_r)*INTIN2desNumber);
  	if(!INTIN2descStartAddr_EHCI){
  		for(;;);
  	}

	INTIN2ADDR = malloc(0x08*INTIN2desNumber);
  	if(!INTIN2ADDR) for(;;);
  
	DESC_ADDR = malloc(0x100);
  	if(!DESC_ADDR){
  		for(;;);
  	}

	CSW_ADDR = malloc(0x100);
  	if(!CSW_ADDR) for(;;);
	
	rtkehci_init_test();
}

void EHCI_Test_Task(void)
{
	
	while(1)
	{
		 OSTimeDly(OS_TICKS_PER_SEC/10);		 
	}
	
}


