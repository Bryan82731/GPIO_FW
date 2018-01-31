#include "bsp.h"
#include "md5.h"
#include "sha1.h"
#include "aes.h"

#if (CONFIG_VERSION  < IC_VERSION_DP_RevF) || (CONFIG_VERSION  >= IC_VERSION_FP_RevA)

void bsp_tls_init(void)
{
	//rlx_dcache_flush_all();
	//rlx_dcache_invalidate_all();//rlx_disable_WMP();
//rlx_dcache_wbinvalidate_all();
    tlsdesc_t *tlsdesc = (tlsdesc_t *) TLSdescStartAddr;
    tlsdesc->offset1.word = 0x40000000; //only set eor
    REG32(TLS_BASE_ADDR + TLS_DESC) = VA2PA(TLSdescStartAddr);	
}

void tls_set(INT8U *keyaddr, INT8U *payloadaddr, INT8U alsel, INT16U length)
{
    tlsdesc_t *tlsdesc = (tlsdesc_t *) TLSdescStartAddr;
    tlsdesc->offset1.cmd.length = length;
    tlsdesc->offset1.cmd.alsel = (alsel & 0x0f);

#if CONFIG_VERSION >= IC_VERSION_FP_RevA
	tlsdesc->offset1.cmd.key_size = ((alsel & 0xf0)>>4);
#endif

#if 1
    if (keyaddr != NULL)
        tlsdesc->key_addr = (INT8U *) VA2PA(keyaddr);
    tlsdesc->payload_addr = (INT8U *) VA2PA(payloadaddr);
#endif
#if 0
    if (keyaddr != NULL)
        tlsdesc->key_addr = keyaddr;
    tlsdesc->payload_addr = payloadaddr;
	#endif
    tlsdesc->offset1.cmd.own = 1; //set own bit
    REG8(TLS_BASE_ADDR + TLS_POLL) = 0x80; //Bit 7 must be set
}

int tls_valid()
{
    tlsdesc_t *tlsdesc = (tlsdesc_t *) TLSdescStartAddr;
    return tlsdesc->offset1.cmd.valid;
}
int tls_poll()
{
    tlsdesc_t *tlsdesc = (tlsdesc_t *) TLSdescStartAddr;
    //int count = 0;

    //while (count<500)
    //    count++;

    while (1)
    {
        if (tlsdesc->offset1.cmd.own == 0)
            break;
    }
    /*
    if(count == 10000)
    {
    	DEBUGMSG(DRIVER_DEBUG, "Time out\n");
    	tls_timout++;
    	return -1;
    }*/
    return tlsdesc->offset1.cmd.length;
}
#endif

#if CONFIG_CODEC_TEST
unsigned int tls_pass_count = 0;
unsigned int md5_pass = 0;
unsigned int sha1_pass = 0;
unsigned int md5_hmac_pass_16 = 0;
unsigned int md5_hmac_pass_24 = 0;
unsigned int sha1_hmac_pass_20 = 0;
unsigned int sha1_hmac_pass_24 = 0;
unsigned int aes_encrypt_pass = 0;
unsigned int aes_decrypt_pass = 0;

unsigned int tls_fail_count = 0;
unsigned int md5_fail = 0;
unsigned int sha1_fail = 0;
unsigned int md5_hmac_fail_16 = 0;
unsigned int md5_hmac_fail_24 = 0;
unsigned int sha1_hmac_fail_20 = 0;
unsigned int sha1_hmac_fail_24 = 0;
unsigned int aes_encrypt_fail = 0;
unsigned int aes_decrypt_fail = 0;

void tls_pass()
{
    //REG8(TLS_BASE_ADDR + TLS_POLL) = 0x01;
    tls_pass_count++;
}

void tls_fail()
{
    //REG8(TLS_BASE_ADDR + TLS_POLL) = 0x02;
    tls_fail_count++;
}


void tls_data_pass()
{
    //REG8(TLS_BASE_ADDR + TLS_POLL) = 0x04;
}


void tls_data_fail()
{
    //REG8(TLS_BASE_ADDR + TLS_POLL) = 0x08;
}

void tls_stop()
{
    //REG8(TLS_BASE_ADDR + TLS_POLL) = 0x40;
}


