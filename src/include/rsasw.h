#ifndef XYSSL_RSASW_H
#define XYSSL_RSASW_H
#include "rsa.h"
#include <sys/inet_types.h>

/**
 * \brief          Checkup routine
 *
 * \return         0 if successful, or 1 if the test failed
 */
int resumeInitKey( int verbose, unsigned char *p, ptcp_pcb pcb);

int tlsInitKeys( int verbose, unsigned char *p, ptcp_pcb pcb);

int tls1_prf( unsigned char *secret, int slen, char *label,
              unsigned char *random, int rlen,
              unsigned char *dstbuf, int dlen );

#endif

