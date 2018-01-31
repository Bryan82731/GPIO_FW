/*
 * Bitbanged MDIO support.
 *
 * Author: Scott Wood <scottwood@freescale.com>
 * Copyright (c) 2007 Freescale Semiconductor
 *
 * Based on CPM2 MDIO code which is:
 *
 * Copyright (c) 2003 Intracom S.A.
 *  by Pantelis Antoniou <panto@intracom.gr>
 *
 * 2005 (c) MontaVista Software, Inc.
 * Vitaly Bordug <vbordug@ru.mvista.com>
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */

#include "parammd.h"
#include <rlx/rlx_types.h>
#include "OOBMAC_COMMON.h"
#include "mdiobitbang.h"



//Reverse with C22 MDIO frame format
#define MDIO_READ 2
#define MDIO_WRITE 1

#define MDIO_C45 (1<<15)
#define MDIO_C45_ADDR (MDIO_C45 | 0)
#define MDIO_C45_READ (MDIO_C45 | 3)
#define MDIO_C45_WRITE (MDIO_C45 | 1)
#define MDIO_C45_POSTREAD (MDIO_C45 | 2)


#define MDIO_SETUP_TIME 10
#define MDIO_HOLD_TIME 10

/* Minimum MDC period is 400 ns, plus some margin for error.  MDIO_DELAY
 * is done twice per period.
 */
//#define MDIO_DELAY 250



/* The PHY may take up to 300 ns to produce data, plus some margin
 * for error.
 */
//#define MDIO_READ_DELAY 350



INT32U check_test_timer(void)
{
	INT32U temp ;
	asm volatile("mfc0   %0, $9"   : "=r"(temp));
	return temp;
	
}

INT32U  get_timer(INT32U base)
{
	//printf("%s = %x\n", __FUNCTION__, mips_count_get() );
	return check_test_timer() - base;
}





extern INT32U Freq_MMD ;

extern INT32U bsp_wait_cnt ;

void wr_ndelay(INT32U delay_ns)
{

	/* the minun inter-time is ~100us !!!

	INT32U count ;
	INT32U data ;
	INT32U timer ;


	//DCO clock  ~400MHz clock 
	OOB_READ_IB(OOBMAC_IOBASE,0xff,0xE018,&data);
	if (data & (1<<10 )){
		count = delay_ns/3 ;  // can not use 2.5 ,why ??
	}else {  // 250Mhz clock
		count = delay_ns/4 ;
	
	}

	clear_test_timer () ;
	timer = check_test_timer () ;
	start_test_timer () ;
	do {
		timer = check_test_timer () ;
	}while ( timer < count) ;
*/

	INT32U i ;
	INT32U t_start, t_stop  ; 
	

	if (	delay_ns <= 50 )
		return ;
	else{
		t_start = get_timer(0);
		delay_ns = delay_ns - 50 ; 
		t_stop = delay_ns* 38 / 100 ; // 2.6
		while ( get_timer(t_start) <=  t_stop  ) ;
	}



/*
	INT32U i ;
	INT32U tmp ;

	//50 is the parameter tuning at 400M clk(after DCO)
	//32 is the parameter tuning at 250M clk(default)
	// // the write  OOB mac gpio reg need 50ns time
	if (delay_ns <50){
		return ;
	}else if (delay_ns <1000){
		tmp = (delay_ns-50)/60 ; 
	}else {
		tmp = bsp_wait_cnt*(delay_ns-50)/1000 ;	
	}
		
    for (i = 0 ; i <= tmp  ; i++)
		asm volatile("ssnop");

*/

}



void rd_ndelay(INT32U delay_ns)
{

	/* the minun inter-time is ~100us !!!

	INT32U count ;
	INT32U data ;
	INT32U timer ;


	//DCO clock  ~400MHz clock 
	OOB_READ_IB(OOBMAC_IOBASE,0xff,0xE018,&data);
	if (data & (1<<10 )){
		count = delay_ns/3 ;  // can not use 2.5 ,why ??
	}else {  // 250Mhz clock
		count = delay_ns/4 ;
	
	}

	clear_test_timer () ;
	timer = check_test_timer () ;
	start_test_timer () ;
	do {
		timer = check_test_timer () ;
	}while ( timer < count) ;
*/
	


	INT32U i ;
	INT32U t_start, t_stop  ; 
	

	if (	delay_ns <= 300 )
		return ;
	else{
		t_start = get_timer(0);
		delay_ns = delay_ns - 300 ; 
		t_stop = delay_ns*38 / 100 ; // 2.5
		while ( get_timer(t_start) <=  t_stop  ) ;
	}

	//50 is the parameter tuning at 400M clk(after DCO)
	//32 is the parameter tuning at 250M clk(default)
	// the read OOB mac gpio reg need 300ns time 
/*
	INT32U tmp ; 
	if (delay_ns <300){
		return ;
	}else if (delay_ns <1000){
		tmp = (delay_ns-300)/60 ;
	}else {
		tmp = bsp_wait_cnt*(delay_ns-300)/1000 ; 
	}
		
	for (i = 0 ; i <= tmp  ; i++)
		asm volatile("ssnop");

*/ 

}




/* MDIO must already be configured as output. */
static void mdiobb_send_bit( int val)
{


//	ops->set_mdio_dir(ctrl,1);

	if (val)
		paraport_setmdio_data_HIGH() ;
	else
		paraport_setmdio_data_LOW();
	
	wr_ndelay(Freq_MMD/2); //half clock 
	paraport_setmdc_HIGH() ;
	
	wr_ndelay(Freq_MMD/2);  
	paraport_setmdc_LOW() ;
}