void md5_rand_test()
{
    int len = 1 + rand() % 1500;
    md5_context md5, md5H;
    int i = 0, j =0;

    unsigned char buf[16] = {0};
    unsigned char buf1[16] = {0};
    unsigned char *p = malloc(len);

    for (i = 0; i < len; i++)
    {
        p[j] = rand() % 256;
    }

    md5_starts(&md5H);
    md5_updateH( &md5H, p, len);
    md5_finishH( &md5H, buf);

    md5_starts(&md5);
    md5_update( &md5, p, len);
    md5_finish( &md5, buf1);
    
    if (memcmp(buf1, buf, 16) != 0)
    {
    	md5_fail++;
        tls_fail();
    }
    else
    {
    	md5_pass++;
        tls_pass();
    }
    free(p);
}
void sha1_rand_test()
{
    sha1_context sha1H;
    sha1_context sha1;
    
    int j = 0, len = 1 + rand()%1500;
    unsigned char outputHW[20] = {0};
    unsigned char outputSW[20] = {0};
    
    unsigned char *p = malloc(len);

    for (j = 0 ; j < len; j++)
    {
        //p[j] = j % 256;
        p[j] = rand() % 256;
    }

    sha1_startsH( &sha1H );
    sha1_updateH( &sha1H, p, len);
    sha1_finishH( &sha1H, outputHW );

    sha1_starts( &sha1 );
    sha1_update( &sha1, p, len);
    sha1_finish( &sha1, outputSW );
    
    if (memcmp(outputSW, outputHW, 20) != 0)
    {
        tls_fail();
        sha1_fail++;
    }
    else
    {
        tls_pass();
        sha1_pass++;
    }
    
    free(p);
}


void md5_hmac_rand_test()
{
    int dsize = 1 + rand() % 1500;
    unsigned char key[24] = {0};   
    unsigned char keySW[24] = {0};
    unsigned char *p = malloc(dsize);

    int i = 0;

    for ( i = 0; i < 16;i++ )
    {
        key[i] = keySW[i] = rand() % 256;
    }

    for ( i = 0; i < dsize;i++ )
    {
        p[i] = rand() % 256;
    }

    md5_hmac(key, 16, p, dsize, key);
    md5_hmacSW(keySW, 16, p, dsize, keySW);

    if (memcmp(key, keySW, 16) != 0)
    {
        tls_fail();
        md5_hmac_fail_16++;
    }
    else
    {
        tls_pass();
        md5_hmac_pass_16++;
    }
	
#if CONFIG_VERSION <= IC_VERSION_DP_RevF	
    for ( i = 0; i < 24;i++ )
    {
         key[i] = keySW[i] = rand() % 256;
    }
 
    md5_hmac(key, 24, p, dsize, key);
    md5_hmacSW(keySW, 24, p, dsize, keySW);

    if (memcmp(key, keySW, 24) != 0)
    {
        tls_fail();
        md5_hmac_fail_24++;
    }
    else
    {
        tls_pass();
        md5_hmac_pass_24++;
    }
 #endif          
    free(p);
}

void sha1_hmac_rand_test()
{
    unsigned char key[24] = {0};
    unsigned char keySW[24] = {0};
    int dsize = 1 + rand()%1500;
    unsigned char ans1[20] = {0x4 ,0xa4 ,0xd1 ,0x32 ,0xe ,0x94 ,0x5b ,0xd2 ,0x9b,0xad ,0x2 ,0x51 ,0xa2 ,0x15 ,0x4f ,0x26 ,0x43 ,0x3 ,0xa0,0x8};
    unsigned char ans2[20] = {0xb6 ,0x43 ,0x5e ,0x2e ,0x78 ,0x82 ,0xe ,0x5 ,0xc5, 0x5a ,0x67 ,0xa2 ,0xe5 ,0xd9 ,0x19 ,0x9c ,0x56 ,0x13 ,0x2a, 0xb7};
    unsigned char *p = malloc(dsize);

    int i = 0;

    for ( i = 0; i < 20;i++ )
    {
         key[i] = keySW[i] = rand() % 256;
    }

    for ( i = 0; i < dsize;i++ )
    {
        p[i] = rand() % 256;
    }
        
    sha1_hmac(key, 20, p, dsize, key); 
    sha1_hmacSW(keySW, 20, p, dsize, keySW);   

    if (memcmp(key, keySW, 20) != 0)
    {
        tls_fail();
        sha1_hmac_fail_20++;
    }
    else
    {
        tls_pass();
        sha1_hmac_pass_20++;
    }
 
#if CONFIG_VERSION <= IC_VERSION_DP_RevF 
    for ( i = 0; i < 24;i++ )
    {
        key[i] = keySW[i] = rand() % 256;
    }
        
    sha1_hmac(key, 24, p, dsize, key); 
    sha1_hmacSW(keySW, 24, p, dsize, keySW);   

    if (memcmp(key, keySW, 24) != 0)
    {
        tls_fail();
        sha1_hmac_fail_24++;
    }
    else
    {
        tls_pass();
        sha1_hmac_pass_24++;
    }
 #endif   
    free(p);
}

