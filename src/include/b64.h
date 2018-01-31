#ifndef __CODEDEC_H__
#define __CODEDEC_H__


int b64_decode( const char* str, unsigned char* space, int size);
int b64_encode( unsigned char *dst, int *dlen, unsigned char *src, int  slen );
#endif
