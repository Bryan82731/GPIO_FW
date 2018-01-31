
dash_dbg.exe:     file format elf32-littlemips

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .boot         00000078  80200000  80200000  00000060  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text         00001818  80200078  80200078  000000d8  2**2
                  CONTENTS, ALLOC, LOAD, CODE
  2 .bss          00000430  80201890  80201890  000018f0  2**4
                  ALLOC
  3 .reginfo      00000018  00000000  00000000  000018f0  2**2
                  CONTENTS, READONLY, LINK_ONCE_SAME_SIZE
  4 .pdr          00000600  00000000  00000000  00001908  2**2
                  CONTENTS, READONLY
  5 .comment      00000041  00000000  00000000  00001f08  2**0
                  CONTENTS, READONLY
  6 .gnu.attributes 00000010  00000000  00000000  00001f49  2**0
                  CONTENTS, READONLY
  7 .debug_info   000008c7  00000000  00000000  00001f59  2**0
                  CONTENTS, READONLY, DEBUGGING
  8 .debug_abbrev 000002ed  00000000  00000000  00002820  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .debug_aranges 000000c0  00000000  00000000  00002b10  2**3
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_line   0000071e  00000000  00000000  00002bd0  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .debug_str    00000270  00000000  00000000  000032ee  2**0
                  CONTENTS, READONLY, DEBUGGING
 12 .debug_frame  00000304  00000000  00000000  00003560  2**2
                  CONTENTS, READONLY, DEBUGGING
 13 .debug_pubnames 0000003a  00000000  00000000  00003864  2**0
                  CONTENTS, READONLY, DEBUGGING

Disassembly of section .boot:

80200000 <_boot>:
_boot():
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:41

    ##################################################################
    # After reset
    # BEV = 1, IM = 0, CU = 0  
    ##################################################################
    mfc0    s4, CP0_STATUS        #save original in s4
80200000:	40146000 	mfc0	s4,$12
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:42
    nop
80200004:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:43
    and     t0, s4, ~SR_BEV
80200008:	3c01ffbf 	lui	at,0xffbf
8020000c:	3421ffff 	ori	at,at,0xffff
80200010:	02814024 	and	t0,s4,at
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:44
    or      t0, (SR_CU0 | SR_CU3)        # make sure coprocessors enabled
80200014:	3c019000 	lui	at,0x9000
80200018:	01014025 	or	t0,t0,at
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:45
    mtc0    t0, CP0_STATUS
8020001c:	40886000 	mtc0	t0,$12
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:46
    mtc0    zero, CP0_CAUSE     # zero the cause register
80200020:	40806800 	mtc0	zero,$13
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:47
    nop
80200024:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:53

    ###########################
    # DMEM configuration      #
    ###########################

    la	    t2,	DMEM_BASE
80200028:	3c0a0004 	lui	t2,0x4
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:54
    la      t3, DMEM_TOP	
8020002c:	3c0b0007 	lui	t3,0x7
80200030:	356bffff 	ori	t3,t3,0xffff
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:55
    mtc3	t2,	$4		#DMEM BASE
80200034:	4c8a2000 	mtc3	t2,$4
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:56
    mtc3	t3,	$5		#DMEM TOP
80200038:	4c8b2800 	mtc3	t3,$5
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:58
	
    mfc0	t0, 	C0_CCTL		#read cctl
8020003c:	4008a000 	mfc0	t0,$20
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:59
    nop
80200040:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:60
    or	t0, 	0x400		        #set bit 10(DMEMon)
80200044:	35080400 	ori	t0,t0,0x400
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:61
    mtc0	t0, 	C0_CCTL		#write back to cctl
80200048:	4088a000 	mtc0	t0,$20
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:62
    nop
8020004c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:69
    ##################################################################
    # Initialize Stack Pointer
    #   _stack is initialized by the linker script to point to the
    #    starting location of the stack in DRAM
    ##################################################################
    la    sp,     _stack_hi
80200050:	3c1d8020 	lui	sp,0x8020
80200054:	27bd1ca0 	addiu	sp,sp,7328
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:76
    ##################################################################
    # Initialize Global Pointer
    #   _gp is initialized by the linker script to point to "middle"
    #   of the small variables region
    ##################################################################
    la	  gp,     _gp
80200058:	3c1c8020 	lui	gp,0x8020
8020005c:	279c1890 	addiu	gp,gp,6288
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:78

    la	  t0, main
80200060:	3c088020 	lui	t0,0x8020
80200064:	25080214 	addiu	t0,t0,532
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:79
    jalr	t0
80200068:	0100f809 	jalr	t0
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:80
    nop		
8020006c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:86

    ##################################################################
    # Just in case, go into infinite loop
    ##################################################################
1:
    b	  1b
80200070:	1000ffff 	b	80200070 <_boot+0x70>
/cygdrive/c/rsdk/8111FPRom/flash/crt0.S:87
    nop
80200074:	00000000 	nop

Disassembly of section .text:

80200078 <disable_master_engine>:
disable_master_engine():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:15
//#define SSLCERT
//#define IMEM


disable_master_engine()
{
80200078:	27bdfff8 	addiu	sp,sp,-8
8020007c:	afbe0004 	sw	s8,4(sp)
80200080:	03a0f021 	move	s8,sp
/cygdrive/c/rsdk/8111FPRom/flash/app.c:16
    REG32(0xB2000044) = 0x00; //disable gmac
80200084:	3c02b200 	lui	v0,0xb200
80200088:	34420044 	ori	v0,v0,0x44
8020008c:	ac400000 	sw	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:17
    REG8(0xB2000036) = 0x00;
80200090:	3c02b200 	lui	v0,0xb200
80200094:	34420036 	ori	v0,v0,0x36
80200098:	a0400000 	sb	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:18
    REG8(0xB2001000) = 0x00;  //disable smbus
8020009c:	3c02b200 	lui	v0,0xb200
802000a0:	34421000 	ori	v0,v0,0x1000
802000a4:	a0400000 	sb	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:19
    REG8(0xB2001810)  = 0x00; //disable tcr
802000a8:	3c02b200 	lui	v0,0xb200
802000ac:	34421810 	ori	v0,v0,0x1810
802000b0:	a0400000 	sb	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:20
    REG32(0xB1000008) = 0x00000000;  //disable timer
802000b4:	3c02b100 	lui	v0,0xb100
802000b8:	34420008 	ori	v0,v0,0x8
802000bc:	ac400000 	sw	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:21
}
802000c0:	03c0e821 	move	sp,s8
802000c4:	8fbe0004 	lw	s8,4(sp)
802000c8:	27bd0008 	addiu	sp,sp,8
802000cc:	03e00008 	jr	ra
802000d0:	00000000 	nop

802000d4 <dummy>:
dummy():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:23

void dummy() { }
802000d4:	27bdfff8 	addiu	sp,sp,-8
802000d8:	afbe0004 	sw	s8,4(sp)
802000dc:	03a0f021 	move	s8,sp
802000e0:	03c0e821 	move	sp,s8
802000e4:	8fbe0004 	lw	s8,4(sp)
802000e8:	27bd0008 	addiu	sp,sp,8
802000ec:	03e00008 	jr	ra
802000f0:	00000000 	nop

802000f4 <dummy0>:
dummy0():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:24
void dummy0() { }
802000f4:	27bdfff8 	addiu	sp,sp,-8
802000f8:	afbe0004 	sw	s8,4(sp)
802000fc:	03a0f021 	move	s8,sp
80200100:	03c0e821 	move	sp,s8
80200104:	8fbe0004 	lw	s8,4(sp)
80200108:	27bd0008 	addiu	sp,sp,8
8020010c:	03e00008 	jr	ra
80200110:	00000000 	nop

80200114 <dummy1>:
dummy1():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:25
void dummy1() { }
80200114:	27bdfff8 	addiu	sp,sp,-8
80200118:	afbe0004 	sw	s8,4(sp)
8020011c:	03a0f021 	move	s8,sp
80200120:	03c0e821 	move	sp,s8
80200124:	8fbe0004 	lw	s8,4(sp)
80200128:	27bd0008 	addiu	sp,sp,8
8020012c:	03e00008 	jr	ra
80200130:	00000000 	nop

80200134 <dummy2>:
dummy2():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:26
void dummy2() { }
80200134:	27bdfff8 	addiu	sp,sp,-8
80200138:	afbe0004 	sw	s8,4(sp)
8020013c:	03a0f021 	move	s8,sp
80200140:	03c0e821 	move	sp,s8
80200144:	8fbe0004 	lw	s8,4(sp)
80200148:	27bd0008 	addiu	sp,sp,8
8020014c:	03e00008 	jr	ra
80200150:	00000000 	nop

80200154 <dummy3>:
dummy3():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:27
void dummy3() { }
80200154:	27bdfff8 	addiu	sp,sp,-8
80200158:	afbe0004 	sw	s8,4(sp)
8020015c:	03a0f021 	move	s8,sp
80200160:	03c0e821 	move	sp,s8
80200164:	8fbe0004 	lw	s8,4(sp)
80200168:	27bd0008 	addiu	sp,sp,8
8020016c:	03e00008 	jr	ra
80200170:	00000000 	nop

80200174 <dummy4>:
dummy4():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:28
void dummy4() { }
80200174:	27bdfff8 	addiu	sp,sp,-8
80200178:	afbe0004 	sw	s8,4(sp)
8020017c:	03a0f021 	move	s8,sp
80200180:	03c0e821 	move	sp,s8
80200184:	8fbe0004 	lw	s8,4(sp)
80200188:	27bd0008 	addiu	sp,sp,8
8020018c:	03e00008 	jr	ra
80200190:	00000000 	nop

80200194 <dummy5>:
dummy5():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:29
void dummy5() { }
80200194:	27bdfff8 	addiu	sp,sp,-8
80200198:	afbe0004 	sw	s8,4(sp)
8020019c:	03a0f021 	move	s8,sp
802001a0:	03c0e821 	move	sp,s8
802001a4:	8fbe0004 	lw	s8,4(sp)
802001a8:	27bd0008 	addiu	sp,sp,8
802001ac:	03e00008 	jr	ra
802001b0:	00000000 	nop

802001b4 <dummy6>:
dummy6():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:30
void dummy6() { }
802001b4:	27bdfff8 	addiu	sp,sp,-8
802001b8:	afbe0004 	sw	s8,4(sp)
802001bc:	03a0f021 	move	s8,sp
802001c0:	03c0e821 	move	sp,s8
802001c4:	8fbe0004 	lw	s8,4(sp)
802001c8:	27bd0008 	addiu	sp,sp,8
802001cc:	03e00008 	jr	ra
802001d0:	00000000 	nop

802001d4 <dummy7>:
dummy7():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:31
void dummy7() { }
802001d4:	27bdfff8 	addiu	sp,sp,-8
802001d8:	afbe0004 	sw	s8,4(sp)
802001dc:	03a0f021 	move	s8,sp
802001e0:	03c0e821 	move	sp,s8
802001e4:	8fbe0004 	lw	s8,4(sp)
802001e8:	27bd0008 	addiu	sp,sp,8
802001ec:	03e00008 	jr	ra
802001f0:	00000000 	nop

802001f4 <dummy8>:
dummy8():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:32
void dummy8() { }
802001f4:	27bdfff8 	addiu	sp,sp,-8
802001f8:	afbe0004 	sw	s8,4(sp)
802001fc:	03a0f021 	move	s8,sp
80200200:	03c0e821 	move	sp,s8
80200204:	8fbe0004 	lw	s8,4(sp)
80200208:	27bd0008 	addiu	sp,sp,8
8020020c:	03e00008 	jr	ra
80200210:	00000000 	nop

80200214 <main>:
main():
/cygdrive/c/rsdk/8111FPRom/flash/app.c:35

