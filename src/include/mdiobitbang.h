#ifndef _MDIO_BITBANG_H
#define _MDIO_BITBANG_H

/* Or MII_ADDR_C45 into regnum for read/write on mii_bus to enable the 21 bit
   IEEE 802.3ae clause 45 addressing mode used by 10GIGE phy chips. */
#define MII_ADDR_C45 (1<<30)


INT16U mdiobb_read( INT32U phy, INT32U reg);
INT8U mdiobb_post_read( INT32U phy, INT32U reg, INT16U* desadr,INT16U step);
INT8U mdiobb_write(INT32U phy, INT32U reg, INT16U val);




#endif
