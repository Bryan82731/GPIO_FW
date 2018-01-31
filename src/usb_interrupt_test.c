#include <rlx/rlx_types.h>
#include <rlx/rlx_cpu_regs.h>
#include "bsp_cfg.h"
#include "bsp_fp_revA.h"
#include "rlx/rlx_types.h"

#include "bsp.h"
#include "telnet.h"
#include "ehci_test.h"
#include "usb_test.h"

#ifdef	__HIGH_SPEED__
#define ep_mps 	512	//high speed
#define ep0_mps 64	//high speed
#else
#define ep_mps 	64	//full speed
#define ep0_mps 64	//full speed
#endif
// WRITE TO DMEM ADDRESS
INT8U * USB_ADDR;

// USB Bulk Out Data Address
INT8U * DMEM_ADDR;
//#define DMEM_ADDR			(DMEMStartAddr + 0x12200)//length=0x200 


//test
extern unsigned char lc;

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



unsigned char LUN = 0;
//tom modified 2011.08.08	
struct usb_ctrlrequest_test *setup_pkt;
//tom add 2011.08.08	
struct usb_ctrlrequest_test *org_setup;
struct command_block_wrapper *CBW;
struct command_status_wrapper *CSW;

//tomadd
unsigned int ep_count = 0;	

unsigned int inLen ;
unsigned int *outaddr;
unsigned int outflag = 0 ;
//tomadd 2011.08.23
unsigned int setup_count=0;//j = 0 ;

#ifdef EHCISTALL
//tomadd 2011.09.18
unsigned int ep_stall;
#endif

extern void Wt_IBIO(INT8U phyaddr,INT32U Wt_Data);

extern void 		bsp_wait(INT32U usec);