int main(void)
{
80200214:	27bdffd8 	addiu	sp,sp,-40
80200218:	afbf0024 	sw	ra,36(sp)
8020021c:	afbe0020 	sw	s8,32(sp)
80200220:	03a0f021 	move	s8,sp
/cygdrive/c/rsdk/8111FPRom/flash/app.c:36
  INT32U i = 0;
80200224:	afc00010 	sw	zero,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:37
  struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
80200228:	3c02b400 	lui	v0,0xb400
8020022c:	afc20014 	sw	v0,20(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:38
  INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;
80200230:	8fc40014 	lw	a0,20(s8)
80200234:	0c08042c 	jal	802010b0 <spi_read_id>
80200238:	00000000 	nop
8020023c:	00401821 	move	v1,v0
80200240:	3c0200ff 	lui	v0,0xff
80200244:	00621024 	and	v0,v1,v0
80200248:	00021403 	sra	v0,v0,0x10
8020024c:	afc20018 	sw	v0,24(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:39
  rlx_icache_invalidate_all();
80200250:	0c080231 	jal	802008c4 <rlx_icache_invalidate_all>
80200254:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:40
  rlx_dmem_enable(DMEM_BASE,DMEM_TOP);
80200258:	3c020007 	lui	v0,0x7
8020025c:	3445ffff 	ori	a1,v0,0xffff
80200260:	3c040004 	lui	a0,0x4
80200264:	0c080272 	jal	802009c8 <rlx_dmem_enable>
80200268:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:42

  disable_master_engine();
8020026c:	0c08001e 	jal	80200078 <disable_master_engine>
80200270:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:43
  REG8(0xB9000004) = 0x00; 
80200274:	3c02b900 	lui	v0,0xb900
80200278:	34420004 	ori	v0,v0,0x4
8020027c:	a0400000 	sb	zero,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:48
  //use normal speed

  //erase block 0 => total 64k

  if (rdid == 0x1f)
80200280:	8fc30018 	lw	v1,24(s8)
80200284:	2402001f 	li	v0,31
80200288:	14620005 	bne	v1,v0,802002a0 <main+0x8c>
8020028c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:49
      spi_se_unprotect(ssi_map, 0x000000);
80200290:	00002821 	move	a1,zero
80200294:	8fc40014 	lw	a0,20(s8)
80200298:	0c080339 	jal	80200ce4 <spi_se_unprotect>
8020029c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:50
  spi_blk_erase(ssi_map, 0x000000);
802002a0:	00002821 	move	a1,zero
802002a4:	8fc40014 	lw	a0,20(s8)
802002a8:	0c0802ef 	jal	80200bbc <spi_blk_erase>
802002ac:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:51
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
802002b0:	3c020003 	lui	v0,0x3
802002b4:	3446ffff 	ori	a2,v0,0xffff
802002b8:	240500ff 	li	a1,255
802002bc:	3c048004 	lui	a0,0x8004
802002c0:	0c0805fb 	jal	802017ec <memset>
802002c4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:53

  dummy();
802002c8:	0c080035 	jal	802000d4 <dummy>
802002cc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:55

  for(i = 0 ; i < NUMLOOP/2 ; i++)
802002d0:	afc00010 	sw	zero,16(s8)
802002d4:	080800c9 	j	80200324 <main+0x110>
802002d8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:56 (discriminator 3)
    spi_write(ssi_map, 0x000000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
802002dc:	8fc20010 	lw	v0,16(s8)
802002e0:	00000000 	nop
802002e4:	00022180 	sll	a0,v0,0x6
802002e8:	8fc30010 	lw	v1,16(s8)
802002ec:	3c020200 	lui	v0,0x200
802002f0:	34424000 	ori	v0,v0,0x4000
802002f4:	00621021 	addu	v0,v1,v0
802002f8:	00021180 	sll	v0,v0,0x6
802002fc:	24070040 	li	a3,64
80200300:	00403021 	move	a2,v0
80200304:	00802821 	move	a1,a0
80200308:	8fc40014 	lw	a0,20(s8)
8020030c:	0c080520 	jal	80201480 <spi_write>
80200310:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:55 (discriminator 3)
  spi_blk_erase(ssi_map, 0x000000);
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);

  dummy();

  for(i = 0 ; i < NUMLOOP/2 ; i++)
80200314:	8fc20010 	lw	v0,16(s8)
80200318:	00000000 	nop
8020031c:	24420001 	addiu	v0,v0,1
80200320:	afc20010 	sw	v0,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:55 (discriminator 1)
80200324:	8fc20010 	lw	v0,16(s8)
80200328:	00000000 	nop
8020032c:	2c420400 	sltiu	v0,v0,1024
80200330:	1440ffea 	bnez	v0,802002dc <main+0xc8>
80200334:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:58
    spi_write(ssi_map, 0x000000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
  
  if(memcmp((void *) 0x80100000, (void *) 0xA1000000, DMEMSIZE/2) != 0)
80200338:	3c060001 	lui	a2,0x1
8020033c:	3c05a100 	lui	a1,0xa100
80200340:	3c048010 	lui	a0,0x8010
80200344:	0c0805ec 	jal	802017b0 <memcmp>
80200348:	00000000 	nop
8020034c:	10400008 	beqz	v0,80200370 <main+0x15c>
80200350:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:59
	  stage++;
80200354:	3c028020 	lui	v0,0x8020
80200358:	90421890 	lbu	v0,6288(v0)
8020035c:	00000000 	nop
80200360:	24420001 	addiu	v0,v0,1
80200364:	304300ff 	andi	v1,v0,0xff
80200368:	3c028020 	lui	v0,0x8020
8020036c:	a0431890 	sb	v1,6288(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:65

#ifdef PART1
  //erase block 0 & block 1 
  //total 128K at once
  do{
  if (rdid == 0x1f)
80200370:	8fc30018 	lw	v1,24(s8)
80200374:	2402001f 	li	v0,31
80200378:	14620005 	bne	v1,v0,80200390 <main+0x17c>
8020037c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:66
      spi_se_unprotect(ssi_map, 0x010000);
80200380:	3c050001 	lui	a1,0x1
80200384:	8fc40014 	lw	a0,20(s8)
80200388:	0c080339 	jal	80200ce4 <spi_se_unprotect>
8020038c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:67
  spi_blk_erase(ssi_map, 0x010000);
80200390:	3c050001 	lui	a1,0x1
80200394:	8fc40014 	lw	a0,20(s8)
80200398:	0c0802ef 	jal	80200bbc <spi_blk_erase>
8020039c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:68
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
802003a0:	3c020003 	lui	v0,0x3
802003a4:	3446ffff 	ori	a2,v0,0xffff
802003a8:	240500ff 	li	a1,255
802003ac:	3c048004 	lui	a0,0x8004
802003b0:	0c0805fb 	jal	802017ec <memset>
802003b4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:69
  if (rdid == 0x1f)
802003b8:	8fc30018 	lw	v1,24(s8)
802003bc:	2402001f 	li	v0,31
802003c0:	14620005 	bne	v1,v0,802003d8 <main+0x1c4>
802003c4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:70
      spi_se_unprotect(ssi_map, 0x020000);
802003c8:	3c050002 	lui	a1,0x2
802003cc:	8fc40014 	lw	a0,20(s8)
802003d0:	0c080339 	jal	80200ce4 <spi_se_unprotect>
802003d4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:71
  spi_blk_erase(ssi_map, 0x020000);
802003d8:	3c050002 	lui	a1,0x2
802003dc:	8fc40014 	lw	a0,20(s8)
802003e0:	0c0802ef 	jal	80200bbc <spi_blk_erase>
802003e4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:74

  //load image from EPI script
  dummy0();
802003e8:	0c08003d 	jal	802000f4 <dummy0>
802003ec:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:76

  for(i= 0; i < NUMLOOP; i++) 
802003f0:	afc00010 	sw	zero,16(s8)
802003f4:	08080112 	j	80200448 <main+0x234>
802003f8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:77 (discriminator 3)
  spi_write(ssi_map, 0x010000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
802003fc:	8fc20010 	lw	v0,16(s8)
80200400:	00000000 	nop
80200404:	24420400 	addiu	v0,v0,1024
80200408:	00022180 	sll	a0,v0,0x6
8020040c:	8fc30010 	lw	v1,16(s8)
80200410:	3c020200 	lui	v0,0x200
80200414:	34424000 	ori	v0,v0,0x4000
80200418:	00621021 	addu	v0,v1,v0
8020041c:	00021180 	sll	v0,v0,0x6
80200420:	24070040 	li	a3,64
80200424:	00403021 	move	a2,v0
80200428:	00802821 	move	a1,a0
8020042c:	8fc40014 	lw	a0,20(s8)
80200430:	0c080520 	jal	80201480 <spi_write>
80200434:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:76 (discriminator 3)
  spi_blk_erase(ssi_map, 0x020000);

  //load image from EPI script
  dummy0();

  for(i= 0; i < NUMLOOP; i++) 
80200438:	8fc20010 	lw	v0,16(s8)
8020043c:	00000000 	nop
80200440:	24420001 	addiu	v0,v0,1
80200444:	afc20010 	sw	v0,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:76 (discriminator 1)
80200448:	8fc20010 	lw	v0,16(s8)
8020044c:	00000000 	nop
80200450:	2c420800 	sltiu	v0,v0,2048
80200454:	1440ffe9 	bnez	v0,802003fc <main+0x1e8>
80200458:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:79
  spi_write(ssi_map, 0x010000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1010000, DMEMSIZE) != 0)
8020045c:	3c060002 	lui	a2,0x2
80200460:	3c05a101 	lui	a1,0xa101
80200464:	3c048010 	lui	a0,0x8010
80200468:	0c0805ec 	jal	802017b0 <memcmp>
8020046c:	00000000 	nop
80200470:	10400008 	beqz	v0,80200494 <main+0x280>
80200474:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:80
		  stage++;
80200478:	3c028020 	lui	v0,0x8020
8020047c:	90421890 	lbu	v0,6288(v0)
80200480:	00000000 	nop
80200484:	24420001 	addiu	v0,v0,1
80200488:	304300ff 	andi	v1,v0,0xff
8020048c:	3c028020 	lui	v0,0x8020
80200490:	a0431890 	sb	v1,6288(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:86
  }while(0);
#endif

#ifdef PART2
  //erase block 2 & block 3 => total 128K
  if (rdid == 0x1f)
80200494:	8fc30018 	lw	v1,24(s8)
80200498:	2402001f 	li	v0,31
8020049c:	14620005 	bne	v1,v0,802004b4 <main+0x2a0>
802004a0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:87
      spi_se_unprotect(ssi_map, 0x030000);
802004a4:	3c050003 	lui	a1,0x3
802004a8:	8fc40014 	lw	a0,20(s8)
802004ac:	0c080339 	jal	80200ce4 <spi_se_unprotect>
802004b0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:88
  spi_blk_erase(ssi_map, 0x030000);
802004b4:	3c050003 	lui	a1,0x3
802004b8:	8fc40014 	lw	a0,20(s8)
802004bc:	0c0802ef 	jal	80200bbc <spi_blk_erase>
802004c0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:89
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
802004c4:	3c020003 	lui	v0,0x3
802004c8:	3446ffff 	ori	a2,v0,0xffff
802004cc:	240500ff 	li	a1,255
802004d0:	3c048004 	lui	a0,0x8004
802004d4:	0c0805fb 	jal	802017ec <memset>
802004d8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:90
  if (rdid == 0x1f)
802004dc:	8fc30018 	lw	v1,24(s8)
802004e0:	2402001f 	li	v0,31
802004e4:	14620005 	bne	v1,v0,802004fc <main+0x2e8>
802004e8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:91
      spi_se_unprotect(ssi_map, 0x040000);
802004ec:	3c050004 	lui	a1,0x4
802004f0:	8fc40014 	lw	a0,20(s8)
802004f4:	0c080339 	jal	80200ce4 <spi_se_unprotect>
802004f8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:92
  spi_blk_erase(ssi_map, 0x040000);
802004fc:	3c050004 	lui	a1,0x4
80200500:	8fc40014 	lw	a0,20(s8)
80200504:	0c0802ef 	jal	80200bbc <spi_blk_erase>
80200508:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:94

  dummy1();
8020050c:	0c080045 	jal	80200114 <dummy1>
80200510:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:96

  for(i= 0; i < NUMLOOP; i++) 
80200514:	afc00010 	sw	zero,16(s8)
80200518:	0808015b 	j	8020056c <main+0x358>
8020051c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:97 (discriminator 3)
  spi_write(ssi_map, 0x030000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
80200520:	8fc20010 	lw	v0,16(s8)
80200524:	00000000 	nop
80200528:	24420c00 	addiu	v0,v0,3072
8020052c:	00022180 	sll	a0,v0,0x6
80200530:	8fc30010 	lw	v1,16(s8)
80200534:	3c020200 	lui	v0,0x200
80200538:	34424000 	ori	v0,v0,0x4000
8020053c:	00621021 	addu	v0,v1,v0
80200540:	00021180 	sll	v0,v0,0x6
80200544:	24070040 	li	a3,64
80200548:	00403021 	move	a2,v0
8020054c:	00802821 	move	a1,a0
80200550:	8fc40014 	lw	a0,20(s8)
80200554:	0c080520 	jal	80201480 <spi_write>
80200558:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:96 (discriminator 3)
      spi_se_unprotect(ssi_map, 0x040000);
  spi_blk_erase(ssi_map, 0x040000);

  dummy1();

  for(i= 0; i < NUMLOOP; i++) 
8020055c:	8fc20010 	lw	v0,16(s8)
80200560:	00000000 	nop
80200564:	24420001 	addiu	v0,v0,1
80200568:	afc20010 	sw	v0,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:96 (discriminator 1)
8020056c:	8fc20010 	lw	v0,16(s8)
80200570:	00000000 	nop
80200574:	2c420800 	sltiu	v0,v0,2048
80200578:	1440ffe9 	bnez	v0,80200520 <main+0x30c>
8020057c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:99
  spi_write(ssi_map, 0x030000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1030000, DMEMSIZE) != 0)
80200580:	3c060002 	lui	a2,0x2
80200584:	3c05a103 	lui	a1,0xa103
80200588:	3c048010 	lui	a0,0x8010
8020058c:	0c0805ec 	jal	802017b0 <memcmp>
80200590:	00000000 	nop
80200594:	10400008 	beqz	v0,802005b8 <main+0x3a4>
80200598:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:100
		  stage++;
8020059c:	3c028020 	lui	v0,0x8020
802005a0:	90421890 	lbu	v0,6288(v0)
802005a4:	00000000 	nop
802005a8:	24420001 	addiu	v0,v0,1
802005ac:	304300ff 	andi	v1,v0,0xff
802005b0:	3c028020 	lui	v0,0x8020
802005b4:	a0431890 	sb	v1,6288(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:106

#endif

#ifdef PART3
  //erase block 4 & block 5 => total 128K
  if (rdid == 0x1f)
802005b8:	8fc30018 	lw	v1,24(s8)
802005bc:	2402001f 	li	v0,31
802005c0:	14620005 	bne	v1,v0,802005d8 <main+0x3c4>
802005c4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:107
      spi_se_unprotect(ssi_map, 0x050000);
802005c8:	3c050005 	lui	a1,0x5
802005cc:	8fc40014 	lw	a0,20(s8)
802005d0:	0c080339 	jal	80200ce4 <spi_se_unprotect>
802005d4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:108
  spi_blk_erase(ssi_map, 0x050000);
802005d8:	3c050005 	lui	a1,0x5
802005dc:	8fc40014 	lw	a0,20(s8)
802005e0:	0c0802ef 	jal	80200bbc <spi_blk_erase>
802005e4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:109
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
802005e8:	3c020003 	lui	v0,0x3
802005ec:	3446ffff 	ori	a2,v0,0xffff
802005f0:	240500ff 	li	a1,255
802005f4:	3c048004 	lui	a0,0x8004
802005f8:	0c0805fb 	jal	802017ec <memset>
802005fc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:110
  if (rdid == 0x1f)
80200600:	8fc30018 	lw	v1,24(s8)
80200604:	2402001f 	li	v0,31
80200608:	14620005 	bne	v1,v0,80200620 <main+0x40c>
8020060c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:111
      spi_se_unprotect(ssi_map, 0x060000);
80200610:	3c050006 	lui	a1,0x6
80200614:	8fc40014 	lw	a0,20(s8)
80200618:	0c080339 	jal	80200ce4 <spi_se_unprotect>
8020061c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:112
  spi_blk_erase(ssi_map, 0x060000);
80200620:	3c050006 	lui	a1,0x6
80200624:	8fc40014 	lw	a0,20(s8)
80200628:	0c0802ef 	jal	80200bbc <spi_blk_erase>
8020062c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:114

  dummy2();
80200630:	0c08004d 	jal	80200134 <dummy2>
80200634:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:116

  for(i= 0; i < NUMLOOP; i++) 
80200638:	afc00010 	sw	zero,16(s8)
8020063c:	080801a4 	j	80200690 <main+0x47c>
80200640:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:117 (discriminator 3)
  spi_write(ssi_map, 0x050000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
80200644:	8fc20010 	lw	v0,16(s8)
80200648:	00000000 	nop
8020064c:	24421400 	addiu	v0,v0,5120
80200650:	00022180 	sll	a0,v0,0x6
80200654:	8fc30010 	lw	v1,16(s8)
80200658:	3c020200 	lui	v0,0x200
8020065c:	34424000 	ori	v0,v0,0x4000
80200660:	00621021 	addu	v0,v1,v0
80200664:	00021180 	sll	v0,v0,0x6
80200668:	24070040 	li	a3,64
8020066c:	00403021 	move	a2,v0
80200670:	00802821 	move	a1,a0
80200674:	8fc40014 	lw	a0,20(s8)
80200678:	0c080520 	jal	80201480 <spi_write>
8020067c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:116 (discriminator 3)
      spi_se_unprotect(ssi_map, 0x060000);
  spi_blk_erase(ssi_map, 0x060000);

  dummy2();

  for(i= 0; i < NUMLOOP; i++) 
80200680:	8fc20010 	lw	v0,16(s8)
80200684:	00000000 	nop
80200688:	24420001 	addiu	v0,v0,1
8020068c:	afc20010 	sw	v0,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:116 (discriminator 1)
80200690:	8fc20010 	lw	v0,16(s8)
80200694:	00000000 	nop
80200698:	2c420800 	sltiu	v0,v0,2048
8020069c:	1440ffe9 	bnez	v0,80200644 <main+0x430>
802006a0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:119
  spi_write(ssi_map, 0x050000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);

  if(memcmp((void *) 0x80100000, (void *) 0xA1050000, DMEMSIZE) != 0)
802006a4:	3c060002 	lui	a2,0x2
802006a8:	3c05a105 	lui	a1,0xa105
802006ac:	3c048010 	lui	a0,0x8010
802006b0:	0c0805ec 	jal	802017b0 <memcmp>
802006b4:	00000000 	nop
802006b8:	10400008 	beqz	v0,802006dc <main+0x4c8>
802006bc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:120
		  stage++;
802006c0:	3c028020 	lui	v0,0x8020
802006c4:	90421890 	lbu	v0,6288(v0)
802006c8:	00000000 	nop
802006cc:	24420001 	addiu	v0,v0,1
802006d0:	304300ff 	andi	v1,v0,0xff
802006d4:	3c028020 	lui	v0,0x8020
802006d8:	a0431890 	sb	v1,6288(v0)
/cygdrive/c/rsdk/8111FPRom/flash/app.c:213
		  stage++;
  }while(0);
#endif


 dummy8();
802006dc:	0c08007d 	jal	802001f4 <dummy8>
802006e0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/app.c:214
}
802006e4:	03c0e821 	move	sp,s8
802006e8:	8fbf0024 	lw	ra,36(sp)
802006ec:	8fbe0020 	lw	s8,32(sp)
802006f0:	27bd0028 	addiu	sp,sp,40
802006f4:	03e00008 	jr	ra
802006f8:	00000000 	nop

802006fc <rlx_dcache_flush>:
rlx_dcache_flush():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:53
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_flush(INT32U address, INT32U size)
{
802006fc:	27bdfff0 	addiu	sp,sp,-16
80200700:	afbe000c 	sw	s8,12(sp)
80200704:	03a0f021 	move	s8,sp
80200708:	afc40010 	sw	a0,16(s8)
8020070c:	afc50014 	sw	a1,20(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:56
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
80200710:	8fc20010 	lw	v0,16(s8)
80200714:	00000000 	nop
80200718:	afc20000 	sw	v0,0(s8)
8020071c:	080801d0 	j	80200740 <rlx_dcache_flush+0x44>
80200720:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:58 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
80200724:	8fc20000 	lw	v0,0(s8)
80200728:	00000000 	nop
8020072c:	bc550000 	cache	0x15,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:56 (discriminator 3)
void 
rlx_dcache_flush(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
80200730:	8fc20000 	lw	v0,0(s8)
80200734:	00000000 	nop
80200738:	24420004 	addiu	v0,v0,4
8020073c:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:56 (discriminator 1)
80200740:	8fc30010 	lw	v1,16(s8)
80200744:	8fc20014 	lw	v0,20(s8)
80200748:	00000000 	nop
8020074c:	00621821 	addu	v1,v1,v0
80200750:	8fc20000 	lw	v0,0(s8)
80200754:	00000000 	nop
80200758:	0043102b 	sltu	v0,v0,v1
8020075c:	1440fff1 	bnez	v0,80200724 <rlx_dcache_flush+0x28>
80200760:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:62
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WBINVAL), "r"(addr) );
    }
   
}
80200764:	03c0e821 	move	sp,s8
80200768:	8fbe000c 	lw	s8,12(sp)
8020076c:	27bd0010 	addiu	sp,sp,16
80200770:	03e00008 	jr	ra
80200774:	00000000 	nop

80200778 <rlx_dcache_invalidate>:
rlx_dcache_invalidate():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:78
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate(INT32U address, INT32U size)
{
80200778:	27bdfff0 	addiu	sp,sp,-16
8020077c:	afbe000c 	sw	s8,12(sp)
80200780:	03a0f021 	move	s8,sp
80200784:	afc40010 	sw	a0,16(s8)
80200788:	afc50014 	sw	a1,20(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:81
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
8020078c:	8fc20010 	lw	v0,16(s8)
80200790:	00000000 	nop
80200794:	afc20000 	sw	v0,0(s8)
80200798:	080801ef 	j	802007bc <rlx_dcache_invalidate+0x44>
8020079c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:83 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
802007a0:	8fc20000 	lw	v0,0(s8)
802007a4:	00000000 	nop
802007a8:	bc510000 	cache	0x11,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:81 (discriminator 3)
void 
rlx_dcache_invalidate(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
802007ac:	8fc20000 	lw	v0,0(s8)
802007b0:	00000000 	nop
802007b4:	24420004 	addiu	v0,v0,4
802007b8:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:81 (discriminator 1)
802007bc:	8fc30010 	lw	v1,16(s8)
802007c0:	8fc20014 	lw	v0,20(s8)
802007c4:	00000000 	nop
802007c8:	00621821 	addu	v1,v1,v0
802007cc:	8fc20000 	lw	v0,0(s8)
802007d0:	00000000 	nop
802007d4:	0043102b 	sltu	v0,v0,v1
802007d8:	1440fff1 	bnez	v0,802007a0 <rlx_dcache_invalidate+0x28>
802007dc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:87
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_INVAL), "r"(addr) );
    }
   
}
802007e0:	03c0e821 	move	sp,s8
802007e4:	8fbe000c 	lw	s8,12(sp)
802007e8:	27bd0010 	addiu	sp,sp,16
802007ec:	03e00008 	jr	ra
802007f0:	00000000 	nop

802007f4 <rlx_dcache_store>:
rlx_dcache_store():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:104
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_store(INT32U address, INT32U size)
{
802007f4:	27bdfff0 	addiu	sp,sp,-16
802007f8:	afbe000c 	sw	s8,12(sp)
802007fc:	03a0f021 	move	s8,sp
80200800:	afc40010 	sw	a0,16(s8)
80200804:	afc50014 	sw	a1,20(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:107
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
80200808:	8fc20010 	lw	v0,16(s8)
8020080c:	00000000 	nop
80200810:	afc20000 	sw	v0,0(s8)
80200814:	0808020e 	j	80200838 <rlx_dcache_store+0x44>
80200818:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:109 (discriminator 3)
    {
      asm volatile ("cache %0,0(%1);"                      
8020081c:	8fc20000 	lw	v0,0(s8)
80200820:	00000000 	nop
80200824:	bc590000 	cache	0x19,0(v0)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:107 (discriminator 3)
void 
rlx_dcache_store(INT32U address, INT32U size)
{
  INT32U     addr;

  for (addr = address; addr < address + size; addr += BSP_DCACHE_LINE_SIZE)
80200828:	8fc20000 	lw	v0,0(s8)
8020082c:	00000000 	nop
80200830:	24420004 	addiu	v0,v0,4
80200834:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:107 (discriminator 1)
80200838:	8fc30010 	lw	v1,16(s8)
8020083c:	8fc20014 	lw	v0,20(s8)
80200840:	00000000 	nop
80200844:	00621821 	addu	v1,v1,v0
80200848:	8fc20000 	lw	v0,0(s8)
8020084c:	00000000 	nop
80200850:	0043102b 	sltu	v0,v0,v1
80200854:	1440fff1 	bnez	v0,8020081c <rlx_dcache_store+0x28>
80200858:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:113
    {
      asm volatile ("cache %0,0(%1);"                      
                 : : "I" (CACHE_CMD_WB), "r"(addr) );
    }
   
}
8020085c:	03c0e821 	move	sp,s8
80200860:	8fbe000c 	lw	s8,12(sp)
80200864:	27bd0010 	addiu	sp,sp,16
80200868:	03e00008 	jr	ra
8020086c:	00000000 	nop

80200870 <rlx_dcache_invalidate_all>:
rlx_dcache_invalidate_all():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:129
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_dcache_invalidate_all(void)
{
80200870:	27bdfff0 	addiu	sp,sp,-16
80200874:	afbe000c 	sw	s8,12(sp)
80200878:	03a0f021 	move	s8,sp
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:130
  INT32U  cctl_val = 0;
8020087c:	afc00000 	sw	zero,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:132
  
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
80200880:	8fc20000 	lw	v0,0(s8)
80200884:	00000000 	nop
80200888:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:133
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
8020088c:	4002a000 	mfc0	v0,$20
80200890:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:134
  cctl_val |= CP0_CCTL_DINVAL;
80200894:	8fc20000 	lw	v0,0(s8)
80200898:	00000000 	nop
8020089c:	34420001 	ori	v0,v0,0x1
802008a0:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:135
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
802008a4:	8fc20000 	lw	v0,0(s8)
802008a8:	00000000 	nop
802008ac:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:137

}
802008b0:	03c0e821 	move	sp,s8
802008b4:	8fbe000c 	lw	s8,12(sp)
802008b8:	27bd0010 	addiu	sp,sp,16
802008bc:	03e00008 	jr	ra
802008c0:	00000000 	nop

802008c4 <rlx_icache_invalidate_all>:
rlx_icache_invalidate_all():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:153
 * Returns    : None
 *****************************************************************************************
 */
void 
rlx_icache_invalidate_all(void)
{
802008c4:	27bdfff0 	addiu	sp,sp,-16
802008c8:	afbe000c 	sw	s8,12(sp)
802008cc:	03a0f021 	move	s8,sp
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:154
  INT32U  cctl_val = 0;
802008d0:	afc00000 	sw	zero,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:156

  asm volatile("mtc0   %z0, $20"  :  : "Jr" ((INT32U)(cctl_val))); 
802008d4:	8fc20000 	lw	v0,0(s8)
802008d8:	00000000 	nop
802008dc:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:157
  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
802008e0:	4002a000 	mfc0	v0,$20
802008e4:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:158
  cctl_val |= CP0_CCTL_IINVAL;
802008e8:	8fc20000 	lw	v0,0(s8)
802008ec:	00000000 	nop
802008f0:	34420002 	ori	v0,v0,0x2
802008f4:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:159
  asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
802008f8:	8fc20000 	lw	v0,0(s8)
802008fc:	00000000 	nop
80200900:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:160
}
80200904:	03c0e821 	move	sp,s8
80200908:	8fbe000c 	lw	s8,12(sp)
8020090c:	27bd0010 	addiu	sp,sp,16
80200910:	03e00008 	jr	ra
80200914:	00000000 	nop

80200918 <rlx_icache_lock>:
rlx_icache_lock():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:178
 * Returns    : BOOLEAN -- OS_TRUE if paramter is valid, and OS_FALSE otherwise
 *****************************************************************************************
 */
BOOLEAN 
rlx_icache_lock(INT32U mode)
{
80200918:	27bdfff0 	addiu	sp,sp,-16
8020091c:	afbe000c 	sw	s8,12(sp)
80200920:	03a0f021 	move	s8,sp
80200924:	afc40010 	sw	a0,16(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:180
  INT32U  cctl_val;
  BOOLEAN mode_valid = OS_FALSE;  
80200928:	a3c00000 	sb	zero,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:182

  asm volatile("mfc0   %0, $20"   : "=r"(cctl_val));
8020092c:	4002a000 	mfc0	v0,$20
80200930:	afc20004 	sw	v0,4(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:184

  cctl_val &= ~CP0_CCTL_ILOCK_MASK;
80200934:	8fc30004 	lw	v1,4(s8)
80200938:	2402fff3 	li	v0,-13
8020093c:	00621024 	and	v0,v1,v0
80200940:	afc20004 	sw	v0,4(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:186

  switch(mode)
80200944:	8fc20010 	lw	v0,16(s8)
80200948:	00000000 	nop
8020094c:	10400005 	beqz	v0,80200964 <rlx_icache_lock+0x4c>
80200950:	00000000 	nop
80200954:	2442fffe 	addiu	v0,v0,-2
80200958:	2c420002 	sltiu	v0,v0,2
8020095c:	10400005 	beqz	v0,80200974 <rlx_icache_lock+0x5c>
80200960:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:191
    {
    case CP0_CCTL_ILOCK_NORMAL: 
    case CP0_CCTL_ILOCK_LOCKGATHER: 
    case CP0_CCTL_ILOCK_LOCKDOWN: 
      mode_valid = OS_TRUE;
80200964:	24020001 	li	v0,1
80200968:	a3c20000 	sb	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:192
      break;
8020096c:	0808025e 	j	80200978 <rlx_icache_lock+0x60>
80200970:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:195

    default:
      break;
80200974:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:199
    }

  /* valid setting, write to CP0_CCTL */    
  if (mode_valid == OS_TRUE)
80200978:	93c30000 	lbu	v1,0(s8)
8020097c:	24020001 	li	v0,1
80200980:	1462000b 	bne	v1,v0,802009b0 <rlx_icache_lock+0x98>
80200984:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:201
    {
      cctl_val |= (mode << CP0_CCTL_ILOCK_OFFSET);
80200988:	8fc20010 	lw	v0,16(s8)
8020098c:	00000000 	nop
80200990:	00021080 	sll	v0,v0,0x2
80200994:	8fc30004 	lw	v1,4(s8)
80200998:	00000000 	nop
8020099c:	00621025 	or	v0,v1,v0
802009a0:	afc20004 	sw	v0,4(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:202
      asm volatile("mtc0   %z0, $20"  : : "Jr" ((INT32U)(cctl_val)));
802009a4:	8fc20004 	lw	v0,4(s8)
802009a8:	00000000 	nop
802009ac:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:205
    }
  
  return (mode_valid);
802009b0:	93c20000 	lbu	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:206
}
802009b4:	03c0e821 	move	sp,s8
802009b8:	8fbe000c 	lw	s8,12(sp)
802009bc:	27bd0010 	addiu	sp,sp,16
802009c0:	03e00008 	jr	ra
802009c4:	00000000 	nop

802009c8 <rlx_dmem_enable>:
rlx_dmem_enable():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:223
 *******************************************************************************
 */

void
rlx_dmem_enable(INT32U base, INT32U top)
{
802009c8:	27bdfff0 	addiu	sp,sp,-16
802009cc:	afbe000c 	sw	s8,12(sp)
802009d0:	03a0f021 	move	s8,sp
802009d4:	afc40010 	sw	a0,16(s8)
802009d8:	afc50014 	sw	a1,20(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:225
    INT32U cctl_dmem;
    asm volatile("mtc3   %z0, $4"  : : "Jr" ((INT32U)(base)));	
802009dc:	8fc20010 	lw	v0,16(s8)
802009e0:	00000000 	nop
802009e4:	4c822000 	mtc3	v0,$4
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:226
    asm volatile("mtc3   %z0, $5"  : : "Jr" ((INT32U)(top)));
802009e8:	8fc20014 	lw	v0,20(s8)
802009ec:	00000000 	nop
802009f0:	4c822800 	mtc3	v0,$5
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:228
    
    asm volatile("mfc0   %0, $20"   : "=r"(cctl_dmem));
802009f4:	4002a000 	mfc0	v0,$20
802009f8:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:229
    cctl_dmem |= CP0_CCTL_DMEMON;
802009fc:	8fc20000 	lw	v0,0(s8)
80200a00:	00000000 	nop
80200a04:	34420400 	ori	v0,v0,0x400
80200a08:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:230
    asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_dmem)));
80200a0c:	8fc20000 	lw	v0,0(s8)
80200a10:	00000000 	nop
80200a14:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:231
}
80200a18:	03c0e821 	move	sp,s8
80200a1c:	8fbe000c 	lw	s8,12(sp)
80200a20:	27bd0010 	addiu	sp,sp,16
80200a24:	03e00008 	jr	ra
80200a28:	00000000 	nop

80200a2c <rlx_imem_refill>:
rlx_imem_refill():
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:248
 *
 * Returns    : None
 *******************************************************************************
 */
void rlx_imem_refill(INT32U base, INT32U top, INT32U mode)
{
80200a2c:	27bdfff0 	addiu	sp,sp,-16
80200a30:	afbe000c 	sw	s8,12(sp)
80200a34:	03a0f021 	move	s8,sp
80200a38:	afc40010 	sw	a0,16(s8)
80200a3c:	afc50014 	sw	a1,20(s8)
80200a40:	afc60018 	sw	a2,24(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:250
    INT32U cctl_imem;
    asm volatile("mtc3   %z0, $0"  : : "Jr" ((INT32U)(base)));	
80200a44:	8fc20010 	lw	v0,16(s8)
80200a48:	00000000 	nop
80200a4c:	4c820000 	mtc3	v0,$0
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:251
    asm volatile("mtc3   %z0, $1"  : : "Jr" ((INT32U)(top)));
80200a50:	8fc20014 	lw	v0,20(s8)
80200a54:	00000000 	nop
80200a58:	4c820800 	mtc3	v0,$1
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:253

    if(mode == 1)
80200a5c:	8fc30018 	lw	v1,24(s8)
80200a60:	24020001 	li	v0,1
80200a64:	14620011 	bne	v1,v0,80200aac <rlx_imem_refill+0x80>
80200a68:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:255
    {
        asm volatile("mfc0   %0, $20"   : "=r"(cctl_imem));
80200a6c:	4002a000 	mfc0	v0,$20
80200a70:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:256
        cctl_imem &=  ~CP0_CCTL_IMEMFILL;
80200a74:	8fc30000 	lw	v1,0(s8)
80200a78:	2402ffef 	li	v0,-17
80200a7c:	00621024 	and	v0,v1,v0
80200a80:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:257
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
80200a84:	8fc20000 	lw	v0,0(s8)
80200a88:	00000000 	nop
80200a8c:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:259

        cctl_imem |=  CP0_CCTL_IMEMFILL;
80200a90:	8fc20000 	lw	v0,0(s8)
80200a94:	00000000 	nop
80200a98:	34420010 	ori	v0,v0,0x10
80200a9c:	afc20000 	sw	v0,0(s8)
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:260
        asm volatile("mtc0   %z0,$20"  : : "Jr" ((INT32U)(cctl_imem)));
80200aa0:	8fc20000 	lw	v0,0(s8)
80200aa4:	00000000 	nop
80200aa8:	4082a000 	mtc0	v0,$20
/cygdrive/c/rsdk/8111FPRom/flash/rlx_cache.c:262
    }
}
80200aac:	03c0e821 	move	sp,s8
80200ab0:	8fbe000c 	lw	s8,12(sp)
80200ab4:	27bd0010 	addiu	sp,sp,16
80200ab8:	03e00008 	jr	ra
80200abc:	00000000 	nop

80200ac0 <ssi_wait_nobusy>:
ssi_wait_nobusy():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:9
##the SSI_BASE is pass parameter

	.globl  ssi_wait_nobusy
	.ent    ssi_wait_nobusy
ssi_wait_nobusy:
        addiu	sp,sp,-8
80200ac0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:10
        sw      ra, 8(sp)
80200ac4:	afbf0008 	sw	ra,8(sp)

80200ac8 <wait_ssi>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:13

wait_ssi:
        lw      t0, 40(a0)
80200ac8:	8c880028 	lw	t0,40(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:14
        li      t1, 0x04 
80200acc:	24090004 	li	t1,4
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:15
        andi    t0, t0, 0x05	
80200ad0:	31080005 	andi	t0,t0,0x5
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:16
	bne     t0, t1, wait_ssi
80200ad4:	1509fffc 	bne	t0,t1,80200ac8 <wait_ssi>
80200ad8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:17
	nop
80200adc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:19

        lw      ra, 8(sp)
80200ae0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:20
        addiu	sp,sp,8
80200ae4:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:21
        jr      ra                         #Return
80200ae8:	03e00008 	jr	ra
80200aec:	00000000 	nop

80200af0 <spi_get_status>:
spi_get_status():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:32
spi_get_status:
        #a: SSI_base
        #v0: return value
        #t0, t1: using   

        addiu	sp,sp,-8
80200af0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:33
        sw      ra, 8(sp)
80200af4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:35

        sw      zero, 0x8(a0)              #disable SSI
80200af8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:36
        li      t0, 0x73c7                 
80200afc:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:37
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM
80200b00:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:38
        sw      zero, 0x4(a0)              #set read 1 of data frame
80200b04:	ac800004 	sw	zero,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:39
        li      t0, 0x1                    #enable SER               
80200b08:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:40
        sw      t0, 0x10(a0)      
80200b0c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:41
        li      t1, 0x1                       
80200b10:	24090001 	li	t1,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:42
        sw      t1, 0x8(a0)                #enable SSI
80200b14:	ac890008 	sw	t1,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:44

        li      t1, FLASH_RDSR_COM         #set read status command of SPI 
80200b18:	24090005 	li	t1,5
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:45
        sw      t1, 0x60(a0)
80200b1c:	ac890060 	sw	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:47
         
        jal     ssi_wait_nobusy       
80200b20:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200b24:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:48
        nop
80200b28:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:49
        jal     spi_wait_busy
80200b2c:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200b30:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:50
        nop
80200b34:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:52

        or      t0, zero, a0               
80200b38:	00044025 	or	t0,zero,a0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:53
        lw      v0, 0x60(a0)               #read read status
80200b3c:	8c820060 	lw	v0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:54
        nop
80200b40:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:56

        lw      ra, 8(sp)
80200b44:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:57
        addiu	sp,sp,8
80200b48:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:58
        jr      ra                         #Return
80200b4c:	03e00008 	jr	ra
80200b50:	00000000 	nop

80200b54 <spi_chip_erase>:
spi_chip_erase():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:68
        .ent    spi_chip_erase
spi_chip_erase:
        #a0: SSI_BASE
        #t0: using    

        addiu	sp,sp,-8
80200b54:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:69
        sw      ra, 8(sp)
80200b58:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:72

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSI   
80200b5c:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:73
        li      t0, 0x71c7
80200b60:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:74
        sw      t0, 0x0(a0)                #set CTRLR0: Transmit only 
80200b64:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:75
        li      t0, 0x1                 
80200b68:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:76
        sw      t0, 0x10(a0)               #enable SER      
80200b6c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:77
        li      t0, 0x1
80200b70:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:78
        sw      t0, 0x8(a0)                #enable SSI
80200b74:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:80

        li      t0, FLASH_WREN_COM
80200b78:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:81
        sw      t0, 0x60(a0)               #write enable of SPI
80200b7c:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:83
    
        jal     ssi_wait_nobusy
80200b80:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200b84:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:84
        nop
80200b88:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:86

        li      t0, FLASH_CE_COM
80200b8c:	240800c7 	li	t0,199
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:87
        sw      t0, 0x60(a0)               #set chip earse command of SPI
80200b90:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:89
        
        jal     ssi_wait_nobusy
80200b94:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200b98:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:90
        nop
80200b9c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:91
        jal     spi_enable_write           #enable WEL and pass data to set_status
80200ba0:	0c0803f4 	jal	80200fd0 <spi_enable_write>
80200ba4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:92
        nop
80200ba8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:94

        lw      ra, 8(sp)
80200bac:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:95
        addiu	sp,sp,8
80200bb0:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:96
        jr      ra                         #Return
80200bb4:	03e00008 	jr	ra
80200bb8:	00000000 	nop

80200bbc <spi_blk_erase>:
spi_blk_erase():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:106
spi_blk_erase:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80200bbc:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:107
        sw      ra, 8(sp)
80200bc0:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:110

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200bc4:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:111
        li      t0, 0x71c7
80200bc8:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:112
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200bcc:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:113
        li      t0, 0x1                 
80200bd0:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:114
        sw      t0, 0x10(a0)               #enable SER      
80200bd4:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:115
        li      t0, 0x1
80200bd8:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:116
        sw      t0, 0x8(a0)                #enable SSIENR
80200bdc:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:118

        li      t0, FLASH_WREN_COM
80200be0:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:119
        sw      t0, 0x60(a0)               #enable write of SPI
80200be4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:121
    
        jal     ssi_wait_nobusy
80200be8:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200bec:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:122
        nop
80200bf0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:124
   
        sw      zero, 0x10(a0)             #disable SER    
80200bf4:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:127
        
        #issue WR
        li      t0, FLASH_BE_COM
80200bf8:	240800d8 	li	t0,216
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:128
        andi    t2, a1,  0xff              #addr (xx_xx--)   
80200bfc:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:129
        srl     a1, a1, 8                  
80200c00:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:130
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
80200c04:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:131
        srl     a1, a1, 8
80200c08:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:132
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
80200c0c:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:135

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
80200c10:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:137
        #addr
        sw      t4, 0x60(a0)
80200c14:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:138
        sw      t3, 0x60(a0)
80200c18:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:139
        sw      t2, 0x60(a0)
80200c1c:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:141
       
        li      t0, 0x1                    #enable SER               
80200c20:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:142
        sw      t0, 0x10(a0)      
80200c24:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:144

        jal     ssi_wait_nobusy
80200c28:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200c2c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:145
        nop
80200c30:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:146
        jal     spi_wait_busy               
80200c34:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200c38:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:147
        nop
80200c3c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:149

        lw      ra, 8(sp)
80200c40:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:150
        addiu	sp,sp,8
80200c44:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:151
        jr      ra                          #Return
80200c48:	03e00008 	jr	ra
80200c4c:	00000000 	nop

80200c50 <spi_command>:
spi_command():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:162
        #a0: SSI_BASE
        #a1: address
        #a2: gernal command
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80200c50:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:163
        sw      ra, 8(sp)
80200c54:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:166

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200c58:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:167
        li      t0, 0x71c7
80200c5c:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:168
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200c60:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:169
        li      t0, 0x1                 
80200c64:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:170
        sw      t0, 0x10(a0)               #enable SER      
80200c68:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:171
        li      t0, 0x1
80200c6c:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:172
        sw      t0, 0x8(a0)                #enable SSIENR
80200c70:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:174

        li      t0, FLASH_WREN_COM
80200c74:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:175
        sw      t0, 0x60(a0)               #enable write of SPI
80200c78:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:177
    
        jal     ssi_wait_nobusy
80200c7c:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200c80:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:178
        nop
80200c84:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:180
   
        sw      zero, 0x10(a0)             #disable SER    
80200c88:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:183
        
        #issue WR
        add     t0, zero, a2
80200c8c:	00064020 	add	t0,zero,a2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:184
        andi    t2, a1,  0xff              #addr (xx_xx--)   
80200c90:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:185
        srl     a1, a1, 8                  
80200c94:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:186
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
80200c98:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:187
        srl     a1, a1, 8
80200c9c:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:188
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
80200ca0:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:191

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
80200ca4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:193
        #addr
        sw      t4, 0x60(a0)
80200ca8:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:194
        sw      t3, 0x60(a0)
80200cac:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:195
        sw      t2, 0x60(a0)
80200cb0:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:197
       
        li      t0, 0x1                    #enable SER               
80200cb4:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:198
        sw      t0, 0x10(a0)      
80200cb8:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:200

        jal     ssi_wait_nobusy
80200cbc:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200cc0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:201
        nop
80200cc4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:202
        jal     spi_wait_busy               
80200cc8:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200ccc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:203
        nop
80200cd0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:205

        lw      ra, 8(sp)
80200cd4:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:206
        addiu	sp,sp,8
80200cd8:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:207
        jr      ra                          #Return
80200cdc:	03e00008 	jr	ra
80200ce0:	00000000 	nop

80200ce4 <spi_se_unprotect>:
spi_se_unprotect():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:217
spi_se_unprotect:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80200ce4:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:218
        sw      ra, 8(sp)
80200ce8:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:221

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200cec:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:222
        li      t0, 0x71c7
80200cf0:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:223
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200cf4:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:224
        li      t0, 0x1                 
80200cf8:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:225
        sw      t0, 0x10(a0)               #enable SER      
80200cfc:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:226
        li      t0, 0x1
80200d00:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:227
        sw      t0, 0x8(a0)                #enable SSIENR
80200d04:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:229

        li      t0, FLASH_WREN_COM
80200d08:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:230
        sw      t0, 0x60(a0)               #enable write of SPI
80200d0c:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:232
    
        jal     ssi_wait_nobusy
80200d10:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200d14:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:233
        nop
80200d18:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:235
   
        sw      zero, 0x10(a0)             #disable SER    
80200d1c:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:238
        
        #issue WR
        li      t0, FLASH_SE_UNPROTECT
80200d20:	24080039 	li	t0,57
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:239
        andi    t2, a1,  0xff              #addr (xx_xx--)   
80200d24:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:240
        srl     a1, a1, 8                  
80200d28:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:241
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
80200d2c:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:242
        srl     a1, a1, 8
80200d30:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:243
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
80200d34:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:246

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
80200d38:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:248
        #addr
        sw      t4, 0x60(a0)
80200d3c:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:249
        sw      t3, 0x60(a0)
80200d40:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:250
        sw      t2, 0x60(a0)
80200d44:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:252
       
        li      t0, 0x1                    #enable SER               
80200d48:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:253
        sw      t0, 0x10(a0)      
80200d4c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:255

        jal     ssi_wait_nobusy
80200d50:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200d54:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:256
        nop
80200d58:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:257
        jal     spi_wait_busy               
80200d5c:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200d60:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:258
        nop
80200d64:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:260

        lw      ra, 8(sp)
80200d68:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:261
        addiu	sp,sp,8
80200d6c:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:262
        jr      ra                          #Return
80200d70:	03e00008 	jr	ra
80200d74:	00000000 	nop

80200d78 <spi_se_protect>:
spi_se_protect():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:272
spi_se_protect:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80200d78:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:273
        sw      ra, 8(sp)
80200d7c:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:276

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200d80:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:277
        li      t0, 0x71c7
80200d84:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:278
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200d88:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:279
        li      t0, 0x1                 
80200d8c:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:280
        sw      t0, 0x10(a0)               #enable SER      
80200d90:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:281
        li      t0, 0x1
80200d94:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:282
        sw      t0, 0x8(a0)                #enable SSIENR
80200d98:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:284

        li      t0, FLASH_WREN_COM
80200d9c:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:285
        sw      t0, 0x60(a0)               #enable write of SPI
80200da0:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:287
    
        jal     ssi_wait_nobusy
80200da4:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200da8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:288
        nop
80200dac:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:290
   
        sw      zero, 0x10(a0)             #disable SER    
80200db0:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:293
        
        #issue WR
        li      t0, FLASH_SE_PROTECT
80200db4:	24080036 	li	t0,54
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:294
        andi    t2, a1,  0xff              #addr (xx_xx--)   
80200db8:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:295
        srl     a1, a1, 8                  
80200dbc:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:296
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
80200dc0:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:297
        srl     a1, a1, 8
80200dc4:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:298
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
80200dc8:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:301

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
80200dcc:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:303
        #addr
        sw      t4, 0x60(a0)
80200dd0:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:304
        sw      t3, 0x60(a0)
80200dd4:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:305
        sw      t2, 0x60(a0)
80200dd8:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:307
       
        li      t0, 0x1                    #enable SER               
80200ddc:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:308
        sw      t0, 0x10(a0)      
80200de0:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:310

        jal     ssi_wait_nobusy
80200de4:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200de8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:311
        nop
80200dec:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:312
        jal     spi_wait_busy               
80200df0:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200df4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:313
        nop
80200df8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:315

        lw      ra, 8(sp)
80200dfc:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:316
        addiu	sp,sp,8
80200e00:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:317
        jr      ra                          #Return
80200e04:	03e00008 	jr	ra
80200e08:	00000000 	nop

80200e0c <spi_se_erase>:
spi_se_erase():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:327
spi_se_erase:
        #a0: SSI_BASE
        #a1: address
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80200e0c:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:328
        sw      ra, 8(sp)
80200e10:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:331

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200e14:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:332
        li      t0, 0x71c7
80200e18:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:333
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200e1c:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:334
        li      t0, 0x1                 
80200e20:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:335
        sw      t0, 0x10(a0)               #enable SER      
80200e24:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:336
        li      t0, 0x1
80200e28:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:337
        sw      t0, 0x8(a0)                #enable SSIENR
80200e2c:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:339

        li      t0, FLASH_WREN_COM
80200e30:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:340
        sw      t0, 0x60(a0)               #enable write of SPI
80200e34:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:342
    
        jal     ssi_wait_nobusy
80200e38:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200e3c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:343
        nop
80200e40:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:345
   
        sw      zero, 0x10(a0)             #disable SER    
80200e44:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:348
        
        #issue WR
        li      t0, FLASH_SE_COM
80200e48:	24080020 	li	t0,32
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:349
        andi    t2, a1,  0xff              #addr (xx_xx--)   
80200e4c:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:350
        srl     a1, a1, 8                  
80200e50:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:351
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
80200e54:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:352
        srl     a1, a1, 8
80200e58:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:353
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
80200e5c:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:356

        #cmd
        sw      t0, 0x60(a0)               #set sector_erase command of SPI
80200e60:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:358
        #addr
        sw      t4, 0x60(a0)
80200e64:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:359
        sw      t3, 0x60(a0)
80200e68:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:360
        sw      t2, 0x60(a0)
80200e6c:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:362
       
        li      t0, 0x1                    #enable SER               
80200e70:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:363
        sw      t0, 0x10(a0)      
80200e74:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:365

        jal     ssi_wait_nobusy
80200e78:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200e7c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:366
        nop
80200e80:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:367
        jal     spi_wait_busy               
80200e84:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200e88:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:368
        nop
80200e8c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:370

        lw      ra, 8(sp)
80200e90:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:371
        addiu	sp,sp,8
80200e94:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:372
        jr      ra                          #Return
80200e98:	03e00008 	jr	ra
80200e9c:	00000000 	nop

80200ea0 <spi_wait_busy>:
spi_wait_busy():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:382

spi_wait_busy:
        #a0: SSI_BASE
        #t0: using

        addiu	sp,sp,-8
80200ea0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:383
        sw      ra, 8(sp)
80200ea4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:385
        
        sw      zero, 0x8(a0)              #disable SSIENR
80200ea8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:386
        sw      zero, 0x4(a0)                #set read 3 of data frame
80200eac:	ac800004 	sw	zero,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:387
        li      t0, 0x73c7
80200eb0:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:388
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
80200eb4:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:389
        li      t0, 0x1
80200eb8:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:390
        sw      t0, 0x10(a0)               #enable SER
80200ebc:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:391
        li      t0, 0x1
80200ec0:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:392
        sw      t0, 0x8(a0)                #enable SSIENR
80200ec4:	ac880008 	sw	t0,8(a0)

80200ec8 <__checkstat2>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:395
    
__checkstat2:
        li      t0, FLASH_RDSR_COM                       
80200ec8:	24080005 	li	t0,5
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:396
        sw      t0, 0x60(a0)               #set read status command of SPI
80200ecc:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:398

        jal     ssi_wait_nobusy
80200ed0:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200ed4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:399
        nop
80200ed8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:401

        lw      t0, 0x60(a0)               #load status of SPI
80200edc:	8c880060 	lw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:402
        nop
80200ee0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:404

        andi    t0, t0, 0x01               #check SPI is not write progress
80200ee4:	31080001 	andi	t0,t0,0x1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:405
        bne     t0, zero, __checkstat2   
80200ee8:	1500fff7 	bnez	t0,80200ec8 <__checkstat2>
80200eec:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:406
        nop
80200ef0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:408

        lw      ra, 8(sp)
80200ef4:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:409
        addiu	sp,sp,8
80200ef8:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:410
        jr      ra                         #Return
80200efc:	03e00008 	jr	ra
80200f00:	00000000 	nop

80200f04 <spi_set_status>:
spi_set_status():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:421
spi_set_status:
        #a0: SSI_BASE, 
        #a1: set number to status register
        #t0: using

        addiu	sp,sp,-8
80200f04:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:422
        sw      ra, 8(sp)
80200f08:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:425

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200f0c:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:426
        li      t0, 0x71c7
80200f10:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:427
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200f14:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:428
        li      t0, 0x1
80200f18:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:429
        sw      t0, 0x10(a0)               #enable SER
80200f1c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:430
        li      t0, 0x1
80200f20:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:431
        sw      t0, 0x8(a0)                #enable SSIER
80200f24:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:433

        li      t0, FLASH_WREN_COM
80200f28:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:434
        sw      t0, 0x60(a0)               #enable write of SPI
80200f2c:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:436

        jal     ssi_wait_nobusy
80200f30:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200f34:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:437
        nop
80200f38:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:439

        sw      zero, 0x10(a0)             #disable SER
80200f3c:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:441
      
        li      t0, FLASH_WRSR_COM                      
80200f40:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:442
        sw      t0, 0x60(a0)               #set write status command of SPI
80200f44:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:443
        sw      a1, 0x60(a0)               #set status value
80200f48:	ac850060 	sw	a1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:445

        li      t0, 0x1
80200f4c:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:446
        sw      t0, 0x10(a0)               #enable SER
80200f50:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:448

        jal     ssi_wait_nobusy
80200f54:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200f58:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:449
        nop
80200f5c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:450
        jal     spi_wait_busy
80200f60:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200f64:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:451
        nop
80200f68:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:453
        
        lw      ra, 8(sp)
80200f6c:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:454
        addiu	sp,sp,8
80200f70:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:455
        jr      ra                         # Return
80200f74:	03e00008 	jr	ra
80200f78:	00000000 	nop

80200f7c <spi_dis_write>:
spi_dis_write():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:465

spi_dis_write: 
        #a0: SSI_BASE 
        #t0: using

        addiu	sp,sp,-8
80200f7c:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:466
        sw      ra, 8(sp)
80200f80:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:469

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80200f84:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:470
        li      t0, 0x71c7
80200f88:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:471
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200f8c:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:472
        li      t0, 0x1            
80200f90:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:473
        sw      t0, 0x10(a0)               #enable SER 
80200f94:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:474
        li      t0, 0x1
80200f98:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:475
        sw      t0, 0x8(a0)                #enable SSIENR
80200f9c:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:477

        li      t0, FLASH_WRDI_COM
80200fa0:	24080004 	li	t0,4
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:478
        sw      t0, 0x60(a0)               #disable write command of SPI
80200fa4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:480
    
        jal     ssi_wait_nobusy
80200fa8:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80200fac:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:481
        nop
80200fb0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:482
        jal     spi_wait_busy
80200fb4:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80200fb8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:483
        nop
80200fbc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:485

        lw      ra, 8(sp)
80200fc0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:486
        addiu	sp,sp,8
80200fc4:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:487
        jr      ra                         #Return
80200fc8:	03e00008 	jr	ra
80200fcc:	00000000 	nop

80200fd0 <spi_enable_write>:
spi_enable_write():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:497

spi_enable_write:  
        #a0: SSI_BASE  
        #t0: using

        addiu	sp,sp,-8
80200fd0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:498
        sw      ra, 8(sp)
80200fd4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:501

        #to command issue mode
        sw      zero, 0x8(a0)              #diable SSIENR
80200fd8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:502
        li      t0, 0x71c7 
80200fdc:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:503
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80200fe0:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:504
        li      t0, 0x1          
80200fe4:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:505
        sw      t0, 0x10(a0)               #enable SER
80200fe8:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:506
        li      t0, 0x1
80200fec:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:507
        sw      t0, 0x8(a0)                #enable SSIENR
80200ff0:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:509

        li      t0, FLASH_WREN_COM
80200ff4:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:510
        sw      t0, 0x60(a0)               #enable write command of SPI
80200ff8:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:512
    
        jal     ssi_wait_nobusy
80200ffc:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201000:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:513
        nop
80201004:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:514
        jal     spi_wait_busy
80201008:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
8020100c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:515
        nop
80201010:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:517

        lw      ra, 8(sp)
80201014:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:518
        addiu	sp,sp,8
80201018:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:519
        jr      ra                         #Return
8020101c:	03e00008 	jr	ra
80201020:	00000000 	nop

80201024 <spi_set_protect_mem>:
spi_set_protect_mem():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:530
spi_set_protect_mem: 
        #a0: SSI_BASE  
        #a1: set protect memory
        #t0, t1, t2, t3: using

        addiu	sp,sp,-8
80201024:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:531
        sw      ra, 8(sp)
80201028:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:533

        jal     spi_get_status             #return status value
8020102c:	0c0802bc 	jal	80200af0 <spi_get_status>
80201030:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:534
        nop
80201034:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:536

        or      t3, zero, v0               #reserve status value
80201038:	00025825 	or	t3,zero,v0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:539

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR 
8020103c:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:540
        li      t0, 0x71c7            
80201040:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:541
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80201044:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:542
        li      t0, 0x1
80201048:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:543
        sw      t0, 0x8(a0)                #enable SSIENR
8020104c:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:545
        
        li      t0, FLASH_WREN_COM
80201050:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:546
        sw      t0, 0x60(a0)               #enable write command of SPI
80201054:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:548
    
        jal     ssi_wait_nobusy
80201058:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020105c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:549
        nop
80201060:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:551
        
        sw      zero, 0x10(a0)             #disable SER    
80201064:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:553

        li      t0, FLASH_WRSR_COM
80201068:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:554
        li      t2, 0xe3                   #remove BP2, BP1, BP0
8020106c:	240a00e3 	li	t2,227
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:555
        and     t3, t3, t2
80201070:	016a5824 	and	t3,t3,t2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:556
        sll     t2, a1, 2
80201074:	00055080 	sll	t2,a1,0x2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:557
        or      t2, t3, t2
80201078:	016a5025 	or	t2,t3,t2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:558
        sw      t0, 0x60(a0)               #set write status command of SPI
8020107c:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:559
        sw      t2, 0x60(a0)               #new value
80201080:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:561
       
        sw      t0, 0x10(a0)               #enable SER       
80201084:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:563

        jal     ssi_wait_nobusy
80201088:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020108c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:564
        nop
80201090:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:565
        jal     spi_wait_busy
80201094:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80201098:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:566
        nop
8020109c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:568

        lw      ra, 8(sp)
802010a0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:569
        addiu	sp,sp,8
802010a4:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:570
        jr      ra                         #Return
802010a8:	03e00008 	jr	ra
802010ac:	00000000 	nop

802010b0 <spi_read_id>:
spi_read_id():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:581
spi_read_id:
        #a0: SSI_BASE  
        #t0: using
        #v0: return ID value(3-byte) 

        addiu	sp,sp,-8
802010b0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:582
        sw      ra, 8(sp)
802010b4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:585

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
802010b8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:586
        li      t0, 0x73c7            
802010bc:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:587
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
802010c0:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:588
        li      t0, 0x1           
802010c4:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:589
        sw      t0, 0x10(a0)               #enable SER   
802010c8:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:590
        li      t0, 0x2            
802010cc:	24080002 	li	t0,2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:591
        sw      t0, 0x4(a0)                #set read 3 of data frames 
802010d0:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:592
        li      t0, 0x1
802010d4:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:593
        sw      t0, 0x8(a0)                #enable SSIENR
802010d8:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:595
        
        li      t0, FLASH_RDID_COM
802010dc:	2408009f 	li	t0,159
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:596
        sw      t0, 0x60(a0)               #set read ID command of SPI
802010e0:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:598
    
        jal     ssi_wait_nobusy
802010e4:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802010e8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:599
        nop
802010ec:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:601

        lw      v0, 0x60(a0)               #read ID(--_xxxx)
802010f0:	8c820060 	lw	v0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:602
        nop 
802010f4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:604

        andi    v0, v0, 0xff
802010f8:	304200ff 	andi	v0,v0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:605
        sll     v0, v0, 0x8
802010fc:	00021200 	sll	v0,v0,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:607

        lw      t0, 0x60(a0)               #read ID(--_--xx)
80201100:	8c880060 	lw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:608
        nop
80201104:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:610

        andi    t0, t0, 0xff
80201108:	310800ff 	andi	t0,t0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:611
        or      v0, v0, t0
8020110c:	00481025 	or	v0,v0,t0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:612
        sll     v0, v0, 0x8
80201110:	00021200 	sll	v0,v0,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:614
         
        lw      t0, 0x60(a0)               #read ID(--_----)
80201114:	8c880060 	lw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:615
        nop
80201118:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:617

        andi    t0, t0, 0xff
8020111c:	310800ff 	andi	t0,t0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:618
        or      v0, v0, t0
80201120:	00481025 	or	v0,v0,t0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:620

        lw      ra, 8(sp)
80201124:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:621
        addiu	sp,sp,8
80201128:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:622
        jr      ra                         # Return
8020112c:	03e00008 	jr	ra
80201130:	00000000 	nop

80201134 <spi_read_ect_id>:
spi_read_ect_id():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:633
spi_read_ect_id:
        ## a0: SSI_BASE
        ## t0, t1, t2, t3: using
        ## v0: return ID value(2-byte)

        addiu	sp,sp,-8
80201134:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:634
        sw      ra, 8(sp)
80201138:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:637

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
8020113c:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:638
        li      t0, 0x73c7            
80201140:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:639
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode
80201144:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:640
        li      t0, 0x0                    #to read 1 bytes
80201148:	24080000 	li	t0,0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:641
        sw      t0, 0x4(a0)      
8020114c:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:642
        li      t0, 0x1
80201150:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:643
        sw      t0, 0x8(a0)                #enable SSIENR
80201154:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:645
            
        jal     ssi_wait_nobusy
80201158:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020115c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:646
        nop
80201160:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:648

        li      t0, 0x0                    #disable slave1            
80201164:	24080000 	li	t0,0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:649
        sw      t0, 0x10(a0)      
80201168:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:652

        #cmd + dummy 3write
        li      t0, FLASH_RES_COM
8020116c:	240800ab 	li	t0,171
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:653
        sw      t0, 0x60(a0)               #set read electronic ID command of SPI
80201170:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:654
        sb      t1, 0x60(a0)               #write dummy data
80201174:	a0890060 	sb	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:655
        sb      t2, 0x60(a0)               #write dummy data
80201178:	a08a0060 	sb	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:656
        sb      t3, 0x60(a0)               #write dummy data
8020117c:	a08b0060 	sb	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:658

        li      t0, 1
80201180:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:659
        sw      t0, 0x10(a0)               #enable SER
80201184:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:661

        jal     ssi_wait_nobusy
80201188:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020118c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:662
        nop
80201190:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:665

        #read ID(1-btye)
        lw      v0, 0x60(a0)               #load ID value
80201194:	8c820060 	lw	v0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:666
        nop 
80201198:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:667
        andi    v0, v0, 0xff
8020119c:	304200ff 	andi	v0,v0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:669

        lw      ra, 8(sp)
802011a0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:670
        addiu	sp,sp,8
802011a4:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:671
        jr      ra                         # Return
802011a8:	03e00008 	jr	ra
802011ac:	00000000 	nop

802011b0 <spi_read_mf_id>:
spi_read_mf_id():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:684
        # a1: formate (1: device->manufacturer, 0: manufacturer->device)
        #     2-byte
        # v0: return ID
        # t0, t1, t2: using 
        
        addiu	sp,sp,-8
802011b0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:685
        sw      ra, 8(sp)
802011b4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:688

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
802011b8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:689
        li      t0, 0x73c7            
802011bc:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:690
        sw      t0, 0x0(a0)                #set CTRLR0: EEPROM mode 
802011c0:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:691
        li      t0, 0x2             
802011c4:	24080002 	li	t0,2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:692
        sw      t0, 0x4(a0)                #to read 3 bytes 
802011c8:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:693
        li      t0, 0x1
802011cc:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:694
        sw      t0, 0x8(a0)                #enable SSIENR
802011d0:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:696
            
        jal     ssi_wait_nobusy
802011d4:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802011d8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:697
        nop
802011dc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:699

        sw      zero, 0x10(a0)             #disable SER 
802011e0:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:702

        #cmd + dummy 2write +addr
        li      t0, FLASH_REMS_COM                    
802011e4:	24080090 	li	t0,144
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:703
        sw      t0, 0x60(a0)               #set read electronic manuf
802011e8:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:704
        sb      t1, 0x60(a0)               #write dummy data
802011ec:	a0890060 	sb	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:705
        sb      t2, 0x60(a0)               #write dummy data               
802011f0:	a08a0060 	sb	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:706
        sw      a1, 0x60(a0)               #set format                     
802011f4:	ac850060 	sw	a1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:708
  
        li      t0, 0x1           
802011f8:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:709
        sw      t0, 0x10(a0)               #enable SER
802011fc:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:711

        jal     ssi_wait_nobusy
80201200:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201204:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:712
        nop
80201208:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:715

        #read ID(2-byte)
        lw      v0, 0x60(a0)               #read ID(0x--xx)
8020120c:	8c820060 	lw	v0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:716
        nop
80201210:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:717
        andi    v0, v0, 0xff
80201214:	304200ff 	andi	v0,v0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:718
        sll     v0, v0, 0x8
80201218:	00021200 	sll	v0,v0,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:720
         
        lw      t0, 0x60(a0)               #read ID(0x----)
8020121c:	8c880060 	lw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:721
        nop
80201220:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:722
        andi    t0, t0, 0xff
80201224:	310800ff 	andi	t0,t0,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:723
        or      v0, v0, t0
80201228:	00481025 	or	v0,v0,t0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:725

        lw      ra, 8(sp)
8020122c:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:726
        addiu	sp,sp,8
80201230:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:727
        jr      ra                         # Return
80201234:	03e00008 	jr	ra
80201238:	00000000 	nop

8020123c <spi_enter_dp>:
spi_enter_dp():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:737

spi_enter_dp:
        # a0: SSI_BASE
        # t0: using 

        addiu	sp,sp,-8
8020123c:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:738
        sw      ra, 8(sp)
80201240:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:741

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80201244:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:742
        li      t0, 0x71c7
80201248:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:743
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only 
8020124c:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:744
        li      t0, 0x1              
80201250:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:745
        sw      t0, 0x10(a0)               #enable SER       
80201254:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:746
        li      t0, 0x1
80201258:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:747
        sw      t0, 0x8(a0)                #enable SSIENR
8020125c:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:749

        li      t0, FLASH_DP_COM
80201260:	240800b9 	li	t0,185
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:750
        sw      t0, 0x60(a0)               #set deep power command of SPI
80201264:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:752

        jal     ssi_wait_nobusy
80201268:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020126c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:753
        nop
80201270:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:755

        lw      ra, 8(sp)
80201274:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:756
        addiu	sp,sp,8
80201278:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:757
        jr      ra                         #Return
8020127c:	03e00008 	jr	ra
80201280:	00000000 	nop

80201284 <spi_release_dp>:
spi_release_dp():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:767

spi_release_dp:
        # a0: SSI_BASE
        # t0: using 

        addiu	sp,sp,-8                   
80201284:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:768
        sw      ra, 8(sp)
80201288:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:771

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
8020128c:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:772
        li      t0, 0x71c7
80201290:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:773
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80201294:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:774
        li      t0, 0x1              
80201298:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:775
        sw      t0, 0x10(a0)               #enable SER       
8020129c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:776
        li      t0, 0x1
802012a0:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:777
        sw      t0, 0x8(a0)                #enable SSIENR
802012a4:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:779

        li      t0, FLASH_RES_COM
802012a8:	240800ab 	li	t0,171
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:780
        sw      t0, 0x60(a0)               #set release deep power command of SPI
802012ac:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:782
    
        jal     ssi_wait_nobusy
802012b0:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802012b4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:783
        nop
802012b8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:785

        lw      ra, 8(sp)
802012bc:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:786
        addiu	sp,sp,8
802012c0:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:787
        jr      ra                         #Return
802012c4:	03e00008 	jr	ra
802012c8:	00000000 	nop

802012cc <spi_write_byte>:
spi_write_byte():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:799
        #a0: SSI_BASE
        #a1: addr (3-byte)
        #a2: data (1-byte)
        #t0, t2, t3, t4: using 

        addiu	sp,sp,-8
802012cc:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:800
        sw      ra, 8(sp)
802012d0:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:803

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSI
802012d4:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:804
        li      t0, 0x71c7
802012d8:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:805
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
802012dc:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:806
        li      t0, 0x1                        
802012e0:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:807
        sw      t0, 0x10(a0)               #enable SER 
802012e4:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:808
        li      t0, 0x1
802012e8:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:809
        sw      t0, 0x8(a0)                #enable SSI
802012ec:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:812

        #issue WREN
        li      t0, FLASH_WREN_COM 
802012f0:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:813
        sw      t0, 0x60(a0)               #set write command of SPI
802012f4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:815
    
        jal     ssi_wait_nobusy
802012f8:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802012fc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:816
        nop
80201300:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:818

        sw      zero, 0x10(a0)             #disable SER                
80201304:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:821

        #issue WR
        li      t0, FLASH_PP_COM                    
80201308:	24080002 	li	t0,2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:822
        andi    t2, a1, 0xff              #addr (xx_xx--)   
8020130c:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:823
        srl     a1, a1, 8                  
80201310:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:824
        andi    t3, a1, 0xff              #addr (xx_--xx) >>8
80201314:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:825
        srl     a1, a1, 8
80201318:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:826
        andi    t4, a1, 0xff              #addr (--_xxxx) >>16
8020131c:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:828

        andi    a2, a2, 0xff              #data (xx_xx--) 
80201320:	30c600ff 	andi	a2,a2,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:831

        #cmd
        sw      t0, 0x60(a0)
80201324:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:833
        #addr
        sw      t4, 0x60(a0)
80201328:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:834
        sw      t3, 0x60(a0)
8020132c:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:835
        sw      t2, 0x60(a0)
80201330:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:837
        #data
        sw      a2, 0x60(a0)
80201334:	ac860060 	sw	a2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:839

        li      t0, 0x1                    #enable SER            
80201338:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:840
        sw      t0, 0x10(a0)      
8020133c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:842
        
        jal     ssi_wait_nobusy
80201340:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201344:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:843
        nop
80201348:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:844
        jal     spi_wait_busy
8020134c:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80201350:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:845
        nop
80201354:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:847

        lw      ra, 8(sp)
80201358:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:848
        addiu	sp,sp,8
8020135c:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:849
        jr      ra                         #Return
80201360:	03e00008 	jr	ra
80201364:	00000000 	nop

80201368 <spi_read_byte>:
spi_read_byte():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:861
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t2, t3, t4: using

        addiu	sp,sp,-8
80201368:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:862
        sw      ra, 8(sp)
8020136c:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:865

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80201370:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:866
        li      t0, 0x73c7
80201374:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:867
        sw      t0, 0x0(a0)                #set CTRLR: transmit only
80201378:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:868
        li      t0, 0x0            
8020137c:	24080000 	li	t0,0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:869
        sw      t0, 0x4(a0)                #set read 1 of data frames 
80201380:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:870
        li      t0, 0x1
80201384:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:871
        sw      t0, 0x8(a0)                #enable SSIENR
80201388:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:873

        jal     ssi_wait_nobusy
8020138c:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201390:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:874
        nop
80201394:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:877

        #issue READ
        li      t0, FLASH_READ_COM 
80201398:	24080003 	li	t0,3
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:880
    
        #issue WR
        andi    t2, a1,  0xff              #addr (xx_xx--)
8020139c:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:881
        srl     a1, a1, 8
802013a0:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:882
        andi    t3, a1,  0xff              #addr (xx_--xx) >>8
802013a4:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:883
        srl     a1, a1, 8
802013a8:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:884
        andi    t4, a1,  0xff              #addr (--_xxxx) >>16
802013ac:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:886

        sw      zero, 0x10(a0)             #disable SER 
802013b0:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:889

        #cmd(READ) +addr
        sw      t0, 0x60(a0)               #set read data command of SPI
802013b4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:890
        sw      t4, 0x60(a0)               #set read addr(--_xxxx) >>16
802013b8:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:891
        sw      t3, 0x60(a0)               #set read addr(xx_--xx) >>8
802013bc:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:892
        sw      t2, 0x60(a0)               #set read addr(xx_xx--)
802013c0:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:894
      
        li      t0, 0x1                  
802013c4:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:895
        sw      t0, 0x10(a0)               #enable SER 
802013c8:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:897

        jal     ssi_wait_nobusy
802013cc:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802013d0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:898
        nop
802013d4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:901

        #read data
        lw      s0, 0x60(a0)               #read data 1-byte
802013d8:	8c900060 	lw	s0,96(a0)
802013dc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:903
 
        or      v0, zero, s0
802013e0:	00101025 	or	v0,zero,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:905

        lw      ra, 8(sp)
802013e4:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:906
        addiu	sp,sp,8
802013e8:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:907
        jr      ra                        #Return
802013ec:	03e00008 	jr	ra
802013f0:	00000000 	nop

802013f4 <spi_fastread_byte>:
spi_fastread_byte():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:919
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-8
802013f4:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:920
        sw      ra, 8(sp)
802013f8:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:923

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
802013fc:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:924
        li      t0, 0x73c7
80201400:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:925
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80201404:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:926
        li      t0, 0x0            
80201408:	24080000 	li	t0,0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:927
        sw      t0, 0x4(a0)                #set read 1 of data frames 
8020140c:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:928
        li      t0, 0x1
80201410:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:929
        sw      t0, 0x8(a0)                #enable SSIENR
80201414:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:931
        
        sw      zero, 0x10(a0)             #disable SER 
80201418:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:934
    
        #issue WR
        andi    t2, a1, 0xff               #addr (xx_xx--) 
8020141c:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:935
        srl     a1, a1, 8
80201420:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:936
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
80201424:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:937
        srl     a1, a1, 8
80201428:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:938
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
8020142c:	30ac00ff 	andi	t4,a1,0xff

80201430 <byte_raddr>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:941
byte_raddr:   
        #cmd(fread) + addr + dummy sw
        li      t0, FLASH_FAST_READ_COM
80201430:	2408000b 	li	t0,11
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:942
        sw      t0, 0x60(a0)               #set fast read command of SPI
80201434:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:943
        sw      t4, 0x60(a0)               #set addr (--_xxxx) >>16
80201438:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:944
        sw      t3, 0x60(a0)               #set addr (xx_--xx) >>8 
8020143c:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:945
        sw      t2, 0x60(a0)               #set addr (xx_xx--) 
80201440:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:946
        sw      t1, 0x60(a0)               #set dummy data
80201444:	ac890060 	sw	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:948

        li      t0, 0x1                  
80201448:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:949
        sw      t0, 0x10(a0)               #enable SER 
8020144c:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:951

        jal     ssi_wait_nobusy
80201450:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201454:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:952
        nop
80201458:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:959
        #jal     spi_wait_busy
        #nop

 
        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
8020145c:	8c900060 	lw	s0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:960
        jal     ssi_wait_nobusy
80201460:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201464:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:961
        nop
80201468:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:963

        or      v0, zero, s0               #return value
8020146c:	00101025 	or	v0,zero,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:965
       
        lw      ra, 8(sp)
80201470:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:966
        addiu	sp,sp,8
80201474:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:967
        jr      ra                         #Return
80201478:	03e00008 	jr	ra
8020147c:	00000000 	nop

80201480 <spi_write>:
spi_write():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:979
# a0: SSI BASE
# a1: addr (3 byte)
# a2: buffer address (4 bytes)
# a3: length for write

        addiu  sp, sp, -8
80201480:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:980
        sw     ra, 8(sp)
80201484:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:983

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80201488:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:984
        li      t0, 0x71c7
8020148c:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:985
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80201490:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:986
        li      t0, 0x1                 
80201494:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:987
        sw      t0, 0x10(a0)               #enable SER 
80201498:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:988
        li      t0, 0x1
8020149c:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:989
        sw      t0, 0x8(a0)                #enable SSIENR
802014a0:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:991

        li      t0, FLASH_WREN_COM 
802014a4:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:992
        sw      t0, 0x60(a0)               #enable write of SPI
802014a8:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:994
    
        jal     ssi_wait_nobusy
802014ac:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802014b0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:995
        nop
802014b4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:997

        sw      zero, 0x10(a0)             #disable SER 
802014b8:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1000

        #issue WR
	li      t0, FLASH_PP_COM
802014bc:	24080002 	li	t0,2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1001
        andi    t2, a1, 0xff               #addr (xx_xx--)
802014c0:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1002
        srl     a1, a1, 8
802014c4:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1003
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
802014c8:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1004
        srl     a1, a1, 8
802014cc:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1005
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
802014d0:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1008

        #cmd: PP
        sw      t0, 0x60(a0)               
802014d4:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1010
        #addr
        sw      t4, 0x60(a0)               #set  addr (--_xxxx) >>16
802014d8:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1011
        sw      t3, 0x60(a0)               #set  addr (--_xxxx) >>8 
802014dc:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1012
        sw      t2, 0x60(a0)               #set  addr (--_xxxx)     
802014e0:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1015
        
        #by copying data 
	add     t0, a2, a3
802014e4:	00c74020 	add	t0,a2,a3

802014e8 <loop>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1017
loop:
        lb      t2, 0x0(a2) 
802014e8:	80ca0000 	lb	t2,0(a2)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1018
        addi    a2, a2, 1
802014ec:	20c60001 	addi	a2,a2,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1019
        sw      t2, 0x60(a0)
802014f0:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1020
        bne     t0, a2, loop 
802014f4:	1506fffc 	bne	t0,a2,802014e8 <loop>
802014f8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1022

        li      t0, 0x1                  
802014fc:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1023
        sw      t0, 0x10(a0)               #enable SER 
80201500:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1025
       
        jal     ssi_wait_nobusy
80201504:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201508:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1026
        nop
8020150c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1027
	jal     spi_wait_busy
80201510:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
80201514:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1028
	nop
80201518:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1030

	lw     ra, 8(sp)
8020151c:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1031
	addiu  sp, sp, 8 
80201520:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1032
	jr     ra
80201524:	03e00008 	jr	ra
80201528:	00000000 	nop

8020152c <spi_write_word>:
spi_write_word():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1045
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #a2: data (4-byte)
        #t0, t1, t2, t3, t4, t5: using

        addiu	sp,sp,-32
8020152c:	27bdffe0 	addiu	sp,sp,-32
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1046
        sw      ra, 16(sp)
80201530:	afbf0010 	sw	ra,16(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1049

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
80201534:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1050
        li      t0, 0x71c7
80201538:	240871c7 	li	t0,29127
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1051
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
8020153c:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1052
        li      t0, 0x1                 
80201540:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1053
        sw      t0, 0x10(a0)               #enable SER 
80201544:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1054
        li      t0, 0x1
80201548:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1055
        sw      t0, 0x8(a0)                #enable SSIENR
8020154c:	ac880008 	sw	t0,8(a0)

80201550 <word_waddr>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1059


word_waddr:   
        li      t0, FLASH_WREN_COM 
80201550:	24080006 	li	t0,6
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1060
        sw      t0, 0x60(a0)               #enable write of SPI
80201554:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1062
    
        jal     ssi_wait_nobusy
80201558:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020155c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1063
        nop
80201560:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1065

        sw      zero, 0x10(a0)             #disable SER 
80201564:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1067

        andi    t2, a1, 0xff               #addr (xx_xx--)
80201568:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1068
        srl     t5, a1, 8
8020156c:	00056a02 	srl	t5,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1069
        andi    t3, t5, 0xff               #addr (xx_--xx) >>8
80201570:	31ab00ff 	andi	t3,t5,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1070
        srl     t5, a1, 8
80201574:	00056a02 	srl	t5,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1071
        andi    t4, t5, 0xff               #addr (--_xxxx) >>16
80201578:	31ac00ff 	andi	t4,t5,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1074

        #cmd: PP
        li      t1, FLASH_PP_COM
8020157c:	24090002 	li	t1,2
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1075
        sw      t1, 0x60(a0)               #set write data command of SPI, WEL is disable after write completly.
80201580:	ac890060 	sw	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1077
        #addr
        sw      t4, 0x60(a0)               #set  addr (--_xxxx) >>16
80201584:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1078
        sw      t3, 0x60(a0)               #set  addr (--_xxxx) >>8 
80201588:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1079
        sw      t2, 0x60(a0)               #set  addr (--_xxxx)     
8020158c:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1082

        #byte data:
        andi    t5, a2, 0xff               #data (xxxx_xx--) 
80201590:	30cd00ff 	andi	t5,a2,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1083
        sw      t5, 0x60(a0)               #wirte data(1-byte)
80201594:	ac8d0060 	sw	t5,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1085
        
        srl     a2, a2, 8                  #data (xxxx_--) 
80201598:	00063202 	srl	a2,a2,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1086
        andi    t5, a2, 0xff               #data (xxxx_--) 
8020159c:	30cd00ff 	andi	t5,a2,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1087
        sw      t5, 0x60(a0)               #wirte data(2-byte)
802015a0:	ac8d0060 	sw	t5,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1089
        
        srl     a2, a2, 8                  #data (xx--) 
802015a4:	00063202 	srl	a2,a2,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1090
        andi    t5, a2, 0xff               #data (xx--) 
802015a8:	30cd00ff 	andi	t5,a2,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1091
        sw      t5, 0x60(a0)               #wirte data(3-byte)
802015ac:	ac8d0060 	sw	t5,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1093
       
        srl     a2, a2, 8                  #data (--) 
802015b0:	00063202 	srl	a2,a2,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1094
        andi    t5, a2, 0xff               #data (--) 
802015b4:	30cd00ff 	andi	t5,a2,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1095
        sw      t5, 0x60(a0)               #wirte data(4-byte)
802015b8:	ac8d0060 	sw	t5,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1097

        li      t0, 0x1                  
802015bc:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1098
        sw      t0, 0x10(a0)               #enable SER 
802015c0:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1100
       
        jal     ssi_wait_nobusy
802015c4:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802015c8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1101
        nop
802015cc:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1103
        
        jal     spi_wait_busy
802015d0:	0c0803a8 	jal	80200ea0 <spi_wait_busy>
802015d4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1104
        nop
802015d8:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1106

        lw      ra, 16(sp)
802015dc:	8fbf0010 	lw	ra,16(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1107
        addiu	sp,sp,32
802015e0:	27bd0020 	addiu	sp,sp,32
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1109

        jr      ra                         #Return
802015e4:	03e00008 	jr	ra
802015e8:	00000000 	nop

802015ec <spi_read_word>:
spi_read_word():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1121
        #a0: SSI_BASE
        #a1: addr 
        #v0: return data
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-32
802015ec:	27bdffe0 	addiu	sp,sp,-32
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1122
        sw      ra,  8(sp)
802015f0:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1123
        sw      s0,  16(sp)
802015f4:	afb00010 	sw	s0,16(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1126
        
        #to command issue mode
        sw      zero, 0x8(a0)              #disabe SSIENR
802015f8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1127
        li      t0, 0x73c7
802015fc:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1128
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
80201600:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1129
        li      t0, 0x3            
80201604:	24080003 	li	t0,3
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1130
        sw      t0, 0x4(a0)                #set read 4 of data frames 
80201608:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1131
        li      t0, 0x1
8020160c:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1132
        sw      t0, 0x8(a0)
80201610:	ac880008 	sw	t0,8(a0)

80201614 <word_raddr>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1135

word_raddr:   
        sw      zero, 0x10(a0)             #disable SER 
80201614:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1137

        andi    t2, a1, 0xff               #addr (xx_xx--)
80201618:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1138
        srl     a1, a1, 8
8020161c:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1139
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
80201620:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1140
        srl     a1, a1, 8
80201624:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1141
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
80201628:	30ac00ff 	andi	t4,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1144

        #cmd(READ) +addr
        li      t1, FLASH_READ_COM 
8020162c:	24090003 	li	t1,3
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1145
        sw      t1, 0x60(a0)
80201630:	ac890060 	sw	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1146
        sw      t4, 0x60(a0)
80201634:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1147
        sw      t3, 0x60(a0)
80201638:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1148
        sw      t2, 0x60(a0)
8020163c:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1150

        li      t0, 0x1                  
80201640:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1151
        sw      t0, 0x10(a0)               #enable SER 
80201644:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1153

        jal     ssi_wait_nobusy
80201648:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020164c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1154
        nop
80201650:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1157

        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
80201654:	8c900060 	lw	s0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1158
        jal     ssi_wait_nobusy
80201658:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020165c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1159
        nop
80201660:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1161

        lw      t4, 0x60(a0)               #load data (2-byte)
80201664:	8c8c0060 	lw	t4,96(a0)
80201668:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1162
        sll     t4, t4, 8
8020166c:	000c6200 	sll	t4,t4,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1163
        or      s0, t4, s0
80201670:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1164
        jal     ssi_wait_nobusy
80201674:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201678:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1165
        nop
8020167c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1167
      
        lw      t4, 0x60(a0)               #load data (3-byte)
80201680:	8c8c0060 	lw	t4,96(a0)
80201684:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1168
        sll     t4, t4, 16
80201688:	000c6400 	sll	t4,t4,0x10
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1169
        or      s0, t4, s0
8020168c:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1170
        jal     ssi_wait_nobusy
80201690:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201694:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1171
        nop
80201698:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1173
 
        lw      t4, 0x60(a0)               #load data (4-byte)
8020169c:	8c8c0060 	lw	t4,96(a0)
802016a0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1174
        sll     t4, t4, 24
802016a4:	000c6600 	sll	t4,t4,0x18
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1175
        or      s0, t4, s0
802016a8:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1176
        jal     ssi_wait_nobusy
802016ac:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
802016b0:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1177
        nop
802016b4:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1179
 
        or      v0, zero, s0 
802016b8:	00101025 	or	v0,zero,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1181
        
        lw      s0, 16(sp)
802016bc:	8fb00010 	lw	s0,16(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1182
        lw      ra, 8(sp)
802016c0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1183
        addiu	sp,sp,32
802016c4:	27bd0020 	addiu	sp,sp,32
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1185

        jr      ra                         #Return
802016c8:	03e00008 	jr	ra
802016cc:	00000000 	nop

802016d0 <spi_fastread_word>:
spi_fastread_word():
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1197
        #a0: SSI_BASE
        #a1: addr (3-byte) 
        #v0: return data (1-byte)
        #t0, t1, t2, t3, t4: using

        addiu	sp,sp,-8
802016d0:	27bdfff8 	addiu	sp,sp,-8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1198
        sw      ra, 8(sp)
802016d4:	afbf0008 	sw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1201

        #to command issue mode
        sw      zero, 0x8(a0)              #disable SSIENR
802016d8:	ac800008 	sw	zero,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1202
        li      t0, 0x73c7
802016dc:	240873c7 	li	t0,29639
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1203
        sw      t0, 0x0(a0)                #set CTRLR0: transmit only
802016e0:	ac880000 	sw	t0,0(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1204
        li      t0, 0x3            
802016e4:	24080003 	li	t0,3
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1205
        sw      t0, 0x4(a0)                #set read 3 of data frames 
802016e8:	ac880004 	sw	t0,4(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1206
        li      t0, 0x1
802016ec:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1207
        sw      t0, 0x8(a0)                #enable SSIENR
802016f0:	ac880008 	sw	t0,8(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1209
        
        sw      zero, 0x10(a0)             #disable SER 
802016f4:	ac800010 	sw	zero,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1213

    
        #issue WR
        andi    t2, a1, 0xff               #addr (xx_xx--) 
802016f8:	30aa00ff 	andi	t2,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1214
        srl     a1, a1, 8
802016fc:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1215
        andi    t3, a1, 0xff               #addr (xx_--xx) >>8
80201700:	30ab00ff 	andi	t3,a1,0xff
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1216
        srl     a1, a1, 8
80201704:	00052a02 	srl	a1,a1,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1217
        andi    t4, a1, 0xff               #addr (--_xxxx) >>16
80201708:	30ac00ff 	andi	t4,a1,0xff

8020170c <word_fraddr>:
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1220
word_fraddr:   
        #cmd(fread) + addr + dummy sw
        li      t0, FLASH_FAST_READ_COM
8020170c:	2408000b 	li	t0,11
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1221
        sw      t0, 0x60(a0)               #set fast read command of SPI
80201710:	ac880060 	sw	t0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1222
        sw      t4, 0x60(a0)               #set addr (--_xxxx) >>16
80201714:	ac8c0060 	sw	t4,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1223
        sw      t3, 0x60(a0)               #set addr (xx_--xx) >>8 
80201718:	ac8b0060 	sw	t3,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1224
        sw      t2, 0x60(a0)               #set addr (xx_xx--) 
8020171c:	ac8a0060 	sw	t2,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1225
        sw      t1, 0x60(a0)               #set dummy data
80201720:	ac890060 	sw	t1,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1227

        li      t0, 0x1                  
80201724:	24080001 	li	t0,1
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1228
        sw      t0, 0x10(a0)               #enable SER 
80201728:	ac880010 	sw	t0,16(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1230

        jal     ssi_wait_nobusy
8020172c:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201730:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1231
        nop
80201734:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1235
        
 
        #read data
        lw      s0, 0x60(a0)               #load data (1-byte)
80201738:	8c900060 	lw	s0,96(a0)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1236
        jal     ssi_wait_nobusy
8020173c:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201740:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1237
        nop
80201744:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1239

        lw      t4, 0x60(a0)               #load data (2-byte)
80201748:	8c8c0060 	lw	t4,96(a0)
8020174c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1240
        sll     t4, t4, 8
80201750:	000c6200 	sll	t4,t4,0x8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1241
        or      s0, t4, s0
80201754:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1242
        jal     ssi_wait_nobusy
80201758:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
8020175c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1243
        nop
80201760:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1245
      
        lw      t4, 0x60(a0)               #load data (3-byte)
80201764:	8c8c0060 	lw	t4,96(a0)
80201768:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1246
        sll     t4, t4, 16
8020176c:	000c6400 	sll	t4,t4,0x10
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1247
        or      s0, t4, s0
80201770:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1248
        jal     ssi_wait_nobusy
80201774:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201778:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1249
        nop
8020177c:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1251
 
        lw      t4, 0x60(a0)               #load data (4-byte)
80201780:	8c8c0060 	lw	t4,96(a0)
80201784:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1252
        sll     t4, t4, 24
80201788:	000c6600 	sll	t4,t4,0x18
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1253
        or      s0, t4, s0
8020178c:	01908025 	or	s0,t4,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1254
        jal     ssi_wait_nobusy
80201790:	0c0802b0 	jal	80200ac0 <ssi_wait_nobusy>
80201794:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1255
        nop
80201798:	00000000 	nop
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1257
 
        or      v0, zero, s0               #return value
8020179c:	00101025 	or	v0,zero,s0
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1259
       
        lw      ra, 8(sp)
802017a0:	8fbf0008 	lw	ra,8(sp)
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1260
        addiu	sp,sp,8
802017a4:	27bd0008 	addiu	sp,sp,8
/cygdrive/c/rsdk/8111FPRom/flash/spi.S:1261
        jr      ra                         #Return
802017a8:	03e00008 	jr	ra
802017ac:	00000000 	nop

802017b0 <memcmp>:
memcmp():
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:69
802017b0:	10c0000c 	beqz	a2,802017e4 <memcmp+0x34>
802017b4:	24c3ffff 	addiu	v1,a2,-1
802017b8:	2407ffff 	li	a3,-1
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:64
802017bc:	90860000 	lbu	a2,0(a0)
802017c0:	90a20000 	lbu	v0,0(a1)
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:69
802017c4:	2463ffff 	addiu	v1,v1,-1
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:68
802017c8:	24840001 	addiu	a0,a0,1
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:64
802017cc:	10c20003 	beq	a2,v0,802017dc <memcmp+0x2c>
802017d0:	24a50001 	addiu	a1,a1,1
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:66
802017d4:	03e00008 	jr	ra
802017d8:	00c21023 	subu	v0,a2,v0
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:69
802017dc:	1467fff7 	bne	v1,a3,802017bc <memcmp+0xc>
802017e0:	00000000 	nop
/home/phinex/newlibmips/mips-elf/newlib/libc/string/../../../../../newlibsource/newlib/libc/string/memcmp.c:112
802017e4:	03e00008 	jr	ra
802017e8:	00001021 	move	v0,zero

802017ec <memset>:
memset():
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:71
802017ec:	2cc20010 	sltiu	v0,a2,16
802017f0:	14400020 	bnez	v0,80201874 <memset+0x88>
802017f4:	00803821 	move	a3,a0
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:73
802017f8:	30830003 	andi	v1,a0,0x3
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:78
802017fc:	10600008 	beqz	v1,80201820 <memset+0x34>
80201800:	2c620004 	sltiu	v0,v1,4
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:81
80201804:	10400006 	beqz	v0,80201820 <memset+0x34>
80201808:	24630001 	addiu	v1,v1,1
8020180c:	a0e50000 	sb	a1,0(a3)
80201810:	2c620004 	sltiu	v0,v1,4
80201814:	24c6ffff 	addiu	a2,a2,-1
80201818:	08080601 	j	80201804 <memset+0x18>
8020181c:	24e70001 	addiu	a3,a3,1
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:92
80201820:	30a500ff 	andi	a1,a1,0xff
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:94
80201824:	10a00005 	beqz	a1,8020183c <memset+0x50>
80201828:	00a01821 	move	v1,a1
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:98
8020182c:	00051200 	sll	v0,a1,0x8
80201830:	00a21825 	or	v1,a1,v0
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:99
80201834:	00031400 	sll	v0,v1,0x10
80201838:	00621825 	or	v1,v1,v0
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:114
8020183c:	000610c2 	srl	v0,a2,0x3
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:115
80201840:	30c60007 	andi	a2,a2,0x7
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:121
80201844:	10400005 	beqz	v0,8020185c <memset+0x70>
80201848:	2442ffff 	addiu	v0,v0,-1
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:118
8020184c:	ace30000 	sw	v1,0(a3)
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:119
80201850:	ace30004 	sw	v1,4(a3)
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:120
80201854:	08080611 	j	80201844 <memset+0x58>
80201858:	24e70008 	addiu	a3,a3,8
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:124
8020185c:	2cc20004 	sltiu	v0,a2,4
80201860:	14400004 	bnez	v0,80201874 <memset+0x88>
80201864:	00000000 	nop
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:126
80201868:	ace30000 	sw	v1,0(a3)
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:127
8020186c:	24c6fffc 	addiu	a2,a2,-4
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:126
80201870:	24e70004 	addiu	a3,a3,4
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:137
80201874:	10c00004 	beqz	a2,80201888 <memset+0x9c>
80201878:	24c6ffff 	addiu	a2,a2,-1
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:136
8020187c:	a0e50000 	sb	a1,0(a3)
80201880:	0808061d 	j	80201874 <memset+0x88>
80201884:	24e70001 	addiu	a3,a3,1
/home/phinex/newlibmips/mips-elf/newlib/libc/machine/mips/../../../../../../newlibsource/newlib/libc/machine/mips/memset.c:142
80201888:	03e00008 	jr	ra
8020188c:	00801021 	move	v0,a0

Disassembly of section .bss:

80201890 <__bss_start>:
	...

802018a0 <_stack_lo>:
	...

80201ca0 <_stack_hi>:
	...

Disassembly of section .reginfo:

00000000 <.reginfo>:
   0:	e00000fc 	sc	zero,252(zero)
	...
  14:	80201890 	lb	zero,6288(at)

Disassembly of section .pdr:

00000000 <.pdr>:
   0:	80200078 	lb	zero,120(at)
   4:	40000000 	mfc0	zero,$0
   8:	fffffffc 	0xfffffffc
	...
  14:	00000008 	jr	zero
  18:	0000001e 	0x1e
  1c:	0000001f 	0x1f
  20:	802000d4 	lb	zero,212(at)
  24:	40000000 	mfc0	zero,$0
  28:	fffffffc 	0xfffffffc
	...
  34:	00000008 	jr	zero
  38:	0000001e 	0x1e
  3c:	0000001f 	0x1f
  40:	802000f4 	lb	zero,244(at)
  44:	40000000 	mfc0	zero,$0
  48:	fffffffc 	0xfffffffc
	...
  54:	00000008 	jr	zero
  58:	0000001e 	0x1e
  5c:	0000001f 	0x1f
  60:	80200114 	lb	zero,276(at)
  64:	40000000 	mfc0	zero,$0
  68:	fffffffc 	0xfffffffc
	...
  74:	00000008 	jr	zero
  78:	0000001e 	0x1e
  7c:	0000001f 	0x1f
  80:	80200134 	lb	zero,308(at)
  84:	40000000 	mfc0	zero,$0
  88:	fffffffc 	0xfffffffc
	...
  94:	00000008 	jr	zero
  98:	0000001e 	0x1e
  9c:	0000001f 	0x1f
  a0:	80200154 	lb	zero,340(at)
  a4:	40000000 	mfc0	zero,$0
  a8:	fffffffc 	0xfffffffc
	...
  b4:	00000008 	jr	zero
  b8:	0000001e 	0x1e
  bc:	0000001f 	0x1f
  c0:	80200174 	lb	zero,372(at)
  c4:	40000000 	mfc0	zero,$0
  c8:	fffffffc 	0xfffffffc
	...
  d4:	00000008 	jr	zero
  d8:	0000001e 	0x1e
  dc:	0000001f 	0x1f
  e0:	80200194 	lb	zero,404(at)
  e4:	40000000 	mfc0	zero,$0
  e8:	fffffffc 	0xfffffffc
	...
  f4:	00000008 	jr	zero
  f8:	0000001e 	0x1e
  fc:	0000001f 	0x1f
 100:	802001b4 	lb	zero,436(at)
 104:	40000000 	mfc0	zero,$0
 108:	fffffffc 	0xfffffffc
	...
 114:	00000008 	jr	zero
 118:	0000001e 	0x1e
 11c:	0000001f 	0x1f
 120:	802001d4 	lb	zero,468(at)
 124:	40000000 	mfc0	zero,$0
 128:	fffffffc 	0xfffffffc
	...
 134:	00000008 	jr	zero
 138:	0000001e 	0x1e
 13c:	0000001f 	0x1f
 140:	802001f4 	lb	zero,500(at)
 144:	40000000 	mfc0	zero,$0
 148:	fffffffc 	0xfffffffc
	...
 154:	00000008 	jr	zero
 158:	0000001e 	0x1e
 15c:	0000001f 	0x1f
 160:	80200214 	lb	zero,532(at)
 164:	c0000000 	ll	zero,0(zero)
 168:	fffffffc 	0xfffffffc
	...
 174:	00000028 	0x28
 178:	0000001e 	0x1e
 17c:	0000001f 	0x1f
 180:	802006fc 	lb	zero,1788(at)
 184:	40000000 	mfc0	zero,$0
 188:	fffffffc 	0xfffffffc
	...
 194:	00000010 	mfhi	zero
 198:	0000001e 	0x1e
 19c:	0000001f 	0x1f
 1a0:	80200778 	lb	zero,1912(at)
 1a4:	40000000 	mfc0	zero,$0
 1a8:	fffffffc 	0xfffffffc
	...
 1b4:	00000010 	mfhi	zero
 1b8:	0000001e 	0x1e
 1bc:	0000001f 	0x1f
 1c0:	802007f4 	lb	zero,2036(at)
 1c4:	40000000 	mfc0	zero,$0
 1c8:	fffffffc 	0xfffffffc
	...
 1d4:	00000010 	mfhi	zero
 1d8:	0000001e 	0x1e
 1dc:	0000001f 	0x1f
 1e0:	80200870 	lb	zero,2160(at)
 1e4:	40000000 	mfc0	zero,$0
 1e8:	fffffffc 	0xfffffffc
	...
 1f4:	00000010 	mfhi	zero
 1f8:	0000001e 	0x1e
 1fc:	0000001f 	0x1f
 200:	802008c4 	lb	zero,2244(at)
 204:	40000000 	mfc0	zero,$0
 208:	fffffffc 	0xfffffffc
	...
 214:	00000010 	mfhi	zero
 218:	0000001e 	0x1e
 21c:	0000001f 	0x1f
 220:	80200918 	lb	zero,2328(at)
 224:	40000000 	mfc0	zero,$0
 228:	fffffffc 	0xfffffffc
	...
 234:	00000010 	mfhi	zero
 238:	0000001e 	0x1e
 23c:	0000001f 	0x1f
 240:	802009c8 	lb	zero,2504(at)
 244:	40000000 	mfc0	zero,$0
 248:	fffffffc 	0xfffffffc
	...
 254:	00000010 	mfhi	zero
 258:	0000001e 	0x1e
 25c:	0000001f 	0x1f
 260:	80200a2c 	lb	zero,2604(at)
 264:	40000000 	mfc0	zero,$0
 268:	fffffffc 	0xfffffffc
	...
 274:	00000010 	mfhi	zero
 278:	0000001e 	0x1e
 27c:	0000001f 	0x1f
 280:	80200000 	lb	zero,0(at)
	...
 2a0:	80200ac0 	lb	zero,2752(at)
	...
 2c0:	80200af0 	lb	zero,2800(at)
	...
 2e0:	80200b54 	lb	zero,2900(at)
	...
 300:	80200bbc 	lb	zero,3004(at)
	...
 320:	80200c50 	lb	zero,3152(at)
	...
 340:	80200ce4 	lb	zero,3300(at)
	...
 360:	80200d78 	lb	zero,3448(at)
	...
 380:	80200e0c 	lb	zero,3596(at)
	...
 3a0:	80200ea0 	lb	zero,3744(at)
	...
 3c0:	80200f04 	lb	zero,3844(at)
	...
 3e0:	80200f7c 	lb	zero,3964(at)
	...
 400:	80200fd0 	lb	zero,4048(at)
	...
 420:	80201024 	lb	zero,4132(at)
	...
 440:	802010b0 	lb	zero,4272(at)
	...
 460:	80201134 	lb	zero,4404(at)
	...
 480:	802011b0 	lb	zero,4528(at)
	...
 4a0:	8020123c 	lb	zero,4668(at)
	...
 4c0:	80201284 	lb	zero,4740(at)
	...
 4e0:	802012cc 	lb	zero,4812(at)
	...
 500:	80201368 	lb	zero,4968(at)
	...
 520:	802013f4 	lb	zero,5108(at)
	...
 540:	80201480 	lb	zero,5248(at)
	...
 560:	8020152c 	lb	zero,5420(at)
	...
 580:	802015ec 	lb	zero,5612(at)
	...
 5a0:	802016d0 	lb	zero,5840(at)
	...
 5c0:	802017b0 	lb	zero,6064(at)
	...
 5d8:	0000001d 	0x1d
 5dc:	0000001f 	0x1f
 5e0:	802017ec 	lb	zero,6124(at)
	...
 5f8:	0000001d 	0x1d
 5fc:	0000001f 	0x1f

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
  20:	29383830 	slti	t8,t1,14384
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
   4:	756e6700 	jalx	5b99c00 <_boot-0x7a666400>0x6700
   8:	00070100 	sll	zero,a3,0x4
   c:	03040000 	0x3040000

Disassembly of section .debug_info:

00000000 <.debug_info>:
   0:	00000213 	0x213
   4:	00000004 	sllv	zero,zero,zero
   8:	01040000 	0x1040000
   c:	0000014b 	0x14b
  10:	00010101 	0x10101
  14:	00012a00 	sll	a1,at,0x8
  18:	20007800 	addi	zero,zero,30720
  1c:	00068480 	sll	s0,a2,0x12
  20:	00000000 	nop
  24:	08010200 	j	40800 <_boot-0x801bf800>
  28:	0000008a 	0x8a
  2c:	00007803 	sra	t7,zero,0x0
  30:	25190200 	addiu	t9,t0,512
  34:	02000000 	0x2000000
  38:	008c0601 	0x8c0601
  3c:	02020000 	0x2020000
  40:	0000bc07 	0xbc07
  44:	05020200 	0x5020200
  48:	0000003c 	0x3c
  4c:	00004b03 	sra	t1,zero,0xc
  50:	571d0200 	0x571d0200
  54:	02000000 	0x2000000
  58:	00c20704 	0xc20704
  5c:	04040000 	0x4040000
  60:	746e6905 	jalx	1b9a414 <_boot-0x7e665bec>0x6905
  64:	00d60500 	0xd60500
  68:	0e010000 	jal	8040000 <_boot-0x781c0000>
  6c:	0000005e 	0x5e
  70:	80200078 	lb	zero,120(at)
  74:	0000005c 	0x5c
  78:	66069c01 	0x66069c01
  7c:	01000000 	0x1000000
  80:	2000d417 	addi	zero,zero,-11241
  84:	00002080 	sll	a0,zero,0x2
  88:	069c0100 	0x69c0100
  8c:	000000ec 	0xec
  90:	00f41801 	0xf41801
  94:	00208020 	add	s0,at,zero
  98:	9c010000 	0x9c010000
  9c:	0000f306 	0xf306
  a0:	14190100 	bne	zero,t9,4a4 <_boot-0x801ffb5c>
  a4:	20802001 	addi	zero,a0,8193
  a8:	01000000 	0x1000000
  ac:	00fa069c 	0xfa069c
  b0:	1a010000 	0x1a010000
  b4:	80200134 	lb	zero,308(at)
  b8:	00000020 	add	zero,zero,zero
  bc:	cf069c01 	lwc3	$6,-25599(t8)
  c0:	01000000 	0x1000000
  c4:	2001541b 	addi	at,zero,21531
  c8:	00002080 	sll	a0,zero,0x2
  cc:	069c0100 	0x69c0100
  d0:	00000107 	0x107
  d4:	01741c01 	0x1741c01
  d8:	00208020 	add	s0,at,zero
  dc:	9c010000 	0x9c010000
  e0:	00010e06 	0x10e06
  e4:	941d0100 	lhu	sp,256(zero)
  e8:	20802001 	addi	zero,a0,8193
  ec:	01000000 	0x1000000
  f0:	0115069c 	0x115069c
  f4:	1e010000 	0x1e010000
  f8:	802001b4 	lb	zero,436(at)
  fc:	00000020 	add	zero,zero,zero
 100:	1c069c01 	0x1c069c01
 104:	01000001 	0x1000001
 108:	2001d41f 	addi	at,zero,-11233
 10c:	00002080 	sll	a0,zero,0x2
 110:	069c0100 	0x69c0100
 114:	00000123 	0x123
 118:	01f42001 	0x1f42001
 11c:	00208020 	add	s0,at,zero
 120:	9c010000 	0x9c010000
 124:	00004607 	0x4607
 128:	5e220100 	0x5e220100
 12c:	14000000 	bnez	zero,130 <_boot-0x801ffed0>
 130:	e8802002 	swc2	$0,8194(a0)
 134:	01000004 	sllv	zero,zero,t0
 138:	0001fd9c 	0x1fd9c
 13c:	00690800 	0x690800
 140:	004c2401 	0x4c2401
 144:	91020000 	lbu	v0,0(t0)
 148:	006c0968 	0x6c0968
 14c:	000a0000 	sll	zero,t2,0x0
 150:	01000000 	0x1000000
 154:	0001fd25 	0x1fd25
 158:	6c910200 	0x6c910200
 15c:	0000a60a 	0xa60a
 160:	4c260100 	0x4c260100
 164:	02000000 	0x2000000
 168:	7e0b7091 	0x7e0b7091
 16c:	01000000 	0x1000000
 170:	00005e26 	0x5e26
 174:	00017b00 	sll	t7,at,0xc
 178:	0b000c00 	j	c003000 <_boot-0x741fd000>
 17c:	00000008 	jr	zero
 180:	005e2701 	0x5e2701
 184:	018c0000 	0x18c0000
 188:	000c0000 	sll	zero,t4,0x0
 18c:	00002c0b 	0x2c0b
 190:	5e280100 	0x5e280100
 194:	9d000000 	0x9d000000
 198:	0c000001 	jal	4 <_boot-0x801ffffc>
 19c:	00ab0b00 	0xab0b00
 1a0:	31010000 	andi	at,t0,0x0
 1a4:	0000005e 	0x5e
 1a8:	000001ae 	0x1ae
 1ac:	980b000c 	lwr	t3,12(zero)
 1b0:	01000000 	0x1000000
 1b4:	00005e32 	0x5e32
 1b8:	0001bf00 	sll	s7,at,0x1c
 1bc:	0d000c00 	jal	4003000 <_boot-0x7c1fd000>
 1c0:	0000005f 	0x5f
 1c4:	02030003 	0x2030003
 1c8:	01de0000 	0x1de0000
 1cc:	030e0000 	0x30e0000
 1d0:	0e000002 	jal	8000008 <_boot-0x781ffff8>
 1d4:	0000005e 	0x5e
 1d8:	0000570e 	0x570e
 1dc:	220b0000 	addi	t3,s0,0
 1e0:	01000000 	0x1000000
 1e4:	00005e38 	0x5e38
 1e8:	0001ef00 	sll	sp,at,0x1c
 1ec:	0f000c00 	jal	c003000 <_boot-0x741fd000>
 1f0:	00000058 	0x58
 1f4:	005e0003 	0x5e0003
 1f8:	000c0000 	sll	zero,t4,0x0
 1fc:	49041000 	0x49041000
 200:	11000001 	beqz	t0,208 <_boot-0x801ffdf8>
 204:	00520a04 	0x520a04
 208:	04010000 	b	20c <_boot-0x801ffdf4>
 20c:	0000002c 	0x2c
 210:	18900305 	0x18900305
 214:	38008020 	xori	zero,zero,0x8020
 218:	04000002 	bltz	zero,224 <_boot-0x801ffddc>
 21c:	0000fd00 	sll	ra,zero,0x14
 220:	4b010400 	c2	0x1010400
 224:	01000001 	0x1000001
 228:	000001de 	0x1de
 22c:	0000012a 	0x12a
 230:	802006fc 	lb	zero,1788(at)
 234:	000003c4 	0x3c4
 238:	00000105 	0x105
 23c:	0001b602 	srl	s6,at,0x18
 240:	30180200 	andi	t8,zero,0x200
 244:	03000000 	0x3000000
 248:	008a0801 	0x8a0801
 24c:	01030000 	0x1030000
 250:	00008c06 	0x8c06
 254:	07020300 	0x7020300
 258:	000000bc 	0xbc
 25c:	3c050203 	lui	a1,0x203
 260:	02000000 	0x2000000
 264:	0000004b 	0x4b
 268:	00571d02 	0x571d02
 26c:	04030000 	0x4030000
 270:	0000c207 	0xc207
 274:	05040400 	0x5040400
 278:	00746e69 	0x746e69
 27c:	00021a05 	0x21a05
 280:	fc340100 	0xfc340100
 284:	7c802006 	0x7c802006
 288:	01000000 	0x1000000
 28c:	0000a59c 	0xa59c
 290:	01ea0600 	0x1ea0600
 294:	34010000 	li	at,0x0
 298:	0000004c 	syscall	0x1
 29c:	06009102 	bltz	s0,fffe46a8 <__bss_end+0x7fde29e8>
 2a0:	0000023b 	0x23b
 2a4:	004c3401 	0x4c3401
 2a8:	91020000 	lbu	v0,0(t0)
 2ac:	01b10704 	0x1b10704
 2b0:	36010000 	ori	at,s0,0x0
 2b4:	0000004c 	syscall	0x1
 2b8:	00709102 	0x709102
 2bc:	00024005 	0x24005
 2c0:	784d0100 	0x784d0100
 2c4:	7c802007 	0x7c802007
 2c8:	01000000 	0x1000000
 2cc:	0000e59c 	0xe59c
 2d0:	01ea0600 	0x1ea0600
 2d4:	4d010000 	bc3t	2d8 <_boot-0x801ffd28>
 2d8:	0000004c 	syscall	0x1
 2dc:	06009102 	bltz	s0,fffe46e8 <__bss_end+0x7fde2a28>
 2e0:	0000023b 	0x23b
 2e4:	004c4d01 	0x4c4d01
 2e8:	91020000 	lbu	v0,0(t0)
 2ec:	01b10704 	0x1b10704
 2f0:	4f010000 	c3	0x1010000
 2f4:	0000004c 	syscall	0x1
 2f8:	00709102 	0x709102
 2fc:	00018c05 	0x18c05
 300:	f4670100 	0xf4670100
 304:	7c802007 	0x7c802007
 308:	01000000 	0x1000000
 30c:	0001259c 	0x1259c
 310:	01ea0600 	0x1ea0600
 314:	67010000 	0x67010000
 318:	0000004c 	syscall	0x1
 31c:	06009102 	bltz	s0,fffe4728 <__bss_end+0x7fde2a68>
 320:	0000023b 	0x23b
 324:	004c6701 	0x4c6701
 328:	91020000 	lbu	v0,0(t0)
 32c:	01b10704 	0x1b10704
 330:	69010000 	0x69010000
 334:	0000004c 	syscall	0x1
 338:	00709102 	0x709102
 33c:	0001f205 	0x1f205
 340:	70800100 	0x70800100
 344:	54802008 	0x54802008
 348:	01000000 	0x1000000
 34c:	0001499c 	0x1499c
 350:	020c0700 	0x20c0700
 354:	82010000 	lb	at,0(s0)
 358:	0000004c 	syscall	0x1
 35c:	00709102 	0x709102
 360:	00000805 	0x805
 364:	c4980100 	lwc1	$f24,256(a0)
 368:	54802008 	0x54802008
 36c:	01000000 	0x1000000
 370:	00016d9c 	0x16d9c
 374:	020c0700 	0x20c0700
 378:	9a010000 	lwr	at,0(s0)
 37c:	0000004c 	syscall	0x1
 380:	00709102 	0x709102
 384:	00022b08 	0x22b08
 388:	25b10100 	addiu	s1,t5,256
 38c:	18000000 	blez	zero,390 <_boot-0x801ffc70>
 390:	b0802009 	0xb0802009
 394:	01000000 	0x1000000
 398:	0001b19c 	0x1b19c
 39c:	02150600 	0x2150600
 3a0:	b1010000 	0xb1010000
 3a4:	0000004c 	syscall	0x1
 3a8:	07009102 	bltz	t8,fffe47b4 <__bss_end+0x7fde2af4>
 3ac:	0000020c 	syscall	0x8
 3b0:	004cb301 	0x4cb301
 3b4:	91020000 	lbu	v0,0(t0)
 3b8:	01ce0774 	0x1ce0774
 3bc:	b4010000 	0xb4010000
 3c0:	00000025 	move	zero,zero
 3c4:	00709102 	0x709102
 3c8:	00002c05 	0x2c05
 3cc:	c8de0100 	lwc2	$30,256(a2)
 3d0:	64802009 	0x64802009
 3d4:	01000000 	0x1000000
 3d8:	0001f19c 	0x1f19c
 3dc:	01d90600 	0x1d90600
 3e0:	de010000 	0xde010000
 3e4:	0000004c 	syscall	0x1
 3e8:	09009102 	j	4024408 <_boot-0x7c1dbbf8>
 3ec:	00706f74 	0x706f74
 3f0:	004cde01 	0x4cde01
 3f4:	91020000 	lbu	v0,0(t0)
 3f8:	019d0704 	0x19d0704
 3fc:	e0010000 	sc	at,0(zero)
 400:	0000004c 	syscall	0x1
 404:	00709102 	0x709102
 408:	0001be0a 	0x1be0a
 40c:	2cf70100 	sltiu	s7,a3,256
 410:	9480200a 	lhu	zero,8202(a0)
 414:	01000000 	0x1000000
 418:	01d9069c 	0x1d9069c
 41c:	f7010000 	0xf7010000
 420:	0000004c 	syscall	0x1
 424:	09009102 	j	4024408 <_boot-0x7c1dbbf8>
 428:	00706f74 	0x706f74
 42c:	004cf701 	0x4cf701
 430:	91020000 	lbu	v0,0(t0)
 434:	02150604 	0x2150604
 438:	f7010000 	0xf7010000
 43c:	0000004c 	syscall	0x1
 440:	07089102 	0x7089102
 444:	000001a7 	0x1a7
 448:	004cf901 	0x4cf901
 44c:	91020000 	lbu	v0,0(t0)
 450:	4c000070 	0x4c000070
 454:	02000000 	0x2000000
 458:	0001af00 	sll	s5,at,0x1c
 45c:	d7010400 	0xd7010400
 460:	00000001 	0x1
 464:	78802000 	0x78802000
 468:	63802000 	0x63802000
 46c:	2e307472 	sltiu	s0,s1,29810
 470:	632f0053 	0x632f0053
 474:	72646779 	0x72646779
 478:	2f657669 	sltiu	a1,k1,30313
 47c:	73722f63 	0x73722f63
 480:	382f6b64 	xori	t7,at,0x6b64
 484:	46313131 	c1	0x313131
 488:	6d6f5250 	0x6d6f5250
 48c:	616c662f 	0x616c662f
 490:	47006873 	c1	0x1006873
 494:	4120554e 	0x4120554e
 498:	2e322053 	sltiu	s2,s1,8275
 49c:	302e3432 	andi	t6,at,0x3432
 4a0:	4b800100 	c2	0x1800100
 4a4:	02000000 	0x2000000
 4a8:	0001c300 	sll	t8,at,0xc
 4ac:	23010400 	addi	at,t8,1024
 4b0:	c0000002 	ll	zero,2(zero)
 4b4:	b080200a 	0xb080200a
 4b8:	73802017 	0x73802017
 4bc:	532e6970 	0x532e6970
 4c0:	79632f00 	0x79632f00
 4c4:	69726467 	0x69726467
 4c8:	632f6576 	0x632f6576
 4cc:	6473722f 	0x6473722f
 4d0:	31382f6b 	andi	t8,t1,0x2f6b
 4d4:	50463131 	0x50463131
 4d8:	2f6d6f52 	sltiu	t5,k1,28498
 4dc:	73616c66 	0x73616c66
 4e0:	4e470068 	c3	0x470068
 4e4:	53412055 	0x53412055
 4e8:	322e3220 	andi	t6,s1,0x3220
 4ec:	00302e34 	0x302e34
 4f0:	01ad8001 	0x1ad8001
 4f4:	00020000 	sll	zero,v0,0x0
 4f8:	000001d7 	0x1d7
 4fc:	05600104 	bltz	t3,910 <_boot-0x801ff6f0>
 500:	17ec0000 	bne	ra,t4,504 <_boot-0x801ffafc>
 504:	17b08020 	bne	sp,s0,fffe0588 <__bss_end+0x7fdde8c8>
 508:	4e478020 	c3	0x478020
 50c:	20432055 	addi	v1,v0,8277
 510:	2e342e33 	sltiu	s4,s1,11827
 514:	2e312d36 	sltiu	s1,s1,11574
 518:	20362e33 	addi	s6,at,11827
 51c:	0100672d 	0x100672d
 520:	2e2f2e2e 	sltiu	t7,s1,11822
 524:	2e2e2f2e 	sltiu	t6,s1,12078
 528:	2f2e2e2f 	sltiu	t6,t9,11823
 52c:	6e2f2e2e 	0x6e2f2e2e
 530:	696c7765 	0x696c7765
 534:	756f7362 	jalx	5bdcd88 <_boot-0x7a623278>0x7362
 538:	2f656372 	sltiu	a1,k1,25458
 53c:	6c77656e 	0x6c77656e
 540:	6c2f6269 	0x6c2f6269
 544:	2f636269 	sltiu	v1,k1,25193
 548:	69727473 	0x69727473
 54c:	6d2f676e 	0x6d2f676e
 550:	6d636d65 	0x6d636d65
 554:	00632e70 	0x632e70
 558:	6d6f682f 	0x6d6f682f
 55c:	68702f65 	0x68702f65
 560:	78656e69 	0x78656e69
 564:	77656e2f 	jalx	d95b8bc <_boot-0x728a4744>0x6e2f
 568:	6d62696c 	0x6d62696c
 56c:	2f737069 	sltiu	s3,k1,28777
 570:	7370696d 	0x7370696d
 574:	666c652d 	0x666c652d
 578:	77656e2f 	jalx	d95b8bc <_boot-0x728a4744>0x6e2f
 57c:	2f62696c 	sltiu	v0,k1,26988
 580:	6362696c 	0x6362696c
 584:	7274732f 	0x7274732f
 588:	00676e69 	0x676e69
 58c:	746e6902 	jalx	1b9a408 <_boot-0x7e665bf8>0x6902
 590:	02050400 	0x2050400
 594:	676e6f6c 	0x676e6f6c
 598:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 59c:	02050400 	0x2050400
 5a0:	676e6f6c 	0x676e6f6c
 5a4:	6e6f6c20 	0x6e6f6c20
 5a8:	6e692067 	0x6e692067
 5ac:	05080074 	0x5080074
 5b0:	00025603 	sra	t2,v0,0x18
 5b4:	03070400 	0x3070400
 5b8:	00000256 	0x256
 5bc:	75020704 	jalx	4081c10 <_boot-0x7c17e3f0>0x704
 5c0:	6769736e 	0x6769736e
 5c4:	2064656e 	addi	a0,v1,25966
 5c8:	72616863 	0x72616863
 5cc:	02080100 	0x2080100
 5d0:	676e6f6c 	0x676e6f6c
 5d4:	736e7520 	0x736e7520
 5d8:	656e6769 	0x656e6769
 5dc:	6e692064 	0x6e692064
 5e0:	07040074 	0x7040074
 5e4:	00cc0404 	0xcc0404
 5e8:	73020000 	mad	t8,v0
 5ec:	74726f68 	jalx	1c9bda0 <_boot-0x7e564260>0x6f68
 5f0:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 5f4:	02050200 	0x2050200
 5f8:	72616863 	0x72616863
 5fc:	02060100 	0x2060100
 600:	726f6873 	0x726f6873
 604:	6e752074 	0x6e752074
 608:	6e676973 	0x6e676973
 60c:	69206465 	0x69206465
 610:	0200746e 	0x200746e
 614:	6f6c0207 	0x6f6c0207
 618:	6c20676e 	0x6c20676e
 61c:	20676e6f 	addi	a3,v1,28271
 620:	69736e75 	0x69736e75
 624:	64656e67 	0x64656e67
 628:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 62c:	05070800 	0x5070800
 630:	657a6973 	0x657a6973
 634:	0200745f 	0x200745f
 638:	0000bed5 	0xbed5
 63c:	01a90600 	0x1a90600
 640:	6d010000 	0x6d010000
 644:	6d636d65 	0x6d636d65
 648:	39010070 	xori	at,t0,0x70
 64c:	00009a01 	0x9a01
 650:	2017b000 	addi	s7,zero,-20480
 654:	2017ec80 	addi	s7,zero,-4992
 658:	0002d480 	sll	k0,v0,0x12
 65c:	076d0100 	0x76d0100
 660:	0100316d 	0x100316d
 664:	0001a935 	0x1a935
 668:	07540100 	0x7540100
 66c:	0100326d 	0x100326d
 670:	0001a935 	0x1a935
 674:	07550100 	0x7550100
 678:	3501006e 	ori	at,t0,0x6e
 67c:	0000013d 	0x13d
 680:	73085301 	0x73085301
 684:	3b010031 	xori	at,t8,0x31
 688:	000000f2 	0xf2
 68c:	73085401 	0x73085401
 690:	3c010032 	lui	at,0x32
 694:	000000f2 	0xf2
 698:	04005501 	bltz	zero,15aa0 <_boot-0x801ea560>
 69c:	0001af04 	0x1af04
 6a0:	20000900 	addi	zero,zero,2304
 6a4:	02000002 	0x2000002
 6a8:	00025600 	sll	t2,v0,0x18
 6ac:	31010400 	andi	at,t0,0x400
 6b0:	90000006 	lbu	zero,6(zero)
 6b4:	ec802018 	swc3	$0,8216(a0)
 6b8:	47802017 	c1	0x1802017
 6bc:	4320554e 	c0	0x120554e
 6c0:	342e3320 	ori	t6,at,0x3320
 6c4:	312d362e 	andi	t5,t1,0x362e
 6c8:	362e332e 	ori	t6,s1,0x332e
 6cc:	00672d20 	0x672d20
 6d0:	2f2e2e01 	sltiu	t6,t9,11777
 6d4:	2e2f2e2e 	sltiu	t7,s1,11822
 6d8:	2e2e2f2e 	sltiu	t6,s1,12078
 6dc:	2f2e2e2f 	sltiu	t6,t9,11823
 6e0:	6e2f2e2e 	0x6e2f2e2e
 6e4:	696c7765 	0x696c7765
 6e8:	756f7362 	jalx	5bdcd88 <_boot-0x7a623278>0x7362
 6ec:	2f656372 	sltiu	a1,k1,25458
 6f0:	6c77656e 	0x6c77656e
 6f4:	6c2f6269 	0x6c2f6269
 6f8:	2f636269 	sltiu	v1,k1,25193
 6fc:	6863616d 	0x6863616d
 700:	2f656e69 	sltiu	a1,k1,28265
 704:	7370696d 	0x7370696d
 708:	6d656d2f 	0x6d656d2f
 70c:	2e746573 	sltiu	s4,s3,25971
 710:	682f0063 	0x682f0063
 714:	2f656d6f 	sltiu	a1,k1,28015
 718:	6e696870 	0x6e696870
 71c:	6e2f7865 	0x6e2f7865
 720:	696c7765 	0x696c7765
 724:	70696d62 	0x70696d62
 728:	696d2f73 	0x696d2f73
 72c:	652d7370 	0x652d7370
 730:	6e2f666c 	0x6e2f666c
 734:	696c7765 	0x696c7765
 738:	696c2f62 	0x696c2f62
 73c:	6d2f6362 	0x6d2f6362
 740:	69686361 	0x69686361
 744:	6d2f656e 	0x6d2f656e
 748:	00737069 	0x737069
 74c:	746e6902 	jalx	1b9a408 <_boot-0x7e665bf8>0x6902
 750:	02050400 	0x2050400
 754:	676e6f6c 	0x676e6f6c
 758:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 75c:	02050400 	0x2050400
 760:	676e6f6c 	0x676e6f6c
 764:	6e6f6c20 	0x6e6f6c20
 768:	6e692067 	0x6e692067
 76c:	05080074 	0x5080074
 770:	00026303 	sra	t4,v0,0xc
 774:	03070400 	0x3070400
 778:	00000263 	0x263
 77c:	75020704 	jalx	4081c10 <_boot-0x7c17e3f0>0x704
 780:	6769736e 	0x6769736e
 784:	2064656e 	addi	a0,v1,25966
 788:	72616863 	0x72616863
 78c:	04080100 	0x4080100
 790:	6f6c0204 	0x6f6c0204
 794:	7520676e 	jalx	4819db8 <_boot-0x7b9e6248>0x676e
 798:	6769736e 	0x6769736e
 79c:	2064656e 	addi	a0,v1,25966
 7a0:	00746e69 	0x746e69
 7a4:	73020704 	0x73020704
 7a8:	74726f68 	jalx	1c9bda0 <_boot-0x7e564260>0x6f68
 7ac:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 7b0:	05050200 	0x5050200
 7b4:	00011604 	0x11604
 7b8:	68630200 	0x68630200
 7bc:	01007261 	0x1007261
 7c0:	68730206 	0x68730206
 7c4:	2074726f 	addi	s4,v1,29295
 7c8:	69736e75 	0x69736e75
 7cc:	64656e67 	0x64656e67
 7d0:	746e6920 	jalx	1b9a480 <_boot-0x7e665b80>0x6920
 7d4:	02070200 	0x2070200
 7d8:	676e6f6c 	0x676e6f6c
 7dc:	6e6f6c20 	0x6e6f6c20
 7e0:	6e752067 	0x6e752067
 7e4:	6e676973 	0x6e676973
 7e8:	69206465 	0x69206465
 7ec:	0800746e 	j	1d1b8 <_boot-0x801e2e48>
 7f0:	69730607 	0x69730607
 7f4:	745f657a 	jalx	17d95e8 <_boot-0x7ea26a18>0x657a
 7f8:	cdd50200 	lwc3	$21,512(t6)
 7fc:	07000000 	bltz	t8,800 <_boot-0x801ff800>
 800:	00000217 	0x217
 804:	6d656d01 	0x6d656d01
 808:	00746573 	0x746573
 80c:	ec013501 	swc3	$1,13569(zero)
 810:	ec000000 	swc3	$0,0(zero)
 814:	90802017 	lbu	zero,8215(a0)
 818:	f4802018 	0xf4802018
 81c:	01000002 	0x1000002
 820:	006d086d 	0x6d086d
 824:	00ec3101 	0xec3101
 828:	54010000 	0x54010000
 82c:	01006308 	0x1006308
 830:	0000a931 	0xa931
 834:	08550100 	j	1540400 <_boot-0x7ecbfc00>
 838:	3101006e 	andi	at,t0,0x6e
 83c:	0000014e 	0x14e
 840:	73095601 	0x73095601
 844:	10400100 	beqz	v0,c48 <_boot-0x801ff3b8>
 848:	01000001 	0x1000001
 84c:	00690a57 	0x690a57
 850:	00a94101 	0xa94101
 854:	62090000 	0x62090000
 858:	65666675 	0x65666675
 85c:	42010072 	c0	0x10072
 860:	000000ee 	0xee
 864:	61095301 	0x61095301
 868:	6e67696c 	0x6e67696c
 86c:	615f6465 	0x615f6465
 870:	00726464 	0x726464
 874:	02174301 	0x2174301
 878:	57010000 	0x57010000
 87c:	6f68730a 	0x6f68730a
 880:	615f7472 	0x615f7472
 884:	00726464 	0x726464
 888:	021d4401 	0x21d4401
 88c:	69090000 	0x69090000
 890:	00726574 	0x726574
 894:	014e4501 	0x14e4501
 898:	52010000 	0x52010000
 89c:	2017f80b 	addi	s7,zero,-2037
 8a0:	20187480 	addi	t8,zero,29824
 8a4:	6e750980 	0x6e750980
 8a8:	67696c61 	0x67696c61
 8ac:	0064656e 	0x64656e
 8b0:	00a94901 	0xa94901
 8b4:	53010000 	0x53010000
 8b8:	04050000 	0x4050000
 8bc:	000000ee 	0xee
 8c0:	011e0405 	0x11e0405
 8c4:	Address 0x00000000000008c4 is out of bounds.


Disassembly of section .debug_abbrev:

00000000 <.debug_abbrev>:
   0:	25011101 	addiu	at,t0,4353
   4:	030b130e 	0x30b130e
   8:	110e1b0e 	beq	t0,t6,6c44 <_boot-0x801f93bc>
   c:	10061201 	beq	zero,a2,4814 <_boot-0x801fb7ec>
  10:	02000017 	0x2000017
  14:	0b0b0024 	j	c2c0090 <_boot-0x73f3ff70>
  18:	0e030b3e 	jal	80c2cf8 <_boot-0x7813d308>
  1c:	16030000 	bne	s0,v1,20 <_boot-0x801fffe0>
  20:	3a0e0300 	xori	t6,s0,0x300
  24:	490b3b0b 	0x490b3b0b
  28:	04000013 	bltz	zero,78 <_boot-0x801fff88>
  2c:	0b0b0024 	j	c2c0090 <_boot-0x73f3ff70>
  30:	08030b3e 	j	c2cf8 <_boot-0x8013d308>
  34:	2e050000 	sltiu	a1,s0,0
  38:	03193f00 	0x3193f00
  3c:	3b0b3a0e 	xori	t3,t8,0x3a0e
  40:	1113490b 	beq	t0,s3,12470 <_boot-0x801edb90>
  44:	40061201 	0x40061201
  48:	19429718 	0x19429718
  4c:	2e060000 	sltiu	a2,s0,0
  50:	03193f00 	0x3193f00
  54:	3b0b3a0e 	xori	t3,t8,0x3a0e
  58:	1201110b 	beq	s0,at,4488 <_boot-0x801fbb78>
  5c:	97184006 	lhu	t8,16390(t8)
  60:	00001942 	srl	v1,zero,0x5
  64:	3f012e07 	0x3f012e07
  68:	3a0e0319 	xori	t6,s0,0x319
  6c:	270b3b0b 	addiu	t3,t8,15115
  70:	11134919 	beq	t0,s3,124d8 <_boot-0x801edb28>
  74:	40061201 	0x40061201
  78:	19429618 	0x19429618
  7c:	00001301 	0x1301
  80:	03003408 	0x3003408
  84:	3b0b3a08 	xori	t3,t8,0x3a08
  88:	0213490b 	0x213490b
  8c:	09000018 	j	4000060 <_boot-0x7c1fffa0>
  90:	0e030013 	jal	80c004c <_boot-0x7813ffb4>
  94:	0000193c 	0x193c
  98:	0300340a 	0x300340a
  9c:	3b0b3a0e 	xori	t3,t8,0x3a0e
  a0:	0213490b 	0x213490b
  a4:	0b000018 	j	c000060 <_boot-0x741fffa0>
  a8:	193f012e 	0x193f012e
  ac:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
  b0:	13490b3b 	beq	k0,t1,2da0 <_boot-0x801fd260>
  b4:	1301193c 	beq	t8,at,65a8 <_boot-0x801f9a58>
  b8:	180c0000 	0x180c0000
  bc:	0d000000 	jal	4000000 <_boot-0x7c200000>
  c0:	193f012e 	0x193f012e
  c4:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
  c8:	19270b3b 	0x19270b3b
  cc:	193c1349 	0x193c1349
  d0:	00001301 	0x1301
  d4:	4900050e 	bc2f	1510 <_boot-0x801feaf0>
  d8:	0f000013 	jal	c00004c <_boot-0x741fffb4>
  dc:	193f012e 	0x193f012e
  e0:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
  e4:	13490b3b 	beq	k0,t1,2dd4 <_boot-0x801fd22c>
  e8:	0000193c 	0x193c
  ec:	0b000f10 	j	c003c40 <_boot-0x741fc3c0>
  f0:	0013490b 	0x13490b
  f4:	000f1100 	sll	v0,t7,0x4
  f8:	00000b0b 	0xb0b
  fc:	01110100 	0x1110100
 100:	0b130e25 	j	c4c3894 <_boot-0x73d3c76c>
 104:	0e1b0e03 	jal	86c380c <_boot-0x77b3c7f4>
 108:	06120111 	0x6120111
 10c:	00001710 	0x1710
 110:	03001602 	0x3001602
 114:	3b0b3a0e 	xori	t3,t8,0x3a0e
 118:	0013490b 	0x13490b
 11c:	00240300 	0x240300
 120:	0b3e0b0b 	j	cf82c2c <_boot-0x7327d3d4>
 124:	00000e03 	sra	at,zero,0x18
 128:	0b002404 	j	c009010 <_boot-0x741f6ff0>
 12c:	030b3e0b 	0x30b3e0b
 130:	05000008 	bltz	t0,154 <_boot-0x801ffeac>
 134:	193f012e 	0x193f012e
 138:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
 13c:	19270b3b 	0x19270b3b
 140:	06120111 	0x6120111
 144:	42971840 	c0	0x971840
 148:	00130119 	0x130119
 14c:	00050600 	sll	zero,a1,0x18
 150:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
 154:	13490b3b 	beq	k0,t1,2e44 <_boot-0x801fd1bc>
 158:	00001802 	srl	v1,zero,0x0
 15c:	03003407 	0x3003407
 160:	3b0b3a0e 	xori	t3,t8,0x3a0e
 164:	0213490b 	0x213490b
 168:	08000018 	j	60 <_boot-0x801fffa0>
 16c:	193f012e 	0x193f012e
 170:	0b3a0e03 	j	ce8380c <_boot-0x7337c7f4>
 174:	19270b3b 	0x19270b3b
 178:	01111349 	0x1111349
 17c:	18400612 	blez	v0,19c8 <_boot-0x801fe638>
 180:	01194297 	0x1194297
 184:	09000013 	j	400004c <_boot-0x7c1fffb4>
 188:	08030005 	j	c0014 <_boot-0x8013ffec>
 18c:	0b3b0b3a 	j	cec2ce8 <_boot-0x7333d318>
 190:	18021349 	0x18021349
 194:	2e0a0000 	sltiu	t2,s0,0
 198:	03193f01 	0x3193f01
 19c:	3b0b3a0e 	xori	t3,t8,0x3a0e
 1a0:	1119270b 	beq	t0,t9,9dd0 <_boot-0x801f6230>
 1a4:	40061201 	0x40061201
 1a8:	19429718 	0x19429718
 1ac:	01000000 	0x1000000
 1b0:	06100011 	bltzal	s0,1f8 <_boot-0x801ffe08>
 1b4:	01120111 	0x1120111
 1b8:	081b0803 	j	6c200c <_boot-0x7fb3dff4>
 1bc:	05130825 	0x5130825
 1c0:	01000000 	0x1000000
 1c4:	06100011 	bltzal	s0,20c <_boot-0x801ffdf4>
 1c8:	01120111 	0x1120111
 1cc:	081b0803 	j	6c200c <_boot-0x7fb3dff4>
 1d0:	05130825 	0x5130825
 1d4:	01000000 	0x1000000
 1d8:	06100111 	bltzal	s0,620 <_boot-0x801ff9e0>
 1dc:	01110112 	0x1110112
 1e0:	0b130825 	j	c4c2094 <_boot-0x73d3df6c>
 1e4:	081b0803 	j	6c200c <_boot-0x7fb3dff4>
 1e8:	24020000 	li	v0,0
 1ec:	0b080300 	j	c200c00 <_boot-0x73fff400>
 1f0:	000b3e0b 	0xb3e0b
 1f4:	00240300 	0x240300
 1f8:	0b0b0e03 	j	c2c380c <_boot-0x73f3c7f4>
 1fc:	00000b3e 	0xb3e
 200:	0b000f04 	j	c003c10 <_boot-0x741fc3f0>
 204:	0013490b 	0x13490b
 208:	00160500 	sll	zero,s6,0x14
 20c:	0b3a0803 	j	ce8200c <_boot-0x7337dff4>
 210:	13490b3b 	beq	k0,t1,2f00 <_boot-0x801fd100>
 214:	2e060000 	sltiu	a2,s0,0
 218:	3f130101 	0x3f130101
 21c:	3a08030c 	xori	t0,s0,0x30c
 220:	270b3b0b 	addiu	t3,t8,15115
 224:	1113490c 	beq	t0,s3,12658 <_boot-0x801ed9a8>
 228:	81011201 	lb	at,4609(t0)
 22c:	0a400640 	j	9001900 <_boot-0x771fe700>
 230:	05070000 	0x5070000
 234:	3a080300 	xori	t0,s0,0x300
 238:	490b3b0b 	0x490b3b0b
 23c:	000a0213 	0xa0213
 240:	00340800 	0x340800
 244:	0b3a0803 	j	ce8200c <_boot-0x7337dff4>
 248:	13490b3b 	beq	k0,t1,2f38 <_boot-0x801fd0c8>
 24c:	00000a02 	srl	at,zero,0x8
 250:	00002609 	0x2609
 254:	11010000 	beq	t0,at,258 <_boot-0x801ffda8>
 258:	12061001 	beq	s0,a2,4260 <_boot-0x801fbda0>
 25c:	25011101 	addiu	at,t0,4353
 260:	030b1308 	0x30b1308
 264:	00081b08 	0x81b08
 268:	00240200 	0x240200
 26c:	0b0b0803 	j	c2c200c <_boot-0x73f3dff4>
 270:	00000b3e 	0xb3e
 274:	03002403 	0x3002403
 278:	3e0b0b0e 	0x3e0b0b0e
 27c:	0400000b 	bltz	zero,2ac <_boot-0x801ffd54>
 280:	0b0b000f 	j	c2c003c <_boot-0x73f3ffc4>
 284:	0f050000 	jal	c140000 <_boot-0x740c0000>
 288:	490b0b00 	0x490b0b00
 28c:	06000013 	bltz	s0,2dc <_boot-0x801ffd24>
 290:	08030016 	j	c0058 <_boot-0x8013ffa8>
 294:	0b3b0b3a 	j	cec2ce8 <_boot-0x7333d318>
 298:	00001349 	0x1349
 29c:	01012e07 	0x1012e07
 2a0:	030c3f13 	0x30c3f13
 2a4:	3b0b3a08 	xori	t3,t8,0x3a08
 2a8:	490c270b 	0x490c270b
 2ac:	12011113 	beq	s0,at,46fc <_boot-0x801fb904>
 2b0:	06408101 	bltz	s2,fffe06b8 <__bss_end+0x7fdde9f8>
 2b4:	00000a40 	sll	at,zero,0x9
 2b8:	03000508 	0x3000508
 2bc:	3b0b3a08 	xori	t3,t8,0x3a08
 2c0:	0213490b 	0x213490b
 2c4:	0900000a 	j	4000028 <_boot-0x7c1fffd8>
 2c8:	08030034 	j	c00d0 <_boot-0x8013ff30>
 2cc:	0b3b0b3a 	j	cec2ce8 <_boot-0x7333d318>
 2d0:	0a021349 	j	8084d24 <_boot-0x7817b2dc>
 2d4:	340a0000 	li	t2,0x0
 2d8:	3a080300 	xori	t0,s0,0x300
 2dc:	490b3b0b 	0x490b3b0b
 2e0:	0b000013 	j	c00004c <_boot-0x741fffb4>
 2e4:	0111010b 	0x111010b
 2e8:	00000112 	0x112
	...

Disassembly of section .debug_aranges:

00000000 <.debug_aranges>:
   0:	0000001c 	0x1c
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80200078 	lb	zero,120(at)
  14:	00000684 	0x684
	...
  20:	0000001c 	0x1c
  24:	02170002 	0x2170002
  28:	00040000 	sll	zero,a0,0x0
  2c:	00000000 	nop
  30:	802006fc 	lb	zero,1788(at)
  34:	000003c4 	0x3c4
	...
  40:	0000001c 	0x1c
  44:	04530002 	0x4530002
  48:	00040000 	sll	zero,a0,0x0
  4c:	00000000 	nop
  50:	80200000 	lb	zero,0(at)
  54:	00000078 	0x78
	...
  60:	0000001c 	0x1c
  64:	04a30002 	0x4a30002
  68:	00040000 	sll	zero,a0,0x0
  6c:	00000000 	nop
  70:	80200ac0 	lb	zero,2752(at)
  74:	00000cf0 	0xcf0
	...
  80:	0000001c 	0x1c
  84:	04f20002 	0x4f20002
  88:	00040000 	sll	zero,a0,0x0
  8c:	00000000 	nop
  90:	802017b0 	lb	zero,6064(at)
  94:	0000003c 	0x3c
	...
  a0:	0000001c 	0x1c
  a4:	06a30002 	0x6a30002
  a8:	00040000 	sll	zero,a0,0x0
  ac:	00000000 	nop
  b0:	802017ec 	lb	zero,6124(at)
  b4:	000000a4 	0xa4
	...

Disassembly of section .debug_line:

00000000 <.debug_line>:
   0:	00000101 	0x101
   4:	00330002 	0x330002
   8:	01010000 	0x1010000
   c:	000d0efb 	0xd0efb
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	2e707061 	sltiu	s0,s3,28769
  20:	00000063 	0x63
  24:	70736200 	0x70736200
  28:	0000682e 	0x682e
  2c:	623c0000 	0x623c0000
  30:	746c6975 	jalx	1b1a5d4 <_boot-0x7e6e5a2c>0x6975
  34:	3e6e692d 	0x3e6e692d
  38:	00000000 	nop
  3c:	02050000 	0x2050000
  40:	80200078 	lb	zero,120(at)
  44:	bb010e03 	swr	at,3587(t8)
  48:	bbbbbbbb 	swr	k1,-17477(sp)
  4c:	ba3e08bb 	swr	s8,2235(s1)
  50:	08ba3d08 	j	2e8f420 <_boot-0x7d370be0>
  54:	3d08ba3d 	0x3d08ba3d
  58:	ba3d08ba 	swr	sp,2234(s1)
  5c:	08ba3d08 	j	2e8f420 <_boot-0x7d370be0>
  60:	3d08ba3d 	0x3d08ba3d
  64:	ba3d08ba 	swr	sp,2234(s1)
  68:	08ba3d08 	j	2e8f420 <_boot-0x7d370be0>
  6c:	834bf33f 	lb	t3,-3265(k0)
  70:	0883e508 	j	20f9420 <_boot-0x7e106be0>
  74:	f3bf833e 	0xf3bf833e
  78:	7608f3f3 	jalx	823cfcc <_boot-0x77fc3034>0xf3f3
  7c:	04020084 	0x4020084
  80:	0200bb03 	0x200bb03
  84:	38020304 	xori	v0,zero,0x304
  88:	04020011 	0x4020011
  8c:	06f20601 	0x6f20601
  90:	ad083f08 	sw	t0,16136(t0)
  94:	f3f3b208 	0xf3f3b208
  98:	f37508f3 	0xf37508f3
  9c:	0084f5f3 	0x84f5f3
  a0:	bb030402 	swr	v1,1026(t8)
  a4:	03040200 	0x3040200
  a8:	00113c02 	srl	a3,s1,0x10
  ac:	06010402 	bgez	s0,10b8 <_boot-0x801fef48>
  b0:	3f0806f2 	0x3f0806f2
  b4:	b208ad08 	0xb208ad08
  b8:	08f3f3f3 	j	3cfcfcc <_boot-0x7c503034>
  bc:	f4f3f375 	0xf4f3f375
  c0:	04020084 	0x4020084
  c4:	0200bb03 	0x200bb03
  c8:	3c020304 	lui	v0,0x304
  cc:	04020011 	0x4020011
  d0:	06f20601 	0x6f20601
  d4:	ad083f08 	sw	t0,16136(t0)
  d8:	f3f3b208 	0xf3f3b208
  dc:	f37508f3 	0xf37508f3
  e0:	0084f4f3 	0x84f4f3
  e4:	bb030402 	swr	v1,1026(t8)
  e8:	03040200 	0x3040200
  ec:	00113c02 	srl	a3,s1,0x10
  f0:	06010402 	bgez	s0,10fc <_boot-0x801fef04>
  f4:	3f0806f2 	0x3f0806f2
  f8:	dd03ad08 	0xdd03ad08
  fc:	83ac0800 	lb	t4,2048(sp)
 100:	01001802 	0x1001802
 104:	0000ce01 	0xce01
 108:	2d000200 	sltiu	zero,t0,512
 10c:	01000000 	0x1000000
 110:	0d0efb01 	jal	43bec04 <_boot-0x7be413fc>
 114:	01010100 	0x1010100
 118:	00000001 	0x1
 11c:	01000001 	0x1000001
 120:	786c7200 	0x786c7200
 124:	6361635f 	0x6361635f
 128:	632e6568 	0x632e6568
 12c:	00000000 	nop
 130:	73622f2e 	0x73622f2e
 134:	00682e70 	0x682e70
 138:	00000000 	nop
 13c:	fc020500 	0xfc020500
 140:	03802006 	srlv	a0,zero,gp
 144:	3f080134 	0x3f080134
 148:	03040200 	0x3040200
 14c:	02003e08 	0x2003e08
 150:	00b80304 	0xb80304
 154:	06010402 	bgez	s0,1160 <_boot-0x801feea0>
 158:	240206f2 	li	v0,1778
 15c:	08100318 	j	400c60 <_boot-0x7fdff3a0>
 160:	003f083c 	0x3f083c
 164:	08030402 	j	c1008 <_boot-0x8013eff8>
 168:	0402003e 	0x402003e
 16c:	0200b803 	0x200b803
 170:	f2060104 	0xf2060104
 174:	18240206 	0x18240206
 178:	3c081103 	lui	t0,0x1103
 17c:	02003f08 	0x2003f08
 180:	3e080304 	0x3e080304
 184:	03040200 	0x3040200
 188:	040200b8 	0x40200b8
 18c:	06f20601 	0x6f20601
 190:	03182402 	0x3182402
 194:	bb3c0810 	swr	gp,2064(t9)
 198:	f383bb4c 	0xf383bb4c
 19c:	081003bc 	j	400ef0 <_boot-0x7fdff110>
 1a0:	bb4cbb3c 	swr	t4,-17604(k0)
 1a4:	03bbf383 	0x3bbf383
 1a8:	f43c0812 	0xf43c0812
 1ac:	08f4844c 	j	3d21130 <_boot-0x7c4deed0>
 1b0:	4e8583e9 	c3	0x8583e9
 1b4:	bdad08f4 	cache	0xd,2292(t5)
 1b8:	0811034b 	j	440d2c <_boot-0x7fdbf2d4>
 1bc:	bb3e083c 	swr	s8,2108(t9)
 1c0:	bbf383bc 	swr	s3,-31812(ra)
 1c4:	3c081103 	lui	t0,0x1103
 1c8:	bcbb7608 	cache	0x1b,30216(a1)
 1cc:	bcf383f4 	cache	0x13,-31756(a3)
 1d0:	1402bcf3 	bne	zero,v0,fffef5a0 <__bss_end+0x7fded8e0>
 1d4:	48010100 	0x48010100
 1d8:	02000000 	0x2000000
 1dc:	00001d00 	sll	v1,zero,0x14
 1e0:	fb010100 	0xfb010100
 1e4:	01000d0e 	0x1000d0e
 1e8:	00010101 	0x10101
 1ec:	00010000 	sll	zero,at,0x0
 1f0:	63000100 	0x63000100
 1f4:	2e307472 	sltiu	s0,s1,29810
 1f8:	00000053 	0x53
 1fc:	05000000 	bltz	t0,200 <_boot-0x801ffe00>
 200:	20000002 	addi	zero,zero,2
 204:	01280380 	0x1280380
 208:	83bb4b4b 	lb	k1,19275(sp)
 20c:	4b504b4b 	c2	0x1504b4b
 210:	4b4c4b83 	c2	0x14c4b83
 214:	514b4b4b 	0x514b4b4b
 218:	4b838489 	c2	0x1838489
 21c:	04024b50 	0x4024b50
 220:	39010100 	xori	at,t0,0x100
 224:	02000003 	0x2000003
 228:	00001c00 	sll	v1,zero,0x10
 22c:	fb010100 	0xfb010100
 230:	01000d0e 	0x1000d0e
 234:	00010101 	0x10101
 238:	00010000 	sll	zero,at,0x0
 23c:	73000100 	0x73000100
 240:	532e6970 	0x532e6970
 244:	00000000 	nop
 248:	02050000 	0x2050000
 24c:	80200ac0 	lb	zero,2752(at)
 250:	4b4d4b1a 	c2	0x14d4b1a
 254:	4c834b4b 	0x4c834b4b
 258:	0b034b4b 	j	c0d2d2c <_boot-0x7412d2d4>
 25c:	4b4c4b82 	c2	0x14c4b82
 260:	4b4b4b4b 	c2	0x14b4b4b
 264:	4b4c4b4b 	c2	0x14c4b4b
 268:	834b834c 	lb	t3,-31924(k0)
 26c:	4c4b4b4c 	0x4c4b4b4c
 270:	0a034b4b 	j	80d2d2c <_boot-0x7812d2d4>
 274:	4b4d4b82 	c2	0x14d4b82
 278:	4b4b4b4b 	c2	0x14b4b4b
 27c:	4c4b4c4b 	0x4c4b4c4b
 280:	4c4b4c83 	0x4c4b4c83
 284:	4c834b83 	0x4c834b83
 288:	0a034b4b 	j	80d2d2c <_boot-0x7812d2d4>
 28c:	4b4d4b82 	c2	0x14d4b82
 290:	4b4b4b4b 	c2	0x14b4b4b
 294:	4c4b4c4b 	0x4c4b4c4b
 298:	4b4d4c83 	c2	0x14d4c83
 29c:	4b4b4b4b 	c2	0x14b4b4b
 2a0:	4b4b4c4d 	c2	0x14b4c4d
 2a4:	834c4b4c 	lb	t4,19276(k0)
 2a8:	4b4c834b 	c2	0x14c834b
 2ac:	820b034b 	lb	t3,843(s0)
 2b0:	4b4b4d4b 	c2	0x14b4d4b
 2b4:	4b4b4b4b 	c2	0x14b4b4b
 2b8:	834c4b4c 	lb	t4,19276(k0)
 2bc:	4b4b4d4c 	c2	0x14b4d4c
 2c0:	4d4b4b4b 	0x4d4b4b4b
 2c4:	4c4b4b4c 	0x4c4b4b4c
 2c8:	4b834c4b 	c2	0x1834c4b
 2cc:	4b4b4c83 	c2	0x14b4c83
 2d0:	4b820a03 	c2	0x1820a03
 2d4:	4b4b4b4d 	c2	0x14b4b4d
 2d8:	4c4b4b4b 	0x4c4b4b4b
 2dc:	4c834c4b 	0x4c834c4b
 2e0:	4b4b4b4d 	c2	0x14b4b4d
 2e4:	4c4d4b4b 	0x4c4d4b4b
 2e8:	4b4c4b4b 	c2	0x14c4b4b
 2ec:	834b834c 	lb	t3,-31924(k0)
 2f0:	034b4b4c 	syscall	0xd2d2d
 2f4:	4d4b820a 	0x4d4b820a
 2f8:	4b4b4b4b 	c2	0x14b4b4b
 2fc:	4b4c4b4b 	c2	0x14c4b4b
 300:	4d4c834c 	0x4d4c834c
 304:	4b4b4b4b 	c2	0x14b4b4b
 308:	4b4c4d4b 	c2	0x14c4d4b
 30c:	4c4b4c4b 	0x4c4b4c4b
 310:	4c834b83 	0x4c834b83
 314:	0a034b4b 	j	80d2d2c <_boot-0x7812d2d4>
 318:	4b4d4b82 	c2	0x14d4b82
 31c:	4b4b4b4b 	c2	0x14b4b4b
 320:	4c4b4c4b 	0x4c4b4c4b
 324:	4b4d4c83 	c2	0x14d4c83
 328:	4b4b4b4b 	c2	0x14b4b4b
 32c:	4b4b4c4d 	c2	0x14b4c4d
 330:	834c4b4c 	lb	t4,19276(k0)
 334:	4b4c834b 	c2	0x14c834b
 338:	820a034b 	lb	t2,843(s0)
 33c:	4b4b4c4b 	c2	0x14b4c4b
 340:	4b4b4b4b 	c2	0x14b4b4b
 344:	4c4b4d4b 	0x4c4b4d4b
 348:	4c4b4c83 	0x4c4b4c83
 34c:	4b4c834b 	c2	0x14c834b
 350:	820b034b 	lb	t3,843(s0)
 354:	4b4b4d4b 	c2	0x14b4d4b
 358:	4b4b4b4b 	c2	0x14b4b4b
 35c:	834c4b4c 	lb	t4,19276(k0)
 360:	4b4b4c4c 	c2	0x14b4c4c
 364:	834c4b4c 	lb	t4,19276(k0)
 368:	4b4c834b 	c2	0x14c834b
 36c:	820a034b 	lb	t2,843(s0)
 370:	4b4b4d4b 	c2	0x14b4d4b
 374:	4b4b4b4b 	c2	0x14b4b4b
 378:	834c4b4c 	lb	t4,19276(k0)
 37c:	4b4c834b 	c2	0x14c834b
 380:	820a034b 	lb	t2,843(s0)
 384:	4b4b4d4b 	c2	0x14b4d4b
 388:	4b4b4b4b 	c2	0x14b4b4b
 38c:	834c4b4c 	lb	t4,19276(k0)
 390:	4b4c834b 	c2	0x14c834b
 394:	820b034b 	lb	t3,843(s0)
 398:	4c834c4b 	0x4c834c4b
 39c:	4b4b4b4d 	c2	0x14b4b4d
 3a0:	4c4b4c4b 	0x4c4b4c4b
 3a4:	4b4c4c83 	c2	0x14c4c83
 3a8:	4b4b4b4b 	c2	0x14b4b4b
 3ac:	834c4c4b 	lb	t4,19531(k0)
 3b0:	4b4c834b 	c2	0x14c834b
 3b4:	820b034b 	lb	t3,843(s0)
 3b8:	4b4b4d4b 	c2	0x14b4d4b
 3bc:	4b4b4b4b 	c2	0x14b4b4b
 3c0:	4b4c4b4b 	c2	0x14c4b4b
 3c4:	4b4c834c 	c2	0x14c834c
 3c8:	4b4c4b4c 	c2	0x14c4b4c
 3cc:	4c4b4b4c 	0x4c4b4b4c
 3d0:	4c4b4c4b 	0x4c4b4c4b
 3d4:	0b034b4b 	j	c0d2d2c <_boot-0x7412d2d4>
 3d8:	4b4d4b82 	c2	0x14d4b82
 3dc:	4b4b4b4b 	c2	0x14b4b4b
 3e0:	4c834c4b 	0x4c834c4b
 3e4:	4b4b4d4b 	c2	0x14b4d4b
 3e8:	4b4c4b4b 	c2	0x14c4b4b
 3ec:	4b4d834c 	c2	0x14d834c
 3f0:	4b4b4c4b 	c2	0x14b4c4b
 3f4:	4b820d03 	c2	0x1820d03
 3f8:	4b4b4b4d 	c2	0x14b4b4d
 3fc:	4c4b4b4b 	0x4c4b4b4b
 400:	4b4d4c83 	c2	0x14d4c83
 404:	4c4b4b4b 	0x4c4b4b4b
 408:	4d834c4b 	0x4d834c4b
 40c:	4c4b4b4b 	0x4c4b4b4b
 410:	4c4b4b4b 	0x4c4b4b4b
 414:	0a034b4b 	j	80d2d2c <_boot-0x7812d2d4>
 418:	4b4d4b82 	c2	0x14d4b82
 41c:	4b4b4b4b 	c2	0x14b4b4b
 420:	4c4b4c4b 	0x4c4b4c4b
 424:	4b4b4c83 	c2	0x14b4c83
 428:	4b820a03 	c2	0x1820a03
 42c:	4b4b4b4d 	c2	0x14b4b4d
 430:	4c4b4b4b 	0x4c4b4b4b
 434:	4c834c4b 	0x4c834c4b
 438:	0c034b4b 	jal	d2d2c <_boot-0x8012d2d4>
 43c:	4b4d4b82 	c2	0x14d4b82
 440:	4b4b4b4b 	c2	0x14b4b4b
 444:	4c4b4d4b 	0x4c4b4d4b
 448:	4b4d4c83 	c2	0x14d4c83
 44c:	4b4b4b4b 	c2	0x14b4b4b
 450:	4b4c4d4c 	c2	0x14c4d4c
 454:	4b4c4c4b 	c2	0x14c4c4b
 458:	834b834c 	lb	t3,-31924(k0)
 45c:	034b4b4c 	syscall	0xd2d2d
 460:	4d4b820c 	0x4d4b820c
 464:	4b4b4b4b 	c2	0x14b4b4b
 468:	834c4b4b 	lb	t4,19275(k0)
 46c:	4b4b4d4d 	c2	0x14b4d4d
 470:	4d4c4b4b 	0x4d4c4b4b
 474:	4c4b4b4b 	0x4c4b4b4b
 478:	4d834c4b 	0x4d834c4b
 47c:	4b4b4c84 	c2	0x14b4c84
 480:	4b820c03 	c2	0x1820c03
 484:	4b4b4b4d 	c2	0x14b4b4d
 488:	4c4b4b4b 	0x4c4b4b4b
 48c:	4b4b4b4d 	c2	0x14b4b4d
 490:	4b4b4d4b 	c2	0x14b4d4b
 494:	4c4b4b4b 	0x4c4b4b4b
 498:	51834c4b 	0x51834c4b
 49c:	4c4c834b 	0x4c4c834b
 4a0:	0c034b4b 	jal	d2d2c <_boot-0x8012d2d4>
 4a4:	4b4d4b82 	c2	0x14d4b82
 4a8:	4b4b4b4b 	c2	0x14b4b4b
 4ac:	4c4b4c4b 	0x4c4b4c4b
 4b0:	4b4d4c83 	c2	0x14d4c83
 4b4:	4b4b4b4b 	c2	0x14b4b4b
 4b8:	4b4b4c4d 	c2	0x14b4c4d
 4bc:	4b4b4c4d 	c2	0x14b4c4d
 4c0:	4c4b844b 	0x4c4b844b
 4c4:	4c834b83 	0x4c834b83
 4c8:	0d034b4b 	jal	40d2d2c <_boot-0x7c12d2d4>
 4cc:	4b4d4b82 	c2	0x14d4b82
 4d0:	4b4b4b4b 	c2	0x14b4b4b
 4d4:	4c4b4e4b 	0x4c4b4e4b
 4d8:	4b4c4c83 	c2	0x14c4c83
 4dc:	4d4b4b4b 	0x4d4b4b4b
 4e0:	4b4b4c4b 	c2	0x14b4c4b
 4e4:	4b4c4b4d 	c2	0x14c4b4d
 4e8:	4b4b4c4b 	c2	0x14b4c4b
 4ec:	4c4b4b4c 	0x4c4b4b4c
 4f0:	4c834c4b 	0x4c834c4b
 4f4:	4c4b4c83 	0x4c4b4c83
 4f8:	4b820c03 	c2	0x1820c03
 4fc:	4b4b4d4b 	c2	0x14b4d4b
 500:	4b4b4b4b 	c2	0x14b4b4b
 504:	4b4b4c4d 	c2	0x14b4c4d
 508:	4b4d4b4b 	c2	0x14d4b4b
 50c:	4c4b4b4b 	0x4c4b4b4b
 510:	4d834c4b 	0x4d834c4b
 514:	834c834b 	lb	t4,-31925(k0)
 518:	4c834b4b 	0x4c834b4b
 51c:	834b4b83 	lb	t3,19331(k0)
 520:	4b4b834c 	c2	0x14b834c
 524:	4b4c4c83 	c2	0x14c4c83
 528:	0c034c4b 	jal	d312c <_boot-0x8012ced4>
 52c:	4b4d4b82 	c2	0x14d4b82
 530:	4b4b4b4b 	c2	0x14b4b4b
 534:	4b4e4c4b 	c2	0x14e4c4b
 538:	4d4b4b4b 	0x4d4b4b4b
 53c:	4b4b4b4b 	c2	0x14b4b4b
 540:	4c4b4c4b 	0x4c4b4c4b
 544:	834b4e83 	lb	t3,20099(k0)
 548:	4b4b834c 	c2	0x14b834c
 54c:	4b834c83 	c2	0x1834c83
 550:	834c834b 	lb	t4,-31925(k0)
 554:	4c834b4b 	0x4c834b4b
 558:	024b4b4c 	syscall	0x92d2d
 55c:	01010008 	0x1010008
 560:	000000cd 	break	0x0,0x3
 564:	00ae0002 	0xae0002
 568:	01010000 	0x1010000
 56c:	000d0efb 	0xd0efb
 570:	01010101 	0x1010101
 574:	01000000 	0x1000000
 578:	2e010000 	sltiu	at,s0,0
 57c:	2e2e2f2e 	sltiu	t6,s1,12078
 580:	2f2e2e2f 	sltiu	t6,t9,11823
 584:	2e2f2e2e 	sltiu	t7,s1,11822
 588:	656e2f2e 	0x656e2f2e
 58c:	62696c77 	0x62696c77
 590:	72756f73 	0x72756f73
 594:	6e2f6563 	0x6e2f6563
 598:	696c7765 	0x696c7765
 59c:	696c2f62 	0x696c2f62
 5a0:	732f6362 	0x732f6362
 5a4:	6e697274 	0x6e697274
 5a8:	752f0067 	jalx	4bc019c <_boot-0x7b63fe64>0x67
 5ac:	6c2f7273 	0x6c2f7273
 5b0:	6c61636f 	0x6c61636f
 5b4:	6473722f 	0x6473722f
 5b8:	2e312d6b 	sltiu	s1,s1,11627
 5bc:	2e362e33 	sltiu	s6,s1,11827
 5c0:	2f646c6f 	sltiu	a0,k1,27759
 5c4:	77677963 	jalx	d9de58c <_boot-0x72821a74>0x7963
 5c8:	6e2f6e69 	0x6e2f6e69
 5cc:	696c7765 	0x696c7765
 5d0:	69622f62 	0x69622f62
 5d4:	2e2e2f6e 	sltiu	t6,s1,12142
 5d8:	62696c2f 	0x62696c2f
 5dc:	6363672f 	0x6363672f
 5e0:	70696d2f 	0x70696d2f
 5e4:	6c652d73 	0x6c652d73
 5e8:	2e332f66 	sltiu	s3,s1,12134
 5ec:	2d362e34 	sltiu	s6,t1,11828
 5f0:	2e332e31 	sltiu	s3,s1,11825
 5f4:	6e692f36 	0x6e692f36
 5f8:	64756c63 	0x64756c63
 5fc:	6d000065 	0x6d000065
 600:	6d636d65 	0x6d636d65
 604:	00632e70 	0x632e70
 608:	73000001 	madu	t8,zero
 60c:	65646474 	0x65646474
 610:	00682e66 	0x682e66
 614:	00000002 	srl	zero,zero,0x0
 618:	b0020500 	0xb0020500
 61c:	03802017 	0x3802017
 620:	b50100c4 	0xb50100c4
 624:	84464987 	lh	a2,18823(v0)
 628:	822b0385 	lb	t3,901(s1)
 62c:	01000802 	0x1000802
 630:	0000e901 	0xe901
 634:	b7000200 	0xb7000200
 638:	01000000 	0x1000000
 63c:	0d0efb01 	jal	43bec04 <_boot-0x7be413fc>
 640:	01010100 	0x1010100
 644:	00000001 	0x1
 648:	01000001 	0x1000001
 64c:	2e2f2e2e 	sltiu	t7,s1,11822
 650:	2e2e2f2e 	sltiu	t6,s1,12078
 654:	2f2e2e2f 	sltiu	t6,t9,11823
 658:	2e2f2e2e 	sltiu	t7,s1,11822
 65c:	656e2f2e 	0x656e2f2e
 660:	62696c77 	0x62696c77
 664:	72756f73 	0x72756f73
 668:	6e2f6563 	0x6e2f6563
 66c:	696c7765 	0x696c7765
 670:	696c2f62 	0x696c2f62
 674:	6d2f6362 	0x6d2f6362
 678:	69686361 	0x69686361
 67c:	6d2f656e 	0x6d2f656e
 680:	00737069 	0x737069
 684:	7273752f 	0x7273752f
 688:	636f6c2f 	0x636f6c2f
 68c:	722f6c61 	0x722f6c61
 690:	2d6b6473 	sltiu	t3,t3,25715
 694:	2e332e31 	sltiu	s3,s1,11825
 698:	6c6f2e36 	0x6c6f2e36
 69c:	79632f64 	0x79632f64
 6a0:	6e697767 	0x6e697767
 6a4:	77656e2f 	jalx	d95b8bc <_boot-0x728a4744>0x6e2f
 6a8:	2f62696c 	sltiu	v0,k1,26988
 6ac:	2f6e6962 	sltiu	t6,k1,26978
 6b0:	6c2f2e2e 	0x6c2f2e2e
 6b4:	672f6269 	0x672f6269
 6b8:	6d2f6363 	0x6d2f6363
 6bc:	2d737069 	sltiu	s3,t3,28777
 6c0:	2f666c65 	sltiu	a2,k1,27749
 6c4:	2e342e33 	sltiu	s4,s1,11827
 6c8:	2e312d36 	sltiu	s1,s1,11574
 6cc:	2f362e33 	sltiu	s6,t9,11827
 6d0:	6c636e69 	0x6c636e69
 6d4:	00656475 	0x656475
 6d8:	6d656d00 	0x6d656d00
 6dc:	2e746573 	sltiu	s4,s3,25971
 6e0:	00010063 	0x10063
 6e4:	64747300 	0x64747300
 6e8:	2e666564 	sltiu	a2,s3,25956
 6ec:	00020068 	0x20068
 6f0:	05000000 	bltz	t0,6f4 <_boot-0x801ff90c>
 6f4:	2017ec02 	addi	s7,zero,-5118
 6f8:	00c60380 	0xc60380
 6fc:	4f844a01 	c3	0x1844a01
 700:	080b0385 	j	2c0e14 <_boot-0x7ff3f1ec>
 704:	83864cac 	lb	a2,19628(gp)
 708:	4b820f03 	c2	0x1820f03
 70c:	4b4b7f50 	c2	0x14b7f50
 710:	494bbc86 	0x494bbc86
 714:	814a0b03 	lb	t2,2819(t2)
 718:	000802c0 	sll	zero,t0,0xb
 71c:	Address 0x000000000000071c is out of bounds.


Disassembly of section .debug_str:

00000000 <.debug_str>:
   0:	5f697373 	0x5f697373
   4:	0070616d 	0x70616d
   8:	5f786c72 	0x5f786c72
   c:	63616369 	0x63616369
  10:	695f6568 	0x695f6568
  14:	6c61766e 	0x6c61766e
  18:	74616469 	jalx	18591a4 <_boot-0x7e9a6e5c>0x6469
  1c:	6c615f65 	0x6c615f65
  20:	7073006c 	0x7073006c
  24:	72775f69 	0x72775f69
  28:	00657469 	0x657469
  2c:	5f786c72 	0x5f786c72
  30:	6d656d64 	0x6d656d64
  34:	616e655f 	0x616e655f
  38:	00656c62 	0x656c62
  3c:	726f6873 	0x726f6873
  40:	6e692074 	0x6e692074
  44:	616d0074 	0x616d0074
  48:	49006e69 	bc2f	1b9f0 <_boot-0x801e4610>
  4c:	3233544e 	andi	s3,s1,0x544e
  50:	74730055 	jalx	1cc0154 <_boot-0x7e53feac>0x55
  54:	00656761 	0x656761
  58:	636d656d 	0x636d656d
  5c:	6d00706d 	0x6d00706d
  60:	65736d65 	0x65736d65
  64:	75640074 	jalx	59001d0 <_boot-0x7a8ffe30>0x74
  68:	00796d6d 	0x796d6d
  6c:	5f697373 	0x5f697373
  70:	74726f70 	jalx	1c9bdc0 <_boot-0x7e564240>0x6f70
  74:	0070616d 	0x70616d
  78:	38544e49 	xori	s4,v0,0x4e49
  7c:	70730055 	0x70730055
  80:	65725f69 	0x65725f69
  84:	695f6461 	0x695f6461
  88:	6e750064 	0x6e750064
  8c:	6e676973 	0x6e676973
  90:	63206465 	0x63206465
  94:	00726168 	0x726168
  98:	5f697073 	0x5f697073
  9c:	5f6b6c62 	0x5f6b6c62
  a0:	73617265 	0x73617265
  a4:	64720065 	0x64720065
  a8:	73006469 	0x73006469
  ac:	735f6970 	0x735f6970
  b0:	6e755f65 	0x6e755f65
  b4:	746f7270 	jalx	1bdc9c0 <_boot-0x7e623640>0x7270
  b8:	00746365 	0x746365
  bc:	726f6873 	0x726f6873
  c0:	6e752074 	0x6e752074
  c4:	6e676973 	0x6e676973
  c8:	69206465 	0x69206465
  cc:	6400746e 	0x6400746e
  d0:	796d6d75 	0x796d6d75
  d4:	69640033 	0x69640033
  d8:	6c626173 	0x6c626173
  dc:	616d5f65 	0x616d5f65
  e0:	72657473 	0x72657473
  e4:	676e655f 	0x676e655f
  e8:	00656e69 	0x656e69
  ec:	6d6d7564 	0x6d6d7564
  f0:	64003079 	0x64003079
  f4:	796d6d75 	0x796d6d75
  f8:	75640031 	jalx	59000c4 <_boot-0x7a8fff3c>0x31
  fc:	32796d6d 	andi	t9,s3,0x6d6d
 100:	70706100 	0x70706100
 104:	6400632e 	0x6400632e
 108:	796d6d75 	0x796d6d75
 10c:	75640034 	jalx	59000d0 <_boot-0x7a8fff30>0x34
 110:	35796d6d 	ori	t9,t3,0x6d6d
 114:	6d756400 	0x6d756400
 118:	0036796d 	0x36796d
 11c:	6d6d7564 	0x6d6d7564
 120:	64003779 	0x64003779
 124:	796d6d75 	0x796d6d75
 128:	632f0038 	0x632f0038
 12c:	72646779 	0x72646779
 130:	2f657669 	sltiu	a1,k1,30313
 134:	73722f63 	0x73722f63
 138:	382f6b64 	xori	t7,at,0x6b64
 13c:	46313131 	c1	0x313131
 140:	6d6f5250 	0x6d6f5250
 144:	616c662f 	0x616c662f
 148:	47006873 	c1	0x1006873
 14c:	4320554e 	c0	0x120554e
 150:	392e3420 	xori	t6,t1,0x3420
 154:	3220322e 	andi	zero,s1,0x322e
 158:	31343130 	andi	s4,t1,0x3130
 15c:	20333130 	addi	s3,at,12592
 160:	65727028 	0x65727028
 164:	656c6572 	0x656c6572
 168:	29657361 	slti	a1,t3,29537
 16c:	20472d20 	addi	a3,v0,11552
 170:	6d2d2030 	0x6d2d2030
 174:	2d206c65 	sltiu	zero,t1,27749
 178:	6372616d 	0x6372616d
 17c:	31343d68 	andi	s4,t1,0x3d68
 180:	2d203138 	sltiu	zero,t1,12600
 184:	746c706d 	jalx	1b1c1b4 <_boot-0x7e6e3e4c>0x706d
 188:	00672d20 	0x672d20
 18c:	5f786c72 	0x5f786c72
 190:	63616364 	0x63616364
 194:	735f6568 	0x735f6568
 198:	65726f74 	0x65726f74
 19c:	74636300 	jalx	18d8c00 <_boot-0x7e927400>0x6300
 1a0:	6d645f6c 	0x6d645f6c
 1a4:	63006d65 	0x63006d65
 1a8:	5f6c7463 	0x5f6c7463
 1ac:	6d656d69 	0x6d656d69
 1b0:	64646100 	0x64646100
 1b4:	4f420072 	c3	0x1420072
 1b8:	41454c4f 	0x41454c4f
 1bc:	6c72004e 	0x6c72004e
 1c0:	6d695f78 	0x6d695f78
 1c4:	725f6d65 	0x725f6d65
 1c8:	6c696665 	0x6c696665
 1cc:	6f6d006c 	0x6f6d006c
 1d0:	765f6564 	jalx	97d9590 <_boot-0x76a26a70>0x6564
 1d4:	64696c61 	0x64696c61
 1d8:	73616200 	0x73616200
 1dc:	6c720065 	0x6c720065
 1e0:	61635f78 	0x61635f78
 1e4:	2e656863 	sltiu	a1,s3,26723
 1e8:	64610063 	0x64610063
 1ec:	73657264 	0x73657264
 1f0:	6c720073 	0x6c720073
 1f4:	63645f78 	0x63645f78
 1f8:	65686361 	0x65686361
 1fc:	766e695f 	jalx	9b9a57c <_boot-0x76665a84>0x695f
 200:	64696c61 	0x64696c61
 204:	5f657461 	0x5f657461
 208:	006c6c61 	0x6c6c61
 20c:	6c746363 	0x6c746363
 210:	6c61765f 	0x6c61765f
 214:	646f6d00 	0x646f6d00
 218:	6c720065 	0x6c720065
 21c:	63645f78 	0x63645f78
 220:	65686361 	0x65686361
 224:	756c665f 	jalx	5b1997c <_boot-0x7a6e6684>0x665f
 228:	72006873 	0x72006873
 22c:	695f786c 	0x695f786c
 230:	68636163 	0x68636163
 234:	6f6c5f65 	0x6f6c5f65
 238:	73006b63 	0x73006b63
 23c:	00657a69 	0x657a69
 240:	5f786c72 	0x5f786c72
 244:	63616364 	0x63616364
 248:	695f6568 	0x695f6568
 24c:	6c61766e 	0x6c61766e
 250:	74616469 	jalx	18591a4 <_boot-0x7e9a6e5c>0x6469
 254:	6e750065 	0x6e750065
 258:	6e676973 	0x6e676973
 25c:	69206465 	0x69206465
 260:	7500746e 	jalx	401d1b8 <_boot-0x7c1e2e48>0x746e
 264:	6769736e 	0x6769736e
 268:	2064656e 	addi	a0,v1,25966
 26c:	00746e69 	0x746e69

Disassembly of section .debug_frame:

00000000 <.debug_frame>:
   0:	0000000c 	syscall
   4:	ffffffff 	0xffffffff
   8:	7c010001 	0x7c010001
   c:	001d0d1f 	0x1d0d1f
  10:	00000020 	add	zero,zero,zero
  14:	00000000 	nop
  18:	80200078 	lb	zero,120(at)
  1c:	0000005c 	0x5c
  20:	44080e44 	0x44080e44
  24:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  28:	0d40021e 	jal	5000878 <_boot-0x7b1ff788>
  2c:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
  30:	00000000 	nop
  34:	0000001c 	0x1c
  38:	00000000 	nop
  3c:	802000d4 	lb	zero,212(at)
  40:	00000020 	add	zero,zero,zero
  44:	44080e44 	0x44080e44
  48:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  4c:	1d0d441e 	0x1d0d441e
  50:	000ede48 	0xede48
  54:	0000001c 	0x1c
  58:	00000000 	nop
  5c:	802000f4 	lb	zero,244(at)
  60:	00000020 	add	zero,zero,zero
  64:	44080e44 	0x44080e44
  68:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  6c:	1d0d441e 	0x1d0d441e
  70:	000ede48 	0xede48
  74:	0000001c 	0x1c
  78:	00000000 	nop
  7c:	80200114 	lb	zero,276(at)
  80:	00000020 	add	zero,zero,zero
  84:	44080e44 	0x44080e44
  88:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  8c:	1d0d441e 	0x1d0d441e
  90:	000ede48 	0xede48
  94:	0000001c 	0x1c
  98:	00000000 	nop
  9c:	80200134 	lb	zero,308(at)
  a0:	00000020 	add	zero,zero,zero
  a4:	44080e44 	0x44080e44
  a8:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  ac:	1d0d441e 	0x1d0d441e
  b0:	000ede48 	0xede48
  b4:	0000001c 	0x1c
  b8:	00000000 	nop
  bc:	80200154 	lb	zero,340(at)
  c0:	00000020 	add	zero,zero,zero
  c4:	44080e44 	0x44080e44
  c8:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  cc:	1d0d441e 	0x1d0d441e
  d0:	000ede48 	0xede48
  d4:	0000001c 	0x1c
  d8:	00000000 	nop
  dc:	80200174 	lb	zero,372(at)
  e0:	00000020 	add	zero,zero,zero
  e4:	44080e44 	0x44080e44
  e8:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
  ec:	1d0d441e 	0x1d0d441e
  f0:	000ede48 	0xede48
  f4:	0000001c 	0x1c
  f8:	00000000 	nop
  fc:	80200194 	lb	zero,404(at)
 100:	00000020 	add	zero,zero,zero
 104:	44080e44 	0x44080e44
 108:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 10c:	1d0d441e 	0x1d0d441e
 110:	000ede48 	0xede48
 114:	0000001c 	0x1c
 118:	00000000 	nop
 11c:	802001b4 	lb	zero,436(at)
 120:	00000020 	add	zero,zero,zero
 124:	44080e44 	0x44080e44
 128:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 12c:	1d0d441e 	0x1d0d441e
 130:	000ede48 	0xede48
 134:	0000001c 	0x1c
 138:	00000000 	nop
 13c:	802001d4 	lb	zero,468(at)
 140:	00000020 	add	zero,zero,zero
 144:	44080e44 	0x44080e44
 148:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 14c:	1d0d441e 	0x1d0d441e
 150:	000ede48 	0xede48
 154:	0000001c 	0x1c
 158:	00000000 	nop
 15c:	802001f4 	lb	zero,500(at)
 160:	00000020 	add	zero,zero,zero
 164:	44080e44 	0x44080e44
 168:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 16c:	1d0d441e 	0x1d0d441e
 170:	000ede48 	0xede48
 174:	00000024 	and	zero,zero,zero
 178:	00000000 	nop
 17c:	80200214 	lb	zero,532(at)
 180:	000004e8 	0x4e8
 184:	48280e44 	0x48280e44
 188:	029e019f 	0x29e019f
 18c:	031e0d44 	0x31e0d44
 190:	1d0d04c4 	0x1d0d04c4
 194:	0edfde4c 	jal	b7f7930 <_boot-0x74a086d0>
 198:	00000000 	nop
 19c:	0000000c 	syscall
 1a0:	ffffffff 	0xffffffff
 1a4:	7c010001 	0x7c010001
 1a8:	001d0d1f 	0x1d0d1f
 1ac:	00000020 	add	zero,zero,zero
 1b0:	0000019c 	0x19c
 1b4:	802006fc 	lb	zero,1788(at)
 1b8:	0000007c 	0x7c
 1bc:	44100e44 	0x44100e44
 1c0:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 1c4:	0d60021e 	jal	5800878 <_boot-0x7a9ff788>
 1c8:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 1cc:	00000000 	nop
 1d0:	00000020 	add	zero,zero,zero
 1d4:	0000019c 	0x19c
 1d8:	80200778 	lb	zero,1912(at)
 1dc:	0000007c 	0x7c
 1e0:	44100e44 	0x44100e44
 1e4:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 1e8:	0d60021e 	jal	5800878 <_boot-0x7a9ff788>
 1ec:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 1f0:	00000000 	nop
 1f4:	00000020 	add	zero,zero,zero
 1f8:	0000019c 	0x19c
 1fc:	802007f4 	lb	zero,2036(at)
 200:	0000007c 	0x7c
 204:	44100e44 	0x44100e44
 208:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 20c:	0d60021e 	jal	5800878 <_boot-0x7a9ff788>
 210:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 214:	00000000 	nop
 218:	0000001c 	0x1c
 21c:	0000019c 	0x19c
 220:	80200870 	lb	zero,2160(at)
 224:	00000054 	0x54
 228:	44100e44 	0x44100e44
 22c:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 230:	1d0d781e 	0x1d0d781e
 234:	000ede48 	0xede48
 238:	0000001c 	0x1c
 23c:	0000019c 	0x19c
 240:	802008c4 	lb	zero,2244(at)
 244:	00000054 	0x54
 248:	44100e44 	0x44100e44
 24c:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 250:	1d0d781e 	0x1d0d781e
 254:	000ede48 	0xede48
 258:	00000020 	add	zero,zero,zero
 25c:	0000019c 	0x19c
 260:	80200918 	lb	zero,2328(at)
 264:	000000b0 	0xb0
 268:	44100e44 	0x44100e44
 26c:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 270:	0d94021e 	jal	6500878 <_boot-0x79cff788>
 274:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 278:	00000000 	nop
 27c:	00000020 	add	zero,zero,zero
 280:	0000019c 	0x19c
 284:	802009c8 	lb	zero,2504(at)
 288:	00000064 	0x64
 28c:	44100e44 	0x44100e44
 290:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 294:	0d48021e 	jal	5200878 <_boot-0x7afff788>
 298:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 29c:	00000000 	nop
 2a0:	00000020 	add	zero,zero,zero
 2a4:	0000019c 	0x19c
 2a8:	80200a2c 	lb	zero,2604(at)
 2ac:	00000094 	0x94
 2b0:	44100e44 	0x44100e44
 2b4:	0d44019e 	jal	5100678 <_boot-0x7b0ff988>
 2b8:	0d78021e 	jal	5e00878 <_boot-0x7a3ff788>
 2bc:	0ede481d 	jal	b792074 <_boot-0x74a6df8c>
 2c0:	00000000 	nop
 2c4:	0000000c 	syscall
 2c8:	ffffffff 	0xffffffff
 2cc:	04010001 	b	2d4 <_boot-0x801ffd2c>
 2d0:	001d0c1f 	0x1d0c1f
 2d4:	0000000c 	syscall
 2d8:	000002c4 	0x2c4
 2dc:	802017b0 	lb	zero,6064(at)
 2e0:	0000003c 	0x3c
 2e4:	0000000c 	syscall
 2e8:	ffffffff 	0xffffffff
 2ec:	04010001 	b	2f4 <_boot-0x801ffd0c>
 2f0:	001d0c1f 	0x1d0c1f
 2f4:	0000000c 	syscall
 2f8:	000002e4 	0x2e4
 2fc:	802017ec 	lb	zero,6124(at)
 300:	000000a4 	0xa4

Disassembly of section .debug_pubnames:

00000000 <.debug_pubnames>:
   0:	00000019 	multu	zero,zero
   4:	04f20002 	0x4f20002
   8:	01b10000 	0x1b10000
   c:	014b0000 	0x14b0000
  10:	656d0000 	0x656d0000
  14:	706d636d 	0x706d636d
  18:	00000000 	nop
  1c:	00001900 	sll	v1,zero,0x4
  20:	a3000200 	sb	zero,512(t8)
  24:	24000006 	li	zero,6
  28:	5c000002 	0x5c000002
  2c:	6d000001 	0x6d000001
  30:	65736d65 	0x65736d65
  34:	00000074 	0x74
	...
