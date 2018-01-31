#include "parammd.h"
#include "OOBMAC_COMMON.h"
#include "OOBMAC.h"
#include "BSP.h"



/* Set the Management Data Clock high if level is one,
	 * low if level is zero.
*/


		


/* Configure the Management Data I/O pin as an input if
 * "output" is zero, or an output if "output" is one.
 */
 /*
void paraport_setmdio_dir(int output){
   //output 
  if(output){
	OOBMAC_gpio_init (MDIO_PIN, OUTPUT , 1);
  //input
  }else{
	OOBMAC_gpio_init (MDIO_PIN, INPUT , 0);
  }

  
   
}
*/

/* Set the Management Data I/O pin high if value is one,
 * low if "value" is zero.  This may only be called
 * when the MDIO pin is configured as an output.
 */



/*
void paraport_setmdio_data( int value){
	//OOBMAC_gpio_output(MDIO_PIN , value) ;
	
	INT32U temp ;

	//for simiply the GPIO ,	
	temp = REG32(OOBMAC_IOBASE+MAC_GPIOCTL3);
	temp = temp & (~(1<<6))|(value<<6);  //set new value for this bit
	REG32(OOBMAC_IOBASE+MAC_GPIOCTL3) = temp;
}
*/

/* Retrieve the state Management Data I/O pin. */
/*
int paraport_getmdio_data(void){

	//return OOBMAC_gpio_input_general (MDIO_PIN ) ;
	return  ;
}
*/





