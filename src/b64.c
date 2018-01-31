#include <b64.h>
#include <sys/dpdef.h>
#include <sys/autoconf.h>

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || defined(CONFIG_BUILDROM)
const static char b64_decode_table[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};

static const unsigned char base64_enc_map[64] ={    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',    'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T',    'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd',    'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',    'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',    'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',    '8', '9', '+', '/'};
int b64_encode( unsigned char *dst, int *dlen, unsigned char *src, int  slen )
{
    int i, n;
    int C1, C2, C3;
    unsigned char *p;
    if ( slen == 0 )
        return( 0 );
    n = (slen << 3) / 6;
    switch ( (slen << 3) - (n * 6) )
    {
    case  2:
        n += 3;
        break;
    case  4:
        n += 2;
        break;
    default:
        break;
    }
    if ( *dlen < n + 1 )
    {
        *dlen = n + 1;
        return( -1 );
    }
    n = (slen / 3) * 3;
    for ( i = 0, p = dst; i < n; i += 3 )
    {
        C1 = *src++;
        C2 = *src++;
        C3 = *src++;
        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 &  3) << 4) + (C2 >> 4)) & 0x3F];
        *p++ = base64_enc_map[(((C2 & 15) << 2) + (C3 >> 6)) & 0x3F];
        *p++ = base64_enc_map[C3 & 0x3F];
    }
    if ( i < slen )
    {
        C1 = *src++;
        C2 = ((i + 1) < slen) ? *src++ : 0;
        *p++ = base64_enc_map[(C1 >> 2) & 0x3F];
        *p++ = base64_enc_map[(((C1 & 3) << 4) + (C2 >> 4)) & 0x3F];
        if ( (i + 1) < slen )
            *p++ = base64_enc_map[((C2 & 15) << 2) & 0x3F];
        else
            *p++ = '=';
        *p++ = '=';
    }
    *dlen = p - dst;
    *p = 0;
    return( 0 );
}
int b64_decode( const char* str, unsigned char* space, int size)
{
    const char* cp;
    int space_idx, phase;
    int d, prev_d = 0;
    unsigned char c;

    space_idx = 0;
    phase = 0;
    for ( cp = str; *cp != '\0'; ++cp )
    {
        d = (int)b64_decode_table[(int) *cp];
        if ( d != -1 )
        {
            switch ( phase )
            {
            case 0:
                ++phase;
                break;
            case 1:
                c = ( ( prev_d << 2 ) | ( ( d & 0x30 ) >> 4 ) );
                if ( space_idx < size )
                    space[space_idx++] = c;
                ++phase;
                break;
            case 2:
                c = ( ( ( prev_d & 0xf ) << 4 ) | ( ( d & 0x3c ) >> 2 ) );
                if ( space_idx < size )
                    space[space_idx++] = c;
                ++phase;
                break;
            case 3:
                c = ( ( ( prev_d & 0x03 ) << 6 ) | d );
                if ( space_idx < size )
                    space[space_idx++] = c;
                phase = 0;
                break;
            }
            prev_d = d;
        }
    }
    return space_idx;
}
#endif