void setup_phase_test(USBdeviceType_e USBdeviceType , INT8U DeviceID)
{
	volatile unsigned char bRequestType, bRequest;
	volatile unsigned short len = 0, wValue, wIndex, wLength, ep;
	INT8U *addr;
	INT32U size;
	int i;
	INT32U D_tmp;
	INT8U  B_tmp = 0;
	unsigned int ctl_count = 0;
	struct usb_device_descriptor *device_desc;
	INT8U	*descriptors_temp;


	//copy IN Transaction data(configuration descriptor) to DMEM
	
	
	addr = (INT8U *)(DESC_ADDR + 0x50);
	//copy String data to DMEM
	for(i=0;i<4;i++)
	{
		*addr	= str0[i];
		addr++;
	}
	addr = (INT8U *)(DESC_ADDR + 0x60);
	//copy String data to DMEM
	for(i=0;i<16;i++)
	{
		*addr	= str1[i];
		addr++;
	}
	addr = (INT8U *)(DESC_ADDR + 0x70);
	//copy String data to DMEM
	for(i=0;i<22;i++)
	{
		*addr	= str2[i];
		addr++;
	}
	//copy String data to DMEM
	addr = (INT8U *)(DESC_ADDR + 0x90);
	for(i=0;i<28;i++)
	{
		*addr	= str3[i];
		addr++;
	}

	//Read data from setup 8 bytes command at DMEM
	//setup_pkt is address of current descriptor's data buffer address
	addr = (INT8U *)(DESC_ADDR + 0xD0);
	*addr = LUN;
	bRequestType = setup_pkt->bRequestType;
	bRequest = setup_pkt->bRequest;
	wValue = cpu_to_le16( setup_pkt->wValue );
	wIndex = cpu_to_le16( setup_pkt->wIndex );
	wLength = cpu_to_le16( setup_pkt->wLength );

    //Sophia
    //assign a variable to device address
	
	if(USBdeviceType == EHCI_USBD)
	{
		setup_count++;
		setup_pkt=(unsigned int)setup_pkt + 0x40;
		if(setup_count==4)
		{
			 setup_pkt=(unsigned int)setup_pkt -setup_count*0x40;
			 setup_count=0;
		}	

	  //Sophia
	  //Shift device address (there are 4 Control OUT descriptors)
	}
	else
	{
		ctl_count  = (ctl_count + 1) % 40;
		setup_pkt = (unsigned int)(setup_pkt) + 8;
		if(ctl_count==0)
	  	setup_pkt = (unsigned int)(org_setup);
		ep0_out_start();
	}

	if( bRequestType & USB_DIR_IN )
	{	
		ep0state[USBdeviceType] = EP0_IN_DATA_PHASE;
	}
	else //OUT
	{	
		ep0state[USBdeviceType] = EP0_OUT_DATA_PHASE;
	}
	len = wLength;  //maybe the first read configuration descriptor will return only just the configuration descritor
	
	#ifdef COMPOSITE_HID
	if(bRequestType!=0x42) {	//if not vendor  
	#endif 
		switch( bRequest ) {
/*----------------------------*/
/*               GET DESCRIPTOR               */
/*----------------------------*/
		case USB_REQ_GET_DESCRIPTOR:
			switch( wValue>>8 ) 
			{
				case USB_DT_DEVICE:
					//prepare device descriptor according device address
					//copy IN Transaction data(device device descriptor) to DMEM

					if(DeviceID == PORT1_address)
						device_desc = &MassStorage_device_desc;
					#ifndef COMPOSITE_HID
						else if (DeviceID == PORT2_address)
							device_desc = &KeyBoard_device_desc;
						else if (DeviceID == PORT3_address )
							device_desc = &Mouse_device_desc;
					#else
						else {
							device_desc = &composite_hid_desc;
						}
					#endif
					/*
					if(port_num == 0){
						device_desc = &MassStorage_device_desc;
					}
					#ifndef COMPOSITE_HID
						else if (port_num == 1)
							device_desc = &KeyBoard_device_desc;
						else if (port_num == 2 )
							device_desc = &Mouse_device_desc;
					#else 
						else {
							device_desc = &composite_hid_desc;
						}
					#endif
					*/
					
					addr = (INT8U *)DESC_ADDR;
					*addr = device_desc->bLength;
					size = device_desc->bLength;
					
					*(addr+1) = device_desc->bDescriptorType;
					*(addr+2) 	= device_desc->bcdUSB & 0x00ff;
					*(addr+3) 	= device_desc->bcdUSB >>8 ;
					*(addr+4) 	= device_desc->bDeviceClass ;
					*(addr+5) 	= device_desc->bDeviceSubClass;
					*(addr+6) 	= device_desc->bDeviceProtocol;	
					*(addr+7)	= device_desc->bMaxPacketSize0;
					*(addr+8) 	= device_desc->idVendor & 0x00ff ;
					*(addr+9) 	= device_desc->idVendor >> 8 ;
					*(addr+10) 	= device_desc->idProduct & 0x00ff;
					*(addr+11) 	= device_desc->idProduct >> 8;
					*(addr+12) 	= device_desc->bcdDevice &0x00ff;
					*(addr+13) 	= device_desc->bcdDevice >> 8;
					*(addr+14) 	= device_desc->iManufacturer ;
					*(addr+15) 	= device_desc->iProduct;
					*(addr+16) 	= device_desc->iSerialNumber;
					*(addr+17) 	= device_desc->bNumConfigurations;
					
					
					if( wLength > sizeof(struct usb_device_descriptor) )
					{
						len = sizeof(struct usb_device_descriptor);
					}

					bufaddr = (INT8U *)DESC_ADDR;
					totalLen = len;	

					if(USBdeviceType == EHCI_USBD)
					{
						rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
					}
					else
					{
						dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
					}
					break;
					
				//  port return configuration descriptor  
				case USB_DT_CONFIG:
					//select the device 
					if(DeviceID == PORT1_address) {
						descriptors_temp = MassStorage_descriptors ;
						if( wLength > sizeof(MassStorage_descriptors) )
						{
							len = sizeof(MassStorage_descriptors);
						}
					}
					#ifndef COMPOSITE_HID
						else if (DeviceID == PORT2_address){
							descriptors_temp = KeyBoard_descriptors ;
							if( wLength > sizeof(KeyBoard_descriptors) ){
								len = sizeof(KeyBoard_descriptors);
							}
						}
						else if (DeviceID == PORT3_address ){
							descriptors_temp = Mouse_descriptors;
							if( wLength > sizeof(Mouse_descriptors) ){
								len = sizeof(Mouse_descriptors);
							}
						}
					#else
						else{
							descriptors_temp = composite_hid_descriptors ;
							if( wLength > sizeof(composite_hid_descriptors) )
							{
								len = sizeof(composite_hid_descriptors);
							}
						}
					#endif 
					//copy Descriptors data to DMEM
					addr = (INT8U *)(DESC_ADDR + 0x20);
					for(i=0;i<len;i++)
					{
						*addr = *(descriptors_temp + i);
						addr++;
					}
					//if( wLength > sizeof(descriptors) )
					//{
					//	len = sizeof(descriptors);
					//}

					bufaddr = (INT8U *)(DESC_ADDR + 0x20);
					totalLen = len;			

					if(USBdeviceType == EHCI_USBD)
					{
						rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
					}
					else
					{
						dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
					}
					break;
					
				case USB_DT_STRING:
					if( (wValue & 0xFF) == 0 ) 
					{
						if( wLength > sizeof(str0) )
						{
							len = sizeof(str0);
						}

						bufaddr = (INT8U *)(DESC_ADDR + 0x50);
						totalLen = len;

						if(USBdeviceType == EHCI_USBD)
						{
							rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
						}
						else
						{
							dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
						}
					}
					else if( (wValue & 0xFF) == 1 ) 
					{
						if( wLength > sizeof(str1) )
						{
							len = sizeof(str1);
						}

						bufaddr = (INT8U *)(DESC_ADDR + 0x60);
						totalLen = len;

						if(USBdeviceType == EHCI_USBD)
							rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
						else
							dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
					}
					else if( (wValue & 0xFF) == 2 ) 
					{
						if( wLength > sizeof(str2) )
						{
							len = sizeof(str2);
						}

						bufaddr = (INT8U *)(DESC_ADDR + 0x70);
						totalLen = len;

						if(USBdeviceType == EHCI_USBD)
						{
							rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
						}
						else
						{
							dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);	
						}
					}
					else if( (wValue & 0xFF) == 3 ) 
					{
						if( wLength > sizeof(str3) )
						{
							len = 28;//sizeof(str3);
						}

						bufaddr = (INT8U *)(DESC_ADDR + 0x90);
						totalLen = len;

						if(USBdeviceType == EHCI_USBD)
						{
							rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
						}
						else
						{
							dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);	
						}
					}
					break;
				default:
					break;
			}
			break;
/*----------------------------*/
/*               SET ADDRESS                    */
/*----------------------------*/
		case USB_REQ_SET_ADDRESS:
			bufaddr = setup_pkt;
			totalLen = 0;
			
			if(USBdeviceType == EHCI_USBD)
			{
				rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);

				if(((wValue & 0x007f)!=(REG32(DEVICE_ADDRESS)&0x00ff))&& \
					((wValue & 0x007f)!=((REG32(DEVICE_ADDRESS)>>8)&0x00ff))&& \
					((wValue & 0x007f)!=((REG32(DEVICE_ADDRESS)>>16)&0x00ff))){
					REG32(DEVICE_ADDRESS) |= (wValue & 0x007f)<<(setaddress_counter*8);

					//when use composite hid ,must also port 3's address 
					#ifdef COMPOSITE_HID
						if(setaddress_counter==1){
							REG32(DEVICE_ADDRESS) |= (wValue & 0x007f)<<((setaddress_counter+1)*8);
						}
					#endif
					PORT1_address = (setaddress_counter==0) ? wValue : PORT1_address;
					PORT2_address = (setaddress_counter==1) ? wValue : PORT2_address;
					PORT3_address = (setaddress_counter==2) ? wValue : PORT3_address;
					setaddress_counter++;
				}
			}
			else
			{
				dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);			
			}

			ep0state[USBdeviceType] = EP0_STATUS;
			break;

			// tomadd 2010.11.01
			// Process USB2.0 Chap 9 GET STATUS 