void aes_rand_test()
{
    
    aes_context asHW, asSW;
    unsigned char aeskeyHW[16], aeskeySW[16];
    unsigned char ivHW[16], ivSW[16];
    unsigned char *dataHW;
    unsigned char *dataSW;
    int len = 1 + rand()%1500;
    int i = 0;

    //len must greater than 20
    i = checkPadLen(16, len);
    if (i)
        len = len + i + 1;
    
    dataHW = malloc(len);
    dataSW = malloc(len);

	if((len == 0x80))
    {
	   aes_decrypt_fail = 0;
    }
    //aes key and iv
    for (i = 0; i< 16; i++)
    {
        //aeskeyHW[i]= aeskeySW[i] = i%256;
        aeskeyHW[i]= aeskeySW[i] = rand()%256;        
    }
   
    for (i = 0; i< 16; i++)
    {
        //ivSW[i]	= ivHW[i]= i%256;
        ivSW[i]	= ivHW[i]= rand() %256;
    }

    for (i = 0; i< len; i++)
    {
        //dataHW[i] = dataSW[i] =i%256;
        dataHW[i] = dataSW[i] =rand() %256;
    }


   aes_setkey_enc_sw(&asSW, aeskeySW, 16*8);
   aes_crypt_cbc_sw(&asSW, AES_ENCRYPT, len, ivSW, dataSW, dataSW);
   
    //Hw
    memcpy(&asHW, aeskeyHW, 16);
    aes_crypt_cbc(&asHW, AES128_CBC_ENCRYPT, len, ivHW, dataHW, dataHW);
   
    		    
    if(memcmp(dataHW, dataSW, len) != 0)
    {
        tls_fail();
        aes_encrypt_fail++;
    }
    else
    {
        tls_pass();
        aes_encrypt_pass++;
    }


    //aes key and iv
    for (i = 0; i< 16; i++)
    {
        //aeskeyHW[i]= aeskeySW[i] = i%256;
        aeskeyHW[i]= aeskeySW[i] = rand() %256;
    }
   
    for (i = 0; i< 16; i++)
    {
        //ivSW[i]	= ivHW[i]= i%256;
        ivSW[i]	= ivHW[i]= rand() %256;
    }
   
   aes_setkey_dec_sw(&asSW, aeskeySW, 16*8);   
   aes_crypt_cbc_sw(&asSW, AES_DECRYPT, len, ivSW, dataSW, dataSW);
 
     //Hw
    memcpy(&asHW, aeskeyHW, 16);
    aes_crypt_cbc(&asHW, AES128_CBC_DECRYPT, len, ivHW, dataHW, dataHW); 
    
    if(memcmp(dataHW, dataSW, len) != 0)
    {
        tls_fail();
        aes_decrypt_fail++;
    }
    else
    {
        tls_pass();
        aes_decrypt_pass++;
    }
                  	   
     free(dataHW);	
     free(dataSW);
}

//static test without lib
void md5_test_static()
{
	unsigned char keyT[80] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98
		,0x76,0x54,0x32,0x10};
	unsigned char dataT[]= {00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x80,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50};	
	unsigned char ans[16] = {0xc5 ,0x6b ,0xd5 ,0x48 ,0xf ,0x6e ,0x54 ,0x13 ,0xcb ,0x62 ,0xa0 ,0xad ,0x96 ,0x66 ,0x61 ,0x3a};
	unsigned char dataT1[sizeof(dataT) + 4];

	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = malloc(sizeof(dataT));	
	
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	memcpy(key + 20, keyT, sizeof(keyT));
#else
	memcpy(key, keyT, sizeof(keyT));
#endif	
	memcpy(data, dataT, sizeof(dataT));	
		
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	tls_set(key, data, MD5_SHA1, 64);
#else
	tls_set(key, data, MD5_HASH, 64);
#endif

	if(tls_poll() == -1)
		tls_fail();
		
#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	if(memcmp(ans, key + 20, 16) != 0)
#else
	if(memcmp(ans, key + 0x40, 16) != 0)
#endif	
	{
		md5_fail++;
		tls_fail();		
	}
	else
	{
		md5_pass++;
		tls_pass();
	}

	free(key);
	free(data);	
}

void sha1_test_static()
{
	unsigned char keyT[64 + 20]= 
	{0x67 ,0x45 ,0x23 ,0x01 ,0xef ,0xcd ,0xab ,0x89 ,0x98 ,0xba ,0xdc ,0xfe ,0x10 ,0x32 ,0x54,0x76
	,0xc3 ,0xd2 ,0xe1 ,0xf0};
	unsigned char dataT[]= 
	{0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 
	,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30
	,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 
	,0x30 ,0x30 ,0x30 ,0x30 ,0x80 ,0x30 ,0x30 ,0x30 ,0x00 ,0x00 ,0x00 ,0x00 ,0xb8 ,0x01 ,0x00 ,0x00
	};
	unsigned char ans[20] = {0xee ,0x58 ,0xc4 ,0xe0 ,0xb7 ,0x3a ,0x63 ,0x9a ,0x2c ,0xbc ,0x01 ,0xa7 ,0xe5 ,0xb5 ,0x1a ,0x49 ,0x31 ,0xa8 ,0x02 ,0x01};	

	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = malloc(sizeof(dataT));

	memset(key, 0, sizeof(key));
	memcpy(key, keyT, sizeof(keyT));
	memcpy(data, dataT, sizeof(dataT));	

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	tls_set(key, data, MD5_SHA1, 64);
#else
	tls_set(key, data, SHA1_HASH, 64);
#endif

	if(tls_poll() == -1)
		tls_fail();

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	if(memcmp(ans, key, 20) != 0)
#else
	if(memcmp(ans, key + 0x40, 20) != 0)
#endif
	{
		sha1_fail++;
		tls_fail();		
	}
	else
	{
		sha1_pass++;
		tls_pass();
	}

	free(key);
	free(data);	
}

