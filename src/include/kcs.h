#ifndef _KCS_H_
#define _KCS_H_

//KCS Configuration
#define KCS_CONF                    0x00
#define KCS_CTRL                     0x01
#define KCS_STATUS1              0x02
//bit 0 => to clear OBF
#define KCS_STATUS2              0x03
#define KCS_ERROR_CODE        0x04
#define KCS_COMMAND             0x05
#define KCS_DATA_OUT           0x06
#define KCS_DATA_IN              0x07
#define KCS_RX_DESC              0x11
#define KCS_RX_DESC_LEN      0x10
#define KCS_RX_DMA_ADDR     0x14
#define KCS_TX_DESC               0x21
#define KCS_TX_DESC_LEN       0x20
#define KCS_TX_DMA_ADDR     0x24
#define KCS_IMR                       0x30
#define KCS_ISR                       0x32



void kcstest_rx_recycle(void);
void kcstest_tx_count(void);
void kcstest_abort(void);
void bsp_kcsTest_handler(void);
void bsp_kcsTest_init(void);
void bsp_kcs_disable(void);
void kcs_send(INT32U len,INT8U data);



#endif

