#include <sys/fcntl.h>
#include <sys/autoconf.h>
#include <unistd.h>
#include <string.h>
#include <lib.h>
#include <stdlib.h>
#include <arp.h>
#include <bsp.h>

extern DPCONF *dpconf;
extern BOOLEAN OSRunning;
static INT8U debugmode = 0;

#if CONFIG_VERSION < 4 
devop_t dev_socket = { "remote", socket_open, socket_close, socket_write, socket_read};
#else
extern devop_t dev_socket;  
#endif

extern devop_t *dev_list[];
extern devop_t dev_uart;

INT8U *dbgbuf;
const INT8U dbghdr[44] = {0x00, 0x00, 0x00, 0xe0, 0x4c, 0x68, 0x00, 0x13, 0x00, 0xe0, 0x4c, 0x81, 0x55, 0x66, 0x08, 0x00, 0x45, 0x04, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 0x00, 0x00, 0xc0, 0xa8, 0x00, 0x0a, 0xc0, 0xa8, 0x00, 0x03, 0x00, 0xa2, 0x00, 0xa2, 0x00, 0x78, 0x00, 0x00};

int socket_open(const void *path, int flags, int mode)
{
    INT8U i = 0;
    dbgbuf = malloc(256);
    memset(dbgbuf, 0, 256);
    memcpy(dbgbuf, dbghdr, 44);
    GetMacAddr_F(&dbgbuf[8], eth0);
    memcpy(&dbgbuf[28], &dpconf->HostIP[eth0].addr, 4);
    memcpy(&dbgbuf[32], &dpconf->DBGIP.addr, 4);

    //if mac address is all zero or all 0xff, using ARP to get mac
    if (check_mac_addr(dpconf->DBGMacAddr))
    {
        if (OSRunning == OS_TRUE)
        {
            for (i = 0; i < 10; i++)
            {
                if (findEtherAddr_F((INT8U *) &dpconf->DBGIP.addr, (INT8U *) &dpconf->DBGMacAddr))
                    break;
                else
                    OSTimeDly(5);
            }
        }
        else
            i = 10;
    }
    else //user set a default mac address
    {
        debugmode = 1;
        i = 0;
    }

    if (i != 10)
        memcpy(&dbgbuf[2], &dpconf->DBGMacAddr, 6);
    else
        memset(&dbgbuf[2], 0xFF, 6);

    *((INT16U *) &dbgbuf[36]) = htons(flags);
    *((INT16U *) &dbgbuf[38]) = htons(flags);
    return 0;
}
int socket_close(int fd) {
    return -1;
}
_ssize_t socket_write(int fd, const void *ptr, size_t len) {

    INT8U  xferlen = 0;
    INT32U totallen = 0;
    INT8U  pktlen;


    if (debugmode == 0 && OSRunning != OS_TRUE)
        return 0;

    memcpy(&dbgbuf[28], &dpconf->HostIP[eth0].addr, 4);

    while (len)
    {
        if (len < 212)
            xferlen = len;
        else
            xferlen = 212;

        len -= xferlen;

        GetMacAddr(&dbgbuf[8], eth0);
        memcpy(&dbgbuf[44], ptr+totallen, xferlen);

        totallen += xferlen;

        xferlen += 8; //inclde udp header
        pktlen = 14 + 20 + xferlen ;
        //ethernet header + ip header length
        dbgbuf[41] = xferlen ;
        gmacsend_F(dbgbuf + 2, pktlen,  OPT_UDPCS | OPT_FREE,  0 );
        //bsp_wait(2);
    }
    return totallen;
}
_ssize_t socket_read(int fd,  void *ptr, size_t len) {
    return -1;
}

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
void set_remote_dbg(unsigned short port)
{
    dev_list[1] = &dev_socket;
    dev_list[2] = &dev_socket;
#if CONFIG_VERSION == 4
    dev_socket.open_d = &socket_open;
    dev_socket.close_d = &socket_close;
    dev_socket.read_d = &socket_read;
#endif
    dev_socket.write_d = &socket_write;
    socket_open("remote", port, 0);
}

#endif

#if CONFIG_VERSION < IC_VERSION_DP_RevF 
void set_uart_dbg(unsigned int freq)
{
    dev_list[1] = &dev_uart;
    dev_list[2] = &dev_uart;

    //turn on uart
    REG8(0xB9000004) |= 0x80;

    bsp_wait(1000);
#if CONFIG_VERSION == 1
    open("uart", freq, 0);
#else
    uart_open("uart", freq, 0);
#endif
    bsp_wait(1000);
}
#endif