void md5_hmac16_test_static()
{
#define md5_hmac16_test_len 7
	unsigned char keyT[64 + 20] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f, 0x0};
	unsigned char dataT[md5_hmac16_test_len] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06};
	unsigned char ans[16] = {0x9, 0x23, 0x4b, 0xe7, 0xeb, 0xbd, 0xa8, 0x72, 0x5b, 0xdc, 0xab, 0xab, 0xbb, 0x3c, 0xa4, 0xe5};

	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = malloc(sizeof(dataT));
	memcpy(key, keyT, sizeof(keyT));
	memcpy(data, dataT, sizeof(dataT));	

	//HW
	tls_set(key, data, MD5_HMAC_16, md5_hmac16_test_len);

	if(tls_poll() == -1)
		return;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	if(memcmp(key, ans, 16) != 0)
#else
	if(memcmp(key  + 0x40, ans, 16) != 0)
#endif
	{
		md5_hmac_fail_16++;
		tls_fail();		
	}
	else
	{
		md5_hmac_pass_16++;
		tls_pass();
	}

	free(key);
	free(data);	
}

void sha1_hmac20_test_static()
{
#define sha1_hmac20_test_len  2
	unsigned char keyT[64 + 20] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,  0x0};	
	unsigned char dataT[sha1_hmac20_test_len] ={0x00,0x01};
	unsigned char ans[]={0xa5,0x86,0x61,0xba,0x4c,0xfd,0xe4,0x41,0x63,0xb7,0xde,0x28,0xe4,0x10,0x5c,0x4e
		,0x30,0xd7,0x7c,0xee};

	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = malloc(sizeof(dataT));
	memcpy(key, keyT, sizeof(keyT));
	memcpy(data, dataT, sizeof(dataT));	

	tls_set(key, data, SHA1_HMAC_20, sha1_hmac20_test_len);

	if(tls_poll() == -1)
		return;

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	if(memcmp(key , ans, 20) != 0)
#else
	if(memcmp(key + 0x40, ans, 20) != 0)
#endif
	{
		sha1_hmac_fail_20++;
		tls_fail();		
	}
	else
	{
		sha1_hmac_pass_20++;
		tls_pass();
	}

	free(key);
	free(data);	
}

#if 1
void aes_test_static()
{ 
	int i = 0;
	aesHwKey* hk;
	
	char cipherText[]={0xc6, 0xa1, 0x3b, 0x37, 0x87, 0x8f, 0x5b, 0x82, 0x6f, 0x4f,
		0x81, 0x62, 0xa1, 0xc8, 0xd8, 0x79, 0x35, 0xd9, 0xdc, 0xdb,
		0x82, 0x9f, 0xec, 0x33, 0x52, 0xe7, 0xbf, 0x10, 0xb8, 0x4b,
		0xe4, 0xa5};

	hk = (aesHwKey*)malloc(sizeof(aesHwKey));
	unsigned char* data = (unsigned char*)malloc(32);
	memset(hk, 0 , sizeof(aesHwKey));

	for (i = 0; i < 16; i++)
	{
		hk->AES_Key[i] = hk->IV[i] = i;	
	}

	for (i = 0; i < 32; i++)
	{
		data[i] = i;		
	}

	tls_set((unsigned char*) hk, data, AES128_CBC_ENCRYPT, 32);

	if(tls_poll() == -1)
		return;	

	if(memcmp(data, cipherText, 32) != 0)
	{
		aes_encrypt_fail++;
		tls_fail();

	}
	else
	{
		aes_encrypt_pass++;
		tls_pass();  	
	}	

	tls_set((unsigned char*) hk, data, AES128_CBC_DECRYPT, 32);

	if(tls_poll() == -1)
		return;

	for (i = 0; i < 32; i++)
	{
		if(data[i] != i)
		{
			tls_fail();
			aes_decrypt_fail++;
			break;
		}
	}

	if(i == 32)
	{
		aes_decrypt_pass++;
		tls_pass();  	
	}

	free(hk);
	free(data);	
}

