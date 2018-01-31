#include "rtskt.h"
#include "tcp.h"
#include "lib.h"
#include "des.h"
#include "vnc.h"
#include "usb.h"
#include "rmcp.h"
#include "telnet.h"

#if CONFIG_VNC_ENABLED
OS_STK  VNCClientStk[TASK_VNC_CLIENT_STK_SIZE];

extern USBCB usbcb;
extern char kb_ready;
extern char mouse_ready ;

extern struct VgaInfo VGAInfo;

unsigned char NewVNCCli = 0;
#define VNCPROTOCOLVER "RFB 003.008\n"

#define VNC_AUT_NONE 1
#define VNC_AUT_VNC 2
#define VNC_AUT_ALG VNC_AUT_VNC
#define VNC_CHALLENGE_LEN 16

int vnc = 0;
enum _VNCclientStates
{
	PtlVerState = 0x00,
	SecurityState = 0x01,
	ClientInitState = 0x02,
	ClientAuthState = 0x03,
	ServerInitState = 0x10,	
	ClientToSrvMsgState = 0x11
}VNCclientStates;

enum _VNKeysym
{
  VNC_KEY_BACKSPACE = 0xFF08, 
  VNC_KEY_TAB = 0xFF09,   
  VNC_KEY_ENTER = 0xFF0D, 
  VNC_KEY_ESCAPE = 0xFF1B,
  VNC_KEY_INSERT = 0xFF63,
  VNC_KEY_KEY_DELETE = 0xFFFF,   
  VNC_KEY_HOME = 0xFF50, 
  VNC_KEY_LEFT_ARROW = 0xFF51, 
  VNC_KEY_UP_ARROW = 0xFF52, 
  VNC_KEY_RIGHT_ARROW = 0xFF53, 
  VNC_KEY_DOWN_ARROW = 0xFF54, 
  VNC_KEY_PAGE_UP = 0xFF55, 
  VNC_KEY_PAGE_DOWN = 0xFF56,  
  VNC_KEY_END = 0xFF57
}VNKeysym;

typedef struct _PIXEL_FORMAT
{
	unsigned char bit_per_pixel;
	unsigned char depth;
	unsigned char big_endian_flag;
	unsigned char true_color_flag;
	unsigned short red_max;
	unsigned short green_max;
	unsigned short blue_max;
	unsigned char red_shift;
	unsigned char green_shift;
	unsigned char blue_shift;
	unsigned char padding[3];	
}PIXEL_FORMAT;

struct _framebufferPara
{
	unsigned short width;
	unsigned short height;
	PIXEL_FORMAT pf;
	unsigned int name_len;	
}FramebufferPara;

struct _framebufferUpdate
{
	unsigned char msg_type;
	unsigned char padding;
	unsigned short numRect;
	unsigned short x;
	unsigned short y;
	unsigned short width;
	unsigned short height;	
	unsigned int encoding_type;	
}framebufferUpdate;


struct _VNCHIDLastState
{
	short diffx;
	short diffy;
	unsigned short x;
	unsigned short y;
}VNCHIDLastState;

struct _HIDKeyReport
{
	char mod;
	char rsv;
	char key0;
	char key1;
	char key2;
	char key3;
	char key4;
	char key5;
	char key6;
}HIDKeyReport;