/*----------------------------*/
/*               GET STATUS                      */
/*----------------------------*/
		case USB_REQ_GET_STATUS:
    			len = 2;
			*((INT8U *) (DESC_ADDR + 0xD4)) = 0x00;
        		bufaddr = (INT8U *)(DESC_ADDR + 0xD4);
			totalLen = len;
			epstate[USBdeviceType] = EP_CBW;

			if(USBdeviceType == EHCI_USBD)
			{
				rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
			}
			else
			{
				dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
				dwc_otg_ep_start_transfer(CBWLength, (INT8U *) CBW, is_OUT);
			}

			break;
			// tomadd 2010.11.01
			// Process USB2.0 Chap 9 GET INTERFACE
/*----------------------------*/
/*               GET INTERFACE                 */
/*----------------------------*/
		case USB_REQ_GET_INTERFACE:
			len = 1;
			bufaddr = (INT8U *)(DESC_ADDR + 0xD4);
			totalLen = len;
			if(USBdeviceType == EHCI_USBD)
			{
				rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
			}
			else
			{
				dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
			}
			break;
			// tomadd 2010.11.01
			// Process USB2.0 Chap 9 SET INTERFACE
/*----------------------------*/
/*               SET INTERFACE                  */
/*----------------------------*/
		case USB_REQ_SET_INTERFACE:
			totalLen = 0;

			if(USBdeviceType == EHCI_USBD)
			{
				//Control write transfers return status information in the data phase(Zero-length data packet) 
				//of the Status stage transaction.
				rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
			}
			else
			{
				dwc_otg_ep0_start_transfer(0, (INT8U *) setup_pkt, is_IN);
			}
			
			ep0state[USBdeviceType] = EP0_STATUS;
			break;
			// tomadd 2010.11.01
			// Process USB2.0 Chap 9 GET CONFIGURATION