#endif
#if 0
void aes256_test_static()
{ 
	int i = 0;
	aesHwKey* hk;
	
	char cipherText[]={0xc6, 0xa1, 0x3b, 0x37, 0x87, 0x8f, 0x5b, 0x82, 0x6f, 0x4f,
		0x81, 0x62, 0xa1, 0xc8, 0xd8, 0x79, 0x35, 0xd9, 0xdc, 0xdb,
		0x82, 0x9f, 0xec, 0x33, 0x52, 0xe7, 0xbf, 0x10, 0xb8, 0x4b,
		0xe4, 0xa5};

	//hk = (aesHwKey*)malloc(sizeof(aesHwKey));
	//unsigned char* data = (unsigned char*)malloc(48);
	hk= (aesHwKey*)0xA8020000;
	unsigned char* data = (unsigned char*)0xA8021001;
	memset(hk, 0 , sizeof(aesHwKey));

	for (i = 0; i < 16; i++)
	{
		hk->IV[i] = i;	
	}

	for (i = 0; i < 32; i++)
	{
		hk->AES_Key[i] = data[i] = i;		
	}

	
	tls_set((unsigned char*) hk, data, AES256_CBC_ENCRYPT, 32);

	if(tls_poll() == -1)
		return;	
#if 0
	if(memcmp(data, cipherText, 32) != 0)
	{
		aes_encrypt_fail++;
		tls_fail();

	}
	else
	{
		aes_encrypt_pass++;
		tls_pass();  	
	}	
#endif
	tls_set((unsigned char*) hk, data, AES256_CBC_DECRYPT, 32);

	if(tls_poll() == -1)
		return;

	for (i = 0; i < 32; i++)
	{
		if(data[i] != i)
		{
			tls_fail();
			aes_decrypt_fail++;
			break;
		}
	}

	if(i == 32)
	{
		aes_decrypt_pass++;
		tls_pass();  	
	}

	free(hk);
	free(data);	
}
#endif
#if(CONFIG_VERSION >= IC_VERSION_FP_RevA)
//Maybe have key unalignment issue if do not use ucos malloc lib
void md5_test_AnyByeAlig(char dOffset)
{
	unsigned char keyT[80] = {0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0xfe,0xdc,0xba,0x98
		,0x76,0x54,0x32,0x10};
	unsigned char dataT[]= {00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x80,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x50};	
	unsigned char ans[16] = {0xc5 ,0x6b ,0xd5 ,0x48 ,0xf ,0x6e ,0x54 ,0x13 ,0xcb ,0x62 ,0xa0 ,0xad ,0x96 ,0x66 ,0x61 ,0x3a};
	unsigned char* dataAnyT = malloc(sizeof(dataT) + 4);	
	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = dataAnyT + dOffset;

	memcpy(key, keyT, sizeof(keyT));
	memcpy(dataAnyT + dOffset, dataT, sizeof(dataT));	

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	tls_set(key, data, MD5_SHA1, 64);
#else
	tls_set(key, data, MD5_HASH, 64);
#endif

	if(tls_poll() == -1)
		tls_fail();

	if(memcmp(ans, key + 0x40, 16) != 0)
	{
		md5_fail++;
		tls_fail();		
	}
	else
	{
		md5_pass++;
		tls_pass();
	}
#if !CONFIG_CODEC_BIT_SIM
	free(key);
	free(dataAnyT);	
#endif
}

void sha1_test_AnyByeAlig(char dOffset)
{
	unsigned char keyT[64 + 20]= 
	{0x67 ,0x45 ,0x23 ,0x01 ,0xef ,0xcd ,0xab ,0x89 ,0x98 ,0xba ,0xdc ,0xfe ,0x10 ,0x32 ,0x54,0x76
	,0xc3 ,0xd2 ,0xe1 ,0xf0};
	unsigned char dataT[]= 
	{0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 
	,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30
	,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 ,0x30 
	,0x30 ,0x30 ,0x30 ,0x30 ,0x80 ,0x30 ,0x30 ,0x30 ,0x00 ,0x00 ,0x00 ,0x00 ,0xb8 ,0x01 ,0x00 ,0x00
	};
	unsigned char ans[20] = {0xee ,0x58 ,0xc4 ,0xe0 ,0xb7 ,0x3a ,0x63 ,0x9a ,0x2c ,0xbc ,0x01 ,0xa7 ,0xe5 ,0xb5 ,0x1a ,0x49 ,0x31 ,0xa8 ,0x02 ,0x01};	

	unsigned char* dataAnyT = malloc(sizeof(dataT) + 4);	
	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = dataAnyT + dOffset;
	
	memcpy(key, keyT, sizeof(keyT));
	memcpy(dataAnyT + dOffset, dataT, sizeof(dataT));	

#if CONFIG_VERSION <= IC_VERSION_DP_RevF
	tls_set(key, data, MD5_SHA1, 64);
#else
	tls_set(key, data, SHA1_HASH, 64);
#endif

	if(tls_poll() == -1)
		tls_fail();

	if(memcmp(ans, key + 0x40, 20) != 0)
	{
		sha1_fail++;
		tls_fail();		
	}
	else
	{
		sha1_pass++;
		tls_pass();
	}

	free(key);
	free(dataAnyT);	
}

void md5_hmac16_test_AnyByeAlig(char dOffset)
{
#define md5_hmac16_test_len 64
	unsigned char keyT[64 + 20] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f, 0x0};
	unsigned char dataT[md5_hmac16_test_len];
	unsigned char ans[16] = {0x2c, 0x8f, 0xa, 0x28, 0xf1, 0x15, 0x32, 0xbc, 0x1b, 0x5f, 0x88, 0x8, 0x36, 0x75, 0x6b, 0xa1};
	unsigned char* dataAnyT = malloc(sizeof(dataT) + 4);	
	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = dataAnyT + dOffset;
	int i=0;
	
	for(; i < md5_hmac16_test_len; i++)
	{
		dataT[i] = i;
	}
	memcpy(key, keyT, sizeof(keyT));	
	memcpy(dataAnyT + dOffset, dataT, sizeof(dataT));
	//HW				
	tls_set(key, data, MD5_HMAC_16, md5_hmac16_test_len);

	if(tls_poll() == -1)
		return;

	if(memcmp(key  + 0x40, ans, 16) != 0)
	{
		tls_fail();		
	}
	else
	{
		tls_pass();
	}

	free(key);
	free(dataAnyT);		
}

