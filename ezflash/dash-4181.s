
dash_dbg.exe:     file format elf32-littlemips

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .boot         00000078  80200000  80200000  00000060  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .text         000017b8  80200078  80200078  000000d8  2**2
                  CONTENTS, ALLOC, LOAD, CODE
  2 .data         00000004  80201830  80201830  00001890  2**0
                  CONTENTS, ALLOC, LOAD, DATA
  3 .bss          00000420  80201840  80201840  00001894  2**4
                  ALLOC
  4 .reginfo      00000018  00000000  00000000  00001894  2**2
                  CONTENTS, READONLY, LINK_ONCE_DISCARD
  5 .pdr          00000560  00000000  00000000  000018ac  2**2
                  CONTENTS, READONLY
  6 .debug_abbrev 000000ea  00000000  00000000  00001e0c  2**0
                  CONTENTS, READONLY, DEBUGGING
  7 .debug_info   000002ec  00000000  00000000  00001ef6  2**0
                  CONTENTS, READONLY, DEBUGGING
  8 .debug_line   000000be  00000000  00000000  000021e2  2**0
                  CONTENTS, READONLY, DEBUGGING
  9 .debug_frame  00000208  00000000  00000000  000022a0  2**2
                  CONTENTS, READONLY, DEBUGGING
 10 .debug_pubnames 00000174  00000000  00000000  000024a8  2**0
                  CONTENTS, READONLY, DEBUGGING
 11 .debug_aranges 00000020  00000000  00000000  0000261c  2**0
                  CONTENTS, READONLY, DEBUGGING
 12 .debug_abbrev.1 000000b6  00000000  00000000  0000263c  2**0
                  CONTENTS, READONLY, DEBUGGING
 13 .debug_info.2 0000036d  00000000  00000000  000026f2  2**0
                  CONTENTS, READONLY, DEBUGGING
 14 .debug_line.3 000000a4  00000000  00000000  00002a5f  2**0
                  CONTENTS, READONLY, DEBUGGING
 15 .debug_aranges.4 00000020  00000000  00000000  00002b03  2**0
                  CONTENTS, READONLY, DEBUGGING
 16 .debug_str    0000002b  00000000  00000000  00002b23  2**0
                  CONTENTS, READONLY, DEBUGGING
 17 .debug_line.5 0000004c  00000000  00000000  00002b4e  2**0
                  CONTENTS, READONLY, DEBUGGING
 18 .debug_info.6 00000052  00000000  00000000  00002b9a  2**0
                  CONTENTS, READONLY, DEBUGGING
 19 .debug_abbrev.7 00000014  00000000  00000000  00002bec  2**0
                  CONTENTS, READONLY, DEBUGGING
 20 .debug_aranges.8 00000020  00000000  00000000  00002c00  2**3
                  CONTENTS, READONLY, DEBUGGING
 21 .debug_line.9 0000033d  00000000  00000000  00002c20  2**0
                  CONTENTS, READONLY, DEBUGGING
 22 .debug_info.10 00000051  00000000  00000000  00002f5d  2**0
                  CONTENTS, READONLY, DEBUGGING
 23 .debug_abbrev.11 00000014  00000000  00000000  00002fae  2**0
                  CONTENTS, READONLY, DEBUGGING
 24 .debug_aranges.12 00000020  00000000  00000000  00002fc8  2**3
                  CONTENTS, READONLY, DEBUGGING
 25 .debug_abbrev.13 0000007f  00000000  00000000  00002fe8  2**0
                  CONTENTS, READONLY, DEBUGGING
 26 .debug_info.14 000001b2  00000000  00000000  00003067  2**0
                  CONTENTS, READONLY, DEBUGGING
 27 .debug_line.15 000000c4  00000000  00000000  00003219  2**0
                  CONTENTS, READONLY, DEBUGGING
 28 .debug_aranges.16 00000020  00000000  00000000  000032dd  2**0
                  CONTENTS, READONLY, DEBUGGING
 29 .debug_abbrev.17 00000097  00000000  00000000  000032fd  2**0
                  CONTENTS, READONLY, DEBUGGING
 30 .debug_info.18 00000225  00000000  00000000  00003394  2**0
                  CONTENTS, READONLY, DEBUGGING
 31 .debug_line.19 000000e0  00000000  00000000  000035b9  2**0
                  CONTENTS, READONLY, DEBUGGING
 32 .debug_aranges.20 00000020  00000000  00000000  00003699  2**0
                  CONTENTS, READONLY, DEBUGGING
Disassembly of section .boot:

80200000 <_boot>:
_boot():
80200000:	40146000 	mfc0	s4,$12
80200004:	00000000 	nop
80200008:	3c01ffbf 	lui	at,0xffbf
8020000c:	3421ffff 	ori	at,at,0xffff
80200010:	02814024 	and	t0,s4,at
80200014:	3c019000 	lui	at,0x9000
80200018:	01014025 	or	t0,t0,at
8020001c:	40886000 	mtc0	t0,$12
80200020:	40806800 	mtc0	zero,$13
80200024:	00000000 	nop
80200028:	3c0a0010 	lui	t2,0x10
8020002c:	3c0b0011 	lui	t3,0x11
80200030:	356bffff 	ori	t3,t3,0xffff
80200034:	4c8a2000 	mtc3	t2,$4
80200038:	4c8b2800 	mtc3	t3,$5
8020003c:	4008a000 	mfc0	t0,$20
80200040:	00000000 	nop
80200044:	35080400 	ori	t0,t0,0x400
80200048:	4088a000 	mtc0	t0,$20
8020004c:	00000000 	nop
80200050:	3c1d8020 	lui	sp,0x8020
80200054:	27bd1c40 	addiu	sp,sp,7232
80200058:	3c1c8020 	lui	gp,0x8020
8020005c:	279c1840 	addiu	gp,gp,6208
80200060:	3c088020 	lui	t0,0x8020
80200064:	25080174 	addiu	t0,t0,372
80200068:	0100f809 	jalr	t0
8020006c:	00000000 	nop
80200070:	1000ffff 	b	80200070 <_boot+0x70>
80200074:	00000000 	nop
Disassembly of section .text:

80200078 <disable_master_engine>:
disable_master_engine():
/home/phinex/uCOSII/ezflash/app.c:15
//#define IMEM


disable_master_engine()
{
80200078:	27bdfff8 	addiu	sp,sp,-8
8020007c:	afbe0000 	sw	s8,0(sp)
80200080:	03a0f021 	move	s8,sp
/home/phinex/uCOSII/ezflash/app.c:16
    REG32(0xB2000044) = 0x00; //disable gmac
80200084:	3c02b200 	lui	v0,0xb200
80200088:	34420044 	ori	v0,v0,0x44
8020008c:	ac400000 	sw	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:17
    REG8(0xB2000036) = 0x00;
80200090:	3c02b200 	lui	v0,0xb200
80200094:	34420036 	ori	v0,v0,0x36
80200098:	a0400000 	sb	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:18
    REG8(0xB2001000) = 0x00;  //disable smbus
8020009c:	3c02b200 	lui	v0,0xb200
802000a0:	34421000 	ori	v0,v0,0x1000
802000a4:	a0400000 	sb	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:19
    REG8(0xB2001810)  = 0x00; //disable tcr
802000a8:	3c02b200 	lui	v0,0xb200
802000ac:	34421810 	ori	v0,v0,0x1810
802000b0:	a0400000 	sb	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:20
    REG32(0xB1000008) = 0x00000000;  //disable timer
802000b4:	3c02b100 	lui	v0,0xb100
802000b8:	34420008 	ori	v0,v0,0x8
802000bc:	ac400000 	sw	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:21
}
802000c0:	03c0e821 	move	sp,s8
802000c4:	8fbe0000 	lw	s8,0(sp)
802000c8:	27bd0008 	addiu	sp,sp,8
802000cc:	03e00008 	jr	ra
802000d0:	00000000 	nop
dummy0():
/home/phinex/uCOSII/ezflash/app.c:23

void dummy0() { }
802000d4:	27bdfff8 	addiu	sp,sp,-8
802000d8:	afbe0000 	sw	s8,0(sp)
802000dc:	03a0f021 	move	s8,sp
802000e0:	03c0e821 	move	sp,s8
802000e4:	8fbe0000 	lw	s8,0(sp)
802000e8:	27bd0008 	addiu	sp,sp,8
802000ec:	03e00008 	jr	ra
802000f0:	00000000 	nop
dummy1():
/home/phinex/uCOSII/ezflash/app.c:24
void dummy1() { }
802000f4:	27bdfff8 	addiu	sp,sp,-8
802000f8:	afbe0000 	sw	s8,0(sp)
802000fc:	03a0f021 	move	s8,sp
80200100:	03c0e821 	move	sp,s8
80200104:	8fbe0000 	lw	s8,0(sp)
80200108:	27bd0008 	addiu	sp,sp,8
8020010c:	03e00008 	jr	ra
80200110:	00000000 	nop
dummy2():
/home/phinex/uCOSII/ezflash/app.c:25
void dummy2() { }
80200114:	27bdfff8 	addiu	sp,sp,-8
80200118:	afbe0000 	sw	s8,0(sp)
8020011c:	03a0f021 	move	s8,sp
80200120:	03c0e821 	move	sp,s8
80200124:	8fbe0000 	lw	s8,0(sp)
80200128:	27bd0008 	addiu	sp,sp,8
8020012c:	03e00008 	jr	ra
80200130:	00000000 	nop
dummy3():
/home/phinex/uCOSII/ezflash/app.c:26
void dummy3() { }
80200134:	27bdfff8 	addiu	sp,sp,-8
80200138:	afbe0000 	sw	s8,0(sp)
8020013c:	03a0f021 	move	s8,sp
80200140:	03c0e821 	move	sp,s8
80200144:	8fbe0000 	lw	s8,0(sp)
80200148:	27bd0008 	addiu	sp,sp,8
8020014c:	03e00008 	jr	ra
80200150:	00000000 	nop
dummy4():
/home/phinex/uCOSII/ezflash/app.c:27
void dummy4() { }
80200154:	27bdfff8 	addiu	sp,sp,-8
80200158:	afbe0000 	sw	s8,0(sp)
8020015c:	03a0f021 	move	s8,sp
80200160:	03c0e821 	move	sp,s8
80200164:	8fbe0000 	lw	s8,0(sp)
80200168:	27bd0008 	addiu	sp,sp,8
8020016c:	03e00008 	jr	ra
80200170:	00000000 	nop
main():
/home/phinex/uCOSII/ezflash/app.c:30

