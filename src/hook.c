#include <stdio.h>
#include "bsp.h"
#include "debug.h"

#define MAX_MEM_RECORDS 500
//#define Last_MEM_RECORDS 40

typedef struct
{
    char FN[16];//alloc file name
    char fFN[16];//free file name
    unsigned int line;//alloc line number
    unsigned int fline;//free line number
    unsigned int* addr;//allocated address
    unsigned short size;//alocated size
    char t;//reserved
    char used;//memory used flag
}_MemRecord;

#ifdef CONFIG_MEMDEBUG
_MemRecord MemRecord[MAX_MEM_RECORDS];
//_MemRecord LastMemRecord[Last_MEM_RECORDS];
int RMemIdx = 0;

void *dbgmalloc(int size, char *fn, int line)
{
    void *ptr;
    int i = 0;
    char found = 0;
    ptr = mallocx(size);
    //if(size > 800)
    // DEBUGMSG(HOOK_DEBUG,"%s-%d allocated %d@%p\n", fn,line, size, ptr);
    if (strlen(fn) < 15 + 6)
    {

        //detack memory full version
        for (i = 0 ;i < MAX_MEM_RECORDS && i < RMemIdx ; i++)
        {
            if (MemRecord[i].used && MemRecord[i].addr == ptr)
            {
                DEBUGMSG(HOOK_DEBUG,"~~~~~ Double alloc addr%x size:%d  ~~~~~~\n", ptr, size);
            }

            if (MemRecord[i].addr == ptr)
            {
                found = 1;
                break;
            }
        }

        if (i == MAX_MEM_RECORDS)
        {
            DEBUGMSG(HOOK_DEBUG,"==========Memory Record not enough==========\n");
        }

        if (found)
        {
            memset(MemRecord[i].FN, 0, 15);
            sprintf(MemRecord[i].FN, "%s", fn + 6);
            MemRecord[i].used = 1;
            MemRecord[i].line = line;
            MemRecord[i].addr = ptr;
        }
        else
        {
            memset(MemRecord[RMemIdx].FN, 0, 15);
            sprintf(MemRecord[RMemIdx].FN, "%s", fn + 6);
            MemRecord[RMemIdx].used = 1;
            MemRecord[RMemIdx].line = line;
            MemRecord[RMemIdx].addr = ptr;
            MemRecord[RMemIdx].size = size;

            RMemIdx ++;

        }
    }




    if (ptr == 0)
        DEBUGMSG(HOOK_DEBUG,"^^^^^^^^^Memory not enough^^^^^^^^^^^^\n");

    return ptr;
}
void dbgfree(void *ptr, char *fn, int line)
{
//   if(parfind(ptr) == 1536)
//       DEBUGMSG(HOOK_DEBUG,"%s-%d free %p\n", fn, line, ptr);
    int i = 0;

    for (i = 0 ;i < MAX_MEM_RECORDS ; i++)
    {
        if (MemRecord[i].addr != ptr)
            continue;
        if (!MemRecord[i].used)
            continue;
#if 0
        memset(LastMemRecord[LAstMemIdx].FN, 0, 15);
        sprintf(LastMemRecord[LAstMemIdx].FN, "%s", fn);
        LastMemRecord[LAstMemIdx].line = line;
        LastMemRecord[LAstMemIdx].addr = ptr;
        LAstMemIdx++;
        if (LAstMemIdx >= Last_MEM_RECORDS )
            LAstMemIdx = 0;
#endif

        memset(MemRecord[i].fFN, 0, 16);
        sprintf(MemRecord[i].fFN, "%s", fn + 6);
        MemRecord[i].fline = line;
        MemRecord[i].used = 0;
        break;
    }

    if (i == MAX_MEM_RECORDS)
    {
        for (i = 0 ;i < MAX_MEM_RECORDS ; i++)
        {
            if (MemRecord[i].addr == ptr)
            {
                DEBUGMSG(HOOK_DEBUG,"double free idx is %i\n", i);
                break;
            }
        }
        DEBUGMSG(HOOK_DEBUG,"########Memory be double free at F:%s L:%i A:%x\n", fn, line, ptr);
    }

    freex(ptr);
}
#endif


#if 0
OS_EVENT     *OSSemCreate  (INT16U  cnt)
{
    static INT8U semcount = 0;
    OS_EVENT *get;

    get = OSSemCreate2(cnt);
    if ( get == NULL)
        DEBUGMSG(HOOK_DEBUG,"Semaphore created failed \n");
    else
        DEBUGMSG(HOOK_DEBUG,"Sem %d is created\n", semcount++);

    return get;

}
#endif

#if 0
INT8U  OSTaskCreate   (void (*task)(void *p_arg), void *p_arg, OS_STK *ptos, INT8U prio)
{
    INT8U status;

    status = OSTaskCreate2(task, p_arg, ptos, prio);
    if (status != OS_ERR_NONE)
        DEBUGMSG(HOOK_DEBUG,"Status is %d Prio is %d\n",status, prio);
    else if (prio != 17)
        DEBUGMSG(HOOK_DEBUG,"Task with Prio %d is created\n", prio);

}

INT8U  OSTaskCreateExt(void(*task)(void *p_arg), void  *p_arg, OS_STK *ptos, INT8U prio, INT16U id, OS_STK *pbos, INT32U stk_size, void *pext, INT16U opt)
{
    INT8U status;
    status = OSTaskCreateExt2(task, p_arg, ptos, prio, id, pbos, stk_size, pext, opt);
    if (status != OS_ERR_NONE)
        DEBUGMSG(HOOK_DEBUG,"Status is %d Prio is %d\n",status, prio);
    else if (prio != 17)
        DEBUGMSG(HOOK_DEBUG,"Task with Prio %d is created\n", prio);
}
patch_event()
{
    INT8U count  = 0;
    INT32U *ptr = (INT32U *) 0x8021f000;
    INT32U *next = (INT32U *) 0x8021f72c;
    *ptr = (INT32U) next;
    //free space for use

    //extend to 32 + 16
    while (count++ < 16)
    {
        ptr = next ;
        *(ptr + 1) = ptr + 8;
        next = ptr + 8;
        *(ptr + 4) = 0x3f;

    }

    *(ptr + 1) = 0x0;
}
#endif
