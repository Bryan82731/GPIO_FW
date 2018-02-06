
./obj/dash_NT.exe:     file format elf32-littlemips

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text.unlikely 000000bc  8205f438  8205f438  00000114  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .bss          00007108  800cf2fc  82071a70  000001d0  2**2
                  ALLOC
  2 .comment      00000041  00000000  00000000  000001d0  2**0
                  CONTENTS, READONLY
  3 .reginfo      00000018  00000000  00000000  00000214  2**2
                  CONTENTS, READONLY, LINK_ONCE_SAME_SIZE
  4 .pdr          00004900  00000000  00000000  0000022c  2**2
                  CONTENTS, READONLY
  5 .gnu.attributes 00000010  00000000  00000000  00004b2c  2**0
                  CONTENTS, READONLY
  6 .mdebug.abi32 00000000  800cf2fc  800cf2fc  00004b3c  2**0
                  CONTENTS, READONLY

Disassembly of section .text.unlikely:

8205f438 <.text.unlikely>:
8205f438:	620963fb 	0x620963fb
8205f43c:	d007d108 	0xd007d108
8205f440:	67266f02 	0x67266f02
8205f444:	18240604 	0x18240604
8205f448:	67042a4c 	0x67042a4c
8205f44c:	98a02a09 	lwr	zero,10761(a1)
8205f450:	18406791 	blez	v0,0x82079298
8205f454:	9604787b 	lhu	a0,30843(s0)
8205f458:	95049880 	lhu	a0,-26496(t0)
8205f45c:	d860e4ad 	0xd860e4ad
8205f460:	91089709 	lbu	t0,-26871(t0)
8205f464:	ef009007 	swc3	$0,-28665(t8)
8205f468:	65006305 	0x65006305
8205f46c:	620763fc 	0x620763fc
8205f470:	d005d106 	0xd005d106
8205f474:	9c606a00 	0x9c606a00
8205f478:	6019ebaa 	0x6019ebaa
8205f47c:	6726a340 	0x6726a340
8205f480:	67044e04 	0x67044e04
8205f484:	de5cf7ff 	0xde5cf7ff
8205f488:	18246aa0 	0x18246aa0
8205f48c:	ef4d2a4c 	swc3	$13,10828(k0)
8205f490:	22066762 	addi	a2,s0,26466
8205f494:	6a007216 	0x6a007216
8205f498:	24096798 	li	t1,26520
8205f49c:	10076743 	beq	zero,a3,0x820791ac
8205f4a0:	d9429840 	0xd9429840
8205f4a4:	99419860 	lwr	at,-26528(t2)
8205f4a8:	d840e349 	0xd840e349
8205f4ac:	97076a00 	lhu	a3,27136(t8)
8205f4b0:	90059106 	lbu	a1,-28410(zero)
8205f4b4:	6304ef00 	0x6304ef00
8205f4b8:	620963fb 	0x620963fb
8205f4bc:	d007d108 	0xd007d108
8205f4c0:	67266f01 	0x67266f01
8205f4c4:	18240604 	0x18240604
8205f4c8:	67042a4c 	0x67042a4c
8205f4cc:	92042a0d 	lbu	a0,10765(s0)
8205f4d0:	6a187201 	0x6a187201
8205f4d4:	98606109 	lwr	zero,24841(v1)
8205f4d8:	4b01a340 	c2	0x101a340
8205f4dc:	f7c0ea4b 	sdc1	$f0,-5557(s8)
8205f4e0:	d9403242 	0xd9403242
8205f4e4:	d8606a00 	0xd8606a00
8205f4e8:	91089709 	lbu	t0,-26871(t0)
8205f4ec:	ef009007 	swc3	$0,-28665(t8)
8205f4f0:	65006305 	0x65006305

Disassembly of section .bss:

800cf2fc <.bss>:
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

Disassembly of section .reginfo:

00000000 <.reginfo>:
   0:	a00100fc 	sb	at,252(zero)
	...

Disassembly of section .pdr:

00000000 <.pdr>:
       0:	8203bcc1 	lb	v1,-17215(s0)
       4:	80000000 	lb	zero,0(zero)
       8:	fffffffc 	0xfffffffc
	...
      14:	00000030 	0x30
      18:	0000001d 	0x1d
      1c:	0000001f 	0x1f
      20:	8205f39d 	lb	a1,-3171(s0)
      24:	80010000 	lb	at,0(zero)
      28:	fffffffc 	0xfffffffc
	...
      34:	00000038 	0x38
      38:	0000001d 	0x1d
      3c:	0000001f 	0x1f
      40:	8203bd59 	lb	v1,-17063(s0)
	...
      54:	00000008 	jr	zero
      58:	0000001d 	0x1d
      5c:	0000001f 	0x1f
      60:	8203bd6d 	lb	v1,-17043(s0)
	...
      74:	00000008 	jr	zero
      78:	0000001d 	0x1d
      7c:	0000001f 	0x1f
      80:	8203bd7d 	lb	v1,-17027(s0)
	...
      94:	00000008 	jr	zero
      98:	0000001d 	0x1d
      9c:	0000001f 	0x1f
      a0:	8203bdc1 	lb	v1,-16959(s0)
      a4:	80000000 	lb	zero,0(zero)
      a8:	fffffffc 	0xfffffffc
	...
      b4:	00000018 	mult	zero,zero
      b8:	0000001d 	0x1d
      bc:	0000001f 	0x1f
      c0:	8203bdfd 	lb	v1,-16899(s0)
	...
      d8:	0000001d 	0x1d
      dc:	0000001f 	0x1f
      e0:	8203be31 	lb	v1,-16847(s0)
      e4:	80030000 	lb	v1,0(zero)
      e8:	fffffffc 	0xfffffffc
	...
      f4:	00000030 	0x30
      f8:	0000001d 	0x1d
      fc:	0000001f 	0x1f
     100:	8203be7d 	lb	v1,-16771(s0)
     104:	80030000 	lb	v1,0(zero)
     108:	fffffffc 	0xfffffffc
	...
     114:	00000030 	0x30
     118:	0000001d 	0x1d
     11c:	0000001f 	0x1f
     120:	8203bec1 	lb	v1,-16703(s0)
     124:	80030000 	lb	v1,0(zero)
     128:	fffffffc 	0xfffffffc
	...
     134:	00000030 	0x30
     138:	0000001d 	0x1d
     13c:	0000001f 	0x1f
     140:	8203bfad 	lb	v1,-16467(s0)
     144:	80030000 	lb	v1,0(zero)
     148:	fffffffc 	0xfffffffc
	...
     154:	00000028 	0x28
     158:	0000001d 	0x1d
     15c:	0000001f 	0x1f
     160:	8203c075 	lb	v1,-16267(s0)
     164:	80030000 	lb	v1,0(zero)
     168:	fffffffc 	0xfffffffc
	...
     174:	00000028 	0x28
     178:	0000001d 	0x1d
     17c:	0000001f 	0x1f
     180:	8203c16d 	lb	v1,-16019(s0)
	...
     198:	0000001d 	0x1d
     19c:	0000001f 	0x1f
     1a0:	8203c1c5 	lb	v1,-15931(s0)
	...
     1b8:	0000001d 	0x1d
     1bc:	0000001f 	0x1f
     1c0:	8203c1e5 	lb	v1,-15899(s0)
	...
     1d8:	0000001d 	0x1d
     1dc:	0000001f 	0x1f
     1e0:	800c3bf9 	lb	t4,15353(zero)
     1e4:	80030000 	lb	v1,0(zero)
     1e8:	fffffffc 	0xfffffffc
	...
     1f4:	00000108 	0x108
     1f8:	0000001d 	0x1d
     1fc:	0000001f 	0x1f
     200:	800c3c79 	lb	t4,15481(zero)
     204:	80010000 	lb	at,0(zero)
     208:	fffffffc 	0xfffffffc
	...
     214:	00000018 	mult	zero,zero
     218:	0000001d 	0x1d
     21c:	0000001f 	0x1f
     220:	800c3c9d 	lb	t4,15517(zero)
     224:	80000000 	lb	zero,0(zero)
     228:	fffffffc 	0xfffffffc
	...
     234:	00000018 	mult	zero,zero
     238:	0000001d 	0x1d
     23c:	0000001f 	0x1f
     240:	800c3cad 	lb	t4,15533(zero)
     244:	80010000 	lb	at,0(zero)
     248:	fffffffc 	0xfffffffc
	...
     254:	00000020 	add	zero,zero,zero
     258:	0000001d 	0x1d
     25c:	0000001f 	0x1f
     260:	800c3cd1 	lb	t4,15569(zero)
     264:	80010000 	lb	at,0(zero)
     268:	fffffffc 	0xfffffffc
	...
     274:	00000018 	mult	zero,zero
     278:	0000001d 	0x1d
     27c:	0000001f 	0x1f
     280:	800c3ced 	lb	t4,15597(zero)
     284:	80010000 	lb	at,0(zero)
     288:	fffffffc 	0xfffffffc
	...
     294:	00000018 	mult	zero,zero
     298:	0000001d 	0x1d
     29c:	0000001f 	0x1f
     2a0:	800c3d11 	lb	t4,15633(zero)
     2a4:	80030000 	lb	v1,0(zero)
     2a8:	fffffffc 	0xfffffffc
	...
     2b4:	00000028 	0x28
     2b8:	0000001d 	0x1d
     2bc:	0000001f 	0x1f
     2c0:	800c3d51 	lb	t4,15697(zero)
     2c4:	80030000 	lb	v1,0(zero)
     2c8:	fffffffc 	0xfffffffc
	...
     2d4:	00000028 	0x28
     2d8:	0000001d 	0x1d
     2dc:	0000001f 	0x1f
     2e0:	800c3dc5 	lb	t4,15813(zero)
     2e4:	80030000 	lb	v1,0(zero)
     2e8:	fffffffc 	0xfffffffc
	...
     2f4:	00000138 	0x138
     2f8:	0000001d 	0x1d
     2fc:	0000001f 	0x1f
     300:	800c3ee1 	lb	t4,16097(zero)
     304:	80030000 	lb	v1,0(zero)
     308:	fffffffc 	0xfffffffc
	...
     314:	00000028 	0x28
     318:	0000001d 	0x1d
     31c:	0000001f 	0x1f
     320:	800c3f49 	lb	t4,16201(zero)
     324:	80030000 	lb	v1,0(zero)
     328:	fffffffc 	0xfffffffc
	...
     334:	00000030 	0x30
     338:	0000001d 	0x1d
     33c:	0000001f 	0x1f
     340:	800c40ad 	lb	t4,16557(zero)
     344:	80030000 	lb	v1,0(zero)
     348:	fffffffc 	0xfffffffc
	...
     354:	00000028 	0x28
     358:	0000001d 	0x1d
     35c:	0000001f 	0x1f
     360:	800c4119 	lb	t4,16665(zero)
     364:	80030000 	lb	v1,0(zero)
     368:	fffffffc 	0xfffffffc
	...
     374:	00000020 	add	zero,zero,zero
     378:	0000001d 	0x1d
     37c:	0000001f 	0x1f
     380:	800c41c9 	lb	t4,16841(zero)
     384:	80030000 	lb	v1,0(zero)
     388:	fffffffc 	0xfffffffc
	...
     394:	00000020 	add	zero,zero,zero
     398:	0000001d 	0x1d
     39c:	0000001f 	0x1f
     3a0:	800c41fd 	lb	t4,16893(zero)
     3a4:	80030000 	lb	v1,0(zero)
     3a8:	fffffffc 	0xfffffffc
	...
     3b4:	00000130 	0x130
     3b8:	0000001d 	0x1d
     3bc:	0000001f 	0x1f
     3c0:	800c4329 	lb	t4,17193(zero)
     3c4:	80030000 	lb	v1,0(zero)
     3c8:	fffffffc 	0xfffffffc
	...
     3d4:	00000020 	add	zero,zero,zero
     3d8:	0000001d 	0x1d
     3dc:	0000001f 	0x1f
     3e0:	800c4369 	lb	t4,17257(zero)
     3e4:	80030000 	lb	v1,0(zero)
     3e8:	fffffffc 	0xfffffffc
	...
     3f4:	00000038 	0x38
     3f8:	0000001d 	0x1d
     3fc:	0000001f 	0x1f
     400:	800c447d 	lb	t4,17533(zero)
     404:	80030000 	lb	v1,0(zero)
     408:	fffffffc 	0xfffffffc
	...
     414:	00000050 	0x50
     418:	0000001d 	0x1d
     41c:	0000001f 	0x1f
     420:	800c4645 	lb	t4,17989(zero)
     424:	80030000 	lb	v1,0(zero)
     428:	fffffffc 	0xfffffffc
	...
     434:	00000030 	0x30
     438:	0000001d 	0x1d
     43c:	0000001f 	0x1f
     440:	800c4829 	lb	t4,18473(zero)
     444:	80000000 	lb	zero,0(zero)
     448:	fffffffc 	0xfffffffc
	...
     454:	00000018 	mult	zero,zero
     458:	0000001d 	0x1d
     45c:	0000001f 	0x1f
     460:	800c483d 	lb	t4,18493(zero)
     464:	80000000 	lb	zero,0(zero)
     468:	fffffffc 	0xfffffffc
	...
     474:	00000018 	mult	zero,zero
     478:	0000001d 	0x1d
     47c:	0000001f 	0x1f
     480:	800c4851 	lb	t4,18513(zero)
     484:	80030000 	lb	v1,0(zero)
     488:	fffffffc 	0xfffffffc
	...
     494:	00000020 	add	zero,zero,zero
     498:	0000001d 	0x1d
     49c:	0000001f 	0x1f
     4a0:	800c48c5 	lb	t4,18629(zero)
     4a4:	80030000 	lb	v1,0(zero)
     4a8:	fffffffc 	0xfffffffc
	...
     4b4:	00000050 	0x50
     4b8:	0000001d 	0x1d
     4bc:	0000001f 	0x1f
     4c0:	800c4e09 	lb	t4,19977(zero)
     4c4:	80030000 	lb	v1,0(zero)
     4c8:	fffffffc 	0xfffffffc
	...
     4d4:	00000140 	sll	zero,zero,0x5
     4d8:	0000001d 	0x1d
     4dc:	0000001f 	0x1f
     4e0:	800c57d1 	lb	t4,22481(zero)
	...
     4f8:	0000001d 	0x1d
     4fc:	0000001f 	0x1f
     500:	800c57d5 	lb	t4,22485(zero)
     504:	80030000 	lb	v1,0(zero)
     508:	fffffffc 	0xfffffffc
	...
     514:	00000140 	sll	zero,zero,0x5
     518:	0000001d 	0x1d
     51c:	0000001f 	0x1f
     520:	800c5ab9 	lb	t4,23225(zero)
     524:	80030000 	lb	v1,0(zero)
     528:	fffffffc 	0xfffffffc
	...
     534:	00000028 	0x28
     538:	0000001d 	0x1d
     53c:	0000001f 	0x1f
     540:	800c5af5 	lb	t4,23285(zero)
     544:	80030000 	lb	v1,0(zero)
     548:	fffffffc 	0xfffffffc
	...
     554:	00000078 	0x78
     558:	0000001d 	0x1d
     55c:	0000001f 	0x1f
     560:	8203c239 	lb	v1,-15815(s0)
     564:	80030000 	lb	v1,0(zero)
     568:	fffffffc 	0xfffffffc
	...
     574:	00000020 	add	zero,zero,zero
     578:	0000001d 	0x1d
     57c:	0000001f 	0x1f
     580:	8203c2a9 	lb	v1,-15703(s0)
	...
     598:	0000001d 	0x1d
     59c:	0000001f 	0x1f
     5a0:	8203c2fd 	lb	v1,-15619(s0)
     5a4:	80000000 	lb	zero,0(zero)
     5a8:	fffffffc 	0xfffffffc
	...
     5b4:	00000018 	mult	zero,zero
     5b8:	0000001d 	0x1d
     5bc:	0000001f 	0x1f
     5c0:	8203c349 	lb	v1,-15543(s0)
     5c4:	80000000 	lb	zero,0(zero)
     5c8:	fffffffc 	0xfffffffc
	...
     5d4:	00000018 	mult	zero,zero
     5d8:	0000001d 	0x1d
     5dc:	0000001f 	0x1f
     5e0:	8203c391 	lb	v1,-15471(s0)
     5e4:	80000000 	lb	zero,0(zero)
     5e8:	fffffffc 	0xfffffffc
	...
     5f4:	00000018 	mult	zero,zero
     5f8:	0000001d 	0x1d
     5fc:	0000001f 	0x1f
     600:	8203c3d9 	lb	v1,-15399(s0)
     604:	80000000 	lb	zero,0(zero)
     608:	fffffffc 	0xfffffffc
	...
     614:	00000018 	mult	zero,zero
     618:	0000001d 	0x1d
     61c:	0000001f 	0x1f
     620:	8203c40d 	lb	v1,-15347(s0)
     624:	80010000 	lb	at,0(zero)
     628:	fffffffc 	0xfffffffc
	...
     634:	00000018 	mult	zero,zero
     638:	0000001d 	0x1d
     63c:	0000001f 	0x1f
     640:	8203c481 	lb	v1,-15231(s0)
     644:	80030000 	lb	v1,0(zero)
     648:	fffffffc 	0xfffffffc
	...
     654:	00000028 	0x28
     658:	0000001d 	0x1d
     65c:	0000001f 	0x1f
     660:	8203c4ed 	lb	v1,-15123(s0)
     664:	80030000 	lb	v1,0(zero)
     668:	fffffffc 	0xfffffffc
	...
     674:	00000028 	0x28
     678:	0000001d 	0x1d
     67c:	0000001f 	0x1f
     680:	8203c559 	lb	v1,-15015(s0)
     684:	80030000 	lb	v1,0(zero)
     688:	fffffffc 	0xfffffffc
	...
     694:	00000028 	0x28
     698:	0000001d 	0x1d
     69c:	0000001f 	0x1f
     6a0:	8203c5c5 	lb	v1,-14907(s0)
     6a4:	80030000 	lb	v1,0(zero)
     6a8:	fffffffc 	0xfffffffc
	...
     6b4:	00000028 	0x28
     6b8:	0000001d 	0x1d
     6bc:	0000001f 	0x1f
     6c0:	8203c631 	lb	v1,-14799(s0)
     6c4:	80030000 	lb	v1,0(zero)
     6c8:	fffffffc 	0xfffffffc
	...
     6d4:	00000028 	0x28
     6d8:	0000001d 	0x1d
     6dc:	0000001f 	0x1f
     6e0:	8203c699 	lb	v1,-14695(s0)
     6e4:	80030000 	lb	v1,0(zero)
     6e8:	fffffffc 	0xfffffffc
	...
     6f4:	00000028 	0x28
     6f8:	0000001d 	0x1d
     6fc:	0000001f 	0x1f
     700:	8203c701 	lb	v1,-14591(s0)
     704:	80030000 	lb	v1,0(zero)
     708:	fffffffc 	0xfffffffc
	...
     714:	00000028 	0x28
     718:	0000001d 	0x1d
     71c:	0000001f 	0x1f
     720:	8203c769 	lb	v1,-14487(s0)
     724:	80030000 	lb	v1,0(zero)
     728:	fffffffc 	0xfffffffc
	...
     734:	00000028 	0x28
     738:	0000001d 	0x1d
     73c:	0000001f 	0x1f
     740:	8203c945 	lb	v1,-14011(s0)
     744:	80010000 	lb	at,0(zero)
     748:	fffffffc 	0xfffffffc
	...
     754:	00000018 	mult	zero,zero
     758:	0000001d 	0x1d
     75c:	0000001f 	0x1f
     760:	8203cab9 	lb	v1,-13639(s0)
     764:	80030000 	lb	v1,0(zero)
     768:	fffffffc 	0xfffffffc
	...
     774:	00000020 	add	zero,zero,zero
     778:	0000001d 	0x1d
     77c:	0000001f 	0x1f
     780:	8203cb51 	lb	v1,-13487(s0)
     784:	80030000 	lb	v1,0(zero)
     788:	fffffffc 	0xfffffffc
	...
     794:	00000020 	add	zero,zero,zero
     798:	0000001d 	0x1d
     79c:	0000001f 	0x1f
     7a0:	8203cb99 	lb	v1,-13415(s0)
     7a4:	80030000 	lb	v1,0(zero)
     7a8:	fffffffc 	0xfffffffc
	...
     7b4:	00000028 	0x28
     7b8:	0000001d 	0x1d
     7bc:	0000001f 	0x1f
     7c0:	8203cfbd 	lb	v1,-12355(s0)
     7c4:	80030000 	lb	v1,0(zero)
     7c8:	fffffffc 	0xfffffffc
	...
     7d4:	00000020 	add	zero,zero,zero
     7d8:	0000001d 	0x1d
     7dc:	0000001f 	0x1f
     7e0:	8203d059 	lb	v1,-12199(s0)
     7e4:	80000000 	lb	zero,0(zero)
     7e8:	fffffffc 	0xfffffffc
	...
     7f4:	00000018 	mult	zero,zero
     7f8:	0000001d 	0x1d
     7fc:	0000001f 	0x1f
     800:	8203d08d 	lb	v1,-12147(s0)
	...
     818:	0000001d 	0x1d
     81c:	0000001f 	0x1f
     820:	8203d09d 	lb	v1,-12131(s0)
     824:	80030000 	lb	v1,0(zero)
     828:	fffffffc 	0xfffffffc
	...
     834:	00000028 	0x28
     838:	0000001d 	0x1d
     83c:	0000001f 	0x1f
     840:	8203d1c5 	lb	v1,-11835(s0)
     844:	80030000 	lb	v1,0(zero)
     848:	fffffffc 	0xfffffffc
	...
     854:	00000028 	0x28
     858:	0000001d 	0x1d
     85c:	0000001f 	0x1f
     860:	8203d2c5 	lb	v1,-11579(s0)
     864:	80030000 	lb	v1,0(zero)
     868:	fffffffc 	0xfffffffc
	...
     874:	00000020 	add	zero,zero,zero
     878:	0000001d 	0x1d
     87c:	0000001f 	0x1f
     880:	8203d361 	lb	v1,-11423(s0)
     884:	80010000 	lb	at,0(zero)
     888:	fffffffc 	0xfffffffc
	...
     894:	00000018 	mult	zero,zero
     898:	0000001d 	0x1d
     89c:	0000001f 	0x1f
     8a0:	8203d3b9 	lb	v1,-11335(s0)
     8a4:	80030000 	lb	v1,0(zero)
     8a8:	fffffffc 	0xfffffffc
	...
     8b4:	00000028 	0x28
     8b8:	0000001d 	0x1d
     8bc:	0000001f 	0x1f
     8c0:	8203d401 	lb	v1,-11263(s0)
     8c4:	80010000 	lb	at,0(zero)
     8c8:	fffffffc 	0xfffffffc
	...
     8d4:	00000018 	mult	zero,zero
     8d8:	0000001d 	0x1d
     8dc:	0000001f 	0x1f
     8e0:	8203d459 	lb	v1,-11175(s0)
     8e4:	80030000 	lb	v1,0(zero)
     8e8:	fffffffc 	0xfffffffc
	...
     8f4:	00000028 	0x28
     8f8:	0000001d 	0x1d
     8fc:	0000001f 	0x1f
     900:	8203d4a1 	lb	v1,-11103(s0)
     904:	80030000 	lb	v1,0(zero)
     908:	fffffffc 	0xfffffffc
	...
     914:	00000068 	0x68
     918:	0000001d 	0x1d
     91c:	0000001f 	0x1f
     920:	8203d5a5 	lb	v1,-10843(s0)
	...
     938:	0000001d 	0x1d
     93c:	0000001f 	0x1f
     940:	8203d5bd 	lb	v1,-10819(s0)
	...
     958:	0000001d 	0x1d
     95c:	0000001f 	0x1f
     960:	8203d629 	lb	v1,-10711(s0)
     964:	80030000 	lb	v1,0(zero)
     968:	fffffffc 	0xfffffffc
	...
     974:	00000028 	0x28
     978:	0000001d 	0x1d
     97c:	0000001f 	0x1f
     980:	8203d6a9 	lb	v1,-10583(s0)
     984:	80030000 	lb	v1,0(zero)
     988:	fffffffc 	0xfffffffc
	...
     994:	00000038 	0x38
     998:	0000001d 	0x1d
     99c:	0000001f 	0x1f
     9a0:	8203d735 	lb	v1,-10443(s0)
     9a4:	80030000 	lb	v1,0(zero)
     9a8:	fffffffc 	0xfffffffc
	...
     9b4:	00000020 	add	zero,zero,zero
     9b8:	0000001d 	0x1d
     9bc:	0000001f 	0x1f
     9c0:	8203d7ad 	lb	v1,-10323(s0)
     9c4:	80010000 	lb	at,0(zero)
     9c8:	fffffffc 	0xfffffffc
	...
     9d4:	00000018 	mult	zero,zero
     9d8:	0000001d 	0x1d
     9dc:	0000001f 	0x1f
     9e0:	8203d7c9 	lb	v1,-10295(s0)
     9e4:	80010000 	lb	at,0(zero)
     9e8:	fffffffc 	0xfffffffc
	...
     9f4:	00000018 	mult	zero,zero
     9f8:	0000001d 	0x1d
     9fc:	0000001f 	0x1f
     a00:	8203d811 	lb	v1,-10223(s0)
     a04:	80010000 	lb	at,0(zero)
     a08:	fffffffc 	0xfffffffc
	...
     a14:	00000018 	mult	zero,zero
     a18:	0000001d 	0x1d
     a1c:	0000001f 	0x1f
     a20:	8203d859 	lb	v1,-10151(s0)
     a24:	80010000 	lb	at,0(zero)
     a28:	fffffffc 	0xfffffffc
	...
     a34:	00000018 	mult	zero,zero
     a38:	0000001d 	0x1d
     a3c:	0000001f 	0x1f
     a40:	8203d8a1 	lb	v1,-10079(s0)
     a44:	80010000 	lb	at,0(zero)
     a48:	fffffffc 	0xfffffffc
	...
     a54:	00000018 	mult	zero,zero
     a58:	0000001d 	0x1d
     a5c:	0000001f 	0x1f
     a60:	8203d8e9 	lb	v1,-10007(s0)
     a64:	80010000 	lb	at,0(zero)
     a68:	fffffffc 	0xfffffffc
	...
     a74:	00000018 	mult	zero,zero
     a78:	0000001d 	0x1d
     a7c:	0000001f 	0x1f
     a80:	8203d931 	lb	v1,-9935(s0)
     a84:	80010000 	lb	at,0(zero)
     a88:	fffffffc 	0xfffffffc
	...
     a94:	00000018 	mult	zero,zero
     a98:	0000001d 	0x1d
     a9c:	0000001f 	0x1f
     aa0:	8203d979 	lb	v1,-9863(s0)
     aa4:	80010000 	lb	at,0(zero)
     aa8:	fffffffc 	0xfffffffc
	...
     ab4:	00000018 	mult	zero,zero
     ab8:	0000001d 	0x1d
     abc:	0000001f 	0x1f
     ac0:	8203d9c1 	lb	v1,-9791(s0)
     ac4:	80030000 	lb	v1,0(zero)
     ac8:	fffffffc 	0xfffffffc
	...
     ad4:	00000028 	0x28
     ad8:	0000001d 	0x1d
     adc:	0000001f 	0x1f
     ae0:	8203db9d 	lb	v1,-9315(s0)
     ae4:	80010000 	lb	at,0(zero)
     ae8:	fffffffc 	0xfffffffc
	...
     af4:	00000018 	mult	zero,zero
     af8:	0000001d 	0x1d
     afc:	0000001f 	0x1f
     b00:	8203dbcd 	lb	v1,-9267(s0)
     b04:	80010000 	lb	at,0(zero)
     b08:	fffffffc 	0xfffffffc
	...
     b14:	00000018 	mult	zero,zero
     b18:	0000001d 	0x1d
     b1c:	0000001f 	0x1f
     b20:	8203dc15 	lb	v1,-9195(s0)
     b24:	80010000 	lb	at,0(zero)
     b28:	fffffffc 	0xfffffffc
	...
     b34:	00000018 	mult	zero,zero
     b38:	0000001d 	0x1d
     b3c:	0000001f 	0x1f
     b40:	8203dc5d 	lb	v1,-9123(s0)
     b44:	80030000 	lb	v1,0(zero)
     b48:	fffffffc 	0xfffffffc
	...
     b54:	00000028 	0x28
     b58:	0000001d 	0x1d
     b5c:	0000001f 	0x1f
     b60:	8203dc99 	lb	v1,-9063(s0)
     b64:	80030000 	lb	v1,0(zero)
     b68:	fffffffc 	0xfffffffc
	...
     b74:	00000020 	add	zero,zero,zero
     b78:	0000001d 	0x1d
     b7c:	0000001f 	0x1f
     b80:	8203dcd5 	lb	v1,-9003(s0)
     b84:	80030000 	lb	v1,0(zero)
     b88:	fffffffc 	0xfffffffc
	...
     b94:	00000020 	add	zero,zero,zero
     b98:	0000001d 	0x1d
     b9c:	0000001f 	0x1f
     ba0:	8203dd69 	lb	v1,-8855(s0)
     ba4:	80030000 	lb	v1,0(zero)
     ba8:	fffffffc 	0xfffffffc
	...
     bb4:	00000028 	0x28
     bb8:	0000001d 	0x1d
     bbc:	0000001f 	0x1f
     bc0:	8203de5d 	lb	v1,-8611(s0)
     bc4:	80030000 	lb	v1,0(zero)
     bc8:	fffffffc 	0xfffffffc
	...
     bd4:	00000030 	0x30
     bd8:	0000001d 	0x1d
     bdc:	0000001f 	0x1f
     be0:	8203ded1 	lb	v1,-8495(s0)
     be4:	80030000 	lb	v1,0(zero)
     be8:	fffffffc 	0xfffffffc
	...
     bf4:	00000030 	0x30
     bf8:	0000001d 	0x1d
     bfc:	0000001f 	0x1f
     c00:	8203e011 	lb	v1,-8175(s0)
     c04:	80030000 	lb	v1,0(zero)
     c08:	fffffffc 	0xfffffffc
	...
     c14:	00000028 	0x28
     c18:	0000001d 	0x1d
     c1c:	0000001f 	0x1f
     c20:	8203e089 	lb	v1,-8055(s0)
     c24:	80030000 	lb	v1,0(zero)
     c28:	fffffffc 	0xfffffffc
	...
     c34:	00000028 	0x28
     c38:	0000001d 	0x1d
     c3c:	0000001f 	0x1f
     c40:	8203e0cd 	lb	v1,-7987(s0)
     c44:	80030000 	lb	v1,0(zero)
     c48:	fffffffc 	0xfffffffc
	...
     c54:	00000048 	0x48
     c58:	0000001d 	0x1d
     c5c:	0000001f 	0x1f
     c60:	8203e219 	lb	v1,-7655(s0)
     c64:	80030000 	lb	v1,0(zero)
     c68:	fffffffc 	0xfffffffc
	...
     c74:	00000028 	0x28
     c78:	0000001d 	0x1d
     c7c:	0000001f 	0x1f
     c80:	8203e279 	lb	v1,-7559(s0)
     c84:	80030000 	lb	v1,0(zero)
     c88:	fffffffc 	0xfffffffc
	...
     c94:	00000040 	ssnop
     c98:	0000001d 	0x1d
     c9c:	0000001f 	0x1f
     ca0:	8203e429 	lb	v1,-7127(s0)
     ca4:	80030000 	lb	v1,0(zero)
     ca8:	fffffffc 	0xfffffffc
	...
     cb4:	00000040 	ssnop
     cb8:	0000001d 	0x1d
     cbc:	0000001f 	0x1f
     cc0:	8203e605 	lb	v1,-6651(s0)
     cc4:	80030000 	lb	v1,0(zero)
     cc8:	fffffffc 	0xfffffffc
	...
     cd4:	00000048 	0x48
     cd8:	0000001d 	0x1d
     cdc:	0000001f 	0x1f
     ce0:	8203e82d 	lb	v1,-6099(s0)
     ce4:	80030000 	lb	v1,0(zero)
     ce8:	fffffffc 	0xfffffffc
	...
     cf4:	00000070 	0x70
     cf8:	0000001d 	0x1d
     cfc:	0000001f 	0x1f
     d00:	8203eb8d 	lb	v1,-5235(s0)
     d04:	80030000 	lb	v1,0(zero)
     d08:	fffffffc 	0xfffffffc
	...
     d14:	00000090 	0x90
     d18:	0000001d 	0x1d
     d1c:	0000001f 	0x1f
     d20:	8203f2bd 	lb	v1,-3395(s0)
     d24:	80010000 	lb	at,0(zero)
     d28:	fffffffc 	0xfffffffc
	...
     d34:	00000018 	mult	zero,zero
     d38:	0000001d 	0x1d
     d3c:	0000001f 	0x1f
     d40:	8203f305 	lb	v1,-3323(s0)
     d44:	80010000 	lb	at,0(zero)
     d48:	fffffffc 	0xfffffffc
	...
     d54:	00000018 	mult	zero,zero
     d58:	0000001d 	0x1d
     d5c:	0000001f 	0x1f
     d60:	8203f349 	lb	v1,-3255(s0)
     d64:	80030000 	lb	v1,0(zero)
     d68:	fffffffc 	0xfffffffc
	...
     d74:	00000048 	0x48
     d78:	0000001d 	0x1d
     d7c:	0000001f 	0x1f
     d80:	8203f3c5 	lb	v1,-3131(s0)
     d84:	80030000 	lb	v1,0(zero)
     d88:	fffffffc 	0xfffffffc
	...
     d94:	00000048 	0x48
     d98:	0000001d 	0x1d
     d9c:	0000001f 	0x1f
     da0:	8203fa59 	lb	v1,-1447(s0)
     da4:	80010000 	lb	at,0(zero)
     da8:	fffffffc 	0xfffffffc
	...
     db4:	00000018 	mult	zero,zero
     db8:	0000001d 	0x1d
     dbc:	0000001f 	0x1f
     dc0:	8203faad 	lb	v1,-1363(s0)
     dc4:	80010000 	lb	at,0(zero)
     dc8:	fffffffc 	0xfffffffc
	...
     dd4:	00000018 	mult	zero,zero
     dd8:	0000001d 	0x1d
     ddc:	0000001f 	0x1f
     de0:	8203faf5 	lb	v1,-1291(s0)
     de4:	80030000 	lb	v1,0(zero)
     de8:	fffffffc 	0xfffffffc
	...
     df4:	00000050 	0x50
     df8:	0000001d 	0x1d
     dfc:	0000001f 	0x1f
     e00:	8203fcf5 	lb	v1,-779(s0)
     e04:	80030000 	lb	v1,0(zero)
     e08:	fffffffc 	0xfffffffc
	...
     e14:	00000048 	0x48
     e18:	0000001d 	0x1d
     e1c:	0000001f 	0x1f
     e20:	8203fe81 	lb	v1,-383(s0)
     e24:	80030000 	lb	v1,0(zero)
     e28:	fffffffc 	0xfffffffc
	...
     e34:	00000050 	0x50
     e38:	0000001d 	0x1d
     e3c:	0000001f 	0x1f
     e40:	820400a1 	lb	a0,161(s0)
     e44:	80030000 	lb	v1,0(zero)
     e48:	fffffffc 	0xfffffffc
	...
     e54:	00000058 	0x58
     e58:	0000001d 	0x1d
     e5c:	0000001f 	0x1f
     e60:	820403b9 	lb	a0,953(s0)
     e64:	80010000 	lb	at,0(zero)
     e68:	fffffffc 	0xfffffffc
	...
     e74:	00000018 	mult	zero,zero
     e78:	0000001d 	0x1d
     e7c:	0000001f 	0x1f
     e80:	820403fd 	lb	a0,1021(s0)
	...
     e98:	0000001d 	0x1d
     e9c:	0000001f 	0x1f
     ea0:	8204043d 	lb	a0,1085(s0)
     ea4:	80030000 	lb	v1,0(zero)
     ea8:	fffffffc 	0xfffffffc
	...
     eb4:	00000040 	ssnop
     eb8:	0000001d 	0x1d
     ebc:	0000001f 	0x1f
     ec0:	820405c1 	lb	a0,1473(s0)
     ec4:	80030000 	lb	v1,0(zero)
     ec8:	fffffffc 	0xfffffffc
	...
     ed4:	00000050 	0x50
     ed8:	0000001d 	0x1d
     edc:	0000001f 	0x1f
     ee0:	8204095d 	lb	a0,2397(s0)
     ee4:	80030000 	lb	v1,0(zero)
     ee8:	fffffffc 	0xfffffffc
	...
     ef4:	00000030 	0x30
     ef8:	0000001d 	0x1d
     efc:	0000001f 	0x1f
     f00:	82040a39 	lb	a0,2617(s0)
     f04:	80030000 	lb	v1,0(zero)
     f08:	fffffffc 	0xfffffffc
	...
     f14:	00000038 	0x38
     f18:	0000001d 	0x1d
     f1c:	0000001f 	0x1f
     f20:	82040c05 	lb	a0,3077(s0)
     f24:	80030000 	lb	v1,0(zero)
     f28:	fffffffc 	0xfffffffc
	...
     f34:	00000078 	0x78
     f38:	0000001d 	0x1d
     f3c:	0000001f 	0x1f
     f40:	82040e31 	lb	a0,3633(s0)
     f44:	80030000 	lb	v1,0(zero)
     f48:	fffffffc 	0xfffffffc
	...
     f54:	00000028 	0x28
     f58:	0000001d 	0x1d
     f5c:	0000001f 	0x1f
     f60:	82040ef5 	lb	a0,3829(s0)
     f64:	80030000 	lb	v1,0(zero)
     f68:	fffffffc 	0xfffffffc
	...
     f74:	00000030 	0x30
     f78:	0000001d 	0x1d
     f7c:	0000001f 	0x1f
     f80:	82041095 	lb	a0,4245(s0)
     f84:	80030000 	lb	v1,0(zero)
     f88:	fffffffc 	0xfffffffc
	...
     f94:	00000030 	0x30
     f98:	0000001d 	0x1d
     f9c:	0000001f 	0x1f
     fa0:	820412d9 	lb	a0,4825(s0)
     fa4:	80030000 	lb	v1,0(zero)
     fa8:	fffffffc 	0xfffffffc
	...
     fb4:	00000030 	0x30
     fb8:	0000001d 	0x1d
     fbc:	0000001f 	0x1f
     fc0:	82041421 	lb	a0,5153(s0)
     fc4:	80030000 	lb	v1,0(zero)
     fc8:	fffffffc 	0xfffffffc
	...
     fd4:	00000028 	0x28
     fd8:	0000001d 	0x1d
     fdc:	0000001f 	0x1f
     fe0:	8204151d 	lb	a0,5405(s0)
     fe4:	80030000 	lb	v1,0(zero)
     fe8:	fffffffc 	0xfffffffc
	...
     ff4:	00000028 	0x28
     ff8:	0000001d 	0x1d
     ffc:	0000001f 	0x1f
    1000:	82041619 	lb	a0,5657(s0)
    1004:	80030000 	lb	v1,0(zero)
    1008:	fffffffc 	0xfffffffc
	...
    1014:	00000028 	0x28
    1018:	0000001d 	0x1d
    101c:	0000001f 	0x1f
    1020:	82041679 	lb	a0,5753(s0)
    1024:	80030000 	lb	v1,0(zero)
    1028:	fffffffc 	0xfffffffc
	...
    1034:	00000038 	0x38
    1038:	0000001d 	0x1d
    103c:	0000001f 	0x1f
    1040:	820417c1 	lb	a0,6081(s0)
    1044:	80030000 	lb	v1,0(zero)
    1048:	fffffffc 	0xfffffffc
	...
    1054:	00000030 	0x30
    1058:	0000001d 	0x1d
    105c:	0000001f 	0x1f
    1060:	820418f9 	lb	a0,6393(s0)
    1064:	80030000 	lb	v1,0(zero)
    1068:	fffffffc 	0xfffffffc
	...
    1074:	00000030 	0x30
    1078:	0000001d 	0x1d
    107c:	0000001f 	0x1f
    1080:	82041989 	lb	a0,6537(s0)
	...
    1098:	0000001d 	0x1d
    109c:	0000001f 	0x1f
    10a0:	820419bd 	lb	a0,6589(s0)
	...
    10b8:	0000001d 	0x1d
    10bc:	0000001f 	0x1f
    10c0:	82041a21 	lb	a0,6689(s0)
    10c4:	80030000 	lb	v1,0(zero)
    10c8:	fffffffc 	0xfffffffc
	...
    10d4:	00000028 	0x28
    10d8:	0000001d 	0x1d
    10dc:	0000001f 	0x1f
    10e0:	82041a79 	lb	a0,6777(s0)
    10e4:	80030000 	lb	v1,0(zero)
    10e8:	fffffffc 	0xfffffffc
	...
    10f4:	00000048 	0x48
    10f8:	0000001d 	0x1d
    10fc:	0000001f 	0x1f
    1100:	82041c51 	lb	a0,7249(s0)
    1104:	80030000 	lb	v1,0(zero)
    1108:	fffffffc 	0xfffffffc
	...
    1114:	00000078 	0x78
    1118:	0000001d 	0x1d
    111c:	0000001f 	0x1f
    1120:	82042b51 	lb	a0,11089(s0)
    1124:	80010000 	lb	at,0(zero)
    1128:	fffffffc 	0xfffffffc
	...
    1134:	00000018 	mult	zero,zero
    1138:	0000001d 	0x1d
    113c:	0000001f 	0x1f
    1140:	82042ba5 	lb	a0,11173(s0)
    1144:	80010000 	lb	at,0(zero)
    1148:	fffffffc 	0xfffffffc
	...
    1154:	00000018 	mult	zero,zero
    1158:	0000001d 	0x1d
    115c:	0000001f 	0x1f
    1160:	82042bf9 	lb	a0,11257(s0)
    1164:	80030000 	lb	v1,0(zero)
    1168:	fffffffc 	0xfffffffc
	...
    1174:	00000030 	0x30
    1178:	0000001d 	0x1d
    117c:	0000001f 	0x1f
    1180:	82042ee9 	lb	a0,12009(s0)
    1184:	80010000 	lb	at,0(zero)
    1188:	fffffffc 	0xfffffffc
	...
    1194:	00000018 	mult	zero,zero
    1198:	0000001d 	0x1d
    119c:	0000001f 	0x1f
    11a0:	82042f39 	lb	a0,12089(s0)
    11a4:	80030000 	lb	v1,0(zero)
    11a8:	fffffffc 	0xfffffffc
	...
    11b4:	00000058 	0x58
    11b8:	0000001d 	0x1d
    11bc:	0000001f 	0x1f
    11c0:	8204334d 	lb	a0,13133(s0)
    11c4:	80030000 	lb	v1,0(zero)
    11c8:	fffffffc 	0xfffffffc
	...
    11d4:	00000060 	0x60
    11d8:	0000001d 	0x1d
    11dc:	0000001f 	0x1f
    11e0:	82043b8d 	lb	a0,15245(s0)
    11e4:	80030000 	lb	v1,0(zero)
    11e8:	fffffffc 	0xfffffffc
	...
    11f4:	00000028 	0x28
    11f8:	0000001d 	0x1d
    11fc:	0000001f 	0x1f
    1200:	82043c8d 	lb	a0,15501(s0)
    1204:	80030000 	lb	v1,0(zero)
    1208:	fffffffc 	0xfffffffc
	...
    1214:	00000028 	0x28
    1218:	0000001d 	0x1d
    121c:	0000001f 	0x1f
    1220:	82043da1 	lb	a0,15777(s0)
    1224:	80030000 	lb	v1,0(zero)
    1228:	fffffffc 	0xfffffffc
	...
    1234:	00000040 	ssnop
    1238:	0000001d 	0x1d
    123c:	0000001f 	0x1f
    1240:	82044021 	lb	a0,16417(s0)
    1244:	80030000 	lb	v1,0(zero)
    1248:	fffffffc 	0xfffffffc
	...
    1254:	00000028 	0x28
    1258:	0000001d 	0x1d
    125c:	0000001f 	0x1f
    1260:	82044149 	lb	a0,16713(s0)
    1264:	80030000 	lb	v1,0(zero)
    1268:	fffffffc 	0xfffffffc
	...
    1274:	00000058 	0x58
    1278:	0000001d 	0x1d
    127c:	0000001f 	0x1f
    1280:	82044475 	lb	a0,17525(s0)
    1284:	80010000 	lb	at,0(zero)
    1288:	fffffffc 	0xfffffffc
	...
    1294:	00000018 	mult	zero,zero
    1298:	0000001d 	0x1d
    129c:	0000001f 	0x1f
    12a0:	82044559 	lb	a0,17753(s0)
    12a4:	80030000 	lb	v1,0(zero)
    12a8:	fffffffc 	0xfffffffc
	...
    12b4:	00000060 	0x60
    12b8:	0000001d 	0x1d
    12bc:	0000001f 	0x1f
    12c0:	82044799 	lb	a0,18329(s0)
    12c4:	80030000 	lb	v1,0(zero)
    12c8:	fffffffc 	0xfffffffc
	...
    12d4:	000000b0 	0xb0
    12d8:	0000001d 	0x1d
    12dc:	0000001f 	0x1f
    12e0:	82044911 	lb	a0,18705(s0)
    12e4:	80030000 	lb	v1,0(zero)
    12e8:	fffffffc 	0xfffffffc
	...
    12f4:	00000030 	0x30
    12f8:	0000001d 	0x1d
    12fc:	0000001f 	0x1f
    1300:	8204498d 	lb	a0,18829(s0)
    1304:	80030000 	lb	v1,0(zero)
    1308:	fffffffc 	0xfffffffc
	...
    1314:	00000058 	0x58
    1318:	0000001d 	0x1d
    131c:	0000001f 	0x1f
    1320:	82044dad 	lb	a0,19885(s0)
    1324:	80030000 	lb	v1,0(zero)
    1328:	fffffffc 	0xfffffffc
	...
    1334:	00000028 	0x28
    1338:	0000001d 	0x1d
    133c:	0000001f 	0x1f
    1340:	82044ec9 	lb	a0,20169(s0)
	...
    1358:	0000001d 	0x1d
    135c:	0000001f 	0x1f
    1360:	82044ecd 	lb	a0,20173(s0)
    1364:	80030000 	lb	v1,0(zero)
    1368:	fffffffc 	0xfffffffc
	...
    1374:	00000028 	0x28
    1378:	0000001d 	0x1d
    137c:	0000001f 	0x1f
    1380:	82044f45 	lb	a0,20293(s0)
    1384:	80030000 	lb	v1,0(zero)
    1388:	fffffffc 	0xfffffffc
	...
    1394:	00000038 	0x38
    1398:	0000001d 	0x1d
    139c:	0000001f 	0x1f
    13a0:	8204506d 	lb	a0,20589(s0)
    13a4:	80030000 	lb	v1,0(zero)
    13a8:	fffffffc 	0xfffffffc
	...
    13b4:	00000028 	0x28
    13b8:	0000001d 	0x1d
    13bc:	0000001f 	0x1f
    13c0:	82045125 	lb	a0,20773(s0)
    13c4:	80030000 	lb	v1,0(zero)
    13c8:	fffffffc 	0xfffffffc
	...
    13d4:	00000040 	ssnop
    13d8:	0000001d 	0x1d
    13dc:	0000001f 	0x1f
    13e0:	820452d9 	lb	a0,21209(s0)
    13e4:	80030000 	lb	v1,0(zero)
    13e8:	fffffffc 	0xfffffffc
	...
    13f4:	00000030 	0x30
    13f8:	0000001d 	0x1d
    13fc:	0000001f 	0x1f
    1400:	82045451 	lb	a0,21585(s0)
    1404:	80030000 	lb	v1,0(zero)
    1408:	fffffffc 	0xfffffffc
	...
    1414:	00000038 	0x38
    1418:	0000001d 	0x1d
    141c:	0000001f 	0x1f
    1420:	820455ed 	lb	a0,21997(s0)
    1424:	80030000 	lb	v1,0(zero)
    1428:	fffffffc 	0xfffffffc
	...
    1434:	00000038 	0x38
    1438:	0000001d 	0x1d
    143c:	0000001f 	0x1f
    1440:	820457cd 	lb	a0,22477(s0)
    1444:	80030000 	lb	v1,0(zero)
    1448:	fffffffc 	0xfffffffc
	...
    1454:	00000028 	0x28
    1458:	0000001d 	0x1d
    145c:	0000001f 	0x1f
    1460:	82045849 	lb	a0,22601(s0)
	...
    1478:	0000001d 	0x1d
    147c:	0000001f 	0x1f
    1480:	82045865 	lb	a0,22629(s0)
    1484:	80030000 	lb	v1,0(zero)
    1488:	fffffffc 	0xfffffffc
	...
    1494:	00000040 	ssnop
    1498:	0000001d 	0x1d
    149c:	0000001f 	0x1f
    14a0:	82045981 	lb	a0,22913(s0)
    14a4:	80030000 	lb	v1,0(zero)
    14a8:	fffffffc 	0xfffffffc
	...
    14b4:	00000040 	ssnop
    14b8:	0000001d 	0x1d
    14bc:	0000001f 	0x1f
    14c0:	82045be1 	lb	a0,23521(s0)
    14c4:	80030000 	lb	v1,0(zero)
    14c8:	fffffffc 	0xfffffffc
	...
    14d4:	00000030 	0x30
    14d8:	0000001d 	0x1d
    14dc:	0000001f 	0x1f
    14e0:	82045d35 	lb	a0,23861(s0)
    14e4:	80030000 	lb	v1,0(zero)
    14e8:	fffffffc 	0xfffffffc
	...
    14f4:	00000028 	0x28
    14f8:	0000001d 	0x1d
    14fc:	0000001f 	0x1f
    1500:	82045df9 	lb	a0,24057(s0)
    1504:	80030000 	lb	v1,0(zero)
    1508:	fffffffc 	0xfffffffc
	...
    1514:	00000028 	0x28
    1518:	0000001d 	0x1d
    151c:	0000001f 	0x1f
    1520:	82045e45 	lb	a0,24133(s0)
	...
    1538:	0000001d 	0x1d
    153c:	0000001f 	0x1f
    1540:	82045e75 	lb	a0,24181(s0)
    1544:	80030000 	lb	v1,0(zero)
    1548:	fffffffc 	0xfffffffc
	...
    1554:	000000c8 	0xc8
    1558:	0000001d 	0x1d
    155c:	0000001f 	0x1f
    1560:	82046c25 	lb	a0,27685(s0)
    1564:	80030000 	lb	v1,0(zero)
    1568:	fffffffc 	0xfffffffc
	...
    1574:	00000068 	0x68
    1578:	0000001d 	0x1d
    157c:	0000001f 	0x1f
    1580:	820471bd 	lb	a0,29117(s0)
    1584:	80030000 	lb	v1,0(zero)
    1588:	fffffffc 	0xfffffffc
	...
    1594:	00000158 	0x158
    1598:	0000001d 	0x1d
    159c:	0000001f 	0x1f
    15a0:	8204745d 	lb	a0,29789(s0)
    15a4:	80010000 	lb	at,0(zero)
    15a8:	fffffffc 	0xfffffffc
	...
    15b4:	00000018 	mult	zero,zero
    15b8:	0000001d 	0x1d
    15bc:	0000001f 	0x1f
    15c0:	8204747d 	lb	a0,29821(s0)
	...
    15d8:	0000001d 	0x1d
    15dc:	0000001f 	0x1f
    15e0:	820474bd 	lb	a0,29885(s0)
    15e4:	80000000 	lb	zero,0(zero)
    15e8:	fffffffc 	0xfffffffc
	...
    15f4:	00000018 	mult	zero,zero
    15f8:	0000001d 	0x1d
    15fc:	0000001f 	0x1f
    1600:	820474e9 	lb	a0,29929(s0)
    1604:	80030000 	lb	v1,0(zero)
    1608:	fffffffc 	0xfffffffc
	...
    1614:	00000050 	0x50
    1618:	0000001d 	0x1d
    161c:	0000001f 	0x1f
    1620:	82047815 	lb	a0,30741(s0)
    1624:	80010000 	lb	at,0(zero)
    1628:	fffffffc 	0xfffffffc
	...
    1634:	00000018 	mult	zero,zero
    1638:	0000001d 	0x1d
    163c:	0000001f 	0x1f
    1640:	820478e1 	lb	a0,30945(s0)
    1644:	80030000 	lb	v1,0(zero)
    1648:	fffffffc 	0xfffffffc
	...
    1654:	000000a8 	0xa8
    1658:	0000001d 	0x1d
    165c:	0000001f 	0x1f
    1660:	82047ff5 	lb	a0,32757(s0)
    1664:	80030000 	lb	v1,0(zero)
    1668:	fffffffc 	0xfffffffc
	...
    1674:	00000020 	add	zero,zero,zero
    1678:	0000001d 	0x1d
    167c:	0000001f 	0x1f
    1680:	820480bd 	lb	a0,-32579(s0)
    1684:	80030000 	lb	v1,0(zero)
    1688:	fffffffc 	0xfffffffc
	...
    1694:	00000020 	add	zero,zero,zero
    1698:	0000001d 	0x1d
    169c:	0000001f 	0x1f
    16a0:	82048171 	lb	a0,-32399(s0)
    16a4:	80030000 	lb	v1,0(zero)
    16a8:	fffffffc 	0xfffffffc
	...
    16b4:	00000028 	0x28
    16b8:	0000001d 	0x1d
    16bc:	0000001f 	0x1f
    16c0:	8204823d 	lb	a0,-32195(s0)
    16c4:	80030000 	lb	v1,0(zero)
    16c8:	fffffffc 	0xfffffffc
	...
    16d4:	00000028 	0x28
    16d8:	0000001d 	0x1d
    16dc:	0000001f 	0x1f
    16e0:	800c5ba9 	lb	t4,23465(zero)
    16e4:	80030000 	lb	v1,0(zero)
    16e8:	fffffffc 	0xfffffffc
	...
    16f4:	00000020 	add	zero,zero,zero
    16f8:	0000001d 	0x1d
    16fc:	0000001f 	0x1f
    1700:	800c5c0d 	lb	t4,23565(zero)
    1704:	80030000 	lb	v1,0(zero)
    1708:	fffffffc 	0xfffffffc
	...
    1714:	00000028 	0x28
    1718:	0000001d 	0x1d
    171c:	0000001f 	0x1f
    1720:	800c5c69 	lb	t4,23657(zero)
    1724:	80010000 	lb	at,0(zero)
    1728:	fffffffc 	0xfffffffc
	...
    1734:	00000018 	mult	zero,zero
    1738:	0000001d 	0x1d
    173c:	0000001f 	0x1f
    1740:	800c5ca1 	lb	t4,23713(zero)
    1744:	80030000 	lb	v1,0(zero)
    1748:	fffffffc 	0xfffffffc
	...
    1754:	00000030 	0x30
    1758:	0000001d 	0x1d
    175c:	0000001f 	0x1f
    1760:	800c5d85 	lb	t4,23941(zero)
    1764:	80030000 	lb	v1,0(zero)
    1768:	fffffffc 	0xfffffffc
	...
    1774:	00000028 	0x28
    1778:	0000001d 	0x1d
    177c:	0000001f 	0x1f
    1780:	800c5fb1 	lb	t4,24497(zero)
    1784:	80030000 	lb	v1,0(zero)
    1788:	fffffffc 	0xfffffffc
	...
    1794:	00000028 	0x28
    1798:	0000001d 	0x1d
    179c:	0000001f 	0x1f
    17a0:	800c605d 	lb	t4,24669(zero)
    17a4:	80030000 	lb	v1,0(zero)
    17a8:	fffffffc 	0xfffffffc
	...
    17b4:	00000028 	0x28
    17b8:	0000001d 	0x1d
    17bc:	0000001f 	0x1f
    17c0:	800c628d 	lb	t4,25229(zero)
    17c4:	80010000 	lb	at,0(zero)
    17c8:	fffffffc 	0xfffffffc
	...
    17d4:	00000018 	mult	zero,zero
    17d8:	0000001d 	0x1d
    17dc:	0000001f 	0x1f
    17e0:	800c62bd 	lb	t4,25277(zero)
    17e4:	80030000 	lb	v1,0(zero)
    17e8:	fffffffc 	0xfffffffc
	...
    17f4:	00000030 	0x30
    17f8:	0000001d 	0x1d
    17fc:	0000001f 	0x1f
    1800:	800c635d 	lb	t4,25437(zero)
	...
    1818:	0000001d 	0x1d
    181c:	0000001f 	0x1f
    1820:	800c638d 	lb	t4,25485(zero)
    1824:	80030000 	lb	v1,0(zero)
    1828:	fffffffc 	0xfffffffc
	...
    1834:	00000020 	add	zero,zero,zero
    1838:	0000001d 	0x1d
    183c:	0000001f 	0x1f
    1840:	800c63c1 	lb	t4,25537(zero)
    1844:	80030000 	lb	v1,0(zero)
    1848:	fffffffc 	0xfffffffc
	...
    1854:	00000060 	0x60
    1858:	0000001d 	0x1d
    185c:	0000001f 	0x1f
    1860:	8204830d 	lb	a0,-31987(s0)
    1864:	80030000 	lb	v1,0(zero)
    1868:	fffffffc 	0xfffffffc
	...
    1874:	00000030 	0x30
    1878:	0000001d 	0x1d
    187c:	0000001f 	0x1f
    1880:	82048505 	lb	a0,-31483(s0)
    1884:	80030000 	lb	v1,0(zero)
    1888:	fffffffc 	0xfffffffc
	...
    1894:	00000038 	0x38
    1898:	0000001d 	0x1d
    189c:	0000001f 	0x1f
    18a0:	82048701 	lb	a0,-30975(s0)
    18a4:	80030000 	lb	v1,0(zero)
    18a8:	fffffffc 	0xfffffffc
	...
    18b4:	00000020 	add	zero,zero,zero
    18b8:	0000001d 	0x1d
    18bc:	0000001f 	0x1f
    18c0:	82048809 	lb	a0,-30711(s0)
    18c4:	80030000 	lb	v1,0(zero)
    18c8:	fffffffc 	0xfffffffc
	...
    18d4:	00000028 	0x28
    18d8:	0000001d 	0x1d
    18dc:	0000001f 	0x1f
    18e0:	82048959 	lb	a0,-30375(s0)
    18e4:	80030000 	lb	v1,0(zero)
    18e8:	fffffffc 	0xfffffffc
	...
    18f4:	00000020 	add	zero,zero,zero
    18f8:	0000001d 	0x1d
    18fc:	0000001f 	0x1f
    1900:	82048a4d 	lb	a0,-30131(s0)
    1904:	00010000 	sll	zero,at,0x0
    1908:	fffffffc 	0xfffffffc
	...
    1914:	00000008 	jr	zero
    1918:	0000001d 	0x1d
    191c:	0000001f 	0x1f
    1920:	82048ad9 	lb	a0,-29991(s0)
    1924:	80010000 	lb	at,0(zero)
    1928:	fffffffc 	0xfffffffc
	...
    1934:	00000020 	add	zero,zero,zero
    1938:	0000001d 	0x1d
    193c:	0000001f 	0x1f
    1940:	82048b51 	lb	a0,-29871(s0)
    1944:	80000000 	lb	zero,0(zero)
    1948:	fffffffc 	0xfffffffc
	...
    1954:	00000018 	mult	zero,zero
    1958:	0000001d 	0x1d
    195c:	0000001f 	0x1f
    1960:	82048dad 	lb	a0,-29267(s0)
    1964:	80030000 	lb	v1,0(zero)
    1968:	fffffffc 	0xfffffffc
	...
    1974:	00000078 	0x78
    1978:	0000001d 	0x1d
    197c:	0000001f 	0x1f
    1980:	8204957d 	lb	a0,-27267(s0)
    1984:	80000000 	lb	zero,0(zero)
    1988:	fffffffc 	0xfffffffc
	...
    1994:	00000018 	mult	zero,zero
    1998:	0000001d 	0x1d
    199c:	0000001f 	0x1f
    19a0:	82049599 	lb	a0,-27239(s0)
    19a4:	80030000 	lb	v1,0(zero)
    19a8:	fffffffc 	0xfffffffc
	...
    19b4:	00000028 	0x28
    19b8:	0000001d 	0x1d
    19bc:	0000001f 	0x1f
    19c0:	82049d65 	lb	a0,-25243(s0)
    19c4:	80030000 	lb	v1,0(zero)
    19c8:	fffffffc 	0xfffffffc
	...
    19d4:	00000030 	0x30
    19d8:	0000001d 	0x1d
    19dc:	0000001f 	0x1f
    19e0:	82049e1d 	lb	a0,-25059(s0)
    19e4:	80030000 	lb	v1,0(zero)
    19e8:	fffffffc 	0xfffffffc
	...
    19f4:	00000030 	0x30
    19f8:	0000001d 	0x1d
    19fc:	0000001f 	0x1f
    1a00:	800c7139 	lb	t4,28985(zero)
    1a04:	80000000 	lb	zero,0(zero)
    1a08:	fffffffc 	0xfffffffc
	...
    1a14:	00000020 	add	zero,zero,zero
    1a18:	0000001d 	0x1d
    1a1c:	0000001f 	0x1f
    1a20:	82049f1d 	lb	a0,-24803(s0)
    1a24:	80030000 	lb	v1,0(zero)
    1a28:	fffffffc 	0xfffffffc
	...
    1a34:	00000040 	ssnop
    1a38:	0000001d 	0x1d
    1a3c:	0000001f 	0x1f
    1a40:	800c7181 	lb	t4,29057(zero)
    1a44:	80000000 	lb	zero,0(zero)
    1a48:	fffffffc 	0xfffffffc
	...
    1a54:	00000018 	mult	zero,zero
    1a58:	0000001d 	0x1d
    1a5c:	0000001f 	0x1f
    1a60:	82049fdd 	lb	a0,-24611(s0)
    1a64:	80010000 	lb	at,0(zero)
    1a68:	fffffffc 	0xfffffffc
	...
    1a74:	00000018 	mult	zero,zero
    1a78:	0000001d 	0x1d
    1a7c:	0000001f 	0x1f
    1a80:	8204a09d 	lb	a0,-24419(s0)
	...
    1a98:	0000001d 	0x1d
    1a9c:	0000001f 	0x1f
    1aa0:	8204a0a1 	lb	a0,-24415(s0)
    1aa4:	80000000 	lb	zero,0(zero)
    1aa8:	fffffffc 	0xfffffffc
	...
    1ab4:	00000018 	mult	zero,zero
    1ab8:	0000001d 	0x1d
    1abc:	0000001f 	0x1f
    1ac0:	8204a0d9 	lb	a0,-24359(s0)
    1ac4:	80000000 	lb	zero,0(zero)
    1ac8:	fffffffc 	0xfffffffc
	...
    1ad4:	00000018 	mult	zero,zero
    1ad8:	0000001d 	0x1d
    1adc:	0000001f 	0x1f
    1ae0:	8204a0f9 	lb	a0,-24327(s0)
	...
    1af8:	0000001d 	0x1d
    1afc:	0000001f 	0x1f
    1b00:	8204a135 	lb	a0,-24267(s0)
	...
    1b18:	0000001d 	0x1d
    1b1c:	0000001f 	0x1f
    1b20:	8204a175 	lb	a0,-24203(s0)
    1b24:	80010000 	lb	at,0(zero)
    1b28:	fffffffc 	0xfffffffc
	...
    1b34:	00000018 	mult	zero,zero
    1b38:	0000001d 	0x1d
    1b3c:	0000001f 	0x1f
    1b40:	8204a1c5 	lb	a0,-24123(s0)
	...
    1b54:	00000008 	jr	zero
    1b58:	0000001d 	0x1d
    1b5c:	0000001f 	0x1f
    1b60:	8204a231 	lb	a0,-24015(s0)
	...
    1b78:	0000001d 	0x1d
    1b7c:	0000001f 	0x1f
    1b80:	8204a26d 	lb	a0,-23955(s0)
	...
    1b98:	0000001d 	0x1d
    1b9c:	0000001f 	0x1f
    1ba0:	8204a2ad 	lb	a0,-23891(s0)
    1ba4:	80030000 	lb	v1,0(zero)
    1ba8:	fffffffc 	0xfffffffc
	...
    1bb4:	00000028 	0x28
    1bb8:	0000001d 	0x1d
    1bbc:	0000001f 	0x1f
    1bc0:	800c7265 	lb	t4,29285(zero)
    1bc4:	80030000 	lb	v1,0(zero)
    1bc8:	fffffffc 	0xfffffffc
	...
    1bd4:	00000020 	add	zero,zero,zero
    1bd8:	0000001d 	0x1d
    1bdc:	0000001f 	0x1f
    1be0:	8204a331 	lb	a0,-23759(s0)
    1be4:	80030000 	lb	v1,0(zero)
    1be8:	fffffffc 	0xfffffffc
	...
    1bf4:	00000028 	0x28
    1bf8:	0000001d 	0x1d
    1bfc:	0000001f 	0x1f
    1c00:	8204a405 	lb	a0,-23547(s0)
    1c04:	80030000 	lb	v1,0(zero)
    1c08:	fffffffc 	0xfffffffc
	...
    1c14:	00000020 	add	zero,zero,zero
    1c18:	0000001d 	0x1d
    1c1c:	0000001f 	0x1f
    1c20:	8204a455 	lb	a0,-23467(s0)
    1c24:	80030000 	lb	v1,0(zero)
    1c28:	fffffffc 	0xfffffffc
	...
    1c34:	00000038 	0x38
    1c38:	0000001d 	0x1d
    1c3c:	0000001f 	0x1f
    1c40:	8204a78d 	lb	a0,-22643(s0)
    1c44:	80030000 	lb	v1,0(zero)
    1c48:	fffffffc 	0xfffffffc
	...
    1c54:	00000020 	add	zero,zero,zero
    1c58:	0000001d 	0x1d
    1c5c:	0000001f 	0x1f
    1c60:	8204a849 	lb	a0,-22455(s0)
    1c64:	00010000 	sll	zero,at,0x0
    1c68:	fffffffc 	0xfffffffc
	...
    1c74:	00000008 	jr	zero
    1c78:	0000001d 	0x1d
    1c7c:	0000001f 	0x1f
    1c80:	8204a8b1 	lb	a0,-22351(s0)
    1c84:	80000000 	lb	zero,0(zero)
    1c88:	fffffffc 	0xfffffffc
	...
    1c94:	00000018 	mult	zero,zero
    1c98:	0000001d 	0x1d
    1c9c:	0000001f 	0x1f
    1ca0:	8204a8d5 	lb	a0,-22315(s0)
    1ca4:	80030000 	lb	v1,0(zero)
    1ca8:	fffffffc 	0xfffffffc
	...
    1cb4:	00000020 	add	zero,zero,zero
    1cb8:	0000001d 	0x1d
    1cbc:	0000001f 	0x1f
    1cc0:	8204a931 	lb	a0,-22223(s0)
    1cc4:	80000000 	lb	zero,0(zero)
    1cc8:	fffffffc 	0xfffffffc
	...
    1cd4:	00000018 	mult	zero,zero
    1cd8:	0000001d 	0x1d
    1cdc:	0000001f 	0x1f
    1ce0:	8204a955 	lb	a0,-22187(s0)
    1ce4:	80030000 	lb	v1,0(zero)
    1ce8:	fffffffc 	0xfffffffc
	...
    1cf4:	00000078 	0x78
    1cf8:	0000001d 	0x1d
    1cfc:	0000001f 	0x1f
    1d00:	8204a9f1 	lb	a0,-22031(s0)
    1d04:	80030000 	lb	v1,0(zero)
    1d08:	fffffffc 	0xfffffffc
	...
    1d14:	00000030 	0x30
    1d18:	0000001d 	0x1d
    1d1c:	0000001f 	0x1f
    1d20:	8205f439 	lb	a1,-3015(s0)
    1d24:	80030000 	lb	v1,0(zero)
    1d28:	fffffffc 	0xfffffffc
	...
    1d34:	00000028 	0x28
    1d38:	0000001d 	0x1d
    1d3c:	0000001f 	0x1f
    1d40:	8205f46d 	lb	a1,-2963(s0)
    1d44:	80030000 	lb	v1,0(zero)
    1d48:	fffffffc 	0xfffffffc
	...
    1d54:	00000020 	add	zero,zero,zero
    1d58:	0000001d 	0x1d
    1d5c:	0000001f 	0x1f
    1d60:	8205f4b9 	lb	a1,-2887(s0)
    1d64:	80030000 	lb	v1,0(zero)
    1d68:	fffffffc 	0xfffffffc
	...
    1d74:	00000028 	0x28
    1d78:	0000001d 	0x1d
    1d7c:	0000001f 	0x1f
    1d80:	8204aae1 	lb	a0,-21791(s0)
    1d84:	80030000 	lb	v1,0(zero)
    1d88:	fffffffc 	0xfffffffc
	...
    1d94:	00000030 	0x30
    1d98:	0000001d 	0x1d
    1d9c:	0000001f 	0x1f
    1da0:	8204ab55 	lb	a0,-21675(s0)
    1da4:	80030000 	lb	v1,0(zero)
    1da8:	fffffffc 	0xfffffffc
	...
    1db4:	00000028 	0x28
    1db8:	0000001d 	0x1d
    1dbc:	0000001f 	0x1f
    1dc0:	8204abad 	lb	a0,-21587(s0)
    1dc4:	80030000 	lb	v1,0(zero)
    1dc8:	fffffffc 	0xfffffffc
	...
    1dd4:	00000028 	0x28
    1dd8:	0000001d 	0x1d
    1ddc:	0000001f 	0x1f
    1de0:	8204ac01 	lb	a0,-21503(s0)
    1de4:	80030000 	lb	v1,0(zero)
    1de8:	fffffffc 	0xfffffffc
	...
    1df4:	00000028 	0x28
    1df8:	0000001d 	0x1d
    1dfc:	0000001f 	0x1f
    1e00:	8204ac51 	lb	a0,-21423(s0)
    1e04:	80030000 	lb	v1,0(zero)
    1e08:	fffffffc 	0xfffffffc
	...
    1e14:	00000028 	0x28
    1e18:	0000001d 	0x1d
    1e1c:	0000001f 	0x1f
    1e20:	8204aca5 	lb	a0,-21339(s0)
    1e24:	80030000 	lb	v1,0(zero)
    1e28:	fffffffc 	0xfffffffc
	...
    1e34:	000000b0 	0xb0
    1e38:	0000001d 	0x1d
    1e3c:	0000001f 	0x1f
    1e40:	8204ae75 	lb	a0,-20875(s0)
    1e44:	80010000 	lb	at,0(zero)
    1e48:	fffffffc 	0xfffffffc
	...
    1e54:	00000020 	add	zero,zero,zero
    1e58:	0000001d 	0x1d
    1e5c:	0000001f 	0x1f
    1e60:	8204aec9 	lb	a0,-20791(s0)
    1e64:	80030000 	lb	v1,0(zero)
    1e68:	fffffffc 	0xfffffffc
	...
    1e74:	00000020 	add	zero,zero,zero
    1e78:	0000001d 	0x1d
    1e7c:	0000001f 	0x1f
    1e80:	8204af09 	lb	a0,-20727(s0)
    1e84:	80030000 	lb	v1,0(zero)
    1e88:	fffffffc 	0xfffffffc
	...
    1e94:	000000a8 	0xa8
    1e98:	0000001d 	0x1d
    1e9c:	0000001f 	0x1f
    1ea0:	8204b119 	lb	a0,-20199(s0)
    1ea4:	80030000 	lb	v1,0(zero)
    1ea8:	fffffffc 	0xfffffffc
	...
    1eb4:	00000028 	0x28
    1eb8:	0000001d 	0x1d
    1ebc:	0000001f 	0x1f
    1ec0:	8204b1bd 	lb	a0,-20035(s0)
    1ec4:	80030000 	lb	v1,0(zero)
    1ec8:	fffffffc 	0xfffffffc
	...
    1ed4:	00000048 	0x48
    1ed8:	0000001d 	0x1d
    1edc:	0000001f 	0x1f
    1ee0:	8204b7fd 	lb	a0,-18435(s0)
    1ee4:	80030000 	lb	v1,0(zero)
    1ee8:	fffffffc 	0xfffffffc
	...
    1ef4:	00000028 	0x28
    1ef8:	0000001d 	0x1d
    1efc:	0000001f 	0x1f
    1f00:	8204b895 	lb	a0,-18283(s0)
    1f04:	80030000 	lb	v1,0(zero)
    1f08:	fffffffc 	0xfffffffc
	...
    1f14:	00000048 	0x48
    1f18:	0000001d 	0x1d
    1f1c:	0000001f 	0x1f
    1f20:	8204bc95 	lb	a0,-17259(s0)
    1f24:	80030000 	lb	v1,0(zero)
    1f28:	fffffffc 	0xfffffffc
	...
    1f34:	00000028 	0x28
    1f38:	0000001d 	0x1d
    1f3c:	0000001f 	0x1f
    1f40:	800c73dd 	lb	t4,29661(zero)
	...
    1f58:	0000001d 	0x1d
    1f5c:	0000001f 	0x1f
    1f60:	8204bd15 	lb	a0,-17131(s0)
	...
    1f78:	0000001d 	0x1d
    1f7c:	0000001f 	0x1f
    1f80:	8204bd19 	lb	a0,-17127(s0)
	...
    1f98:	0000001d 	0x1d
    1f9c:	0000001f 	0x1f
    1fa0:	8204bd49 	lb	a0,-17079(s0)
    1fa4:	80030000 	lb	v1,0(zero)
    1fa8:	fffffffc 	0xfffffffc
	...
    1fb4:	00000038 	0x38
    1fb8:	0000001d 	0x1d
    1fbc:	0000001f 	0x1f
    1fc0:	8204bf49 	lb	a0,-16567(s0)
    1fc4:	80000000 	lb	zero,0(zero)
    1fc8:	fffffffc 	0xfffffffc
	...
    1fd4:	00000018 	mult	zero,zero
    1fd8:	0000001d 	0x1d
    1fdc:	0000001f 	0x1f
    1fe0:	8204bfb9 	lb	a0,-16455(s0)
    1fe4:	80000000 	lb	zero,0(zero)
    1fe8:	fffffffc 	0xfffffffc
	...
    1ff4:	00000018 	mult	zero,zero
    1ff8:	0000001d 	0x1d
    1ffc:	0000001f 	0x1f
    2000:	8204bfe1 	lb	a0,-16415(s0)
	...
    2018:	0000001d 	0x1d
    201c:	0000001f 	0x1f
    2020:	8204bff1 	lb	a0,-16399(s0)
    2024:	80000000 	lb	zero,0(zero)
    2028:	fffffffc 	0xfffffffc
	...
    2034:	00000018 	mult	zero,zero
    2038:	0000001d 	0x1d
    203c:	0000001f 	0x1f
    2040:	8204c015 	lb	a0,-16363(s0)
    2044:	80000000 	lb	zero,0(zero)
    2048:	fffffffc 	0xfffffffc
	...
    2054:	00000018 	mult	zero,zero
    2058:	0000001d 	0x1d
    205c:	0000001f 	0x1f
    2060:	8204c039 	lb	a0,-16327(s0)
    2064:	80000000 	lb	zero,0(zero)
    2068:	fffffffc 	0xfffffffc
	...
    2074:	00000020 	add	zero,zero,zero
    2078:	0000001d 	0x1d
    207c:	0000001f 	0x1f
    2080:	8204c0a1 	lb	a0,-16223(s0)
    2084:	80030000 	lb	v1,0(zero)
    2088:	fffffffc 	0xfffffffc
	...
    2094:	00000038 	0x38
    2098:	0000001d 	0x1d
    209c:	0000001f 	0x1f
    20a0:	8204c235 	lb	a0,-15819(s0)
	...
    20b8:	0000001d 	0x1d
    20bc:	0000001f 	0x1f
    20c0:	8204c261 	lb	a0,-15775(s0)
    20c4:	80030000 	lb	v1,0(zero)
    20c8:	fffffffc 	0xfffffffc
	...
    20d4:	00000040 	ssnop
    20d8:	0000001d 	0x1d
    20dc:	0000001f 	0x1f
    20e0:	8204c3e1 	lb	a0,-15391(s0)
	...
    20f8:	0000001d 	0x1d
    20fc:	0000001f 	0x1f
    2100:	8204c411 	lb	a0,-15343(s0)
	...
    2118:	0000001d 	0x1d
    211c:	0000001f 	0x1f
    2120:	8204c415 	lb	a0,-15339(s0)
	...
    2138:	0000001d 	0x1d
    213c:	0000001f 	0x1f
    2140:	800c7621 	lb	t4,30241(zero)
    2144:	80030000 	lb	v1,0(zero)
    2148:	fffffffc 	0xfffffffc
	...
    2154:	00000030 	0x30
    2158:	0000001d 	0x1d
    215c:	0000001f 	0x1f
    2160:	8204c419 	lb	a0,-15335(s0)
    2164:	80030000 	lb	v1,0(zero)
    2168:	fffffffc 	0xfffffffc
	...
    2174:	00000078 	0x78
    2178:	0000001d 	0x1d
    217c:	0000001f 	0x1f
    2180:	800c76e1 	lb	t4,30433(zero)
    2184:	80030000 	lb	v1,0(zero)
    2188:	fffffffc 	0xfffffffc
	...
    2194:	00000030 	0x30
    2198:	0000001d 	0x1d
    219c:	0000001f 	0x1f
    21a0:	800c7999 	lb	t4,31129(zero)
    21a4:	80030000 	lb	v1,0(zero)
    21a8:	fffffffc 	0xfffffffc
	...
    21b4:	00000020 	add	zero,zero,zero
    21b8:	0000001d 	0x1d
    21bc:	0000001f 	0x1f
    21c0:	800c79e9 	lb	t4,31209(zero)
    21c4:	80030000 	lb	v1,0(zero)
    21c8:	fffffffc 	0xfffffffc
	...
    21d4:	00000030 	0x30
    21d8:	0000001d 	0x1d
    21dc:	0000001f 	0x1f
    21e0:	8204c7d9 	lb	a0,-14375(s0)
    21e4:	80030000 	lb	v1,0(zero)
    21e8:	fffffffc 	0xfffffffc
	...
    21f4:	00000020 	add	zero,zero,zero
    21f8:	0000001d 	0x1d
    21fc:	0000001f 	0x1f
    2200:	800c7c05 	lb	t4,31749(zero)
    2204:	80030000 	lb	v1,0(zero)
    2208:	fffffffc 	0xfffffffc
	...
    2214:	00000038 	0x38
    2218:	0000001d 	0x1d
    221c:	0000001f 	0x1f
    2220:	800c8111 	lb	t4,-32495(zero)
    2224:	80030000 	lb	v1,0(zero)
    2228:	fffffffc 	0xfffffffc
	...
    2234:	00000030 	0x30
    2238:	0000001d 	0x1d
    223c:	0000001f 	0x1f
    2240:	8204c8e5 	lb	a0,-14107(s0)
    2244:	80030000 	lb	v1,0(zero)
    2248:	fffffffc 	0xfffffffc
	...
    2254:	00000028 	0x28
    2258:	0000001d 	0x1d
    225c:	0000001f 	0x1f
    2260:	8204ca4d 	lb	a0,-13747(s0)
	...
    2278:	0000001d 	0x1d
    227c:	0000001f 	0x1f
    2280:	800c82c5 	lb	t4,-32059(zero)
    2284:	80030000 	lb	v1,0(zero)
    2288:	fffffffc 	0xfffffffc
	...
    2294:	00000030 	0x30
    2298:	0000001d 	0x1d
    229c:	0000001f 	0x1f
    22a0:	800c8605 	lb	t4,-31227(zero)
    22a4:	80030000 	lb	v1,0(zero)
    22a8:	fffffffc 	0xfffffffc
	...
    22b4:	00000028 	0x28
    22b8:	0000001d 	0x1d
    22bc:	0000001f 	0x1f
    22c0:	800c86a9 	lb	t4,-31063(zero)
    22c4:	80030000 	lb	v1,0(zero)
    22c8:	fffffffc 	0xfffffffc
	...
    22d4:	00000020 	add	zero,zero,zero
    22d8:	0000001d 	0x1d
    22dc:	0000001f 	0x1f
    22e0:	8204ca71 	lb	a0,-13711(s0)
    22e4:	80030000 	lb	v1,0(zero)
    22e8:	fffffffc 	0xfffffffc
	...
    22f4:	00000028 	0x28
    22f8:	0000001d 	0x1d
    22fc:	0000001f 	0x1f
    2300:	8204cb39 	lb	a0,-13511(s0)
    2304:	80010000 	lb	at,0(zero)
    2308:	fffffffc 	0xfffffffc
	...
    2314:	00000018 	mult	zero,zero
    2318:	0000001d 	0x1d
    231c:	0000001f 	0x1f
    2320:	8204cb79 	lb	a0,-13447(s0)
    2324:	80010000 	lb	at,0(zero)
    2328:	fffffffc 	0xfffffffc
	...
    2334:	00000018 	mult	zero,zero
    2338:	0000001d 	0x1d
    233c:	0000001f 	0x1f
    2340:	8204cbf9 	lb	a0,-13319(s0)
    2344:	80030000 	lb	v1,0(zero)
    2348:	fffffffc 	0xfffffffc
	...
    2354:	00000040 	ssnop
    2358:	0000001d 	0x1d
    235c:	0000001f 	0x1f
    2360:	8204ce01 	lb	a0,-12799(s0)
    2364:	80030000 	lb	v1,0(zero)
    2368:	fffffffc 	0xfffffffc
	...
    2374:	00000030 	0x30
    2378:	0000001d 	0x1d
    237c:	0000001f 	0x1f
    2380:	8204cf9d 	lb	a0,-12387(s0)
    2384:	80030000 	lb	v1,0(zero)
    2388:	fffffffc 	0xfffffffc
	...
    2394:	00000048 	0x48
    2398:	0000001d 	0x1d
    239c:	0000001f 	0x1f
    23a0:	8204d135 	lb	a0,-11979(s0)
    23a4:	80030000 	lb	v1,0(zero)
    23a8:	fffffffc 	0xfffffffc
	...
    23b4:	00000030 	0x30
    23b8:	0000001d 	0x1d
    23bc:	0000001f 	0x1f
    23c0:	800c87a1 	lb	t4,-30815(zero)
    23c4:	80030000 	lb	v1,0(zero)
    23c8:	fffffffc 	0xfffffffc
	...
    23d4:	00000038 	0x38
    23d8:	0000001d 	0x1d
    23dc:	0000001f 	0x1f
    23e0:	800c8a5d 	lb	t4,-30115(zero)
    23e4:	80030000 	lb	v1,0(zero)
    23e8:	fffffffc 	0xfffffffc
	...
    23f4:	00000030 	0x30
    23f8:	0000001d 	0x1d
    23fc:	0000001f 	0x1f
    2400:	800c672d 	lb	t4,26413(zero)
    2404:	80030000 	lb	v1,0(zero)
    2408:	fffffffc 	0xfffffffc
	...
    2414:	00000030 	0x30
    2418:	0000001d 	0x1d
    241c:	0000001f 	0x1f
    2420:	800c684d 	lb	t4,26701(zero)
    2424:	80030000 	lb	v1,0(zero)
    2428:	fffffffc 	0xfffffffc
	...
    2434:	00000030 	0x30
    2438:	0000001d 	0x1d
    243c:	0000001f 	0x1f
    2440:	800c6941 	lb	t4,26945(zero)
    2444:	80030000 	lb	v1,0(zero)
    2448:	fffffffc 	0xfffffffc
	...
    2454:	00000040 	ssnop
    2458:	0000001d 	0x1d
    245c:	0000001f 	0x1f
    2460:	800c6a89 	lb	t4,27273(zero)
    2464:	80030000 	lb	v1,0(zero)
    2468:	fffffffc 	0xfffffffc
	...
    2474:	00000040 	ssnop
    2478:	0000001d 	0x1d
    247c:	0000001f 	0x1f
    2480:	800c6bf5 	lb	t4,27637(zero)
    2484:	80030000 	lb	v1,0(zero)
    2488:	fffffffc 	0xfffffffc
	...
    2494:	00000028 	0x28
    2498:	0000001d 	0x1d
    249c:	0000001f 	0x1f
    24a0:	800c6c61 	lb	t4,27745(zero)
    24a4:	80030000 	lb	v1,0(zero)
    24a8:	fffffffc 	0xfffffffc
	...
    24b4:	00000030 	0x30
    24b8:	0000001d 	0x1d
    24bc:	0000001f 	0x1f
    24c0:	800c6cb1 	lb	t4,27825(zero)
    24c4:	80030000 	lb	v1,0(zero)
    24c8:	fffffffc 	0xfffffffc
	...
    24d4:	00000030 	0x30
    24d8:	0000001d 	0x1d
    24dc:	0000001f 	0x1f
    24e0:	800c6d75 	lb	t4,28021(zero)
    24e4:	80030000 	lb	v1,0(zero)
    24e8:	fffffffc 	0xfffffffc
	...
    24f4:	00000028 	0x28
    24f8:	0000001d 	0x1d
    24fc:	0000001f 	0x1f
    2500:	800c6e2d 	lb	t4,28205(zero)
    2504:	80030000 	lb	v1,0(zero)
    2508:	fffffffc 	0xfffffffc
	...
    2514:	00000050 	0x50
    2518:	0000001d 	0x1d
    251c:	0000001f 	0x1f
    2520:	800c6fe1 	lb	t4,28641(zero)
    2524:	80030000 	lb	v1,0(zero)
    2528:	fffffffc 	0xfffffffc
	...
    2534:	00000048 	0x48
    2538:	0000001d 	0x1d
    253c:	0000001f 	0x1f
    2540:	800c1cc9 	lb	t4,7369(zero)
    2544:	80000000 	lb	zero,0(zero)
    2548:	fffffffc 	0xfffffffc
	...
    2554:	00000018 	mult	zero,zero
    2558:	0000001d 	0x1d
    255c:	0000001f 	0x1f
    2560:	800c1d69 	lb	t4,7529(zero)
    2564:	80030000 	lb	v1,0(zero)
    2568:	fffffffc 	0xfffffffc
	...
    2574:	00000040 	ssnop
    2578:	0000001d 	0x1d
    257c:	0000001f 	0x1f
    2580:	800c1e15 	lb	t4,7701(zero)
    2584:	80000000 	lb	zero,0(zero)
    2588:	fffffffc 	0xfffffffc
	...
    2594:	00000018 	mult	zero,zero
    2598:	0000001d 	0x1d
    259c:	0000001f 	0x1f
    25a0:	800c1e79 	lb	t4,7801(zero)
    25a4:	80030000 	lb	v1,0(zero)
    25a8:	fffffffc 	0xfffffffc
	...
    25b4:	00000020 	add	zero,zero,zero
    25b8:	0000001d 	0x1d
    25bc:	0000001f 	0x1f
    25c0:	800c1f41 	lb	t4,8001(zero)
    25c4:	80030000 	lb	v1,0(zero)
    25c8:	fffffffc 	0xfffffffc
	...
    25d4:	00000028 	0x28
    25d8:	0000001d 	0x1d
    25dc:	0000001f 	0x1f
    25e0:	800c2061 	lb	t4,8289(zero)
    25e4:	80030000 	lb	v1,0(zero)
    25e8:	fffffffc 	0xfffffffc
	...
    25f4:	00000030 	0x30
    25f8:	0000001d 	0x1d
    25fc:	0000001f 	0x1f
    2600:	800c219d 	lb	t4,8605(zero)
    2604:	80030000 	lb	v1,0(zero)
    2608:	fffffffc 	0xfffffffc
	...
    2614:	00000028 	0x28
    2618:	0000001d 	0x1d
    261c:	0000001f 	0x1f
    2620:	800c230d 	lb	t4,8973(zero)
    2624:	80000000 	lb	zero,0(zero)
    2628:	fffffffc 	0xfffffffc
	...
    2634:	00000018 	mult	zero,zero
    2638:	0000001d 	0x1d
    263c:	0000001f 	0x1f
    2640:	800c234d 	lb	t4,9037(zero)
    2644:	80010000 	lb	at,0(zero)
    2648:	fffffffc 	0xfffffffc
	...
    2654:	00000018 	mult	zero,zero
    2658:	0000001d 	0x1d
    265c:	0000001f 	0x1f
    2660:	800c2415 	lb	t4,9237(zero)
    2664:	80030000 	lb	v1,0(zero)
    2668:	fffffffc 	0xfffffffc
	...
    2674:	00000030 	0x30
    2678:	0000001d 	0x1d
    267c:	0000001f 	0x1f
    2680:	800c24fd 	lb	t4,9469(zero)
    2684:	80010000 	lb	at,0(zero)
    2688:	fffffffc 	0xfffffffc
	...
    2694:	00000018 	mult	zero,zero
    2698:	0000001d 	0x1d
    269c:	0000001f 	0x1f
    26a0:	800c2535 	lb	t4,9525(zero)
    26a4:	80030000 	lb	v1,0(zero)
    26a8:	fffffffc 	0xfffffffc
	...
    26b4:	00000028 	0x28
    26b8:	0000001d 	0x1d
    26bc:	0000001f 	0x1f
    26c0:	800c25e1 	lb	t4,9697(zero)
    26c4:	80030000 	lb	v1,0(zero)
    26c8:	fffffffc 	0xfffffffc
	...
    26d4:	00000030 	0x30
    26d8:	0000001d 	0x1d
    26dc:	0000001f 	0x1f
    26e0:	800c2685 	lb	t4,9861(zero)
    26e4:	80010000 	lb	at,0(zero)
    26e8:	fffffffc 	0xfffffffc
	...
    26f4:	00000018 	mult	zero,zero
    26f8:	0000001d 	0x1d
    26fc:	0000001f 	0x1f
    2700:	800c26bd 	lb	t4,9917(zero)
    2704:	80030000 	lb	v1,0(zero)
    2708:	fffffffc 	0xfffffffc
	...
    2714:	00000020 	add	zero,zero,zero
    2718:	0000001d 	0x1d
    271c:	0000001f 	0x1f
    2720:	800c272d 	lb	t4,10029(zero)
	...
    2738:	0000001d 	0x1d
    273c:	0000001f 	0x1f
    2740:	800c2731 	lb	t4,10033(zero)
    2744:	80030000 	lb	v1,0(zero)
    2748:	fffffffc 	0xfffffffc
	...
    2754:	00000028 	0x28
    2758:	0000001d 	0x1d
    275c:	0000001f 	0x1f
    2760:	8204d38d 	lb	a0,-11379(s0)
    2764:	80030000 	lb	v1,0(zero)
    2768:	fffffffc 	0xfffffffc
	...
    2774:	00000028 	0x28
    2778:	0000001d 	0x1d
    277c:	0000001f 	0x1f
    2780:	8204d3ed 	lb	a0,-11283(s0)
	...
    2798:	0000001d 	0x1d
    279c:	0000001f 	0x1f
    27a0:	8204d41d 	lb	a0,-11235(s0)
    27a4:	80030000 	lb	v1,0(zero)
    27a8:	fffffffc 	0xfffffffc
	...
    27b4:	00000020 	add	zero,zero,zero
    27b8:	0000001d 	0x1d
    27bc:	0000001f 	0x1f
    27c0:	8204d459 	lb	a0,-11175(s0)
    27c4:	80030000 	lb	v1,0(zero)
    27c8:	fffffffc 	0xfffffffc
	...
    27d4:	00000020 	add	zero,zero,zero
    27d8:	0000001d 	0x1d
    27dc:	0000001f 	0x1f
    27e0:	8204d4cd 	lb	a0,-11059(s0)
    27e4:	80030000 	lb	v1,0(zero)
    27e8:	fffffffc 	0xfffffffc
	...
    27f4:	00000070 	0x70
    27f8:	0000001d 	0x1d
    27fc:	0000001f 	0x1f
    2800:	8204d675 	lb	a0,-10635(s0)
    2804:	80030000 	lb	v1,0(zero)
    2808:	fffffffc 	0xfffffffc
	...
    2814:	00000030 	0x30
    2818:	0000001d 	0x1d
    281c:	0000001f 	0x1f
    2820:	8204d7dd 	lb	a0,-10275(s0)
    2824:	80000000 	lb	zero,0(zero)
    2828:	fffffffc 	0xfffffffc
	...
    2834:	00000018 	mult	zero,zero
    2838:	0000001d 	0x1d
    283c:	0000001f 	0x1f
    2840:	8204d809 	lb	a0,-10231(s0)
    2844:	00030000 	sll	zero,v1,0x0
    2848:	fffffffc 	0xfffffffc
	...
    2854:	00000008 	jr	zero
    2858:	0000001d 	0x1d
    285c:	0000001f 	0x1f
    2860:	800c8c55 	lb	t4,-29611(zero)
    2864:	80030000 	lb	v1,0(zero)
    2868:	fffffffc 	0xfffffffc
	...
    2874:	00000038 	0x38
    2878:	0000001d 	0x1d
    287c:	0000001f 	0x1f
    2880:	8204d889 	lb	a0,-10103(s0)
    2884:	80030000 	lb	v1,0(zero)
    2888:	fffffffc 	0xfffffffc
	...
    2894:	00000028 	0x28
    2898:	0000001d 	0x1d
    289c:	0000001f 	0x1f
    28a0:	8204d9d9 	lb	a0,-9767(s0)
    28a4:	80030000 	lb	v1,0(zero)
    28a8:	fffffffc 	0xfffffffc
	...
    28b4:	00000030 	0x30
    28b8:	0000001d 	0x1d
    28bc:	0000001f 	0x1f
    28c0:	8204dd45 	lb	a0,-8891(s0)
	...
    28d8:	0000001d 	0x1d
    28dc:	0000001f 	0x1f
    28e0:	8204dd49 	lb	a0,-8887(s0)
	...
    28f8:	0000001d 	0x1d
    28fc:	0000001f 	0x1f
    2900:	8204dd75 	lb	a0,-8843(s0)
    2904:	80030000 	lb	v1,0(zero)
    2908:	fffffffc 	0xfffffffc
	...
    2914:	00000038 	0x38
    2918:	0000001d 	0x1d
    291c:	0000001f 	0x1f
    2920:	8204ded5 	lb	a0,-8491(s0)
    2924:	80010000 	lb	at,0(zero)
    2928:	fffffffc 	0xfffffffc
	...
    2934:	00000020 	add	zero,zero,zero
    2938:	0000001d 	0x1d
    293c:	0000001f 	0x1f
    2940:	8204df6d 	lb	a0,-8339(s0)
    2944:	80030000 	lb	v1,0(zero)
    2948:	fffffffc 	0xfffffffc
	...
    2954:	00000030 	0x30
    2958:	0000001d 	0x1d
    295c:	0000001f 	0x1f
    2960:	8204e239 	lb	a0,-7623(s0)
    2964:	80030000 	lb	v1,0(zero)
    2968:	fffffffc 	0xfffffffc
	...
    2974:	00000030 	0x30
    2978:	0000001d 	0x1d
    297c:	0000001f 	0x1f
    2980:	8204e3bd 	lb	a0,-7235(s0)
    2984:	80030000 	lb	v1,0(zero)
    2988:	fffffffc 	0xfffffffc
	...
    2994:	00000060 	0x60
    2998:	0000001d 	0x1d
    299c:	0000001f 	0x1f
    29a0:	800c28b9 	lb	t4,10425(zero)
	...
    29b8:	0000001d 	0x1d
    29bc:	0000001f 	0x1f
    29c0:	800c28ed 	lb	t4,10477(zero)
    29c4:	80000000 	lb	zero,0(zero)
    29c8:	fffffffc 	0xfffffffc
	...
    29d4:	00000018 	mult	zero,zero
    29d8:	0000001d 	0x1d
    29dc:	0000001f 	0x1f
    29e0:	800c2929 	lb	t4,10537(zero)
    29e4:	80000000 	lb	zero,0(zero)
    29e8:	fffffffc 	0xfffffffc
	...
    29f4:	00000018 	mult	zero,zero
    29f8:	0000001d 	0x1d
    29fc:	0000001f 	0x1f
    2a00:	800c296d 	lb	t4,10605(zero)
	...
    2a18:	0000001d 	0x1d
    2a1c:	0000001f 	0x1f
    2a20:	800c2995 	lb	t4,10645(zero)
	...
    2a38:	0000001d 	0x1d
    2a3c:	0000001f 	0x1f
    2a40:	800c29ad 	lb	t4,10669(zero)
    2a44:	80030000 	lb	v1,0(zero)
    2a48:	fffffffc 	0xfffffffc
	...
    2a54:	00000020 	add	zero,zero,zero
    2a58:	0000001d 	0x1d
    2a5c:	0000001f 	0x1f
    2a60:	800c2a21 	lb	t4,10785(zero)
    2a64:	00030000 	sll	zero,v1,0x0
    2a68:	fffffffc 	0xfffffffc
	...
    2a74:	00000008 	jr	zero
    2a78:	0000001d 	0x1d
    2a7c:	0000001f 	0x1f
    2a80:	800c2aa1 	lb	t4,10913(zero)
    2a84:	80030000 	lb	v1,0(zero)
    2a88:	fffffffc 	0xfffffffc
	...
    2a94:	00000030 	0x30
    2a98:	0000001d 	0x1d
    2a9c:	0000001f 	0x1f
    2aa0:	800c2b2d 	lb	t4,11053(zero)
    2aa4:	80030000 	lb	v1,0(zero)
    2aa8:	fffffffc 	0xfffffffc
	...
    2ab4:	00000028 	0x28
    2ab8:	0000001d 	0x1d
    2abc:	0000001f 	0x1f
    2ac0:	800c2b79 	lb	t4,11129(zero)
	...
    2ad8:	0000001d 	0x1d
    2adc:	0000001f 	0x1f
    2ae0:	800c2bad 	lb	t4,11181(zero)
    2ae4:	80030000 	lb	v1,0(zero)
    2ae8:	fffffffc 	0xfffffffc
	...
    2af4:	00000028 	0x28
    2af8:	0000001d 	0x1d
    2afc:	0000001f 	0x1f
    2b00:	800c2db5 	lb	t4,11701(zero)
    2b04:	80030000 	lb	v1,0(zero)
    2b08:	fffffffc 	0xfffffffc
	...
    2b14:	00000020 	add	zero,zero,zero
    2b18:	0000001d 	0x1d
    2b1c:	0000001f 	0x1f
    2b20:	800c2e49 	lb	t4,11849(zero)
	...
    2b38:	0000001d 	0x1d
    2b3c:	0000001f 	0x1f
    2b40:	800c2eb9 	lb	t4,11961(zero)
    2b44:	80000000 	lb	zero,0(zero)
    2b48:	fffffffc 	0xfffffffc
	...
    2b54:	00000018 	mult	zero,zero
    2b58:	0000001d 	0x1d
    2b5c:	0000001f 	0x1f
    2b60:	800c2fc5 	lb	t4,12229(zero)
	...
    2b78:	0000001d 	0x1d
    2b7c:	0000001f 	0x1f
    2b80:	800c2ff5 	lb	t4,12277(zero)
	...
    2b98:	0000001d 	0x1d
    2b9c:	0000001f 	0x1f
    2ba0:	800c3021 	lb	t4,12321(zero)
    2ba4:	80030000 	lb	v1,0(zero)
    2ba8:	fffffffc 	0xfffffffc
	...
    2bb4:	00000028 	0x28
    2bb8:	0000001d 	0x1d
    2bbc:	0000001f 	0x1f
    2bc0:	800c30f9 	lb	t4,12537(zero)
	...
    2bd8:	0000001d 	0x1d
    2bdc:	0000001f 	0x1f
    2be0:	800c3169 	lb	t4,12649(zero)
    2be4:	80030000 	lb	v1,0(zero)
    2be8:	fffffffc 	0xfffffffc
	...
    2bf4:	00000028 	0x28
    2bf8:	0000001d 	0x1d
    2bfc:	0000001f 	0x1f
    2c00:	800c32cd 	lb	t4,13005(zero)
    2c04:	00030000 	sll	zero,v1,0x0
    2c08:	fffffffc 	0xfffffffc
	...
    2c14:	00000008 	jr	zero
    2c18:	0000001d 	0x1d
    2c1c:	0000001f 	0x1f
    2c20:	800c33a5 	lb	t4,13221(zero)
	...
    2c38:	0000001d 	0x1d
    2c3c:	0000001f 	0x1f
    2c40:	800c3411 	lb	t4,13329(zero)
    2c44:	80000000 	lb	zero,0(zero)
    2c48:	fffffffc 	0xfffffffc
	...
    2c54:	00000018 	mult	zero,zero
    2c58:	0000001d 	0x1d
    2c5c:	0000001f 	0x1f
    2c60:	800c3461 	lb	t4,13409(zero)
    2c64:	80030000 	lb	v1,0(zero)
    2c68:	fffffffc 	0xfffffffc
	...
    2c74:	00000028 	0x28
    2c78:	0000001d 	0x1d
    2c7c:	0000001f 	0x1f
    2c80:	800c3871 	lb	t4,14449(zero)
    2c84:	80010000 	lb	at,0(zero)
    2c88:	fffffffc 	0xfffffffc
	...
    2c94:	00000018 	mult	zero,zero
    2c98:	0000001d 	0x1d
    2c9c:	0000001f 	0x1f
    2ca0:	800c38b9 	lb	t4,14521(zero)
    2ca4:	80030000 	lb	v1,0(zero)
    2ca8:	fffffffc 	0xfffffffc
	...
    2cb4:	00000038 	0x38
    2cb8:	0000001d 	0x1d
    2cbc:	0000001f 	0x1f
    2cc0:	800c3a01 	lb	t4,14849(zero)
    2cc4:	80030000 	lb	v1,0(zero)
    2cc8:	fffffffc 	0xfffffffc
	...
    2cd4:	00000028 	0x28
    2cd8:	0000001d 	0x1d
    2cdc:	0000001f 	0x1f
    2ce0:	800c3ad1 	lb	t4,15057(zero)
    2ce4:	80000000 	lb	zero,0(zero)
    2ce8:	fffffffc 	0xfffffffc
	...
    2cf4:	00000028 	0x28
    2cf8:	0000001d 	0x1d
    2cfc:	0000001f 	0x1f
    2d00:	800c9085 	lb	t4,-28539(zero)
    2d04:	80030000 	lb	v1,0(zero)
    2d08:	fffffffc 	0xfffffffc
	...
    2d14:	00000030 	0x30
    2d18:	0000001d 	0x1d
    2d1c:	0000001f 	0x1f
    2d20:	800c9165 	lb	t4,-28315(zero)
    2d24:	80030000 	lb	v1,0(zero)
    2d28:	fffffffc 	0xfffffffc
	...
    2d34:	00000028 	0x28
    2d38:	0000001d 	0x1d
    2d3c:	0000001f 	0x1f
    2d40:	800c923d 	lb	t4,-28099(zero)
    2d44:	80030000 	lb	v1,0(zero)
    2d48:	fffffffc 	0xfffffffc
	...
    2d54:	00000020 	add	zero,zero,zero
    2d58:	0000001d 	0x1d
    2d5c:	0000001f 	0x1f
    2d60:	800c9279 	lb	t4,-28039(zero)
    2d64:	80010000 	lb	at,0(zero)
    2d68:	fffffffc 	0xfffffffc
	...
    2d74:	00000018 	mult	zero,zero
    2d78:	0000001d 	0x1d
    2d7c:	0000001f 	0x1f
    2d80:	800c92b5 	lb	t4,-27979(zero)
    2d84:	80010000 	lb	at,0(zero)
    2d88:	fffffffc 	0xfffffffc
	...
    2d94:	00000018 	mult	zero,zero
    2d98:	0000001d 	0x1d
    2d9c:	0000001f 	0x1f
    2da0:	800c92f1 	lb	t4,-27919(zero)
    2da4:	80030000 	lb	v1,0(zero)
    2da8:	fffffffc 	0xfffffffc
	...
    2db4:	00000030 	0x30
    2db8:	0000001d 	0x1d
    2dbc:	0000001f 	0x1f
    2dc0:	800c937d 	lb	t4,-27779(zero)
    2dc4:	80010000 	lb	at,0(zero)
    2dc8:	fffffffc 	0xfffffffc
	...
    2dd4:	00000020 	add	zero,zero,zero
    2dd8:	0000001d 	0x1d
    2ddc:	0000001f 	0x1f
    2de0:	800c9409 	lb	t4,-27639(zero)
    2de4:	80030000 	lb	v1,0(zero)
    2de8:	fffffffc 	0xfffffffc
	...
    2df4:	00000030 	0x30
    2df8:	0000001d 	0x1d
    2dfc:	0000001f 	0x1f
    2e00:	800c9479 	lb	t4,-27527(zero)
    2e04:	80030000 	lb	v1,0(zero)
    2e08:	fffffffc 	0xfffffffc
	...
    2e14:	00000030 	0x30
    2e18:	0000001d 	0x1d
    2e1c:	0000001f 	0x1f
    2e20:	800c951d 	lb	t4,-27363(zero)
    2e24:	80030000 	lb	v1,0(zero)
    2e28:	fffffffc 	0xfffffffc
	...
    2e34:	00000030 	0x30
    2e38:	0000001d 	0x1d
    2e3c:	0000001f 	0x1f
    2e40:	800c958d 	lb	t4,-27251(zero)
    2e44:	80030000 	lb	v1,0(zero)
    2e48:	fffffffc 	0xfffffffc
	...
    2e54:	00000030 	0x30
    2e58:	0000001d 	0x1d
    2e5c:	0000001f 	0x1f
    2e60:	800c9631 	lb	t4,-27087(zero)
    2e64:	80030000 	lb	v1,0(zero)
    2e68:	fffffffc 	0xfffffffc
	...
    2e74:	00000058 	0x58
    2e78:	0000001d 	0x1d
    2e7c:	0000001f 	0x1f
    2e80:	800c9b61 	lb	t4,-25759(zero)
    2e84:	80030000 	lb	v1,0(zero)
    2e88:	fffffffc 	0xfffffffc
	...
    2e94:	00000028 	0x28
    2e98:	0000001d 	0x1d
    2e9c:	0000001f 	0x1f
    2ea0:	800c9c49 	lb	t4,-25527(zero)
    2ea4:	80000000 	lb	zero,0(zero)
    2ea8:	fffffffc 	0xfffffffc
	...
    2eb4:	00000018 	mult	zero,zero
    2eb8:	0000001d 	0x1d
    2ebc:	0000001f 	0x1f
    2ec0:	800c9c69 	lb	t4,-25495(zero)
    2ec4:	80030000 	lb	v1,0(zero)
    2ec8:	fffffffc 	0xfffffffc
	...
    2ed4:	00000028 	0x28
    2ed8:	0000001d 	0x1d
    2edc:	0000001f 	0x1f
    2ee0:	800c9d01 	lb	t4,-25343(zero)
	...
    2ef8:	0000001d 	0x1d
    2efc:	0000001f 	0x1f
    2f00:	800c9d25 	lb	t4,-25307(zero)
	...
    2f18:	0000001d 	0x1d
    2f1c:	0000001f 	0x1f
    2f20:	800c9d49 	lb	t4,-25271(zero)
	...
    2f38:	0000001d 	0x1d
    2f3c:	0000001f 	0x1f
    2f40:	800c9da9 	lb	t4,-25175(zero)
    2f44:	80030000 	lb	v1,0(zero)
    2f48:	fffffffc 	0xfffffffc
	...
    2f54:	00000020 	add	zero,zero,zero
    2f58:	0000001d 	0x1d
    2f5c:	0000001f 	0x1f
    2f60:	800c9e01 	lb	t4,-25087(zero)
    2f64:	80030000 	lb	v1,0(zero)
    2f68:	fffffffc 	0xfffffffc
	...
    2f74:	00000020 	add	zero,zero,zero
    2f78:	0000001d 	0x1d
    2f7c:	0000001f 	0x1f
    2f80:	800c9e2d 	lb	t4,-25043(zero)
    2f84:	80030000 	lb	v1,0(zero)
    2f88:	fffffffc 	0xfffffffc
	...
    2f94:	00000028 	0x28
    2f98:	0000001d 	0x1d
    2f9c:	0000001f 	0x1f
    2fa0:	800c9ea9 	lb	t4,-24919(zero)
	...
    2fb8:	0000001d 	0x1d
    2fbc:	0000001f 	0x1f
    2fc0:	800c9f0d 	lb	t4,-24819(zero)
    2fc4:	80030000 	lb	v1,0(zero)
    2fc8:	fffffffc 	0xfffffffc
	...
    2fd4:	00000020 	add	zero,zero,zero
    2fd8:	0000001d 	0x1d
    2fdc:	0000001f 	0x1f
    2fe0:	800c9f69 	lb	t4,-24727(zero)
    2fe4:	80030000 	lb	v1,0(zero)
    2fe8:	fffffffc 	0xfffffffc
	...
    2ff4:	00000020 	add	zero,zero,zero
    2ff8:	0000001d 	0x1d
    2ffc:	0000001f 	0x1f
    3000:	800c9fc1 	lb	t4,-24639(zero)
    3004:	80030000 	lb	v1,0(zero)
    3008:	fffffffc 	0xfffffffc
	...
    3014:	00000020 	add	zero,zero,zero
    3018:	0000001d 	0x1d
    301c:	0000001f 	0x1f
    3020:	800ca059 	lb	t4,-24487(zero)
    3024:	80030000 	lb	v1,0(zero)
    3028:	fffffffc 	0xfffffffc
	...
    3034:	00000030 	0x30
    3038:	0000001d 	0x1d
    303c:	0000001f 	0x1f
    3040:	800ca0f1 	lb	t4,-24335(zero)
    3044:	80030000 	lb	v1,0(zero)
    3048:	fffffffc 	0xfffffffc
	...
    3054:	00000020 	add	zero,zero,zero
    3058:	0000001d 	0x1d
    305c:	0000001f 	0x1f
    3060:	800ca16d 	lb	t4,-24211(zero)
    3064:	80000000 	lb	zero,0(zero)
    3068:	fffffffc 	0xfffffffc
	...
    3074:	00000018 	mult	zero,zero
    3078:	0000001d 	0x1d
    307c:	0000001f 	0x1f
    3080:	800ca195 	lb	t4,-24171(zero)
    3084:	80010000 	lb	at,0(zero)
    3088:	fffffffc 	0xfffffffc
	...
    3094:	00000018 	mult	zero,zero
    3098:	0000001d 	0x1d
    309c:	0000001f 	0x1f
    30a0:	800ca1cd 	lb	t4,-24115(zero)
    30a4:	80030000 	lb	v1,0(zero)
    30a8:	fffffffc 	0xfffffffc
	...
    30b4:	00000020 	add	zero,zero,zero
    30b8:	0000001d 	0x1d
    30bc:	0000001f 	0x1f
    30c0:	800ca205 	lb	t4,-24059(zero)
    30c4:	80030000 	lb	v1,0(zero)
    30c8:	fffffffc 	0xfffffffc
	...
    30d4:	00000028 	0x28
    30d8:	0000001d 	0x1d
    30dc:	0000001f 	0x1f
    30e0:	800ca27d 	lb	t4,-23939(zero)
    30e4:	80000000 	lb	zero,0(zero)
    30e8:	fffffffc 	0xfffffffc
	...
    30f4:	00000018 	mult	zero,zero
    30f8:	0000001d 	0x1d
    30fc:	0000001f 	0x1f
    3100:	800ca291 	lb	t4,-23919(zero)
    3104:	80000000 	lb	zero,0(zero)
    3108:	fffffffc 	0xfffffffc
	...
    3114:	00000018 	mult	zero,zero
    3118:	0000001d 	0x1d
    311c:	0000001f 	0x1f
    3120:	800ca2a5 	lb	t4,-23899(zero)
    3124:	80030000 	lb	v1,0(zero)
    3128:	fffffffc 	0xfffffffc
	...
    3134:	00000030 	0x30
    3138:	0000001d 	0x1d
    313c:	0000001f 	0x1f
    3140:	800ca321 	lb	t4,-23775(zero)
    3144:	80000000 	lb	zero,0(zero)
    3148:	fffffffc 	0xfffffffc
	...
    3154:	00000018 	mult	zero,zero
    3158:	0000001d 	0x1d
    315c:	0000001f 	0x1f
    3160:	800ca335 	lb	t4,-23755(zero)
    3164:	80030000 	lb	v1,0(zero)
    3168:	fffffffc 	0xfffffffc
	...
    3174:	00000028 	0x28
    3178:	0000001d 	0x1d
    317c:	0000001f 	0x1f
    3180:	800ca3e1 	lb	t4,-23583(zero)
    3184:	80030000 	lb	v1,0(zero)
    3188:	fffffffc 	0xfffffffc
	...
    3194:	00000028 	0x28
    3198:	0000001d 	0x1d
    319c:	0000001f 	0x1f
    31a0:	800ca471 	lb	t4,-23439(zero)
    31a4:	80030000 	lb	v1,0(zero)
    31a8:	fffffffc 	0xfffffffc
	...
    31b4:	00000030 	0x30
    31b8:	0000001d 	0x1d
    31bc:	0000001f 	0x1f
    31c0:	800ca509 	lb	t4,-23287(zero)
    31c4:	80000000 	lb	zero,0(zero)
    31c8:	fffffffc 	0xfffffffc
	...
    31d4:	00000018 	mult	zero,zero
    31d8:	0000001d 	0x1d
    31dc:	0000001f 	0x1f
    31e0:	800ca51d 	lb	t4,-23267(zero)
    31e4:	80010000 	lb	at,0(zero)
    31e8:	fffffffc 	0xfffffffc
	...
    31f4:	00000020 	add	zero,zero,zero
    31f8:	0000001d 	0x1d
    31fc:	0000001f 	0x1f
    3200:	800ca53d 	lb	t4,-23235(zero)
    3204:	80010000 	lb	at,0(zero)
    3208:	fffffffc 	0xfffffffc
	...
    3214:	00000020 	add	zero,zero,zero
    3218:	0000001d 	0x1d
    321c:	0000001f 	0x1f
    3220:	800ca585 	lb	t4,-23163(zero)
    3224:	80010000 	lb	at,0(zero)
    3228:	fffffffc 	0xfffffffc
	...
    3234:	00000020 	add	zero,zero,zero
    3238:	0000001d 	0x1d
    323c:	0000001f 	0x1f
    3240:	800ca5c1 	lb	t4,-23103(zero)
    3244:	80000000 	lb	zero,0(zero)
    3248:	fffffffc 	0xfffffffc
	...
    3254:	00000018 	mult	zero,zero
    3258:	0000001d 	0x1d
    325c:	0000001f 	0x1f
    3260:	800ca5d5 	lb	t4,-23083(zero)
    3264:	80030000 	lb	v1,0(zero)
    3268:	fffffffc 	0xfffffffc
	...
    3274:	00000028 	0x28
    3278:	0000001d 	0x1d
    327c:	0000001f 	0x1f
    3280:	800ca62d 	lb	t4,-22995(zero)
    3284:	80010000 	lb	at,0(zero)
    3288:	fffffffc 	0xfffffffc
	...
    3294:	00000020 	add	zero,zero,zero
    3298:	0000001d 	0x1d
    329c:	0000001f 	0x1f
    32a0:	800ca671 	lb	t4,-22927(zero)
    32a4:	80000000 	lb	zero,0(zero)
    32a8:	fffffffc 	0xfffffffc
	...
    32b4:	00000018 	mult	zero,zero
    32b8:	0000001d 	0x1d
    32bc:	0000001f 	0x1f
    32c0:	800ca685 	lb	t4,-22907(zero)
    32c4:	80000000 	lb	zero,0(zero)
    32c8:	fffffffc 	0xfffffffc
	...
    32d4:	00000018 	mult	zero,zero
    32d8:	0000001d 	0x1d
    32dc:	0000001f 	0x1f
    32e0:	800ca699 	lb	t4,-22887(zero)
    32e4:	80030000 	lb	v1,0(zero)
    32e8:	fffffffc 	0xfffffffc
	...
    32f4:	00000028 	0x28
    32f8:	0000001d 	0x1d
    32fc:	0000001f 	0x1f
    3300:	800ca6e9 	lb	t4,-22807(zero)
    3304:	80030000 	lb	v1,0(zero)
    3308:	fffffffc 	0xfffffffc
	...
    3314:	00000028 	0x28
    3318:	0000001d 	0x1d
    331c:	0000001f 	0x1f
    3320:	800ca73d 	lb	t4,-22723(zero)
    3324:	80030000 	lb	v1,0(zero)
    3328:	fffffffc 	0xfffffffc
	...
    3334:	00000028 	0x28
    3338:	0000001d 	0x1d
    333c:	0000001f 	0x1f
    3340:	800ca7a5 	lb	t4,-22619(zero)
    3344:	80030000 	lb	v1,0(zero)
    3348:	fffffffc 	0xfffffffc
	...
    3354:	00000030 	0x30
    3358:	0000001d 	0x1d
    335c:	0000001f 	0x1f
    3360:	800ca811 	lb	t4,-22511(zero)
    3364:	80010000 	lb	at,0(zero)
    3368:	fffffffc 	0xfffffffc
	...
    3374:	00000020 	add	zero,zero,zero
    3378:	0000001d 	0x1d
    337c:	0000001f 	0x1f
    3380:	800ca849 	lb	t4,-22455(zero)
    3384:	80000000 	lb	zero,0(zero)
    3388:	fffffffc 	0xfffffffc
	...
    3394:	00000018 	mult	zero,zero
    3398:	0000001d 	0x1d
    339c:	0000001f 	0x1f
    33a0:	800ca85d 	lb	t4,-22435(zero)
    33a4:	80010000 	lb	at,0(zero)
    33a8:	fffffffc 	0xfffffffc
	...
    33b4:	00000020 	add	zero,zero,zero
    33b8:	0000001d 	0x1d
    33bc:	0000001f 	0x1f
    33c0:	800ca8a5 	lb	t4,-22363(zero)
    33c4:	80030000 	lb	v1,0(zero)
    33c8:	fffffffc 	0xfffffffc
	...
    33d4:	00000028 	0x28
    33d8:	0000001d 	0x1d
    33dc:	0000001f 	0x1f
    33e0:	800ca901 	lb	t4,-22271(zero)
    33e4:	80000000 	lb	zero,0(zero)
    33e8:	fffffffc 	0xfffffffc
	...
    33f4:	00000018 	mult	zero,zero
    33f8:	0000001d 	0x1d
    33fc:	0000001f 	0x1f
    3400:	800ca915 	lb	t4,-22251(zero)
    3404:	80030000 	lb	v1,0(zero)
    3408:	fffffffc 	0xfffffffc
	...
    3414:	00000030 	0x30
    3418:	0000001d 	0x1d
    341c:	0000001f 	0x1f
    3420:	800ca995 	lb	t4,-22123(zero)
    3424:	80030000 	lb	v1,0(zero)
    3428:	fffffffc 	0xfffffffc
	...
    3434:	00000040 	ssnop
    3438:	0000001d 	0x1d
    343c:	0000001f 	0x1f
    3440:	800cabd9 	lb	t4,-21543(zero)
    3444:	80030000 	lb	v1,0(zero)
    3448:	fffffffc 	0xfffffffc
	...
    3454:	00000030 	0x30
    3458:	0000001d 	0x1d
    345c:	0000001f 	0x1f
    3460:	800cacf9 	lb	t4,-21255(zero)
    3464:	80000000 	lb	zero,0(zero)
    3468:	fffffffc 	0xfffffffc
	...
    3474:	00000018 	mult	zero,zero
    3478:	0000001d 	0x1d
    347c:	0000001f 	0x1f
    3480:	800cad3d 	lb	t4,-21187(zero)
    3484:	80000000 	lb	zero,0(zero)
    3488:	fffffffc 	0xfffffffc
	...
    3494:	00000018 	mult	zero,zero
    3498:	0000001d 	0x1d
    349c:	0000001f 	0x1f
    34a0:	800cad71 	lb	t4,-21135(zero)
    34a4:	80000000 	lb	zero,0(zero)
    34a8:	fffffffc 	0xfffffffc
	...
    34b4:	00000020 	add	zero,zero,zero
    34b8:	0000001d 	0x1d
    34bc:	0000001f 	0x1f
    34c0:	800cadb5 	lb	t4,-21067(zero)
    34c4:	80000000 	lb	zero,0(zero)
    34c8:	fffffffc 	0xfffffffc
	...
    34d4:	00000018 	mult	zero,zero
    34d8:	0000001d 	0x1d
    34dc:	0000001f 	0x1f
    34e0:	800caded 	lb	t4,-21011(zero)
    34e4:	80000000 	lb	zero,0(zero)
    34e8:	fffffffc 	0xfffffffc
	...
    34f4:	00000018 	mult	zero,zero
    34f8:	0000001d 	0x1d
    34fc:	0000001f 	0x1f
    3500:	800cae25 	lb	t4,-20955(zero)
    3504:	80030000 	lb	v1,0(zero)
    3508:	fffffffc 	0xfffffffc
	...
    3514:	00000020 	add	zero,zero,zero
    3518:	0000001d 	0x1d
    351c:	0000001f 	0x1f
    3520:	800cae99 	lb	t4,-20839(zero)
    3524:	80030000 	lb	v1,0(zero)
    3528:	fffffffc 	0xfffffffc
	...
    3534:	00000028 	0x28
    3538:	0000001d 	0x1d
    353c:	0000001f 	0x1f
    3540:	800caf09 	lb	t4,-20727(zero)
    3544:	80030000 	lb	v1,0(zero)
    3548:	fffffffc 	0xfffffffc
	...
    3554:	00000028 	0x28
    3558:	0000001d 	0x1d
    355c:	0000001f 	0x1f
    3560:	800caf69 	lb	t4,-20631(zero)
    3564:	80030000 	lb	v1,0(zero)
    3568:	fffffffc 	0xfffffffc
	...
    3574:	00000030 	0x30
    3578:	0000001d 	0x1d
    357c:	0000001f 	0x1f
    3580:	800cafbd 	lb	t4,-20547(zero)
    3584:	80000000 	lb	zero,0(zero)
    3588:	fffffffc 	0xfffffffc
	...
    3594:	00000020 	add	zero,zero,zero
    3598:	0000001d 	0x1d
    359c:	0000001f 	0x1f
    35a0:	800cafe1 	lb	t4,-20511(zero)
    35a4:	80010000 	lb	at,0(zero)
    35a8:	fffffffc 	0xfffffffc
	...
    35b4:	00000020 	add	zero,zero,zero
    35b8:	0000001d 	0x1d
    35bc:	0000001f 	0x1f
    35c0:	800cb019 	lb	t4,-20455(zero)
    35c4:	80030000 	lb	v1,0(zero)
    35c8:	fffffffc 	0xfffffffc
	...
    35d4:	00000028 	0x28
    35d8:	0000001d 	0x1d
    35dc:	0000001f 	0x1f
    35e0:	800cb085 	lb	t4,-20347(zero)
    35e4:	80030000 	lb	v1,0(zero)
    35e8:	fffffffc 	0xfffffffc
	...
    35f4:	00000020 	add	zero,zero,zero
    35f8:	0000001d 	0x1d
    35fc:	0000001f 	0x1f
    3600:	800cb0c5 	lb	t4,-20283(zero)
    3604:	80030000 	lb	v1,0(zero)
    3608:	fffffffc 	0xfffffffc
	...
    3614:	00000028 	0x28
    3618:	0000001d 	0x1d
    361c:	0000001f 	0x1f
    3620:	800cb12d 	lb	t4,-20179(zero)
    3624:	80000000 	lb	zero,0(zero)
    3628:	fffffffc 	0xfffffffc
	...
    3634:	00000018 	mult	zero,zero
    3638:	0000001d 	0x1d
    363c:	0000001f 	0x1f
    3640:	800cb141 	lb	t4,-20159(zero)
    3644:	80030000 	lb	v1,0(zero)
    3648:	fffffffc 	0xfffffffc
	...
    3654:	00000030 	0x30
    3658:	0000001d 	0x1d
    365c:	0000001f 	0x1f
    3660:	800cb209 	lb	t4,-19959(zero)
    3664:	80030000 	lb	v1,0(zero)
    3668:	fffffffc 	0xfffffffc
	...
    3674:	00000030 	0x30
    3678:	0000001d 	0x1d
    367c:	0000001f 	0x1f
    3680:	800cb2dd 	lb	t4,-19747(zero)
	...
    3698:	0000001d 	0x1d
    369c:	0000001f 	0x1f
    36a0:	800cb309 	lb	t4,-19703(zero)
	...
    36b4:	00000008 	jr	zero
    36b8:	0000001d 	0x1d
    36bc:	0000001f 	0x1f
    36c0:	800cb409 	lb	t4,-19447(zero)
    36c4:	80000000 	lb	zero,0(zero)
    36c8:	fffffffc 	0xfffffffc
	...
    36d4:	00000018 	mult	zero,zero
    36d8:	0000001d 	0x1d
    36dc:	0000001f 	0x1f
    36e0:	800cb419 	lb	t4,-19431(zero)
    36e4:	00010000 	sll	zero,at,0x0
    36e8:	fffffffc 	0xfffffffc
	...
    36f4:	00000008 	jr	zero
    36f8:	0000001d 	0x1d
    36fc:	0000001f 	0x1f
    3700:	800cb469 	lb	t4,-19351(zero)
    3704:	80000000 	lb	zero,0(zero)
    3708:	fffffffc 	0xfffffffc
	...
    3714:	00000018 	mult	zero,zero
    3718:	0000001d 	0x1d
    371c:	0000001f 	0x1f
    3720:	800cb4ad 	lb	t4,-19283(zero)
	...
    3738:	0000001d 	0x1d
    373c:	0000001f 	0x1f
    3740:	800cb4dd 	lb	t4,-19235(zero)
    3744:	80030000 	lb	v1,0(zero)
    3748:	fffffffc 	0xfffffffc
	...
    3754:	00000020 	add	zero,zero,zero
    3758:	0000001d 	0x1d
    375c:	0000001f 	0x1f
    3760:	800cb549 	lb	t4,-19127(zero)
    3764:	80000000 	lb	zero,0(zero)
    3768:	fffffffc 	0xfffffffc
	...
    3774:	00000018 	mult	zero,zero
    3778:	0000001d 	0x1d
    377c:	0000001f 	0x1f
    3780:	800cb5a9 	lb	t4,-19031(zero)
    3784:	80010000 	lb	at,0(zero)
    3788:	fffffffc 	0xfffffffc
	...
    3794:	00000018 	mult	zero,zero
    3798:	0000001d 	0x1d
    379c:	0000001f 	0x1f
    37a0:	800cb5fd 	lb	t4,-18947(zero)
    37a4:	80030000 	lb	v1,0(zero)
    37a8:	fffffffc 	0xfffffffc
	...
    37b4:	00000028 	0x28
    37b8:	0000001d 	0x1d
    37bc:	0000001f 	0x1f
    37c0:	800cb669 	lb	t4,-18839(zero)
    37c4:	80030000 	lb	v1,0(zero)
    37c8:	fffffffc 	0xfffffffc
	...
    37d4:	00000028 	0x28
    37d8:	0000001d 	0x1d
    37dc:	0000001f 	0x1f
    37e0:	800cb6bd 	lb	t4,-18755(zero)
    37e4:	80030000 	lb	v1,0(zero)
    37e8:	fffffffc 	0xfffffffc
	...
    37f4:	00000028 	0x28
    37f8:	0000001d 	0x1d
    37fc:	0000001f 	0x1f
    3800:	800cb70d 	lb	t4,-18675(zero)
    3804:	80030000 	lb	v1,0(zero)
    3808:	fffffffc 	0xfffffffc
	...
    3814:	00000028 	0x28
    3818:	0000001d 	0x1d
    381c:	0000001f 	0x1f
    3820:	800cb799 	lb	t4,-18535(zero)
    3824:	80030000 	lb	v1,0(zero)
    3828:	fffffffc 	0xfffffffc
	...
    3834:	00000038 	0x38
    3838:	0000001d 	0x1d
    383c:	0000001f 	0x1f
    3840:	800cb7fd 	lb	t4,-18435(zero)
    3844:	80030000 	lb	v1,0(zero)
    3848:	fffffffc 	0xfffffffc
	...
    3854:	00000028 	0x28
    3858:	0000001d 	0x1d
    385c:	0000001f 	0x1f
    3860:	800cb881 	lb	t4,-18303(zero)
    3864:	80030000 	lb	v1,0(zero)
    3868:	fffffffc 	0xfffffffc
	...
    3874:	00000038 	0x38
    3878:	0000001d 	0x1d
    387c:	0000001f 	0x1f
    3880:	800cb99d 	lb	t4,-18019(zero)
    3884:	80000000 	lb	zero,0(zero)
    3888:	fffffffc 	0xfffffffc
	...
    3894:	00000020 	add	zero,zero,zero
    3898:	0000001d 	0x1d
    389c:	0000001f 	0x1f
    38a0:	800cb9dd 	lb	t4,-17955(zero)
    38a4:	80030000 	lb	v1,0(zero)
    38a8:	fffffffc 	0xfffffffc
	...
    38b4:	00000030 	0x30
    38b8:	0000001d 	0x1d
    38bc:	0000001f 	0x1f
    38c0:	82030000 	lb	v1,0(s0)
    38c4:	80000000 	lb	zero,0(zero)
    38c8:	fffffffc 	0xfffffffc
	...
    38d4:	00000018 	mult	zero,zero
    38d8:	0000001d 	0x1d
    38dc:	0000001f 	0x1f
    38e0:	82030080 	lb	v1,128(s0)
    38e4:	80000000 	lb	zero,0(zero)
    38e8:	fffffffc 	0xfffffffc
	...
    38f4:	00000020 	add	zero,zero,zero
    38f8:	0000001d 	0x1d
    38fc:	0000001f 	0x1f
    3900:	820300e0 	lb	v1,224(s0)
    3904:	80010000 	lb	at,0(zero)
    3908:	fffffffc 	0xfffffffc
	...
    3914:	00000018 	mult	zero,zero
    3918:	0000001d 	0x1d
    391c:	0000001f 	0x1f
    3920:	800c0000 	lb	t4,0(zero)
    3924:	807f0000 	lb	ra,0(v1)
    3928:	fffffffc 	0xfffffffc
	...
    3934:	00000030 	0x30
    3938:	0000001d 	0x1d
    393c:	0000001f 	0x1f
    3940:	800c02a4 	lb	t4,676(zero)
    3944:	c0ff0000 	ll	ra,0(a3)
    3948:	fffffffc 	0xfffffffc
	...
    3954:	00000048 	0x48
    3958:	0000001d 	0x1d
    395c:	0000001f 	0x1f
    3960:	800c062c 	lb	t4,1580(zero)
    3964:	80000000 	lb	zero,0(zero)
    3968:	fffffffc 	0xfffffffc
	...
    3974:	00000018 	mult	zero,zero
    3978:	0000001d 	0x1d
    397c:	0000001f 	0x1f
    3980:	800c0664 	lb	t4,1636(zero)
    3984:	803f0000 	lb	ra,0(at)
    3988:	fffffffc 	0xfffffffc
	...
    3994:	00000038 	0x38
    3998:	0000001d 	0x1d
    399c:	0000001f 	0x1f
    39a0:	800c0718 	lb	t4,1816(zero)
    39a4:	80070000 	lb	a3,0(zero)
    39a8:	fffffffc 	0xfffffffc
	...
    39b4:	00000020 	add	zero,zero,zero
    39b8:	0000001d 	0x1d
    39bc:	0000001f 	0x1f
    39c0:	800c08dc 	lb	t4,2268(zero)
    39c4:	80070000 	lb	a3,0(zero)
    39c8:	fffffffc 	0xfffffffc
	...
    39d4:	00000020 	add	zero,zero,zero
    39d8:	0000001d 	0x1d
    39dc:	0000001f 	0x1f
    39e0:	800c094c 	lb	t4,2380(zero)
    39e4:	80030000 	lb	v1,0(zero)
    39e8:	fffffffc 	0xfffffffc
	...
    39f4:	00000020 	add	zero,zero,zero
    39f8:	0000001d 	0x1d
    39fc:	0000001f 	0x1f
    3a00:	800c09c8 	lb	t4,2504(zero)
    3a04:	80ff0000 	lb	ra,0(a3)
    3a08:	fffffffc 	0xfffffffc
	...
    3a14:	00000038 	0x38
    3a18:	0000001d 	0x1d
    3a1c:	0000001f 	0x1f
    3a20:	800c0b20 	lb	t4,2848(zero)
	...
    3a38:	0000001d 	0x1d
    3a3c:	0000001f 	0x1f
    3a40:	800c0b50 	lb	t4,2896(zero)
    3a44:	80030000 	lb	v1,0(zero)
    3a48:	fffffffc 	0xfffffffc
	...
    3a54:	00000020 	add	zero,zero,zero
    3a58:	0000001d 	0x1d
    3a5c:	0000001f 	0x1f
    3a60:	800c0ba4 	lb	t4,2980(zero)
    3a64:	800f0000 	lb	t7,0(zero)
    3a68:	fffffffc 	0xfffffffc
	...
    3a74:	00000030 	0x30
    3a78:	0000001d 	0x1d
    3a7c:	0000001f 	0x1f
    3a80:	800c0fec 	lb	t4,4076(zero)
    3a84:	80030000 	lb	v1,0(zero)
    3a88:	fffffffc 	0xfffffffc
	...
    3a94:	00000020 	add	zero,zero,zero
    3a98:	0000001d 	0x1d
    3a9c:	0000001f 	0x1f
    3aa0:	800c106c 	lb	t4,4204(zero)
    3aa4:	c0ff0000 	ll	ra,0(a3)
    3aa8:	fffffffc 	0xfffffffc
	...
    3ab4:	00000038 	0x38
    3ab8:	0000001d 	0x1d
    3abc:	0000001f 	0x1f
    3ac0:	800c1294 	lb	t4,4756(zero)
    3ac4:	80010000 	lb	at,0(zero)
    3ac8:	fffffffc 	0xfffffffc
	...
    3ad4:	00000018 	mult	zero,zero
    3ad8:	0000001d 	0x1d
    3adc:	0000001f 	0x1f
    3ae0:	800c1368 	lb	t4,4968(zero)
	...
    3af8:	0000001d 	0x1d
    3afc:	0000001f 	0x1f
    3b00:	800c1370 	lb	t4,4976(zero)
    3b04:	80010000 	lb	at,0(zero)
    3b08:	fffffffc 	0xfffffffc
	...
    3b14:	00000018 	mult	zero,zero
    3b18:	0000001d 	0x1d
    3b1c:	0000001f 	0x1f
    3b20:	800c141c 	lb	t4,5148(zero)
	...
    3b38:	0000001d 	0x1d
    3b3c:	0000001f 	0x1f
    3b40:	800c1424 	lb	t4,5156(zero)
    3b44:	80030000 	lb	v1,0(zero)
    3b48:	fffffffc 	0xfffffffc
	...
    3b54:	00000028 	0x28
    3b58:	0000001d 	0x1d
    3b5c:	0000001f 	0x1f
    3b60:	800c1670 	lb	t4,5744(zero)
    3b64:	80000000 	lb	zero,0(zero)
    3b68:	fffffffc 	0xfffffffc
	...
    3b74:	00000018 	mult	zero,zero
    3b78:	0000001d 	0x1d
    3b7c:	0000001f 	0x1f
    3b80:	800c16cc 	lb	t4,5836(zero)
    3b84:	803f0000 	lb	ra,0(at)
    3b88:	fffffffc 	0xfffffffc
	...
    3b94:	00000030 	0x30
    3b98:	0000001d 	0x1d
    3b9c:	0000001f 	0x1f
    3ba0:	800c17e8 	lb	t4,6120(zero)
    3ba4:	c0ff0000 	ll	ra,0(a3)
    3ba8:	fffffffc 	0xfffffffc
	...
    3bb4:	00000040 	ssnop
    3bb8:	0000001d 	0x1d
    3bbc:	0000001f 	0x1f
    3bc0:	8204f018 	lb	a0,-4072(s0)
    3bc4:	80070000 	lb	a3,0(zero)
    3bc8:	fffffffc 	0xfffffffc
	...
    3bd4:	00000020 	add	zero,zero,zero
    3bd8:	0000001d 	0x1d
    3bdc:	0000001f 	0x1f
    3be0:	8204f0e0 	lb	a0,-3872(s0)
    3be4:	c0ff0000 	ll	ra,0(a3)
    3be8:	fffffffc 	0xfffffffc
	...
    3bf4:	00000070 	0x70
    3bf8:	0000001d 	0x1d
    3bfc:	0000001f 	0x1f
    3c00:	8204f8f0 	lb	a0,-1808(s0)
	...
    3c18:	0000001d 	0x1d
    3c1c:	0000001f 	0x1f
    3c20:	8204f958 	lb	a0,-1704(s0)
    3c24:	80030000 	lb	v1,0(zero)
    3c28:	fffffffc 	0xfffffffc
	...
    3c34:	00000020 	add	zero,zero,zero
    3c38:	0000001d 	0x1d
    3c3c:	0000001f 	0x1f
    3c40:	8204f9e0 	lb	a0,-1568(s0)
    3c44:	c0ff0000 	ll	ra,0(a3)
    3c48:	fffffffc 	0xfffffffc
	...
    3c54:	00000040 	ssnop
    3c58:	0000001d 	0x1d
    3c5c:	0000001f 	0x1f
    3c60:	800c8e74 	lb	t4,-29068(zero)
    3c64:	80030000 	lb	v1,0(zero)
    3c68:	fffffffc 	0xfffffffc
	...
    3c74:	00000020 	add	zero,zero,zero
    3c78:	0000001d 	0x1d
    3c7c:	0000001f 	0x1f
    3c80:	8204ff14 	lb	a0,-236(s0)
    3c84:	c0ff0000 	ll	ra,0(a3)
    3c88:	fffffffc 	0xfffffffc
	...
    3c94:	00000060 	0x60
    3c98:	0000001d 	0x1d
    3c9c:	0000001f 	0x1f
    3ca0:	82050398 	lb	a1,920(s0)
    3ca4:	c0ff0000 	ll	ra,0(a3)
    3ca8:	fffffffc 	0xfffffffc
	...
    3cb4:	00000048 	0x48
    3cb8:	0000001d 	0x1d
    3cbc:	0000001f 	0x1f
    3cc0:	82050518 	lb	a1,1304(s0)
    3cc4:	c0ff0000 	ll	ra,0(a3)
    3cc8:	fffffffc 	0xfffffffc
	...
    3cd4:	00000048 	0x48
    3cd8:	0000001d 	0x1d
    3cdc:	0000001f 	0x1f
    3ce0:	82050698 	lb	a1,1688(s0)
    3ce4:	c0ff0000 	ll	ra,0(a3)
    3ce8:	fffffffc 	0xfffffffc
	...
    3cf4:	00000048 	0x48
    3cf8:	0000001d 	0x1d
    3cfc:	0000001f 	0x1f
    3d00:	82050818 	lb	a1,2072(s0)
    3d04:	803f0000 	lb	ra,0(at)
    3d08:	fffffffc 	0xfffffffc
	...
    3d14:	00000040 	ssnop
    3d18:	0000001d 	0x1d
    3d1c:	0000001f 	0x1f
    3d20:	82050938 	lb	a1,2360(s0)
    3d24:	c0ff0000 	ll	ra,0(a3)
    3d28:	fffffffc 	0xfffffffc
	...
    3d34:	00000048 	0x48
    3d38:	0000001d 	0x1d
    3d3c:	0000001f 	0x1f
    3d40:	82050b48 	lb	a1,2888(s0)
    3d44:	c0ff0000 	ll	ra,0(a3)
    3d48:	fffffffc 	0xfffffffc
	...
    3d54:	00000048 	0x48
    3d58:	0000001d 	0x1d
    3d5c:	0000001f 	0x1f
    3d60:	82050d44 	lb	a1,3396(s0)
    3d64:	c0ff0000 	ll	ra,0(a3)
    3d68:	fffffffc 	0xfffffffc
	...
    3d74:	00000040 	ssnop
    3d78:	0000001d 	0x1d
    3d7c:	0000001f 	0x1f
    3d80:	82050f04 	lb	a1,3844(s0)
    3d84:	c0ff0000 	ll	ra,0(a3)
    3d88:	fffffffc 	0xfffffffc
	...
    3d94:	00000048 	0x48
    3d98:	0000001d 	0x1d
    3d9c:	0000001f 	0x1f
    3da0:	820510d8 	lb	a1,4312(s0)
    3da4:	c0ff0000 	ll	ra,0(a3)
    3da8:	fffffffc 	0xfffffffc
	...
    3db4:	00000048 	0x48
    3db8:	0000001d 	0x1d
    3dbc:	0000001f 	0x1f
    3dc0:	820512a8 	lb	a1,4776(s0)
    3dc4:	c0ff0000 	ll	ra,0(a3)
    3dc8:	fffffffc 	0xfffffffc
	...
    3dd4:	00000048 	0x48
    3dd8:	0000001d 	0x1d
    3ddc:	0000001f 	0x1f
    3de0:	8205144c 	lb	a1,5196(s0)
    3de4:	c0ff0000 	ll	ra,0(a3)
    3de8:	fffffffc 	0xfffffffc
	...
    3df4:	00000060 	0x60
    3df8:	0000001d 	0x1d
    3dfc:	0000001f 	0x1f
    3e00:	8205161c 	lb	a1,5660(s0)
    3e04:	c0ff0000 	ll	ra,0(a3)
    3e08:	fffffffc 	0xfffffffc
	...
    3e14:	00000038 	0x38
    3e18:	0000001d 	0x1d
    3e1c:	0000001f 	0x1f
    3e20:	820517e8 	lb	a1,6120(s0)
    3e24:	c0ff0000 	ll	ra,0(a3)
    3e28:	fffffffc 	0xfffffffc
	...
    3e34:	00000078 	0x78
    3e38:	0000001d 	0x1d
    3e3c:	0000001f 	0x1f
    3e40:	82051b18 	lb	a1,6936(s0)
    3e44:	80070000 	lb	a3,0(zero)
    3e48:	fffffffc 	0xfffffffc
	...
    3e54:	00000028 	0x28
    3e58:	0000001d 	0x1d
    3e5c:	0000001f 	0x1f
    3e60:	82051c14 	lb	a1,7188(s0)
    3e64:	c0ff0000 	ll	ra,0(a3)
    3e68:	fffffffc 	0xfffffffc
	...
    3e74:	00000058 	0x58
    3e78:	0000001d 	0x1d
    3e7c:	0000001f 	0x1f
    3e80:	82051f30 	lb	a1,7984(s0)
	...
    3e98:	0000001d 	0x1d
    3e9c:	0000001f 	0x1f
    3ea0:	82051f40 	lb	a1,8000(s0)
    3ea4:	801f0000 	lb	ra,0(zero)
    3ea8:	fffffffc 	0xfffffffc
	...
    3eb4:	00000040 	ssnop
    3eb8:	0000001d 	0x1d
    3ebc:	0000001f 	0x1f
    3ec0:	820520c8 	lb	a1,8392(s0)
    3ec4:	c0ff0000 	ll	ra,0(a3)
    3ec8:	fffffffc 	0xfffffffc
	...
    3ed4:	00000038 	0x38
    3ed8:	0000001d 	0x1d
    3edc:	0000001f 	0x1f
    3ee0:	8205225c 	lb	a1,8796(s0)
    3ee4:	c0ff0000 	ll	ra,0(a3)
    3ee8:	fffffffc 	0xfffffffc
	...
    3ef4:	00000068 	0x68
    3ef8:	0000001d 	0x1d
    3efc:	0000001f 	0x1f
    3f00:	82052c38 	lb	a1,11320(s0)
    3f04:	800f0000 	lb	t7,0(zero)
    3f08:	fffffffc 	0xfffffffc
	...
    3f14:	00000028 	0x28
    3f18:	0000001d 	0x1d
    3f1c:	0000001f 	0x1f
    3f20:	82052e0c 	lb	a1,11788(s0)
    3f24:	807f0000 	lb	ra,0(v1)
    3f28:	fffffffc 	0xfffffffc
	...
    3f34:	00000038 	0x38
    3f38:	0000001d 	0x1d
    3f3c:	0000001f 	0x1f
    3f40:	82053324 	lb	a1,13092(s0)
    3f44:	801f0000 	lb	ra,0(zero)
    3f48:	fffffffc 	0xfffffffc
	...
    3f54:	00000050 	0x50
    3f58:	0000001d 	0x1d
    3f5c:	0000001f 	0x1f
    3f60:	82053524 	lb	a1,13604(s0)
    3f64:	801f0000 	lb	ra,0(zero)
    3f68:	fffffffc 	0xfffffffc
	...
    3f74:	00000028 	0x28
    3f78:	0000001d 	0x1d
    3f7c:	0000001f 	0x1f
    3f80:	82053618 	lb	a1,13848(s0)
    3f84:	807f0000 	lb	ra,0(v1)
    3f88:	fffffffc 	0xfffffffc
	...
    3f94:	00000030 	0x30
    3f98:	0000001d 	0x1d
    3f9c:	0000001f 	0x1f
    3fa0:	82053704 	lb	a1,14084(s0)
    3fa4:	803f0000 	lb	ra,0(at)
    3fa8:	fffffffc 	0xfffffffc
	...
    3fb4:	00000030 	0x30
    3fb8:	0000001d 	0x1d
    3fbc:	0000001f 	0x1f
    3fc0:	820537e8 	lb	a1,14312(s0)
    3fc4:	803f0000 	lb	ra,0(at)
    3fc8:	fffffffc 	0xfffffffc
	...
    3fd4:	00000030 	0x30
    3fd8:	0000001d 	0x1d
    3fdc:	0000001f 	0x1f
    3fe0:	820539c0 	lb	a1,14784(s0)
    3fe4:	803f0000 	lb	ra,0(at)
    3fe8:	fffffffc 	0xfffffffc
	...
    3ff4:	00000030 	0x30
    3ff8:	0000001d 	0x1d
    3ffc:	0000001f 	0x1f
    4000:	82053ac0 	lb	a1,15040(s0)
    4004:	807f0000 	lb	ra,0(v1)
    4008:	fffffffc 	0xfffffffc
	...
    4014:	00000030 	0x30
    4018:	0000001d 	0x1d
    401c:	0000001f 	0x1f
    4020:	82053ca4 	lb	a1,15524(s0)
    4024:	c0ff0000 	ll	ra,0(a3)
    4028:	fffffffc 	0xfffffffc
	...
    4034:	000000a8 	0xa8
    4038:	0000001d 	0x1d
    403c:	0000001f 	0x1f
    4040:	82054120 	lb	a1,16672(s0)
    4044:	c0ff0000 	ll	ra,0(a3)
    4048:	fffffffc 	0xfffffffc
	...
    4054:	00000048 	0x48
    4058:	0000001d 	0x1d
    405c:	0000001f 	0x1f
    4060:	82054808 	lb	a1,18440(s0)
    4064:	c0ff0000 	ll	ra,0(a3)
    4068:	fffffffc 	0xfffffffc
	...
    4074:	00000050 	0x50
    4078:	0000001d 	0x1d
    407c:	0000001f 	0x1f
    4080:	82054de0 	lb	a1,19936(s0)
    4084:	801f0000 	lb	ra,0(zero)
    4088:	fffffffc 	0xfffffffc
	...
    4094:	00000030 	0x30
    4098:	0000001d 	0x1d
    409c:	0000001f 	0x1f
    40a0:	82055044 	lb	a1,20548(s0)
    40a4:	c0ff0000 	ll	ra,0(a3)
    40a8:	fffffffc 	0xfffffffc
	...
    40b4:	00000048 	0x48
    40b8:	0000001d 	0x1d
    40bc:	0000001f 	0x1f
    40c0:	82055580 	lb	a1,21888(s0)
    40c4:	801f0000 	lb	ra,0(zero)
    40c8:	fffffffc 	0xfffffffc
	...
    40d4:	00000028 	0x28
    40d8:	0000001d 	0x1d
    40dc:	0000001f 	0x1f
    40e0:	8205565c 	lb	a1,22108(s0)
    40e4:	801f0000 	lb	ra,0(zero)
    40e8:	fffffffc 	0xfffffffc
	...
    40f4:	00000030 	0x30
    40f8:	0000001d 	0x1d
    40fc:	0000001f 	0x1f
    4100:	820557fc 	lb	a1,22524(s0)
    4104:	80070000 	lb	a3,0(zero)
    4108:	fffffffc 	0xfffffffc
	...
    4114:	00000020 	add	zero,zero,zero
    4118:	0000001d 	0x1d
    411c:	0000001f 	0x1f
    4120:	820558e0 	lb	a1,22752(s0)
    4124:	80030000 	lb	v1,0(zero)
    4128:	fffffffc 	0xfffffffc
	...
    4134:	00000020 	add	zero,zero,zero
    4138:	0000001d 	0x1d
    413c:	0000001f 	0x1f
    4140:	820559b4 	lb	a1,22964(s0)
    4144:	800f0000 	lb	t7,0(zero)
    4148:	fffffffc 	0xfffffffc
	...
    4154:	00000028 	0x28
    4158:	0000001d 	0x1d
    415c:	0000001f 	0x1f
    4160:	82055bf8 	lb	a1,23544(s0)
    4164:	807f0000 	lb	ra,0(v1)
    4168:	fffffffc 	0xfffffffc
	...
    4174:	00000030 	0x30
    4178:	0000001d 	0x1d
    417c:	0000001f 	0x1f
    4180:	82055e1c 	lb	a1,24092(s0)
	...
    4198:	0000001d 	0x1d
    419c:	0000001f 	0x1f
    41a0:	82055e24 	lb	a1,24100(s0)
    41a4:	80010000 	lb	at,0(zero)
    41a8:	fffffffc 	0xfffffffc
	...
    41b4:	00000018 	mult	zero,zero
    41b8:	0000001d 	0x1d
    41bc:	0000001f 	0x1f
    41c0:	82055e70 	lb	a1,24176(s0)
    41c4:	80010000 	lb	at,0(zero)
    41c8:	fffffffc 	0xfffffffc
	...
    41d4:	00000018 	mult	zero,zero
    41d8:	0000001d 	0x1d
    41dc:	0000001f 	0x1f
    41e0:	82055eac 	lb	a1,24236(s0)
    41e4:	80030000 	lb	v1,0(zero)
    41e8:	fffffffc 	0xfffffffc
	...
    41f4:	00000020 	add	zero,zero,zero
    41f8:	0000001d 	0x1d
    41fc:	0000001f 	0x1f
    4200:	82055f10 	lb	a1,24336(s0)
    4204:	800f0000 	lb	t7,0(zero)
    4208:	fffffffc 	0xfffffffc
	...
    4214:	00000028 	0x28
    4218:	0000001d 	0x1d
    421c:	0000001f 	0x1f
    4220:	82056214 	lb	a1,25108(s0)
    4224:	80070000 	lb	a3,0(zero)
    4228:	fffffffc 	0xfffffffc
	...
    4234:	00000020 	add	zero,zero,zero
    4238:	0000001d 	0x1d
    423c:	0000001f 	0x1f
    4240:	8205634c 	lb	a1,25420(s0)
    4244:	800f0000 	lb	t7,0(zero)
    4248:	fffffffc 	0xfffffffc
	...
    4254:	00000030 	0x30
    4258:	0000001d 	0x1d
    425c:	0000001f 	0x1f
    4260:	82056568 	lb	a1,25960(s0)
    4264:	807f0000 	lb	ra,0(v1)
    4268:	fffffffc 	0xfffffffc
	...
    4274:	00000060 	0x60
    4278:	0000001d 	0x1d
    427c:	0000001f 	0x1f
    4280:	82056b80 	lb	a1,27520(s0)
    4284:	80070000 	lb	a3,0(zero)
    4288:	fffffffc 	0xfffffffc
	...
    4294:	00000020 	add	zero,zero,zero
    4298:	0000001d 	0x1d
    429c:	0000001f 	0x1f
    42a0:	82056c1c 	lb	a1,27676(s0)
	...
    42b8:	0000001d 	0x1d
    42bc:	0000001f 	0x1f
    42c0:	82056c40 	lb	a1,27712(s0)
    42c4:	801f0000 	lb	ra,0(zero)
    42c8:	fffffffc 	0xfffffffc
	...
    42d4:	00000028 	0x28
    42d8:	0000001d 	0x1d
    42dc:	0000001f 	0x1f
    42e0:	82056d70 	lb	a1,28016(s0)
    42e4:	803f0000 	lb	ra,0(at)
    42e8:	fffffffc 	0xfffffffc
	...
    42f4:	00000030 	0x30
    42f8:	0000001d 	0x1d
    42fc:	0000001f 	0x1f
    4300:	82056f5c 	lb	a1,28508(s0)
    4304:	800f0000 	lb	t7,0(zero)
    4308:	fffffffc 	0xfffffffc
	...
    4314:	00000028 	0x28
    4318:	0000001d 	0x1d
    431c:	0000001f 	0x1f
    4320:	820570c8 	lb	a1,28872(s0)
    4324:	80070000 	lb	a3,0(zero)
    4328:	fffffffc 	0xfffffffc
	...
    4334:	00000020 	add	zero,zero,zero
    4338:	0000001d 	0x1d
    433c:	0000001f 	0x1f
    4340:	820571bc 	lb	a1,29116(s0)
    4344:	80030000 	lb	v1,0(zero)
    4348:	fffffffc 	0xfffffffc
	...
    4354:	00000020 	add	zero,zero,zero
    4358:	0000001d 	0x1d
    435c:	0000001f 	0x1f
    4360:	82057228 	lb	a1,29224(s0)
    4364:	80030000 	lb	v1,0(zero)
    4368:	fffffffc 	0xfffffffc
	...
    4374:	00000020 	add	zero,zero,zero
    4378:	0000001d 	0x1d
    437c:	0000001f 	0x1f
    4380:	820572a8 	lb	a1,29352(s0)
    4384:	80030000 	lb	v1,0(zero)
    4388:	fffffffc 	0xfffffffc
	...
    4394:	00000020 	add	zero,zero,zero
    4398:	0000001d 	0x1d
    439c:	0000001f 	0x1f
    43a0:	82057338 	lb	a1,29496(s0)
    43a4:	803f0000 	lb	ra,0(at)
    43a8:	fffffffc 	0xfffffffc
	...
    43b4:	00000040 	ssnop
    43b8:	0000001d 	0x1d
    43bc:	0000001f 	0x1f
    43c0:	82057690 	lb	a1,30352(s0)
    43c4:	80030000 	lb	v1,0(zero)
    43c8:	fffffffc 	0xfffffffc
	...
    43d4:	00000020 	add	zero,zero,zero
    43d8:	0000001d 	0x1d
    43dc:	0000001f 	0x1f
    43e0:	820576e0 	lb	a1,30432(s0)
    43e4:	800f0000 	lb	t7,0(zero)
    43e8:	fffffffc 	0xfffffffc
	...
    43f4:	00000028 	0x28
    43f8:	0000001d 	0x1d
    43fc:	0000001f 	0x1f
    4400:	820577a0 	lb	a1,30624(s0)
	...
    4418:	0000001d 	0x1d
    441c:	0000001f 	0x1f
    4420:	820577e8 	lb	a1,30696(s0)
    4424:	80ff0000 	lb	ra,0(a3)
    4428:	fffffffc 	0xfffffffc
	...
    4434:	00000040 	ssnop
    4438:	0000001d 	0x1d
    443c:	0000001f 	0x1f
    4440:	82057bec 	lb	a1,31724(s0)
    4444:	801f0000 	lb	ra,0(zero)
    4448:	fffffffc 	0xfffffffc
	...
    4454:	00000030 	0x30
    4458:	0000001d 	0x1d
    445c:	0000001f 	0x1f
    4460:	82057e28 	lb	a1,32296(s0)
    4464:	80070000 	lb	a3,0(zero)
    4468:	fffffffc 	0xfffffffc
	...
    4474:	00000020 	add	zero,zero,zero
    4478:	0000001d 	0x1d
    447c:	0000001f 	0x1f
    4480:	82057fac 	lb	a1,32684(s0)
    4484:	800f0000 	lb	t7,0(zero)
    4488:	fffffffc 	0xfffffffc
	...
    4494:	00000028 	0x28
    4498:	0000001d 	0x1d
    449c:	0000001f 	0x1f
    44a0:	82058338 	lb	a1,-31944(s0)
    44a4:	c0ff0000 	ll	ra,0(a3)
    44a8:	fffffffc 	0xfffffffc
	...
    44b4:	00000048 	0x48
    44b8:	0000001d 	0x1d
    44bc:	0000001f 	0x1f
    44c0:	82058a80 	lb	a1,-30080(s0)
    44c4:	801f0000 	lb	ra,0(zero)
    44c8:	fffffffc 	0xfffffffc
	...
    44d4:	00000028 	0x28
    44d8:	0000001d 	0x1d
    44dc:	0000001f 	0x1f
    44e0:	82058b88 	lb	a1,-29816(s0)
    44e4:	80010000 	lb	at,0(zero)
    44e8:	fffffffc 	0xfffffffc
	...
    44f4:	00000018 	mult	zero,zero
    44f8:	0000001d 	0x1d
    44fc:	0000001f 	0x1f
    4500:	82058bf8 	lb	a1,-29704(s0)
    4504:	80010000 	lb	at,0(zero)
    4508:	fffffffc 	0xfffffffc
	...
    4514:	00000018 	mult	zero,zero
    4518:	0000001d 	0x1d
    451c:	0000001f 	0x1f
    4520:	82058ce0 	lb	a1,-29472(s0)
    4524:	80070000 	lb	a3,0(zero)
    4528:	fffffffc 	0xfffffffc
	...
    4534:	00000020 	add	zero,zero,zero
    4538:	0000001d 	0x1d
    453c:	0000001f 	0x1f
    4540:	82058dc4 	lb	a1,-29244(s0)
    4544:	80000000 	lb	zero,0(zero)
    4548:	fffffffc 	0xfffffffc
	...
    4554:	00000020 	add	zero,zero,zero
    4558:	0000001d 	0x1d
    455c:	0000001f 	0x1f
    4560:	82058e04 	lb	a1,-29180(s0)
    4564:	80000000 	lb	zero,0(zero)
    4568:	fffffffc 	0xfffffffc
	...
    4574:	00000018 	mult	zero,zero
    4578:	0000001d 	0x1d
    457c:	0000001f 	0x1f
    4580:	82058e30 	lb	a1,-29136(s0)
    4584:	800f0000 	lb	t7,0(zero)
    4588:	fffffffc 	0xfffffffc
	...
    4594:	00000028 	0x28
    4598:	0000001d 	0x1d
    459c:	0000001f 	0x1f
    45a0:	82058f34 	lb	a1,-28876(s0)
    45a4:	c0ff0000 	ll	ra,0(a3)
    45a8:	fffffffc 	0xfffffffc
	...
    45b4:	00000070 	0x70
    45b8:	0000001d 	0x1d
    45bc:	0000001f 	0x1f
    45c0:	82059190 	lb	a1,-28272(s0)
    45c4:	803f0000 	lb	ra,0(at)
    45c8:	fffffffc 	0xfffffffc
	...
    45d4:	00000040 	ssnop
    45d8:	0000001d 	0x1d
    45dc:	0000001f 	0x1f
    45e0:	8205926c 	lb	a1,-28052(s0)
    45e4:	c0ff0000 	ll	ra,0(a3)
    45e8:	fffffffc 	0xfffffffc
	...
    45f4:	00000078 	0x78
    45f8:	0000001d 	0x1d
    45fc:	0000001f 	0x1f
    4600:	820596f8 	lb	a1,-26888(s0)
    4604:	c0ff0000 	ll	ra,0(a3)
    4608:	fffffffc 	0xfffffffc
	...
    4614:	000000b8 	0xb8
    4618:	0000001d 	0x1d
    461c:	0000001f 	0x1f
    4620:	82059c5c 	lb	a1,-25508(s0)
    4624:	c0ff0000 	ll	ra,0(a3)
    4628:	fffffffc 	0xfffffffc
	...
    4634:	00000068 	0x68
    4638:	0000001d 	0x1d
    463c:	0000001f 	0x1f
    4640:	8205a060 	lb	a1,-24480(s0)
    4644:	c0ff0000 	ll	ra,0(a3)
    4648:	fffffffc 	0xfffffffc
	...
    4654:	00000068 	0x68
    4658:	0000001d 	0x1d
    465c:	0000001f 	0x1f
    4660:	8205a470 	lb	a1,-23440(s0)
    4664:	c0ff0000 	ll	ra,0(a3)
    4668:	fffffffc 	0xfffffffc
	...
    4674:	000000a0 	0xa0
    4678:	0000001d 	0x1d
    467c:	0000001f 	0x1f
    4680:	8205b3f0 	lb	a1,-19472(s0)
    4684:	c0ff0000 	ll	ra,0(a3)
    4688:	fffffffc 	0xfffffffc
	...
    4694:	00000058 	0x58
    4698:	0000001d 	0x1d
    469c:	0000001f 	0x1f
    46a0:	8205b8e0 	lb	a1,-18208(s0)
    46a4:	c0ff0000 	ll	ra,0(a3)
    46a8:	fffffffc 	0xfffffffc
	...
    46b4:	00000068 	0x68
    46b8:	0000001d 	0x1d
    46bc:	0000001f 	0x1f
    46c0:	8205bcec 	lb	a1,-17172(s0)
    46c4:	c0ff0000 	ll	ra,0(a3)
    46c8:	fffffffc 	0xfffffffc
	...
    46d4:	00000068 	0x68
    46d8:	0000001d 	0x1d
    46dc:	0000001f 	0x1f
    46e0:	8205bf64 	lb	a1,-16540(s0)
    46e4:	801f0000 	lb	ra,0(zero)
    46e8:	fffffffc 	0xfffffffc
	...
    46f4:	00000028 	0x28
    46f8:	0000001d 	0x1d
    46fc:	0000001f 	0x1f
    4700:	8205c000 	lb	a1,-16384(s0)
    4704:	800f0000 	lb	t7,0(zero)
    4708:	fffffffc 	0xfffffffc
	...
    4714:	00000028 	0x28
    4718:	0000001d 	0x1d
    471c:	0000001f 	0x1f
    4720:	8205c09c 	lb	a1,-16228(s0)
    4724:	80030000 	lb	v1,0(zero)
    4728:	fffffffc 	0xfffffffc
	...
    4734:	00000020 	add	zero,zero,zero
    4738:	0000001d 	0x1d
    473c:	0000001f 	0x1f
    4740:	8205c34c 	lb	a1,-15540(s0)
    4744:	80010000 	lb	at,0(zero)
    4748:	fffffffc 	0xfffffffc
	...
    4754:	00000018 	mult	zero,zero
    4758:	0000001d 	0x1d
    475c:	0000001f 	0x1f
    4760:	8205c7cc 	lb	a1,-14388(s0)
    4764:	80000000 	lb	zero,0(zero)
    4768:	fffffffc 	0xfffffffc
	...
    4774:	00000018 	mult	zero,zero
    4778:	0000001d 	0x1d
    477c:	0000001f 	0x1f
    4780:	8205c824 	lb	a1,-14300(s0)
    4784:	80000000 	lb	zero,0(zero)
    4788:	fffffffc 	0xfffffffc
	...
    4794:	00000020 	add	zero,zero,zero
    4798:	0000001d 	0x1d
    479c:	0000001f 	0x1f
    47a0:	8205c884 	lb	a1,-14204(s0)
	...
    47b8:	0000001d 	0x1d
    47bc:	0000001f 	0x1f
    47c0:	8205c930 	lb	a1,-14032(s0)
    47c4:	801f0000 	lb	ra,0(zero)
    47c8:	fffffffc 	0xfffffffc
	...
    47d4:	000000a0 	0xa0
    47d8:	0000001d 	0x1d
    47dc:	0000001f 	0x1f
    47e0:	8205ced0 	lb	a1,-12592(s0)
    47e4:	80070000 	lb	a3,0(zero)
    47e8:	fffffffc 	0xfffffffc
	...
    47f4:	000000b8 	0xb8
    47f8:	0000001d 	0x1d
    47fc:	0000001f 	0x1f
    4800:	8205d0c0 	lb	a1,-12096(s0)
    4804:	80070000 	lb	a3,0(zero)
    4808:	fffffffc 	0xfffffffc
	...
    4814:	00000028 	0x28
    4818:	0000001d 	0x1d
    481c:	0000001f 	0x1f
    4820:	8205d164 	lb	a1,-11932(s0)
    4824:	80ff0000 	lb	ra,0(a3)
    4828:	fffffffc 	0xfffffffc
	...
    4834:	00000038 	0x38
    4838:	0000001d 	0x1d
    483c:	0000001f 	0x1f
    4840:	8205d334 	lb	a1,-11468(s0)
    4844:	c0ff0000 	ll	ra,0(a3)
    4848:	fffffffc 	0xfffffffc
	...
    4854:	00000048 	0x48
    4858:	0000001d 	0x1d
    485c:	0000001f 	0x1f
    4860:	8205d60c 	lb	a1,-10740(s0)
    4864:	801f0000 	lb	ra,0(zero)
    4868:	fffffffc 	0xfffffffc
	...
    4874:	00000028 	0x28
    4878:	0000001d 	0x1d
    487c:	0000001f 	0x1f
    4880:	8205d6d0 	lb	a1,-10544(s0)
    4884:	803f0000 	lb	ra,0(at)
    4888:	fffffffc 	0xfffffffc
	...
    4894:	00000030 	0x30
    4898:	0000001d 	0x1d
    489c:	0000001f 	0x1f
    48a0:	8205d7b8 	lb	a1,-10312(s0)
    48a4:	803f0000 	lb	ra,0(at)
    48a8:	fffffffc 	0xfffffffc
	...
    48b4:	00000038 	0x38
    48b8:	0000001d 	0x1d
    48bc:	0000001f 	0x1f
    48c0:	8205df48 	lb	a1,-8376(s0)
    48c4:	c0ff0000 	ll	ra,0(a3)
    48c8:	fffffffc 	0xfffffffc
	...
    48d4:	00000038 	0x38
    48d8:	0000001d 	0x1d
    48dc:	0000001f 	0x1f
    48e0:	8205e100 	lb	a1,-7936(s0)
    48e4:	c0ff0000 	ll	ra,0(a3)
    48e8:	fffffffc 	0xfffffffc
	...
    48f4:	00000080 	sll	zero,zero,0x2
    48f8:	0000001d 	0x1d
    48fc:	0000001f 	0x1f

Disassembly of section .gnu.attributes:

00000000 <.gnu.attributes>:
   0:	00000f41 	0xf41
   4:	756e6700 	jalx	0x5b99c000x6700
   8:	00070100 	sll	zero,a3,0x4
   c:	03040000 	0x3040000