/*----------------------------*/
/*               GET CONFIGURATION         */
/*----------------------------*/
		 case USB_REQ_GET_CONFIGURATION:
		 	len = 1;
			bufaddr = (INT8U *)(DESC_ADDR + 0xD8);
			totalLen = len;
			
			if(USBdeviceType == EHCI_USBD)
			{
				rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
					
			}
			else
			{
				dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
			}
			break;
/*----------------------------*/
/*               SET CONFIGURATION         */
/*----------------------------*/
		case USB_REQ_SET_CONFIGURATION:
			bufaddr = setup_pkt;
			totalLen = 0;
			if(USBdeviceType == EHCI_USBD)
			{
				rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);			
			}
			else
			{
				dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
				dwc_otg_ep_activate( 1 );
				dwc_otg_ep_activate( 2 );
				dwc_otg_ep_start_transfer(CBWLength, CBW, is_OUT);
			}

			ep0state[USBdeviceType] = EP0_STATUS;
			break;
/*----------------------------*/
/*               CLEAR FEATURE                */
/*----------------------------*/
		case USB_REQ_CLEAR_FEATURE:
			switch(bRequestType & USB_RECIP_MASK) 
			{
				case USB_RECIP_DEVICE:
					if(USBdeviceType == EHCI_USBD)
					{
						rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
					}
					else
					{
						dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
					}
					ep0state[USBdeviceType] = EP0_STATUS;
					break;
			
				case USB_RECIP_ENDPOINT:
					ep = wIndex & 0x0f;
					if ( ep == 0 ) 
					{
						ep_clear_stall( 0, is_OUT);
						ep_clear_stall( 0, is_IN);
					} 
					else if ( ep == 1 ) 
					{
						ep_clear_stall( 1, is_IN );
						dwc_otg_ep_activate( ep );
					}
					else 
					{
						ep_clear_stall( 2, is_OUT );
						clearfeature = 1;
					}
					bufaddr = setup_pkt;
					totalLen = len;	
					if(USBdeviceType == EHCI_USBD)
					{
						#ifdef EHCISTALL
						ep_stall=1;
						#endif
						rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
						#ifdef EHCISTALL
						ep_stall=0;
						#endif
					}
					else
					{
						ep0state[USBdeviceType] = EP0_STATUS;
						dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
					}
					break;
			}
			break;