void FramebufferUpdate(RTSkt* cs, PKT* rxPkt)
{
	//VNC viewer, low 64 color setting
	static int vncmm = 0;
	#define BLK_SIZE 16*16*4 
	//4 is RGBA	
	PKT* txPkt;	
	char *data = malloc(BLK_SIZE);
	int i = 0;
	int t = 800*600*4;
	
	vncmm++;

	
	{
		txPkt = (PKT*)allocPkt(sizeof(framebufferUpdate));
		memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
		framebufferUpdate.msg_type = 0;
		framebufferUpdate.numRect = htons(1);
		
		framebufferUpdate.x = 0;//no more than 800
		framebufferUpdate.y = 0;
		framebufferUpdate.x = htons(framebufferUpdate.x);
		framebufferUpdate.y = htons(framebufferUpdate.y);
		framebufferUpdate.height = htons(600);
		framebufferUpdate.width = htons(800);
		
		framebufferUpdate.encoding_type = 0;
		memcpy(txPkt->wp, &framebufferUpdate, sizeof(framebufferUpdate));
		rtSktSend(cs, txPkt, sizeof(framebufferUpdate));
				
		//memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
		while(t >= BLK_SIZE)
		{
			txPkt = (PKT*)allocPkt(BLK_SIZE);
			for(i = 0; i <BLK_SIZE ; i++)
			{
				data[i] = 0x77;//rand()%256;
			}
			memcpy(txPkt->wp, data, BLK_SIZE);
			rtSktSend(cs, txPkt, BLK_SIZE);
			t  = t - BLK_SIZE;
		}

			
		free(data);
		
		//OSTimeDly(10);

	}
	return;
	//while(1)
	{
		txPkt = (PKT*)allocPkt(sizeof(framebufferUpdate));
		memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
		framebufferUpdate.msg_type = 0;
		framebufferUpdate.numRect = htons(1);
		
		framebufferUpdate.x = 0 + (rand()%780);//no more than 800
		framebufferUpdate.y = 0 + (rand()%580);
		framebufferUpdate.x = htons(framebufferUpdate.x);
		framebufferUpdate.y = htons(framebufferUpdate.y);
		framebufferUpdate.height = htons(16);
		framebufferUpdate.width = htons(16);
		
		framebufferUpdate.encoding_type = 0;
		memcpy(txPkt->wp, &framebufferUpdate, sizeof(framebufferUpdate));
		rtSktSend(cs, txPkt, sizeof(framebufferUpdate));
		
		txPkt = (PKT*)allocPkt(BLK_SIZE);
		//memset(&framebufferUpdate, 0, sizeof(framebufferUpdate));
		for(i = 0; i <BLK_SIZE ; i++)
		{
			data[i] = 0x77;//rand()%256;
		}

		memcpy(txPkt->wp, data, BLK_SIZE);
		free(data);
		rtSktSend(cs, txPkt, BLK_SIZE);
		//OSTimeDly(10);

	}
	
}


void flip_VNC_PWD(char *in, char *out, int len)
{
	int l = 0;
	int i = 0;

	memset(out, 0, len);

	for(l = 0; l <len; l++)
	{
		//one char have 8 bit
		for(i = 0; i < 8; i++)
		{
			if(in[l]& (1<<i))
			{
				out[l] += (1<<(7-i));
			}
		}
	}
}