int main(void)
{
80200174:	27bdffd8 	addiu	sp,sp,-40
80200178:	afbf0024 	sw	ra,36(sp)
8020017c:	afbe0020 	sw	s8,32(sp)
80200180:	03a0f021 	move	s8,sp
/home/phinex/uCOSII/ezflash/app.c:31
  INT32U i = 0;
80200184:	afc00010 	sw	zero,16(s8)
/home/phinex/uCOSII/ezflash/app.c:32
  struct ssi_portmap *ssi_map = (struct ssi_portmap *) FLASH_BASE_ADDR;
80200188:	3c02b400 	lui	v0,0xb400
8020018c:	afc20014 	sw	v0,20(s8)
/home/phinex/uCOSII/ezflash/app.c:33
  INT32U rdid = (spi_read_id(ssi_map)  & 0x00ff0000) >> 16;
80200190:	8fc40014 	lw	a0,20(s8)
80200194:	0c080414 	jal	80201050 <_end_boot+0xfd8>
80200198:	00000000 	nop
8020019c:	00401821 	move	v1,v0
802001a0:	3c0200ff 	lui	v0,0xff
802001a4:	00621024 	and	v0,v1,v0
802001a8:	00021403 	sra	v0,v0,0x10
802001ac:	afc20018 	sw	v0,24(s8)
/home/phinex/uCOSII/ezflash/app.c:34
  rlx_icache_invalidate_all();
802001b0:	0c080210 	jal	80200840 <_end_boot+0x7c8>
802001b4:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:35
  rlx_dmem_enable(DMEM_BASE,DMEM_TOP);
802001b8:	3c040010 	lui	a0,0x10
802001bc:	3c020011 	lui	v0,0x11
802001c0:	3445ffff 	ori	a1,v0,0xffff
802001c4:	0c080257 	jal	8020095c <_end_boot+0x8e4>
802001c8:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:37

  disable_master_engine();
802001cc:	0c08001e 	jal	80200078 <_end_boot>
802001d0:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:38
  REG8(0xB9000004) = 0x00; 
802001d4:	3c02b900 	lui	v0,0xb900
802001d8:	34420004 	ori	v0,v0,0x4
802001dc:	a0400000 	sb	zero,0(v0)
/home/phinex/uCOSII/ezflash/app.c:43
  //use normal speed

  //erase block 0 & block 1 
  //total 128K at once
  if (rdid == 0x1f)
802001e0:	8fc30018 	lw	v1,24(s8)
802001e4:	2402001f 	li	v0,31
802001e8:	14620005 	bne	v1,v0,80200200 <_end_boot+0x188>
802001ec:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:44
      spi_se_unprotect(ssi_map, 0x080000);
802001f0:	8fc40014 	lw	a0,20(s8)
802001f4:	3c050008 	lui	a1,0x8
802001f8:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
802001fc:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:45
  spi_blk_erase(ssi_map, 0x080000);
80200200:	8fc40014 	lw	a0,20(s8)
80200204:	3c050008 	lui	a1,0x8
80200208:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
8020020c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:46
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
80200210:	3c048010 	lui	a0,0x8010
80200214:	240500ff 	li	a1,255
80200218:	3c060002 	lui	a2,0x2
8020021c:	0c0805e3 	jal	8020178c <_end_boot+0x1714>
80200220:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:47
  if (rdid == 0x1f)
80200224:	8fc30018 	lw	v1,24(s8)
80200228:	2402001f 	li	v0,31
8020022c:	14620005 	bne	v1,v0,80200244 <_end_boot+0x1cc>
80200230:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:48
      spi_se_unprotect(ssi_map, 0x090000);
80200234:	8fc40014 	lw	a0,20(s8)
80200238:	3c050009 	lui	a1,0x9
8020023c:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200240:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:49
  spi_blk_erase(ssi_map, 0x090000);
80200244:	8fc40014 	lw	a0,20(s8)
80200248:	3c050009 	lui	a1,0x9
8020024c:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
80200250:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:52

  //load image from EPI script
  dummy0();
80200254:	0c080035 	jal	802000d4 <_end_boot+0x5c>
80200258:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:54

  for(i= 0; i < NUMLOOP; i++) 
8020025c:	afc00010 	sw	zero,16(s8)
80200260:	8fc20010 	lw	v0,16(s8)
80200264:	00000000 	nop
80200268:	2c420800 	sltiu	v0,v0,2048
8020026c:	10400016 	beqz	v0,802002c8 <_end_boot+0x250>
80200270:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:55
  spi_write(ssi_map, 0x080000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
80200274:	8fc20010 	lw	v0,16(s8)
80200278:	00000000 	nop
8020027c:	00021980 	sll	v1,v0,0x6
80200280:	3c020008 	lui	v0,0x8
80200284:	00622821 	addu	a1,v1,v0
80200288:	8fc20010 	lw	v0,16(s8)
8020028c:	00000000 	nop
80200290:	00021980 	sll	v1,v0,0x6
80200294:	3c028010 	lui	v0,0x8010
80200298:	00621021 	addu	v0,v1,v0
8020029c:	8fc40014 	lw	a0,20(s8)
802002a0:	00403021 	move	a2,v0
802002a4:	24070040 	li	a3,64
802002a8:	0c080508 	jal	80201420 <_end_boot+0x13a8>
802002ac:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:54
802002b0:	8fc20010 	lw	v0,16(s8)
802002b4:	00000000 	nop
802002b8:	24420001 	addiu	v0,v0,1
802002bc:	afc20010 	sw	v0,16(s8)
802002c0:	08080098 	j	80200260 <_end_boot+0x1e8>
802002c4:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:57

  if(memcmp((void *) 0x80100000, (void *) 0xA1080000, DMEMSIZE) != 0)
802002c8:	3c048010 	lui	a0,0x8010
802002cc:	3c05a108 	lui	a1,0xa108
802002d0:	3c060002 	lui	a2,0x2
802002d4:	0c0805d4 	jal	80201750 <_end_boot+0x16d8>
802002d8:	00000000 	nop
802002dc:	10400007 	beqz	v0,802002fc <_end_boot+0x284>
802002e0:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:58
		  stage++;
802002e4:	3c038020 	lui	v1,0x8020
802002e8:	3c028020 	lui	v0,0x8020
802002ec:	90421830 	lbu	v0,6192(v0)
802002f0:	00000000 	nop
802002f4:	24420001 	addiu	v0,v0,1
802002f8:	a0621830 	sb	v0,6192(v1)
/home/phinex/uCOSII/ezflash/app.c:61

  //erase block 2 & block 3 => total 128K
  if (rdid == 0x1f)
802002fc:	8fc30018 	lw	v1,24(s8)
80200300:	2402001f 	li	v0,31
80200304:	14620005 	bne	v1,v0,8020031c <_end_boot+0x2a4>
80200308:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:62
      spi_se_unprotect(ssi_map, 0x0A0000);
8020030c:	8fc40014 	lw	a0,20(s8)
80200310:	3c05000a 	lui	a1,0xa
80200314:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200318:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:63
  spi_blk_erase(ssi_map, 0x0A0000);
8020031c:	8fc40014 	lw	a0,20(s8)
80200320:	3c05000a 	lui	a1,0xa
80200324:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
80200328:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:64
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
8020032c:	3c048010 	lui	a0,0x8010
80200330:	240500ff 	li	a1,255
80200334:	3c060002 	lui	a2,0x2
80200338:	0c0805e3 	jal	8020178c <_end_boot+0x1714>
8020033c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:65
  if (rdid == 0x1f)
80200340:	8fc30018 	lw	v1,24(s8)
80200344:	2402001f 	li	v0,31
80200348:	14620005 	bne	v1,v0,80200360 <_end_boot+0x2e8>
8020034c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:66
      spi_se_unprotect(ssi_map, 0x0B0000);
80200350:	8fc40014 	lw	a0,20(s8)
80200354:	3c05000b 	lui	a1,0xb
80200358:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
8020035c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:67
  spi_blk_erase(ssi_map, 0x0B0000);
80200360:	8fc40014 	lw	a0,20(s8)
80200364:	3c05000b 	lui	a1,0xb
80200368:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
8020036c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:69

  dummy1();
80200370:	0c08003d 	jal	802000f4 <_end_boot+0x7c>
80200374:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:71

  for(i= 0; i < NUMLOOP; i++) 
80200378:	afc00010 	sw	zero,16(s8)
8020037c:	8fc20010 	lw	v0,16(s8)
80200380:	00000000 	nop
80200384:	2c420800 	sltiu	v0,v0,2048
80200388:	10400016 	beqz	v0,802003e4 <_end_boot+0x36c>
8020038c:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:72
  spi_write(ssi_map, 0x0A0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
80200390:	8fc20010 	lw	v0,16(s8)
80200394:	00000000 	nop
80200398:	00021980 	sll	v1,v0,0x6
8020039c:	3c02000a 	lui	v0,0xa
802003a0:	00622821 	addu	a1,v1,v0
802003a4:	8fc20010 	lw	v0,16(s8)
802003a8:	00000000 	nop
802003ac:	00021980 	sll	v1,v0,0x6
802003b0:	3c028010 	lui	v0,0x8010
802003b4:	00621021 	addu	v0,v1,v0
802003b8:	8fc40014 	lw	a0,20(s8)
802003bc:	00403021 	move	a2,v0
802003c0:	24070040 	li	a3,64
802003c4:	0c080508 	jal	80201420 <_end_boot+0x13a8>
802003c8:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:71
802003cc:	8fc20010 	lw	v0,16(s8)
802003d0:	00000000 	nop
802003d4:	24420001 	addiu	v0,v0,1
802003d8:	afc20010 	sw	v0,16(s8)
802003dc:	080800df 	j	8020037c <_end_boot+0x304>
802003e0:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:74

  if(memcmp((void *) 0x80100000, (void *) 0xA10A0000, DMEMSIZE) != 0)
802003e4:	3c048010 	lui	a0,0x8010
802003e8:	3c05a10a 	lui	a1,0xa10a
802003ec:	3c060002 	lui	a2,0x2
802003f0:	0c0805d4 	jal	80201750 <_end_boot+0x16d8>
802003f4:	00000000 	nop
802003f8:	10400007 	beqz	v0,80200418 <_end_boot+0x3a0>
802003fc:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:75
		  stage++;
80200400:	3c038020 	lui	v1,0x8020
80200404:	3c028020 	lui	v0,0x8020
80200408:	90421830 	lbu	v0,6192(v0)
8020040c:	00000000 	nop
80200410:	24420001 	addiu	v0,v0,1
80200414:	a0621830 	sb	v0,6192(v1)
/home/phinex/uCOSII/ezflash/app.c:78

  //erase block 4 & block 5 => total 128K
  if (rdid == 0x1f)
80200418:	8fc30018 	lw	v1,24(s8)
8020041c:	2402001f 	li	v0,31
80200420:	14620005 	bne	v1,v0,80200438 <_end_boot+0x3c0>
80200424:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:79
      spi_se_unprotect(ssi_map, 0x0C0000);
80200428:	8fc40014 	lw	a0,20(s8)
8020042c:	3c05000c 	lui	a1,0xc
80200430:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200434:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:80
  spi_blk_erase(ssi_map, 0x0C0000);
80200438:	8fc40014 	lw	a0,20(s8)
8020043c:	3c05000c 	lui	a1,0xc
80200440:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
80200444:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:81
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
80200448:	3c048010 	lui	a0,0x8010
8020044c:	240500ff 	li	a1,255
80200450:	3c060002 	lui	a2,0x2
80200454:	0c0805e3 	jal	8020178c <_end_boot+0x1714>
80200458:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:82
  if (rdid == 0x1f)
8020045c:	8fc30018 	lw	v1,24(s8)
80200460:	2402001f 	li	v0,31
80200464:	14620005 	bne	v1,v0,8020047c <_end_boot+0x404>
80200468:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:83
      spi_se_unprotect(ssi_map, 0x0D0000);
8020046c:	8fc40014 	lw	a0,20(s8)
80200470:	3c05000d 	lui	a1,0xd
80200474:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200478:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:84
  spi_blk_erase(ssi_map, 0x0D0000);
8020047c:	8fc40014 	lw	a0,20(s8)
80200480:	3c05000d 	lui	a1,0xd
80200484:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
80200488:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:86

  dummy2();
8020048c:	0c080045 	jal	80200114 <_end_boot+0x9c>
80200490:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:88

  for(i= 0; i < NUMLOOP; i++) 
80200494:	afc00010 	sw	zero,16(s8)
80200498:	8fc20010 	lw	v0,16(s8)
8020049c:	00000000 	nop
802004a0:	2c420800 	sltiu	v0,v0,2048
802004a4:	10400016 	beqz	v0,80200500 <_end_boot+0x488>
802004a8:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:89
  spi_write(ssi_map, 0x0C0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
802004ac:	8fc20010 	lw	v0,16(s8)
802004b0:	00000000 	nop
802004b4:	00021980 	sll	v1,v0,0x6
802004b8:	3c02000c 	lui	v0,0xc
802004bc:	00622821 	addu	a1,v1,v0
802004c0:	8fc20010 	lw	v0,16(s8)
802004c4:	00000000 	nop
802004c8:	00021980 	sll	v1,v0,0x6
802004cc:	3c028010 	lui	v0,0x8010
802004d0:	00621021 	addu	v0,v1,v0
802004d4:	8fc40014 	lw	a0,20(s8)
802004d8:	00403021 	move	a2,v0
802004dc:	24070040 	li	a3,64
802004e0:	0c080508 	jal	80201420 <_end_boot+0x13a8>
802004e4:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:88
802004e8:	8fc20010 	lw	v0,16(s8)
802004ec:	00000000 	nop
802004f0:	24420001 	addiu	v0,v0,1
802004f4:	afc20010 	sw	v0,16(s8)
802004f8:	08080126 	j	80200498 <_end_boot+0x420>
802004fc:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:91

  if(memcmp((void *) 0x80100000, (void *) 0xA10C0000, DMEMSIZE) != 0)
80200500:	3c048010 	lui	a0,0x8010
80200504:	3c05a10c 	lui	a1,0xa10c
80200508:	3c060002 	lui	a2,0x2
8020050c:	0c0805d4 	jal	80201750 <_end_boot+0x16d8>
80200510:	00000000 	nop
80200514:	10400007 	beqz	v0,80200534 <_end_boot+0x4bc>
80200518:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:92
		  stage++;
8020051c:	3c038020 	lui	v1,0x8020
80200520:	3c028020 	lui	v0,0x8020
80200524:	90421830 	lbu	v0,6192(v0)
80200528:	00000000 	nop
8020052c:	24420001 	addiu	v0,v0,1
80200530:	a0621830 	sb	v0,6192(v1)
/home/phinex/uCOSII/ezflash/app.c:94

  if (rdid == 0x1f)
80200534:	8fc30018 	lw	v1,24(s8)
80200538:	2402001f 	li	v0,31
8020053c:	14620005 	bne	v1,v0,80200554 <_end_boot+0x4dc>
80200540:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:95
      spi_se_unprotect(ssi_map, 0x0D0000);
80200544:	8fc40014 	lw	a0,20(s8)
80200548:	3c05000d 	lui	a1,0xd
8020054c:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200550:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:96
  spi_blk_erase(ssi_map, 0x0D0000);
80200554:	8fc40014 	lw	a0,20(s8)
80200558:	3c05000d 	lui	a1,0xd
8020055c:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
80200560:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:97
  memset((void *) DMEM_V_BASE, 0xff, DMEM_V_TOP - DMEM_V_BASE);
80200564:	3c048010 	lui	a0,0x8010
80200568:	240500ff 	li	a1,255
8020056c:	3c060002 	lui	a2,0x2
80200570:	0c0805e3 	jal	8020178c <_end_boot+0x1714>
80200574:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:98
  if (rdid == 0x1f)
80200578:	8fc30018 	lw	v1,24(s8)
8020057c:	2402001f 	li	v0,31
80200580:	14620005 	bne	v1,v0,80200598 <_end_boot+0x520>
80200584:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:99
      spi_se_unprotect(ssi_map, 0x0E0000);
80200588:	8fc40014 	lw	a0,20(s8)
8020058c:	3c05000e 	lui	a1,0xe
80200590:	0c080321 	jal	80200c84 <_end_boot+0xc0c>
80200594:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:100
  spi_blk_erase(ssi_map, 0x0E0000);
80200598:	8fc40014 	lw	a0,20(s8)
8020059c:	3c05000e 	lui	a1,0xe
802005a0:	0c0802d7 	jal	80200b5c <_end_boot+0xae4>
802005a4:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:102

  dummy3();
802005a8:	0c08004d 	jal	80200134 <_end_boot+0xbc>
802005ac:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:104

  for(i= 0; i < NUMLOOP; i++) 
802005b0:	afc00010 	sw	zero,16(s8)
802005b4:	8fc20010 	lw	v0,16(s8)
802005b8:	00000000 	nop
802005bc:	2c420800 	sltiu	v0,v0,2048
802005c0:	10400016 	beqz	v0,8020061c <_end_boot+0x5a4>
802005c4:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:105
  spi_write(ssi_map, 0x0D0000+i*WRSIZE, 0x80100000+i*WRSIZE, WRSIZE);
802005c8:	8fc20010 	lw	v0,16(s8)
802005cc:	00000000 	nop
802005d0:	00021980 	sll	v1,v0,0x6
802005d4:	3c02000d 	lui	v0,0xd
802005d8:	00622821 	addu	a1,v1,v0
802005dc:	8fc20010 	lw	v0,16(s8)
802005e0:	00000000 	nop
802005e4:	00021980 	sll	v1,v0,0x6
802005e8:	3c028010 	lui	v0,0x8010
802005ec:	00621021 	addu	v0,v1,v0
802005f0:	8fc40014 	lw	a0,20(s8)
802005f4:	00403021 	move	a2,v0
802005f8:	24070040 	li	a3,64
802005fc:	0c080508 	jal	80201420 <_end_boot+0x13a8>
80200600:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:104
80200604:	8fc20010 	lw	v0,16(s8)
80200608:	00000000 	nop
8020060c:	24420001 	addiu	v0,v0,1
80200610:	afc20010 	sw	v0,16(s8)
80200614:	0808016d 	j	802005b4 <_end_boot+0x53c>
80200618:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:107

  if(memcmp((void *) 0x80100000, (void *) 0xA10D0000, DMEMSIZE) != 0)
8020061c:	3c048010 	lui	a0,0x8010
80200620:	3c05a10d 	lui	a1,0xa10d
80200624:	3c060002 	lui	a2,0x2
80200628:	0c0805d4 	jal	80201750 <_end_boot+0x16d8>
8020062c:	00000000 	nop
80200630:	10400007 	beqz	v0,80200650 <_end_boot+0x5d8>
80200634:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:108
		  stage++;
80200638:	3c038020 	lui	v1,0x8020
8020063c:	3c028020 	lui	v0,0x8020
80200640:	90421830 	lbu	v0,6192(v0)
80200644:	00000000 	nop
80200648:	24420001 	addiu	v0,v0,1
8020064c:	a0621830 	sb	v0,6192(v1)
/home/phinex/uCOSII/ezflash/app.c:110

 dummy4();
80200650:	0c080055 	jal	80200154 <_end_boot+0xdc>
80200654:	00000000 	nop
/home/phinex/uCOSII/ezflash/app.c:111
}
80200658:	03c0e821 	move	sp,s8
8020065c:	8fbf0024 	lw	ra,36(sp)
80200660:	8fbe0020 	lw	s8,32(sp)
80200664:	27bd0028 	addiu	sp,sp,40
80200668:	03e00008 	jr	ra
8020066c:	00000000 	nop
rlx_dcache_flush():
80200670:	27bdfff0 	addiu	sp,sp,-16
80200674:	afbe0008 	sw	s8,8(sp)
80200678:	03a0f021 	move	s8,sp
8020067c:	afc40010 	sw	a0,16(s8)
80200680:	afc50014 	sw	a1,20(s8)
80200684:	8fc20010 	lw	v0,16(s8)
80200688:	00000000 	nop
8020068c:	afc20000 	sw	v0,0(s8)
80200690:	8fc30010 	lw	v1,16(s8)
80200694:	8fc20014 	lw	v0,20(s8)
80200698:	00000000 	nop
8020069c:	00621821 	addu	v1,v1,v0
802006a0:	8fc20000 	lw	v0,0(s8)
802006a4:	00000000 	nop
802006a8:	0043102b 	sltu	v0,v0,v1
802006ac:	1040000a 	beqz	v0,802006d8 <_end_boot+0x660>
802006b0:	00000000 	nop
802006b4:	8fc20000 	lw	v0,0(s8)
802006b8:	00000000 	nop
802006bc:	bc550000 	cache	0x15,0(v0)
802006c0:	8fc20000 	lw	v0,0(s8)
802006c4:	00000000 	nop
802006c8:	24420004 	addiu	v0,v0,4
802006cc:	afc20000 	sw	v0,0(s8)
802006d0:	080801a4 	j	80200690 <_end_boot+0x618>
802006d4:	00000000 	nop
802006d8:	03c0e821 	move	sp,s8
802006dc:	8fbe0008 	lw	s8,8(sp)
802006e0:	27bd0010 	addiu	sp,sp,16
802006e4:	03e00008 	jr	ra
802006e8:	00000000 	nop
rlx_dcache_invalidate():
802006ec:	27bdfff0 	addiu	sp,sp,-16
802006f0:	afbe0008 	sw	s8,8(sp)
802006f4:	03a0f021 	move	s8,sp
802006f8:	afc40010 	sw	a0,16(s8)
802006fc:	afc50014 	sw	a1,20(s8)
80200700:	8fc20010 	lw	v0,16(s8)
80200704:	00000000 	nop
80200708:	afc20000 	sw	v0,0(s8)
8020070c:	8fc30010 	lw	v1,16(s8)
80200710:	8fc20014 	lw	v0,20(s8)
80200714:	00000000 	nop
80200718:	00621821 	addu	v1,v1,v0
8020071c:	8fc20000 	lw	v0,0(s8)
80200720:	00000000 	nop
80200724:	0043102b 	sltu	v0,v0,v1
80200728:	1040000a 	beqz	v0,80200754 <_end_boot+0x6dc>
8020072c:	00000000 	nop
80200730:	8fc20000 	lw	v0,0(s8)
80200734:	00000000 	nop
80200738:	bc510000 	cache	0x11,0(v0)
8020073c:	8fc20000 	lw	v0,0(s8)
80200740:	00000000 	nop
80200744:	24420004 	addiu	v0,v0,4
80200748:	afc20000 	sw	v0,0(s8)
8020074c:	080801c3 	j	8020070c <_end_boot+0x694>
80200750:	00000000 	nop
80200754:	03c0e821 	move	sp,s8
80200758:	8fbe0008 	lw	s8,8(sp)
8020075c:	27bd0010 	addiu	sp,sp,16
80200760:	03e00008 	jr	ra
80200764:	00000000 	nop
rlx_dcache_store():
80200768:	27bdfff0 	addiu	sp,sp,-16
8020076c:	afbe0008 	sw	s8,8(sp)
80200770:	03a0f021 	move	s8,sp
80200774:	afc40010 	sw	a0,16(s8)
80200778:	afc50014 	sw	a1,20(s8)
8020077c:	8fc20010 	lw	v0,16(s8)
80200780:	00000000 	nop
80200784:	afc20000 	sw	v0,0(s8)
80200788:	8fc30010 	lw	v1,16(s8)
8020078c:	8fc20014 	lw	v0,20(s8)
80200790:	00000000 	nop
80200794:	00621821 	addu	v1,v1,v0
80200798:	8fc20000 	lw	v0,0(s8)
8020079c:	00000000 	nop
802007a0:	0043102b 	sltu	v0,v0,v1
802007a4:	1040000a 	beqz	v0,802007d0 <_end_boot+0x758>
802007a8:	00000000 	nop
802007ac:	8fc20000 	lw	v0,0(s8)
802007b0:	00000000 	nop
802007b4:	bc590000 	cache	0x19,0(v0)
802007b8:	8fc20000 	lw	v0,0(s8)
802007bc:	00000000 	nop
802007c0:	24420004 	addiu	v0,v0,4
802007c4:	afc20000 	sw	v0,0(s8)
802007c8:	080801e2 	j	80200788 <_end_boot+0x710>
802007cc:	00000000 	nop
802007d0:	03c0e821 	move	sp,s8
802007d4:	8fbe0008 	lw	s8,8(sp)
802007d8:	27bd0010 	addiu	sp,sp,16
802007dc:	03e00008 	jr	ra
802007e0:	00000000 	nop
rlx_dcache_invalidate_all():
802007e4:	27bdfff0 	addiu	sp,sp,-16
802007e8:	afbe0008 	sw	s8,8(sp)
802007ec:	03a0f021 	move	s8,sp
802007f0:	afc00000 	sw	zero,0(s8)
802007f4:	8fc20000 	lw	v0,0(s8)
802007f8:	00000000 	nop
802007fc:	4082a000 	mtc0	v0,$20
80200800:	00000000 	nop
80200804:	4002a000 	mfc0	v0,$20
80200808:	00000000 	nop
8020080c:	afc20000 	sw	v0,0(s8)
80200810:	8fc20000 	lw	v0,0(s8)
80200814:	00000000 	nop
80200818:	34420001 	ori	v0,v0,0x1
8020081c:	afc20000 	sw	v0,0(s8)
80200820:	8fc20000 	lw	v0,0(s8)
80200824:	00000000 	nop
80200828:	4082a000 	mtc0	v0,$20
8020082c:	03c0e821 	move	sp,s8
80200830:	8fbe0008 	lw	s8,8(sp)
80200834:	27bd0010 	addiu	sp,sp,16
80200838:	03e00008 	jr	ra
8020083c:	00000000 	nop
rlx_icache_invalidate_all():
80200840:	27bdfff0 	addiu	sp,sp,-16
80200844:	afbe0008 	sw	s8,8(sp)
80200848:	03a0f021 	move	s8,sp
8020084c:	afc00000 	sw	zero,0(s8)
80200850:	8fc20000 	lw	v0,0(s8)
80200854:	00000000 	nop
80200858:	4082a000 	mtc0	v0,$20
8020085c:	00000000 	nop
80200860:	4002a000 	mfc0	v0,$20
80200864:	00000000 	nop
80200868:	afc20000 	sw	v0,0(s8)
8020086c:	8fc20000 	lw	v0,0(s8)
80200870:	00000000 	nop
80200874:	34420002 	ori	v0,v0,0x2
80200878:	afc20000 	sw	v0,0(s8)
8020087c:	8fc20000 	lw	v0,0(s8)
80200880:	00000000 	nop
80200884:	4082a000 	mtc0	v0,$20
80200888:	03c0e821 	move	sp,s8
8020088c:	8fbe0008 	lw	s8,8(sp)
80200890:	27bd0010 	addiu	sp,sp,16
80200894:	03e00008 	jr	ra
80200898:	00000000 	nop
rlx_icache_lock():
8020089c:	27bdfff0 	addiu	sp,sp,-16
802008a0:	afbe0008 	sw	s8,8(sp)
802008a4:	03a0f021 	move	s8,sp
802008a8:	afc40010 	sw	a0,16(s8)
802008ac:	a3c00004 	sb	zero,4(s8)
802008b0:	4002a000 	mfc0	v0,$20
802008b4:	00000000 	nop
802008b8:	afc20000 	sw	v0,0(s8)
802008bc:	8fc30000 	lw	v1,0(s8)
802008c0:	2402fff3 	li	v0,-13
802008c4:	00621024 	and	v0,v1,v0
802008c8:	afc20000 	sw	v0,0(s8)
802008cc:	8fc20010 	lw	v0,16(s8)
802008d0:	00000000 	nop
802008d4:	2c420004 	sltiu	v0,v0,4
802008d8:	1040000c 	beqz	v0,8020090c <_end_boot+0x894>
802008dc:	00000000 	nop
802008e0:	24030001 	li	v1,1
802008e4:	8fc20010 	lw	v0,16(s8)
802008e8:	00000000 	nop
802008ec:	00431004 	sllv	v0,v1,v0
802008f0:	3042000d 	andi	v0,v0,0xd
802008f4:	14400003 	bnez	v0,80200904 <_end_boot+0x88c>
802008f8:	00000000 	nop
802008fc:	08080243 	j	8020090c <_end_boot+0x894>
80200900:	00000000 	nop
80200904:	24020001 	li	v0,1
80200908:	a3c20004 	sb	v0,4(s8)
8020090c:	93c30004 	lbu	v1,4(s8)
80200910:	24020001 	li	v0,1
80200914:	1462000b 	bne	v1,v0,80200944 <_end_boot+0x8cc>
80200918:	00000000 	nop
8020091c:	8fc20010 	lw	v0,16(s8)
80200920:	00000000 	nop
80200924:	00021880 	sll	v1,v0,0x2
80200928:	8fc20000 	lw	v0,0(s8)
8020092c:	00000000 	nop
80200930:	00431025 	or	v0,v0,v1
80200934:	afc20000 	sw	v0,0(s8)
80200938:	8fc20000 	lw	v0,0(s8)
8020093c:	00000000 	nop
80200940:	4082a000 	mtc0	v0,$20
80200944:	93c20004 	lbu	v0,4(s8)
80200948:	03c0e821 	move	sp,s8
8020094c:	8fbe0008 	lw	s8,8(sp)
80200950:	27bd0010 	addiu	sp,sp,16
80200954:	03e00008 	jr	ra
80200958:	00000000 	nop
rlx_dmem_enable():
8020095c:	27bdfff0 	addiu	sp,sp,-16
80200960:	afbe0008 	sw	s8,8(sp)
80200964:	03a0f021 	move	s8,sp
80200968:	afc40010 	sw	a0,16(s8)
8020096c:	afc50014 	sw	a1,20(s8)
80200970:	8fc20010 	lw	v0,16(s8)
80200974:	00000000 	nop
80200978:	4c822000 	mtc3	v0,$4
8020097c:	8fc20014 	lw	v0,20(s8)
80200980:	00000000 	nop
80200984:	4c822800 	mtc3	v0,$5
80200988:	00000000 	nop
8020098c:	4002a000 	mfc0	v0,$20
80200990:	00000000 	nop
80200994:	afc20000 	sw	v0,0(s8)
80200998:	8fc20000 	lw	v0,0(s8)
8020099c:	00000000 	nop
802009a0:	34420400 	ori	v0,v0,0x400
802009a4:	afc20000 	sw	v0,0(s8)
802009a8:	8fc20000 	lw	v0,0(s8)
802009ac:	00000000 	nop
802009b0:	4082a000 	mtc0	v0,$20
802009b4:	03c0e821 	move	sp,s8
802009b8:	8fbe0008 	lw	s8,8(sp)
802009bc:	27bd0010 	addiu	sp,sp,16
802009c0:	03e00008 	jr	ra
802009c4:	00000000 	nop
rlx_imem_refill():
802009c8:	27bdfff0 	addiu	sp,sp,-16
802009cc:	afbe0008 	sw	s8,8(sp)
802009d0:	03a0f021 	move	s8,sp
802009d4:	afc40010 	sw	a0,16(s8)
802009d8:	afc50014 	sw	a1,20(s8)
802009dc:	afc60018 	sw	a2,24(s8)
802009e0:	8fc20010 	lw	v0,16(s8)
802009e4:	00000000 	nop
802009e8:	4c820000 	mtc3	v0,$0
802009ec:	8fc20014 	lw	v0,20(s8)
802009f0:	00000000 	nop
802009f4:	4c820800 	mtc3	v0,$1
802009f8:	8fc30018 	lw	v1,24(s8)
802009fc:	24020001 	li	v0,1
80200a00:	14620012 	bne	v1,v0,80200a4c <_end_boot+0x9d4>
80200a04:	00000000 	nop
80200a08:	4002a000 	mfc0	v0,$20
80200a0c:	00000000 	nop
80200a10:	afc20000 	sw	v0,0(s8)
80200a14:	8fc30000 	lw	v1,0(s8)
80200a18:	2402ffef 	li	v0,-17
80200a1c:	00621024 	and	v0,v1,v0
80200a20:	afc20000 	sw	v0,0(s8)
80200a24:	8fc20000 	lw	v0,0(s8)
80200a28:	00000000 	nop
80200a2c:	4082a000 	mtc0	v0,$20
80200a30:	8fc20000 	lw	v0,0(s8)
80200a34:	00000000 	nop
80200a38:	34420010 	ori	v0,v0,0x10
80200a3c:	afc20000 	sw	v0,0(s8)
80200a40:	8fc20000 	lw	v0,0(s8)
80200a44:	00000000 	nop
80200a48:	4082a000 	mtc0	v0,$20
80200a4c:	03c0e821 	move	sp,s8
80200a50:	8fbe0008 	lw	s8,8(sp)
80200a54:	27bd0010 	addiu	sp,sp,16
80200a58:	03e00008 	jr	ra
80200a5c:	00000000 	nop
ssi_wait_nobusy():
80200a60:	27bdfff8 	addiu	sp,sp,-8
80200a64:	afbf0008 	sw	ra,8(sp)
wait_ssi():
80200a68:	8c880028 	lw	t0,40(a0)
80200a6c:	24090004 	li	t1,4
80200a70:	31080005 	andi	t0,t0,0x5
80200a74:	1509fffc 	bne	t0,t1,80200a68 <_end_boot+0x9f0>
80200a78:	00000000 	nop
80200a7c:	00000000 	nop
80200a80:	8fbf0008 	lw	ra,8(sp)
80200a84:	27bd0008 	addiu	sp,sp,8
80200a88:	03e00008 	jr	ra
80200a8c:	00000000 	nop
spi_get_status():
80200a90:	27bdfff8 	addiu	sp,sp,-8
80200a94:	afbf0008 	sw	ra,8(sp)
80200a98:	ac800008 	sw	zero,8(a0)
80200a9c:	240873c7 	li	t0,29639
80200aa0:	ac880000 	sw	t0,0(a0)
80200aa4:	ac800004 	sw	zero,4(a0)
80200aa8:	24080001 	li	t0,1
80200aac:	ac880010 	sw	t0,16(a0)
80200ab0:	24090001 	li	t1,1
80200ab4:	ac890008 	sw	t1,8(a0)
80200ab8:	24090005 	li	t1,5
80200abc:	ac890060 	sw	t1,96(a0)
80200ac0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200ac4:	00000000 	nop
80200ac8:	00000000 	nop
80200acc:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200ad0:	00000000 	nop
80200ad4:	00000000 	nop
80200ad8:	00044025 	or	t0,zero,a0
80200adc:	8c820060 	lw	v0,96(a0)
80200ae0:	00000000 	nop
80200ae4:	8fbf0008 	lw	ra,8(sp)
80200ae8:	27bd0008 	addiu	sp,sp,8
80200aec:	03e00008 	jr	ra
80200af0:	00000000 	nop
spi_chip_erase():
80200af4:	27bdfff8 	addiu	sp,sp,-8
80200af8:	afbf0008 	sw	ra,8(sp)
80200afc:	ac800008 	sw	zero,8(a0)
80200b00:	240871c7 	li	t0,29127
80200b04:	ac880000 	sw	t0,0(a0)
80200b08:	24080001 	li	t0,1
80200b0c:	ac880010 	sw	t0,16(a0)
80200b10:	24080001 	li	t0,1
80200b14:	ac880008 	sw	t0,8(a0)
80200b18:	24080006 	li	t0,6
80200b1c:	ac880060 	sw	t0,96(a0)
80200b20:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200b24:	00000000 	nop
80200b28:	00000000 	nop
80200b2c:	240800c7 	li	t0,199
80200b30:	ac880060 	sw	t0,96(a0)
80200b34:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200b38:	00000000 	nop
80200b3c:	00000000 	nop
80200b40:	0c0803dc 	jal	80200f70 <_end_boot+0xef8>
80200b44:	00000000 	nop
80200b48:	00000000 	nop
80200b4c:	8fbf0008 	lw	ra,8(sp)
80200b50:	27bd0008 	addiu	sp,sp,8
80200b54:	03e00008 	jr	ra
80200b58:	00000000 	nop
spi_blk_erase():
80200b5c:	27bdfff8 	addiu	sp,sp,-8
80200b60:	afbf0008 	sw	ra,8(sp)
80200b64:	ac800008 	sw	zero,8(a0)
80200b68:	240871c7 	li	t0,29127
80200b6c:	ac880000 	sw	t0,0(a0)
80200b70:	24080001 	li	t0,1
80200b74:	ac880010 	sw	t0,16(a0)
80200b78:	24080001 	li	t0,1
80200b7c:	ac880008 	sw	t0,8(a0)
80200b80:	24080006 	li	t0,6
80200b84:	ac880060 	sw	t0,96(a0)
80200b88:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200b8c:	00000000 	nop
80200b90:	00000000 	nop
80200b94:	ac800010 	sw	zero,16(a0)
80200b98:	240800d8 	li	t0,216
80200b9c:	30aa00ff 	andi	t2,a1,0xff
80200ba0:	00052a02 	srl	a1,a1,0x8
80200ba4:	30ab00ff 	andi	t3,a1,0xff
80200ba8:	00052a02 	srl	a1,a1,0x8
80200bac:	30ac00ff 	andi	t4,a1,0xff
80200bb0:	ac880060 	sw	t0,96(a0)
80200bb4:	ac8c0060 	sw	t4,96(a0)
80200bb8:	ac8b0060 	sw	t3,96(a0)
80200bbc:	ac8a0060 	sw	t2,96(a0)
80200bc0:	24080001 	li	t0,1
80200bc4:	ac880010 	sw	t0,16(a0)
80200bc8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200bcc:	00000000 	nop
80200bd0:	00000000 	nop
80200bd4:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200bd8:	00000000 	nop
80200bdc:	00000000 	nop
80200be0:	8fbf0008 	lw	ra,8(sp)
80200be4:	27bd0008 	addiu	sp,sp,8
80200be8:	03e00008 	jr	ra
80200bec:	00000000 	nop
spi_command():
80200bf0:	27bdfff8 	addiu	sp,sp,-8
80200bf4:	afbf0008 	sw	ra,8(sp)
80200bf8:	ac800008 	sw	zero,8(a0)
80200bfc:	240871c7 	li	t0,29127
80200c00:	ac880000 	sw	t0,0(a0)
80200c04:	24080001 	li	t0,1
80200c08:	ac880010 	sw	t0,16(a0)
80200c0c:	24080001 	li	t0,1
80200c10:	ac880008 	sw	t0,8(a0)
80200c14:	24080006 	li	t0,6
80200c18:	ac880060 	sw	t0,96(a0)
80200c1c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200c20:	00000000 	nop
80200c24:	00000000 	nop
80200c28:	ac800010 	sw	zero,16(a0)
80200c2c:	00064020 	add	t0,zero,a2
80200c30:	30aa00ff 	andi	t2,a1,0xff
80200c34:	00052a02 	srl	a1,a1,0x8
80200c38:	30ab00ff 	andi	t3,a1,0xff
80200c3c:	00052a02 	srl	a1,a1,0x8
80200c40:	30ac00ff 	andi	t4,a1,0xff
80200c44:	ac880060 	sw	t0,96(a0)
80200c48:	ac8c0060 	sw	t4,96(a0)
80200c4c:	ac8b0060 	sw	t3,96(a0)
80200c50:	ac8a0060 	sw	t2,96(a0)
80200c54:	24080001 	li	t0,1
80200c58:	ac880010 	sw	t0,16(a0)
80200c5c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200c60:	00000000 	nop
80200c64:	00000000 	nop
80200c68:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200c6c:	00000000 	nop
80200c70:	00000000 	nop
80200c74:	8fbf0008 	lw	ra,8(sp)
80200c78:	27bd0008 	addiu	sp,sp,8
80200c7c:	03e00008 	jr	ra
80200c80:	00000000 	nop
spi_se_unprotect():
80200c84:	27bdfff8 	addiu	sp,sp,-8
80200c88:	afbf0008 	sw	ra,8(sp)
80200c8c:	ac800008 	sw	zero,8(a0)
80200c90:	240871c7 	li	t0,29127
80200c94:	ac880000 	sw	t0,0(a0)
80200c98:	24080001 	li	t0,1
80200c9c:	ac880010 	sw	t0,16(a0)
80200ca0:	24080001 	li	t0,1
80200ca4:	ac880008 	sw	t0,8(a0)
80200ca8:	24080006 	li	t0,6
80200cac:	ac880060 	sw	t0,96(a0)
80200cb0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200cb4:	00000000 	nop
80200cb8:	00000000 	nop
80200cbc:	ac800010 	sw	zero,16(a0)
80200cc0:	24080039 	li	t0,57
80200cc4:	30aa00ff 	andi	t2,a1,0xff
80200cc8:	00052a02 	srl	a1,a1,0x8
80200ccc:	30ab00ff 	andi	t3,a1,0xff
80200cd0:	00052a02 	srl	a1,a1,0x8
80200cd4:	30ac00ff 	andi	t4,a1,0xff
80200cd8:	ac880060 	sw	t0,96(a0)
80200cdc:	ac8c0060 	sw	t4,96(a0)
80200ce0:	ac8b0060 	sw	t3,96(a0)
80200ce4:	ac8a0060 	sw	t2,96(a0)
80200ce8:	24080001 	li	t0,1
80200cec:	ac880010 	sw	t0,16(a0)
80200cf0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200cf4:	00000000 	nop
80200cf8:	00000000 	nop
80200cfc:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200d00:	00000000 	nop
80200d04:	00000000 	nop
80200d08:	8fbf0008 	lw	ra,8(sp)
80200d0c:	27bd0008 	addiu	sp,sp,8
80200d10:	03e00008 	jr	ra
80200d14:	00000000 	nop
spi_se_protect():
80200d18:	27bdfff8 	addiu	sp,sp,-8
80200d1c:	afbf0008 	sw	ra,8(sp)
80200d20:	ac800008 	sw	zero,8(a0)
80200d24:	240871c7 	li	t0,29127
80200d28:	ac880000 	sw	t0,0(a0)
80200d2c:	24080001 	li	t0,1
80200d30:	ac880010 	sw	t0,16(a0)
80200d34:	24080001 	li	t0,1
80200d38:	ac880008 	sw	t0,8(a0)
80200d3c:	24080006 	li	t0,6
80200d40:	ac880060 	sw	t0,96(a0)
80200d44:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200d48:	00000000 	nop
80200d4c:	00000000 	nop
80200d50:	ac800010 	sw	zero,16(a0)
80200d54:	24080036 	li	t0,54
80200d58:	30aa00ff 	andi	t2,a1,0xff
80200d5c:	00052a02 	srl	a1,a1,0x8
80200d60:	30ab00ff 	andi	t3,a1,0xff
80200d64:	00052a02 	srl	a1,a1,0x8
80200d68:	30ac00ff 	andi	t4,a1,0xff
80200d6c:	ac880060 	sw	t0,96(a0)
80200d70:	ac8c0060 	sw	t4,96(a0)
80200d74:	ac8b0060 	sw	t3,96(a0)
80200d78:	ac8a0060 	sw	t2,96(a0)
80200d7c:	24080001 	li	t0,1
80200d80:	ac880010 	sw	t0,16(a0)
80200d84:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200d88:	00000000 	nop
80200d8c:	00000000 	nop
80200d90:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200d94:	00000000 	nop
80200d98:	00000000 	nop
80200d9c:	8fbf0008 	lw	ra,8(sp)
80200da0:	27bd0008 	addiu	sp,sp,8
80200da4:	03e00008 	jr	ra
80200da8:	00000000 	nop
spi_se_erase():
80200dac:	27bdfff8 	addiu	sp,sp,-8
80200db0:	afbf0008 	sw	ra,8(sp)
80200db4:	ac800008 	sw	zero,8(a0)
80200db8:	240871c7 	li	t0,29127
80200dbc:	ac880000 	sw	t0,0(a0)
80200dc0:	24080001 	li	t0,1
80200dc4:	ac880010 	sw	t0,16(a0)
80200dc8:	24080001 	li	t0,1
80200dcc:	ac880008 	sw	t0,8(a0)
80200dd0:	24080006 	li	t0,6
80200dd4:	ac880060 	sw	t0,96(a0)
80200dd8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200ddc:	00000000 	nop
80200de0:	00000000 	nop
80200de4:	ac800010 	sw	zero,16(a0)
80200de8:	24080020 	li	t0,32
80200dec:	30aa00ff 	andi	t2,a1,0xff
80200df0:	00052a02 	srl	a1,a1,0x8
80200df4:	30ab00ff 	andi	t3,a1,0xff
80200df8:	00052a02 	srl	a1,a1,0x8
80200dfc:	30ac00ff 	andi	t4,a1,0xff
80200e00:	ac880060 	sw	t0,96(a0)
80200e04:	ac8c0060 	sw	t4,96(a0)
80200e08:	ac8b0060 	sw	t3,96(a0)
80200e0c:	ac8a0060 	sw	t2,96(a0)
80200e10:	24080001 	li	t0,1
80200e14:	ac880010 	sw	t0,16(a0)
80200e18:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200e1c:	00000000 	nop
80200e20:	00000000 	nop
80200e24:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200e28:	00000000 	nop
80200e2c:	00000000 	nop
80200e30:	8fbf0008 	lw	ra,8(sp)
80200e34:	27bd0008 	addiu	sp,sp,8
80200e38:	03e00008 	jr	ra
80200e3c:	00000000 	nop
spi_wait_busy():
80200e40:	27bdfff8 	addiu	sp,sp,-8
80200e44:	afbf0008 	sw	ra,8(sp)
80200e48:	ac800008 	sw	zero,8(a0)
80200e4c:	ac800004 	sw	zero,4(a0)
80200e50:	240873c7 	li	t0,29639
80200e54:	ac880000 	sw	t0,0(a0)
80200e58:	24080001 	li	t0,1
80200e5c:	ac880010 	sw	t0,16(a0)
80200e60:	24080001 	li	t0,1
80200e64:	ac880008 	sw	t0,8(a0)
__checkstat2():
80200e68:	24080005 	li	t0,5
80200e6c:	ac880060 	sw	t0,96(a0)
80200e70:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200e74:	00000000 	nop
80200e78:	00000000 	nop
80200e7c:	8c880060 	lw	t0,96(a0)
80200e80:	00000000 	nop
80200e84:	31080001 	andi	t0,t0,0x1
80200e88:	1500fff7 	bnez	t0,80200e68 <_end_boot+0xdf0>
80200e8c:	00000000 	nop
80200e90:	00000000 	nop
80200e94:	8fbf0008 	lw	ra,8(sp)
80200e98:	27bd0008 	addiu	sp,sp,8
80200e9c:	03e00008 	jr	ra
80200ea0:	00000000 	nop
spi_set_status():
80200ea4:	27bdfff8 	addiu	sp,sp,-8
80200ea8:	afbf0008 	sw	ra,8(sp)
80200eac:	ac800008 	sw	zero,8(a0)
80200eb0:	240871c7 	li	t0,29127
80200eb4:	ac880000 	sw	t0,0(a0)
80200eb8:	24080001 	li	t0,1
80200ebc:	ac880010 	sw	t0,16(a0)
80200ec0:	24080001 	li	t0,1
80200ec4:	ac880008 	sw	t0,8(a0)
80200ec8:	24080006 	li	t0,6
80200ecc:	ac880060 	sw	t0,96(a0)
80200ed0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200ed4:	00000000 	nop
80200ed8:	00000000 	nop
80200edc:	ac800010 	sw	zero,16(a0)
80200ee0:	24080001 	li	t0,1
80200ee4:	ac880060 	sw	t0,96(a0)
80200ee8:	ac850060 	sw	a1,96(a0)
80200eec:	24080001 	li	t0,1
80200ef0:	ac880010 	sw	t0,16(a0)
80200ef4:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200ef8:	00000000 	nop
80200efc:	00000000 	nop
80200f00:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200f04:	00000000 	nop
80200f08:	00000000 	nop
80200f0c:	8fbf0008 	lw	ra,8(sp)
80200f10:	27bd0008 	addiu	sp,sp,8
80200f14:	03e00008 	jr	ra
80200f18:	00000000 	nop
spi_dis_write():
80200f1c:	27bdfff8 	addiu	sp,sp,-8
80200f20:	afbf0008 	sw	ra,8(sp)
80200f24:	ac800008 	sw	zero,8(a0)
80200f28:	240871c7 	li	t0,29127
80200f2c:	ac880000 	sw	t0,0(a0)
80200f30:	24080001 	li	t0,1
80200f34:	ac880010 	sw	t0,16(a0)
80200f38:	24080001 	li	t0,1
80200f3c:	ac880008 	sw	t0,8(a0)
80200f40:	24080004 	li	t0,4
80200f44:	ac880060 	sw	t0,96(a0)
80200f48:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200f4c:	00000000 	nop
80200f50:	00000000 	nop
80200f54:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200f58:	00000000 	nop
80200f5c:	00000000 	nop
80200f60:	8fbf0008 	lw	ra,8(sp)
80200f64:	27bd0008 	addiu	sp,sp,8
80200f68:	03e00008 	jr	ra
80200f6c:	00000000 	nop
spi_enable_write():
80200f70:	27bdfff8 	addiu	sp,sp,-8
80200f74:	afbf0008 	sw	ra,8(sp)
80200f78:	ac800008 	sw	zero,8(a0)
80200f7c:	240871c7 	li	t0,29127
80200f80:	ac880000 	sw	t0,0(a0)
80200f84:	24080001 	li	t0,1
80200f88:	ac880010 	sw	t0,16(a0)
80200f8c:	24080001 	li	t0,1
80200f90:	ac880008 	sw	t0,8(a0)
80200f94:	24080006 	li	t0,6
80200f98:	ac880060 	sw	t0,96(a0)
80200f9c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200fa0:	00000000 	nop
80200fa4:	00000000 	nop
80200fa8:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80200fac:	00000000 	nop
80200fb0:	00000000 	nop
80200fb4:	8fbf0008 	lw	ra,8(sp)
80200fb8:	27bd0008 	addiu	sp,sp,8
80200fbc:	03e00008 	jr	ra
80200fc0:	00000000 	nop
spi_set_protect_mem():
80200fc4:	27bdfff8 	addiu	sp,sp,-8
80200fc8:	afbf0008 	sw	ra,8(sp)
80200fcc:	0c0802a4 	jal	80200a90 <_end_boot+0xa18>
80200fd0:	00000000 	nop
80200fd4:	00000000 	nop
80200fd8:	00025825 	or	t3,zero,v0
80200fdc:	ac800008 	sw	zero,8(a0)
80200fe0:	240871c7 	li	t0,29127
80200fe4:	ac880000 	sw	t0,0(a0)
80200fe8:	24080001 	li	t0,1
80200fec:	ac880008 	sw	t0,8(a0)
80200ff0:	24080006 	li	t0,6
80200ff4:	ac880060 	sw	t0,96(a0)
80200ff8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80200ffc:	00000000 	nop
80201000:	00000000 	nop
80201004:	ac800010 	sw	zero,16(a0)
80201008:	24080001 	li	t0,1
8020100c:	240a00e3 	li	t2,227
80201010:	016a5824 	and	t3,t3,t2
80201014:	00055080 	sll	t2,a1,0x2
80201018:	016a5025 	or	t2,t3,t2
8020101c:	ac880060 	sw	t0,96(a0)
80201020:	ac8a0060 	sw	t2,96(a0)
80201024:	ac880010 	sw	t0,16(a0)
80201028:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
8020102c:	00000000 	nop
80201030:	00000000 	nop
80201034:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80201038:	00000000 	nop
8020103c:	00000000 	nop
80201040:	8fbf0008 	lw	ra,8(sp)
80201044:	27bd0008 	addiu	sp,sp,8
80201048:	03e00008 	jr	ra
8020104c:	00000000 	nop
spi_read_id():
80201050:	27bdfff8 	addiu	sp,sp,-8
80201054:	afbf0008 	sw	ra,8(sp)
80201058:	ac800008 	sw	zero,8(a0)
8020105c:	240873c7 	li	t0,29639
80201060:	ac880000 	sw	t0,0(a0)
80201064:	24080001 	li	t0,1
80201068:	ac880010 	sw	t0,16(a0)
8020106c:	24080002 	li	t0,2
80201070:	ac880004 	sw	t0,4(a0)
80201074:	24080001 	li	t0,1
80201078:	ac880008 	sw	t0,8(a0)
8020107c:	2408009f 	li	t0,159
80201080:	ac880060 	sw	t0,96(a0)
80201084:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201088:	00000000 	nop
8020108c:	00000000 	nop
80201090:	8c820060 	lw	v0,96(a0)
80201094:	00000000 	nop
80201098:	304200ff 	andi	v0,v0,0xff
8020109c:	00021200 	sll	v0,v0,0x8
802010a0:	8c880060 	lw	t0,96(a0)
802010a4:	00000000 	nop
802010a8:	310800ff 	andi	t0,t0,0xff
802010ac:	00481025 	or	v0,v0,t0
802010b0:	00021200 	sll	v0,v0,0x8
802010b4:	8c880060 	lw	t0,96(a0)
802010b8:	00000000 	nop
802010bc:	310800ff 	andi	t0,t0,0xff
802010c0:	00481025 	or	v0,v0,t0
802010c4:	8fbf0008 	lw	ra,8(sp)
802010c8:	27bd0008 	addiu	sp,sp,8
802010cc:	03e00008 	jr	ra
802010d0:	00000000 	nop
spi_read_ect_id():
802010d4:	27bdfff8 	addiu	sp,sp,-8
802010d8:	afbf0008 	sw	ra,8(sp)
802010dc:	ac800008 	sw	zero,8(a0)
802010e0:	240873c7 	li	t0,29639
802010e4:	ac880000 	sw	t0,0(a0)
802010e8:	24080000 	li	t0,0
802010ec:	ac880004 	sw	t0,4(a0)
802010f0:	24080001 	li	t0,1
802010f4:	ac880008 	sw	t0,8(a0)
802010f8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802010fc:	00000000 	nop
80201100:	00000000 	nop
80201104:	24080000 	li	t0,0
80201108:	ac880010 	sw	t0,16(a0)
8020110c:	240800ab 	li	t0,171
80201110:	ac880060 	sw	t0,96(a0)
80201114:	a0890060 	sb	t1,96(a0)
80201118:	a08a0060 	sb	t2,96(a0)
8020111c:	a08b0060 	sb	t3,96(a0)
80201120:	24080001 	li	t0,1
80201124:	ac880010 	sw	t0,16(a0)
80201128:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
8020112c:	00000000 	nop
80201130:	00000000 	nop
80201134:	8c820060 	lw	v0,96(a0)
80201138:	00000000 	nop
8020113c:	304200ff 	andi	v0,v0,0xff
80201140:	8fbf0008 	lw	ra,8(sp)
80201144:	27bd0008 	addiu	sp,sp,8
80201148:	03e00008 	jr	ra
8020114c:	00000000 	nop
spi_read_mf_id():
80201150:	27bdfff8 	addiu	sp,sp,-8
80201154:	afbf0008 	sw	ra,8(sp)
80201158:	ac800008 	sw	zero,8(a0)
8020115c:	240873c7 	li	t0,29639
80201160:	ac880000 	sw	t0,0(a0)
80201164:	24080002 	li	t0,2
80201168:	ac880004 	sw	t0,4(a0)
8020116c:	24080001 	li	t0,1
80201170:	ac880008 	sw	t0,8(a0)
80201174:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201178:	00000000 	nop
8020117c:	00000000 	nop
80201180:	ac800010 	sw	zero,16(a0)
80201184:	24080090 	li	t0,144
80201188:	ac880060 	sw	t0,96(a0)
8020118c:	a0890060 	sb	t1,96(a0)
80201190:	a08a0060 	sb	t2,96(a0)
80201194:	ac850060 	sw	a1,96(a0)
80201198:	24080001 	li	t0,1
8020119c:	ac880010 	sw	t0,16(a0)
802011a0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802011a4:	00000000 	nop
802011a8:	00000000 	nop
802011ac:	8c820060 	lw	v0,96(a0)
802011b0:	00000000 	nop
802011b4:	304200ff 	andi	v0,v0,0xff
802011b8:	00021200 	sll	v0,v0,0x8
802011bc:	8c880060 	lw	t0,96(a0)
802011c0:	00000000 	nop
802011c4:	310800ff 	andi	t0,t0,0xff
802011c8:	00481025 	or	v0,v0,t0
802011cc:	8fbf0008 	lw	ra,8(sp)
802011d0:	27bd0008 	addiu	sp,sp,8
802011d4:	03e00008 	jr	ra
802011d8:	00000000 	nop
spi_enter_dp():
802011dc:	27bdfff8 	addiu	sp,sp,-8
802011e0:	afbf0008 	sw	ra,8(sp)
802011e4:	ac800008 	sw	zero,8(a0)
802011e8:	240871c7 	li	t0,29127
802011ec:	ac880000 	sw	t0,0(a0)
802011f0:	24080001 	li	t0,1
802011f4:	ac880010 	sw	t0,16(a0)
802011f8:	24080001 	li	t0,1
802011fc:	ac880008 	sw	t0,8(a0)
80201200:	240800b9 	li	t0,185
80201204:	ac880060 	sw	t0,96(a0)
80201208:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
8020120c:	00000000 	nop
80201210:	00000000 	nop
80201214:	8fbf0008 	lw	ra,8(sp)
80201218:	27bd0008 	addiu	sp,sp,8
8020121c:	03e00008 	jr	ra
80201220:	00000000 	nop
spi_release_dp():
80201224:	27bdfff8 	addiu	sp,sp,-8
80201228:	afbf0008 	sw	ra,8(sp)
8020122c:	ac800008 	sw	zero,8(a0)
80201230:	240871c7 	li	t0,29127
80201234:	ac880000 	sw	t0,0(a0)
80201238:	24080001 	li	t0,1
8020123c:	ac880010 	sw	t0,16(a0)
80201240:	24080001 	li	t0,1
80201244:	ac880008 	sw	t0,8(a0)
80201248:	240800ab 	li	t0,171
8020124c:	ac880060 	sw	t0,96(a0)
80201250:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201254:	00000000 	nop
80201258:	00000000 	nop
8020125c:	8fbf0008 	lw	ra,8(sp)
80201260:	27bd0008 	addiu	sp,sp,8
80201264:	03e00008 	jr	ra
80201268:	00000000 	nop
spi_write_byte():
8020126c:	27bdfff8 	addiu	sp,sp,-8
80201270:	afbf0008 	sw	ra,8(sp)
80201274:	ac800008 	sw	zero,8(a0)
80201278:	240871c7 	li	t0,29127
8020127c:	ac880000 	sw	t0,0(a0)
80201280:	24080001 	li	t0,1
80201284:	ac880010 	sw	t0,16(a0)
80201288:	24080001 	li	t0,1
8020128c:	ac880008 	sw	t0,8(a0)
80201290:	24080006 	li	t0,6
80201294:	ac880060 	sw	t0,96(a0)
80201298:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
8020129c:	00000000 	nop
802012a0:	00000000 	nop
802012a4:	ac800010 	sw	zero,16(a0)
802012a8:	24080002 	li	t0,2
802012ac:	30aa00ff 	andi	t2,a1,0xff
802012b0:	00052a02 	srl	a1,a1,0x8
802012b4:	30ab00ff 	andi	t3,a1,0xff
802012b8:	00052a02 	srl	a1,a1,0x8
802012bc:	30ac00ff 	andi	t4,a1,0xff
802012c0:	30c600ff 	andi	a2,a2,0xff
802012c4:	ac880060 	sw	t0,96(a0)
802012c8:	ac8c0060 	sw	t4,96(a0)
802012cc:	ac8b0060 	sw	t3,96(a0)
802012d0:	ac8a0060 	sw	t2,96(a0)
802012d4:	ac860060 	sw	a2,96(a0)
802012d8:	24080001 	li	t0,1
802012dc:	ac880010 	sw	t0,16(a0)
802012e0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802012e4:	00000000 	nop
802012e8:	00000000 	nop
802012ec:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
802012f0:	00000000 	nop
802012f4:	00000000 	nop
802012f8:	8fbf0008 	lw	ra,8(sp)
802012fc:	27bd0008 	addiu	sp,sp,8
80201300:	03e00008 	jr	ra
80201304:	00000000 	nop
spi_read_byte():
80201308:	27bdfff8 	addiu	sp,sp,-8
8020130c:	afbf0008 	sw	ra,8(sp)
80201310:	ac800008 	sw	zero,8(a0)
80201314:	240873c7 	li	t0,29639
80201318:	ac880000 	sw	t0,0(a0)
8020131c:	24080000 	li	t0,0
80201320:	ac880004 	sw	t0,4(a0)
80201324:	24080001 	li	t0,1
80201328:	ac880008 	sw	t0,8(a0)
8020132c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201330:	00000000 	nop
80201334:	00000000 	nop
80201338:	24080003 	li	t0,3
8020133c:	30aa00ff 	andi	t2,a1,0xff
80201340:	00052a02 	srl	a1,a1,0x8
80201344:	30ab00ff 	andi	t3,a1,0xff
80201348:	00052a02 	srl	a1,a1,0x8
8020134c:	30ac00ff 	andi	t4,a1,0xff
80201350:	ac800010 	sw	zero,16(a0)
80201354:	ac880060 	sw	t0,96(a0)
80201358:	ac8c0060 	sw	t4,96(a0)
8020135c:	ac8b0060 	sw	t3,96(a0)
80201360:	ac8a0060 	sw	t2,96(a0)
80201364:	24080001 	li	t0,1
80201368:	ac880010 	sw	t0,16(a0)
8020136c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201370:	00000000 	nop
80201374:	00000000 	nop
80201378:	8c900060 	lw	s0,96(a0)
8020137c:	00000000 	nop
80201380:	00101025 	or	v0,zero,s0
80201384:	8fbf0008 	lw	ra,8(sp)
80201388:	27bd0008 	addiu	sp,sp,8
8020138c:	03e00008 	jr	ra
80201390:	00000000 	nop
spi_fastread_byte():
80201394:	27bdfff8 	addiu	sp,sp,-8
80201398:	afbf0008 	sw	ra,8(sp)
8020139c:	ac800008 	sw	zero,8(a0)
802013a0:	240873c7 	li	t0,29639
802013a4:	ac880000 	sw	t0,0(a0)
802013a8:	24080000 	li	t0,0
802013ac:	ac880004 	sw	t0,4(a0)
802013b0:	24080001 	li	t0,1
802013b4:	ac880008 	sw	t0,8(a0)
802013b8:	ac800010 	sw	zero,16(a0)
802013bc:	30aa00ff 	andi	t2,a1,0xff
802013c0:	00052a02 	srl	a1,a1,0x8
802013c4:	30ab00ff 	andi	t3,a1,0xff
802013c8:	00052a02 	srl	a1,a1,0x8
802013cc:	30ac00ff 	andi	t4,a1,0xff
byte_raddr():
802013d0:	2408000b 	li	t0,11
802013d4:	ac880060 	sw	t0,96(a0)
802013d8:	ac8c0060 	sw	t4,96(a0)
802013dc:	ac8b0060 	sw	t3,96(a0)
802013e0:	ac8a0060 	sw	t2,96(a0)
802013e4:	ac890060 	sw	t1,96(a0)
802013e8:	24080001 	li	t0,1
802013ec:	ac880010 	sw	t0,16(a0)
802013f0:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802013f4:	00000000 	nop
802013f8:	00000000 	nop
802013fc:	8c900060 	lw	s0,96(a0)
80201400:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201404:	00000000 	nop
80201408:	00000000 	nop
8020140c:	00101025 	or	v0,zero,s0
80201410:	8fbf0008 	lw	ra,8(sp)
80201414:	27bd0008 	addiu	sp,sp,8
80201418:	03e00008 	jr	ra
8020141c:	00000000 	nop
spi_write():
80201420:	27bdfff8 	addiu	sp,sp,-8
80201424:	afbf0008 	sw	ra,8(sp)
80201428:	ac800008 	sw	zero,8(a0)
8020142c:	240871c7 	li	t0,29127
80201430:	ac880000 	sw	t0,0(a0)
80201434:	24080001 	li	t0,1
80201438:	ac880010 	sw	t0,16(a0)
8020143c:	24080001 	li	t0,1
80201440:	ac880008 	sw	t0,8(a0)
80201444:	24080006 	li	t0,6
80201448:	ac880060 	sw	t0,96(a0)
8020144c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201450:	00000000 	nop
80201454:	00000000 	nop
80201458:	ac800010 	sw	zero,16(a0)
8020145c:	24080002 	li	t0,2
80201460:	30aa00ff 	andi	t2,a1,0xff
80201464:	00052a02 	srl	a1,a1,0x8
80201468:	30ab00ff 	andi	t3,a1,0xff
8020146c:	00052a02 	srl	a1,a1,0x8
80201470:	30ac00ff 	andi	t4,a1,0xff
80201474:	ac880060 	sw	t0,96(a0)
80201478:	ac8c0060 	sw	t4,96(a0)
8020147c:	ac8b0060 	sw	t3,96(a0)
80201480:	ac8a0060 	sw	t2,96(a0)
80201484:	00c74020 	add	t0,a2,a3
loop():
80201488:	80ca0000 	lb	t2,0(a2)
8020148c:	20c60001 	addi	a2,a2,1
80201490:	ac8a0060 	sw	t2,96(a0)
80201494:	1506fffc 	bne	t0,a2,80201488 <_end_boot+0x1410>
80201498:	00000000 	nop
8020149c:	24080001 	li	t0,1
802014a0:	ac880010 	sw	t0,16(a0)
802014a4:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802014a8:	00000000 	nop
802014ac:	00000000 	nop
802014b0:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
802014b4:	00000000 	nop
802014b8:	00000000 	nop
802014bc:	8fbf0008 	lw	ra,8(sp)
802014c0:	27bd0008 	addiu	sp,sp,8
802014c4:	03e00008 	jr	ra
802014c8:	00000000 	nop
spi_write_word():
802014cc:	27bdffe0 	addiu	sp,sp,-32
802014d0:	afbf0010 	sw	ra,16(sp)
802014d4:	ac800008 	sw	zero,8(a0)
802014d8:	240871c7 	li	t0,29127
802014dc:	ac880000 	sw	t0,0(a0)
802014e0:	24080001 	li	t0,1
802014e4:	ac880010 	sw	t0,16(a0)
802014e8:	24080001 	li	t0,1
802014ec:	ac880008 	sw	t0,8(a0)
word_waddr():
802014f0:	24080006 	li	t0,6
802014f4:	ac880060 	sw	t0,96(a0)
802014f8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802014fc:	00000000 	nop
80201500:	00000000 	nop
80201504:	ac800010 	sw	zero,16(a0)
80201508:	30aa00ff 	andi	t2,a1,0xff
8020150c:	00056a02 	srl	t5,a1,0x8
80201510:	31ab00ff 	andi	t3,t5,0xff
80201514:	00056a02 	srl	t5,a1,0x8
80201518:	31ac00ff 	andi	t4,t5,0xff
8020151c:	24090002 	li	t1,2
80201520:	ac890060 	sw	t1,96(a0)
80201524:	ac8c0060 	sw	t4,96(a0)
80201528:	ac8b0060 	sw	t3,96(a0)
8020152c:	ac8a0060 	sw	t2,96(a0)
80201530:	30cd00ff 	andi	t5,a2,0xff
80201534:	ac8d0060 	sw	t5,96(a0)
80201538:	00063202 	srl	a2,a2,0x8
8020153c:	30cd00ff 	andi	t5,a2,0xff
80201540:	ac8d0060 	sw	t5,96(a0)
80201544:	00063202 	srl	a2,a2,0x8
80201548:	30cd00ff 	andi	t5,a2,0xff
8020154c:	ac8d0060 	sw	t5,96(a0)
80201550:	00063202 	srl	a2,a2,0x8
80201554:	30cd00ff 	andi	t5,a2,0xff
80201558:	ac8d0060 	sw	t5,96(a0)
8020155c:	24080001 	li	t0,1
80201560:	ac880010 	sw	t0,16(a0)
80201564:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201568:	00000000 	nop
8020156c:	00000000 	nop
80201570:	0c080390 	jal	80200e40 <_end_boot+0xdc8>
80201574:	00000000 	nop
80201578:	00000000 	nop
8020157c:	8fbf0010 	lw	ra,16(sp)
80201580:	27bd0020 	addiu	sp,sp,32
80201584:	03e00008 	jr	ra
80201588:	00000000 	nop
spi_read_word():
8020158c:	27bdffe0 	addiu	sp,sp,-32
80201590:	afbf0008 	sw	ra,8(sp)
80201594:	afb00010 	sw	s0,16(sp)
80201598:	ac800008 	sw	zero,8(a0)
8020159c:	240873c7 	li	t0,29639
802015a0:	ac880000 	sw	t0,0(a0)
802015a4:	24080003 	li	t0,3
802015a8:	ac880004 	sw	t0,4(a0)
802015ac:	24080001 	li	t0,1
802015b0:	ac880008 	sw	t0,8(a0)
word_raddr():
802015b4:	ac800010 	sw	zero,16(a0)
802015b8:	30aa00ff 	andi	t2,a1,0xff
802015bc:	00052a02 	srl	a1,a1,0x8
802015c0:	30ab00ff 	andi	t3,a1,0xff
802015c4:	00052a02 	srl	a1,a1,0x8
802015c8:	30ac00ff 	andi	t4,a1,0xff
802015cc:	24090003 	li	t1,3
802015d0:	ac890060 	sw	t1,96(a0)
802015d4:	ac8c0060 	sw	t4,96(a0)
802015d8:	ac8b0060 	sw	t3,96(a0)
802015dc:	ac8a0060 	sw	t2,96(a0)
802015e0:	24080001 	li	t0,1
802015e4:	ac880010 	sw	t0,16(a0)
802015e8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802015ec:	00000000 	nop
802015f0:	00000000 	nop
802015f4:	8c900060 	lw	s0,96(a0)
802015f8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802015fc:	00000000 	nop
80201600:	00000000 	nop
80201604:	8c8c0060 	lw	t4,96(a0)
80201608:	00000000 	nop
8020160c:	000c6200 	sll	t4,t4,0x8
80201610:	01908025 	or	s0,t4,s0
80201614:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201618:	00000000 	nop
8020161c:	00000000 	nop
80201620:	8c8c0060 	lw	t4,96(a0)
80201624:	00000000 	nop
80201628:	000c6400 	sll	t4,t4,0x10
8020162c:	01908025 	or	s0,t4,s0
80201630:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201634:	00000000 	nop
80201638:	00000000 	nop
8020163c:	8c8c0060 	lw	t4,96(a0)
80201640:	00000000 	nop
80201644:	000c6600 	sll	t4,t4,0x18
80201648:	01908025 	or	s0,t4,s0
8020164c:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201650:	00000000 	nop
80201654:	00000000 	nop
80201658:	00101025 	or	v0,zero,s0
8020165c:	8fb00010 	lw	s0,16(sp)
80201660:	8fbf0008 	lw	ra,8(sp)
80201664:	27bd0020 	addiu	sp,sp,32
80201668:	03e00008 	jr	ra
8020166c:	00000000 	nop
spi_fastread_word():
80201670:	27bdfff8 	addiu	sp,sp,-8
80201674:	afbf0008 	sw	ra,8(sp)
80201678:	ac800008 	sw	zero,8(a0)
8020167c:	240873c7 	li	t0,29639
80201680:	ac880000 	sw	t0,0(a0)
80201684:	24080003 	li	t0,3
80201688:	ac880004 	sw	t0,4(a0)
8020168c:	24080001 	li	t0,1
80201690:	ac880008 	sw	t0,8(a0)
80201694:	ac800010 	sw	zero,16(a0)
80201698:	30aa00ff 	andi	t2,a1,0xff
8020169c:	00052a02 	srl	a1,a1,0x8
802016a0:	30ab00ff 	andi	t3,a1,0xff
802016a4:	00052a02 	srl	a1,a1,0x8
802016a8:	30ac00ff 	andi	t4,a1,0xff
word_fraddr():
802016ac:	2408000b 	li	t0,11
802016b0:	ac880060 	sw	t0,96(a0)
802016b4:	ac8c0060 	sw	t4,96(a0)
802016b8:	ac8b0060 	sw	t3,96(a0)
802016bc:	ac8a0060 	sw	t2,96(a0)
802016c0:	ac890060 	sw	t1,96(a0)
802016c4:	24080001 	li	t0,1
802016c8:	ac880010 	sw	t0,16(a0)
802016cc:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802016d0:	00000000 	nop
802016d4:	00000000 	nop
802016d8:	8c900060 	lw	s0,96(a0)
802016dc:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802016e0:	00000000 	nop
802016e4:	00000000 	nop
802016e8:	8c8c0060 	lw	t4,96(a0)
802016ec:	00000000 	nop
802016f0:	000c6200 	sll	t4,t4,0x8
802016f4:	01908025 	or	s0,t4,s0
802016f8:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
802016fc:	00000000 	nop
80201700:	00000000 	nop
80201704:	8c8c0060 	lw	t4,96(a0)
80201708:	00000000 	nop
8020170c:	000c6400 	sll	t4,t4,0x10
80201710:	01908025 	or	s0,t4,s0
80201714:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201718:	00000000 	nop
8020171c:	00000000 	nop
80201720:	8c8c0060 	lw	t4,96(a0)
80201724:	00000000 	nop
80201728:	000c6600 	sll	t4,t4,0x18
8020172c:	01908025 	or	s0,t4,s0
80201730:	0c080298 	jal	80200a60 <_end_boot+0x9e8>
80201734:	00000000 	nop
80201738:	00000000 	nop
8020173c:	00101025 	or	v0,zero,s0
80201740:	8fbf0008 	lw	ra,8(sp)
80201744:	27bd0008 	addiu	sp,sp,8
80201748:	03e00008 	jr	ra
8020174c:	00000000 	nop
memcmp():
80201750:	10c0000c 	beqz	a2,80201784 <_end_boot+0x170c>
80201754:	24c3ffff 	addiu	v1,a2,-1
80201758:	2407ffff 	li	a3,-1
8020175c:	90860000 	lbu	a2,0(a0)
80201760:	90a20000 	lbu	v0,0(a1)
80201764:	2463ffff 	addiu	v1,v1,-1
80201768:	24840001 	addiu	a0,a0,1
8020176c:	10c20003 	beq	a2,v0,8020177c <_end_boot+0x1704>
80201770:	24a50001 	addiu	a1,a1,1
80201774:	03e00008 	jr	ra
80201778:	00c21023 	subu	v0,a2,v0
8020177c:	1467fff7 	bne	v1,a3,8020175c <_end_boot+0x16e4>
80201780:	00000000 	nop
80201784:	03e00008 	jr	ra
80201788:	00001021 	move	v0,zero
memset():
8020178c:	2cc20010 	sltiu	v0,a2,16
80201790:	14400020 	bnez	v0,80201814 <_end_boot+0x179c>
80201794:	00803821 	move	a3,a0
80201798:	30830003 	andi	v1,a0,0x3
8020179c:	10600008 	beqz	v1,802017c0 <_end_boot+0x1748>
802017a0:	2c620004 	sltiu	v0,v1,4
802017a4:	10400006 	beqz	v0,802017c0 <_end_boot+0x1748>
802017a8:	24630001 	addiu	v1,v1,1
802017ac:	a0e50000 	sb	a1,0(a3)
802017b0:	2c620004 	sltiu	v0,v1,4
802017b4:	24c6ffff 	addiu	a2,a2,-1
802017b8:	080805e9 	j	802017a4 <_end_boot+0x172c>
802017bc:	24e70001 	addiu	a3,a3,1
802017c0:	30a500ff 	andi	a1,a1,0xff
802017c4:	10a00005 	beqz	a1,802017dc <_end_boot+0x1764>
802017c8:	00a01821 	move	v1,a1
802017cc:	00051200 	sll	v0,a1,0x8
802017d0:	00a21825 	or	v1,a1,v0
802017d4:	00031400 	sll	v0,v1,0x10
802017d8:	00621825 	or	v1,v1,v0
802017dc:	000610c2 	srl	v0,a2,0x3
802017e0:	30c60007 	andi	a2,a2,0x7
802017e4:	10400005 	beqz	v0,802017fc <_end_boot+0x1784>
802017e8:	2442ffff 	addiu	v0,v0,-1
802017ec:	ace30000 	sw	v1,0(a3)
802017f0:	ace30004 	sw	v1,4(a3)
802017f4:	080805f9 	j	802017e4 <_end_boot+0x176c>
802017f8:	24e70008 	addiu	a3,a3,8
802017fc:	2cc20004 	sltiu	v0,a2,4
80201800:	14400004 	bnez	v0,80201814 <_end_boot+0x179c>
80201804:	00000000 	nop
80201808:	ace30000 	sw	v1,0(a3)
8020180c:	24c6fffc 	addiu	a2,a2,-4
80201810:	24e70004 	addiu	a3,a3,4
80201814:	10c00004 	beqz	a2,80201828 <_end_boot+0x17b0>
80201818:	24c6ffff 	addiu	a2,a2,-1
8020181c:	a0e50000 	sb	a1,0(a3)
80201820:	08080605 	j	80201814 <_end_boot+0x179c>
80201824:	24e70001 	addiu	a3,a3,1
80201828:	03e00008 	jr	ra
8020182c:	00801021 	move	v0,a0
Disassembly of section .data:

80201830 <__data_begin>:
__data_begin():
80201830:	00000000 	nop
Disassembly of section .bss:

80201840 <__bss_start>:
	...

80201c40 <_stack_hi>:
	...
Disassembly of section .reginfo:

00000000 <.reginfo>:
   0:	e00000fc 	sc	zero,252(zero)
	...
  14:	80201840 	lb	zero,6208(at)
Disassembly of section .pdr:

00000000 <.pdr>:
   0:	80200078 	lb	zero,120(at)
   4:	40000000 	mfc0	zero,$0
   8:	fffffff8 	sd	ra,-8(ra)
	...
  14:	00000008 	jr	zero
  18:	0000001e 	ddiv	zero,zero,zero
  1c:	0000001f 	ddivu	zero,zero,zero
  20:	802000d4 	lb	zero,212(at)
  24:	40000000 	mfc0	zero,$0
  28:	fffffff8 	sd	ra,-8(ra)
	...
  34:	00000008 	jr	zero
  38:	0000001e 	ddiv	zero,zero,zero
  3c:	0000001f 	ddivu	zero,zero,zero
  40:	802000f4 	lb	zero,244(at)
  44:	40000000 	mfc0	zero,$0
  48:	fffffff8 	sd	ra,-8(ra)
	...
  54:	00000008 	jr	zero
  58:	0000001e 	ddiv	zero,zero,zero
  5c:	0000001f 	ddivu	zero,zero,zero
  60:	80200114 	lb	zero,276(at)
  64:	40000000 	mfc0	zero,$0
  68:	fffffff8 	sd	ra,-8(ra)
	...
  74:	00000008 	jr	zero
  78:	0000001e 	ddiv	zero,zero,zero
  7c:	0000001f 	ddivu	zero,zero,zero
  80:	80200134 	lb	zero,308(at)
  84:	40000000 	mfc0	zero,$0
  88:	fffffff8 	sd	ra,-8(ra)
	...
  94:	00000008 	jr	zero
  98:	0000001e 	ddiv	zero,zero,zero
  9c:	0000001f 	ddivu	zero,zero,zero
  a0:	80200154 	lb	zero,340(at)
  a4:	40000000 	mfc0	zero,$0
  a8:	fffffff8 	sd	ra,-8(ra)
	...
  b4:	00000008 	jr	zero
  b8:	0000001e 	ddiv	zero,zero,zero
  bc:	0000001f 	ddivu	zero,zero,zero
  c0:	80200174 	lb	zero,372(at)
  c4:	c0000000 	ll	zero,0(zero)
  c8:	fffffffc 	sd	ra,-4(ra)
	...
  d4:	00000028 	0x28
  d8:	0000001e 	ddiv	zero,zero,zero
  dc:	0000001f 	ddivu	zero,zero,zero
  e0:	80200670 	lb	zero,1648(at)
  e4:	40000000 	mfc0	zero,$0
  e8:	fffffff8 	sd	ra,-8(ra)
	...
  f4:	00000010 	mfhi	zero
  f8:	0000001e 	ddiv	zero,zero,zero
  fc:	0000001f 	ddivu	zero,zero,zero
 100:	802006ec 	lb	zero,1772(at)
 104:	40000000 	mfc0	zero,$0
 108:	fffffff8 	sd	ra,-8(ra)
	...
 114:	00000010 	mfhi	zero
 118:	0000001e 	ddiv	zero,zero,zero
 11c:	0000001f 	ddivu	zero,zero,zero
 120:	80200768 	lb	zero,1896(at)
 124:	40000000 	mfc0	zero,$0
 128:	fffffff8 	sd	ra,-8(ra)
	...
 134:	00000010 	mfhi	zero
 138:	0000001e 	ddiv	zero,zero,zero
 13c:	0000001f 	ddivu	zero,zero,zero
 140:	802007e4 	lb	zero,2020(at)
 144:	40000000 	mfc0	zero,$0
 148:	fffffff8 	sd	ra,-8(ra)
	...
 154:	00000010 	mfhi	zero
 158:	0000001e 	ddiv	zero,zero,zero
 15c:	0000001f 	ddivu	zero,zero,zero
 160:	80200840 	lb	zero,2112(at)
 164:	40000000 	mfc0	zero,$0
 168:	fffffff8 	sd	ra,-8(ra)
	...
 174:	00000010 	mfhi	zero
 178:	0000001e 	ddiv	zero,zero,zero
 17c:	0000001f 	ddivu	zero,zero,zero
 180:	8020089c 	lb	zero,2204(at)
 184:	40000000 	mfc0	zero,$0
 188:	fffffff8 	sd	ra,-8(ra)
	...
 194:	00000010 	mfhi	zero
 198:	0000001e 	ddiv	zero,zero,zero
 19c:	0000001f 	ddivu	zero,zero,zero
 1a0:	8020095c 	lb	zero,2396(at)
 1a4:	40000000 	mfc0	zero,$0
 1a8:	fffffff8 	sd	ra,-8(ra)
	...
 1b4:	00000010 	mfhi	zero
 1b8:	0000001e 	ddiv	zero,zero,zero
 1bc:	0000001f 	ddivu	zero,zero,zero
 1c0:	802009c8 	lb	zero,2504(at)
 1c4:	40000000 	mfc0	zero,$0
 1c8:	fffffff8 	sd	ra,-8(ra)
	...
 1d4:	00000010 	mfhi	zero
 1d8:	0000001e 	ddiv	zero,zero,zero
 1dc:	0000001f 	ddivu	zero,zero,zero
 1e0:	80200000 	lb	zero,0(at)
	...
 200:	80200a60 	lb	zero,2656(at)
	...
 220:	80200a90 	lb	zero,2704(at)
	...
 240:	80200af4 	lb	zero,2804(at)
	...
 260:	80200b5c 	lb	zero,2908(at)
	...
 280:	80200bf0 	lb	zero,3056(at)
	...
 2a0:	80200c84 	lb	zero,3204(at)
	...
 2c0:	80200d18 	lb	zero,3352(at)
	...
 2e0:	80200dac 	lb	zero,3500(at)
	...
 300:	80200e40 	lb	zero,3648(at)
	...
 320:	80200ea4 	lb	zero,3748(at)
	...
 340:	80200f1c 	lb	zero,3868(at)
	...
 360:	80200f70 	lb	zero,3952(at)
	...
 380:	80200fc4 	lb	zero,4036(at)
	...
 3a0:	80201050 	lb	zero,4176(at)
	...
 3c0:	802010d4 	lb	zero,4308(at)
	...
 3e0:	80201150 	lb	zero,4432(at)
	...
 400:	802011dc 	lb	zero,4572(at)
	...
 420:	80201224 	lb	zero,4644(at)
	...
 440:	8020126c 	lb	zero,4716(at)
	...
 460:	80201308 	lb	zero,4872(at)
	...
 480:	80201394 	lb	zero,5012(at)
	...
 4a0:	80201420 	lb	zero,5152(at)
	...
 4c0:	802014cc 	lb	zero,5324(at)
	...
 4e0:	8020158c 	lb	zero,5516(at)
	...
 500:	80201670 	lb	zero,5744(at)
	...
 520:	80201750 	lb	zero,5968(at)
	...
 538:	0000001d 	dmultu	zero,zero
 53c:	0000001f 	ddivu	zero,zero,zero
 540:	8020178c 	lb	zero,6028(at)
	...
 558:	0000001d 	dmultu	zero,zero
 55c:	0000001f 	ddivu	zero,zero,zero
Disassembly of section .debug_abbrev:

00000000 <.debug_abbrev>:
   0:	10011101 	beq	zero,at,4408 <dummy0-0x801fbccc>
   4:	11011206 	beq	t0,at,4820 <dummy0-0x801fb8b4>
   8:	13082501 	beq	t8,t0,9410 <dummy0-0x801f6cc4>
   c:	1b08030b 	0x1b08030b
  10:	02000008 	jr	s0
  14:	08030024 	j	c0090 <dummy0-0x80140044>
  18:	0b3e0b0b 	j	cf82c2c <dummy0-0x7327d4a8>
  1c:	16030000 	bne	s0,v1,20 <dummy0-0x802000b4>
  20:	3a080300 	xori	t0,s0,0x300
  24:	490b3b0b 	0x490b3b0b
  28:	04000013 	bltz	zero,78 <dummy0-0x8020005c>
  2c:	0c3f002e 	jal	fc00b8 <dummy0-0x7f24001c>
  30:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  34:	13490b3b 	beq	k0,t1,2d24 <dummy0-0x801fd3b0>
  38:	01120111 	0x1120111
  3c:	40064081 	0x40064081
  40:	0500000a 	bltz	t0,6c <dummy0-0x80200068>
  44:	0c3f002e 	jal	fc00b8 <dummy0-0x7f24001c>
  48:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  4c:	01110b3b 	0x1110b3b
  50:	40810112 	0x40810112
  54:	000a4006 	srlv	t0,t2,zero
  58:	012e0600 	0x12e0600
  5c:	0c3f1301 	jal	fc4c04 <dummy0-0x7f23b4d0>
  60:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  64:	0c270b3b 	jal	9c2cec <dummy0-0x7f83d3e8>
  68:	01111349 	0x1111349
  6c:	40810112 	0x40810112
  70:	000a4006 	srlv	t0,t2,zero
  74:	00340700 	0x340700
  78:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  7c:	13490b3b 	beq	k0,t1,2d6c <dummy0-0x801fd368>
  80:	00000a02 	srl	at,zero,0x8
  84:	03001308 	0x3001308
  88:	000c3c08 	0xc3c08
  8c:	012e0900 	0x12e0900
  90:	0c3f1301 	jal	fc4c04 <dummy0-0x7f23b4d0>
  94:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  98:	13490b3b 	beq	k0,t1,2d88 <dummy0-0x801fd34c>
  9c:	00000c3c 	dsll32	at,zero,0x10
  a0:	0000180a 	0x180a
  a4:	012e0b00 	0x12e0b00
  a8:	0c3f1301 	jal	fc4c04 <dummy0-0x7f23b4d0>
  ac:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  b0:	0c270b3b 	jal	9c2cec <dummy0-0x7f83d3e8>
  b4:	0c3c1349 	jal	f04d24 <dummy0-0x7f2fb3b0>
  b8:	050c0000 	teqi	t0,0
  bc:	00134900 	sll	t1,s3,0x4
  c0:	012e0d00 	0x12e0d00
  c4:	08030c3f 	j	c30fc <dummy0-0x8013cfd8>
  c8:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  cc:	13490c27 	beq	k0,t1,316c <dummy0-0x801fcf68>
  d0:	00000c3c 	dsll32	at,zero,0x10
  d4:	0b000f0e 	j	c003c38 <dummy0-0x741fc49c>
  d8:	0013490b 	0x13490b
  dc:	000f0f00 	sll	at,t7,0x1c
  e0:	00000b0b 	0xb0b
  e4:	00002610 	0x2610
	...
Disassembly of section .debug_info:

00000000 <.debug_info>:
   0:	000002e8 	0x2e8
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	80200670 	lb	zero,1648(at)
  14:	80200078 	lb	zero,120(at)
  18:	20554e47 	addi	s5,v0,20039
  1c:	2e332043 	sltiu	s3,s1,8259
  20:	2d362e34 	sltiu	s6,t1,11828
  24:	2e332e31 	sltiu	s3,s1,11825
  28:	672d2036 	daddiu	t5,t9,8246
  2c:	70610100 	0x70610100
  30:	00632e70 	tge	v1,v1,0xb9
  34:	6d6f682f 	ldr	t7,26671(t3)
  38:	68702f65 	ldl	s0,12133(v1)
  3c:	78656e69 	0x78656e69
  40:	4f43752f 	c3	0x143752f
  44:	2f494953 	sltiu	t1,k0,18771
  48:	6c667a65 	ldr	a2,31333(v1)
  4c:	00687361 	0x687361
  50:	736e7502 	0x736e7502
  54:	656e6769 	daddiu	t6,t3,26473
  58:	68632064 	ldl	v1,8292(v1)
  5c:	01007261 	0x1007261
  60:	4e490308 	c3	0x490308
  64:	00553854 	0x553854
  68:	00501902 	0x501902
  6c:	73020000 	0x73020000
  70:	656e6769 	daddiu	t6,t3,26473
  74:	68632064 	ldl	v1,8292(v1)
  78:	01007261 	0x1007261
  7c:	68730206 	ldl	s3,518(v1)
  80:	2074726f 	addi	s4,v1,29295
  84:	69736e75 	ldl	s3,28277(t3)
  88:	64656e67 	daddiu	a1,v1,28263
  8c:	746e6920 	jalx	1b9a480 <dummy0-0x7e665c54>
  90:	02070200 	0x2070200
  94:	726f6873 	0x726f6873
  98:	6e692074 	ldr	t1,8308(s3)
  9c:	05020074 	bltzl	t0,270 <dummy0-0x801ffe64>
  a0:	544e4903 	bnel	v0,t6,124b0 <dummy0-0x801edc24>
  a4:	00553233 	tltu	v0,s5,0xc8
  a8:	00ae1d02 	0xae1d02
  ac:	75020000 	jalx	4080000 <dummy0-0x7c1800d4>
  b0:	6769736e 	daddiu	t1,k1,29550
  b4:	2064656e 	addi	a0,v1,25966
  b8:	00746e69 	0x746e69
  bc:	69020704 	ldl	v0,1796(t0)
  c0:	0400746e 	bltz	zero,1d27c <dummy0-0x801e2e58>
  c4:	64010405 	daddiu	at,zero,1029
  c8:	62617369 	daddi	at,s3,29545
  cc:	6d5f656c 	ldr	ra,25964(t2)
  d0:	65747361 	daddiu	s4,t3,29537
  d4:	6e655f72 	ldr	a1,24434(s3)
  d8:	656e6967 	daddiu	t6,t3,26983
  dc:	be0f0100 	cache	0xf,256(s0)
  e0:	78000000 	0x78000000
  e4:	d4802000 	ldc1	$f0,8192(a0)
  e8:	10802000 	beqz	a0,80ec <dummy0-0x801f7fe8>
  ec:	01000000 	0x1000000
  f0:	6401056e 	daddiu	at,zero,1390
  f4:	796d6d75 	0x796d6d75
  f8:	17010030 	bne	t8,at,1bc <dummy0-0x801fff18>
  fc:	802000d4 	lb	zero,212(at)
 100:	802000f4 	lb	zero,244(at)
 104:	0000002c 	dadd	zero,zero,zero
 108:	01056e01 	0x1056e01
 10c:	6d6d7564 	ldr	t5,30052(t3)
 110:	01003179 	0x1003179
 114:	2000f418 	addi	zero,zero,-3048
 118:	20011480 	addi	at,zero,5248
 11c:	00004880 	sll	t1,zero,0x2
 120:	056e0100 	tnei	t3,256
 124:	6d756401 	ldr	s5,25601(t3)
 128:	0032796d 	0x32796d
 12c:	01141901 	0x1141901
 130:	01348020 	add	s0,t1,s4
 134:	00648020 	add	s0,v1,a0
 138:	6e010000 	ldr	at,0(s0)
 13c:	75640105 	jalx	5900414 <dummy0-0x7a8ffcc0>
 140:	33796d6d 	andi	t9,k1,0x6d6d
 144:	341a0100 	li	k0,0x100
 148:	54802001 	bnezl	a0,8150 <dummy0-0x801f7f84>
 14c:	80802001 	lb	zero,8193(a0)
 150:	01000000 	0x1000000
 154:	6401056e 	daddiu	at,zero,1390
 158:	796d6d75 	0x796d6d75
 15c:	1b010034 	0x1b010034
 160:	80200154 	lb	zero,340(at)
 164:	80200174 	lb	zero,372(at)
 168:	0000009c 	0x9c
 16c:	c9066e01 	lwc2	$6,28161(t0)
 170:	01000002 	0x1000002
 174:	6e69616d 	ldr	t1,24941(s3)
 178:	011e0100 	0x11e0100
 17c:	000000be 	dsrl32	zero,zero,0x2
 180:	80200174 	lb	zero,372(at)
 184:	80200670 	lb	zero,1648(at)
 188:	000000b8 	dsll	zero,zero,0x2
 18c:	69076e01 	ldl	a3,28161(t0)
 190:	a01f0100 	sb	ra,256(zero)
 194:	02000000 	0x2000000
 198:	73081091 	0x73081091
 19c:	705f6973 	0x705f6973
 1a0:	6d74726f 	ldr	s4,29295(t3)
 1a4:	01007061 	0x1007061
 1a8:	69737307 	ldl	s3,29447(t3)
 1ac:	70616d5f 	0x70616d5f
 1b0:	c9200100 	lwc2	$0,256(t1)
 1b4:	02000002 	0x2000002
 1b8:	72071491 	0x72071491
 1bc:	00646964 	0x646964
 1c0:	00a02101 	0xa02101
 1c4:	91020000 	lbu	v0,0(t0)
 1c8:	01e40918 	0x1e40918
 1cc:	73010000 	0x73010000
 1d0:	725f6970 	0x725f6970
 1d4:	5f646165 	0x5f646165
 1d8:	01006469 	0x1006469
 1dc:	0000be21 	0xbe21
 1e0:	000a0100 	sll	zero,t2,0x4
 1e4:	00020d09 	0x20d09
 1e8:	6c720100 	ldr	s2,256(v1)
 1ec:	63695f78 	daddi	t1,k1,24440
 1f0:	65686361 	daddiu	t0,t3,25441
 1f4:	766e695f 	jalx	9b9a57c <dummy0-0x76665b58>
 1f8:	64696c61 	daddiu	t1,v1,27745
 1fc:	5f657461 	0x5f657461
 200:	006c6c61 	0x6c6c61
 204:	00be2201 	0xbe2201
 208:	0a010000 	j	8040000 <dummy0-0x781c00d4>
 20c:	022c0900 	0x22c0900
 210:	72010000 	0x72010000
 214:	645f786c 	daddiu	ra,v0,30828
 218:	5f6d656d 	0x5f6d656d
 21c:	62616e65 	daddi	at,s3,28261
 220:	0100656c 	0x100656c
 224:	0000be23 	0xbe23
 228:	000a0100 	sll	zero,t2,0x4
 22c:	00024c09 	0x24c09
 230:	70730100 	0x70730100
 234:	65735f69 	daddiu	s3,t3,24425
 238:	706e755f 	0x706e755f
 23c:	65746f72 	daddiu	s4,t3,28530
 240:	01007463 	0x1007463
 244:	0000be2c 	0xbe2c
 248:	000a0100 	sll	zero,t2,0x4
 24c:	00026909 	0x26909
 250:	70730100 	0x70730100
 254:	6c625f69 	ldr	v0,24425(v1)
 258:	72655f6b 	0x72655f6b
 25c:	00657361 	0x657361
 260:	00be2d01 	0xbe2d01
 264:	0a010000 	j	8040000 <dummy0-0x781c00d4>
 268:	028e0b00 	0x28e0b00
 26c:	6d010000 	ldr	at,0(t0)
 270:	65736d65 	daddiu	s3,t3,28005
 274:	00030074 	teq	zero,v1,0x1
 278:	0002cf01 	0x2cf01
 27c:	cf0c0100 	lwc3	$12,256(t8)
 280:	0c000002 	jal	8 <dummy0-0x802000cc>
 284:	000000be 	dsrl32	zero,zero,0x2
 288:	0000ae0c 	syscall	0x2b8
 28c:	a7090000 	sh	t1,0(t8)
 290:	01000002 	0x1000002
 294:	5f697073 	0x5f697073
 298:	74697277 	jalx	1a5c9dc <dummy0-0x7e7a36f8>
 29c:	37010065 	ori	at,t8,0x65
 2a0:	000000be 	dsrl32	zero,zero,0x2
 2a4:	0d000a01 	jal	4002804 <dummy0-0x7c1fd8d0>
 2a8:	6d656d01 	ldr	a1,27905(t3)
 2ac:	00706d63 	0x706d63
 2b0:	be010003 	cache	0x1,3(s0)
 2b4:	01000000 	0x1000000
 2b8:	0002d10c 	syscall	0xb44
 2bc:	02d10c00 	0x2d10c00
 2c0:	ae0c0000 	sw	t4,0(s0)
 2c4:	00000000 	nop
 2c8:	9a040e00 	lwr	a0,3584(s0)
 2cc:	0f000001 	jal	c000004 <dummy0-0x742000d0>
 2d0:	d7040e04 	ldc1	$f4,3588(t8)
 2d4:	10000002 	b	2e0 <dummy0-0x801ffdf4>
 2d8:	61747307 	daddi	s4,t3,29447
 2dc:	01006567 	0x1006567
 2e0:	00006104 	0x6104
 2e4:	30030500 	andi	v1,zero,0x500
 2e8:	00802018 	0x802018
Disassembly of section .debug_line:

00000000 <.debug_line>:
   0:	000000ba 	dsrl	zero,zero,0x2
   4:	00330002 	0x330002
   8:	01010000 	0x1010000
   c:	000d0efb 	dsra	at,t5,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	2e707061 	sltiu	s0,s3,28769
  20:	00000063 	0x63
  24:	70736200 	0x70736200
  28:	0000682e 	dneg	t5,zero
  2c:	693c0000 	ldl	gp,0(t1)
  30:	7265746e 	0x7265746e
  34:	3e6c616e 	0x3e6c616e
  38:	00000000 	nop
  3c:	02050000 	0x2050000
  40:	80200078 	lb	zero,120(at)
  44:	bb010e03 	swr	at,3587(t8)
  48:	bbbbbbbb 	swr	k1,-17477(sp)
  4c:	ba3e08bb 	swr	s8,2235(s1)
  50:	08ba3d08 	j	2e8f420 <dummy0-0x7d370cb4>
  54:	3d08ba3d 	0x3d08ba3d
  58:	ba3d08ba 	swr	sp,2234(s1)
  5c:	4bf33f08 	c2	0x1f33f08
  60:	83e50883 	lb	a1,2179(ra)
  64:	bf833e08 	cache	0x3,15880(gp)
  68:	08f3f3f3 	j	3cfcfcc <dummy0-0x7c503108>
  6c:	f5f3f33d 	sdc1	$f19,-3267(t7)
  70:	02750884 	0x2750884
  74:	7708113c 	jalx	c2044f0 <dummy0-0x73ffbbe4>
  78:	7708ad08 	jalx	c22b420 <dummy0-0x73fd4cb4>
  7c:	08f3f3f3 	j	3cfcfcc <dummy0-0x7c503108>
  80:	f4f3f33d 	sdc1	$f19,-3267(a3)
  84:	02750884 	0x2750884
  88:	7708113c 	jalx	c2044f0 <dummy0-0x73ffbbe4>
  8c:	7708ad08 	jalx	c22b420 <dummy0-0x73fd4cb4>
  90:	08f3f3f3 	j	3cfcfcc <dummy0-0x7c503108>
  94:	f4f3f33d 	sdc1	$f19,-3267(a3)
  98:	02750884 	0x2750884
  9c:	7708113c 	jalx	c2044f0 <dummy0-0x73ffbbe4>
  a0:	7608ad08 	jalx	822b420 <dummy0-0x77fd4cb4>
  a4:	08f3f3f3 	j	3cfcfcc <dummy0-0x7c503108>
  a8:	f4f3f33d 	sdc1	$f19,-3267(a3)
  ac:	02750884 	0x2750884
  b0:	7708113c 	jalx	c2044f0 <dummy0-0x73ffbbe4>
  b4:	7608ad08 	jalx	822b420 <dummy0-0x77fd4cb4>
  b8:	00180283 	sra	zero,t8,0xa
  bc:	Address 0x00000000000000bc is out of bounds.

Disassembly of section .debug_frame:

00000000 <.debug_frame>:
   0:	0000000c 	syscall
   4:	ffffffff 	sd	ra,-1(ra)
   8:	04010001 	b	10 <dummy0-0x802000c4>
   c:	001d0c1f 	0x1d0c1f
  10:	00000018 	mult	zero,zero
  14:	00000000 	nop
  18:	80200078 	lb	zero,120(at)
  1c:	0000005c 	0x5c
  20:	44080e44 	0x44080e44
  24:	447e1e11 	0x447e1e11
  28:	00081e0c 	syscall	0x2078
  2c:	00000018 	mult	zero,zero
  30:	00000000 	nop
  34:	802000d4 	lb	zero,212(at)
  38:	00000020 	add	zero,zero,zero
  3c:	44080e44 	0x44080e44
  40:	447e1e11 	0x447e1e11
  44:	00081e0c 	syscall	0x2078
  48:	00000018 	mult	zero,zero
  4c:	00000000 	nop
  50:	802000f4 	lb	zero,244(at)
  54:	00000020 	add	zero,zero,zero
  58:	44080e44 	0x44080e44
  5c:	447e1e11 	0x447e1e11
  60:	00081e0c 	syscall	0x2078
  64:	00000018 	mult	zero,zero
  68:	00000000 	nop
  6c:	80200114 	lb	zero,276(at)
  70:	00000020 	add	zero,zero,zero
  74:	44080e44 	0x44080e44
  78:	447e1e11 	0x447e1e11
  7c:	00081e0c 	syscall	0x2078
  80:	00000018 	mult	zero,zero
  84:	00000000 	nop
  88:	80200134 	lb	zero,308(at)
  8c:	00000020 	add	zero,zero,zero
  90:	44080e44 	0x44080e44
  94:	447e1e11 	0x447e1e11
  98:	00081e0c 	syscall	0x2078
  9c:	00000018 	mult	zero,zero
  a0:	00000000 	nop
  a4:	80200154 	lb	zero,340(at)
  a8:	00000020 	add	zero,zero,zero
  ac:	44080e44 	0x44080e44
  b0:	447e1e11 	0x447e1e11
  b4:	00081e0c 	syscall	0x2078
  b8:	0000001c 	dmult	zero,zero
  bc:	00000000 	nop
  c0:	80200174 	lb	zero,372(at)
  c4:	000004fc 	dsll32	zero,zero,0x13
  c8:	48280e44 	0x48280e44
  cc:	117e1e11 	beq	t3,s8,7914 <dummy0-0x801f87c0>
  d0:	0c447f1f 	jal	111fc7c <dummy0-0x7f0e0458>
  d4:	0000281e 	0x281e
  d8:	0000000c 	syscall
  dc:	ffffffff 	sd	ra,-1(ra)
  e0:	04010001 	b	e8 <dummy0-0x801fffec>
  e4:	001d0c1f 	0x1d0c1f
  e8:	00000018 	mult	zero,zero
  ec:	000000d8 	0xd8
  f0:	80200670 	lb	zero,1648(at)
  f4:	0000007c 	dsll32	zero,zero,0x1
  f8:	44100e44 	0x44100e44
  fc:	447e1e11 	0x447e1e11
 100:	00101e0c 	syscall	0x4078
 104:	00000018 	mult	zero,zero
 108:	000000d8 	0xd8
 10c:	802006ec 	lb	zero,1772(at)
 110:	0000007c 	dsll32	zero,zero,0x1
 114:	44100e44 	0x44100e44
 118:	447e1e11 	0x447e1e11
 11c:	00101e0c 	syscall	0x4078
 120:	00000018 	mult	zero,zero
 124:	000000d8 	0xd8
 128:	80200768 	lb	zero,1896(at)
 12c:	0000007c 	dsll32	zero,zero,0x1
 130:	44100e44 	0x44100e44
 134:	447e1e11 	0x447e1e11
 138:	00101e0c 	syscall	0x4078
 13c:	00000018 	mult	zero,zero
 140:	000000d8 	0xd8
 144:	802007e4 	lb	zero,2020(at)
 148:	0000005c 	0x5c
 14c:	44100e44 	0x44100e44
 150:	447e1e11 	0x447e1e11
 154:	00101e0c 	syscall	0x4078
 158:	00000018 	mult	zero,zero
 15c:	000000d8 	0xd8
 160:	80200840 	lb	zero,2112(at)
 164:	0000005c 	0x5c
 168:	44100e44 	0x44100e44
 16c:	447e1e11 	0x447e1e11
 170:	00101e0c 	syscall	0x4078
 174:	00000018 	mult	zero,zero
 178:	000000d8 	0xd8
 17c:	8020089c 	lb	zero,2204(at)
 180:	000000c0 	sll	zero,zero,0x3
 184:	44100e44 	0x44100e44
 188:	447e1e11 	0x447e1e11
 18c:	00101e0c 	syscall	0x4078
 190:	00000018 	mult	zero,zero
 194:	000000d8 	0xd8
 198:	8020095c 	lb	zero,2396(at)
 19c:	0000006c 	0x6c
 1a0:	44100e44 	0x44100e44
 1a4:	447e1e11 	0x447e1e11
 1a8:	00101e0c 	syscall	0x4078
 1ac:	00000018 	mult	zero,zero
 1b0:	000000d8 	0xd8
 1b4:	802009c8 	lb	zero,2504(at)
 1b8:	00000098 	0x98
 1bc:	44100e44 	0x44100e44
 1c0:	447e1e11 	0x447e1e11
 1c4:	00101e0c 	syscall	0x4078
 1c8:	0000000c 	syscall
 1cc:	ffffffff 	sd	ra,-1(ra)
 1d0:	04010001 	b	1d8 <dummy0-0x801ffefc>
 1d4:	001d0c1f 	0x1d0c1f
 1d8:	0000000c 	syscall
 1dc:	000001c8 	0x1c8
 1e0:	80201750 	lb	zero,5968(at)
 1e4:	0000003c 	dsll32	zero,zero,0x0
 1e8:	0000000c 	syscall
 1ec:	ffffffff 	sd	ra,-1(ra)
 1f0:	04010001 	b	1f8 <dummy0-0x801ffedc>
 1f4:	001d0c1f 	0x1d0c1f
 1f8:	0000000c 	syscall
 1fc:	000001e8 	0x1e8
 200:	8020178c 	lb	zero,6028(at)
 204:	000000a4 	0xa4
Disassembly of section .debug_pubnames:

00000000 <.debug_pubnames>:
   0:	00000068 	0x68
   4:	00000002 	srl	zero,zero,0x0
   8:	02ec0000 	0x2ec0000
   c:	00c50000 	0xc50000
  10:	69640000 	ldl	a0,0(t3)
  14:	6c626173 	ldr	v0,24947(v1)
  18:	616d5f65 	daddi	t5,t3,24421
  1c:	72657473 	0x72657473
  20:	676e655f 	daddiu	t6,k1,25951
  24:	00656e69 	0x656e69
  28:	000000f1 	tgeu	zero,zero,0x3
  2c:	6d6d7564 	ldr	t5,30052(t3)
  30:	0a003079 	j	800c1e4 <dummy0-0x781f3ef0>
  34:	64000001 	daddiu	zero,zero,1
  38:	796d6d75 	0x796d6d75
  3c:	01230031 	tgeu	t1,v1
  40:	75640000 	jalx	5900000 <dummy0-0x7a9000d4>
  44:	32796d6d 	andi	t9,s3,0x6d6d
  48:	00013c00 	sll	a3,at,0x10
  4c:	6d756400 	ldr	s5,25600(t3)
  50:	0033796d 	0x33796d
  54:	00000155 	0x155
  58:	6d6d7564 	ldr	t5,30052(t3)
  5c:	6e003479 	ldr	zero,13433(s0)
  60:	6d000001 	ldr	zero,1(t0)
  64:	006e6961 	0x6e6961
  68:	00000000 	nop
  6c:	000000ca 	0xca
  70:	00000002 	srl	zero,zero,0x0
  74:	036d0000 	0x36d0000
  78:	00cd0000 	0xcd0000
  7c:	6c720000 	ldr	s2,0(v1)
  80:	63645f78 	daddi	a0,k1,24440
  84:	65686361 	daddiu	t0,t3,25441
  88:	756c665f 	jalx	5b1997c <dummy0-0x7a6e6758>
  8c:	22006873 	addi	zero,s0,26739
  90:	72000001 	0x72000001
  94:	645f786c 	daddiu	ra,v0,30828
  98:	68636163 	ldl	v1,24931(v1)
  9c:	6e695f65 	ldr	t1,24421(s3)
  a0:	696c6176 	ldl	t4,24950(t3)
  a4:	65746164 	daddiu	s4,t3,24932
  a8:	00017c00 	sll	t7,at,0x10
  ac:	786c7200 	0x786c7200
  b0:	6163645f 	daddi	v1,t3,25695
  b4:	5f656863 	0x5f656863
  b8:	726f7473 	0x726f7473
  bc:	01d10065 	0x1d10065
  c0:	6c720000 	ldr	s2,0(v1)
  c4:	63645f78 	daddi	a0,k1,24440
  c8:	65686361 	daddiu	t0,t3,25441
  cc:	766e695f 	jalx	9b9a57c <dummy0-0x76665b58>
  d0:	64696c61 	daddiu	t1,v1,27745
  d4:	5f657461 	0x5f657461
  d8:	006c6c61 	0x6c6c61
  dc:	00000211 	0x211
  e0:	5f786c72 	0x5f786c72
  e4:	63616369 	daddi	at,k1,25449
  e8:	695f6568 	ldl	ra,25960(t2)
  ec:	6c61766e 	ldr	at,30318(v1)
  f0:	74616469 	jalx	18591a4 <dummy0-0x7e9a6f30>
  f4:	6c615f65 	ldr	at,24421(v1)
  f8:	0251006c 	0x251006c
  fc:	6c720000 	ldr	s2,0(v1)
 100:	63695f78 	daddi	t1,k1,24440
 104:	65686361 	daddiu	t0,t3,25441
 108:	636f6c5f 	daddi	t7,k1,27743
 10c:	02af006b 	0x2af006b
 110:	6c720000 	ldr	s2,0(v1)
 114:	6d645f78 	ldr	a0,24440(t3)
 118:	655f6d65 	daddiu	ra,t2,28005
 11c:	6c62616e 	ldr	v0,24942(v1)
 120:	03080065 	0x3080065
 124:	6c720000 	ldr	s2,0(v1)
 128:	6d695f78 	ldr	t1,24440(t3)
 12c:	725f6d65 	0x725f6d65
 130:	6c696665 	ldr	t1,26213(v1)
 134:	0000006c 	0x6c
 138:	00190000 	sll	zero,t9,0x0
 13c:	00020000 	sll	zero,v0,0x0
 140:	00000000 	nop
 144:	000001b2 	tlt	zero,zero,0x6
 148:	0000014c 	syscall	0x5
 14c:	636d656d 	daddi	t5,k1,25965
 150:	0000706d 	0x706d
 154:	19000000 	blez	t0,158 <dummy0-0x801fff7c>
 158:	02000000 	0x2000000
 15c:	00000000 	nop
 160:	00022500 	sll	a0,v0,0x14
 164:	00015d00 	sll	t3,at,0x14
 168:	6d656d00 	ldr	a1,27904(t3)
 16c:	00746573 	tltu	v1,s4,0x195
 170:	00000000 	nop
Disassembly of section .debug_aranges:

00000000 <.debug_aranges>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80200078 	lb	zero,120(at)
  14:	000005f8 	dsll	zero,zero,0x17
	...
Disassembly of section .debug_abbrev.1:

00000000 <.debug_abbrev.1>:
   0:	10011101 	beq	zero,at,4408 <dummy0-0x801fbccc>
   4:	11011206 	beq	t0,at,4820 <dummy0-0x801fb8b4>
   8:	13082501 	beq	t8,t0,9410 <dummy0-0x801f6cc4>
   c:	1b08030b 	0x1b08030b
  10:	02000008 	jr	s0
  14:	08030016 	j	c0058 <dummy0-0x8014007c>
  18:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  1c:	00001349 	0x1349
  20:	03002403 	0x3002403
  24:	3e0b0b08 	0x3e0b0b08
  28:	0400000b 	bltz	zero,58 <dummy0-0x8020007c>
  2c:	1301012e 	beq	t8,at,4e8 <dummy0-0x801ffbec>
  30:	08030c3f 	j	c30fc <dummy0-0x8013cfd8>
  34:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  38:	01110c27 	0x1110c27
  3c:	40810112 	0x40810112
  40:	000a4006 	srlv	t0,t2,zero
  44:	00050500 	sll	zero,a1,0x14
  48:	0b3a0e03 	j	ce8380c <dummy0-0x7337c8c8>
  4c:	13490b3b 	beq	k0,t1,2d3c <dummy0-0x801fd398>
  50:	00000a02 	srl	at,zero,0x8
  54:	03000506 	0x3000506
  58:	3b0b3a08 	xori	t3,t8,0x3a08
  5c:	0213490b 	0x213490b
  60:	0700000a 	bltz	t8,8c <dummy0-0x80200048>
  64:	08030034 	j	c00d0 <dummy0-0x80140004>
  68:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  6c:	0a021349 	j	8084d24 <dummy0-0x7817b3b0>
  70:	34080000 	li	t0,0x0
  74:	3a0e0300 	xori	t6,s0,0x300
  78:	490b3b0b 	0x490b3b0b
  7c:	000a0213 	0xa0213
  80:	012e0900 	0x12e0900
  84:	0c3f1301 	jal	fc4c04 <dummy0-0x7f23b4d0>
  88:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  8c:	0c270b3b 	jal	9c2cec <dummy0-0x7f83d3e8>
  90:	01111349 	0x1111349
  94:	40810112 	0x40810112
  98:	000a4006 	srlv	t0,t2,zero
  9c:	012e0a00 	0x12e0a00
  a0:	08030c3f 	j	c30fc <dummy0-0x8013cfd8>
  a4:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  a8:	01110c27 	0x1110c27
  ac:	40810112 	0x40810112
  b0:	000a4006 	srlv	t0,t2,zero
	...
Disassembly of section .debug_info.2:

00000000 <.debug_info.2>:
   0:	00000369 	0x369
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	80200a60 	lb	zero,2656(at)
  14:	80200670 	lb	zero,1648(at)
  18:	20554e47 	addi	s5,v0,20039
  1c:	2e332043 	sltiu	s3,s1,8259
  20:	2d362e34 	sltiu	s6,t1,11828
  24:	2e332e31 	sltiu	s3,s1,11825
  28:	672d2036 	daddiu	t5,t9,8246
  2c:	6c720100 	ldr	s2,256(v1)
  30:	61635f78 	daddi	v1,t3,24440
  34:	2e656863 	sltiu	a1,s3,26723
  38:	682f0063 	ldl	t7,99(at)
  3c:	2f656d6f 	sltiu	a1,k1,28015
  40:	6e696870 	ldr	t1,26736(s3)
  44:	752f7865 	jalx	4bde194 <dummy0-0x7b621f40>
  48:	49534f43 	0x49534f43
  4c:	7a652f49 	0x7a652f49
  50:	73616c66 	0x73616c66
  54:	42020068 	c0	0x20068
  58:	454c4f4f 	0x454c4f4f
  5c:	02004e41 	0x2004e41
  60:	00006518 	0x6518
  64:	6e750300 	ldr	s5,768(s3)
  68:	6e676973 	ldr	a3,26995(s3)
  6c:	63206465 	daddi	zero,t9,25701
  70:	00726168 	0x726168
  74:	73030801 	0x73030801
  78:	656e6769 	daddiu	t6,t3,26473
  7c:	68632064 	ldl	v1,8292(v1)
  80:	01007261 	0x1007261
  84:	68730306 	ldl	s3,774(v1)
  88:	2074726f 	addi	s4,v1,29295
  8c:	69736e75 	ldl	s3,28277(t3)
  90:	64656e67 	daddiu	a1,v1,28263
  94:	746e6920 	jalx	1b9a480 <dummy0-0x7e665c54>
  98:	03070200 	0x3070200
  9c:	726f6873 	0x726f6873
  a0:	6e692074 	ldr	t1,8308(s3)
  a4:	05020074 	bltzl	t0,278 <dummy0-0x801ffe5c>
  a8:	544e4902 	bnel	v0,t6,124b4 <dummy0-0x801edc20>
  ac:	00553233 	tltu	v0,s5,0xc8
  b0:	00b61d02 	0xb61d02
  b4:	75030000 	jalx	40c0000 <dummy0-0x7c1400d4>
  b8:	6769736e 	daddiu	t1,k1,29550
  bc:	2064656e 	addi	a0,v1,25966
  c0:	00746e69 	0x746e69
  c4:	69030704 	ldl	v1,1796(t0)
  c8:	0400746e 	bltz	zero,1d284 <dummy0-0x801e2e50>
  cc:	01220405 	0x1220405
  d0:	72010000 	0x72010000
  d4:	645f786c 	daddiu	ra,v0,30828
  d8:	68636163 	ldl	v1,24931(v1)
  dc:	6c665f65 	ldr	a2,24421(v1)
  e0:	00687375 	0x687375
  e4:	70013501 	0x70013501
  e8:	ec802006 	swc3	$0,8198(a0)
  ec:	e8802006 	swc2	$0,8198(a0)
  f0:	01000000 	0x1000000
  f4:	0000056e 	0x56e
  f8:	34010000 	li	at,0x0
  fc:	000000a8 	0xa8
 100:	06109102 	bltzal	s0,fffe450c <__bss_end+0x7fde28ac>
 104:	657a6973 	daddiu	k0,t3,26995
 108:	a8340100 	swl	s4,256(at)
 10c:	02000000 	0x2000000
 110:	61071491 	daddi	a3,t0,5265
 114:	00726464 	0x726464
 118:	00a83601 	0xa83601
 11c:	91020000 	lbu	v0,0(t0)
 120:	7c040000 	0x7c040000
 124:	01000001 	0x1000001
 128:	5f786c72 	0x5f786c72
 12c:	63616364 	daddi	at,k1,25444
 130:	695f6568 	ldl	ra,25960(t2)
 134:	6c61766e 	ldr	at,30318(v1)
 138:	74616469 	jalx	18591a4 <dummy0-0x7e9a6f30>
 13c:	4e010065 	c3	0x10065
 140:	2006ec01 	addi	a2,zero,-5119
 144:	20076880 	addi	a3,zero,26752
 148:	00010480 	sll	zero,at,0x12
 14c:	056e0100 	tnei	t3,256
 150:	00000000 	nop
 154:	00a84d01 	0xa84d01
 158:	91020000 	lbu	v0,0(t0)
 15c:	69730610 	ldl	s3,1552(t3)
 160:	0100657a 	0x100657a
 164:	0000a84d 	break	0x0,0x2a1
 168:	14910200 	bne	a0,s1,96c <dummy0-0x801ff768>
 16c:	64646107 	daddiu	a0,v1,24839
 170:	4f010072 	c3	0x1010072
 174:	000000a8 	0xa8
 178:	00009102 	srl	s2,zero,0x4
 17c:	0001d104 	0x1d104
 180:	6c720100 	ldr	s2,256(v1)
 184:	63645f78 	daddi	a0,k1,24440
 188:	65686361 	daddiu	t0,t3,25441
 18c:	6f74735f 	ldr	s4,29535(k1)
 190:	01006572 	tlt	t0,zero,0x195
 194:	07680168 	tgei	k1,360
 198:	07e48020 	0x7e48020
 19c:	01208020 	add	s0,t1,zero
 1a0:	6e010000 	ldr	at,0(s0)
 1a4:	00000005 	0x5
 1a8:	a8670100 	swl	a3,256(v1)
 1ac:	02000000 	0x2000000
 1b0:	73061091 	0x73061091
 1b4:	00657a69 	0x657a69
 1b8:	00a86701 	0xa86701
 1bc:	91020000 	lbu	v0,0(t0)
 1c0:	64610714 	daddiu	at,v1,1812
 1c4:	01007264 	0x1007264
 1c8:	0000a869 	0xa869
 1cc:	00910200 	0x910200
 1d0:	02110400 	0x2110400
 1d4:	72010000 	0x72010000
 1d8:	645f786c 	daddiu	ra,v0,30828
 1dc:	68636163 	ldl	v1,24931(v1)
 1e0:	6e695f65 	ldr	t1,24421(s3)
 1e4:	696c6176 	ldl	t4,24950(t3)
 1e8:	65746164 	daddiu	s4,t3,24932
 1ec:	6c6c615f 	ldr	t4,24927(v1)
 1f0:	01810100 	0x1810100
 1f4:	802007e4 	lb	zero,2020(at)
 1f8:	80200840 	lb	zero,2112(at)
 1fc:	0000013c 	dsll32	zero,zero,0x4
 200:	08086e01 	j	21b804 <dummy0-0x7ffe48d0>
 204:	01000000 	0x1000000
 208:	0000a882 	srl	s5,zero,0x2
 20c:	00910200 	0x910200
 210:	02510400 	0x2510400
 214:	72010000 	0x72010000
 218:	695f786c 	ldl	ra,30828(t2)
 21c:	68636163 	ldl	v1,24931(v1)
 220:	6e695f65 	ldr	t1,24421(s3)
 224:	696c6176 	ldl	t4,24950(t3)
 228:	65746164 	daddiu	s4,t3,24932
 22c:	6c6c615f 	ldr	t4,24927(v1)
 230:	01990100 	0x1990100
 234:	80200840 	lb	zero,2112(at)
 238:	8020089c 	lb	zero,2204(at)
 23c:	00000158 	0x158
 240:	08086e01 	j	21b804 <dummy0-0x7ffe48d0>
 244:	01000000 	0x1000000
 248:	0000a89a 	0xa89a
 24c:	00910200 	0x910200
 250:	02af0900 	0x2af0900
 254:	72010000 	0x72010000
 258:	695f786c 	ldl	ra,30828(t2)
 25c:	68636163 	ldl	v1,24931(v1)
 260:	6f6c5f65 	ldr	t4,24421(k1)
 264:	01006b63 	0x1006b63
 268:	005601b2 	tlt	v0,s6,0x6
 26c:	089c0000 	j	2700000 <dummy0-0x7db000d4>
 270:	095c8020 	j	5720080 <dummy0-0x7aae0054>
 274:	01748020 	add	s0,t3,s4
 278:	6e010000 	ldr	at,0(s0)
 27c:	646f6d06 	daddiu	t7,v1,27910
 280:	b1010065 	sdl	at,101(t0)
 284:	000000a8 	0xa8
 288:	08109102 	j	424408 <dummy0-0x7fddbccc>
 28c:	00000008 	jr	zero
 290:	00a8b301 	0xa8b301
 294:	91020000 	lbu	v0,0(t0)
 298:	6f6d0700 	ldr	t5,1792(k1)
 29c:	765f6564 	jalx	97d9590 <dummy0-0x76a26b44>
 2a0:	64696c61 	daddiu	t1,v1,27745
 2a4:	56b40100 	bnel	s5,s4,6a8 <dummy0-0x801ffa2c>
 2a8:	02000000 	0x2000000
 2ac:	04000491 	bltz	zero,14f4 <dummy0-0x801febe0>
 2b0:	00000308 	0x308
 2b4:	786c7201 	0x786c7201
 2b8:	656d645f 	daddiu	t5,t3,25695
 2bc:	6e655f6d 	ldr	a1,24429(s3)
 2c0:	656c6261 	daddiu	t4,t3,25185
 2c4:	01df0100 	0x1df0100
 2c8:	8020095c 	lb	zero,2396(at)
 2cc:	802009c8 	lb	zero,2504(at)
 2d0:	00000190 	0x190
 2d4:	62066e01 	daddi	a2,s0,28161
 2d8:	00657361 	0x657361
 2dc:	00a8de01 	0xa8de01
 2e0:	91020000 	lbu	v0,0(t0)
 2e4:	6f740610 	ldr	s4,1552(k1)
 2e8:	de010070 	ld	at,112(s0)
 2ec:	000000a8 	0xa8
 2f0:	07149102 	0x7149102
 2f4:	6c746363 	ldr	s4,25443(v1)
 2f8:	656d645f 	daddiu	t5,t3,25695
 2fc:	e001006d 	sc	at,109(zero)
 300:	000000a8 	0xa8
 304:	00009102 	srl	s2,zero,0x4
 308:	6c72010a 	ldr	s2,266(v1)
 30c:	6d695f78 	ldr	t1,24440(t3)
 310:	725f6d65 	0x725f6d65
 314:	6c696665 	ldr	t1,26213(v1)
 318:	f801006c 	sdc2	$1,108(zero)
 31c:	2009c801 	addi	t1,zero,-14335
 320:	200a6080 	addi	t2,zero,24704
 324:	0001ac80 	sll	s5,at,0x12
 328:	066e0100 	tnei	s3,256
 32c:	65736162 	daddiu	s3,t3,24930
 330:	a8f70100 	swl	s7,256(a3)
 334:	02000000 	0x2000000
 338:	74061091 	jalx	184244 <dummy0-0x8007be90>
 33c:	0100706f 	0x100706f
 340:	0000a8f7 	0xa8f7
 344:	14910200 	bne	a0,s1,b48 <dummy0-0x801ff58c>
 348:	646f6d06 	daddiu	t7,v1,27910
 34c:	f7010065 	sdc1	$f1,101(t8)
 350:	000000a8 	0xa8
 354:	07189102 	0x7189102
 358:	6c746363 	ldr	s4,25443(v1)
 35c:	656d695f 	daddiu	t5,t3,26975
 360:	f901006d 	sdc2	$1,109(t0)
 364:	000000a8 	0xa8
 368:	00009102 	srl	s2,zero,0x4
	...
Disassembly of section .debug_line.3:

00000000 <.debug_line.3>:
   0:	000000a0 	0xa0
   4:	002d0002 	0x2d0002
   8:	01010000 	0x1010000
   c:	000d0efb 	dsra	at,t5,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	5f786c72 	0x5f786c72
  20:	68636163 	ldl	v1,24931(v1)
  24:	00632e65 	0x632e65
  28:	2e000000 	sltiu	zero,s0,0
  2c:	7073622f 	0x7073622f
  30:	0000682e 	dneg	t5,zero
  34:	00000000 	nop
  38:	06700205 	bltzal	s3,850 <dummy0-0x801ff884>
  3c:	34038020 	li	v1,0x8020
  40:	023f0801 	0x23f0801
  44:	08b81430 	j	2e050c0 <dummy0-0x7d3fb014>
  48:	0810037a 	j	400de8 <dummy0-0x7fdff2ec>
  4c:	023f083c 	0x23f083c
  50:	08b81430 	j	2e050c0 <dummy0-0x7d3fb014>
  54:	0811037a 	j	440de8 <dummy0-0x7fdbf2ec>
  58:	023f083c 	0x23f083c
  5c:	08b81430 	j	2e050c0 <dummy0-0x7d3fb014>
  60:	0810037a 	j	400de8 <dummy0-0x7fdff2ec>
  64:	f34cbb3c 	scd	t4,-17604(k0)
  68:	03bcf3bb 	0x3bcf3bb
  6c:	bb3c0810 	swr	gp,2064(t9)
  70:	f3bbf34c 	scd	k1,-3252(sp)
  74:	081203bb 	j	480eec <dummy0-0x7fd7f1e8>
  78:	bc4cf43c 	cache	0xc,-3012(v0)
  7c:	173802f4 	bne	t9,t8,c50 <dummy0-0x801ff484>
  80:	ad08f48a 	sw	t0,-2934(t0)
  84:	11034bbd 	beq	t0,v1,12f7c <dummy0-0x801ed158>
  88:	3e083c08 	0x3e083c08
  8c:	f3bbf4bb 	scd	k1,-2885(sp)
  90:	081103bb 	j	440eec <dummy0-0x7fdbf1e8>
  94:	bb76083c 	swr	s6,2108(k1)
  98:	f3bbf4bc 	scd	k1,-2884(sp)
  9c:	02bcf3bc 	0x2bcf3bc
  a0:	01010014 	dsllv	zero,at,t0
Disassembly of section .debug_aranges.4:

00000000 <.debug_aranges.4>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80200670 	lb	zero,1648(at)
  14:	000003f0 	tge	zero,zero,0xf
	...
Disassembly of section .debug_str:

00000000 <.debug_str>:
   0:	72646461 	0x72646461
   4:	00737365 	0x737365
   8:	6c746363 	ldr	s4,25443(v1)
   c:	6c61765f 	ldr	at,30303(v1)
  10:	736e7500 	0x736e7500
  14:	656e6769 	daddiu	t6,t3,26473
  18:	6e692064 	ldr	t1,8292(s3)
  1c:	6e750074 	ldr	s5,116(s3)
  20:	6e676973 	ldr	a3,26995(s3)
  24:	69206465 	ldl	zero,25701(t1)
  28:	Address 0x0000000000000028 is out of bounds.

Disassembly of section .debug_line.5:

00000000 <.debug_line.5>:
   0:	00000048 	0x48
   4:	001d0002 	srl	zero,sp,0x0
   8:	01010000 	0x1010000
   c:	000d0efb 	dsra	at,t5,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	30747263 	andi	s4,v1,0x7263
  20:	0000532e 	0x532e
  24:	00000000 	nop
  28:	00000205 	0x205
  2c:	28038020 	slti	v1,zero,-32736
  30:	bb4b4b01 	swr	t3,19201(k0)
  34:	504b4b83 	beql	v0,t3,12e44 <dummy0-0x801ed290>
  38:	4c4b834b 	0x4c4b834b
  3c:	4b4b4b4b 	c2	0x14b4b4b
  40:	83848951 	lb	a0,-30383(gp)
  44:	024b504b 	0x24b504b
  48:	01010004 	sllv	zero,at,t0
Disassembly of section .debug_info.6:

00000000 <.debug_info.6>:
   0:	0000004e 	0x4e
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	80200000 	lb	zero,0(at)
  14:	80200078 	lb	zero,120(at)
  18:	30747263 	andi	s4,v1,0x7263
  1c:	2f00532e 	sltiu	zero,t8,21294
  20:	656d6f68 	daddiu	t5,t3,28520
  24:	6968702f 	ldl	t0,28719(t3)
  28:	2f78656e 	sltiu	t8,k1,25966
  2c:	534f4375 	beql	k0,t7,10e04 <dummy0-0x801ef2d0>
  30:	652f4949 	daddiu	t7,t1,18761
  34:	616c667a 	daddi	t4,t3,26234
  38:	47006873 	c1	0x1006873
  3c:	4120554e 	0x4120554e
  40:	2e322053 	sltiu	s2,s1,8275
  44:	392e3631 	xori	t6,t1,0x3631
  48:	2e312d34 	sltiu	s1,s1,11572
  4c:	00362e33 	tltu	at,s6,0xb8
  50:	Address 0x0000000000000050 is out of bounds.

Disassembly of section .debug_abbrev.7:

00000000 <.debug_abbrev.7>:
   0:	10001101 	b	4408 <dummy0-0x801fbccc>
   4:	12011106 	beq	s0,at,4420 <dummy0-0x801fbcb4>
   8:	1b080301 	0x1b080301
   c:	13082508 	beq	t8,t0,9430 <dummy0-0x801f6ca4>
  10:	00000005 	0x5
Disassembly of section .debug_aranges.8:

00000000 <.debug_aranges.8>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80200000 	lb	zero,0(at)
  14:	00000078 	dsll	zero,zero,0x1
	...
Disassembly of section .debug_line.9:

00000000 <.debug_line.9>:
   0:	00000339 	0x339
   4:	001c0002 	srl	zero,gp,0x0
   8:	01010000 	0x1010000
   c:	000d0efb 	dsra	at,t5,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	00010000 	sll	zero,at,0x0
  1c:	2e697073 	sltiu	t1,s3,28787
  20:	00000053 	0x53
  24:	05000000 	bltz	t0,28 <dummy0-0x802000ac>
  28:	200a6002 	addi	t2,zero,24578
  2c:	4d4b1a80 	0x4d4b1a80
  30:	834b4b4b 	lb	t3,19275(k0)
  34:	034b4b4c 	syscall	0xd2d2d
  38:	4c4b820b 	0x4c4b820b
  3c:	4b4b4b4b 	c2	0x14b4b4b
  40:	4c4b4b4b 	0x4c4b4b4b
  44:	4b834c4b 	c2	0x1834c4b
  48:	4b4b4c83 	c2	0x14b4c83
  4c:	034b4b4c 	syscall	0xd2d2d
  50:	4d4b820a 	0x4d4b820a
  54:	4b4b4b4b 	c2	0x14b4b4b
  58:	4b4c4b4b 	c2	0x14c4b4b
  5c:	4b4c834c 	c2	0x14c834c
  60:	834b834c 	lb	t3,-31924(k0)
  64:	034b4b4c 	syscall	0xd2d2d
  68:	4d4b820a 	0x4d4b820a
  6c:	4b4b4b4b 	c2	0x14b4b4b
  70:	4b4c4b4b 	c2	0x14c4b4b
  74:	4d4c834c 	0x4d4c834c
  78:	4b4b4b4b 	c2	0x14b4b4b
  7c:	4b4c4d4b 	c2	0x14c4d4b
  80:	4c4b4c4b 	0x4c4b4c4b
  84:	4c834b83 	0x4c834b83
  88:	0b034b4b 	j	c0d2d2c <dummy0-0x7412d3a8>
  8c:	4b4d4b82 	c2	0x14d4b82
  90:	4b4b4b4b 	c2	0x14b4b4b
  94:	4c4b4c4b 	0x4c4b4c4b
  98:	4b4d4c83 	c2	0x14d4c83
  9c:	4b4b4b4b 	c2	0x14b4b4b
  a0:	4b4b4c4d 	c2	0x14b4c4d
  a4:	834c4b4c 	lb	t4,19276(k0)
  a8:	4b4c834b 	c2	0x14c834b
  ac:	820a034b 	lb	t2,843(s0)
  b0:	4b4b4d4b 	c2	0x14b4d4b
  b4:	4b4b4b4b 	c2	0x14b4b4b
  b8:	834c4b4c 	lb	t4,19276(k0)
  bc:	4b4b4d4c 	c2	0x14b4d4c
  c0:	4d4b4b4b 	0x4d4b4b4b
  c4:	4c4b4b4c 	0x4c4b4b4c
  c8:	4b834c4b 	c2	0x1834c4b
  cc:	4b4b4c83 	c2	0x14b4c83
  d0:	4b820a03 	c2	0x1820a03
  d4:	4b4b4b4d 	c2	0x14b4b4d
  d8:	4c4b4b4b 	0x4c4b4b4b
  dc:	4c834c4b 	0x4c834c4b
  e0:	4b4b4b4d 	c2	0x14b4b4d
  e4:	4c4d4b4b 	0x4c4d4b4b
  e8:	4b4c4b4b 	c2	0x14c4b4b
  ec:	834b834c 	lb	t3,-31924(k0)
  f0:	034b4b4c 	syscall	0xd2d2d
  f4:	4d4b820a 	0x4d4b820a
  f8:	4b4b4b4b 	c2	0x14b4b4b
  fc:	4b4c4b4b 	c2	0x14c4b4b
 100:	4d4c834c 	0x4d4c834c
 104:	4b4b4b4b 	c2	0x14b4b4b
 108:	4b4c4d4b 	c2	0x14c4d4b
 10c:	4c4b4c4b 	0x4c4b4c4b
 110:	4c834b83 	0x4c834b83
 114:	0a034b4b 	j	80d2d2c <dummy0-0x7812d3a8>
 118:	4b4c4b82 	c2	0x14c4b82
 11c:	4b4b4b4b 	c2	0x14b4b4b
 120:	4b4d4b4b 	c2	0x14d4b4b
 124:	4b4c834c 	c2	0x14c834c
 128:	4c834b4c 	0x4c834b4c
 12c:	0b034b4b 	j	c0d2d2c <dummy0-0x7412d3a8>
 130:	4b4d4b82 	c2	0x14d4b82
 134:	4b4b4b4b 	c2	0x14b4b4b
 138:	4c4b4c4b 	0x4c4b4c4b
 13c:	4b4c4c83 	c2	0x14c4c83
 140:	4c4b4c4b 	0x4c4b4c4b
 144:	4c834b83 	0x4c834b83
 148:	0a034b4b 	j	80d2d2c <dummy0-0x7812d3a8>
 14c:	4b4d4b82 	c2	0x14d4b82
 150:	4b4b4b4b 	c2	0x14b4b4b
 154:	4c4b4c4b 	0x4c4b4c4b
 158:	4c834b83 	0x4c834b83
 15c:	0a034b4b 	j	80d2d2c <dummy0-0x7812d3a8>
 160:	4b4d4b82 	c2	0x14d4b82
 164:	4b4b4b4b 	c2	0x14b4b4b
 168:	4c4b4c4b 	0x4c4b4c4b
 16c:	4c834b83 	0x4c834b83
 170:	0b034b4b 	j	c0d2d2c <dummy0-0x7412d3a8>
 174:	834c4b82 	lb	t4,19330(k0)
 178:	4b4b4d4c 	c2	0x14b4d4c
 17c:	4b4c4b4b 	c2	0x14c4b4b
 180:	4c4c834c 	0x4c4c834c
 184:	4b4b4b4b 	c2	0x14b4b4b
 188:	4c4c4b4b 	0x4c4c4b4b
 18c:	4c834b83 	0x4c834b83
 190:	0b034b4b 	j	c0d2d2c <dummy0-0x7412d3a8>
 194:	4b4d4b82 	c2	0x14d4b82
 198:	4b4b4b4b 	c2	0x14b4b4b
 19c:	4c4b4b4b 	0x4c4b4b4b
 1a0:	4c834c4b 	0x4c834c4b
 1a4:	4c4b4c4b 	0x4c4b4c4b
 1a8:	4b4b4c4b 	c2	0x14b4c4b
 1ac:	4b4c4b4c 	c2	0x14c4b4c
 1b0:	034b4b4c 	syscall	0xd2d2d
 1b4:	4d4b820b 	0x4d4b820b
 1b8:	4b4b4b4b 	c2	0x14b4b4b
 1bc:	834c4b4b 	lb	t4,19275(k0)
 1c0:	4b4d4b4c 	c2	0x14d4b4c
 1c4:	4c4b4b4b 	0x4c4b4b4b
 1c8:	4d834c4b 	0x4d834c4b
 1cc:	4b4c4b4b 	c2	0x14c4b4b
 1d0:	820d034b 	lb	t5,843(s0)
 1d4:	4b4b4d4b 	c2	0x14b4d4b
 1d8:	4b4b4b4b 	c2	0x14b4b4b
 1dc:	4d4c834c 	0x4d4c834c
 1e0:	4b4b4b4b 	c2	0x14b4b4b
 1e4:	834c4b4c 	lb	t4,19276(k0)
 1e8:	4b4b4b4d 	c2	0x14b4b4d
 1ec:	4b4b4b4c 	c2	0x14b4b4c
 1f0:	034b4b4c 	syscall	0xd2d2d
 1f4:	4d4b820a 	0x4d4b820a
 1f8:	4b4b4b4b 	c2	0x14b4b4b
 1fc:	4b4c4b4b 	c2	0x14c4b4b
 200:	4b4c834c 	c2	0x14c834c
 204:	820a034b 	lb	t2,843(s0)
 208:	4b4b4d4b 	c2	0x14b4d4b
 20c:	4b4b4b4b 	c2	0x14b4b4b
 210:	834c4b4c 	lb	t4,19276(k0)
 214:	034b4b4c 	syscall	0xd2d2d
 218:	4d4b820c 	0x4d4b820c
 21c:	4b4b4b4b 	c2	0x14b4b4b
 220:	4b4d4b4b 	c2	0x14d4b4b
 224:	4d4c834c 	0x4d4c834c
 228:	4b4b4b4b 	c2	0x14b4b4b
 22c:	4c4d4c4b 	0x4c4d4c4b
 230:	4c4c4b4b 	0x4c4c4b4b
 234:	4b834c4b 	c2	0x1834c4b
 238:	4b4b4c83 	c2	0x14b4c83
 23c:	4b820c03 	c2	0x1820c03
 240:	4b4b4b4d 	c2	0x14b4b4d
 244:	4c4b4b4b 	0x4c4b4b4b
 248:	4b4d4d83 	c2	0x14d4d83
 24c:	4c4b4b4b 	0x4c4b4b4b
 250:	4b4b4b4d 	c2	0x14b4b4d
 254:	834c4b4c 	lb	t4,19276(k0)
 258:	4b4c844d 	c2	0x14c844d
 25c:	820c034b 	lb	t4,843(s0)
 260:	4b4b4d4b 	c2	0x14b4d4b
 264:	4b4b4b4b 	c2	0x14b4b4b
 268:	4b4b4d4c 	c2	0x14b4d4c
 26c:	4b4d4b4b 	c2	0x14d4b4b
 270:	4b4b4b4b 	c2	0x14b4b4b
 274:	834c4b4c 	lb	t4,19276(k0)
 278:	4c834b51 	0x4c834b51
 27c:	034b4b4c 	syscall	0xd2d2d
 280:	4d4b820c 	0x4d4b820c
 284:	4b4b4b4b 	c2	0x14b4b4b
 288:	4b4c4b4b 	c2	0x14c4b4b
 28c:	4d4c834c 	0x4d4c834c
 290:	4b4b4b4b 	c2	0x14b4b4b
 294:	4b4c4d4b 	c2	0x14c4d4b
 298:	4b4c4d4b 	c2	0x14c4d4b
 29c:	4b844b4b 	c2	0x1844b4b
 2a0:	834b834c 	lb	t3,-31924(k0)
 2a4:	034b4b4c 	syscall	0xd2d2d
 2a8:	4d4b820d 	0x4d4b820d
 2ac:	4b4b4b4b 	c2	0x14b4b4b
 2b0:	4b4e4b4b 	c2	0x14e4b4b
 2b4:	4c4c834c 	0x4c4c834c
 2b8:	4b4b4b4b 	c2	0x14b4b4b
 2bc:	4b4c4b4d 	c2	0x14c4b4d
 2c0:	4c4b4d4b 	0x4c4b4d4b
 2c4:	4b4c4b4b 	c2	0x14c4b4b
 2c8:	4b4b4c4b 	c2	0x14b4c4b
 2cc:	834c4b4c 	lb	t4,19276(k0)
 2d0:	4b4c834c 	c2	0x14c834c
 2d4:	820c034c 	lb	t4,844(s0)
 2d8:	4b4d4b4b 	c2	0x14d4b4b
 2dc:	4b4b4b4b 	c2	0x14b4b4b
 2e0:	4b4c4d4b 	c2	0x14c4d4b
 2e4:	4d4b4b4b 	0x4d4b4b4b
 2e8:	4b4b4b4b 	c2	0x14b4b4b
 2ec:	834c4b4c 	lb	t4,19276(k0)
 2f0:	4c834b4d 	0x4c834b4d
 2f4:	834b4b83 	lb	t3,19331(k0)
 2f8:	4b4b834c 	c2	0x14b834c
 2fc:	4b834c83 	c2	0x1834c83
 300:	4c4c834b 	0x4c4c834b
 304:	034c4b4b 	0x34c4b4b
 308:	4d4b820c 	0x4d4b820c
 30c:	4b4b4b4b 	c2	0x14b4b4b
 310:	4e4c4b4b 	c3	0x4c4b4b
 314:	4b4b4b4b 	c2	0x14b4b4b
 318:	4b4b4b4d 	c2	0x14b4b4d
 31c:	4b4c4b4b 	c2	0x14c4b4b
 320:	4b4e834c 	c2	0x14e834c
 324:	4b834c83 	c2	0x1834c83
 328:	834c834b 	lb	t4,-31925(k0)
 32c:	4c834b4b 	0x4c834b4b
 330:	834b4b83 	lb	t3,19331(k0)
 334:	4b4b4c4c 	c2	0x14b4c4c
 338:	01000802 	0x1000802
 33c:	Address 0x000000000000033c is out of bounds.

Disassembly of section .debug_info.10:

00000000 <.debug_info.10>:
   0:	0000004d 	break	0x0,0x1
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	80200a60 	lb	zero,2656(at)
  14:	80201750 	lb	zero,5968(at)
  18:	2e697073 	sltiu	t1,s3,28787
  1c:	682f0053 	ldl	t7,83(at)
  20:	2f656d6f 	sltiu	a1,k1,28015
  24:	6e696870 	ldr	t1,26736(s3)
  28:	752f7865 	jalx	4bde194 <dummy0-0x7b621f40>
  2c:	49534f43 	0x49534f43
  30:	7a652f49 	0x7a652f49
  34:	73616c66 	0x73616c66
  38:	4e470068 	c3	0x470068
  3c:	53412055 	beql	k0,at,8194 <dummy0-0x801f7f40>
  40:	312e3220 	andi	t6,t1,0x3220
  44:	34392e36 	ori	t9,at,0x2e36
  48:	332e312d 	andi	t6,t9,0x312d
  4c:	0100362e 	0x100362e
  50:	Address 0x0000000000000050 is out of bounds.

Disassembly of section .debug_abbrev.11:

00000000 <.debug_abbrev.11>:
   0:	10001101 	b	4408 <dummy0-0x801fbccc>
   4:	12011106 	beq	s0,at,4420 <dummy0-0x801fbcb4>
   8:	1b080301 	0x1b080301
   c:	13082508 	beq	t8,t0,9430 <dummy0-0x801f6ca4>
  10:	00000005 	0x5
Disassembly of section .debug_aranges.12:

00000000 <.debug_aranges.12>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80200a60 	lb	zero,2656(at)
  14:	00000cf0 	tge	zero,zero,0x33
	...
Disassembly of section .debug_abbrev.13:

00000000 <.debug_abbrev.13>:
   0:	10011101 	beq	zero,at,4408 <dummy0-0x801fbccc>
   4:	11011206 	beq	t0,at,4820 <dummy0-0x801fb8b4>
   8:	13082501 	beq	t8,t0,9410 <dummy0-0x801f6cc4>
   c:	1b08030b 	0x1b08030b
  10:	02000008 	jr	s0
  14:	08030024 	j	c0090 <dummy0-0x80140044>
  18:	0b3e0b0b 	j	cf82c2c <dummy0-0x7327d4a8>
  1c:	24030000 	li	v1,0
  20:	0b0e0300 	j	c380c00 <dummy0-0x73e7f4d4>
  24:	000b3e0b 	0xb3e0b
  28:	000f0400 	sll	zero,t7,0x10
  2c:	13490b0b 	beq	k0,t1,2c5c <dummy0-0x801fd478>
  30:	16050000 	bne	s0,a1,34 <dummy0-0x802000a0>
  34:	3a080300 	xori	t0,s0,0x300
  38:	490b3b0b 	0x490b3b0b
  3c:	06000013 	bltz	s0,8c <dummy0-0x80200048>
  40:	1301012e 	beq	t8,at,4fc <dummy0-0x801ffbd8>
  44:	08030c3f 	j	c30fc <dummy0-0x8013cfd8>
  48:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  4c:	13490c27 	beq	k0,t1,30ec <dummy0-0x801fcfe8>
  50:	01120111 	0x1120111
  54:	40064081 	0x40064081
  58:	0700000a 	bltz	t8,84 <dummy0-0x80200050>
  5c:	08030005 	j	c0014 <dummy0-0x801400c0>
  60:	0b3b0b3a 	j	cec2ce8 <dummy0-0x7333d3ec>
  64:	0a021349 	j	8084d24 <dummy0-0x7817b3b0>
  68:	34080000 	li	t0,0x0
  6c:	3a080300 	xori	t0,s0,0x300
  70:	490b3b0b 	0x490b3b0b
  74:	000a0213 	0xa0213
  78:	00260900 	0x260900
  7c:	Address 0x000000000000007c is out of bounds.

Disassembly of section .debug_info.14:

00000000 <.debug_info.14>:
   0:	000001ae 	0x1ae
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	8020178c 	lb	zero,6028(at)
  14:	80201750 	lb	zero,5968(at)
  18:	20554e47 	addi	s5,v0,20039
  1c:	2e332043 	sltiu	s3,s1,8259
  20:	2d362e34 	sltiu	s6,t1,11828
  24:	2e332e31 	sltiu	s3,s1,11825
  28:	672d2033 	daddiu	t5,t9,8243
  2c:	2e2e0100 	sltiu	t6,s1,256
  30:	2f2e2e2f 	sltiu	t6,t9,11823
  34:	2e2f2e2e 	sltiu	t7,s1,11822
  38:	2e2e2f2e 	sltiu	t6,s1,12078
  3c:	77656e2f 	jalx	d95b8bc <dummy0-0x728a4818>
  40:	2f62696c 	sltiu	v0,k1,26988
  44:	6c77656e 	ldr	s7,25966(v1)
  48:	6c2f6269 	ldr	t7,25193(at)
  4c:	2f636269 	sltiu	v1,k1,25193
  50:	69727473 	ldl	s2,29811(t3)
  54:	6d2f676e 	ldr	t7,26478(t1)
  58:	6d636d65 	ldr	v1,28005(t3)
  5c:	00632e70 	tge	v1,v1,0xb9
  60:	6d6f682f 	ldr	t7,26671(t3)
  64:	64412f65 	daddiu	at,v0,12133
  68:	696e696d 	ldl	t6,26989(t3)
  6c:	61727473 	daddi	s2,t3,29811
  70:	2f726f74 	sltiu	s2,k1,28532
  74:	6c77656e 	ldr	s7,25966(v1)
  78:	696d6269 	ldl	t5,25193(t3)
  7c:	6d2f7370 	ldr	t7,29552(t1)
  80:	2d737069 	sltiu	s3,t3,28777
  84:	2f666c65 	sltiu	a2,k1,27749
  88:	6c77656e 	ldr	s7,25966(v1)
  8c:	6c2f6269 	ldr	t7,25193(at)
  90:	2f636269 	sltiu	v1,k1,25193
  94:	69727473 	ldl	s2,29811(t3)
  98:	0200676e 	0x200676e
  9c:	00746e69 	0x746e69
  a0:	6c020504 	ldr	v0,1284(zero)
  a4:	20676e6f 	addi	a3,v1,28271
  a8:	00746e69 	0x746e69
  ac:	6c020504 	ldr	v0,1284(zero)
  b0:	20676e6f 	addi	a3,v1,28271
  b4:	676e6f6c 	daddiu	t6,k1,28524
  b8:	746e6920 	jalx	1b9a480 <dummy0-0x7e665c54>
  bc:	03050800 	0x3050800
  c0:	00000011 	mthi	zero
  c4:	11030704 	beq	t0,v1,1cd8 <dummy0-0x801fe3fc>
  c8:	04000000 	bltz	zero,cc <dummy0-0x80200008>
  cc:	6e750207 	ldr	s5,519(s3)
  d0:	6e676973 	ldr	a3,26995(s3)
  d4:	63206465 	daddi	zero,t9,25701
  d8:	00726168 	0x726168
  dc:	6c020801 	ldr	v0,2049(zero)
  e0:	20676e6f 	addi	a3,v1,28271
  e4:	69736e75 	ldl	s3,28277(t3)
  e8:	64656e67 	daddiu	a1,v1,28263
  ec:	746e6920 	jalx	1b9a480 <dummy0-0x7e665c54>
  f0:	04070400 	0x4070400
  f4:	0000cd04 	0xcd04
  f8:	68730200 	ldl	s3,512(v1)
  fc:	2074726f 	addi	s4,v1,29295
 100:	00746e69 	0x746e69
 104:	63020502 	daddi	v0,t8,1282
 108:	00726168 	0x726168
 10c:	73020601 	0x73020601
 110:	74726f68 	jalx	1c9bda0 <dummy0-0x7e564334>
 114:	736e7520 	0x736e7520
 118:	656e6769 	daddiu	t6,t3,26473
 11c:	6e692064 	ldr	t1,8292(s3)
 120:	07020074 	bltzl	t8,2f4 <dummy0-0x801ffde0>
 124:	6e6f6c02 	ldr	t7,27650(s3)
 128:	6f6c2067 	ldr	t4,8295(k1)
 12c:	7520676e 	jalx	4819db8 <dummy0-0x7b9e631c>
 130:	6769736e 	daddiu	t1,k1,29550
 134:	2064656e 	addi	a0,v1,25966
 138:	00746e69 	0x746e69
 13c:	73050708 	0x73050708
 140:	5f657a69 	0x5f657a69
 144:	d5020074 	ldc1	$f2,116(t0)
 148:	000000bf 	dsra32	zero,zero,0x2
 14c:	0001aa06 	0x1aa06
 150:	656d0100 	daddiu	t5,t3,256
 154:	706d636d 	0x706d636d
 158:	01390100 	0x1390100
 15c:	0000009b 	0x9b
 160:	80201750 	lb	zero,5968(at)
 164:	8020178c 	lb	zero,6028(at)
 168:	000001d8 	0x1d8
 16c:	6d076d01 	ldr	a3,27905(t0)
 170:	35010031 	ori	at,t0,0x31
 174:	000001aa 	0x1aa
 178:	6d075401 	ldr	a3,21505(t0)
 17c:	35010032 	ori	at,t0,0x32
 180:	000001aa 	0x1aa
 184:	6e075501 	ldr	a3,21761(s0)
 188:	3e350100 	0x3e350100
 18c:	01000001 	0x1000001
 190:	31730853 	andi	s3,t3,0x853
 194:	f33b0100 	scd	k1,256(t9)
 198:	01000000 	0x1000000
 19c:	32730854 	andi	s3,s3,0x854
 1a0:	f33c0100 	scd	gp,256(t9)
 1a4:	01000000 	0x1000000
 1a8:	04040055 	0x4040055
 1ac:	000001b0 	tge	zero,zero,0x6
 1b0:	Address 0x00000000000001b0 is out of bounds.

Disassembly of section .debug_line.15:

00000000 <.debug_line.15>:
   0:	000000c0 	sll	zero,zero,0x3
   4:	00a10002 	0xa10002
   8:	01010000 	0x1010000
   c:	000a0efb 	dsra	at,t2,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	2e2f2e2e 	sltiu	t7,s1,11822
  1c:	2e2e2f2e 	sltiu	t6,s1,12078
  20:	2f2e2e2f 	sltiu	t6,t9,11823
  24:	6e2f2e2e 	ldr	t7,11822(s1)
  28:	696c7765 	ldl	t4,30565(t3)
  2c:	656e2f62 	daddiu	t6,t3,12130
  30:	62696c77 	daddi	t1,s3,27767
  34:	62696c2f 	daddi	t1,s3,27695
  38:	74732f63 	jalx	1ccbd8c <dummy0-0x7e534348>
  3c:	676e6972 	daddiu	t6,k1,26994
  40:	73752f00 	0x73752f00
  44:	6f6c2f72 	ldr	t4,12146(k1)
  48:	2f6c6163 	sltiu	t4,k1,24931
  4c:	6b647372 	ldl	a0,29554(k1)
  50:	332e312d 	andi	t6,t9,0x312d
  54:	632f332e 	daddi	t7,t9,13102
  58:	69776779 	ldl	s7,26489(t3)
  5c:	656e2f6e 	daddiu	t6,t3,12142
  60:	62696c77 	daddi	t1,s3,27767
  64:	6e69622f 	ldr	t1,25135(s3)
  68:	2f2e2e2f 	sltiu	t6,t9,11823
  6c:	2f62696c 	sltiu	v0,k1,26988
  70:	2f636367 	sltiu	v1,k1,25447
  74:	7370696d 	0x7370696d
  78:	666c652d 	daddiu	t4,s3,25901
  7c:	342e332f 	ori	t6,at,0x332f
  80:	312d362e 	andi	t5,t1,0x362e
  84:	332e332e 	andi	t6,t9,0x332e
  88:	636e692f 	daddi	t6,k1,26927
  8c:	6564756c 	daddiu	a0,t3,30060
  90:	656d0000 	daddiu	t5,t3,0
  94:	706d636d 	0x706d636d
  98:	0100632e 	0x100632e
  9c:	74730000 	jalx	1cc0000 <dummy0-0x7e5400d4>
  a0:	66656464 	daddiu	a1,s3,25700
  a4:	0200682e 	dsub	t5,s0,zero
  a8:	00000000 	nop
  ac:	17500205 	bne	k0,s0,8c4 <dummy0-0x801ff810>
  b0:	c4038020 	lwc1	$f3,-32736(zero)
  b4:	84b20100 	lh	s2,256(a1)
  b8:	82814346 	lb	at,17222(s4)
  bc:	027f2b03 	0x27f2b03
  c0:	01010008 	0x1010008
Disassembly of section .debug_aranges.16:

00000000 <.debug_aranges.16>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	80201750 	lb	zero,5968(at)
  14:	0000003c 	dsll32	zero,zero,0x0
	...
Disassembly of section .debug_abbrev.17:

00000000 <.debug_abbrev.17>:
   0:	10011101 	beq	zero,at,4408 <dummy0-0x801fbccc>
   4:	11011206 	beq	t0,at,4820 <dummy0-0x801fb8b4>
   8:	13082501 	beq	t8,t0,9410 <dummy0-0x801f6cc4>
   c:	1b08030b 	0x1b08030b
  10:	02000008 	jr	s0
  14:	08030024 	j	c0090 <dummy0-0x80140044>
  18:	0b3e0b0b 	j	cf82c2c <dummy0-0x7327d4a8>
  1c:	24030000 	li	v1,0
  20:	0b0e0300 	j	c380c00 <dummy0-0x73e7f4d4>
  24:	000b3e0b 	0xb3e0b
  28:	000f0400 	sll	zero,t7,0x10
  2c:	00000b0b 	0xb0b
  30:	0b000f05 	j	c003c14 <dummy0-0x741fc4c0>
  34:	0013490b 	0x13490b
  38:	00160600 	sll	zero,s6,0x18
  3c:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  40:	13490b3b 	beq	k0,t1,2d30 <dummy0-0x801fd3a4>
  44:	2e070000 	sltiu	a3,s0,0
  48:	3f130101 	0x3f130101
  4c:	3a08030c 	xori	t0,s0,0x30c
  50:	270b3b0b 	addiu	t3,t8,15115
  54:	1113490c 	beq	t0,s3,12488 <dummy0-0x801edc4c>
  58:	81011201 	lb	at,4609(t0)
  5c:	0a400640 	j	9001900 <dummy0-0x771fe7d4>
  60:	05080000 	tgei	t0,0
  64:	3a080300 	xori	t0,s0,0x300
  68:	490b3b0b 	0x490b3b0b
  6c:	000a0213 	0xa0213
  70:	00340900 	0x340900
  74:	0b3a0803 	j	ce8200c <dummy0-0x7337e0c8>
  78:	13490b3b 	beq	k0,t1,2d68 <dummy0-0x801fd36c>
  7c:	00000a02 	srl	at,zero,0x8
  80:	0300340a 	0x300340a
  84:	3b0b3a08 	xori	t3,t8,0x3a08
  88:	0013490b 	0x13490b
  8c:	010b0b00 	0x10b0b00
  90:	01120111 	0x1120111
  94:	Address 0x0000000000000094 is out of bounds.

Disassembly of section .debug_info.18:

00000000 <.debug_info.18>:
   0:	00000221 	0x221
   4:	00000002 	srl	zero,zero,0x0
   8:	01040000 	0x1040000
   c:	00000000 	nop
  10:	80201830 	lb	zero,6192(at)
  14:	8020178c 	lb	zero,6028(at)
  18:	20554e47 	addi	s5,v0,20039
  1c:	2e332043 	sltiu	s3,s1,8259
  20:	2d362e34 	sltiu	s6,t1,11828
  24:	2e332e31 	sltiu	s3,s1,11825
  28:	672d2033 	daddiu	t5,t9,8243
  2c:	2e2e0100 	sltiu	t6,s1,256
  30:	2f2e2e2f 	sltiu	t6,t9,11823
  34:	2e2f2e2e 	sltiu	t7,s1,11822
  38:	2e2e2f2e 	sltiu	t6,s1,12078
  3c:	2f2e2e2f 	sltiu	t6,t9,11823
  40:	6c77656e 	ldr	s7,25966(v1)
  44:	6e2f6269 	ldr	t7,25193(s1)
  48:	696c7765 	ldl	t4,30565(t3)
  4c:	696c2f62 	ldl	t4,12130(t3)
  50:	6d2f6362 	ldr	t7,25442(t1)
  54:	69686361 	ldl	t0,25441(t3)
  58:	6d2f656e 	ldr	t7,25966(t1)
  5c:	2f737069 	sltiu	s3,k1,28777
  60:	736d656d 	0x736d656d
  64:	632e7465 	daddi	t6,t9,29797
  68:	6f682f00 	ldr	t0,12032(k1)
  6c:	412f656d 	0x412f656d
  70:	6e696d64 	ldr	t1,28004(s3)
  74:	72747369 	0x72747369
  78:	726f7461 	0x726f7461
  7c:	77656e2f 	jalx	d95b8bc <dummy0-0x728a4818>
  80:	6d62696c 	ldr	v0,26988(t3)
  84:	2f737069 	sltiu	s3,k1,28777
  88:	7370696d 	0x7370696d
  8c:	666c652d 	daddiu	t4,s3,25901
  90:	77656e2f 	jalx	d95b8bc <dummy0-0x728a4818>
  94:	2f62696c 	sltiu	v0,k1,26988
  98:	6362696c 	daddi	v0,k1,26988
  9c:	63616d2f 	daddi	at,k1,27951
  a0:	656e6968 	daddiu	t6,t3,26984
  a4:	70696d2f 	0x70696d2f
  a8:	69020073 	ldl	v0,115(t0)
  ac:	0400746e 	bltz	zero,1d268 <dummy0-0x801e2e6c>
  b0:	6f6c0205 	ldr	t4,517(k1)
  b4:	6920676e 	ldl	zero,26478(t1)
  b8:	0400746e 	bltz	zero,1d274 <dummy0-0x801e2e60>
  bc:	6f6c0205 	ldr	t4,517(k1)
  c0:	6c20676e 	ldr	zero,26478(at)
  c4:	20676e6f 	addi	a3,v1,28271
  c8:	00746e69 	0x746e69
  cc:	1e030508 	0x1e030508
  d0:	04000000 	bltz	zero,d4 <dummy0-0x80200000>
  d4:	001e0307 	0x1e0307
  d8:	07040000 	0x7040000
  dc:	736e7502 	0x736e7502
  e0:	656e6769 	daddiu	t6,t3,26473
  e4:	68632064 	ldl	v1,8292(v1)
  e8:	01007261 	0x1007261
  ec:	02040408 	0x2040408
  f0:	676e6f6c 	daddiu	t6,k1,28524
  f4:	736e7520 	0x736e7520
  f8:	656e6769 	daddiu	t6,t3,26473
  fc:	6e692064 	ldr	t1,8292(s3)
 100:	07040074 	0x7040074
 104:	6f687302 	ldr	t0,29442(k1)
 108:	69207472 	ldl	zero,29810(t1)
 10c:	0200746e 	0x200746e
 110:	17040505 	bne	t8,a0,1528 <dummy0-0x801febac>
 114:	02000001 	0x2000001
 118:	72616863 	0x72616863
 11c:	02060100 	0x2060100
 120:	726f6873 	0x726f6873
 124:	6e752074 	ldr	s5,8308(s3)
 128:	6e676973 	ldr	a3,26995(s3)
 12c:	69206465 	ldl	zero,25701(t1)
 130:	0200746e 	0x200746e
 134:	6f6c0207 	ldr	t4,519(k1)
 138:	6c20676e 	ldr	zero,26478(at)
 13c:	20676e6f 	addi	a3,v1,28271
 140:	69736e75 	ldl	s3,28277(t3)
 144:	64656e67 	daddiu	a1,v1,28263
 148:	746e6920 	jalx	1b9a480 <dummy0-0x7e665c54>
 14c:	06070800 	0x6070800
 150:	657a6973 	daddiu	k0,t3,26995
 154:	0200745f 	0x200745f
 158:	0000ced5 	0xced5
 15c:	02180700 	0x2180700
 160:	6d010000 	ldr	at,0(t0)
 164:	65736d65 	daddiu	s3,t3,28005
 168:	35010074 	ori	at,t0,0x74
 16c:	0000ed01 	0xed01
 170:	20178c00 	addi	s7,zero,-29696
 174:	20183080 	addi	t8,zero,12416
 178:	0001f880 	sll	ra,at,0x2
 17c:	086d0100 	j	1b40400 <dummy0-0x7e6bfcd4>
 180:	3101006d 	andi	at,t0,0x6d
 184:	000000ed 	0xed
 188:	63085401 	daddi	t0,t8,21505
 18c:	aa310100 	swl	s1,256(s1)
 190:	01000000 	0x1000000
 194:	006e0855 	0x6e0855
 198:	014f3101 	0x14f3101
 19c:	56010000 	bnel	s0,at,1a0 <dummy0-0x801fff34>
 1a0:	01007309 	0x1007309
 1a4:	00011140 	sll	v0,at,0x5
 1a8:	0a570100 	j	95c0400 <dummy0-0x76c3fcd4>
 1ac:	41010069 	bc0t	354 <dummy0-0x801ffd80>
 1b0:	000000aa 	0xaa
 1b4:	66756209 	daddiu	s5,s3,25097
 1b8:	00726566 	0x726566
 1bc:	00ef4201 	0xef4201
 1c0:	53010000 	beql	t8,at,1c4 <dummy0-0x801fff10>
 1c4:	696c6109 	ldl	t4,24841(t3)
 1c8:	64656e67 	daddiu	a1,v1,28263
 1cc:	6464615f 	daddiu	a0,v1,24927
 1d0:	43010072 	c0	0x1010072
 1d4:	00000218 	0x218
 1d8:	730a5701 	0x730a5701
 1dc:	74726f68 	jalx	1c9bda0 <dummy0-0x7e564334>
 1e0:	6464615f 	daddiu	a0,v1,24927
 1e4:	44010072 	0x44010072
 1e8:	0000021e 	0x21e
 1ec:	65746909 	daddiu	s4,t3,26889
 1f0:	45010072 	bc1t	3bc <dummy0-0x801ffd18>
 1f4:	0000014f 	0x14f
 1f8:	980b5201 	lwr	t3,20993(zero)
 1fc:	14802017 	bnez	a0,825c <dummy0-0x801f7e78>
 200:	09802018 	j	6008060 <dummy0-0x7a1f8074>
 204:	6c616e75 	ldr	at,28277(v1)
 208:	656e6769 	daddiu	t6,t3,26473
 20c:	49010064 	bc2t	3a0 <dummy0-0x801ffd34>
 210:	000000aa 	0xaa
 214:	00005301 	0x5301
 218:	00ef0405 	0xef0405
 21c:	04050000 	0x4050000
 220:	0000011f 	0x11f
	...
Disassembly of section .debug_line.19:

00000000 <.debug_line.19>:
   0:	000000dc 	0xdc
   4:	00aa0002 	0xaa0002
   8:	01010000 	0x1010000
   c:	000a0efb 	dsra	at,t2,0x1b
  10:	01010101 	0x1010101
  14:	01000000 	0x1000000
  18:	2e2f2e2e 	sltiu	t7,s1,11822
  1c:	2e2e2f2e 	sltiu	t6,s1,12078
  20:	2f2e2e2f 	sltiu	t6,t9,11823
  24:	2e2f2e2e 	sltiu	t7,s1,11822
  28:	656e2f2e 	daddiu	t6,t3,12078
  2c:	62696c77 	daddi	t1,s3,27767
  30:	77656e2f 	jalx	d95b8bc <dummy0-0x728a4818>
  34:	2f62696c 	sltiu	v0,k1,26988
  38:	6362696c 	daddi	v0,k1,26988
  3c:	63616d2f 	daddi	at,k1,27951
  40:	656e6968 	daddiu	t6,t3,26984
  44:	70696d2f 	0x70696d2f
  48:	752f0073 	jalx	4bc01cc <dummy0-0x7b63ff08>
  4c:	6c2f7273 	ldr	t7,29299(at)
  50:	6c61636f 	ldr	at,25455(v1)
  54:	6473722f 	daddiu	s3,v1,29231
  58:	2e312d6b 	sltiu	s1,s1,11627
  5c:	2f332e33 	sltiu	s3,t9,11827
  60:	77677963 	jalx	d9de58c <dummy0-0x72821b48>
  64:	6e2f6e69 	ldr	t7,28265(s1)
  68:	696c7765 	ldl	t4,30565(t3)
  6c:	69622f62 	ldl	v0,12130(t3)
  70:	2e2e2f6e 	sltiu	t6,s1,12142
  74:	62696c2f 	daddi	t1,s3,27695
  78:	6363672f 	daddi	v1,k1,26415
  7c:	70696d2f 	0x70696d2f
  80:	6c652d73 	ldr	a1,11635(v1)
  84:	2e332f66 	sltiu	s3,s1,12134
  88:	2d362e34 	sltiu	s6,t1,11828
  8c:	2e332e31 	sltiu	s3,s1,11825
  90:	6e692f33 	ldr	t1,12083(s3)
  94:	64756c63 	daddiu	s5,v1,27747
  98:	6d000065 	ldr	zero,101(t0)
  9c:	65736d65 	daddiu	s3,t3,28005
  a0:	00632e74 	teq	v1,v1,0xb9
  a4:	73000001 	0x73000001
  a8:	65646474 	daddiu	a0,t3,25716
  ac:	00682e66 	0x682e66
  b0:	00000002 	srl	zero,zero,0x0
  b4:	8c020500 	lw	v0,1280(zero)
  b8:	03802017 	dsrav	a0,zero,gp
  bc:	470100c6 	c1	0x10100c6
  c0:	03824c81 	0x3824c81
  c4:	49a9080b 	0x49a9080b
  c8:	0f038083 	jal	c0e020c <dummy0-0x7411fec8>
  cc:	7c4d487f 	0x7c4d487f
  d0:	b9834848 	swr	v1,18504(t4)
  d4:	0b034648 	j	c0d1920 <dummy0-0x7412e7b4>
  d8:	02bd7e47 	0x2bd7e47
  dc:	01010008 	0x1010008
Disassembly of section .debug_aranges.20:

00000000 <.debug_aranges.20>:
   0:	0000001c 	dmult	zero,zero
   4:	00000002 	srl	zero,zero,0x0
   8:	00040000 	sll	zero,a0,0x0
   c:	00000000 	nop
  10:	8020178c 	lb	zero,6028(at)
  14:	000000a4 	0xa4
	...
