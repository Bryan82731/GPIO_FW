
flashprog-4281_dbg.exe:     file format elf32-littlemips

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .boot         00000078  800c0000  800c0000  00000060  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text         00001430  800c0078  800c0078  000000d8  2**2
                  CONTENTS, ALLOC, LOAD, CODE
  2 .bss          00000434  800c14b0  800c14b0  00001508  2**4
                  ALLOC
  3 .reginfo      00000018  00000000  00000000  00001508  2**2
                  CONTENTS, READONLY, LINK_ONCE_SAME_SIZE
  4 .pdr          000003a0  00000000  00000000  00001520  2**2
                  CONTENTS, READONLY
  5 .comment      00000041  00000000  00000000  000018c0  2**0
                  CONTENTS, READONLY
  6 .gnu.attributes 00000010  00000000  00000000  00001901  2**0
                  CONTENTS, READONLY
  7 .debug_info   00000aa1  00000000  00000000  00001911  2**0
                  CONTENTS, READONLY, DEBUGGING
  8 .debug_abbrev 00000429  00000000  00000000  000023b2  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .debug_aranges 000000c0  00000000  00000000  000027e0  2**3
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_line   000004ec  00000000  00000000  000028a0  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .debug_str    0000044a  00000000  00000000  00002d8c  2**0
                  CONTENTS, READONLY, DEBUGGING
 12 .debug_frame  000003d8  00000000  00000000  000031d8  2**2
                  CONTENTS, READONLY, DEBUGGING

Disassembly of section .boot:

800c0000 <_boot>:
_boot():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:41

    ##################################################################
    # After reset
    # BEV = 1, IM = 0, CU = 0  
    ##################################################################
    mfc0    s4, CP0_STATUS        #save original in s4
800c0000:	40146000 	mfc0	s4,$12
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:42
    nop
800c0004:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:43
    and     t0, s4, ~SR_BEV
800c0008:	3c01ffbf 	lui	at,0xffbf
800c000c:	3421ffff 	ori	at,at,0xffff
800c0010:	02814024 	and	t0,s4,at
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:44
    or      t0, (SR_CU0 | SR_CU3)        # make sure coprocessors enabled
800c0014:	3c019000 	lui	at,0x9000
800c0018:	01014025 	or	t0,t0,at
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:45
    mtc0    t0, CP0_STATUS
800c001c:	40886000 	mtc0	t0,$12
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:46
    mtc0    zero, CP0_CAUSE     # zero the cause register
800c0020:	40806800 	mtc0	zero,$13
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:47
    nop
800c0024:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:53

    ###########################
    # DMEM configuration      #
    ###########################

    la	    t2,	DMEM_BASE
800c0028:	3c0a0004 	lui	t2,0x4
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:54
    la      t3, DMEM_TOP	
800c002c:	3c0b0007 	lui	t3,0x7
800c0030:	356bffff 	ori	t3,t3,0xffff
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:55
    mtc3	t2,	$4		#DMEM BASE
800c0034:	4c8a2000 	mtc3	t2,$4
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:56
    mtc3	t3,	$5		#DMEM TOP
800c0038:	4c8b2800 	mtc3	t3,$5
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:58
	
    mfc0	t0, 	C0_CCTL		#read cctl
800c003c:	4008a000 	mfc0	t0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:59
    nop
800c0040:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:60
    or	t0, 	0x400		        #set bit 10(DMEMon)
800c0044:	35080400 	ori	t0,t0,0x400
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:61
    mtc0	t0, 	C0_CCTL		#write back to cctl
800c0048:	4088a000 	mtc0	t0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:62
    nop
800c004c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:69
    ##################################################################
    # Initialize Stack Pointer
    #   _stack is initialized by the linker script to point to the
    #    starting location of the stack in DRAM
    ##################################################################
    la    sp,     _stack_hi
800c0050:	3c1d800c 	lui	sp,0x800c
800c0054:	27bd18b0 	addiu	sp,sp,6320
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:76
    ##################################################################
    # Initialize Global Pointer
    #   _gp is initialized by the linker script to point to "middle"
    #   of the small variables region
    ##################################################################
    la	  gp,     _gp
800c0058:	3c1c800c 	lui	gp,0x800c
800c005c:	279c14b0 	addiu	gp,gp,5296
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:78

    la	  t0, main
800c0060:	3c08800c 	lui	t0,0x800c
800c0064:	250801e8 	addiu	t0,t0,488
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:79
    jalr	t0
800c0068:	0100f809 	jalr	t0
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:80
    nop		
800c006c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:86

    ##################################################################
    # Just in case, go into infinite loop
    ##################################################################
1:
    b	  1b
800c0070:	1000ffff 	b	800c0070 <_boot+0x70>
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/crt0.S:87
    nop
800c0074:	00000000 	nop

Disassembly of section .text:

800c0078 <bsp_wait>:
bsp_wait():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:7
#include "flash.h"

INT32U FLASH_ID;

void bsp_wait(INT32U usec)
{
800c0078:	27bdfff0 	addiu	sp,sp,-16
800c007c:	afbe000c 	sw	s8,12(sp)
800c0080:	03a0f021 	move	s8,sp
800c0084:	afc40010 	sw	a0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:9
    INT32U i, j;
    for (i = 0 ; i < usec ; i++)
800c0088:	afc00000 	sw	zero,0(s8)
800c008c:	08030033 	j	800c00cc <bsp_wait+0x54>
800c0090:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:11
        //inner loop is around 1u if runnign at 125 MHz
        for (j = 0 ; j < 31; j++)
800c0094:	afc00004 	sw	zero,4(s8)
800c0098:	0803002c 	j	800c00b0 <bsp_wait+0x38>
800c009c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:12 (discriminator 3)
            asm("nop");
800c00a0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:11 (discriminator 3)
void bsp_wait(INT32U usec)
{
    INT32U i, j;
    for (i = 0 ; i < usec ; i++)
        //inner loop is around 1u if runnign at 125 MHz
        for (j = 0 ; j < 31; j++)
800c00a4:	8fc20004 	lw	v0,4(s8)
800c00a8:	24420001 	addiu	v0,v0,1
800c00ac:	afc20004 	sw	v0,4(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:11 (discriminator 1)
800c00b0:	8fc20004 	lw	v0,4(s8)
800c00b4:	2c42001f 	sltiu	v0,v0,31
800c00b8:	1440fff9 	bnez	v0,800c00a0 <bsp_wait+0x28>
800c00bc:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:9 (discriminator 2)
INT32U FLASH_ID;

void bsp_wait(INT32U usec)
{
    INT32U i, j;
    for (i = 0 ; i < usec ; i++)
800c00c0:	8fc20000 	lw	v0,0(s8)
800c00c4:	24420001 	addiu	v0,v0,1
800c00c8:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:9 (discriminator 1)
800c00cc:	8fc30000 	lw	v1,0(s8)
800c00d0:	8fc20010 	lw	v0,16(s8)
800c00d4:	0062102b 	sltu	v0,v1,v0
800c00d8:	1440ffee 	bnez	v0,800c0094 <bsp_wait+0x1c>
800c00dc:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:14
        //inner loop is around 1u if runnign at 125 MHz
        for (j = 0 ; j < 31; j++)
            asm("nop");

}
800c00e0:	03c0e821 	move	sp,s8
800c00e4:	8fbe000c 	lw	s8,12(sp)
800c00e8:	27bd0010 	addiu	sp,sp,16
800c00ec:	03e00008 	jr	ra
800c00f0:	00000000 	nop

800c00f4 <bsp_gmac_disable>:
bsp_gmac_disable():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:17

void bsp_gmac_disable()
{
800c00f4:	27bdfff8 	addiu	sp,sp,-8
800c00f8:	afbe0004 	sw	s8,4(sp)
800c00fc:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:19
    //Check TPPoll to wait TX Finish
    while(REG8(MAC_BASE_ADDR + MAC_TPPoll) & 0x80)
800c0100:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:19 (discriminator 1)
800c0104:	3c02baf7 	lui	v0,0xbaf7
800c0108:	34420030 	ori	v0,v0,0x30
800c010c:	90420000 	lbu	v0,0(v0)
800c0110:	304200ff 	andi	v0,v0,0xff
800c0114:	00021600 	sll	v0,v0,0x18
800c0118:	00021603 	sra	v0,v0,0x18
800c011c:	0440fff9 	bltz	v0,800c0104 <bsp_gmac_disable+0x10>
800c0120:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:23
        ;

    //TX FIFO is not empty
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x0C00)
800c0124:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:23 (discriminator 1)
800c0128:	3c02baf7 	lui	v0,0xbaf7
800c012c:	34420036 	ori	v0,v0,0x36
800c0130:	94420000 	lhu	v0,0(v0)
800c0134:	3042ffff 	andi	v0,v0,0xffff
800c0138:	30420c00 	andi	v0,v0,0xc00
800c013c:	1440fffa 	bnez	v0,800c0128 <bsp_gmac_disable+0x34>
800c0140:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:26
        ;

    REG8(MAC_BASE_ADDR + MAC_CMD) = 0x00;
800c0144:	3c02baf7 	lui	v0,0xbaf7
800c0148:	34420036 	ori	v0,v0,0x36
800c014c:	a0400000 	sb	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:29

    //wait engine goest down
    while(REG16(MAC_BASE_ADDR + MAC_CMD) & 0x300)
800c0150:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:29 (discriminator 1)
800c0154:	3c02baf7 	lui	v0,0xbaf7
800c0158:	34420036 	ori	v0,v0,0x36
800c015c:	94420000 	lhu	v0,0(v0)
800c0160:	3042ffff 	andi	v0,v0,0xffff
800c0164:	30420300 	andi	v0,v0,0x300
800c0168:	1440fffa 	bnez	v0,800c0154 <bsp_gmac_disable+0x60>
800c016c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:32
        ;

}
800c0170:	03c0e821 	move	sp,s8
800c0174:	8fbe0004 	lw	s8,4(sp)
800c0178:	27bd0008 	addiu	sp,sp,8
800c017c:	03e00008 	jr	ra
800c0180:	00000000 	nop

800c0184 <disable_master_engine>:
disable_master_engine():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:35

void disable_master_engine()
{
800c0184:	27bdffe8 	addiu	sp,sp,-24
800c0188:	afbf0014 	sw	ra,20(sp)
800c018c:	afbe0010 	sw	s8,16(sp)
800c0190:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:37
#ifdef RTL8111EP
    bsp_gmac_disable();
800c0194:	0c03003d 	jal	800c00f4 <bsp_gmac_disable>
800c0198:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:47
  //  REG8(0xB2001000) = 0x00;  //disable smbus
  //  REG8(0xB2001810)  = 0x00; //disable tcr
  //  REG32(0xB1000008) = 0x00000000;  //disable timer
#define SMBUS_BASE_ADDR      0xBAF30000
#define SEIO_SMEn                0x0000
    REG8(SMBUS_BASE_ADDR + SEIO_SMEn) &= 0x08;
800c019c:	3c02baf3 	lui	v0,0xbaf3
800c01a0:	3c03baf3 	lui	v1,0xbaf3
800c01a4:	90630000 	lbu	v1,0(v1)
800c01a8:	306300ff 	andi	v1,v1,0xff
800c01ac:	30630008 	andi	v1,v1,0x8
800c01b0:	306300ff 	andi	v1,v1,0xff
800c01b4:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:51

#define TCR_BASE_ADDR            0xBAF50000	
#define TCR_CONF0                0x10
    REG8(TCR_BASE_ADDR + TCR_CONF0)  = 0x00; //disable tcr
800c01b8:	3c02baf5 	lui	v0,0xbaf5
800c01bc:	34420010 	ori	v0,v0,0x10
800c01c0:	a0400000 	sb	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:55

	#define TIMER_IOBASE         0xBA800000
#define TIMER_CR             0x08
    REG32(TIMER_IOBASE + TIMER_CR) = 0x00000000;  //disable timer	
800c01c4:	3c02ba80 	lui	v0,0xba80
800c01c8:	34420008 	ori	v0,v0,0x8
800c01cc:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:56
}
800c01d0:	03c0e821 	move	sp,s8
800c01d4:	8fbf0014 	lw	ra,20(sp)
800c01d8:	8fbe0010 	lw	s8,16(sp)
800c01dc:	27bd0018 	addiu	sp,sp,24
800c01e0:	03e00008 	jr	ra
800c01e4:	00000000 	nop

800c01e8 <main>:
main():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:59

int main(void)
{
800c01e8:	27bdfe38 	addiu	sp,sp,-456
800c01ec:	afbf01c4 	sw	ra,452(sp)
800c01f0:	afbe01c0 	sw	s8,448(sp)
800c01f4:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:60
    INT32U i = 0;
800c01f8:	afc00010 	sw	zero,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:61
    struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
800c01fc:	3c02bc00 	lui	v0,0xbc00
800c0200:	afc20018 	sw	v0,24(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:63
    INT32U rdid;
    INT8U flag = 0;    
800c0204:	a3c00014 	sb	zero,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:70
	//flash_addr, load_addr and load_size will be set by gdb_flash.scr
	INT32U flash_addr;
    INT8U *load_addr;
	INT32U load_size;	

	int test[100] ={0x3c03a800,0x24630518,0x00001021,0x24040064,
800c0208:	27c2002c 	addiu	v0,s8,44
800c020c:	24030190 	li	v1,400
800c0210:	00603021 	move	a2,v1
800c0214:	00002821 	move	a1,zero
800c0218:	00402021 	move	a0,v0
800c021c:	0c0304fe 	jal	800c13f8 <memset>
800c0220:	00000000 	nop
800c0224:	3c023c03 	lui	v0,0x3c03
800c0228:	3442a800 	ori	v0,v0,0xa800
800c022c:	afc2002c 	sw	v0,44(s8)
800c0230:	3c022463 	lui	v0,0x2463
800c0234:	34420518 	ori	v0,v0,0x518
800c0238:	afc20030 	sw	v0,48(s8)
800c023c:	24021021 	li	v0,4129
800c0240:	afc20034 	sw	v0,52(s8)
800c0244:	3c022404 	lui	v0,0x2404
800c0248:	34420064 	ori	v0,v0,0x64
800c024c:	afc20038 	sw	v0,56(s8)
800c0250:	3c02ac62 	lui	v0,0xac62
800c0254:	afc2003c 	sw	v0,60(s8)
800c0258:	3c022442 	lui	v0,0x2442
800c025c:	34420001 	ori	v0,v0,0x1
800c0260:	afc20040 	sw	v0,64(s8)
800c0264:	3c021444 	lui	v0,0x1444
800c0268:	3442fffd 	ori	v0,v0,0xfffd
800c026c:	afc20044 	sw	v0,68(s8)
800c0270:	3c022463 	lui	v0,0x2463
800c0274:	34420004 	ori	v0,v0,0x4
800c0278:	afc20048 	sw	v0,72(s8)
800c027c:	3c0227bd 	lui	v0,0x27bd
800c0280:	3442ffe8 	ori	v0,v0,0xffe8
800c0284:	afc2004c 	sw	v0,76(s8)
800c0288:	3c02afbf 	lui	v0,0xafbf
800c028c:	34420014 	ori	v0,v0,0x14
800c0290:	afc20050 	sw	v0,80(s8)
800c0294:	3c02afb0 	lui	v0,0xafb0
800c0298:	34420010 	ori	v0,v0,0x10
800c029c:	afc20054 	sw	v0,84(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:76
	0xac620000,0x24420001,0x1444fffd,0x24630004,0x27bdffe8,0xafbf0014,0xafb00010
	
		
	};
	
	for(i =0; i < 100; i++)
800c02a0:	afc00010 	sw	zero,16(s8)
800c02a4:	080300ae 	j	800c02b8 <main+0xd0>
800c02a8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:76 (discriminator 3)
800c02ac:	8fc20010 	lw	v0,16(s8)
800c02b0:	24420001 	addiu	v0,v0,1
800c02b4:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:76 (discriminator 1)
800c02b8:	8fc20010 	lw	v0,16(s8)
800c02bc:	2c420064 	sltiu	v0,v0,100
800c02c0:	1440fffa 	bnez	v0,800c02ac <main+0xc4>
800c02c4:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:85

    
	#if hao
    bsp_gmac_disable();
	#endif
    rlx_icache_invalidate_all();	 
800c02c8:	0c030172 	jal	800c05c8 <rlx_icache_invalidate_all>
800c02cc:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:86
    rlx_dmem_enable(DMEM_BASE,DMEM_TOP);
800c02d0:	3c020007 	lui	v0,0x7
800c02d4:	3445ffff 	ori	a1,v0,0xffff
800c02d8:	3c040004 	lui	a0,0x4
800c02dc:	0c0301ac 	jal	800c06b0 <rlx_dmem_enable>
800c02e0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:89
	//Flash_Reset();
	//Set_SPIC_Read_one_channel_15M();	
	FLASH_ID = Flash_RDID(&flash_struct);
800c02e4:	3c02800c 	lui	v0,0x800c
800c02e8:	244418d4 	addiu	a0,v0,6356
800c02ec:	0c0303e8 	jal	800c0fa0 <Flash_RDID>
800c02f0:	00000000 	nop
800c02f4:	00401821 	move	v1,v0
800c02f8:	3c02800c 	lui	v0,0x800c
800c02fc:	ac4318d0 	sw	v1,6352(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:90
	rdid = (FLASH_ID  & 0x00ff0000) >> 16;
800c0300:	3c02800c 	lui	v0,0x800c
800c0304:	8c4318d0 	lw	v1,6352(v0)
800c0308:	3c0200ff 	lui	v0,0xff
800c030c:	00621024 	and	v0,v1,v0
800c0310:	00021402 	srl	v0,v0,0x10
800c0314:	afc2001c 	sw	v0,28(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:91
	WRSR_Flash_one_two_channel();	
800c0318:	0c030447 	jal	800c111c <WRSR_Flash_one_two_channel>
800c031c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:111
		return 0;
	#endif	
	#if 1
    do
    {
        if (rdid == 0x1f)
800c0320:	8fc3001c 	lw	v1,28(s8)
800c0324:	2402001f 	li	v0,31
800c0328:	14620008 	bne	v1,v0,800c034c <load_flash_addr>
800c032c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:112
            spi_se_unprotect(&flash_struct,(flash_addr & 0x00FFFFFF));
800c0330:	8fc30020 	lw	v1,32(s8)
800c0334:	3c0200ff 	lui	v0,0xff
800c0338:	3442ffff 	ori	v0,v0,0xffff
800c033c:	00621024 	and	v0,v1,v0
800c0340:	00402021 	move	a0,v0
800c0344:	0c0304a2 	jal	800c1288 <Flash_Unprotect>
800c0348:	00000000 	nop

800c034c <load_flash_addr>:
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:115

        asm volatile("load_flash_addr:");//gdb break point for gdb_flash.scr use
        spi_blk_erase(&flash_struct, (flash_addr & 0x00FFFFFF));
800c034c:	8fc30020 	lw	v1,32(s8)
800c0350:	3c0200ff 	lui	v0,0xff
800c0354:	3442ffff 	ori	v0,v0,0xffff
800c0358:	00621024 	and	v0,v1,v0
800c035c:	00402021 	move	a0,v0
800c0360:	0c030250 	jal	800c0940 <Flash_erase_block>
800c0364:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:116
        memset((void *) load_addr, 0xff, load_size);
800c0368:	8fc60028 	lw	a2,40(s8)
800c036c:	240500ff 	li	a1,255
800c0370:	8fc40024 	lw	a0,36(s8)
800c0374:	0c0304fe 	jal	800c13f8 <memset>
800c0378:	00000000 	nop

800c037c <load_memory>:
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:120


        asm volatile("load_memory:");
        for(i = 0 ; i < NUMLOOP/2 ; i++)
800c037c:	afc00010 	sw	zero,16(s8)
800c0380:	080300f5 	j	800c03d4 <load_memory+0x58>
800c0384:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:121 (discriminator 3)
            spi_write(&flash_struct, (flash_addr & 0x00FFFFFF)+i*WRSIZE, load_addr+i*WRSIZE, WRSIZE);
800c0388:	8fc30020 	lw	v1,32(s8)
800c038c:	3c0200ff 	lui	v0,0xff
800c0390:	3442ffff 	ori	v0,v0,0xffff
800c0394:	00621824 	and	v1,v1,v0
800c0398:	8fc20010 	lw	v0,16(s8)
800c039c:	000211c0 	sll	v0,v0,0x7
800c03a0:	00622021 	addu	a0,v1,v0
800c03a4:	8fc20010 	lw	v0,16(s8)
800c03a8:	000211c0 	sll	v0,v0,0x7
800c03ac:	8fc30024 	lw	v1,36(s8)
800c03b0:	00621021 	addu	v0,v1,v0
800c03b4:	00403021 	move	a2,v0
800c03b8:	00802821 	move	a1,a0
800c03bc:	24040080 	li	a0,128
800c03c0:	0c030302 	jal	800c0c08 <Flash_write_one_channel_User>
800c03c4:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:120 (discriminator 3)
        spi_blk_erase(&flash_struct, (flash_addr & 0x00FFFFFF));
        memset((void *) load_addr, 0xff, load_size);


        asm volatile("load_memory:");
        for(i = 0 ; i < NUMLOOP/2 ; i++)
800c03c8:	8fc20010 	lw	v0,16(s8)
800c03cc:	24420001 	addiu	v0,v0,1
800c03d0:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:120 (discriminator 1)
800c03d4:	8fc20010 	lw	v0,16(s8)
800c03d8:	2c420200 	sltiu	v0,v0,512
800c03dc:	1440ffea 	bnez	v0,800c0388 <load_memory+0xc>
800c03e0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:124
            spi_write(&flash_struct, (flash_addr & 0x00FFFFFF)+i*WRSIZE, load_addr+i*WRSIZE, WRSIZE);


        if(memcmp((void *) load_addr, (void *) (flash_addr | 0x20000000), load_size) != 0)
800c03e4:	8fc30020 	lw	v1,32(s8)
800c03e8:	3c022000 	lui	v0,0x2000
800c03ec:	00621025 	or	v0,v1,v0
800c03f0:	8fc60028 	lw	a2,40(s8)
800c03f4:	00402821 	move	a1,v0
800c03f8:	8fc40024 	lw	a0,36(s8)
800c03fc:	0c0304d6 	jal	800c1358 <memcmp>
800c0400:	00000000 	nop
800c0404:	10400003 	beqz	v0,800c0414 <compare_error+0x8>
800c0408:	00000000 	nop

800c040c <compare_error>:
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:127
        {
            asm volatile("compare_error:");
	    flag = 1;
800c040c:	24020001 	li	v0,1
800c0410:	a3c20014 	sb	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:130
        }

    } while(flag == 0);
800c0414:	93c20014 	lbu	v0,20(s8)
800c0418:	1040ffc1 	beqz	v0,800c0320 <main+0x138>
800c041c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:132
	#endif
    rlx_icache_invalidate_all();
800c0420:	0c030172 	jal	800c05c8 <rlx_icache_invalidate_all>
800c0424:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:133
    rlx_dcache_invalidate_all();
800c0428:	0c030160 	jal	800c0580 <rlx_dcache_invalidate_all>
800c042c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/app.c:135

}
800c0430:	03c0e821 	move	sp,s8
800c0434:	8fbf01c4 	lw	ra,452(sp)
800c0438:	8fbe01c0 	lw	s8,448(sp)
800c043c:	27bd01c8 	addiu	sp,sp,456
800c0440:	03e00008 	jr	ra
800c0444:	00000000 	nop

800c0448 <rlx_dcache_flush>:
rlx_dcache_flush():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:53
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_flush(INT32U address, INT32U size)
{
800c0448:	27bdfff0 	addiu	sp,sp,-16
800c044c:	afbe000c 	sw	s8,12(sp)
800c0450:	03a0f021 	move	s8,sp
800c0454:	afc40010 	sw	a0,16(s8)
800c0458:	afc50014 	sw	a1,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:56
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c045c:	8fc20010 	lw	v0,16(s8)
800c0460:	afc20000 	sw	v0,0(s8)
800c0464:	08030120 	j	800c0480 <rlx_dcache_flush+0x38>
800c0468:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:58 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
800c046c:	8fc20000 	lw	v0,0(s8)
800c0470:	bc550000 	cache	0x15,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:56 (discriminator 3)
void 
rlx_dcache_flush(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c0474:	8fc20000 	lw	v0,0(s8)
800c0478:	24420004 	addiu	v0,v0,4
800c047c:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:56 (discriminator 1)
800c0480:	8fc30010 	lw	v1,16(s8)
800c0484:	8fc20014 	lw	v0,20(s8)
800c0488:	00621821 	addu	v1,v1,v0
800c048c:	8fc20000 	lw	v0,0(s8)
800c0490:	0043102b 	sltu	v0,v0,v1
800c0494:	1440fff5 	bnez	v0,800c046c <rlx_dcache_flush+0x24>
800c0498:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:62
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WBINVAL), "r"(addr) );
    }
   
}
800c049c:	03c0e821 	move	sp,s8
800c04a0:	8fbe000c 	lw	s8,12(sp)
800c04a4:	27bd0010 	addiu	sp,sp,16
800c04a8:	03e00008 	jr	ra
800c04ac:	00000000 	nop

800c04b0 <rlx_dcache_invalidate>:
rlx_dcache_invalidate():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:78
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate(INT32U address, INT32U size)
{
800c04b0:	27bdfff0 	addiu	sp,sp,-16
800c04b4:	afbe000c 	sw	s8,12(sp)
800c04b8:	03a0f021 	move	s8,sp
800c04bc:	afc40010 	sw	a0,16(s8)
800c04c0:	afc50014 	sw	a1,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:81
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c04c4:	8fc20010 	lw	v0,16(s8)
800c04c8:	afc20000 	sw	v0,0(s8)
800c04cc:	0803013a 	j	800c04e8 <rlx_dcache_invalidate+0x38>
800c04d0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:83 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
800c04d4:	8fc20000 	lw	v0,0(s8)
800c04d8:	bc510000 	cache	0x11,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:81 (discriminator 3)
void 
rlx_dcache_invalidate(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c04dc:	8fc20000 	lw	v0,0(s8)
800c04e0:	24420004 	addiu	v0,v0,4
800c04e4:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:81 (discriminator 1)
800c04e8:	8fc30010 	lw	v1,16(s8)
800c04ec:	8fc20014 	lw	v0,20(s8)
800c04f0:	00621821 	addu	v1,v1,v0
800c04f4:	8fc20000 	lw	v0,0(s8)
800c04f8:	0043102b 	sltu	v0,v0,v1
800c04fc:	1440fff5 	bnez	v0,800c04d4 <rlx_dcache_invalidate+0x24>
800c0500:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:87
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_INVAL), "r"(addr) );
    }
   
}
800c0504:	03c0e821 	move	sp,s8
800c0508:	8fbe000c 	lw	s8,12(sp)
800c050c:	27bd0010 	addiu	sp,sp,16
800c0510:	03e00008 	jr	ra
800c0514:	00000000 	nop

800c0518 <rlx_dcache_store>:
rlx_dcache_store():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:104
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_store(INT32U address, INT32U size)
{
800c0518:	27bdfff0 	addiu	sp,sp,-16
800c051c:	afbe000c 	sw	s8,12(sp)
800c0520:	03a0f021 	move	s8,sp
800c0524:	afc40010 	sw	a0,16(s8)
800c0528:	afc50014 	sw	a1,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:107
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c052c:	8fc20010 	lw	v0,16(s8)
800c0530:	afc20000 	sw	v0,0(s8)
800c0534:	08030154 	j	800c0550 <rlx_dcache_store+0x38>
800c0538:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:109 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
800c053c:	8fc20000 	lw	v0,0(s8)
800c0540:	bc590000 	cache	0x19,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:107 (discriminator 3)
void 
rlx_dcache_store(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
800c0544:	8fc20000 	lw	v0,0(s8)
800c0548:	24420004 	addiu	v0,v0,4
800c054c:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:107 (discriminator 1)
800c0550:	8fc30010 	lw	v1,16(s8)
800c0554:	8fc20014 	lw	v0,20(s8)
800c0558:	00621821 	addu	v1,v1,v0
800c055c:	8fc20000 	lw	v0,0(s8)
800c0560:	0043102b 	sltu	v0,v0,v1
800c0564:	1440fff5 	bnez	v0,800c053c <rlx_dcache_store+0x24>
800c0568:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:113
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WB), "r"(addr) );
    }
   
}
800c056c:	03c0e821 	move	sp,s8
800c0570:	8fbe000c 	lw	s8,12(sp)
800c0574:	27bd0010 	addiu	sp,sp,16
800c0578:	03e00008 	jr	ra
800c057c:	00000000 	nop