void sha1_hmac20_test_AnyByeAlig(char dOffset)
{
#define sha1_hmac20_test_len  64
	unsigned char keyT[64 + 20] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,  0x0};	
	unsigned char dataT[sha1_hmac20_test_len];
	unsigned char ans[]={0x70,0xe7,0xeb, 0xd1, 0x3d, 0xf2, 0xfa, 0x2, 0x5e,0xe9, 0x65, 0xc1, 0x13, 0x2a, 0x49, 0x32, 0x54, 0x8b, 0x3d,0xf5};
	unsigned char* dataAnyT = malloc(sizeof(dataT) + 4);	
	unsigned char* key = malloc(sizeof(keyT));
	unsigned char* data = dataAnyT + dOffset;
	int i = 0;
	
	for(; i < sha1_hmac20_test_len; i++)
	{
		dataT[i] = i;
	}
	memcpy(key, keyT, sizeof(keyT));	
	memcpy(dataAnyT + dOffset, dataT, sizeof(dataT));

	tls_set(key, data, SHA1_HMAC_20, sha1_hmac20_test_len);

	if(tls_poll() == -1)
		return;

	if(memcmp(key + 0x40, ans, 20) != 0)
	{
		sha1_hmac_fail_20++;
		tls_fail();		
	}
	else
	{
		sha1_hmac_pass_20++;
		tls_pass();
	}

	free(key);
	free(dataAnyT);	
}

void aes_test_AnyByeAlig(char dOffset)
{ 
	int i = 0;
	aesHwKey* hk;	

	char cipherText[]={0xc6, 0xa1, 0x3b, 0x37, 0x87, 0x8f, 0x5b, 0x82, 0x6f, 0x4f,
		0x81, 0x62, 0xa1, 0xc8, 0xd8, 0x79, 0x35, 0xd9, 0xdc, 0xdb,
		0x82, 0x9f, 0xec, 0x33, 0x52, 0xe7, 0xbf, 0x10, 0xb8, 0x4b,
		0xe4, 0xa5};
	char* dataT;

	hk = (aesHwKey*)malloc(sizeof(aesHwKey));
	memset(hk, 0 , sizeof(aesHwKey));
	unsigned char* data = (unsigned char*)malloc(32 + 4);

	dataT = data + dOffset;

	for (i = 0; i < 16; i++)
	{
		hk->AES_Key[i] = hk->IV[i] = i;		
	}

	for (i = 0; i < 32; i++)
	{
		dataT[i] = i;		
	}

	tls_set((unsigned char*) hk, dataT, AES128_CBC_ENCRYPT, 32);

	if(tls_poll() == -1)
		return;	

	if(memcmp(dataT, cipherText, 32) != 0)
	{
		aes_encrypt_fail++;
		tls_fail();
	}
	else
	{
		aes_encrypt_pass++;
		tls_pass();  	
	}	

	tls_set((unsigned char*) hk, dataT, AES128_CBC_DECRYPT, 32);

	if(tls_poll() == -1)
		return;

	for (i = 0; i < 32; i++)
	{
		if(dataT[i] != i)
		{
			tls_fail();
			aes_decrypt_fail++;
			break;
		}
	}

	if(i == 32)
	{
		aes_decrypt_pass++;
		tls_pass();  	
	}

	free(hk);
	free(data);
}


void md5_any_rand_test()
{
	int len = 1 + rand() % 1500;
	md5_context md5, md5H;
	int i = 0, j =0;
	unsigned char buf[16] = {0};
	unsigned char buf1[16] = {0};
	unsigned char *p = malloc(len);

	for (i = 0; i < len; i++)
		p[i] = rand() % 256;

	md5_starts(&md5H);
	md5_updateH( &md5H, p, len);
	md5_finishH( &md5H, buf);

	md5_starts(&md5);
	md5_update( &md5, p, len);
	md5_finish( &md5, buf1);

	if (memcmp(buf1, buf, 16) != 0)
	{
		md5_fail++;
		tls_fail();
	}
	else
	{
		md5_pass++;
		tls_pass();
	}
	free(p);
}