void vnc_key_down(unsigned int key)
{	
	//0-0x20??
	//VNC only pass ascii key and xlib key
	if(key >=0x20 && key <=0x7f)
	{
		if(key == 0x20)//SPACE
		{
			HIDKeyReport.key0 = 0x2c;
		}
		else if(key >=0x21 && key <=0x28)//1-9 + shift (ASCII) <=> 
		{
			HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
			switch(key)
			{
				case '!':
				HIDKeyReport.key0 = 0x1e;
				break;
				case '"':
				HIDKeyReport.key0 = 0x34;
				break;
				case '#':
				HIDKeyReport.key0 = 0x20;
				break;
				case '$':
				HIDKeyReport.key0 = 0x21;
				break;
				case '%':
				HIDKeyReport.key0 = 0x22;
				break;			
				case '&':
				HIDKeyReport.key0 = 0x24;
				break;
				case '\'':
				HIDKeyReport.mod = 0;
				HIDKeyReport.key0 = 0x34;
				break;
				case '(':
				HIDKeyReport.key0 = 0x26;
				break;
				case ')':
				HIDKeyReport.key0 = 0x27;
				break;
			}			
		}
		else if(key ==0x29)//0 + shift(ASCII) <=> HID 0x27
		{
			HIDKeyReport.key0 = 0x27;
			HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
		}
		else if(key >= 0x2a && key <=0x2f) //0x2a- 0x2f
		{
			switch(key)
			{
				case '*':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x55;
				break;
				case '+':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x57;
				break;
				case ',':
				HIDKeyReport.key0 = 0x36;
				break;
				case '-':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x56;
				break;
				case '.':
				HIDKeyReport.key0 = 0x37;
				break;
				case '/':
				HIDKeyReport.key0 = 0x54;
				break;				
			}
		}
		else if(key ==0x30)//0(ASCII) <=> HID 0x27
		{
			HIDKeyReport.key0 = 0x27;			
		}		
		else if(key >=0x31 && key <=0x39)//1-9(ASCII) <=> HID 0x1e-0x26
		{
			HIDKeyReport.key0 = key - 0x31 + 0x1e;
		}
		else if(key >= 0x3a && key <=0x40)
		{
			switch(key)
			{			
				case ':':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;				
				case ';':
				HIDKeyReport.key0 = 0x33;
				break;				
				case '<':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x36;
				break;
				case '=':
				HIDKeyReport.key0 = 0x2e;
				break;
				case '>':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x37;
				break;
				case '?':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x38;
				break;				
				case '@':
				HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
				HIDKeyReport.key0 = 0x1f;
				break;
			}
		}
		else if(key >=0x41 && key <=0x5a)//A-Z(ASCII) <=> HID 4-29
		{
			HIDKeyReport.key0 = key - 0x41 + 4;
			HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
		}
		else if(key >=0x5b && key <=0x60)//0x5b-60
		{
			switch(key)
			{			
			case '[':
			HIDKeyReport.key0 = 0x2f;
			break;
			case '\\':
			HIDKeyReport.key0 = 0x31;
			break;
			case ']':
			HIDKeyReport.key0 = 0x30;
			break;
			case '_':
			HIDKeyReport.key0 = 0x2d;
			break;
			case '`':
			HIDKeyReport.key0 = 0x35;
			break;
			case '^':
			HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
			HIDKeyReport.key0 = 0x23;
			break;
			}		
		}
		else if(key >=0x61 && key <=0x7a)//a-z(ASCII) <=> HID 4-29
		{
			HIDKeyReport.key0 = key - 0x61 + 4;
		}
		else if(key >=0x7b && key <=0x7f)//0x7b-0x7f
		{
			HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
			switch(key)
			{			
				case '{':
				HIDKeyReport.key0 = 0x2f;
				break;			
				case '|':
				HIDKeyReport.key0 = 0x31;
				break;	
				case '}':
				HIDKeyReport.key0 = 0x30;
				break;	
				case '~':
				HIDKeyReport.key0 = 0x35;
				break;	
			}
		}
	}	
	else if(key >=0xffbe && key <=0xffc9)//F1-F12
	{
		HIDKeyReport.key0 = (char)(key - 0xffbe + 0x58);
	}	
	else if(key == VNC_Left_Shift) //shift, control, alt
	{
		HIDKeyReport.mod |=KEY_MODIFIER_LEFT_SHIFT;
	}	
	else if(key == VNC_Right_Shift)
	{
		HIDKeyReport.mod |=KEY_MODIFIER_RIGHT_SHIFT;
	}
	else if(key == VNC_Left_Ctl)
	{
		HIDKeyReport.mod |=KEY_MODIFIER_LEFT_CTRL;
	}	
	else if(key == VNC_Right_Ctl)
	{
		HIDKeyReport.mod |=KEY_MODIFIER_RIGHT_CTRL;
	}	
	else if(key == VNC_Left_Ctl)
	{
		HIDKeyReport.mod |=KEY_MODIFIER_LEFT_CTRL;
	}	
	else if(key == VNC_Right_Ctl)
	{
		HIDKeyReport.mod |=KEY_MODIFIER_RIGHT_CTRL;
	}	
	else if(key == VNC_KEY_BACKSPACE)
	{
		HIDKeyReport.key0 = 0x2a;
	}
	else if(key == VNC_KEY_TAB)
	{
		HIDKeyReport.key0 = 0x2b;
	}
	else if(key == VNC_KEY_ENTER)
	{
		HIDKeyReport.key0 = 0x28;
	} 
  	else if(key == VNC_KEY_ESCAPE)
  	{
  		HIDKeyReport.key0 = 0x29;
  	}
  	else if(key == VNC_KEY_INSERT)
  	{
  		HIDKeyReport.key0 = 0x49;
  	}
  	else if(key == VNC_KEY_KEY_DELETE)
  	{
  		HIDKeyReport.key0 = 0x4c;
  	}  	
  	else if(key == VNC_KEY_HOME)        
  	{
  		HIDKeyReport.key0 = 0x4a;
  	}
  	else if(key == VNC_KEY_LEFT_ARROW)
  	{
  		HIDKeyReport.key0 = 0x50;
  	}
  	else if(key == VNC_KEY_UP_ARROW)
  	{
  		HIDKeyReport.key0 = 0x52;
  	}
  	else if(key == VNC_KEY_RIGHT_ARROW)
  	{
  		HIDKeyReport.key0 = 0x4f;
  	}
  	else if(key == VNC_KEY_DOWN_ARROW)
  	{
  		HIDKeyReport.key0 = 0x51;
  	}  	 
  	else if(key == VNC_KEY_PAGE_UP) 
  	{
  		HIDKeyReport.key0 = 0x4b;
  	}  	
  	else if(key == VNC_KEY_PAGE_DOWN)  
  	{
  		HIDKeyReport.key0 = 0x5e;
  	}  	
  	else if(key == VNC_KEY_END)
  	{
  		HIDKeyReport.key0 = 0x4d;
  	}  
}

