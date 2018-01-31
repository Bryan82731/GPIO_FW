#ifndef _PARAMMD_H
#define _PARAMMD_H

#include "OOBMAC.h"
#include "bsp_fp_revA.h"



//	#include "mdiobitbang.h"


		/* Set the Management Chip Select  high if level is one,
		 * low if level is zero.
		 */
		#define paraport_setmcs_HIGH() 	REG8(OOBMAC_IOBASE+MAC_GPIOCTL+2)=0x07   // fix me 
		#define paraport_setmcs_LOW() 	REG8(OOBMAC_IOBASE+MAC_GPIOCTL+2)=0x03

	
		/* Set the Management Data Clock high if level is one,
		 * low if level is zero.
		 */
		#define paraport_setmdc_HIGH() 	REG8(OOBMAC_IOBASE+MAC_GPIOCTL3)=0x07
		#define paraport_setmdc_LOW() 	REG8(OOBMAC_IOBASE+MAC_GPIOCTL3)=0x03
	
		/* Configure the Management Data I/O pin as an input if
		 * "output" is zero, or an output if "output" is one.
		 */
		#define  paraport_setmdio_dir_OUT() REG8(OOBMAC_IOBASE+MAC_GPIOCTL2+1)=0x07
		#define  paraport_setmdio_dir_IN() REG8(OOBMAC_IOBASE+MAC_GPIOCTL2+1)=0x01
	
		/* Set the Management Data I/O pin high if value is one,
		 * low if "value" is zero.	This may only be called
		 * when the MDIO pin is configured as an output.
		 */
	#define paraport_setmdio_data_HIGH()	REG8(OOBMAC_IOBASE+MAC_GPIOCTL2+1)=0x07
	#define paraport_setmdio_data_LOW() 	REG8(OOBMAC_IOBASE+MAC_GPIOCTL2+1)=0x03
	
		/* Retrieve the state Management Data I/O pin. */
		#define paraport_getmdio_data() (((REG8(OOBMAC_IOBASE+MAC_GPIOCTL2+1)>>3)& 0x01)  ) 
		//just test read OOB_mac register lantency
		//#define paraport_getmdio_data() (((REG8(OOBMAC_IOBASE+MAC_SWINT_ISR+1)>>3)& 0x01)  ) 
	

		
	




#endif

