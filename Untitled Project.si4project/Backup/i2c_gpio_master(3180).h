#include "bsp.h"
#include "OOBMAC_COMMON.h"


#define MAC_GPIOCTL3            0x0508


INT8U get_gpio_input(OOBMAC_gpio_no gpio_num);
INT8U i2c_read_byte( BOOLEAN nack , BOOLEAN send_stop );
INT8U i2c_write_byte( BOOLEAN send_start, BOOLEAN send_stop, INT8U byte) ;
INT8U i2c_read_bit( void ) ;
void i2c_write_bit( INT8U bit ) ;
void i2c_stop_cond( void );
void i2c_start_cond( void ) ;
INT8U get_gpio_input(OOBMAC_gpio_no gpio_num);
void set_gpio_output(OOBMAC_gpio_no gpio_num,INT8U val);
void init_gpio_dir(OOBMAC_gpio_no gpio_num,OOBMAC_gpio_dir dir,INT8U val);
void I2C_delay();
void Start_Condition(void);
void I2C_gpio_init(void);