/* MDIO must already be configured as input. */
static int mdiobb_get_bit(void )
{
	int temp;
	int i ;
	
	rd_ndelay(Freq_MMD/2); //half clock 
	paraport_setmdc_HIGH() ;

	wr_ndelay(Freq_MMD/2); //half clock 
	paraport_setmdc_LOW() ;

	
	//place here to input data
	temp=paraport_getmdio_data ();

	return temp;
}


/* MDIO must already be configured as output.*/ 
static void mdiobb_send_num( INT16U val, int bits)
{
	int i;

	for (i = bits - 1; i >= 0; i--)
		mdiobb_send_bit((val >> i) & 1);
}



/* MDIO must already be configured as input. */
static INT16U mdiobb_get_num( int bits)
{
	int i;
	INT16U ret = 0;

	for (i = bits - 1; i >= 0; i--) {
		ret <<= 1;
		ret |= mdiobb_get_bit();
	}

	return ret;
}

/* Utility to send the preamble, address, and
 * register (common to read and write).
 */
static void mdiobb_cmd(int op, INT8U phy, INT8U reg)
{
	INT32U i;

	paraport_setmdio_dir_OUT();
	
	

	/*
	 * Send a 32 bit preamble ('1's) with an extra '1' bit for good
	 * measure.  The IEEE spec says this is a PHY optional
	 * requirement.  The AMD 79C874 requires one after power up and
	 * one after a MII communications error.  This means that we are
	 * doing more preambles than we need, but it is safer and will be
	 * much more robust.
	 */

	for (i = 0; i < 32; i++)
		mdiobb_send_bit( 1);

	/* send the start bit (01) and the read opcode (10) or write (10).
	   Clause 45 operation uses 00 for the start and 11, 01 for
	   read/write,00 for address,10 for post read  */
	mdiobb_send_bit( 0);
	if (op & MDIO_C45)
		mdiobb_send_bit( 0);
	else
		mdiobb_send_bit( 1);
	mdiobb_send_bit( (op >> 1) & 1);
	mdiobb_send_bit( (op >> 0) & 1);

	mdiobb_send_num( phy, 5);
	mdiobb_send_num( reg, 5);
}

/* In clause 45 mode all commands are prefixed by MDIO_ADDR to specify the
   lower 16 bits of the 21 bit address. This transfer is done identically to a
   MDIO_WRITE except for a different code. To enable clause 45 mode or
   MII_ADDR_C45 into the address. Theoretically clause 45 and normal devices
   can exist on the same bus. Normal devices should ignore the MDIO_ADDR
   phase. */
static int mdiobb_cmd_addr( int phy, INT32U addr)
{
	INT32U dev_addr = (addr >> 16) & 0x1F;
	INT32U reg = addr & 0xFFFF;
	mdiobb_cmd(MDIO_C45_ADDR, phy, dev_addr);

	/* send the turnaround (10) */
	mdiobb_send_bit( 1);
	mdiobb_send_bit( 0);

	mdiobb_send_num( reg, 16);

	paraport_setmdio_dir_IN();
	mdiobb_get_bit();

	return dev_addr;
}


 INT16U mdiobb_read( INT32U phy, INT32U reg)
{
	INT32U ret, i;

	if (reg & MII_ADDR_C45) {
		reg = mdiobb_cmd_addr( phy, reg);
		mdiobb_cmd( MDIO_C45_READ, phy, reg);
	} else{
		
		mdiobb_cmd(MDIO_READ, phy, reg);
		
	}
	paraport_setmdio_dir_IN();

	/* check the turnaround bit: the PHY should be driving it to zero, if this
	 * PHY is listed in phy_ignore_ta_mask as having broken TA, skip that
	 */
	if (mdiobb_get_bit() != 0) {
		/* PHY didn't drive TA low -- flush any bits it
		 * may be trying to send.
		 */
		for (i = 0; i < 32; i++)
			mdiobb_get_bit();

		return 0xffff;
	}

	ret = mdiobb_get_num( 16);
	mdiobb_get_bit();
	return ret;
}


 
INT8U mdiobb_post_read( INT32U phy, INT32U reg, INT16U* desadr,INT16U step)
 {
	 INT32U ret, i;
 
	 reg = mdiobb_cmd_addr( phy, reg);
	 while(step--){
		 mdiobb_cmd( MDIO_C45_POSTREAD, phy, reg);
		 paraport_setmdio_dir_IN();
	 
		 /* check the turnaround bit: the PHY should be driving it to zero, if this
		  * PHY is listed in phy_ignore_ta_mask as having broken TA, skip that
		  */
		 if (mdiobb_get_bit() != 0) {
			 /* PHY didn't drive TA low -- flush any bits it
			  * may be trying to send.
			  */
			 for (i = 0; i < 32; i++)
				 mdiobb_get_bit();
	 
			 return 0xff;
		 }
	 
		 ret = mdiobb_get_num( 16);
		 *desadr = (INT16U)ret;
		 desadr++;
		 mdiobb_get_bit();

	 }

	 return 0;
	 
 }

 INT8U mdiobb_write(INT32U phy, INT32U reg, INT16U val)
{

	if (reg & MII_ADDR_C45) {
		reg = mdiobb_cmd_addr(phy, reg);
		mdiobb_cmd( MDIO_C45_WRITE, phy, reg);
	} else
		mdiobb_cmd( MDIO_WRITE, phy, reg);

	/* send the turnaround (10) */
	mdiobb_send_bit( 1);
	mdiobb_send_bit( 0);

	mdiobb_send_num( val, 16);

	paraport_setmdio_dir_IN();  
	mdiobb_get_bit();
	return 0;
}