void vnc_key_up(unsigned int key)
{

}

void VNCClient(void *data)
{
	PKT* txPkt;
	PKT* rxPkt;
	PKT* lastPkt;
	int status = 0;
	unsigned int  i = 0, t = 0;
	RTSkt* cs = (RTSkt*)data;
	char shareflag = 0;
	des_context ctx;    
    unsigned char buf[16] ={0};
	//unsigned char buf1[64];
	unsigned char *tbm = (unsigned char *)malloc(64);
	char VNCChallengeMsg[VNC_CHALLENGE_LEN];
	
	char pwd[8] = {0};
	char key[8] = {0};
	char tmpx;
	char tmpy;
	char difftype;
	unsigned char aggcnt;
	unsigned char aggcode;
	char rgba[4]={0xff, 0, 0, 0};
	int timeout = 1;
	
		
	short diffx, diffy;
	VNCPointEvent *pe;
	VNCKeyEvent	*ke;
	HIDMouse_Report hmr;

	pwd[0] = 'r';
	pwd[1] = 't';
	pwd[2] = 'k';
	
	printf("VNC Client \n");
	memset(VNCHIDLastState, 0, sizeof(VNCHIDLastState));

	//First is the handshaking phase, the purpose of which is to agree upon the
	//protocol version and the type of security to be used
	VNCclientStates = PtlVerState;
	txPkt = (PKT*)allocPkt(strlen(VNCPROTOCOLVER));
	memcpy(txPkt->wp, VNCPROTOCOLVER, strlen(VNCPROTOCOLVER));
	rtSktSend(cs, txPkt, strlen(VNCPROTOCOLVER));
	
	do
    {        
		rxPkt = rtSktRx(cs, timeout, &status);
		if((timeout == 1) && !rxPkt)
		{
			timeout = 2;
		}
		else if(rxPkt)
		{
			timeout = 1;
		}

		if (rxPkt && (VNCclientStates == ClientAuthState))
		{
			txPkt = (PKT*)allocPkt(4);
			memset(txPkt->wp, 0, 4);

			flip_VNC_PWD(pwd, key, sizeof(pwd));
			des_setkey_dec( &ctx, (unsigned char *) key );
			
			//VNC use ecb not cbc
			des_crypt_ecb( &ctx,
				rxPkt->wp,
				buf);
			
			des_crypt_ecb( &ctx,
				rxPkt->wp + 8,
				buf + 8);
			
			if(memcmp(VNCChallengeMsg, buf, VNC_CHALLENGE_LEN) == 0)
			{

			}
			else
			{
				//close
				txPkt->wp[3] = 1;
			}

			memset(VNCChallengeMsg, 0, VNC_CHALLENGE_LEN);
			
			VNCclientStates = ClientInitState;		
			
			rtSktSend(cs, txPkt, 4);
		//	vnc = 1;
		}
		else if (rxPkt && (VNCclientStates == ClientInitState))
		{
			shareflag = rxPkt->wp[0];
			VNCclientStates = ServerInitState;

			txPkt = (PKT*)allocPkt(sizeof(FramebufferPara) + 7);
			memset(txPkt->wp, 0, sizeof(FramebufferPara) + 7);
			#ifndef CONFIG_VNC_VIDEO_ENABLED
			VGAInfo.FB1_BPP = 4;
			#endif
			FramebufferPara.height = htons(768);//tmp fixed
			FramebufferPara.width = htons(1024);//tmp fixed
			FramebufferPara.pf.bit_per_pixel = VGAInfo.FB1_BPP*8;//RGBA, 8 bit
			FramebufferPara.pf.depth = 24;
			FramebufferPara.pf.red_max = htons(255);
			FramebufferPara.pf.green_max = htons(255);
			FramebufferPara.pf.blue_max = htons(255);
			FramebufferPara.pf.red_shift= 16;
			FramebufferPara.pf.green_shift = 8;
			FramebufferPara.pf.blue_shift = 0;
			
			FramebufferPara.name_len = htonl(7);
			memcpy(txPkt->wp + sizeof(FramebufferPara), "RtkDash", 7);
			//FramebufferPara.pf.big_endian_flag = 0;
			FramebufferPara.pf.true_color_flag = 1;
			memcpy(txPkt->wp, &FramebufferPara, sizeof(FramebufferPara));
			
			rtSktSend(cs, txPkt, sizeof(FramebufferPara) + 7);
			freePkt(rxPkt);
			VNCclientStates = ClientToSrvMsgState;
			break;
		}
		
		if (rxPkt && (VNCclientStates == SecurityState))
        {
			if(VNC_AUT_ALG == VNC_AUT_NONE)
			{
				txPkt = (PKT*)allocPkt(4);
				memset(txPkt->wp, 0, 4);
				
				if(rxPkt->wp[0] != VNC_AUT_NONE)
				{
					txPkt->wp[0] = VNC_AUT_NONE;
				}
				
				VNCclientStates = ClientInitState;
				
				rtSktSend(cs, txPkt, 4);
			}
			else
			{
			
				if(rxPkt->wp[0] != VNC_AUT_VNC)
				{					
					rtSktClose(cs);
				}
				else
				{
					txPkt = (PKT*)allocPkt(VNC_CHALLENGE_LEN);
					
					for(i = 0; i < VNC_CHALLENGE_LEN; i++)
					{
						VNCChallengeMsg[i] = rand() % 256;
					}

					memcpy(txPkt->wp, VNCChallengeMsg, VNC_CHALLENGE_LEN);
					rtSktSend(cs, txPkt, VNC_CHALLENGE_LEN);
					VNCclientStates = ClientAuthState;
				}
			}
        }

        if (rxPkt && (VNCclientStates == PtlVerState))
        {
			if(memcmp(rxPkt->wp, VNCPROTOCOLVER, strlen(VNCPROTOCOLVER)) == 0)
			{
				VNCclientStates = SecurityState;
				txPkt = (PKT*)allocPkt(2);

				txPkt->wp[0] = 1;
				txPkt->wp[1] = VNC_AUT_ALG;
				rtSktSend(cs, txPkt, 2);
			}
        }
		
		if(rxPkt)
			freePkt(rxPkt);
    } while ((status != SKTDISCONNECT) && (!NewVNCCli));
	
	printf("client VNC syn fin\n");

	lastPkt = 0;
	
	while(!NewVNCCli)
	{
		//In FPGA, if mouse move too fast, system will hang
		
	
		//vga_send_test(cs, rgba,0 , 0);
		rgba[0] = 0;
		rgba[1] = 0xff;		
		//vga_send_test(cs, rgba,128 , 128);
		//vga_send(cs, 0x88000000, 0,0);
		//memcpy(0x88200000,0x88000000,800*600*4);
		//vga_send_dump(cs, 0x88000000);
		//vga_send(cs, 0x88000000+(128*800*4)+128*4, 128,128);
		//printf("cli rx go ==>\n");		
		
		//rxPkt = rtSktRx(cs, OS_TICKS_PER_SEC/100, &status);	
		rxPkt = rtSktRx(cs, timeout, &status);
		if((timeout == 1) && !rxPkt)
		{
			timeout = 2;
		}
		else if(rxPkt)
		{
			timeout = 1;
		}		

		if(rxPkt)
		{
			printf("TYP %d\n", rxPkt->wp[0]);
			switch(rxPkt->wp[0])
			{
			case 0:
				printf("SetPixelFormat\n");
				break;
			case 1:
				printf("Error No this type\n");
				break;			
			case 2:
				printf("SetEncoding\n");
				break;
			case 3:
				printf("FramebufferUpdate>>>\n");
				//FramebufferUpdate(cs, rxPkt);
				//update_frame_buffer_hao(cs);
				printf("FramebufferUpdate<<\n");
				break;
			case 4:
				#if CONFIG_VNC_KM_ENABLED
				break;
				memset(&HIDKeyReport, 0, sizeof(HIDKeyReport));
				ke = (VNCKeyEvent*)rxPkt->wp;				
				ke->key = ntohl(ke->key);
				if(ke->down_flag)
				{
					printf("Down ");
					vnc_key_down(ke->key);
				}
				else
				{
					printf("up ");
					vnc_key_up(ke->key);
				}
				if(ke->key >=0x20 && ke->key <0x7e)
					printf("KeyEvent %c\n", ke->key);
				else
					printf("KeyEvent %x\n", ke->key);
				
				printf("rtkehci_INTIN_start_transfer==>\n");
				memcpy(usbcb.tmpbuf, &HIDKeyReport, 8);
				rtkehci_INTIN_start_transfer(8 , usbcb.tmpbuf, is_IN, 1);
				printf("rtkehci_INTIN_start_transfer<==\n");				
				memset(usbcb.tmpbuf, 0, 16);    	
    			
				#endif				
				break;
				
			case 5:
printf("PointerEvent\n");			
				#if CONFIG_VNC_KM_ENABLED
				break;
				pe = (VNCPointEvent*)rxPkt->wp;
				pe->x = ntohs(pe->x);
				pe->y = ntohs(pe->y);
				memset(&hmr, 0, sizeof(hmr));
					hmr.rID = 1;
					hmr.x = pe->x;
					hmr.y = (pe->y*768)/1024 ;
				memcpy(tbm, &hmr, 7);
				rtkehci_INTIN_start_transfer(7 , tbm, is_IN, 2);
					//memcpy(usbcb.tmpbuf, &hmr, 7);
					//rtkehci_INTIN_start_transfer(7 , usbcb.tmpbuf, is_IN, 2);
				#endif
				break;
				printf("PointerEvent %d %d %d\n", pe->button_mask, pe->x, pe->y);
				
				if(VNCHIDLastState.x !=  pe->x)
				{
					diffx = pe->x - VNCHIDLastState.x;					
				}
				
				if(VNCHIDLastState.y !=  pe->y)
				{
					diffy = pe->y - VNCHIDLastState.y;					
				}				
								 
				do
				{
					tmpx = tmpy =0;
					if(diffx)
					{
						if(diffx < -127)
						{
							diffx = diffx + 127;
							tmpx = -127;
						}
						else if(diffx > 127)
						{
							diffx = diffx - 127;
							tmpx = 127;
						}
						else
						{
							tmpx = diffx;
							diffx = 0;
						}
					}
					
					if(diffy)
					{
						if(diffy < -127)
						{
							tmpy = -127;
							diffy = diffy + 127;
						}
						else if(diffy > 127)
						{
							tmpy = 127;
							diffy = diffy - 127;
						}
						else
						{
							tmpy = diffy;
							diffy = 0;
						}
					}
					memset(&hmr, 0, sizeof(hmr));
					hmr.buttons = pe->button_mask;
					
					if(hmr.buttons == 4)
						hmr.buttons = 2;
					else if(hmr.buttons == 2)
						hmr.buttons = 4;
						
					hmr.rID = 1;
					hmr.x = tmpx;
					hmr.y = tmpy;
					memcpy(usbcb.tmpbuf, &hmr, 7);
					rtkehci_INTIN_start_transfer(7 , usbcb.tmpbuf, is_IN, 2);
					
					
					
					//memset(usbcb.tmpbuf, 0, 16);
					//usbcb.tmpbuf[0] = 1;
					//rtkehci_INTIN_start_transfer(5 , usbcb.tmpbuf, is_IN, 2);
					
					printf("PointerEvent to usb %d %d %d\n", hmr.buttons, hmr.x, hmr.y );
					OSTimeDly(30);
				}while(diffx || diffy);
				
				
				VNCHIDLastState.x = pe->x;
				VNCHIDLastState.y = pe->y;

				//VNCHIDLastState.HIDbutton
				//hmr.buttons = 1; 
				//buttons.x
				//buttons.y
				break;
			case 6:
				printf("ClientVutText\n");
				break;
			}
			freePkt(rxPkt);
			continue;
		}
		else
		{
			if (status == SKTDISCONNECT)
			{
				break;
			}
			#ifdef CONFIG_VNC_VIDEO_ENABLED
			//update_frame_buffer_hao(cs);
			#endif
			continue;
		}
	}
	#if CONFIG_VNC_KM_ENABLED
	#if 0
	disable_vnc_usb_dev();
	#endif
	#endif
	stopconn(cs);
    OSTaskDel(OS_PRIO_SELF);
}