void sha1_any_rand_test()
{
	sha1_context sha1H;
	sha1_context sha1;

	int j = 0, len = 1 + rand()%1500;
	unsigned char outputHW[20] = {0};
	unsigned char outputSW[20] = {0};
	int offset = 0;

	unsigned char *p = malloc(len);

	for (j = 0 ; j < len; j++)
	{
		p[j] = rand() % 256;
	}

	for(offset = 0; (offset < 4) && (offset < len); offset++)
	{
		sha1_startsH( &sha1H );
		sha1_updateH( &sha1H, p + offset, len - offset);
		sha1_finishH( &sha1H, outputHW );

		sha1_starts( &sha1 );
		sha1_update( &sha1, p + offset, len - offset);
		sha1_finish( &sha1, outputSW );

		if (memcmp(outputSW, outputHW, 20) != 0)
		{
			tls_fail();
			sha1_fail++;
		}
		else
		{
			tls_pass();
			sha1_pass++;
		}
	}

	free(p);
}

void md5_hmac_any_rand_test()
{
	int dsize = 1 + rand() % 1500;
	int keySize = 1 + rand() % 100;
	unsigned char key[100] = {0};   
	unsigned char keySW[100] = {0};

	unsigned char *p = malloc(dsize);

	int i = 0;

	for ( i = 0; i < keySize;i++ )
	{
		key[i] = keySW[i] = rand() % 256;
	}

	for ( i = 0; i < dsize;i++ )
	{
		p[i] = rand() % 256;
	}

	md5_hmac(key, keySize, p, dsize, key);
	md5_hmacSWHash(keySW, keySize, p, dsize, keySW);

	if (memcmp(key, keySW, keySize) != 0)
	{
		tls_fail();
		md5_hmac_fail_16++;
	}
	else
	{
		tls_pass();
		md5_hmac_pass_16++;
	}

	free(p);
}

void sha1_hmac_any_rand_test()
{
	unsigned char key[100] = {0};
	unsigned char keySW[100] = {0};
	int keySize = 1 + rand() % 100;
	int dsize = 1 + rand()%1500;
	unsigned char ans1[20] = {0};
	unsigned char ans2[20] = {0};
	unsigned char *p = malloc(dsize);
	int i = 0;
	int offset = 0;

	for ( i = 0; i < keySize;i++ )
	{
		key[i] = keySW[i] = rand() % 256;
	}

	for ( i = 0; i < dsize;i++ )
	{
		p[i] = rand() % 256;
	}

	for(offset = 0; (offset < 4) && (offset < dsize); offset++)
	{
		sha1_hmac(key, keySize, p + offset, dsize - offset, ans1); 
		sha1_hmacSW(keySW, keySize, p + offset, dsize - offset, ans2);   

		if (memcmp(ans1, ans2, 20) != 0)
		{
			tls_fail();
			sha1_hmac_fail_20++;
		}
		else
		{
			tls_pass();
			sha1_hmac_pass_20++;
		}
	}

	free(p);
}

#if 0
void aes_any_rand_test()
{    
	aes_context asHW, asSW;
	unsigned char aesKey[32], aeskeyHW[32], aeskeySW[32];
	unsigned char iv[16],ivHW[16], ivSW[16];
	unsigned char *dataHW;
	unsigned char *dataHWInitPos;
	unsigned char *dataSW;
	unsigned char *dataSWInitPos;
	unsigned int keySize;
	unsigned int offset = 0;

	int len = 1 + rand()% 2000;
	len = 32;
	int i = 0;

	keySize = rand()%3;

	switch(keySize)
	{
	case 0:
		keySize = 16;//128
		break;
	case 1:
		keySize = 24;//192
		break;
		case 2:
			keySize = 32;//256
			break;
	}

	//len must greater than 20
	i = checkPadLen(16, len);
	if (i)
		len = len + i + 1;

	dataHWInitPos = malloc(len + 4);
	dataSWInitPos = malloc(len + 4);

	dataHW = dataHWInitPos;
	dataSW = dataSWInitPos;

	for(offset = 0; (offset < 4) && (offset < len); offset++)
	{
		dataHW = dataHWInitPos + offset;
		dataSW = dataSWInitPos + offset;


		//aes key and iv
		for (i = 0; i< keySize; i++)
		{
			aesKey[i] = aeskeyHW[i]= aeskeySW[i] = rand()%256;        
		}

		for (i = 0; i< 16; i++)
		{
			iv[i] = ivSW[i]	= ivHW[i]= rand() %256;
		}

		for (i = 0; i< len; i++)
		{
			dataHW[i] = dataSW[i] =rand() %256;
		}


		aes_setkey_enc_sw(&asSW, aeskeySW, keySize*8);
		aes_crypt_cbc_sw(&asSW, AES_ENCRYPT, len, ivSW, dataSW, dataSW);

		//Hw
		memcpy(&asHW, aeskeyHW, keySize);
		aes_crypt_cbc(&asHW, keySize, AES_ENCRYPT, len, ivHW, dataHW, dataHW);

		if(memcmp(dataHW, dataSW, len) != 0)
		{
			tls_fail();
			aes_encrypt_fail++;
		}
		else
		{
			tls_pass();
			aes_encrypt_pass++;
		}


		//aes key and iv
		for (i = 0; i< keySize; i++)
		{
			aeskeyHW[i]= aeskeySW[i] = aesKey[i];
		}

		for (i = 0; i< 16; i++)
		{
			ivSW[i]	= ivHW[i]= iv[i];
		}

		aes_setkey_dec_sw(&asSW, aeskeySW, keySize*8);   
		aes_crypt_cbc_sw(&asSW, AES_DECRYPT, len, ivSW, dataSW, dataSW);

		//Hw
		memcpy(&asHW, aeskeyHW, keySize);
		aes_crypt_cbc(&asHW, keySize, AES_DECRYPT, len, ivHW, dataHW, dataHW); 

		if(memcmp(dataHW, dataSW, len) != 0)
		{
			tls_fail();
			aes_decrypt_fail++;
		}
		else
		{
			tls_pass();
			aes_decrypt_pass++;
		}
	}

	free(dataHWInitPos);	
	free(dataSWInitPos);
}
#endif
#endif

