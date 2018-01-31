#ifndef _TESTIO_H
#define _TESTIO_H

#define Interrupt_IMSR  (0xBAF70000+0x0002c)

#define RISC_ACC_DATA 	(0xBAF70000+0x0600)

#define RISC_ACC_ADDR	(0xBAF70000+0x0604)

#define RISC_ACC_SET	(0xBAF70000+0x0608)

#define RISC_ACC_FLAG_bit	(1<<31)
enum {
	RISC_ACC_START = (1<<31),
	RISC_ACC_END   = (0<<31),
}RISC_ACC_STATUS;

#define RISC_RW_SEL_bit		(1<<23)

enum {
	 RISC_READ =(0<<23),
	 RISC_WRITE =(1<<23),
}RISC_RW_SEL;


#define RISC_ACC_BYTEEN_bits (0xf<<16)
enum {
	 RISC_ACC_1BYTEEN_0 =(1<<16),
	 RISC_ACC_1BYTEEN_1 =(1<<17),
	 RISC_ACC_1BYTEEN_2 =(1<<18),
	 RISC_ACC_1BYTEEN_3 =(1<<19),
	 RISC_ACC_2BYTEEN_01 =(3<<16),
	 RISC_ACC_2BYTEEN_23 =(3<<18),
	 RISC_ACC_3BYTEEN =(7<<16),
	 RISC_ACC_4BYTEEN =(0xf<<16),
}RISC_ACC_BYTEEN;

void IB_Access_OOB_Handler (void);
//void bsp_testio_inten(void);
#endif 






