#include "ucos_ii.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

OS_EVENT *DummyMbox;
OS_EVENT *DummyMutex;

void dummy_std_call()
{
   INT8U *ptr =malloc(1024);
   ptr = mempcpy(ptr , (void *) 0x80100000, 1024); 
   ptr = memccpy(ptr, (void *) 0x80100000, 0, 1024);
   puts("Test\n"); 
   bcopy(ptr, (void *) 0x80100000, 1024);
   bzero(ptr, 1024);

}

void dummy_call()
{
    void *pmsg;
    INT8U err, temp;
    OS_MBOX_DATA mbox_data;
    OS_MUTEX_DATA mutex_data;
    BOOLEAN test;
    INT8U testx[16];

    urldecode(pmsg, "TEST=", testx);
    DummyMbox = OSMboxCreate( (void *) 0 );
    pmsg = OSMboxAccept(DummyMbox);

    err = OSMboxQuery(DummyMbox, &mbox_data);
    err = OSMboxPost(DummyMbox, (void *) &temp);
    pmsg = OSMboxPend(DummyMbox, 10 , &err);
    err = OSMboxPostOpt(DummyMbox, (void *) &temp, OS_POST_OPT_BROADCAST);
    temp = OSMboxPendAbort(DummyMbox, OS_PEND_OPT_BROADCAST, &err);
    DummyMbox = OSMboxDel(DummyMbox, OS_DEL_ALWAYS, &err);

    DummyMutex = OSMutexCreate(20, &err);
    test = OSMutexAccept(DummyMutex, &err);

    err = OSMutexPost(DummyMutex);
    OSMutexPend(DummyMutex, 0 , &err);
    
    err = OSMutexQuery(DummyMutex, &mutex_data);

    DummyMutex = OSMutexDel(DummyMutex, OS_DEL_ALWAYS, &err);

    dummy_std_call();

    write_pattern(16, testx);

    loginit();
}