800c0580 <rlx_dcache_invalidate_all>:
rlx_dcache_invalidate_all():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:129
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate_all(void)
{
800c0580:	27bdfff0 	addiu	sp,sp,-16
800c0584:	afbe000c 	sw	s8,12(sp)
800c0588:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:130
  INT32U  cctl_val = 0;
800c058c:	afc00000 	sw	zero,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:132
  
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
800c0590:	8fc20000 	lw	v0,0(s8)
800c0594:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:133
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
800c0598:	4002a000 	mfc0	v0,$20
800c059c:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:134
  cctl_val |= CP0_CCTL_DINVAL;
800c05a0:	8fc20000 	lw	v0,0(s8)
800c05a4:	34420001 	ori	v0,v0,0x1
800c05a8:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:135
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
800c05ac:	8fc20000 	lw	v0,0(s8)
800c05b0:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:137

}
800c05b4:	03c0e821 	move	sp,s8
800c05b8:	8fbe000c 	lw	s8,12(sp)
800c05bc:	27bd0010 	addiu	sp,sp,16
800c05c0:	03e00008 	jr	ra
800c05c4:	00000000 	nop

800c05c8 <rlx_icache_invalidate_all>:
rlx_icache_invalidate_all():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:153
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_icache_invalidate_all(void)
{
800c05c8:	27bdfff0 	addiu	sp,sp,-16
800c05cc:	afbe000c 	sw	s8,12(sp)
800c05d0:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:154
  INT32U  cctl_val = 0;
800c05d4:	afc00000 	sw	zero,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:156

  asm volatile("mtc0   %z0, $20"  :  : "Jr" ((INT32U)(cctl_val))); 
800c05d8:	8fc20000 	lw	v0,0(s8)
800c05dc:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:157
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
800c05e0:	4002a000 	mfc0	v0,$20
800c05e4:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:158
  cctl_val |= CP0_CCTL_IINVAL;
800c05e8:	8fc20000 	lw	v0,0(s8)
800c05ec:	34420002 	ori	v0,v0,0x2
800c05f0:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:159
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
800c05f4:	8fc20000 	lw	v0,0(s8)
800c05f8:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:160
}
800c05fc:	03c0e821 	move	sp,s8
800c0600:	8fbe000c 	lw	s8,12(sp)
800c0604:	27bd0010 	addiu	sp,sp,16
800c0608:	03e00008 	jr	ra
800c060c:	00000000 	nop

800c0610 <rlx_icache_lock>:
rlx_icache_lock():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:178
 * Returns    : BOOLEAN -- OS_TRUE if paramter is valid, and OS_FALSE otherwise
 *****************************************************************************************
 */
BOOLEAN 
rlx_icache_lock(INT32U mode)
{
800c0610:	27bdfff0 	addiu	sp,sp,-16
800c0614:	afbe000c 	sw	s8,12(sp)
800c0618:	03a0f021 	move	s8,sp
800c061c:	afc40010 	sw	a0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:180
  INT32U  cctl_val;
  BOOLEAN mode_valid = OS_FALSE;  
800c0620:	a3c00000 	sb	zero,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:182

  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
800c0624:	4002a000 	mfc0	v0,$20
800c0628:	afc20004 	sw	v0,4(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:184

  cctl_val &= ~CP0_CCTL_ILOCK_MASK;
800c062c:	8fc30004 	lw	v1,4(s8)
800c0630:	2402fff3 	li	v0,-13
800c0634:	00621024 	and	v0,v1,v0
800c0638:	afc20004 	sw	v0,4(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:186

  switch(mode)
800c063c:	8fc20010 	lw	v0,16(s8)
800c0640:	10400005 	beqz	v0,800c0658 <rlx_icache_lock+0x48>
800c0644:	00000000 	nop
800c0648:	2442fffe 	addiu	v0,v0,-2
800c064c:	2c420002 	sltiu	v0,v0,2
800c0650:	10400005 	beqz	v0,800c0668 <rlx_icache_lock+0x58>
800c0654:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:191
    {
    case CP0_CCTL_ILOCK_NORMAL: 
    case CP0_CCTL_ILOCK_LOCKGATHER: 
    case CP0_CCTL_ILOCK_LOCKDOWN: 
      mode_valid = OS_TRUE;
800c0658:	24020001 	li	v0,1
800c065c:	a3c20000 	sb	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:192
      break;
800c0660:	0803019b 	j	800c066c <rlx_icache_lock+0x5c>
800c0664:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:195

    default:
      break;
800c0668:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:199
    }

  /* valid setting, write to CP0_CCTL */    
  if (mode_valid == OS_TRUE)
800c066c:	93c30000 	lbu	v1,0(s8)
800c0670:	24020001 	li	v0,1
800c0674:	14620008 	bne	v1,v0,800c0698 <rlx_icache_lock+0x88>
800c0678:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:201
    {
      cctl_val |= (mode << CP0_CCTL_ILOCK_OFFSET);
800c067c:	8fc20010 	lw	v0,16(s8)
800c0680:	00021080 	sll	v0,v0,0x2
800c0684:	8fc30004 	lw	v1,4(s8)
800c0688:	00621025 	or	v0,v1,v0
800c068c:	afc20004 	sw	v0,4(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:202
      asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
800c0690:	8fc20004 	lw	v0,4(s8)
800c0694:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:205
    }
  
  return (mode_valid);
800c0698:	93c20000 	lbu	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:206
}
800c069c:	03c0e821 	move	sp,s8
800c06a0:	8fbe000c 	lw	s8,12(sp)
800c06a4:	27bd0010 	addiu	sp,sp,16
800c06a8:	03e00008 	jr	ra
800c06ac:	00000000 	nop

800c06b0 <rlx_dmem_enable>:
rlx_dmem_enable():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:223
 *******************************************************************************
 */

void
rlx_dmem_enable(INT32U base, INT32U top)
{
800c06b0:	27bdfff0 	addiu	sp,sp,-16
800c06b4:	afbe000c 	sw	s8,12(sp)
800c06b8:	03a0f021 	move	s8,sp
800c06bc:	afc40010 	sw	a0,16(s8)
800c06c0:	afc50014 	sw	a1,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:225
    INT32U cctl_dmem;
    asm volatile("mtc3   %z0, $4"  : : "Jr" ((INT32U)(base)));	
800c06c4:	8fc20010 	lw	v0,16(s8)
800c06c8:	4c822000 	mtc3	v0,$4
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:226
    asm volatile("mtc3   %z0, $5"  : : "Jr" ((INT32U)(top)));
800c06cc:	8fc20014 	lw	v0,20(s8)
800c06d0:	4c822800 	mtc3	v0,$5
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:228
    
    asm volatile("mfc0   %0, $20"   : "=r"(cctl_dmem));
800c06d4:	4002a000 	mfc0	v0,$20
800c06d8:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:229
    cctl_dmem |= CP0_CCTL_DMEMON;
800c06dc:	8fc20000 	lw	v0,0(s8)
800c06e0:	34420400 	ori	v0,v0,0x400
800c06e4:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:230
    asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_dmem)));
800c06e8:	8fc20000 	lw	v0,0(s8)
800c06ec:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:231
}
800c06f0:	03c0e821 	move	sp,s8
800c06f4:	8fbe000c 	lw	s8,12(sp)
800c06f8:	27bd0010 	addiu	sp,sp,16
800c06fc:	03e00008 	jr	ra
800c0700:	00000000 	nop

800c0704 <rlx_imem_refill>:
rlx_imem_refill():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:248
 *
 * Returns    : None
 *******************************************************************************
 */
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode)
{
800c0704:	27bdfff0 	addiu	sp,sp,-16
800c0708:	afbe000c 	sw	s8,12(sp)
800c070c:	03a0f021 	move	s8,sp
800c0710:	afc40010 	sw	a0,16(s8)
800c0714:	afc50014 	sw	a1,20(s8)
800c0718:	afc60018 	sw	a2,24(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:250
    INT32U cctl_imem;
    asm volatile("mtc3   %z0, $0"  : : "Jr" ((INT32U)(base)));	
800c071c:	8fc20010 	lw	v0,16(s8)
800c0720:	4c820000 	mtc3	v0,$0
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:251
    asm volatile("mtc3   %z0, $1"  : : "Jr" ((INT32U)(top)));
800c0724:	8fc20014 	lw	v0,20(s8)
800c0728:	4c820800 	mtc3	v0,$1
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:253

    if(mode == 1)
800c072c:	8fc30018 	lw	v1,24(s8)
800c0730:	24020001 	li	v0,1
800c0734:	1462000e 	bne	v1,v0,800c0770 <rlx_imem_refill+0x6c>
800c0738:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:255
    {
        asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
800c073c:	4002a000 	mfc0	v0,$20
800c0740:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:256
        cctl_imem &=  ~CP0_CCTL_IMEMFILL;
800c0744:	8fc30000 	lw	v1,0(s8)
800c0748:	2402ffef 	li	v0,-17
800c074c:	00621024 	and	v0,v1,v0
800c0750:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:257
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
800c0754:	8fc20000 	lw	v0,0(s8)
800c0758:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:259

        cctl_imem |=  CP0_CCTL_IMEMFILL;
800c075c:	8fc20000 	lw	v0,0(s8)
800c0760:	34420010 	ori	v0,v0,0x10
800c0764:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:260
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
800c0768:	8fc20000 	lw	v0,0(s8)
800c076c:	4082a000 	mtc0	v0,$20
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/rlx_cache.c:262
    }
}
800c0770:	03c0e821 	move	sp,s8
800c0774:	8fbe000c 	lw	s8,12(sp)
800c0778:	27bd0010 	addiu	sp,sp,16
800c077c:	03e00008 	jr	ra
800c0780:	00000000 	nop

800c0784 <Check_SPIC_Busy>:
Check_SPIC_Busy():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:16

extern char char_temp[20];
SPI_FLASH_STRUCT flash_struct;

void Check_SPIC_Busy(void)
{
800c0784:	27bdfff0 	addiu	sp,sp,-16
800c0788:	afbe000c 	sw	s8,12(sp)
800c078c:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:20 (discriminator 1)
	INT32U   spic_busy;

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
800c0790:	3c02bc00 	lui	v0,0xbc00
800c0794:	34420028 	ori	v0,v0,0x28
800c0798:	8c420000 	lw	v0,0(v0)
800c079c:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:21 (discriminator 1)
	}while(spic_busy!=0x06);
800c07a0:	8fc30000 	lw	v1,0(s8)
800c07a4:	24020006 	li	v0,6
800c07a8:	1462fff9 	bne	v1,v0,800c0790 <Check_SPIC_Busy+0xc>
800c07ac:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:22
}
800c07b0:	03c0e821 	move	sp,s8
800c07b4:	8fbe000c 	lw	s8,12(sp)
800c07b8:	27bd0010 	addiu	sp,sp,16
800c07bc:	03e00008 	jr	ra
800c07c0:	00000000 	nop

800c07c4 <Flash_RDSR>:
Flash_RDSR():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:26


void Flash_RDSR(void )
{
800c07c4:	27bdfff0 	addiu	sp,sp,-16
800c07c8:	afbe000c 	sw	s8,12(sp)
800c07cc:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:29
	INT8U   spic_busy;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c07d0:	3c02bc00 	lui	v0,0xbc00
800c07d4:	34420008 	ori	v0,v0,0x8
800c07d8:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:30
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
800c07dc:	3c02bc00 	lui	v0,0xbc00
800c07e0:	24030300 	li	v1,768
800c07e4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:31
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
800c07e8:	3c02bc00 	lui	v0,0xbc00
800c07ec:	34420010 	ori	v0,v0,0x10
800c07f0:	24030001 	li	v1,1
800c07f4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:32
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x01;//read one byte back
800c07f8:	3c02bc00 	lui	v0,0xbc00
800c07fc:	34420004 	ori	v0,v0,0x4
800c0800:	24030001 	li	v1,1
800c0804:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:33
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c0808:	3c02bc00 	lui	v0,0xbc00
800c080c:	34420008 	ori	v0,v0,0x8
800c0810:	24030001 	li	v1,1
800c0814:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:35

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDSR_COM;//RDSR
800c0818:	3c02bc00 	lui	v0,0xbc00
800c081c:	34420060 	ori	v0,v0,0x60
800c0820:	24030005 	li	v1,5
800c0824:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:39 (discriminator 1)
	
	//check spic busy?
	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
800c0828:	3c02bc00 	lui	v0,0xbc00
800c082c:	34420028 	ori	v0,v0,0x28
800c0830:	8c420000 	lw	v0,0(v0)
800c0834:	a3c20000 	sb	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:40 (discriminator 1)
		spic_busy=spic_busy & 0x01;
800c0838:	93c20000 	lbu	v0,0(s8)
800c083c:	30420001 	andi	v0,v0,0x1
800c0840:	a3c20000 	sb	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:41 (discriminator 1)
	}while(spic_busy!=0x00);
800c0844:	93c20000 	lbu	v0,0(s8)
800c0848:	1440fff7 	bnez	v0,800c0828 <Flash_RDSR+0x64>
800c084c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:42
}
800c0850:	03c0e821 	move	sp,s8
800c0854:	8fbe000c 	lw	s8,12(sp)
800c0858:	27bd0010 	addiu	sp,sp,16
800c085c:	03e00008 	jr	ra
800c0860:	00000000 	nop

800c0864 <Flash_erase_all>:
Flash_erase_all():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:47


/*erase chip*/
void Flash_erase_all()//Kaiwen
{
800c0864:	27bdffe0 	addiu	sp,sp,-32
800c0868:	afbf001c 	sw	ra,28(sp)
800c086c:	afbe0018 	sw	s8,24(sp)
800c0870:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:50
	INT8U flash_busy;
	
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0874:	3c02bc00 	lui	v0,0xbc00
800c0878:	34420008 	ori	v0,v0,0x8
800c087c:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:51
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
800c0880:	3c02bc00 	lui	v0,0xbc00
800c0884:	34420118 	ori	v0,v0,0x118
800c0888:	24030001 	li	v1,1
800c088c:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:52
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
800c0890:	3c02bc00 	lui	v0,0xbc00
800c0894:	34420010 	ori	v0,v0,0x10
800c0898:	24030001 	li	v1,1
800c089c:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:53
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
800c08a0:	3c02bc00 	lui	v0,0xbc00
800c08a4:	34420004 	ori	v0,v0,0x4
800c08a8:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:54
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
800c08ac:	3c02bc00 	lui	v0,0xbc00
800c08b0:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:55
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c08b4:	3c02bc00 	lui	v0,0xbc00
800c08b8:	34420008 	ori	v0,v0,0x8
800c08bc:	24030001 	li	v1,1
800c08c0:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:57

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
800c08c4:	3c02bc00 	lui	v0,0xbc00
800c08c8:	34420060 	ori	v0,v0,0x60
800c08cc:	24030006 	li	v1,6
800c08d0:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:58
	Check_SPIC_Busy();
800c08d4:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c08d8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:60

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_CHIP_ERA;//Chip erase	
800c08dc:	3c02bc00 	lui	v0,0xbc00
800c08e0:	34420060 	ori	v0,v0,0x60
800c08e4:	24030060 	li	v1,96
800c08e8:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:61
	Check_SPIC_Busy();
800c08ec:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c08f0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:63 (discriminator 1)
	do{
		Flash_RDSR();	
800c08f4:	0c0301f1 	jal	800c07c4 <Flash_RDSR>
800c08f8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:65 (discriminator 1)
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
800c08fc:	3c02bc00 	lui	v0,0xbc00
800c0900:	34420060 	ori	v0,v0,0x60
800c0904:	90420000 	lbu	v0,0(v0)
800c0908:	a3c20010 	sb	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:66 (discriminator 1)
		flash_busy=flash_busy & 0x03;
800c090c:	93c20010 	lbu	v0,16(s8)
800c0910:	30420003 	andi	v0,v0,0x3
800c0914:	a3c20010 	sb	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:67 (discriminator 1)
	}while(flash_busy==0x03);
800c0918:	93c30010 	lbu	v1,16(s8)
800c091c:	24020003 	li	v0,3
800c0920:	1062fff4 	beq	v1,v0,800c08f4 <Flash_erase_all+0x90>
800c0924:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:68
 }
800c0928:	03c0e821 	move	sp,s8
800c092c:	8fbf001c 	lw	ra,28(sp)
800c0930:	8fbe0018 	lw	s8,24(sp)
800c0934:	27bd0020 	addiu	sp,sp,32
800c0938:	03e00008 	jr	ra
800c093c:	00000000 	nop

800c0940 <Flash_erase_block>:
Flash_erase_block():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:72