/*----------------------------*/
/*               SET FEATURE                     */
/*----------------------------*/
		//tomadd 2011.05.24
		case USB_REQ_SET_FEATURE:
		    switch (wValue) 
			{
			        case USB_DEVICE_REMOTE_WAKEUP:
			        if(USBdeviceType == EHCI_USBD)
			        {
						rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
					}
					else
					{
			        		dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
					}
			        	ep0state[USBdeviceType] = EP0_STATUS;
			            break;

			        case USB_DEVICE_TEST_MODE:
					ep = (wIndex & 0x0f00)>>8;
					switch (ep)
					{
						case 1: // TEST_J
							if(USBdeviceType == EHCI_USBD)
							{
								rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
							}
							else
							{
								dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
							}
							B_tmp = 0x10;
						break;

						case 2: // TEST_K
							if(USBdeviceType == EHCI_USBD)
							{
								rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
							}
							else
							{
								dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
							}
							B_tmp = 0x20;
						break;

						case 3: // TEST_SE0_NAK
							if(USBdeviceType == EHCI_USBD)
							{
								rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
							}
							else
							{
								dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
							}
							B_tmp = 0x30;
						break;

						case 4: // TEST_PACKET
							if(USBdeviceType == EHCI_USBD)
							{
								rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
							}
							else
							{
								dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
							}
							B_tmp = 0x40;
						break;

						case 5: // TEST_FORCE_ENABLE
							if(USBdeviceType == EHCI_USBD)
							{
								rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
							}
							else
							{
								dwc_otg_ep0_start_transfer(0, setup_pkt, is_IN);
							}
							B_tmp = 0x50;
						break;
					}
						
					if(USBdeviceType != EHCI_USBD)
					{
						D_tmp = REG32(DCTL);
						//delay for test mode, tom modified 2010.08.11
						bsp_wait(50);
						REG32(DCTL)= D_tmp | B_tmp ;
					}
					ep0state[USBdeviceType] = EP0_STATUS;
			            break;
		        }
		        break;
/*----------------------------*/
/*               IdeGetMaxLun??                */
/*----------------------------*/
		case IdeGetMaxLun:
			len = 1;
			bufaddr = (INT8U *)(DESC_ADDR + 0xD0);
			totalLen = len;
			
			if(USBdeviceType == EHCI_USBD)
			{
				#ifdef EHCISTALL
				ep_stall=1;
				#endif
				
				#if 1
				//tomadd 2011.10.26
				//for EP phase II debug
				Wt_IBIO(IBACTADDR,0x00000001);
				#endif
				
				//tomadd 2011.05.02
				//for EHCI engine usb ch9 reponse data 
				rtkehci_ep0_start_transfer_test(len,bufaddr,is_IN);
				#ifdef EHCISTALL
				ep_stall=0;
				#endif
			}
			else
			{
				dwc_otg_ep0_start_transfer(len, bufaddr, is_IN);
			}
			break;
			
		default:
			break;
	 }
	#ifdef COMPOSITE_HID
	}
	else{
			//rtkehci_ep0_start_transfer_test(0,bufaddr,is_IN);
			HID_ep 	= bRequest;
			is_out	= 1;
	}
	#endif
 
//	dbg("new pointer = 0x%08x\n", (unsigned int)setup_pkt);
}