void VNCSrv(void *data)
{
	RTSkt* s = rtSkt(IPv4, IP_PROTO_TCP);
	RTSkt* cs;
	RTSkt* pcs;
	OS_TCB pdata;
	char VNC_cli_first_conn = 1;
	
	rtSktEnableIBPortFilter_F(s);
	rtSktBind(s, 5900);
	rtSktListen(s, 2);

	#if CONFIG_VNC_KM_ENABLED
	#if 0
	rtkehci_init();
	
	usbcb.usbstate = LISTENED;
	usbcb.usbstate = ACCEPTED;
	
	usbcb.waitRx = 0;
	usbcb.usbmode = SERVER_MODE;
	kb_ready = 0;
	mouse_ready = 0;

	//usbcb.bsp_usb_enabled();
	usbcb.usbstate = ENABLED;//+Tom 20131105
	ehci_intep_enabled(1);
	#endif

	#endif
		
	printf("VNC Srv\n");


		
	while(1)
	{
		cs = rtSktAccept(s);
		if(!bsp_bits_get(MAC_MAC_STATUS, BIT_ISOLATE, 1))
		{
			SMBus_Prepare_RmtCtrl(RMCP_PWR_On, 0);
			OSTimeDly(OS_TICKS_PER_SEC*2);
			#ifdef CONFIG_VNC_VIDEO_ENABLED
			VGA_enable();
			#endif
		}

		#ifdef CONFIG_VNC_KM_ENABLED
		#if 0
		if(!VNC_cli_first_conn)
		{
			kb_ready = 0;
			mouse_ready = 0;
			printf("usb enable ==>\n");		
	 		//usbcb.bsp_usb_enabled();
			usbcb.usbstate = ENABLED;//+Tom 20131105			
			ehci_intep_enabled(1); 	
			printf("usb enable <==\n");
		}
		else
		{
			VNC_cli_first_conn = 0;
		}
		//while((kb_ready == 0) || (mouse_ready == 0))
		//	;
		#endif
		#endif
		
		NewVNCCli = 1;

		while(OSTaskQuery(TASK_VNC_CLIENT_PRIO, &pdata)  == OS_ERR_NONE)
		{		
			printf("VNC Client Task exist. Wait for Close\n");
			OSTimeDly(10);
		}
		printf("Create VNC Client\n");

		NewVNCCli = 0;

		OSTaskCreate(VNCClient, cs, (unsigned long*)&VNCClientStk[TASK_VNC_CLIENT_STK_SIZE - 1], TASK_VNC_CLIENT_PRIO);
	}
}
#endif