void freePkt_R(PKT *pkt);
void updateArpTable_R(unsigned char* ethAddr, unsigned char* ipAddr);
INT32U send_R(INT8U *txbufferaddress, INT32U length,INT8U options, void *free);
void bsp_set_sstate_R(INT8U state) ;