#if 1
void checkBoundaryHash()
{
	int len;
	int  i = 0;
	int offset = 0;	
	int loop=0;
	char sel = 0;	
	unsigned char key[64 + 20 + 20]; //sha1 and md5 result is at offset 64, 20 is check region 
	unsigned char* data;
	char alog[] ={SHA1_HASH, MD5_HASH, SHA1_HMAC_20, MD5_HMAC_16};		
	sel = alog[3];
	
	while(1)
	{
		loop++;
		offset = rand()%0x10;
		
		len = 64* (1 + (rand() %30)) + 0x0f + 20; //0xf is offset 20 is check region 
		data = malloc(len);
		memset(key, 0, sizeof(key));
		memset(data, 0, len);
		
		for(i = 0; i < 20; i++)
		{
			key[i] = i;		
		}
		
		for(i = 0; i < len - 20 - offset; i++)
		{
			data[i] = i;		
		}
		
		tls_set(key, data + offset, sel, (len - 0x0 - 20));
		
		if(tls_poll() == -1)
			return;
		
		if((sel == MD5_HASH) || (sel == MD5_HMAC_16))
		{
			for(i = 80; i < 20; i++)
			{
				if(key[i] !=0)
					tls_fail();					
			}
		}
		else
		{
			for(i = 84; i < 20; i++)
			{
				if(key[i] !=0)
					tls_fail();					
			}			
		}
		
		for(i = len - 20 - offset; i < len; i++)
		{
			if(data[i] != 0)
				tls_fail();	
				
		}
		
		free(data);
	}
}
#endif

void checkBoundaryAES()
{
	int len;
	int  i = 0;
	int offset = 0;	
	int loop=0;
	char sel = 0;	
	unsigned char key[32 + 16 + 20]; //key is 32 , iv is 16, 20 is check region 
	unsigned char* data;
	char alog[] ={AES128_CBC_ENCRYPT, AES128_CBC_DECRYPT,AES192_CBC_ENCRYPT, AES192_CBC_DECRYPT,AES192_CBC_ENCRYPT, AES256_CBC_DECRYPT};		
	sel = alog[4];
	
	while(1)
	{		
		loop++;
		offset = rand()%0x10;
		
		int blk = (1 + (rand() %30));
		
		len = 64* blk + 0x0f + 20; //oxf is offset 20 is check region 
		data = malloc(len);
		memset(key, 0, sizeof(key));
		memset(data, 0, len);
		
		for(i = 0; i < 48; i++)
		{
			key[i] = i;		
		}
		
		for(i = offset; i < 64* blk ; i++)
		{
			data[i] = i;		
		}
		
		tls_set(key, data + offset, sel, 64* blk );//len muset 32n
		
		if(tls_poll() == -1)
			return;
		
			for(i = 48; i < 20; i++)
			{
				if(key[i] !=0)
					tls_fail();					
			}
		

		
		for(i = offset + (64* blk) ; i < len; i++)
		{
			if(data[i] != 0)
			{
				tls_fail();	
				break;
			}				
		}
		
		if(i == len)
			tls_pass();	
		
		free(data);
	}

}

void tls_test(void)
{
 int offset = 0;	
// #if (CONFIG_VERSION  <= IC_VERSION_DP_RevF) 
	bsp_tls_init();
//#endif
disablecache();


//md5_test_static();
//aes256_test_static();	
//aes_test_static();
//checkBoundaryHash();
//checkBoundaryAES();
#if 0
	while(1)
	{

	    md5_rand_test();
	    md5_hmac_rand_test();    
	    sha1_rand_test();    
	    sha1_hmac_rand_test();
	    aes_rand_test();
	}
#endif

#if 1
	while(1)
	{
		md5_test_static();
		//sha1_test_static();
		//md5_hmac16_test_static();
		//sha1_hmac20_test_static();
		//aes_test_static();
	}
#endif    

	
	
	/*May be have key unalignment issue when not use ucos lib*/
	while(1)
	{
		md5_hmac16_test_AnyByeAlig(offset%4);
		md5_test_AnyByeAlig(offset%4);
		sha1_hmac20_test_AnyByeAlig(offset%4);
		sha1_test_AnyByeAlig(offset%4);
		aes_test_AnyByeAlig(offset%4);
		offset++;
	};
}
#endif
