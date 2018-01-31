#ifndef _I2C_GPIO_SLAVE_
#define _I2C_GPIO_SLAVE_

#include "bsp.h"
#include "OOBMAC_COMMON.h"
#define MAC_GPIOCTL3            0x0508

INT8U get_gpio_input(OOBMAC_gpio_no gpio_num);
INT8U I2C_check_address(INT8U addr);
void Start_Condition(void);
void I2C_gpio_init(void);


#endif /* _I2C_GPIO_SLAVE_ */