//erase block 
void Flash_erase_block(INT32U Address)
{
800c0940:	27bdffe0 	addiu	sp,sp,-32
800c0944:	afbf001c 	sw	ra,28(sp)
800c0948:	afbe0018 	sw	s8,24(sp)
800c094c:	03a0f021 	move	s8,sp
800c0950:	afc40020 	sw	a0,32(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:77
	INT32U   DWtmp;
	INT8U 	 flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0954:	3c02bc00 	lui	v0,0xbc00
800c0958:	34420008 	ori	v0,v0,0x8
800c095c:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:78
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=3;//SPI Address byte in user mode
800c0960:	3c02bc00 	lui	v0,0xbc00
800c0964:	34420118 	ori	v0,v0,0x118
800c0968:	24030003 	li	v1,3
800c096c:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:79
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
800c0970:	3c02bc00 	lui	v0,0xbc00
800c0974:	34420010 	ori	v0,v0,0x10
800c0978:	24030001 	li	v1,1
800c097c:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:80
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
800c0980:	3c02bc00 	lui	v0,0xbc00
800c0984:	34420004 	ori	v0,v0,0x4
800c0988:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:81
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
800c098c:	3c02bc00 	lui	v0,0xbc00
800c0990:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:82
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	
800c0994:	3c02bc00 	lui	v0,0xbc00
800c0998:	34420008 	ori	v0,v0,0x8
800c099c:	24030001 	li	v1,1
800c09a0:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:85

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
800c09a4:	3c02bc00 	lui	v0,0xbc00
800c09a8:	34420060 	ori	v0,v0,0x60
800c09ac:	24030006 	li	v1,6
800c09b0:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:86
	Check_SPIC_Busy();
800c09b4:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c09b8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:88

	DWtmp = Address >> 16;
800c09bc:	8fc20020 	lw	v0,32(s8)
800c09c0:	00021402 	srl	v0,v0,0x10
800c09c4:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:89
	DWtmp = DWtmp + (Address & 0x0000ff00);
800c09c8:	8fc20020 	lw	v0,32(s8)
800c09cc:	3042ff00 	andi	v0,v0,0xff00
800c09d0:	8fc30010 	lw	v1,16(s8)
800c09d4:	00621021 	addu	v0,v1,v0
800c09d8:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:90
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
800c09dc:	8fc20020 	lw	v0,32(s8)
800c09e0:	00021c00 	sll	v1,v0,0x10
800c09e4:	3c0200ff 	lui	v0,0xff
800c09e8:	00621024 	and	v0,v1,v0
800c09ec:	8fc30010 	lw	v1,16(s8)
800c09f0:	00621021 	addu	v0,v1,v0
800c09f4:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:91
	DWtmp = (DWtmp << 8 )+ FLASH_BE_COM;
800c09f8:	8fc20010 	lw	v0,16(s8)
800c09fc:	00021200 	sll	v0,v0,0x8
800c0a00:	244200d8 	addiu	v0,v0,216
800c0a04:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:92
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
800c0a08:	3c02bc00 	lui	v0,0xbc00
800c0a0c:	34420060 	ori	v0,v0,0x60
800c0a10:	8fc30010 	lw	v1,16(s8)
800c0a14:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:95

	
	Check_SPIC_Busy();	
800c0a18:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0a1c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:98 (discriminator 1)

	do{
		Flash_RDSR();	
800c0a20:	0c0301f1 	jal	800c07c4 <Flash_RDSR>
800c0a24:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:100 (discriminator 1)
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
800c0a28:	3c02bc00 	lui	v0,0xbc00
800c0a2c:	34420060 	ori	v0,v0,0x60
800c0a30:	90420000 	lbu	v0,0(v0)
800c0a34:	a3c20014 	sb	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:101 (discriminator 1)
		flash_busy=flash_busy & 0x03;
800c0a38:	93c20014 	lbu	v0,20(s8)
800c0a3c:	30420003 	andi	v0,v0,0x3
800c0a40:	a3c20014 	sb	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:102 (discriminator 1)
	}while(flash_busy==0x03);
800c0a44:	93c30014 	lbu	v1,20(s8)
800c0a48:	24020003 	li	v0,3
800c0a4c:	1062fff4 	beq	v1,v0,800c0a20 <Flash_erase_block+0xe0>
800c0a50:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:105

	
}
800c0a54:	03c0e821 	move	sp,s8
800c0a58:	8fbf001c 	lw	ra,28(sp)
800c0a5c:	8fbe0018 	lw	s8,24(sp)
800c0a60:	27bd0020 	addiu	sp,sp,32
800c0a64:	03e00008 	jr	ra
800c0a68:	00000000 	nop

800c0a6c <Flash_erase_sector>:
Flash_erase_sector():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:109

//erase sector 
void Flash_erase_sector(INT32U Address)
{
800c0a6c:	27bdffe0 	addiu	sp,sp,-32
800c0a70:	afbf001c 	sw	ra,28(sp)
800c0a74:	afbe0018 	sw	s8,24(sp)
800c0a78:	03a0f021 	move	s8,sp
800c0a7c:	afc40020 	sw	a0,32(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:114
	INT32U    DWtmp;
	INT8U     flash_busy;
	
	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0a80:	3c02bc00 	lui	v0,0xbc00
800c0a84:	34420008 	ori	v0,v0,0x8
800c0a88:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:115
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=3;//SPI Address byte in user mode
800c0a8c:	3c02bc00 	lui	v0,0xbc00
800c0a90:	34420118 	ori	v0,v0,0x118
800c0a94:	24030003 	li	v1,3
800c0a98:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:116
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
800c0a9c:	3c02bc00 	lui	v0,0xbc00
800c0aa0:	34420010 	ori	v0,v0,0x10
800c0aa4:	24030001 	li	v1,1
800c0aa8:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:117
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
800c0aac:	3c02bc00 	lui	v0,0xbc00
800c0ab0:	34420004 	ori	v0,v0,0x4
800c0ab4:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:118
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode 
800c0ab8:	3c02bc00 	lui	v0,0xbc00
800c0abc:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:119
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	
800c0ac0:	3c02bc00 	lui	v0,0xbc00
800c0ac4:	34420008 	ori	v0,v0,0x8
800c0ac8:	24030001 	li	v1,1
800c0acc:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:122

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
800c0ad0:	3c02bc00 	lui	v0,0xbc00
800c0ad4:	34420060 	ori	v0,v0,0x60
800c0ad8:	24030006 	li	v1,6
800c0adc:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:123
	Check_SPIC_Busy();
800c0ae0:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0ae4:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:125

	DWtmp = Address >> 16;
800c0ae8:	8fc20020 	lw	v0,32(s8)
800c0aec:	00021402 	srl	v0,v0,0x10
800c0af0:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:126
	DWtmp = DWtmp + (Address & 0x0000ff00);
800c0af4:	8fc20020 	lw	v0,32(s8)
800c0af8:	3042ff00 	andi	v0,v0,0xff00
800c0afc:	8fc30010 	lw	v1,16(s8)
800c0b00:	00621021 	addu	v0,v1,v0
800c0b04:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:127
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
800c0b08:	8fc20020 	lw	v0,32(s8)
800c0b0c:	00021c00 	sll	v1,v0,0x10
800c0b10:	3c0200ff 	lui	v0,0xff
800c0b14:	00621024 	and	v0,v1,v0
800c0b18:	8fc30010 	lw	v1,16(s8)
800c0b1c:	00621021 	addu	v0,v1,v0
800c0b20:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:128
	DWtmp = (DWtmp << 8 )+ FLASH_CHIP_SEC;
800c0b24:	8fc20010 	lw	v0,16(s8)
800c0b28:	00021200 	sll	v0,v0,0x8
800c0b2c:	24420020 	addiu	v0,v0,32
800c0b30:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:129
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
800c0b34:	3c02bc00 	lui	v0,0xbc00
800c0b38:	34420060 	ori	v0,v0,0x60
800c0b3c:	8fc30010 	lw	v1,16(s8)
800c0b40:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:132
	
	
	Check_SPIC_Busy();	
800c0b44:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0b48:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:135 (discriminator 1)

	do{
		Flash_RDSR();
800c0b4c:	0c0301f1 	jal	800c07c4 <Flash_RDSR>
800c0b50:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:137 (discriminator 1)
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
800c0b54:	3c02bc00 	lui	v0,0xbc00
800c0b58:	34420060 	ori	v0,v0,0x60
800c0b5c:	90420000 	lbu	v0,0(v0)
800c0b60:	a3c20014 	sb	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:138 (discriminator 1)
		flash_busy=flash_busy & 0x03;
800c0b64:	93c20014 	lbu	v0,20(s8)
800c0b68:	30420003 	andi	v0,v0,0x3
800c0b6c:	a3c20014 	sb	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:139 (discriminator 1)
	}while(flash_busy==0x03);
800c0b70:	93c30014 	lbu	v1,20(s8)
800c0b74:	24020003 	li	v0,3
800c0b78:	1062fff4 	beq	v1,v0,800c0b4c <Flash_erase_sector+0xe0>
800c0b7c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:142


}
800c0b80:	03c0e821 	move	sp,s8
800c0b84:	8fbf001c 	lw	ra,28(sp)
800c0b88:	8fbe0018 	lw	s8,24(sp)
800c0b8c:	27bd0020 	addiu	sp,sp,32
800c0b90:	03e00008 	jr	ra
800c0b94:	00000000 	nop

800c0b98 <Set_SPIC_Write_one_channel>:
Set_SPIC_Write_one_channel():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:146

//set  write auto mode 
void Set_SPIC_Write_one_channel(void)//Kaiwen
{
800c0b98:	27bdfff8 	addiu	sp,sp,-8
800c0b9c:	afbe0004 	sw	s8,4(sp)
800c0ba0:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:147
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0ba4:	3c02bc00 	lui	v0,0xbc00
800c0ba8:	34420008 	ori	v0,v0,0x8
800c0bac:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:148
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//TX mode
800c0bb0:	3c02bc00 	lui	v0,0xbc00
800c0bb4:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:149
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read data back
800c0bb8:	3c02bc00 	lui	v0,0xbc00
800c0bbc:	34420004 	ori	v0,v0,0x4
800c0bc0:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:150
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x03;//SPI Address byte =3 in user mode
800c0bc4:	3c02bc00 	lui	v0,0xbc00
800c0bc8:	34420118 	ori	v0,v0,0x118
800c0bcc:	24030003 	li	v1,3
800c0bd0:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:151
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
800c0bd4:	3c02bc00 	lui	v0,0xbc00
800c0bd8:	34420120 	ori	v0,v0,0x120
800c0bdc:	24030200 	li	v1,512
800c0be0:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:152
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC		
800c0be4:	3c02bc00 	lui	v0,0xbc00
800c0be8:	34420008 	ori	v0,v0,0x8
800c0bec:	24030001 	li	v1,1
800c0bf0:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:153
}
800c0bf4:	03c0e821 	move	sp,s8
800c0bf8:	8fbe0004 	lw	s8,4(sp)
800c0bfc:	27bd0008 	addiu	sp,sp,8
800c0c00:	03e00008 	jr	ra
800c0c04:	00000000 	nop

800c0c08 <Flash_write_one_channel_User>:
Flash_write_one_channel_User():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:168
	DReadBuffer: write buffer's point
*Note:ex,when write flash men address 0x82000000,then ,address is 0x00;
	the max  NDF is 128 bytes !!!
**********************************************************/
void Flash_write_one_channel_User(INT32U NDF, INT32U Address, INT32U *DReadBuffer)
{	
800c0c08:	27bdffd0 	addiu	sp,sp,-48
800c0c0c:	afbf002c 	sw	ra,44(sp)
800c0c10:	afbe0028 	sw	s8,40(sp)
800c0c14:	03a0f021 	move	s8,sp
800c0c18:	afc40030 	sw	a0,48(s8)
800c0c1c:	afc50034 	sw	a1,52(s8)
800c0c20:	afc60038 	sw	a2,56(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:172
	INT32U    DWtmp, i;
	INT8U     flash_busy;
	INT8U    *BReadBuffer;
    int b  = (((INT32U)DReadBuffer)%4);	
800c0c24:	8fc20038 	lw	v0,56(s8)
800c0c28:	30420003 	andi	v0,v0,0x3
800c0c2c:	afc20014 	sw	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:174
			
	Set_SPIC_Write_one_channel();
800c0c30:	0c0302e6 	jal	800c0b98 <Set_SPIC_Write_one_channel>
800c0c34:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:176
	
	BReadBuffer=DReadBuffer;
800c0c38:	8fc20038 	lw	v0,56(s8)
800c0c3c:	afc20018 	sw	v0,24(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:178

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;//Write Enable
800c0c40:	3c02bc00 	lui	v0,0xbc00
800c0c44:	34420060 	ori	v0,v0,0x60
800c0c48:	24030006 	li	v1,6
800c0c4c:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:179
	Check_SPIC_Busy();
800c0c50:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0c54:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:181

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC	
800c0c58:	3c02bc00 	lui	v0,0xbc00
800c0c5c:	34420008 	ori	v0,v0,0x8
800c0c60:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:183

	DWtmp = Address >> 16;
800c0c64:	8fc20034 	lw	v0,52(s8)
800c0c68:	00021402 	srl	v0,v0,0x10
800c0c6c:	afc2001c 	sw	v0,28(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:184
	DWtmp = DWtmp + (Address & 0x0000ff00);
800c0c70:	8fc20034 	lw	v0,52(s8)
800c0c74:	3042ff00 	andi	v0,v0,0xff00
800c0c78:	8fc3001c 	lw	v1,28(s8)
800c0c7c:	00621021 	addu	v0,v1,v0
800c0c80:	afc2001c 	sw	v0,28(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:185
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
800c0c84:	8fc20034 	lw	v0,52(s8)
800c0c88:	00021c00 	sll	v1,v0,0x10
800c0c8c:	3c0200ff 	lui	v0,0xff
800c0c90:	00621024 	and	v0,v1,v0
800c0c94:	8fc3001c 	lw	v1,28(s8)
800c0c98:	00621021 	addu	v0,v1,v0
800c0c9c:	afc2001c 	sw	v0,28(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:186
	DWtmp = (DWtmp << 8 )+ FLASH_PP_COM;
800c0ca0:	8fc2001c 	lw	v0,28(s8)
800c0ca4:	00021200 	sll	v0,v0,0x8
800c0ca8:	24420002 	addiu	v0,v0,2
800c0cac:	afc2001c 	sw	v0,28(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:187
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address
800c0cb0:	3c02bc00 	lui	v0,0xbc00
800c0cb4:	34420060 	ori	v0,v0,0x60
800c0cb8:	8fc3001c 	lw	v1,28(s8)
800c0cbc:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:190

	
	for(i=0;i<NDF/4;i++)
800c0cc0:	afc00010 	sw	zero,16(s8)
800c0cc4:	08030366 	j	800c0d98 <Flash_write_one_channel_User+0x190>
800c0cc8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:192
	{
		if(b)
800c0ccc:	8fc20014 	lw	v0,20(s8)
800c0cd0:	10400026 	beqz	v0,800c0d6c <Flash_write_one_channel_User+0x164>
800c0cd4:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:194
		{
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4);
800c0cd8:	3c02bc00 	lui	v0,0xbc00
800c0cdc:	34420060 	ori	v0,v0,0x60
800c0ce0:	8fc30010 	lw	v1,16(s8)
800c0ce4:	00031880 	sll	v1,v1,0x2
800c0ce8:	8fc40018 	lw	a0,24(s8)
800c0cec:	00831821 	addu	v1,a0,v1
800c0cf0:	90630000 	lbu	v1,0(v1)
800c0cf4:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:195
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+1);
800c0cf8:	3c02bc00 	lui	v0,0xbc00
800c0cfc:	34420060 	ori	v0,v0,0x60
800c0d00:	8fc30010 	lw	v1,16(s8)
800c0d04:	00031880 	sll	v1,v1,0x2
800c0d08:	24630001 	addiu	v1,v1,1
800c0d0c:	8fc40018 	lw	a0,24(s8)
800c0d10:	00831821 	addu	v1,a0,v1
800c0d14:	90630000 	lbu	v1,0(v1)
800c0d18:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:196
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+2);
800c0d1c:	3c02bc00 	lui	v0,0xbc00
800c0d20:	34420060 	ori	v0,v0,0x60
800c0d24:	8fc30010 	lw	v1,16(s8)
800c0d28:	00031880 	sll	v1,v1,0x2
800c0d2c:	24630002 	addiu	v1,v1,2
800c0d30:	8fc40018 	lw	a0,24(s8)
800c0d34:	00831821 	addu	v1,a0,v1
800c0d38:	90630000 	lbu	v1,0(v1)
800c0d3c:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:197
			REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+i*4+3);	
800c0d40:	3c02bc00 	lui	v0,0xbc00
800c0d44:	34420060 	ori	v0,v0,0x60
800c0d48:	8fc30010 	lw	v1,16(s8)
800c0d4c:	00031880 	sll	v1,v1,0x2
800c0d50:	24630003 	addiu	v1,v1,3
800c0d54:	8fc40018 	lw	a0,24(s8)
800c0d58:	00831821 	addu	v1,a0,v1
800c0d5c:	90630000 	lbu	v1,0(v1)
800c0d60:	a0430000 	sb	v1,0(v0)
800c0d64:	08030363 	j	800c0d8c <Flash_write_one_channel_User+0x184>
800c0d68:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:201
		}
		else
		{
			REG32(FLASH_BASE_ADDR+FLASH_DR)=*(DReadBuffer+i);		
800c0d6c:	3c02bc00 	lui	v0,0xbc00
800c0d70:	34420060 	ori	v0,v0,0x60
800c0d74:	8fc30010 	lw	v1,16(s8)
800c0d78:	00031880 	sll	v1,v1,0x2
800c0d7c:	8fc40038 	lw	a0,56(s8)
800c0d80:	00831821 	addu	v1,a0,v1
800c0d84:	8c630000 	lw	v1,0(v1)
800c0d88:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:190 (discriminator 2)
	DWtmp = DWtmp + ((Address << 16) & 0x00ff0000);
	DWtmp = (DWtmp << 8 )+ FLASH_PP_COM;
	REG32(FLASH_BASE_ADDR+FLASH_DR)=DWtmp;//Write Command and Address

	
	for(i=0;i<NDF/4;i++)
800c0d8c:	8fc20010 	lw	v0,16(s8)
800c0d90:	24420001 	addiu	v0,v0,1
800c0d94:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:190 (discriminator 1)
800c0d98:	8fc20030 	lw	v0,48(s8)
800c0d9c:	00021882 	srl	v1,v0,0x2
800c0da0:	8fc20010 	lw	v0,16(s8)
800c0da4:	0043102b 	sltu	v0,v0,v1
800c0da8:	1440ffc8 	bnez	v0,800c0ccc <Flash_write_one_channel_User+0xc4>
800c0dac:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:204
		else
		{
			REG32(FLASH_BASE_ADDR+FLASH_DR)=*(DReadBuffer+i);		
		}
	}
	if((NDF%4)==1)
800c0db0:	8fc20030 	lw	v0,48(s8)
800c0db4:	30430003 	andi	v1,v0,0x3
800c0db8:	24020001 	li	v0,1
800c0dbc:	1462000b 	bne	v1,v0,800c0dec <Flash_write_one_channel_User+0x1e4>
800c0dc0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:205
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
800c0dc4:	3c02bc00 	lui	v0,0xbc00
800c0dc8:	34420060 	ori	v0,v0,0x60
800c0dcc:	8fc30030 	lw	v1,48(s8)
800c0dd0:	2463ffff 	addiu	v1,v1,-1
800c0dd4:	8fc40018 	lw	a0,24(s8)
800c0dd8:	00831821 	addu	v1,a0,v1
800c0ddc:	90630000 	lbu	v1,0(v1)
800c0de0:	a0430000 	sb	v1,0(v0)
800c0de4:	080303af 	j	800c0ebc <Flash_write_one_channel_User+0x2b4>
800c0de8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:206
	else if((NDF%4)==2)
800c0dec:	8fc20030 	lw	v0,48(s8)
800c0df0:	30430003 	andi	v1,v0,0x3
800c0df4:	24020002 	li	v0,2
800c0df8:	14620013 	bne	v1,v0,800c0e48 <Flash_write_one_channel_User+0x240>
800c0dfc:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:208
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
800c0e00:	3c02bc00 	lui	v0,0xbc00
800c0e04:	34420060 	ori	v0,v0,0x60
800c0e08:	8fc30030 	lw	v1,48(s8)
800c0e0c:	2463fffe 	addiu	v1,v1,-2
800c0e10:	8fc40018 	lw	a0,24(s8)
800c0e14:	00831821 	addu	v1,a0,v1
800c0e18:	90630000 	lbu	v1,0(v1)
800c0e1c:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:209
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
800c0e20:	3c02bc00 	lui	v0,0xbc00
800c0e24:	34420060 	ori	v0,v0,0x60
800c0e28:	8fc30030 	lw	v1,48(s8)
800c0e2c:	2463ffff 	addiu	v1,v1,-1
800c0e30:	8fc40018 	lw	a0,24(s8)
800c0e34:	00831821 	addu	v1,a0,v1
800c0e38:	90630000 	lbu	v1,0(v1)
800c0e3c:	a0430000 	sb	v1,0(v0)
800c0e40:	080303af 	j	800c0ebc <Flash_write_one_channel_User+0x2b4>
800c0e44:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:211
	}
	else if((NDF%4)==3)
800c0e48:	8fc20030 	lw	v0,48(s8)
800c0e4c:	30430003 	andi	v1,v0,0x3
800c0e50:	24020003 	li	v0,3
800c0e54:	14620019 	bne	v1,v0,800c0ebc <Flash_write_one_channel_User+0x2b4>
800c0e58:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:213
	{
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-3);
800c0e5c:	3c02bc00 	lui	v0,0xbc00
800c0e60:	34420060 	ori	v0,v0,0x60
800c0e64:	8fc30030 	lw	v1,48(s8)
800c0e68:	2463fffd 	addiu	v1,v1,-3
800c0e6c:	8fc40018 	lw	a0,24(s8)
800c0e70:	00831821 	addu	v1,a0,v1
800c0e74:	90630000 	lbu	v1,0(v1)
800c0e78:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:214
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-2);
800c0e7c:	3c02bc00 	lui	v0,0xbc00
800c0e80:	34420060 	ori	v0,v0,0x60
800c0e84:	8fc30030 	lw	v1,48(s8)
800c0e88:	2463fffe 	addiu	v1,v1,-2
800c0e8c:	8fc40018 	lw	a0,24(s8)
800c0e90:	00831821 	addu	v1,a0,v1
800c0e94:	90630000 	lbu	v1,0(v1)
800c0e98:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:215
		REG8(FLASH_BASE_ADDR+FLASH_DR)=*(BReadBuffer+NDF-1);
800c0e9c:	3c02bc00 	lui	v0,0xbc00
800c0ea0:	34420060 	ori	v0,v0,0x60
800c0ea4:	8fc30030 	lw	v1,48(s8)
800c0ea8:	2463ffff 	addiu	v1,v1,-1
800c0eac:	8fc40018 	lw	a0,24(s8)
800c0eb0:	00831821 	addu	v1,a0,v1
800c0eb4:	90630000 	lbu	v1,0(v1)
800c0eb8:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:221
	}
	else
	{
	}

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=1;//enable SPIC			
800c0ebc:	3c02bc00 	lui	v0,0xbc00
800c0ec0:	34420008 	ori	v0,v0,0x8
800c0ec4:	24030001 	li	v1,1
800c0ec8:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:222
	Check_SPIC_Busy();
800c0ecc:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0ed0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:226 (discriminator 1)
	
	//Read Flash Status 
	do{
		Flash_RDSR();		
800c0ed4:	0c0301f1 	jal	800c07c4 <Flash_RDSR>
800c0ed8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:228 (discriminator 1)
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
800c0edc:	3c02bc00 	lui	v0,0xbc00
800c0ee0:	34420060 	ori	v0,v0,0x60
800c0ee4:	90420000 	lbu	v0,0(v0)
800c0ee8:	a3c20020 	sb	v0,32(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:229 (discriminator 1)
		flash_busy=flash_busy & 0x03;
800c0eec:	93c20020 	lbu	v0,32(s8)
800c0ef0:	30420003 	andi	v0,v0,0x3
800c0ef4:	a3c20020 	sb	v0,32(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:230 (discriminator 1)
	}while(flash_busy==0x03);
800c0ef8:	93c30020 	lbu	v1,32(s8)
800c0efc:	24020003 	li	v0,3
800c0f00:	1062fff4 	beq	v1,v0,800c0ed4 <Flash_write_one_channel_User+0x2cc>
800c0f04:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:232

}
800c0f08:	03c0e821 	move	sp,s8
800c0f0c:	8fbf002c 	lw	ra,44(sp)
800c0f10:	8fbe0028 	lw	s8,40(sp)
800c0f14:	27bd0030 	addiu	sp,sp,48
800c0f18:	03e00008 	jr	ra
800c0f1c:	00000000 	nop

800c0f20 <Flash_Exit4byte_Addrmode>:
Flash_Exit4byte_Addrmode():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:236


static void Flash_Exit4byte_Addrmode (void)
{
800c0f20:	27bdffe8 	addiu	sp,sp,-24
800c0f24:	afbf0014 	sw	ra,20(sp)
800c0f28:	afbe0010 	sw	s8,16(sp)
800c0f2c:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:237
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0f30:	3c02bc00 	lui	v0,0xbc00
800c0f34:	34420008 	ori	v0,v0,0x8
800c0f38:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:238
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
800c0f3c:	3c02bc00 	lui	v0,0xbc00
800c0f40:	34420010 	ori	v0,v0,0x10
800c0f44:	24030001 	li	v1,1
800c0f48:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:239
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
800c0f4c:	3c02bc00 	lui	v0,0xbc00
800c0f50:	34420004 	ori	v0,v0,0x4
800c0f54:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:240
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
800c0f58:	3c02bc00 	lui	v0,0xbc00
800c0f5c:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:241
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c0f60:	3c02bc00 	lui	v0,0xbc00
800c0f64:	34420008 	ori	v0,v0,0x8
800c0f68:	24030001 	li	v1,1
800c0f6c:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:243

	REG8(FLASH_BASE_ADDR+FLASH_DR)=OPCODE_EX4B;//enter 4byte command 	
800c0f70:	3c02bc00 	lui	v0,0xbc00
800c0f74:	34420060 	ori	v0,v0,0x60
800c0f78:	2403ffe9 	li	v1,-23
800c0f7c:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:244
	Check_SPIC_Busy();
800c0f80:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c0f84:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:246

}
800c0f88:	03c0e821 	move	sp,s8
800c0f8c:	8fbf0014 	lw	ra,20(sp)
800c0f90:	8fbe0010 	lw	s8,16(sp)
800c0f94:	27bd0018 	addiu	sp,sp,24
800c0f98:	03e00008 	jr	ra
800c0f9c:	00000000 	nop

800c0fa0 <Flash_RDID>:
Flash_RDID():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:250


INT32U Flash_RDID(void )
{
800c0fa0:	27bdffe0 	addiu	sp,sp,-32
800c0fa4:	afbf001c 	sw	ra,28(sp)
800c0fa8:	afbe0018 	sw	s8,24(sp)
800c0fac:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:256
	INT32U   flash_ID,spic_busy;
	INT8U	 i;
	INT32U   size;
	INT16U   ext_id;

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c0fb0:	3c02bc00 	lui	v0,0xbc00
800c0fb4:	34420008 	ori	v0,v0,0x8
800c0fb8:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:257
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x300;//EEPROM mode
800c0fbc:	3c02bc00 	lui	v0,0xbc00
800c0fc0:	24030300 	li	v1,768
800c0fc4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:258
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Enable spi_slave0
800c0fc8:	3c02bc00 	lui	v0,0xbc00
800c0fcc:	34420010 	ori	v0,v0,0x10
800c0fd0:	24030001 	li	v1,1
800c0fd4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:259
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x03;//read five byte back
800c0fd8:	3c02bc00 	lui	v0,0xbc00
800c0fdc:	34420004 	ori	v0,v0,0x4
800c0fe0:	24030003 	li	v1,3
800c0fe4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:260
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c0fe8:	3c02bc00 	lui	v0,0xbc00
800c0fec:	34420008 	ori	v0,v0,0x8
800c0ff0:	24030001 	li	v1,1
800c0ff4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:262

	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_RDID_COM;//RDID
800c0ff8:	3c02bc00 	lui	v0,0xbc00
800c0ffc:	34420060 	ori	v0,v0,0x60
800c1000:	2403ff9f 	li	v1,-97
800c1004:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:265 (discriminator 1)

	do{
		spic_busy=REG32(FLASH_BASE_ADDR+FLASH_SR);
800c1008:	3c02bc00 	lui	v0,0xbc00
800c100c:	34420028 	ori	v0,v0,0x28
800c1010:	8c420000 	lw	v0,0(v0)
800c1014:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:266 (discriminator 1)
		spic_busy=spic_busy & 0x00000001;
800c1018:	8fc20010 	lw	v0,16(s8)
800c101c:	30420001 	andi	v0,v0,0x1
800c1020:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:267 (discriminator 1)
	}while(spic_busy!=0x00);
800c1024:	8fc20010 	lw	v0,16(s8)
800c1028:	1440fff7 	bnez	v0,800c1008 <Flash_RDID+0x68>
800c102c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:269
	
	flash_ID =(REG8(FLASH_BASE_ADDR+FLASH_DR)<<16)+(REG8(FLASH_BASE_ADDR+FLASH_DR)<<8)+REG8(FLASH_BASE_ADDR+FLASH_DR);
800c1030:	3c02bc00 	lui	v0,0xbc00
800c1034:	34420060 	ori	v0,v0,0x60
800c1038:	90420000 	lbu	v0,0(v0)
800c103c:	304200ff 	andi	v0,v0,0xff
800c1040:	00021c00 	sll	v1,v0,0x10
800c1044:	3c02bc00 	lui	v0,0xbc00
800c1048:	34420060 	ori	v0,v0,0x60
800c104c:	90420000 	lbu	v0,0(v0)
800c1050:	304200ff 	andi	v0,v0,0xff
800c1054:	00021200 	sll	v0,v0,0x8
800c1058:	00621021 	addu	v0,v1,v0
800c105c:	3c03bc00 	lui	v1,0xbc00
800c1060:	34630060 	ori	v1,v1,0x60
800c1064:	90630000 	lbu	v1,0(v1)
800c1068:	306300ff 	andi	v1,v1,0xff
800c106c:	00431021 	addu	v0,v0,v1
800c1070:	afc20014 	sw	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:272

	//winbond 256Mb flash default address mode is 4byte address !!!
	if (((flash_ID>>16) == 0xef)&&((flash_ID&0x0000ff)==0x19)){
800c1074:	8fc20014 	lw	v0,20(s8)
800c1078:	00021c02 	srl	v1,v0,0x10
800c107c:	240200ef 	li	v0,239
800c1080:	14620008 	bne	v1,v0,800c10a4 <Flash_RDID+0x104>
800c1084:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:272 (discriminator 1)
800c1088:	8fc20014 	lw	v0,20(s8)
800c108c:	304300ff 	andi	v1,v0,0xff
800c1090:	24020019 	li	v0,25
800c1094:	14620003 	bne	v1,v0,800c10a4 <Flash_RDID+0x104>
800c1098:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:273
		Flash_Exit4byte_Addrmode();
800c109c:	0c0303c8 	jal	800c0f20 <Flash_Exit4byte_Addrmode>
800c10a0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:277
	}
	
	
	return flash_ID;
800c10a4:	8fc20014 	lw	v0,20(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:278
}
800c10a8:	03c0e821 	move	sp,s8
800c10ac:	8fbf001c 	lw	ra,28(sp)
800c10b0:	8fbe0018 	lw	s8,24(sp)
800c10b4:	27bd0020 	addiu	sp,sp,32
800c10b8:	03e00008 	jr	ra
800c10bc:	00000000 	nop

800c10c0 <Set_SPIC_Clock>:
Set_SPIC_Clock():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:284


/*Define spi_sclk divider value.The frequency of spi_sclk is derived from:
Frequency of spi_sclk = Frequency of bus_clk / (2*baudr).*/
void Set_SPIC_Clock(INT8U baudr)
{
800c10c0:	27bdfff8 	addiu	sp,sp,-8
800c10c4:	afbe0004 	sw	s8,4(sp)
800c10c8:	03a0f021 	move	s8,sp
800c10cc:	00801021 	move	v0,a0
800c10d0:	a3c20008 	sb	v0,8(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:285
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c10d4:	3c02bc00 	lui	v0,0xbc00
800c10d8:	34420008 	ori	v0,v0,0x8
800c10dc:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:286
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x0;//SPI mode 0
800c10e0:	3c02bc00 	lui	v0,0xbc00
800c10e4:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:287
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_CTRLR2)=0x81;//setup FIFO depth = 256, SO pin at pin 1
800c10e8:	3c02bc00 	lui	v0,0xbc00
800c10ec:	34420110 	ori	v0,v0,0x110
800c10f0:	24030081 	li	v1,129
800c10f4:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:307
	}
#endif

//if ASIC ,use SPIC default value 

	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c10f8:	3c02bc00 	lui	v0,0xbc00
800c10fc:	34420008 	ori	v0,v0,0x8
800c1100:	24030001 	li	v1,1
800c1104:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:308
}
800c1108:	03c0e821 	move	sp,s8
800c110c:	8fbe0004 	lw	s8,4(sp)
800c1110:	27bd0008 	addiu	sp,sp,8
800c1114:	03e00008 	jr	ra
800c1118:	00000000 	nop

800c111c <WRSR_Flash_one_two_channel>:
WRSR_Flash_one_two_channel():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:319
//when write /erase falsh ,must call this function firstly 
//NOTE: because different  rand flash has different manner to declare/set quad enable ,
//so just use one

void WRSR_Flash_one_two_channel(void )
{
800c111c:	27bdffe0 	addiu	sp,sp,-32
800c1120:	afbf001c 	sw	ra,28(sp)
800c1124:	afbe0018 	sw	s8,24(sp)
800c1128:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:323
	INT8U   flash_busy;

	//Setup SPIC
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c112c:	3c02bc00 	lui	v0,0xbc00
800c1130:	34420008 	ori	v0,v0,0x8
800c1134:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:324
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//single channel, blocking write
800c1138:	3c02bc00 	lui	v0,0xbc00
800c113c:	34420120 	ori	v0,v0,0x120
800c1140:	24030200 	li	v1,512
800c1144:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:325
	REG32(FLASH_BASE_ADDR+FLASH_ADDR_LENGTH)=0x01;//SPI Address byte = 1 in user mode
800c1148:	3c02bc00 	lui	v0,0xbc00
800c114c:	34420118 	ori	v0,v0,0x118
800c1150:	24030001 	li	v1,1
800c1154:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:326
	REG32(FLASH_BASE_ADDR+FLASH_VALID_CMD)=0x200;//one channel address/data, blocking write
800c1158:	3c02bc00 	lui	v0,0xbc00
800c115c:	34420120 	ori	v0,v0,0x120
800c1160:	24030200 	li	v1,512
800c1164:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:327
	REG32(FLASH_BASE_ADDR+FLASH_SER)=0x01;//Set SER
800c1168:	3c02bc00 	lui	v0,0xbc00
800c116c:	34420010 	ori	v0,v0,0x10
800c1170:	24030001 	li	v1,1
800c1174:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:328
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR1)=0x00;//No read any byte back
800c1178:	3c02bc00 	lui	v0,0xbc00
800c117c:	34420004 	ori	v0,v0,0x4
800c1180:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:329
	REG32(FLASH_BASE_ADDR+FLASH_CTRLR0)=0x000;//Tx mode
800c1184:	3c02bc00 	lui	v0,0xbc00
800c1188:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:330
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC	
800c118c:	3c02bc00 	lui	v0,0xbc00
800c1190:	34420008 	ori	v0,v0,0x8
800c1194:	24030001 	li	v1,1
800c1198:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:333

	//Write enable
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WREN_COM;
800c119c:	3c02bc00 	lui	v0,0xbc00
800c11a0:	34420060 	ori	v0,v0,0x60
800c11a4:	24030006 	li	v1,6
800c11a8:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:334
	Check_SPIC_Busy();
800c11ac:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c11b0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:337

	//Write Status
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0;//Disable SPIC
800c11b4:	3c02bc00 	lui	v0,0xbc00
800c11b8:	34420008 	ori	v0,v0,0x8
800c11bc:	ac400000 	sw	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:338
	REG8(FLASH_BASE_ADDR+FLASH_DR)=FLASH_WRSR_COM;
800c11c0:	3c02bc00 	lui	v0,0xbc00
800c11c4:	34420060 	ori	v0,v0,0x60
800c11c8:	24030001 	li	v1,1
800c11cc:	a0430000 	sb	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:339
	REG8(FLASH_BASE_ADDR+FLASH_DR)=0x00;//one/two channel Read mode Enable
800c11d0:	3c02bc00 	lui	v0,0xbc00
800c11d4:	34420060 	ori	v0,v0,0x60
800c11d8:	a0400000 	sb	zero,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:340
	REG32(FLASH_BASE_ADDR+FLASH_SSIENR)=0x01;//Enable SPIC
800c11dc:	3c02bc00 	lui	v0,0xbc00
800c11e0:	34420008 	ori	v0,v0,0x8
800c11e4:	24030001 	li	v1,1
800c11e8:	ac430000 	sw	v1,0(v0)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:341
	Check_SPIC_Busy();	
800c11ec:	0c0301e1 	jal	800c0784 <Check_SPIC_Busy>
800c11f0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:343 (discriminator 1)
	do{
		Flash_RDSR();	
800c11f4:	0c0301f1 	jal	800c07c4 <Flash_RDSR>
800c11f8:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:345 (discriminator 1)
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
800c11fc:	3c02bc00 	lui	v0,0xbc00
800c1200:	34420060 	ori	v0,v0,0x60
800c1204:	90420000 	lbu	v0,0(v0)
800c1208:	a3c20010 	sb	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:346 (discriminator 1)
		flash_busy=flash_busy & 0x03;
800c120c:	93c20010 	lbu	v0,16(s8)
800c1210:	30420003 	andi	v0,v0,0x3
800c1214:	a3c20010 	sb	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:347 (discriminator 1)
	}while(flash_busy==0x03);
800c1218:	93c30010 	lbu	v1,16(s8)
800c121c:	24020003 	li	v0,3
800c1220:	1062fff4 	beq	v1,v0,800c11f4 <WRSR_Flash_one_two_channel+0xd8>
800c1224:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:349

}
800c1228:	03c0e821 	move	sp,s8
800c122c:	8fbf001c 	lw	ra,28(sp)
800c1230:	8fbe0018 	lw	s8,24(sp)
800c1234:	27bd0020 	addiu	sp,sp,32
800c1238:	03e00008 	jr	ra
800c123c:	00000000 	nop

800c1240 <Flash_Init>:
Flash_Init():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:355





void Flash_Init(void){
800c1240:	27bdffe0 	addiu	sp,sp,-32
800c1244:	afbf001c 	sw	ra,28(sp)
800c1248:	afbe0018 	sw	s8,24(sp)
800c124c:	03a0f021 	move	s8,sp
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:358
	INT32U FLASH_ID;
	
	Set_SPIC_Clock(1);//Frequency of spi_sclk = Frequency of bus_clk / (2*baudr)
800c1250:	24040001 	li	a0,1
800c1254:	0c030430 	jal	800c10c0 <Set_SPIC_Clock>
800c1258:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:359
	FLASH_ID=Flash_RDID();
800c125c:	0c0303e8 	jal	800c0fa0 <Flash_RDID>
800c1260:	00000000 	nop
800c1264:	afc20010 	sw	v0,16(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:361

	WRSR_Flash_one_two_channel(); // unprotect all 
800c1268:	0c030447 	jal	800c111c <WRSR_Flash_one_two_channel>
800c126c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:364

	
}
800c1270:	03c0e821 	move	sp,s8
800c1274:	8fbf001c 	lw	ra,28(sp)
800c1278:	8fbe0018 	lw	s8,24(sp)
800c127c:	27bd0020 	addiu	sp,sp,32
800c1280:	03e00008 	jr	ra
800c1284:	00000000 	nop

800c1288 <Flash_Unprotect>:
Flash_Unprotect():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:368

//WRSR_Flash_one_two_channel will unprotect all
void Flash_Unprotect(INT32U Address)
{
800c1288:	27bdfff8 	addiu	sp,sp,-8
800c128c:	afbe0004 	sw	s8,4(sp)
800c1290:	03a0f021 	move	s8,sp
800c1294:	afc40008 	sw	a0,8(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:401
		//check flash busy?	
		flash_busy=REG8(FLASH_BASE_ADDR+FLASH_DR);
		flash_busy=flash_busy & 0x03;
	}while(flash_busy==0x03);
	#endif
}
800c1298:	03c0e821 	move	sp,s8
800c129c:	8fbe0004 	lw	s8,4(sp)
800c12a0:	27bd0008 	addiu	sp,sp,8
800c12a4:	03e00008 	jr	ra
800c12a8:	00000000 	nop

800c12ac <Compare_Flash>:
Compare_Flash():
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:403

int Compare_Flash(INT32U *source, INT32U *destination, int size){
800c12ac:	27bdffe8 	addiu	sp,sp,-24
800c12b0:	afbe0014 	sw	s8,20(sp)
800c12b4:	03a0f021 	move	s8,sp
800c12b8:	afc40018 	sw	a0,24(s8)
800c12bc:	afc5001c 	sw	a1,28(s8)
800c12c0:	afc60020 	sw	a2,32(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:404
	int i = 0;
800c12c4:	afc00000 	sw	zero,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:408
	INT32U *addr1;
	INT32U *addr2;
		
	for ( i = 0 ; i < size ; i++ ){
800c12c8:	afc00000 	sw	zero,0(s8)
800c12cc:	080304cb 	j	800c132c <Compare_Flash+0x80>
800c12d0:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:409
		addr1 = source + i; 
800c12d4:	8fc20000 	lw	v0,0(s8)
800c12d8:	00021080 	sll	v0,v0,0x2
800c12dc:	8fc30018 	lw	v1,24(s8)
800c12e0:	00621021 	addu	v0,v1,v0
800c12e4:	afc20004 	sw	v0,4(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:410
		addr2 = destination + i;
800c12e8:	8fc20000 	lw	v0,0(s8)
800c12ec:	00021080 	sll	v0,v0,0x2
800c12f0:	8fc3001c 	lw	v1,28(s8)
800c12f4:	00621021 	addu	v0,v1,v0
800c12f8:	afc20008 	sw	v0,8(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:411
		if ( *addr1 != *addr2 ){
800c12fc:	8fc20004 	lw	v0,4(s8)
800c1300:	8c430000 	lw	v1,0(v0)
800c1304:	8fc20008 	lw	v0,8(s8)
800c1308:	8c420000 	lw	v0,0(v0)
800c130c:	10620004 	beq	v1,v0,800c1320 <Compare_Flash+0x74>
800c1310:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:412
			return -1 ;	
800c1314:	2402ffff 	li	v0,-1
800c1318:	080304d1 	j	800c1344 <Compare_Flash+0x98>
800c131c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:408 (discriminator 2)
int Compare_Flash(INT32U *source, INT32U *destination, int size){
	int i = 0;
	INT32U *addr1;
	INT32U *addr2;
		
	for ( i = 0 ; i < size ; i++ ){
800c1320:	8fc20000 	lw	v0,0(s8)
800c1324:	24420001 	addiu	v0,v0,1
800c1328:	afc20000 	sw	v0,0(s8)
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:408 (discriminator 1)
800c132c:	8fc30000 	lw	v1,0(s8)
800c1330:	8fc20020 	lw	v0,32(s8)
800c1334:	0062102a 	slt	v0,v1,v0
800c1338:	1440ffe6 	bnez	v0,800c12d4 <Compare_Flash+0x28>
800c133c:	00000000 	nop
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:416
		if ( *addr1 != *addr2 ){
			return -1 ;	
		}	 	
	}

	return 0;
800c1340:	00001021 	move	v0,zero
/work/8111FP_uCOS2_88pin/FT_EQC/flash_auto_run/8111FPRevA_lib_v5_DDK_CombineTest_v19.6/flashfp/flash_fp.c:417
}
800c1344:	03c0e821 	move	sp,s8
800c1348:	8fbe0014 	lw	s8,20(sp)
800c134c:	27bd0018 	addiu	sp,sp,24
800c1350:	03e00008 	jr	ra
800c1354:	00000000 	nop

800c1358 <memcmp>:
memcmp():
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:87
800c1358:	28c90004 	slti	t1,a2,4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:88
800c135c:	11200005 	beqz	t1,800c1374 <memcmp+0x1c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:89
800c1360:	00863821 	addu	a3,a0,a2
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:91
800c1364:	1cc00017 	bgtz	a2,800c13c4 <memcmp+0x6c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:92
800c1368:	00000000 	nop
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:97
800c136c:	03e00008 	jr	ra
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:98
800c1370:	00001021 	move	v0,zero
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:99
800c1374:	00854025 	or	t0,a0,a1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:100
800c1378:	31080003 	andi	t0,t0,0x3
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:102
800c137c:	11000003 	beqz	t0,800c138c <memcmp+0x34>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:103
800c1380:	00000000 	nop
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:108
800c1384:	10000010 	b	800c13c8 <memcmp+0x70>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:109
800c1388:	90880000 	lbu	t0,0(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:113
800c138c:	8c880000 	lw	t0,0(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:115
800c1390:	8ca90000 	lw	t1,0(a1)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:116
800c1394:	24840004 	addiu	a0,a0,4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:118
800c1398:	11090005 	beq	t0,t1,800c13b0 <memcmp+0x58>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:119
800c139c:	24a50004 	addiu	a1,a1,4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:120
800c13a0:	24a5fffc 	addiu	a1,a1,-4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:125
800c13a4:	2484fffc 	addiu	a0,a0,-4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:126
800c13a8:	10000007 	b	800c13c8 <memcmp+0x70>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:127
800c13ac:	90880000 	lbu	t0,0(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:128
800c13b0:	24c6fffc 	addiu	a2,a2,-4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:130
800c13b4:	1cc0fff5 	bgtz	a2,800c138c <memcmp+0x34>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:131
800c13b8:	00000000 	nop
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:136
800c13bc:	03e00008 	jr	ra
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:137
800c13c0:	00001021 	move	v0,zero
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:141
800c13c4:	90880000 	lbu	t0,0(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:143
800c13c8:	90a90000 	lbu	t1,0(a1)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:144
800c13cc:	24840001 	addiu	a0,a0,1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:145
800c13d0:	11090003 	beq	t0,t1,800c13e0 <memcmp+0x88>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:146
800c13d4:	24a50001 	addiu	a1,a1,1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:147
800c13d8:	03e00008 	jr	ra
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:148
800c13dc:	01091023 	subu	v0,t0,t1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:150
800c13e0:	1487fff8 	bne	a0,a3,800c13c4 <memcmp+0x6c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:151
800c13e4:	00000000 	nop
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:156
800c13e8:	03e00008 	jr	ra
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memcmp.S:157
800c13ec:	00001021 	move	v0,zero

800c13f0 <bzero>:
bzero():
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:67
800c13f0:	00a03021 	move	a2,a1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:68
800c13f4:	00002821 	move	a1,zero

800c13f8 <memset>:
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:72
800c13f8:	00801021 	move	v0,a0
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:73
800c13fc:	10c00028 	beqz	a2,800c14a0 <memset+0xa8>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:74
800c1400:	2cca0010 	sltiu	t2,a2,16
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:75
800c1404:	15400021 	bnez	t2,800c148c <memset+0x94>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:78
800c1408:	30a500ff 	andi	a1,a1,0xff
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:87
800c140c:	00055200 	sll	t2,a1,0x8
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:88
800c1410:	00aa2825 	or	a1,a1,t2
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:89
800c1414:	00055400 	sll	t2,a1,0x10
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:90
800c1418:	00aa2825 	or	a1,a1,t2
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:93
800c141c:	30830003 	andi	v1,a0,0x3
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:94
800c1420:	10600007 	beqz	v1,800c1440 <memset+0x48>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:95
800c1424:	24070004 	li	a3,4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:96
800c1428:	00e33823 	subu	a3,a3,v1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:98
800c142c:	00c73023 	subu	a2,a2,a3
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:103
800c1430:	00871821 	addu	v1,a0,a3
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:104
800c1434:	24840001 	addiu	a0,a0,1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:105
800c1438:	1464fffe 	bne	v1,a0,800c1434 <memset+0x3c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:106
800c143c:	a085ffff 	sb	a1,-1(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:110
800c1440:	30c3000f 	andi	v1,a2,0xf
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:111
800c1444:	00c33823 	subu	a3,a2,v1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:112
800c1448:	10e00008 	beqz	a3,800c146c <memset+0x74>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:113
800c144c:	00603021 	move	a2,v1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:114
800c1450:	00e43821 	addu	a3,a3,a0
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:116
800c1454:	ac850000 	sw	a1,0(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:117
800c1458:	ac850004 	sw	a1,4(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:118
800c145c:	ac850008 	sw	a1,8(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:119
800c1460:	24840010 	addiu	a0,a0,16
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:120
800c1464:	1487fffb 	bne	a0,a3,800c1454 <memset+0x5c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:121
800c1468:	ac85fffc 	sw	a1,-4(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:124
800c146c:	30c30003 	andi	v1,a2,0x3
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:125
800c1470:	00c33823 	subu	a3,a2,v1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:126
800c1474:	10e00005 	beqz	a3,800c148c <memset+0x94>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:127
800c1478:	00603021 	move	a2,v1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:128
800c147c:	00e43821 	addu	a3,a3,a0
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:130
800c1480:	24840004 	addiu	a0,a0,4
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:131
800c1484:	1487fffe 	bne	a0,a3,800c1480 <memset+0x88>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:132
800c1488:	ac85fffc 	sw	a1,-4(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:136
800c148c:	10c00004 	beqz	a2,800c14a0 <memset+0xa8>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:137
800c1490:	00c43821 	addu	a3,a2,a0
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:139
800c1494:	24840001 	addiu	a0,a0,1
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:140
800c1498:	1487fffe 	bne	a0,a3,800c1494 <memset+0x9c>
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:141
800c149c:	a085ffff 	sb	a1,-1(a0)
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:143
800c14a0:	03e00008 	jr	ra
/cygdrive/g/RTL8168DP/newlib/rsdk-1.5.6/mips-elf/newlib/libc/machine/mips/../../../../../../newlib/libc/machine/mips/memset.S:144
800c14a4:	00000000 	nop

Disassembly of section .bss:

800c14b0 <_stack_lo>:
	...

800c18b0 <_stack_hi>:
	...

800c18d0 <FLASH_ID>:
800c18d0:	00000000 	nop

800c18d4 <flash_struct>:
	...

Disassembly of section .reginfo:

00000000 <.reginfo>:
   0:	e000007c 	sc	zero,124(zero)
	...
  14:	800c14b0 	lb	t4,5296(zero)

Disassembly of section .pdr:

00000000 <.pdr>:
   0:	800c0078 	lb	t4,120(zero)
   4:	40000000 	mfc0	zero,$0
   8:	fffffffc 	0xfffffffc
	...
  14:	00000010 	mfhi	zero
  18:	0000001e 	0x1e
  1c:	0000001f 	0x1f
  20:	800c00f4 	lb	t4,244(zero)
  24:	40000000 	mfc0	zero,$0
  28:	fffffffc 	0xfffffffc
	...
  34:	00000008 	jr	zero
  38:	0000001e 	0x1e
  3c:	0000001f 	0x1f
  40:	800c0184 	lb	t4,388(zero)
  44:	c0000000 	ll	zero,0(zero)
  48:	fffffffc 	0xfffffffc
	...
  54:	00000018 	mult	zero,zero
  58:	0000001e 	0x1e
  5c:	0000001f 	0x1f
  60:	800c01e8 	lb	t4,488(zero)
  64:	c0000000 	ll	zero,0(zero)
  68:	fffffffc 	0xfffffffc
	...
  74:	000001c8 	0x1c8
  78:	0000001e 	0x1e
  7c:	0000001f 	0x1f
  80:	800c0448 	lb	t4,1096(zero)
  84:	40000000 	mfc0	zero,$0
  88:	fffffffc 	0xfffffffc
	...
  94:	00000010 	mfhi	zero
  98:	0000001e 	0x1e
  9c:	0000001f 	0x1f
  a0:	800c04b0 	lb	t4,1200(zero)
  a4:	40000000 	mfc0	zero,$0
  a8:	fffffffc 	0xfffffffc
	...
  b4:	00000010 	mfhi	zero
  b8:	0000001e 	0x1e
  bc:	0000001f 	0x1f
  c0:	800c0518 	lb	t4,1304(zero)
  c4:	40000000 	mfc0	zero,$0
  c8:	fffffffc 	0xfffffffc
	...
  d4:	00000010 	mfhi	zero
  d8:	0000001e 	0x1e
  dc:	0000001f 	0x1f
  e0:	800c0580 	lb	t4,1408(zero)
  e4:	40000000 	mfc0	zero,$0
  e8:	fffffffc 	0xfffffffc
	...
  f4:	00000010 	mfhi	zero
  f8:	0000001e 	0x1e
  fc:	0000001f 	0x1f
 100:	800c05c8 	lb	t4,1480(zero)
 104:	40000000 	mfc0	zero,$0
 108:	fffffffc 	0xfffffffc
	...
 114:	00000010 	mfhi	zero
 118:	0000001e 	0x1e
 11c:	0000001f 	0x1f
 120:	800c0610 	lb	t4,1552(zero)
 124:	40000000 	mfc0	zero,$0
 128:	fffffffc 	0xfffffffc
	...
 134:	00000010 	mfhi	zero
 138:	0000001e 	0x1e
 13c:	0000001f 	0x1f
 140:	800c06b0 	lb	t4,1712(zero)
 144:	40000000 	mfc0	zero,$0
 148:	fffffffc 	0xfffffffc
	...
 154:	00000010 	mfhi	zero
 158:	0000001e 	0x1e
 15c:	0000001f 	0x1f
 160:	800c0704 	lb	t4,1796(zero)
 164:	40000000 	mfc0	zero,$0
 168:	fffffffc 	0xfffffffc
	...
 174:	00000010 	mfhi	zero
 178:	0000001e 	0x1e
 17c:	0000001f 	0x1f
 180:	800c0784 	lb	t4,1924(zero)
 184:	40000000 	mfc0	zero,$0
 188:	fffffffc 	0xfffffffc
	...
 194:	00000010 	mfhi	zero
 198:	0000001e 	0x1e
 19c:	0000001f 	0x1f
 1a0:	800c07c4 	lb	t4,1988(zero)
 1a4:	40000000 	mfc0	zero,$0
 1a8:	fffffffc 	0xfffffffc
	...
 1b4:	00000010 	mfhi	zero
 1b8:	0000001e 	0x1e
 1bc:	0000001f 	0x1f
 1c0:	800c0864 	lb	t4,2148(zero)
 1c4:	c0000000 	ll	zero,0(zero)
 1c8:	fffffffc 	0xfffffffc
	...
 1d4:	00000020 	add	zero,zero,zero
 1d8:	0000001e 	0x1e
 1dc:	0000001f 	0x1f
 1e0:	800c0940 	lb	t4,2368(zero)
 1e4:	c0000000 	ll	zero,0(zero)
 1e8:	fffffffc 	0xfffffffc
	...
 1f4:	00000020 	add	zero,zero,zero
 1f8:	0000001e 	0x1e
 1fc:	0000001f 	0x1f
 200:	800c0a6c 	lb	t4,2668(zero)
 204:	c0000000 	ll	zero,0(zero)
 208:	fffffffc 	0xfffffffc
	...
 214:	00000020 	add	zero,zero,zero
 218:	0000001e 	0x1e
 21c:	0000001f 	0x1f
 220:	800c0b98 	lb	t4,2968(zero)
 224:	40000000 	mfc0	zero,$0
 228:	fffffffc 	0xfffffffc
	...
 234:	00000008 	jr	zero
 238:	0000001e 	0x1e
 23c:	0000001f 	0x1f
 240:	800c0c08 	lb	t4,3080(zero)
 244:	c0000000 	ll	zero,0(zero)
 248:	fffffffc 	0xfffffffc
	...
 254:	00000030 	0x30
 258:	0000001e 	0x1e
 25c:	0000001f 	0x1f
 260:	800c0f20 	lb	t4,3872(zero)
 264:	c0000000 	ll	zero,0(zero)
 268:	fffffffc 	0xfffffffc
	...
 274:	00000018 	mult	zero,zero
 278:	0000001e 	0x1e
 27c:	0000001f 	0x1f
 280:	800c0fa0 	lb	t4,4000(zero)
 284:	c0000000 	ll	zero,0(zero)
 288:	fffffffc 	0xfffffffc
	...
 294:	00000020 	add	zero,zero,zero
 298:	0000001e 	0x1e
 29c:	0000001f 	0x1f
 2a0:	800c10c0 	lb	t4,4288(zero)
 2a4:	40000000 	mfc0	zero,$0
 2a8:	fffffffc 	0xfffffffc
	...
 2b4:	00000008 	jr	zero
 2b8:	0000001e 	0x1e
 2bc:	0000001f 	0x1f
 2c0:	800c111c 	lb	t4,4380(zero)
 2c4:	c0000000 	ll	zero,0(zero)
 2c8:	fffffffc 	0xfffffffc
	...
 2d4:	00000020 	add	zero,zero,zero
 2d8:	0000001e 	0x1e
 2dc:	0000001f 	0x1f
 2e0:	800c1240 	lb	t4,4672(zero)
 2e4:	c0000000 	ll	zero,0(zero)
 2e8:	fffffffc 	0xfffffffc
	...
 2f4:	00000020 	add	zero,zero,zero
 2f8:	0000001e 	0x1e
 2fc:	0000001f 	0x1f
 300:	800c1288 	lb	t4,4744(zero)
 304:	40000000 	mfc0	zero,$0
 308:	fffffffc 	0xfffffffc
	...
 314:	00000008 	jr	zero
 318:	0000001e 	0x1e
 31c:	0000001f 	0x1f
 320:	800c12ac 	lb	t4,4780(zero)
 324:	40000000 	mfc0	zero,$0
 328:	fffffffc 	0xfffffffc
	...
 334:	00000018 	mult	zero,zero
 338:	0000001e 	0x1e
 33c:	0000001f 	0x1f
 340:	800c0000 	lb	t4,0(zero)
	...
 360:	800c1358 	lb	t4,4952(zero)
	...
 380:	800c13f0 	lb	t4,5104(zero)
	...

Disassembly of section .comment:

00000000 <.comment>:
   0:	3a434347 	xori	v1,s2,0x4347
   4:	65522820 	0x65522820
   8:	65746c61 	0x65746c61
   c:	5352206b 	0x5352206b
  10:	342d4b44 	ori	t5,at,0x4b44
  14:	322e392e 	andi	t6,s1,0x392e
  18:	69754220 	0x69754220
  1c:	3220646c 	andi	zero,s1,0x646c
  20:	29353930 	slti	s5,t1,14640
  24:	392e3420 	xori	t6,t1,0x3420
  28:	3220322e 	andi	zero,s1,0x322e
  2c:	31343130 	andi	s4,t1,0x3130
  30:	20333130 	addi	s3,at,12592
  34:	65727028 	0x65727028
  38:	656c6572 	0x656c6572
  3c:	29657361 	slti	a1,t3,29537
	...

Disassembly of section .gnu.attributes:

00000000 <.gnu.attributes>:
   0:	00000f41 	0xf41
   4:	756e6700 	jalx	5b99c00 <_boot-0x7a526400>0x6700
   8:	00070100 	sll	zero,a3,0x4
   c:	03040000 	0x3040000

Disassembly of section .debug_info:

00000000 <.debug_info>:
   0:	0000028d 	break	0x0,0xa
   4:	00000004 	sllv	zero,zero,zero
   8:	01040000 	0x1040000
   c:	00000208 	0x208
  10:	0001d301 	0x1d301
  14:	00003a00 	sll	a3,zero,0x8
  18:	0c007800 	jal	1e000 <_boot-0x800a2000>
  1c:	0003d080 	sll	k0,v1,0x2
  20:	00000000 	nop
  24:	08010200 	j	40800 <_boot-0x8007f800>
  28:	00000145 	0x145
  2c:	00013f03 	sra	a3,at,0x1c
  30:	25190200 	addiu	t9,t0,512
  34:	02000000 	0x2000000
  38:	01470601 	0x1470601
  3c:	d7030000 	ldc1	$f3,0(t8)
  40:	02000000 	0x2000000
  44:	0000491b 	0x491b
  48:	07020200 	0x7020200
  4c:	00000184 	0x184
  50:	99050202 	lwr	a1,514(t0)
  54:	03000000 	0x3000000
  58:	000000de 	0xde
  5c:	00621d02 	0x621d02
  60:	04020000 	0x4020000
  64:	00039a07 	0x39a07
  68:	05040400 	0x5040400
  6c:	00746e69 	0x746e69
  70:	002c0405 	0x2c0405
  74:	10060000 	beq	zero,a2,78 <_boot-0x800bff88>
  78:	00bb0403 	0xbb0403
  7c:	bd070000 	cache	0x7,0(t0)
  80:	03000001 	movf	zero,t8,$fcc0
  84:	00005709 	0x5709
  88:	27070000 	addiu	a3,t8,0
  8c:	03000001 	movf	zero,t8,$fcc0
  90:	0000570d 	break	0x0,0x15c
  94:	e5070400 	swc1	$f7,1024(t0)
  98:	03000000 	0x3000000
  9c:	00003e0e 	0x3e0e
  a0:	79070800 	0x79070800
  a4:	03000001 	movf	zero,t8,$fcc0
  a8:	00003e0f 	0x3e0f
  ac:	53070a00 	0x53070a00
  b0:	03000001 	movf	zero,t8,$fcc0
  b4:	00003e10 	0x3e10
  b8:	03000c00 	0x3000c00
  bc:	000001a1 	0x1a1
  c0:	00761103 	0x761103
  c4:	fa080000 	0xfa080000
  c8:	01000001 	movf	zero,t0,$fcc0
  cc:	0c007806 	jal	1e018 <_boot-0x800a1fe8>
  d0:	00007c80 	sll	t7,zero,0x12
  d4:	029c0100 	0x29c0100
  d8:	09000001 	j	4000004 <_boot-0x7c0bfffc>
  dc:	0000016f 	0x16f
  e0:	00570601 	0x570601
  e4:	91020000 	lbu	v0,0(t0)
  e8:	00690a00 	0x690a00
  ec:	00570801 	0x570801
  f0:	91020000 	lbu	v0,0(t0)
  f4:	006a0a70 	0x6a0a70
  f8:	00570801 	0x570801
  fc:	91020000 	lbu	v0,0(t0)
 100:	e90b0074 	swc2	$11,116(t0)
 104:	01000001 	movf	zero,t0,$fcc0
 108:	0c00f410 	jal	3d040 <_boot-0x80082fc0>
 10c:	00009080 	sll	s2,zero,0x2
 110:	0c9c0100 	jal	2700400 <_boot-0x7d9bfc00>
 114:	00000159 	0x159
 118:	01842201 	0x1842201
 11c:	0064800c 	syscall	0x19200
 120:	9c010000 	0x9c010000
 124:	0000b70d 	break	0x0,0x2dc
 128:	693a0100 	0x693a0100
 12c:	e8000000 	swc2	$0,0(zero)
 130:	60800c01 	0x60800c01
 134:	01000002 	0x1000002
 138:	0002559c 	0x2559c
 13c:	00690a00 	0x690a00
 140:	00573c01 	0x573c01
 144:	91030000 	lbu	v1,0(t0)
 148:	330e7cc8 	andi	t6,t8,0x7cc8
 14c:	0f000001 	jal	c000004 <_boot-0x740bfffc>
 150:	000001cb 	0x1cb
 154:	02553d01 	0x2553d01
 158:	91030000 	lbu	v1,0(t0)
 15c:	740f7cd0 	jalx	3df340 <_boot-0x7fce0cc0>0x7cd0
 160:	01000001 	movf	zero,t0,$fcc0
 164:	0000573e 	0x573e
 168:	d4910300 	ldc1	$f17,768(a0)
 16c:	02030f7c 	0x2030f7c
 170:	3f010000 	0x3f010000
 174:	0000002c 	0x2c
 178:	7ccc9103 	0x7ccc9103
 17c:	0000ac0f 	0xac0f
 180:	57420100 	0x57420100
 184:	03000000 	0x3000000
 188:	0f7cd891 	jal	df36244 <_boot-0x72189dbc>
 18c:	00000197 	0x197
 190:	00704301 	0x704301
 194:	91030000 	lbu	v1,0(t0)
 198:	270f7cdc 	addiu	t7,t8,31964
 19c:	01000000 	0x1000000
 1a0:	00005744 	0x5744
 1a4:	e0910300 	sc	s1,768(a0)
 1a8:	01c60f7c 	0x1c60f7c
 1ac:	46010000 	add.s	$f0,$f0,$f1
 1b0:	0000025b 	0x25b
 1b4:	7ce49103 	0x7ce49103
 1b8:	00000d10 	0xd10
 1bc:	69550100 	0x69550100
 1c0:	c9000000 	lwc2	$0,0(t0)
 1c4:	11000001 	beqz	t0,1cc <_boot-0x800bfe34>
 1c8:	01d91000 	0x1d91000
 1cc:	56010000 	0x56010000
 1d0:	00000069 	0x69
 1d4:	000001da 	0x1da
 1d8:	b2100011 	0xb2100011
 1dc:	01000001 	movf	zero,t0,$fcc0
 1e0:	00006959 	0x6959
 1e4:	0001eb00 	sll	sp,at,0xc
 1e8:	10001100 	b	45ec <_boot-0x800bba14>
 1ec:	000000bc 	0xbc
 1f0:	00695b01 	0x695b01
 1f4:	01fc0000 	0x1fc0000
 1f8:	00110000 	sll	zero,s1,0x0
 1fc:	0000ef10 	0xef10
 200:	69850100 	0x69850100
 204:	0d000000 	jal	4000000 <_boot-0x7c0c0000>
 208:	11000002 	beqz	t0,214 <_boot-0x800bfdec>
 20c:	03201200 	0x3201200
 210:	00f4800c 	syscall	0x3d200
 214:	10100000 	beq	zero,s0,218 <_boot-0x800bfde8>
 218:	01000001 	movf	zero,t0,$fcc0
 21c:	00006970 	0x6970
 220:	00022700 	sll	a0,v0,0x1c
 224:	13001100 	beqz	t8,4628 <_boot-0x800bb9d8>
 228:	00000120 	0x120
 22c:	02720004 	sllv	zero,s2,s3
 230:	02460000 	0x2460000
 234:	72140000 	mad	s0,s4
 238:	14000002 	bnez	zero,244 <_boot-0x800bfdbc>
 23c:	00000069 	0x69
 240:	00006214 	0x6214
 244:	09150000 	j	4540000 <_boot-0x7bb80000>
 248:	04000001 	bltz	zero,250 <_boot-0x800bfdb0>
 24c:	00006900 	sll	t5,zero,0x4
 250:	00001100 	sll	v0,zero,0x4
 254:	4a040500 	c2	0x40500
 258:	16000001 	bnez	s0,260 <_boot-0x800bfda0>
 25c:	00000069 	0x69
 260:	0000026b 	0x26b
 264:	00026b17 	0x26b17
 268:	02006300 	0x2006300
 26c:	00a30704 	0xa30704
 270:	04180000 	0x4180000
 274:	00000019 	multu	zero,zero
 278:	bb130300 	swr	s3,768(t8)
 27c:	1a000000 	blez	s0,280 <_boot-0x800bfd80>
 280:	00000031 	0x31
 284:	00570401 	0x570401
 288:	03050000 	0x3050000
 28c:	800c18d0 	lb	t4,6352(zero)
 290:	00023800 	sll	a3,v0,0x0
 294:	7d000400 	0x7d000400
 298:	04000001 	bltz	zero,2a0 <_boot-0x800bfd60>
 29c:	00020801 	0x20801
 2a0:	02960100 	0x2960100
 2a4:	003a0000 	0x3a0000
 2a8:	04480000 	0x4480000
 2ac:	033c800c 	syscall	0xcf200
 2b0:	00e70000 	0xe70000
 2b4:	6e020000 	0x6e020000
 2b8:	02000002 	0x2000002
 2bc:	00003018 	0x3018
 2c0:	08010300 	j	40c00 <_boot-0x8007f400>
 2c4:	00000145 	0x145
 2c8:	47060103 	c1	0x1060103
 2cc:	03000001 	movf	zero,t8,$fcc0
 2d0:	01840702 	0x1840702
 2d4:	02030000 	0x2030000
 2d8:	00009905 	0x9905
 2dc:	00de0200 	0xde0200
 2e0:	1d020000 	0x1d020000
 2e4:	00000057 	0x57
 2e8:	9a070403 	lwr	a3,1027(s0)
 2ec:	04000003 	bltz	zero,2fc <_boot-0x800bfd04>
 2f0:	6e690504 	0x6e690504
 2f4:	b3050074 	0xb3050074
 2f8:	01000002 	0x1000002
 2fc:	0c044834 	jal	1120d0 <_boot-0x7ffadf30>
 300:	00006880 	sll	t5,zero,0x2
 304:	a59c0100 	sh	gp,256(t4)
 308:	06000000 	bltz	s0,30c <_boot-0x800bfcf4>
 30c:	000002a2 	0x2a2
 310:	004c3401 	0x4c3401
 314:	91020000 	lbu	v0,0(t0)
 318:	002c0600 	0x2c0600
 31c:	34010000 	li	at,0x0
 320:	0000004c 	syscall	0x1
 324:	07049102 	0x7049102
 328:	0000019c 	0x19c
 32c:	004c3601 	0x4c3601
 330:	91020000 	lbu	v0,0(t0)
 334:	d4050070 	ldc1	$f5,112(zero)
 338:	01000002 	0x1000002
 33c:	0c04b04d 	jal	12c134 <_boot-0x7ff93ecc>
 340:	00006880 	sll	t5,zero,0x2
 344:	e59c0100 	swc1	$f28,256(t4)
 348:	06000000 	bltz	s0,34c <_boot-0x800bfcb4>
 34c:	000002a2 	0x2a2
 350:	004c4d01 	0x4c4d01
 354:	91020000 	lbu	v0,0(t0)
 358:	002c0600 	0x2c0600
 35c:	4d010000 	bc3t	360 <_boot-0x800bfca0>
 360:	0000004c 	syscall	0x1
 364:	07049102 	0x7049102
 368:	0000019c 	0x19c
 36c:	004c4f01 	0x4c4f01
 370:	91020000 	lbu	v0,0(t0)
 374:	49050070 	0x49050070
 378:	01000002 	0x1000002
 37c:	0c051867 	jal	14619c <_boot-0x7ff79e64>
 380:	00006880 	sll	t5,zero,0x2
 384:	259c0100 	addiu	gp,t4,256
 388:	06000001 	bltz	s0,390 <_boot-0x800bfc70>
 38c:	000002a2 	0x2a2
 390:	004c6701 	0x4c6701
 394:	91020000 	lbu	v0,0(t0)
 398:	002c0600 	0x2c0600
 39c:	67010000 	0x67010000
 3a0:	0000004c 	syscall	0x1
 3a4:	07049102 	0x7049102
 3a8:	0000019c 	0x19c
 3ac:	004c6901 	0x4c6901
 3b0:	91020000 	lbu	v0,0(t0)
 3b4:	ef050070 	swc3	$5,112(t8)
 3b8:	01000000 	0x1000000
 3bc:	0c058080 	jal	160200 <_boot-0x7ff5fe00>
 3c0:	00004880 	sll	t1,zero,0x2
 3c4:	499c0100 	0x499c0100
 3c8:	07000001 	bltz	t8,3d0 <_boot-0x800bfc30>
 3cc:	000002aa 	0x2aa
 3d0:	004c8201 	0x4c8201
 3d4:	91020000 	lbu	v0,0(t0)
 3d8:	0d050070 	jal	41401c0 <_boot-0x7bf7fe40>
 3dc:	01000000 	0x1000000
 3e0:	0c05c898 	jal	172260 <_boot-0x7ff4dda0>
 3e4:	00004880 	sll	t1,zero,0x2
 3e8:	6d9c0100 	0x6d9c0100
 3ec:	07000001 	bltz	t8,3f4 <_boot-0x800bfc0c>
 3f0:	000002aa 	0x2aa
 3f4:	004c9a01 	0x4c9a01
 3f8:	91020000 	lbu	v0,0(t0)
 3fc:	c4080070 	lwc1	$f8,112(zero)
 400:	01000002 	0x1000002
 404:	000025b1 	0x25b1
 408:	0c061000 	jal	184000 <_boot-0x7ff3c000>
 40c:	0000a080 	sll	s4,zero,0x2
 410:	b19c0100 	0xb19c0100
 414:	06000001 	bltz	s0,41c <_boot-0x800bfbe4>
 418:	000003cf 	sync	0xf
 41c:	004cb101 	0x4cb101
 420:	91020000 	lbu	v0,0(t0)
 424:	02aa0700 	0x2aa0700
 428:	b3010000 	0xb3010000
 42c:	0000004c 	syscall	0x1
 430:	07749102 	0x7749102
 434:	00000286 	0x286
 438:	0025b401 	0x25b401
 43c:	91020000 	lbu	v0,0(t0)
 440:	d9050070 	0xd9050070
 444:	01000001 	movf	zero,t0,$fcc0
 448:	0c06b0de 	jal	1ac378 <_boot-0x7ff13c88>
 44c:	00005480 	sll	t2,zero,0x12
 450:	f19c0100 	0xf19c0100
 454:	06000001 	bltz	s0,45c <_boot-0x800bfba4>
 458:	00000291 	0x291
 45c:	004cde01 	0x4cde01
 460:	91020000 	lbu	v0,0(t0)
 464:	6f740900 	0x6f740900
 468:	de010070 	0xde010070
 46c:	0000004c 	syscall	0x1
 470:	07049102 	0x7049102
 474:	0000025a 	0x25a
 478:	004ce001 	movf	gp,v0,$fcc3
 47c:	91020000 	lbu	v0,0(t0)
 480:	760a0070 	jalx	82801c0 <_boot-0x77e3fe40>0x70
 484:	01000002 	0x1000002
 488:	0c0704f7 	jal	1c13dc <_boot-0x7fefec24>
 48c:	00008080 	sll	s0,zero,0x2
 490:	069c0100 	0x69c0100
 494:	00000291 	0x291
 498:	004cf701 	0x4cf701
 49c:	91020000 	lbu	v0,0(t0)
 4a0:	6f740900 	0x6f740900
 4a4:	f7010070 	sdc1	$f1,112(t8)
 4a8:	0000004c 	syscall	0x1
 4ac:	06049102 	0x6049102
 4b0:	000003cf 	sync	0xf
 4b4:	004cf701 	0x4cf701
 4b8:	91020000 	lbu	v0,0(t0)
 4bc:	02640708 	0x2640708
 4c0:	f9010000 	0xf9010000
 4c4:	0000004c 	syscall	0x1
 4c8:	00709102 	0x709102
 4cc:	0003f400 	sll	s8,v1,0x10
 4d0:	2f000400 	sltiu	zero,t8,1024
 4d4:	04000002 	bltz	zero,4e0 <_boot-0x800bfb20>
 4d8:	00020801 	0x20801
 4dc:	036a0100 	0x36a0100
 4e0:	003a0000 	0x3a0000
 4e4:	07840000 	0x7840000
 4e8:	0bd4800c 	j	f520030 <_boot-0x70b9ffd0>
 4ec:	01b30000 	0x1b30000
 4f0:	01020000 	0x1020000
 4f4:	00014508 	0x14508
 4f8:	013f0300 	0x13f0300
 4fc:	19020000 	0x19020000
 500:	00000025 	move	zero,zero
 504:	47060102 	c1	0x1060102
 508:	03000001 	movf	zero,t8,$fcc0
 50c:	000000d7 	0xd7
 510:	00491b02 	0x491b02
 514:	02020000 	0x2020000
 518:	00018407 	0x18407
 51c:	05020200 	0x5020200
 520:	00000099 	0x99
 524:	0000de03 	sra	k1,zero,0x18
 528:	621d0200 	0x621d0200
 52c:	02000000 	0x2000000
 530:	039a0704 	0x39a0704
 534:	04040000 	0x4040000
 538:	746e6905 	jalx	1b9a414 <_boot-0x7e525bec>0x6905
 53c:	2c040500 	sltiu	a0,zero,1280
 540:	06000000 	bltz	s0,544 <_boot-0x800bfabc>
 544:	bb040310 	swr	a0,784(t8)
 548:	07000000 	bltz	t8,54c <_boot-0x800bfab4>
 54c:	000001bd 	0x1bd
 550:	00570903 	0x570903
 554:	07000000 	bltz	t8,558 <_boot-0x800bfaa8>
 558:	00000127 	0x127
 55c:	00570d03 	0x570d03
 560:	07040000 	0x7040000
 564:	000000e5 	0xe5
 568:	003e0e03 	0x3e0e03
 56c:	07080000 	0x7080000
 570:	00000179 	0x179
 574:	003e0f03 	0x3e0f03
 578:	070a0000 	0x70a0000
 57c:	00000153 	0x153
 580:	003e1003 	0x3e1003
 584:	000c0000 	sll	zero,t4,0x0
 588:	0001a103 	sra	s4,at,0x4
 58c:	76110300 	jalx	8440c00 <_boot-0x77c7f400>0x300
 590:	02000000 	0x2000000
 594:	03b20504 	0x3b20504
 598:	04020000 	0x4020000
 59c:	00039507 	0x39507
 5a0:	05080200 	0x5080200
 5a4:	000003ad 	0x3ad
 5a8:	90070802 	lbu	a3,2050(zero)
 5ac:	02000003 	0x2000003
 5b0:	00a30704 	0xa30704
 5b4:	01020000 	0x1020000
 5b8:	00014e06 	0x14e06
 5bc:	03fd0800 	0x3fd0800
 5c0:	0f010000 	jal	c040000 <_boot-0x74080000>
 5c4:	800c0784 	lb	t4,1924(zero)
 5c8:	00000040 	ssnop
 5cc:	01149c01 	0x1149c01
 5d0:	23090000 	addi	t1,t8,0
 5d4:	01000004 	sllv	zero,zero,t0
 5d8:	00005711 	0x5711
 5dc:	70910200 	0x70910200
 5e0:	040d0800 	0x40d0800
 5e4:	19010000 	0x19010000
 5e8:	800c07c4 	lb	t4,1988(zero)
 5ec:	000000a0 	0xa0
 5f0:	01389c01 	0x1389c01
 5f4:	23090000 	addi	t1,t8,0
 5f8:	01000004 	sllv	zero,zero,t0
 5fc:	00002c1b 	0x2c1b
 600:	70910200 	0x70910200
 604:	043a0a00 	0x43a0a00
 608:	2e010000 	sltiu	at,s0,0
 60c:	800c0864 	lb	t4,2148(zero)
 610:	000000dc 	0xdc
 614:	015c9c01 	0x15c9c01
 618:	18090000 	0x18090000
 61c:	01000004 	sllv	zero,zero,t0
 620:	00002c30 	0x2c30
 624:	70910200 	0x70910200
 628:	02f10a00 	0x2f10a00
 62c:	47010000 	c1	0x1010000
 630:	800c0940 	lb	t4,2368(zero)
 634:	0000012c 	0x12c
 638:	019c9c01 	0x19c9c01
 63c:	570b0000 	0x570b0000
 640:	01000003 	0x1000003
 644:	00005747 	0x5747
 648:	00910200 	0x910200
 64c:	00037b09 	0x37b09
 650:	57490100 	0x57490100
 654:	02000000 	0x2000000
 658:	18097091 	0x18097091
 65c:	01000004 	sllv	zero,zero,t0
 660:	00002c4a 	0x2c4a
 664:	74910200 	jalx	2440800 <_boot-0x7dc7f800>0x200
 668:	03290a00 	0x3290a00
 66c:	6c010000 	0x6c010000
 670:	800c0a6c 	lb	t4,2668(zero)
 674:	0000012c 	0x12c
 678:	01dc9c01 	0x1dc9c01
 67c:	570b0000 	0x570b0000
 680:	01000003 	0x1000003
 684:	0000576c 	0x576c
 688:	00910200 	0x910200
 68c:	00037b09 	0x37b09
 690:	576e0100 	0x576e0100
 694:	02000000 	0x2000000
 698:	18097091 	0x18097091
 69c:	01000004 	sllv	zero,zero,t0
 6a0:	00002c6f 	0x2c6f
 6a4:	74910200 	jalx	2440800 <_boot-0x7dc7f800>0x200
 6a8:	033c0c00 	0x33c0c00
 6ac:	91010000 	lbu	at,0(t0)
 6b0:	800c0b98 	lb	t4,2968(zero)
 6b4:	00000070 	0x70
 6b8:	e00a9c01 	sc	t2,-25599(zero)
 6bc:	01000003 	0x1000003
 6c0:	0c0c08a7 	jal	30229c <_boot-0x7fdbdd64>
 6c4:	00031880 	sll	v1,v1,0x2
 6c8:	6f9c0100 	0x6f9c0100
 6cc:	0d000002 	jal	4000008 <_boot-0x7c0bfff8>
 6d0:	0046444e 	0x46444e
 6d4:	0057a701 	0x57a701
 6d8:	91020000 	lbu	v0,0(t0)
 6dc:	03570b00 	0x3570b00
 6e0:	a7010000 	sh	at,0(t8)
 6e4:	00000057 	0x57
 6e8:	0b049102 	j	c124408 <_boot-0x73f9bbf8>
 6ec:	00000303 	sra	zero,zero,0xc
 6f0:	026fa701 	0x26fa701
 6f4:	91020000 	lbu	v0,0(t0)
 6f8:	037b0908 	0x37b0908
 6fc:	a9010000 	swl	at,0(t0)
 700:	00000057 	0x57
 704:	0e6c9102 	jal	9b24408 <_boot-0x7659bbf8>
 708:	a9010069 	swl	at,105(t0)
 70c:	00000057 	0x57
 710:	09609102 	j	5824408 <_boot-0x7a89bbf8>
 714:	00000418 	0x418
 718:	002caa01 	0x2caa01
 71c:	91020000 	lbu	v0,0(t0)
 720:	03d40970 	0x3d40970
 724:	ab010000 	swl	at,0(t8)
 728:	00000070 	0x70
 72c:	0e689102 	jal	9a24408 <_boot-0x7669bbf8>
 730:	ac010062 	sw	at,98(zero)
 734:	00000069 	0x69
 738:	00649102 	0x649102
 73c:	00570405 	0x570405
 740:	bb0f0000 	swr	t7,0(t8)
 744:	01000003 	0x1000003
 748:	0c0f20eb 	jal	3c83ac <_boot-0x7fcf7c54>
 74c:	00008080 	sll	s0,zero,0x2
 750:	109c0100 	beq	a0,gp,b54 <_boot-0x800bf4ac>
 754:	000001b2 	0x1b2
 758:	0057f901 	0x57f901
 75c:	0fa00000 	jal	e800000 <_boot-0x718c0000>
 760:	0120800c 	syscall	0x48200
 764:	9c010000 	0x9c010000
 768:	000002db 	0x2db
 76c:	0001bd09 	0x1bd09
 770:	57fb0100 	0x57fb0100
 774:	02000000 	0x2000000
 778:	23097491 	addi	t1,t8,29841
 77c:	01000004 	sllv	zero,zero,t0
 780:	000057fb 	0x57fb
 784:	70910200 	0x70910200
 788:	01006911 	0x1006911
 78c:	00002cfc 	0x2cfc
 790:	002c1200 	0x2c1200
 794:	fd010000 	0xfd010000
 798:	00000057 	0x57
 79c:	0002ea12 	0x2ea12
 7a0:	3efe0100 	0x3efe0100
 7a4:	00000000 	nop
 7a8:	00038113 	0x38113
 7ac:	011b0100 	0x11b0100
 7b0:	800c10c0 	lb	t4,4288(zero)
 7b4:	0000005c 	0x5c
 7b8:	03019c01 	0x3019c01
 7bc:	75140000 	jalx	4500000 <_boot-0x7bbc0000>0x0
 7c0:	01000003 	0x1000003
 7c4:	002c011b 	0x2c011b
 7c8:	91020000 	lbu	v0,0(t0)
 7cc:	bc150000 	cache	0x15,0(zero)
 7d0:	01000000 	0x1000000
 7d4:	111c013e 	beq	t0,gp,cd0 <_boot-0x800bf330>
 7d8:	0124800c 	syscall	0x49200
 7dc:	9c010000 	0x9c010000
 7e0:	00000327 	0x327
 7e4:	00041816 	0x41816
 7e8:	01400100 	0x1400100
 7ec:	0000002c 	0x2c
 7f0:	00709102 	0x709102
 7f4:	00035f15 	0x35f15
 7f8:	01630100 	0x1630100
 7fc:	800c1240 	lb	t4,4672(zero)
 800:	00000048 	0x48
 804:	034d9c01 	0x34d9c01
 808:	31160000 	andi	s6,t0,0x0
 80c:	01000000 	0x1000000
 810:	00570164 	0x570164
 814:	91020000 	lbu	v0,0(t0)
 818:	10130070 	beq	zero,s3,9dc <_boot-0x800bf624>
 81c:	01000001 	movf	zero,t0,$fcc0
 820:	1288016f 	beq	s4,t0,de0 <_boot-0x800bf220>
 824:	0024800c 	syscall	0x9200
 828:	9c010000 	0x9c010000
 82c:	00000373 	0x373
 830:	00035714 	0x35714
 834:	016f0100 	0x16f0100
 838:	00000057 	0x57
 83c:	00009102 	srl	s2,zero,0x4
 840:	00030f17 	0x30f17
 844:	01930100 	0x1930100
 848:	00000069 	0x69
 84c:	800c12ac 	lb	t4,4780(zero)
 850:	000000ac 	0xac
 854:	03e69c01 	0x3e69c01
 858:	2d140000 	sltiu	s4,t0,0
 85c:	01000004 	sllv	zero,zero,t0
 860:	026f0193 	0x26f0193
 864:	91020000 	lbu	v0,0(t0)
 868:	031d1400 	0x31d1400
 86c:	93010000 	lbu	at,0(t8)
 870:	00026f01 	0x26f01
 874:	04910200 	bgezal	a0,1078 <_boot-0x800bef88>
 878:	00002c14 	0x2c14
 87c:	01930100 	0x1930100
 880:	00000069 	0x69
 884:	18089102 	0x18089102
 888:	94010069 	lhu	at,105(zero)
 88c:	00006901 	0x6901
 890:	68910200 	0x68910200
 894:	00043416 	0x43416
 898:	01950100 	0x1950100
 89c:	0000026f 	0x26f
 8a0:	166c9102 	bne	s3,t4,fffe4cac <__bss_end+0x7ff233c8>
 8a4:	000003a7 	0x3a7
 8a8:	6f019601 	0x6f019601
 8ac:	02000002 	0x2000002
 8b0:	19007091 	blez	t0,1caf8 <_boot-0x800a3508>
 8b4:	00000000 	nop
 8b8:	00bb0d01 	0xbb0d01
 8bc:	03050000 	0x3050000
 8c0:	800c18d4 	lb	t4,6356(zero)
 8c4:	00008a00 	sll	s1,zero,0x8
 8c8:	ed000200 	swc3	$0,512(t0)
 8cc:	04000003 	bltz	zero,8dc <_boot-0x800bf724>
 8d0:	00038401 	0x38401
 8d4:	0c000000 	jal	0 <_boot-0x800c0000>
 8d8:	0c007880 	jal	1e200 <_boot-0x800a1e00>
 8dc:	74726380 	jalx	1c98e00 <_boot-0x7e427200>0x6380
 8e0:	00532e30 	0x532e30
 8e4:	726f772f 	0x726f772f
 8e8:	31382f6b 	andi	t8,t1,0x2f6b
 8ec:	50463131 	0x50463131
 8f0:	4f43755f 	c3	0x143755f
 8f4:	385f3253 	xori	ra,v0,0x3253
 8f8:	6e697038 	0x6e697038
 8fc:	5f54462f 	0x5f54462f
 900:	2f435145 	sltiu	v1,k0,20805
 904:	73616c66 	0x73616c66
 908:	75615f68 	jalx	5857da0 <_boot-0x7a868260>0x5f68
 90c:	725f6f74 	0x725f6f74
 910:	382f6e75 	xori	t7,at,0x6e75
 914:	46313131 	c.un.d	$fcc1,$f6,$f17
 918:	76655250 	jalx	9954940 <_boot-0x7676b6c0>0x5250
 91c:	696c5f41 	0x696c5f41
 920:	35765f62 	ori	s6,t3,0x5f62
 924:	4b44445f 	c2	0x144445f
 928:	6d6f435f 	0x6d6f435f
 92c:	656e6962 	0x656e6962
 930:	74736554 	jalx	1cd9550 <_boot-0x7e3e6ab0>0x6554
 934:	3931765f 	xori	s1,t1,0x765f
 938:	662f362e 	0x662f362e
 93c:	6873616c 	0x6873616c
 940:	47007066 	c1	0x1007066
 944:	4120554e 	0x4120554e
 948:	2e322053 	sltiu	s2,s1,8275
 94c:	302e3432 	andi	t6,at,0x3432
 950:	a3800100 	sb	zero,256(gp)
 954:	02000000 	0x2000000
 958:	00040100 	sll	zero,a0,0x4
 95c:	d0010400 	0xd0010400
 960:	58000003 	0x58000003
 964:	f0800c13 	0xf0800c13
 968:	2e800c13 	sltiu	zero,s4,3091
 96c:	2e2e2f2e 	sltiu	t6,s1,12078
 970:	2f2e2e2f 	sltiu	t6,t9,11823
 974:	2e2f2e2e 	sltiu	t7,s1,11822
 978:	2e2e2f2e 	sltiu	t6,s1,12078
 97c:	77656e2f 	jalx	d95b8bc <_boot-0x72764744>0x6e2f
 980:	2f62696c 	sltiu	v0,k1,26988
 984:	6362696c 	0x6362696c
 988:	63616d2f 	0x63616d2f
 98c:	656e6968 	0x656e6968
 990:	70696d2f 	0x70696d2f
 994:	656d2f73 	0x656d2f73
 998:	706d636d 	0x706d636d
 99c:	2f00532e 	sltiu	zero,t8,21294
 9a0:	64677963 	0x64677963
 9a4:	65766972 	0x65766972
 9a8:	522f672f 	0x522f672f
 9ac:	31384c54 	andi	t8,t1,0x4c54
 9b0:	50443836 	0x50443836
 9b4:	77656e2f 	jalx	d95b8bc <_boot-0x72764744>0x6e2f
 9b8:	2f62696c 	sltiu	v0,k1,26988
 9bc:	6b647372 	0x6b647372
 9c0:	352e312d 	ori	t6,t1,0x312d
 9c4:	6d2f362e 	0x6d2f362e
 9c8:	2d737069 	sltiu	s3,t3,28777
 9cc:	2f666c65 	sltiu	a2,k1,27749
 9d0:	6c77656e 	0x6c77656e
 9d4:	6c2f6269 	0x6c2f6269
 9d8:	2f636269 	sltiu	v1,k1,25193
 9dc:	6863616d 	0x6863616d
 9e0:	2f656e69 	sltiu	a1,k1,28265
 9e4:	7370696d 	0x7370696d
 9e8:	554e4700 	0x554e4700
 9ec:	20534120 	addi	s3,v0,16672
 9f0:	39312e32 	xori	s1,t1,0x2e32
 9f4:	0032392e 	0x32392e
 9f8:	00a38001 	0xa38001
 9fc:	00020000 	sll	zero,v0,0x0
 a00:	00000415 	0x415
 a04:	04590104 	0x4590104
 a08:	13f00000 	beq	ra,s0,a0c <_boot-0x800bf5f4>
 a0c:	14a8800c 	bne	a1,t0,fffe0a40 <__bss_end+0x7ff1f15c>
 a10:	2e2e800c 	sltiu	t6,s1,-32756
 a14:	2f2e2e2f 	sltiu	t6,t9,11823
 a18:	2e2f2e2e 	sltiu	t7,s1,11822
 a1c:	2e2e2f2e 	sltiu	t6,s1,12078
 a20:	2f2e2e2f 	sltiu	t6,t9,11823
 a24:	6c77656e 	0x6c77656e
 a28:	6c2f6269 	0x6c2f6269
 a2c:	2f636269 	sltiu	v1,k1,25193
 a30:	6863616d 	0x6863616d
 a34:	2f656e69 	sltiu	a1,k1,28265
 a38:	7370696d 	0x7370696d
 a3c:	6d656d2f 	0x6d656d2f
 a40:	2e746573 	sltiu	s4,s3,25971
 a44:	632f0053 	0x632f0053
 a48:	72646779 	0x72646779
 a4c:	2f657669 	sltiu	a1,k1,30313
 a50:	54522f67 	0x54522f67
 a54:	3631384c 	ori	s1,s1,0x384c
 a58:	2f504438 	sltiu	s0,k0,17464
 a5c:	6c77656e 	0x6c77656e
 a60:	722f6269 	0x722f6269
 a64:	2d6b6473 	sltiu	t3,t3,25715
 a68:	2e352e31 	sltiu	s5,s1,11825
 a6c:	696d2f36 	0x696d2f36
 a70:	652d7370 	0x652d7370
 a74:	6e2f666c 	0x6e2f666c
 a78:	696c7765 	0x696c7765
 a7c:	696c2f62 	0x696c2f62
 a80:	6d2f6362 	0x6d2f6362
 a84:	69686361 	0x69686361
 a88:	6d2f656e 	0x6d2f656e
 a8c:	00737069 	0x737069
 a90:	20554e47 	addi	s5,v0,20039
 a94:	32205341 	andi	zero,s1,0x5341
 a98:	2e39312e 	sltiu	t9,s1,12590
 a9c:	01003239 	0x1003239
 aa0:	Address 0x0000000000000aa0 is out of bounds.


Disassembly of section .debug_abbrev:

00000000 <.debug_abbrev>:
   0:	25011101 	addiu	at,t0,4353
   4:	030b130e 	0x30b130e
   8:	110e1b0e 	beq	t0,t6,6c44 <_boot-0x800b93bc>
   c:	10061201 	beq	zero,a2,4814 <_boot-0x800bb7ec>
  10:	02000017 	0x2000017
  14:	0b0b0024 	j	c2c0090 <_boot-0x73dfff70>
  18:	0e030b3e 	jal	80c2cf8 <_boot-0x77ffd308>
  1c:	16030000 	bne	s0,v1,20 <_boot-0x800bffe0>
  20:	3a0e0300 	xori	t6,s0,0x300
  24:	490b3b0b 	0x490b3b0b
  28:	04000013 	bltz	zero,78 <_boot-0x800bff88>
  2c:	0b0b0024 	j	c2c0090 <_boot-0x73dfff70>
  30:	08030b3e 	j	c2cf8 <_boot-0x7fffd308>
  34:	0f050000 	jal	c140000 <_boot-0x73f80000>
  38:	490b0b00 	0x490b0b00
  3c:	06000013 	bltz	s0,8c <_boot-0x800bff74>
  40:	0b0b0113 	j	c2c044c <_boot-0x73dffbb4>
  44:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
  48:	00001301 	0x1301
  4c:	03000d07 	0x3000d07
  50:	3b0b3a0e 	xori	t3,t8,0x3a0e
  54:	3813490b 	xori	s3,zero,0x490b
  58:	0800000b 	j	2c <_boot-0x800bffd4>
  5c:	193f012e 	0x193f012e
  60:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
  64:	19270b3b 	0x19270b3b
  68:	06120111 	0x6120111
  6c:	42971840 	c0	0x971840
  70:	00130119 	0x130119
  74:	00050900 	sll	at,a1,0x4
  78:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
  7c:	13490b3b 	beq	k0,t1,2d6c <_boot-0x800bd294>
  80:	00001802 	srl	v1,zero,0x0
  84:	0300340a 	0x300340a
  88:	3b0b3a08 	xori	t3,t8,0x3a08
  8c:	0213490b 	0x213490b
  90:	0b000018 	j	c000060 <_boot-0x740bffa0>
  94:	193f002e 	0x193f002e
  98:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
  9c:	01110b3b 	0x1110b3b
  a0:	18400612 	blez	v0,18ec <_boot-0x800be714>
  a4:	00194297 	0x194297
  a8:	002e0c00 	0x2e0c00
  ac:	0e03193f 	jal	80c64fc <_boot-0x77ff9b04>
  b0:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
  b4:	06120111 	0x6120111
  b8:	42961840 	c0	0x961840
  bc:	0d000019 	jal	4000064 <_boot-0x7c0bff9c>
  c0:	193f012e 	0x193f012e
  c4:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
  c8:	19270b3b 	0x19270b3b
  cc:	01111349 	0x1111349
  d0:	18400612 	blez	v0,191c <_boot-0x800be6e4>
  d4:	01194296 	0x1194296
  d8:	0e000013 	jal	800004c <_boot-0x780bffb4>
  dc:	0e030013 	jal	80c004c <_boot-0x77ffffb4>
  e0:	0000193c 	0x193c
  e4:	0300340f 	0x300340f
  e8:	3b0b3a0e 	xori	t3,t8,0x3a0e
  ec:	0213490b 	0x213490b
  f0:	10000018 	b	154 <_boot-0x800bfeac>
  f4:	193f012e 	0x193f012e
  f8:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
  fc:	13490b3b 	beq	k0,t1,2dec <_boot-0x800bd214>
 100:	1301193c 	beq	t8,at,65f4 <_boot-0x800b9a0c>
 104:	18110000 	0x18110000
 108:	12000000 	beqz	s0,10c <_boot-0x800bfef4>
 10c:	0111010b 	0x111010b
 110:	00000612 	0x612
 114:	3f012e13 	0x3f012e13
 118:	3a0e0319 	xori	t6,s0,0x319
 11c:	270b3b0b 	addiu	t3,t8,15115
 120:	3c134919 	lui	s3,0x4919
 124:	00130119 	0x130119
 128:	00051400 	sll	v0,a1,0x10
 12c:	00001349 	0x1349
 130:	3f012e15 	0x3f012e15
 134:	3a0e0319 	xori	t6,s0,0x319
 138:	490b3b0b 	0x490b3b0b
 13c:	00193c13 	0x193c13
 140:	01011600 	0x1011600
 144:	13011349 	beq	t8,at,4e6c <_boot-0x800bb194>
 148:	21170000 	addi	s7,t0,0
 14c:	2f134900 	sltiu	s3,t8,18688
 150:	1800000b 	blez	zero,180 <_boot-0x800bfe80>
 154:	0b0b000f 	j	c2c003c <_boot-0x73dfffc4>
 158:	34190000 	li	t9,0x0
 15c:	3a0e0300 	xori	t6,s0,0x300
 160:	490b3b0b 	0x490b3b0b
 164:	3c193f13 	lui	t9,0x3f13
 168:	1a000019 	blez	s0,1d0 <_boot-0x800bfe30>
 16c:	0e030034 	jal	80c00d0 <_boot-0x77ffff30>
 170:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
 174:	193f1349 	0x193f1349
 178:	00001802 	srl	v1,zero,0x0
 17c:	01110100 	0x1110100
 180:	0b130e25 	j	c4c3894 <_boot-0x73bfc76c>
 184:	0e1b0e03 	jal	86c380c <_boot-0x779fc7f4>
 188:	06120111 	0x6120111
 18c:	00001710 	0x1710
 190:	03001602 	0x3001602
 194:	3b0b3a0e 	xori	t3,t8,0x3a0e
 198:	0013490b 	0x13490b
 19c:	00240300 	0x240300
 1a0:	0b3e0b0b 	j	cf82c2c <_boot-0x7313d3d4>
 1a4:	00000e03 	sra	at,zero,0x18
 1a8:	0b002404 	j	c009010 <_boot-0x740b6ff0>
 1ac:	030b3e0b 	0x30b3e0b
 1b0:	05000008 	bltz	t0,1d4 <_boot-0x800bfe2c>
 1b4:	193f012e 	0x193f012e
 1b8:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 1bc:	19270b3b 	0x19270b3b
 1c0:	06120111 	0x6120111
 1c4:	42971840 	c0	0x971840
 1c8:	00130119 	0x130119
 1cc:	00050600 	sll	zero,a1,0x18
 1d0:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 1d4:	13490b3b 	beq	k0,t1,2ec4 <_boot-0x800bd13c>
 1d8:	00001802 	srl	v1,zero,0x0
 1dc:	03003407 	0x3003407
 1e0:	3b0b3a0e 	xori	t3,t8,0x3a0e
 1e4:	0213490b 	0x213490b
 1e8:	08000018 	j	60 <_boot-0x800bffa0>
 1ec:	193f012e 	0x193f012e
 1f0:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 1f4:	19270b3b 	0x19270b3b
 1f8:	01111349 	0x1111349
 1fc:	18400612 	blez	v0,1a48 <_boot-0x800be5b8>
 200:	01194297 	0x1194297
 204:	09000013 	j	400004c <_boot-0x7c0bffb4>
 208:	08030005 	j	c0014 <_boot-0x7fffffec>
 20c:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
 210:	18021349 	0x18021349
 214:	2e0a0000 	sltiu	t2,s0,0
 218:	03193f01 	0x3193f01
 21c:	3b0b3a0e 	xori	t3,t8,0x3a0e
 220:	1119270b 	beq	t0,t9,9e50 <_boot-0x800b61b0>
 224:	40061201 	0x40061201
 228:	19429718 	0x19429718
 22c:	01000000 	0x1000000
 230:	0e250111 	jal	8940444 <_boot-0x7777fbbc>
 234:	0e030b13 	jal	80c2c4c <_boot-0x77ffd3b4>
 238:	01110e1b 	0x1110e1b
 23c:	17100612 	bne	t8,s0,1a88 <_boot-0x800be578>
 240:	24020000 	li	v0,0
 244:	3e0b0b00 	0x3e0b0b00
 248:	000e030b 	0xe030b
 24c:	00160300 	sll	zero,s6,0xc
 250:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 254:	13490b3b 	beq	k0,t1,2f44 <_boot-0x800bd0bc>
 258:	24040000 	li	a0,0
 25c:	3e0b0b00 	0x3e0b0b00
 260:	0008030b 	0x8030b
 264:	000f0500 	sll	zero,t7,0x14
 268:	13490b0b 	beq	k0,t1,2e98 <_boot-0x800bd168>
 26c:	13060000 	beq	t8,a2,270 <_boot-0x800bfd90>
 270:	3a0b0b01 	xori	t3,s0,0xb01
 274:	010b3b0b 	0x10b3b0b
 278:	07000013 	bltz	t8,2c8 <_boot-0x800bfd38>
 27c:	0e03000d 	jal	80c0034 <_boot-0x77ffffcc>
 280:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
 284:	0b381349 	j	ce04d24 <_boot-0x732bb2dc>
 288:	2e080000 	sltiu	t0,s0,0
 28c:	03193f01 	0x3193f01
 290:	3b0b3a0e 	xori	t3,t8,0x3a0e
 294:	1119270b 	beq	t0,t9,9ec4 <_boot-0x800b613c>
 298:	40061201 	0x40061201
 29c:	19429718 	0x19429718
 2a0:	00001301 	0x1301
 2a4:	03003409 	0x3003409
 2a8:	3b0b3a0e 	xori	t3,t8,0x3a0e
 2ac:	0213490b 	0x213490b
 2b0:	0a000018 	j	8000060 <_boot-0x780bffa0>
 2b4:	193f012e 	0x193f012e
 2b8:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 2bc:	19270b3b 	0x19270b3b
 2c0:	06120111 	0x6120111
 2c4:	42961840 	c0	0x961840
 2c8:	00130119 	0x130119
 2cc:	00050b00 	sll	at,a1,0xc
 2d0:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 2d4:	13490b3b 	beq	k0,t1,2fc4 <_boot-0x800bd03c>
 2d8:	00001802 	srl	v1,zero,0x0
 2dc:	3f002e0c 	0x3f002e0c
 2e0:	3a0e0319 	xori	t6,s0,0x319
 2e4:	270b3b0b 	addiu	t3,t8,15115
 2e8:	12011119 	beq	s0,at,4750 <_boot-0x800bb8b0>
 2ec:	97184006 	lhu	t8,16390(t8)
 2f0:	00001942 	srl	v1,zero,0x5
 2f4:	0300050d 	break	0x300,0x14
 2f8:	3b0b3a08 	xori	t3,t8,0x3a08
 2fc:	0213490b 	0x213490b
 300:	0e000018 	jal	8000060 <_boot-0x780bffa0>
 304:	08030034 	j	c00d0 <_boot-0x7fffff30>
 308:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
 30c:	18021349 	0x18021349
 310:	2e0f0000 	sltiu	t7,s0,0
 314:	3a0e0300 	xori	t6,s0,0x300
 318:	270b3b0b 	addiu	t3,t8,15115
 31c:	12011119 	beq	s0,at,4784 <_boot-0x800bb87c>
 320:	96184006 	lhu	t8,16390(s0)
 324:	00001942 	srl	v1,zero,0x5
 328:	3f012e10 	0x3f012e10
 32c:	3a0e0319 	xori	t6,s0,0x319
 330:	270b3b0b 	addiu	t3,t8,15115
 334:	11134919 	beq	t0,s3,1279c <_boot-0x800ad864>
 338:	40061201 	0x40061201
 33c:	19429618 	0x19429618
 340:	00001301 	0x1301
 344:	03003411 	0x3003411
 348:	3b0b3a08 	xori	t3,t8,0x3a08
 34c:	0013490b 	0x13490b
 350:	00341200 	0x341200
 354:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 358:	13490b3b 	beq	k0,t1,3048 <_boot-0x800bcfb8>
 35c:	2e130000 	sltiu	s3,s0,0
 360:	03193f01 	0x3193f01
 364:	3b0b3a0e 	xori	t3,t8,0x3a0e
 368:	11192705 	beq	t0,t9,9f80 <_boot-0x800b6080>
 36c:	40061201 	0x40061201
 370:	19429718 	0x19429718
 374:	00001301 	0x1301
 378:	03000514 	0x3000514
 37c:	3b0b3a0e 	xori	t3,t8,0x3a0e
 380:	02134905 	0x2134905
 384:	15000018 	bnez	t0,3e8 <_boot-0x800bfc18>
 388:	193f012e 	0x193f012e
 38c:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 390:	1927053b 	0x1927053b
 394:	06120111 	0x6120111
 398:	42961840 	c0	0x961840
 39c:	00130119 	0x130119
 3a0:	00341600 	0x341600
 3a4:	0b3a0e03 	j	ce8380c <_boot-0x7323c7f4>
 3a8:	1349053b 	beq	k0,t1,1898 <_boot-0x800be768>
 3ac:	00001802 	srl	v1,zero,0x0
 3b0:	3f012e17 	0x3f012e17
 3b4:	3a0e0319 	xori	t6,s0,0x319
 3b8:	27053b0b 	addiu	a1,t8,15115
 3bc:	11134919 	beq	t0,s3,12824 <_boot-0x800ad7dc>
 3c0:	40061201 	0x40061201
 3c4:	19429718 	0x19429718
 3c8:	00001301 	0x1301
 3cc:	03003418 	0x3003418
 3d0:	3b0b3a08 	xori	t3,t8,0x3a08
 3d4:	02134905 	0x2134905
 3d8:	19000018 	blez	t0,43c <_boot-0x800bfbc4>
 3dc:	0e030034 	jal	80c00d0 <_boot-0x77ffff30>
 3e0:	0b3b0b3a 	j	cec2ce8 <_boot-0x731fd318>
 3e4:	193f1349 	0x193f1349
 3e8:	00001802 	srl	v1,zero,0x0
 3ec:	00110100 	sll	zero,s1,0x4
 3f0:	01110610 	0x1110610
 3f4:	08030112 	j	c0448 <_boot-0x7ffffbb8>
 3f8:	0825081b 	j	94206c <_boot-0x7f77df94>
 3fc:	00000513 	0x513
 400:	00110100 	sll	zero,s1,0x4
 404:	01110610 	0x1110610
 408:	08030112 	j	c0448 <_boot-0x7ffffbb8>
 40c:	0825081b 	j	94206c <_boot-0x7f77df94>
 410:	00000513 	0x513
 414:	00110100 	sll	zero,s1,0x4
 418:	01110610 	0x1110610
 41c:	08030112 	j	c0448 <_boot-0x7ffffbb8>
 420:	0825081b 	j	94206c <_boot-0x7f77df94>
 424:	00000513 	0x513
	...

Disassembly of section .debug_aranges:

00000000 <.debug_aranges>:
   0:	0000001c 	0x1c
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	800c0078 	lb	t4,120(zero)
  14:	000003d0 	0x3d0
	...
  20:	0000001c 	0x1c
  24:	02910002 	0x2910002
  28:	00040000 	sll	zero,a0,0x0
  2c:	00000000 	nop
  30:	800c0448 	lb	t4,1096(zero)
  34:	0000033c 	0x33c
	...
  40:	0000001c 	0x1c
  44:	04cd0002 	0x4cd0002
  48:	00040000 	sll	zero,a0,0x0
  4c:	00000000 	nop
  50:	800c0784 	lb	t4,1924(zero)
  54:	00000bd4 	0xbd4
	...
  60:	0000001c 	0x1c
  64:	08c50002 	j	3140008 <_boot-0x7cf7fff8>
  68:	00040000 	sll	zero,a0,0x0
  6c:	00000000 	nop
  70:	800c0000 	lb	t4,0(zero)
  74:	00000078 	0x78
	...
  80:	0000001c 	0x1c
  84:	09530002 	j	54c0008 <_boot-0x7abffff8>
  88:	00040000 	sll	zero,a0,0x0
  8c:	00000000 	nop
  90:	800c1358 	lb	t4,4952(zero)
  94:	00000098 	0x98
	...
  a0:	0000001c 	0x1c
  a4:	09fa0002 	j	7e80008 <_boot-0x7823fff8>
  a8:	00040000 	sll	zero,a0,0x0
  ac:	00000000 	nop
  b0:	800c13f0 	lb	t4,5104(zero)
  b4:	000000b8 	0xb8
	...

Disassembly of section .debug_line:

00000000 <.debug_line>:
   0:	000000e3 	0xe3
   4:	003e0002 	0x3e0002
   8:	01010000 	0x1010000
   c:	000d0efb 	0xd0efb
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	2e707061 	sltiu	s0,s3,28769
  20:	00000063 	0x63
  24:	70736200 	0x70736200
  28:	0000682e 	0x682e
  2c:	6c660000 	0x6c660000
  30:	2e687361 	sltiu	t0,s3,29537
  34:	00000068 	0x68
  38:	75623c00 	jalx	588f000 <_boot-0x7a831000>0x3c00
  3c:	2d746c69 	sltiu	s4,t3,27753
  40:	003e6e69 	0x3e6e69
  44:	00000000 	nop
  48:	78020500 	0x78020500
  4c:	18800c00 	blez	a0,3050 <_boot-0x800bcfb0>
  50:	0200bcf4 	0x200bcf4
  54:	00bb0304 	0xbb0304
  58:	49030402 	0x49030402
  5c:	01040200 	0x1040200
  60:	0200ba06 	0x200ba06
  64:	f0060204 	0xf0060204
  68:	01040200 	0x1040200
  6c:	0806ba06 	j	1ae818 <_boot-0x7ff117e8>
  70:	bc3f0841 	cache	0x1f,2113(at)
  74:	01040200 	0x1040200
  78:	08064a06 	j	192818 <_boot-0x7ff2d7e8>
  7c:	040200e8 	0x40200e8
  80:	064a0601 	0x64a0601
  84:	00bdaf08 	0xbdaf08
  88:	06010402 	bgez	s0,1094 <_boot-0x800bef6c>
  8c:	af08064a 	sw	t0,1610(t8)
  90:	03f43f08 	0x3f43f08
  94:	b008820a 	0xb008820a
  98:	7708bbbe 	jalx	c22eef8 <_boot-0x73e91108>0xbbbe
  9c:	51844bf3 	0x51844bf3
  a0:	18019802 	0x18019802
  a4:	03040200 	0x3040200
  a8:	0200ba06 	0x200ba06
  ac:	06ba0104 	0x6ba0104
  b0:	83f20903 	lb	s2,2307(ra)
  b4:	ad083f08 	sw	t0,16136(t0)
  b8:	14037508 	bne	zero,v1,1d4dc <_boot-0x800a2b24>
  bc:	ae08f382 	sw	t0,-3198(s0)
  c0:	08ad0813 	j	2b4204c <_boot-0x7d57dfb4>
  c4:	0200133f 	0x200133f
  c8:	00bb0304 	0xbb0304
  cc:	02030402 	0x2030402
  d0:	02001140 	0x2001140
  d4:	ba060104 	swr	a2,260(s0)
  d8:	2802f606 	slti	v0,zero,-2554
  dc:	bc851314 	cache	0x5,4884(a0)
  e0:	18028483 	0x18028483
  e4:	c8010100 	lwc2	$1,256(zero)
  e8:	02000000 	0x2000000
  ec:	00002d00 	sll	a1,zero,0x14
  f0:	fb010100 	0xfb010100
  f4:	01000d0e 	0x1000d0e
  f8:	00010101 	0x10101
  fc:	00010000 	sll	zero,at,0x0
 100:	72000100 	0x72000100
 104:	635f786c 	0x635f786c
 108:	65686361 	0x65686361
 10c:	0000632e 	0x632e
 110:	2f2e0000 	sltiu	t6,t9,0
 114:	2e707362 	sltiu	s0,s3,29538
 118:	00000068 	0x68
 11c:	05000000 	bltz	t0,120 <_boot-0x800bfee0>
 120:	0c044802 	jal	112008 <_boot-0x7ffadff8>
 124:	01340380 	0x1340380
 128:	02003f08 	0x2003f08
 12c:	00f40304 	0xf40304
 130:	80030402 	lb	v1,1026(zero)
 134:	01040200 	0x1040200
 138:	0806ba06 	j	1ae818 <_boot-0x7ff117e8>
 13c:	081003b2 	j	400ec8 <_boot-0x7fcbf138>
 140:	003f083c 	0x3f083c
 144:	f4030402 	sdc1	$f3,1026(zero)
 148:	03040200 	0x3040200
 14c:	04020080 	0x4020080
 150:	06ba0601 	0x6ba0601
 154:	1103b208 	beq	t0,v1,fffec978 <__bss_end+0x7ff2b094>
 158:	3f083c08 	0x3f083c08
 15c:	03040200 	0x3040200
 160:	040200f4 	0x40200f4
 164:	02008003 	0x2008003
 168:	ba060104 	swr	a2,260(s0)
 16c:	03b20806 	srlv	at,s2,sp
 170:	bb3c0810 	swr	gp,2064(t9)
 174:	bb83834c 	swr	v1,-31924(gp)
 178:	08100384 	j	400e10 <_boot-0x7fcbf1f0>
 17c:	834cbb3c 	lb	t4,-17604(k0)
 180:	0383bb83 	0x383bb83
 184:	f43c0812 	sdc1	$f28,2066(at)
 188:	08f4844c 	j	3d21130 <_boot-0x7c39eed0>
 18c:	4e8583b1 	c3	0x8583b1
 190:	853d08f4 	lh	sp,2292(t1)
 194:	0811034b 	j	440d2c <_boot-0x7fc7f2d4>
 198:	833e083c 	lb	s8,2108(t9)
 19c:	83bb8384 	lb	k1,-31868(sp)
 1a0:	3c081103 	lui	t0,0x1103
 1a4:	84837608 	lh	v1,30216(a0)
 1a8:	84f383f4 	lh	s3,-31756(a3)
 1ac:	140284bb 	bne	zero,v0,fffe149c <__bss_end+0x7ff1fbb8>
 1b0:	cd010100 	lwc3	$1,256(t0)
 1b4:	02000001 	movf	zero,s0,$fcc0
 1b8:	00003500 	sll	a2,zero,0x14
 1bc:	fb010100 	0xfb010100
 1c0:	01000d0e 	0x1000d0e
 1c4:	00010101 	0x10101
 1c8:	00010000 	sll	zero,at,0x0
 1cc:	66000100 	0x66000100
 1d0:	6873616c 	0x6873616c
 1d4:	2e70665f 	sltiu	s0,s3,26207
 1d8:	00000063 	0x63
 1dc:	70736200 	0x70736200
 1e0:	0000682e 	0x682e
 1e4:	6c660000 	0x6c660000
 1e8:	2e687361 	sltiu	t0,s3,29537
 1ec:	00000068 	0x68
 1f0:	05000000 	bltz	t0,1f4 <_boot-0x800bfe0c>
 1f4:	0c078402 	jal	1e1008 <_boot-0x7fedeff8>
 1f8:	010f0380 	0x10f0380
 1fc:	01040200 	0x1040200
 200:	040200be 	0x40200be
 204:	08f3f301 	j	3cfcc04 <_boot-0x7c3c33fc>
 208:	bbbbbd40 	swr	k1,-17088(sp)
 20c:	00f4f3f3 	0xf4f3f3
 210:	f6010402 	sdc1	$f1,1026(s0)
 214:	01040200 	0x1040200
 218:	040200f3 	0x40200f3
 21c:	08bbbb01 	j	2eeec04 <_boot-0x7d1d13fc>
 220:	f3bbf541 	0xf3bbf541
 224:	f483bbf3 	sdc1	$f3,-17421(a0)
 228:	00f384f3 	0xf384f3
 22c:	84010402 	lh	at,1026(zero)
 230:	01040200 	0x1040200
 234:	04020084 	0x4020084
 238:	0200f301 	0x200f301
 23c:	f3bb0104 	0xf3bb0104
 240:	41087808 	0x41087808
 244:	bbf3f3bb 	swr	s3,-3141(ra)
 248:	84f3f583 	lh	s3,-2685(a3)
 24c:	083d08bb 	j	f422ec <_boot-0x7f17dd14>
 250:	00f5f3ad 	0xf5f3ad
 254:	85010402 	lh	at,1026(t0)
 258:	01040200 	0x1040200
 25c:	04020084 	0x4020084
 260:	0200f301 	0x200f301
 264:	f5bb0104 	sdc1	$f27,260(t5)
 268:	41087808 	0x41087808
 26c:	bbf3f3bb 	swr	s3,-3141(ra)
 270:	84f3f583 	lh	s3,-2685(a3)
 274:	083d08bb 	j	f422ec <_boot-0x7f17dd14>
 278:	00f5f3ad 	0xf5f3ad
 27c:	85010402 	lh	at,1026(t0)
 280:	01040200 	0x1040200
 284:	04020084 	0x4020084
 288:	0200f301 	0x200f301
 28c:	f5bb0104 	sdc1	$f27,260(t5)
 290:	bbbb7808 	swr	k1,30728(sp)
 294:	f3f3bb83 	0xf3f3bb83
 298:	080f03f3 	j	3c0fcc <_boot-0x7fcff034>
 29c:	bcb0083c 	cache	0x10,2108(a1)
 2a0:	84f38484 	lh	s3,-31612(a3)
 2a4:	3d08bbbc 	0x3d08bbbc
 2a8:	f5f3ad08 	sdc1	$f19,-21240(t7)
 2ac:	e508bcbc 	swc1	$f8,-17220(t0)
 2b0:	02132402 	0x2132402
 2b4:	2c021324 	sltiu	v0,zero,4900
 2b8:	04020016 	0x4020016
 2bc:	08750302 	j	1d40c08 <_boot-0x7e37f3f8>
 2c0:	040200e4 	0x40200e4
 2c4:	06ba0601 	0x6ba0601
 2c8:	74080e03 	jalx	20380c <_boot-0x7febc7f4>0xe03
 2cc:	28023d08 	slti	v0,zero,15624
 2d0:	083e0813 	j	f8204c <_boot-0x7f13dfb4>
 2d4:	142802e5 	bne	at,t0,e6c <_boot-0x800bf194>
 2d8:	e5083e08 	swc1	$f8,15880(t0)
 2dc:	ea08e508 	swc2	$8,-6904(s0)
 2e0:	040200f3 	0x40200f3
 2e4:	02008601 	0x2008601
 2e8:	00840104 	0x840104
 2ec:	f3010402 	0xf3010402
 2f0:	01040200 	0x1040200
 2f4:	7808f4bb 	0x7808f4bb
 2f8:	bbf3bbf3 	swr	s3,-17421(ra)
 2fc:	84f3f483 	lh	s3,-2941(a3)
 300:	bbf87808 	swr	t8,30728(ra)
 304:	f4f3f3bb 	sdc1	$f19,-3141(a3)
 308:	01040200 	0x1040200
 30c:	040200f5 	0x40200f5
 310:	0200f301 	0x200f301
 314:	bcbb0104 	cache	0x1b,260(a1)
 318:	00154402 	srl	t0,s5,0x10
 31c:	06010402 	bgez	s0,1328 <_boot-0x800becd8>
 320:	08063c08 	j	18f020 <_boot-0x7ff30fe0>
 324:	084b863d 	j	12e18f4 <_boot-0x7edde70c>
 328:	bb3d087a 	swr	sp,2170(t9)
 32c:	f2140383 	0xf2140383
 330:	080b03f3 	j	2c0fcc <_boot-0x7fdff034>
 334:	f3bbf63c 	0xf3bbf63c
 338:	bbf3f3f3 	swr	s3,-3085(ra)
 33c:	85f3f583 	lh	s3,-2685(t7)
 340:	f3bbf3bb 	0xf3bbf3bb
 344:	01040200 	0x1040200
 348:	04020084 	0x4020084
 34c:	02008401 	0x2008401
 350:	00f30104 	0xf30104
 354:	bb010402 	swr	at,1026(t8)
 358:	f57a08f4 	sdc1	$f26,2292(t3)
 35c:	0885bcbb 	j	216f2ec <_boot-0x7df50d14>
 360:	f2210378 	0xf2210378
 364:	75083e08 	jalx	420f820 <_boot-0x7beb07e0>0x3e08
 368:	3d08bb4e 	0x3d08bb4e
 36c:	75083d08 	jalx	420f420 <_boot-0x7beb0be0>0x3d08
 370:	02040200 	0x2040200
 374:	040200b6 	0x40200b6
 378:	06ba0601 	0x6ba0601
 37c:	024b4408 	0x24b4408
 380:	01010014 	0x1010014
 384:	00000048 	0x48
 388:	001d0002 	srl	zero,sp,0x0
 38c:	01010000 	0x1010000
 390:	000d0efb 	0xd0efb
 394:	01010101 	0x1010101
 398:	01000000 	0x1000000
 39c:	00010000 	sll	zero,at,0x0
 3a0:	30747263 	andi	s4,v1,0x7263
 3a4:	0000532e 	0x532e
 3a8:	00000000 	nop
 3ac:	00000205 	0x205
 3b0:	2803800c 	slti	v1,zero,-32756
 3b4:	bb4b4b01 	swr	t3,19201(k0)
 3b8:	504b4b83 	0x504b4b83
 3bc:	4c4b834b 	0x4c4b834b
 3c0:	4b4b4b4b 	c2	0x14b4b4b
 3c4:	83848951 	lb	a0,-30383(gp)
 3c8:	024b504b 	0x24b504b
 3cc:	01010004 	sllv	zero,at,t0
 3d0:	00000085 	0x85
 3d4:	004a0002 	0x4a0002
 3d8:	01010000 	0x1010000
 3dc:	000d0efb 	0xd0efb
 3e0:	01010101 	0x1010101
 3e4:	01000000 	0x1000000
 3e8:	2e010000 	sltiu	at,s0,0
 3ec:	2e2e2f2e 	sltiu	t6,s1,12078
 3f0:	2f2e2e2f 	sltiu	t6,t9,11823
 3f4:	2e2f2e2e 	sltiu	t7,s1,11822
 3f8:	2e2e2f2e 	sltiu	t6,s1,12078
 3fc:	77656e2f 	jalx	d95b8bc <_boot-0x72764744>0x6e2f
 400:	2f62696c 	sltiu	v0,k1,26988
 404:	6362696c 	0x6362696c
 408:	63616d2f 	0x63616d2f
 40c:	656e6968 	0x656e6968
 410:	70696d2f 	0x70696d2f
 414:	6d000073 	0x6d000073
 418:	6d636d65 	0x6d636d65
 41c:	00532e70 	0x532e70
 420:	00000001 	movf	zero,zero,$fcc0
 424:	58020500 	0x58020500
 428:	03800c13 	0x3800c13
 42c:	4b0100d6 	c2	0x10100d6
 430:	4f4b4c4b 	c3	0x14b4c4b
 434:	4c4b4b4b 	0x4c4b4b4b
 438:	4e4b4f4b 	c3	0x4b4f4b
 43c:	4b4c4b4c 	c2	0x14c4b4c
 440:	4b4b4f4b 	c2	0x14b4f4b
 444:	4f4b4c4b 	c3	0x14b4c4b
 448:	4b4c4e4b 	c2	0x14c4e4b
 44c:	4b4b4b4b 	c2	0x14b4b4b
 450:	4b4f4b4c 	c2	0x14f4b4c
 454:	01000402 	0x1000402
 458:	00008f01 	0x8f01
 45c:	4a000200 	c2	0x200
 460:	01000000 	0x1000000
 464:	0d0efb01 	jal	43bec04 <_boot-0x7bd013fc>
 468:	01010100 	0x1010100
 46c:	00000001 	movf	zero,zero,$fcc0
 470:	01000001 	movf	zero,t0,$fcc0
 474:	2e2f2e2e 	sltiu	t7,s1,11822
 478:	2e2e2f2e 	sltiu	t6,s1,12078
 47c:	2f2e2e2f 	sltiu	t6,t9,11823
 480:	2e2f2e2e 	sltiu	t7,s1,11822
 484:	656e2f2e 	0x656e2f2e
 488:	62696c77 	0x62696c77
 48c:	62696c2f 	0x62696c2f
 490:	616d2f63 	0x616d2f63
 494:	6e696863 	0x6e696863
 498:	696d2f65 	0x696d2f65
 49c:	00007370 	0x7370
 4a0:	736d656d 	0x736d656d
 4a4:	532e7465 	0x532e7465
 4a8:	00000100 	sll	zero,zero,0x4
 4ac:	02050000 	0x2050000
 4b0:	800c13f0 	lb	t4,5104(zero)
 4b4:	0100c203 	0x100c203
 4b8:	4b4b4e4b 	c2	0x14b4e4b
 4bc:	09034d4b 	j	40d352c <_boot-0x7bfecad4>
 4c0:	4b4b4b4a 	c2	0x14b4b4a
 4c4:	4b4b4b4d 	c2	0x14b4b4d
 4c8:	4b4b4f4c 	c2	0x14b4f4c
 4cc:	4b4b4e4b 	c2	0x14b4e4b
 4d0:	4b4c4b4b 	c2	0x14c4b4b
 4d4:	4b4b4b4b 	c2	0x14b4b4b
 4d8:	4b4b4b4d 	c2	0x14b4b4d
 4dc:	4b4b4c4b 	c2	0x14b4c4b
 4e0:	4b4c4b4e 	c2	0x14c4b4e
 4e4:	024b4c4b 	0x24b4c4b
 4e8:	01010004 	sllv	zero,at,t0

Disassembly of section .debug_str:

00000000 <.debug_str>:
   0:	73616c66 	0x73616c66
   4:	74735f68 	jalx	1cd7da0 <_boot-0x7e3e8260>0x5f68
   8:	74637572 	jalx	18dd5c8 <_boot-0x7e7e2a38>0x7572
   c:	786c7200 	0x786c7200
  10:	6163695f 	0x6163695f
  14:	5f656863 	0x5f656863
  18:	61766e69 	0x61766e69
  1c:	6164696c 	0x6164696c
  20:	615f6574 	0x615f6574
  24:	6c006c6c 	0x6c006c6c
  28:	5f64616f 	0x5f64616f
  2c:	657a6973 	0x657a6973
  30:	414c4600 	0x414c4600
  34:	495f4853 	0x495f4853
  38:	772f0044 	jalx	cbc0110 <_boot-0x734ffef0>0x44
  3c:	2f6b726f 	sltiu	t3,k1,29295
  40:	31313138 	andi	s1,t1,0x3138
  44:	755f5046 	jalx	57d4118 <_boot-0x7a8ebee8>0x5046
  48:	32534f43 	andi	s3,s2,0x4f43
  4c:	7038385f 	0x7038385f
  50:	462f6e69 	c1	0x2f6e69
  54:	51455f54 	0x51455f54
  58:	6c662f43 	0x6c662f43
  5c:	5f687361 	0x5f687361
  60:	6f747561 	0x6f747561
  64:	6e75725f 	0x6e75725f
  68:	3131382f 	andi	s1,t1,0x382f
  6c:	52504631 	0x52504631
  70:	5f417665 	0x5f417665
  74:	5f62696c 	0x5f62696c
  78:	445f3576 	0x445f3576
  7c:	435f4b44 	c0	0x15f4b44
  80:	69626d6f 	0x69626d6f
  84:	6554656e 	0x6554656e
  88:	765f7473 	jalx	97dd1cc <_boot-0x768e2e34>0x7473
  8c:	362e3931 	ori	t6,s1,0x3931
  90:	616c662f 	0x616c662f
  94:	70666873 	0x70666873
  98:	6f687300 	0x6f687300
  9c:	69207472 	0x69207472
  a0:	7300746e 	0x7300746e
  a4:	74657a69 	jalx	195e9a4 <_boot-0x7e76165c>0x7a69
  a8:	00657079 	0x657079
  ac:	73616c66 	0x73616c66
  b0:	64615f68 	0x64615f68
  b4:	6d007264 	0x6d007264
  b8:	006e6961 	0x6e6961
  bc:	52535257 	0x52535257
  c0:	616c465f 	0x616c465f
  c4:	6f5f6873 	0x6f5f6873
  c8:	745f656e 	jalx	17d95b8 <_boot-0x7e8e6a48>0x656e
  cc:	635f6f77 	0x635f6f77
  d0:	6e6e6168 	0x6e6e6168
  d4:	49006c65 	bc2f	1b26c <_boot-0x800a4d94>
  d8:	3631544e 	ori	s1,s1,0x544e
  dc:	4e490055 	c3	0x490055
  e0:	55323354 	0x55323354
  e4:	735f6e00 	0x735f6e00
  e8:	6f746365 	0x6f746365
  ec:	72007372 	0x72007372
  f0:	645f786c 	0x645f786c
  f4:	68636163 	0x68636163
  f8:	6e695f65 	0x6e695f65
  fc:	696c6176 	0x696c6176
 100:	65746164 	0x65746164
 104:	6c6c615f 	0x6c6c615f
 108:	6d656d00 	0x6d656d00
 10c:	00706d63 	0x706d63
 110:	73616c46 	0x73616c46
 114:	6e555f68 	0x6e555f68
 118:	746f7270 	jalx	1bdc9c0 <_boot-0x7e4e3640>0x7270
 11c:	00746365 	0x746365
 120:	736d656d 	0x736d656d
 124:	73007465 	0x73007465
 128:	6f746365 	0x6f746365
 12c:	69735f72 	0x69735f72
 130:	7300657a 	0x7300657a
 134:	705f6973 	0x705f6973
 138:	6d74726f 	0x6d74726f
 13c:	49007061 	bc2f	1c2c4 <_boot-0x800a3d3c>
 140:	5538544e 	0x5538544e
 144:	736e7500 	0x736e7500
 148:	656e6769 	0x656e6769
 14c:	68632064 	0x68632064
 150:	66007261 	0x66007261
 154:	7367616c 	0x7367616c
 158:	73696400 	0x73696400
 15c:	656c6261 	0x656c6261
 160:	73616d5f 	0x73616d5f
 164:	5f726574 	0x5f726574
 168:	69676e65 	0x69676e65
 16c:	7500656e 	jalx	40195b8 <_boot-0x7c0a6a48>0x656e
 170:	00636573 	0x636573
 174:	64696472 	0x64696472
 178:	64646100 	0x64646100
 17c:	69775f72 	0x69775f72
 180:	00687464 	0x687464
 184:	726f6873 	0x726f6873
 188:	6e752074 	0x6e752074
 18c:	6e676973 	0x6e676973
 190:	69206465 	0x69206465
 194:	6c00746e 	0x6c00746e
 198:	5f64616f 	0x5f64616f
 19c:	72646461 	0x72646461
 1a0:	49505300 	0x49505300
 1a4:	414c465f 	0x414c465f
 1a8:	535f4853 	0x535f4853
 1ac:	43555254 	c0	0x1555254
 1b0:	6c460054 	0x6c460054
 1b4:	5f687361 	0x5f687361
 1b8:	44494452 	0x44494452
 1bc:	616c6600 	0x616c6600
 1c0:	495f6873 	0x495f6873
 1c4:	65740044 	0x65740044
 1c8:	73007473 	0x73007473
 1cc:	6d5f6973 	0x6d5f6973
 1d0:	61007061 	0x61007061
 1d4:	632e7070 	0x632e7070
 1d8:	786c7200 	0x786c7200
 1dc:	656d645f 	0x656d645f
 1e0:	6e655f6d 	0x6e655f6d
 1e4:	656c6261 	0x656c6261
 1e8:	70736200 	0x70736200
 1ec:	616d675f 	0x616d675f
 1f0:	69645f63 	0x69645f63
 1f4:	6c626173 	0x6c626173
 1f8:	73620065 	0x73620065
 1fc:	61775f70 	0x61775f70
 200:	66007469 	0x66007469
 204:	0067616c 	0x67616c
 208:	20554e47 	addi	s5,v0,20039
 20c:	2e342043 	sltiu	s4,s1,8259
 210:	20322e39 	addi	s2,at,11833
 214:	34313032 	ori	s1,at,0x3032
 218:	33313031 	andi	s1,t9,0x3031
 21c:	72702820 	0x72702820
 220:	6c657265 	0x6c657265
 224:	65736165 	0x65736165
 228:	472d2029 	c1	0x12d2029
 22c:	2d203020 	sltiu	zero,t1,12320
 230:	206c656d 	addi	t4,v1,25965
 234:	72616d2d 	0x72616d2d
 238:	343d6863 	ori	sp,at,0x6863
 23c:	20313832 	addi	s1,at,14386
 240:	6c706d2d 	0x6c706d2d
 244:	672d2074 	0x672d2074
 248:	786c7200 	0x786c7200
 24c:	6163645f 	0x6163645f
 250:	5f656863 	0x5f656863
 254:	726f7473 	0x726f7473
 258:	63630065 	0x63630065
 25c:	645f6c74 	0x645f6c74
 260:	006d656d 	0x6d656d
 264:	6c746363 	0x6c746363
 268:	656d695f 	0x656d695f
 26c:	4f42006d 	c3	0x142006d
 270:	41454c4f 	0x41454c4f
 274:	6c72004e 	0x6c72004e
 278:	6d695f78 	0x6d695f78
 27c:	725f6d65 	0x725f6d65
 280:	6c696665 	0x6c696665
 284:	6f6d006c 	0x6f6d006c
 288:	765f6564 	jalx	97d9590 <_boot-0x768e6a70>0x6564
 28c:	64696c61 	0x64696c61
 290:	73616200 	0x73616200
 294:	6c720065 	0x6c720065
 298:	61635f78 	0x61635f78
 29c:	2e656863 	sltiu	a1,s3,26723
 2a0:	64610063 	0x64610063
 2a4:	73657264 	0x73657264
 2a8:	63630073 	0x63630073
 2ac:	765f6c74 	jalx	97db1d0 <_boot-0x768e4e30>0x6c74
 2b0:	72006c61 	0x72006c61
 2b4:	645f786c 	0x645f786c
 2b8:	68636163 	0x68636163
 2bc:	6c665f65 	0x6c665f65
 2c0:	00687375 	0x687375
 2c4:	5f786c72 	0x5f786c72
 2c8:	63616369 	0x63616369
 2cc:	6c5f6568 	0x6c5f6568
 2d0:	006b636f 	0x6b636f
 2d4:	5f786c72 	0x5f786c72
 2d8:	63616364 	0x63616364
 2dc:	695f6568 	0x695f6568
 2e0:	6c61766e 	0x6c61766e
 2e4:	74616469 	jalx	18591a4 <_boot-0x7e866e5c>0x6469
 2e8:	78650065 	0x78650065
 2ec:	64695f74 	0x64695f74
 2f0:	616c4600 	0x616c4600
 2f4:	655f6873 	0x655f6873
 2f8:	65736172 	0x65736172
 2fc:	6f6c625f 	0x6f6c625f
 300:	44006b63 	0x44006b63
 304:	64616552 	0x64616552
 308:	66667542 	0x66667542
 30c:	43007265 	c0	0x1007265
 310:	61706d6f 	0x61706d6f
 314:	465f6572 	c1	0x5f6572
 318:	6873616c 	0x6873616c
 31c:	73656400 	0x73656400
 320:	616e6974 	0x616e6974
 324:	6e6f6974 	0x6e6f6974
 328:	616c4600 	0x616c4600
 32c:	655f6873 	0x655f6873
 330:	65736172 	0x65736172
 334:	6365735f 	0x6365735f
 338:	00726f74 	0x726f74
 33c:	5f746553 	0x5f746553
 340:	43495053 	c0	0x1495053
 344:	6972575f 	0x6972575f
 348:	6f5f6574 	0x6f5f6574
 34c:	635f656e 	0x635f656e
 350:	6e6e6168 	0x6e6e6168
 354:	41006c65 	bc0f	1b4ec <_boot-0x800a4b14>
 358:	65726464 	0x65726464
 35c:	46007373 	c1	0x7373
 360:	6873616c 	0x6873616c
 364:	696e495f 	0x696e495f
 368:	6c660074 	0x6c660074
 36c:	5f687361 	0x5f687361
 370:	632e7066 	0x632e7066
 374:	75616200 	jalx	5858800 <_boot-0x7a867800>0x6200
 378:	44007264 	0x44007264
 37c:	706d7457 	0x706d7457
 380:	74655300 	jalx	1954c00 <_boot-0x7e76b400>0x5300
 384:	4950535f 	0x4950535f
 388:	6c435f43 	0x6c435f43
 38c:	006b636f 	0x6b636f
 390:	676e6f6c 	0x676e6f6c
 394:	6e6f6c20 	0x6e6f6c20
 398:	6e752067 	0x6e752067
 39c:	6e676973 	0x6e676973
 3a0:	69206465 	0x69206465
 3a4:	6100746e 	0x6100746e
 3a8:	32726464 	andi	s2,s3,0x6464
 3ac:	6e6f6c00 	0x6e6f6c00
 3b0:	6f6c2067 	0x6f6c2067
 3b4:	6920676e 	0x6920676e
 3b8:	4600746e 	c1	0x746e
 3bc:	6873616c 	0x6873616c
 3c0:	6978455f 	0x6978455f
 3c4:	79623474 	0x79623474
 3c8:	415f6574 	0x415f6574
 3cc:	6d726464 	0x6d726464
 3d0:	0065646f 	0x65646f
 3d4:	61655242 	0x61655242
 3d8:	66754264 	0x66754264
 3dc:	00726566 	0x726566
 3e0:	73616c46 	0x73616c46
 3e4:	72775f68 	0x72775f68
 3e8:	5f657469 	0x5f657469
 3ec:	5f656e6f 	0x5f656e6f
 3f0:	6e616863 	0x6e616863
 3f4:	5f6c656e 	0x5f6c656e
 3f8:	72657355 	0x72657355
 3fc:	65684300 	0x65684300
 400:	535f6b63 	0x535f6b63
 404:	5f434950 	0x5f434950
 408:	79737542 	0x79737542
 40c:	616c4600 	0x616c4600
 410:	525f6873 	0x525f6873
 414:	00525344 	0x525344
 418:	73616c66 	0x73616c66
 41c:	75625f68 	jalx	5897da0 <_boot-0x7a828260>0x5f68
 420:	73007973 	0x73007973
 424:	5f636970 	0x5f636970
 428:	79737562 	0x79737562
 42c:	756f7300 	jalx	5bdcc00 <_boot-0x7a4e3400>0x7300
 430:	00656372 	0x656372
 434:	72646461 	0x72646461
 438:	6c460031 	0x6c460031
 43c:	5f687361 	0x5f687361
 440:	73617265 	0x73617265
 444:	6c615f65 	0x6c615f65
 448:	Address 0x0000000000000448 is out of bounds.


Disassembly of section .debug_frame:

00000000 <.debug_frame>:
   0:	0000000c 	syscall
   4:	ffffffff 	0xffffffff
   8:	7c010001 	0x7c010001
   c:	001d0d1f 	0x1d0d1f
  10:	00000020 	add	zero,zero,zero
  14:	00000000 	nop
  18:	800c0078 	lb	t4,120(zero)
  1c:	0000007c 	0x7c
  20:	44100e44 	0x44100e44
  24:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
  28:	0d60021e 	jal	5800878 <_boot-0x7a8bf788>
  2c:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
  30:	00000000 	nop
  34:	00000020 	add	zero,zero,zero
  38:	00000000 	nop
  3c:	800c00f4 	lb	t4,244(zero)
  40:	00000090 	0x90
  44:	44080e44 	0x44080e44
  48:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
  4c:	0d74021e 	jal	5d00878 <_boot-0x7a3bf788>
  50:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
  54:	00000000 	nop
  58:	00000020 	add	zero,zero,zero
  5c:	00000000 	nop
  60:	800c0184 	lb	t4,388(zero)
  64:	00000064 	0x64
  68:	48180e44 	0x48180e44
  6c:	029e019f 	0x29e019f
  70:	021e0d44 	0x21e0d44
  74:	4c1d0d40 	0x4c1d0d40
  78:	000edfde 	0xedfde
  7c:	00000024 	and	zero,zero,zero
  80:	00000000 	nop
  84:	800c01e8 	lb	t4,488(zero)
  88:	00000260 	0x260
  8c:	03c80e44 	0x3c80e44
  90:	9e019f48 	0x9e019f48
  94:	1e0d4402 	0x1e0d4402
  98:	0d023c03 	jal	408f00c <_boot-0x7c030ff4>
  9c:	dfde4c1d 	0xdfde4c1d
  a0:	0000000e 	0xe
  a4:	0000000c 	syscall
  a8:	ffffffff 	0xffffffff
  ac:	7c010001 	0x7c010001
  b0:	001d0d1f 	0x1d0d1f
  b4:	00000020 	add	zero,zero,zero
  b8:	000000a4 	0xa4
  bc:	800c0448 	lb	t4,1096(zero)
  c0:	00000068 	0x68
  c4:	44100e44 	0x44100e44
  c8:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
  cc:	0d4c021e 	jal	5300878 <_boot-0x7adbf788>
  d0:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
  d4:	00000000 	nop
  d8:	00000020 	add	zero,zero,zero
  dc:	000000a4 	0xa4
  e0:	800c04b0 	lb	t4,1200(zero)
  e4:	00000068 	0x68
  e8:	44100e44 	0x44100e44
  ec:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
  f0:	0d4c021e 	jal	5300878 <_boot-0x7adbf788>
  f4:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
  f8:	00000000 	nop
  fc:	00000020 	add	zero,zero,zero
 100:	000000a4 	0xa4
 104:	800c0518 	lb	t4,1304(zero)
 108:	00000068 	0x68
 10c:	44100e44 	0x44100e44
 110:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 114:	0d4c021e 	jal	5300878 <_boot-0x7adbf788>
 118:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 11c:	00000000 	nop
 120:	0000001c 	0x1c
 124:	000000a4 	0xa4
 128:	800c0580 	lb	t4,1408(zero)
 12c:	00000048 	0x48
 130:	44100e44 	0x44100e44
 134:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 138:	1d0d6c1e 	0x1d0d6c1e
 13c:	000ede48 	0xede48
 140:	0000001c 	0x1c
 144:	000000a4 	0xa4
 148:	800c05c8 	lb	t4,1480(zero)
 14c:	00000048 	0x48
 150:	44100e44 	0x44100e44
 154:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 158:	1d0d6c1e 	0x1d0d6c1e
 15c:	000ede48 	0xede48
 160:	00000020 	add	zero,zero,zero
 164:	000000a4 	0xa4
 168:	800c0610 	lb	t4,1552(zero)
 16c:	000000a0 	0xa0
 170:	44100e44 	0x44100e44
 174:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 178:	0d84021e 	jal	6100878 <_boot-0x79fbf788>
 17c:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 180:	00000000 	nop
 184:	0000001c 	0x1c
 188:	000000a4 	0xa4
 18c:	800c06b0 	lb	t4,1712(zero)
 190:	00000054 	0x54
 194:	44100e44 	0x44100e44
 198:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 19c:	1d0d781e 	0x1d0d781e
 1a0:	000ede48 	0xede48
 1a4:	00000020 	add	zero,zero,zero
 1a8:	000000a4 	0xa4
 1ac:	800c0704 	lb	t4,1796(zero)
 1b0:	00000080 	sll	zero,zero,0x2
 1b4:	44100e44 	0x44100e44
 1b8:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 1bc:	0d64021e 	jal	5900878 <_boot-0x7a7bf788>
 1c0:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 1c4:	00000000 	nop
 1c8:	0000000c 	syscall
 1cc:	ffffffff 	0xffffffff
 1d0:	7c010001 	0x7c010001
 1d4:	001d0d1f 	0x1d0d1f
 1d8:	0000001c 	0x1c
 1dc:	000001c8 	0x1c8
 1e0:	800c0784 	lb	t4,1924(zero)
 1e4:	00000040 	ssnop
 1e8:	44100e44 	0x44100e44
 1ec:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 1f0:	1d0d641e 	0x1d0d641e
 1f4:	000ede48 	0xede48
 1f8:	00000020 	add	zero,zero,zero
 1fc:	000001c8 	0x1c8
 200:	800c07c4 	lb	t4,1988(zero)
 204:	000000a0 	0xa0
 208:	44100e44 	0x44100e44
 20c:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 210:	0d84021e 	jal	6100878 <_boot-0x79fbf788>
 214:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 218:	00000000 	nop
 21c:	00000020 	add	zero,zero,zero
 220:	000001c8 	0x1c8
 224:	800c0864 	lb	t4,2148(zero)
 228:	000000dc 	0xdc
 22c:	48200e44 	0x48200e44
 230:	029e019f 	0x29e019f
 234:	021e0d44 	0x21e0d44
 238:	4c1d0db8 	0x4c1d0db8
 23c:	000edfde 	0xedfde
 240:	00000024 	and	zero,zero,zero
 244:	000001c8 	0x1c8
 248:	800c0940 	lb	t4,2368(zero)
 24c:	0000012c 	0x12c
 250:	48200e44 	0x48200e44
 254:	029e019f 	0x29e019f
 258:	031e0d44 	0x31e0d44
 25c:	1d0d0108 	0x1d0d0108
 260:	0edfde4c 	jal	b7f7930 <_boot-0x748c86d0>
 264:	00000000 	nop
 268:	00000024 	and	zero,zero,zero
 26c:	000001c8 	0x1c8
 270:	800c0a6c 	lb	t4,2668(zero)
 274:	0000012c 	0x12c
 278:	48200e44 	0x48200e44
 27c:	029e019f 	0x29e019f
 280:	031e0d44 	0x31e0d44
 284:	1d0d0108 	0x1d0d0108
 288:	0edfde4c 	jal	b7f7930 <_boot-0x748c86d0>
 28c:	00000000 	nop
 290:	00000020 	add	zero,zero,zero
 294:	000001c8 	0x1c8
 298:	800c0b98 	lb	t4,2968(zero)
 29c:	00000070 	0x70
 2a0:	44080e44 	0x44080e44
 2a4:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 2a8:	0d54021e 	jal	5500878 <_boot-0x7abbf788>
 2ac:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 2b0:	00000000 	nop
 2b4:	00000024 	and	zero,zero,zero
 2b8:	000001c8 	0x1c8
 2bc:	800c0c08 	lb	t4,3080(zero)
 2c0:	00000318 	0x318
 2c4:	48300e44 	0x48300e44
 2c8:	029e019f 	0x29e019f
 2cc:	031e0d44 	0x31e0d44
 2d0:	1d0d02f4 	0x1d0d02f4
 2d4:	0edfde4c 	jal	b7f7930 <_boot-0x748c86d0>
 2d8:	00000000 	nop
 2dc:	00000020 	add	zero,zero,zero
 2e0:	000001c8 	0x1c8
 2e4:	800c0f20 	lb	t4,3872(zero)
 2e8:	00000080 	sll	zero,zero,0x2
 2ec:	48180e44 	0x48180e44
 2f0:	029e019f 	0x29e019f
 2f4:	021e0d44 	0x21e0d44
 2f8:	4c1d0d5c 	0x4c1d0d5c
 2fc:	000edfde 	0xedfde
 300:	00000020 	add	zero,zero,zero
 304:	000001c8 	0x1c8
 308:	800c0fa0 	lb	t4,4000(zero)
 30c:	00000120 	0x120
 310:	48200e44 	0x48200e44
 314:	029e019f 	0x29e019f
 318:	021e0d44 	0x21e0d44
 31c:	4c1d0dfc 	0x4c1d0dfc
 320:	000edfde 	0xedfde
 324:	00000020 	add	zero,zero,zero
 328:	000001c8 	0x1c8
 32c:	800c10c0 	lb	t4,4288(zero)
 330:	0000005c 	0x5c
 334:	44080e44 	0x44080e44
 338:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 33c:	0d40021e 	jal	5000878 <_boot-0x7b0bf788>
 340:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 344:	00000000 	nop
 348:	00000024 	and	zero,zero,zero
 34c:	000001c8 	0x1c8
 350:	800c111c 	lb	t4,4380(zero)
 354:	00000124 	0x124
 358:	48200e44 	0x48200e44
 35c:	029e019f 	0x29e019f
 360:	031e0d44 	0x31e0d44
 364:	1d0d0100 	0x1d0d0100
 368:	0edfde4c 	jal	b7f7930 <_boot-0x748c86d0>
 36c:	00000000 	nop
 370:	00000020 	add	zero,zero,zero
 374:	000001c8 	0x1c8
 378:	800c1240 	lb	t4,4672(zero)
 37c:	00000048 	0x48
 380:	48200e44 	0x48200e44
 384:	029e019f 	0x29e019f
 388:	641e0d44 	0x641e0d44
 38c:	de4c1d0d 	0xde4c1d0d
 390:	00000edf 	0xedf
 394:	0000001c 	0x1c
 398:	000001c8 	0x1c8
 39c:	800c1288 	lb	t4,4744(zero)
 3a0:	00000024 	and	zero,zero,zero
 3a4:	44080e44 	0x44080e44
 3a8:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 3ac:	1d0d481e 	0x1d0d481e
 3b0:	000ede48 	0xede48
 3b4:	00000020 	add	zero,zero,zero
 3b8:	000001c8 	0x1c8
 3bc:	800c12ac 	lb	t4,4780(zero)
 3c0:	000000ac 	0xac
 3c4:	44180e44 	0x44180e44
 3c8:	0d44019e 	jal	5100678 <_boot-0x7afbf988>
 3cc:	0d90021e 	jal	6400878 <_boot-0x79cbf788>
 3d0:	0ede481d 	jal	b792074 <_boot-0x7492df8c>
 3d4:	00000000 	nop
