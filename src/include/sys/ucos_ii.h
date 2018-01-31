/*
********************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : uCOS_II.H
* By      : Jean J. Labrosse
* Version : V2.86
*
* LICENSING TERMS:
* ---------------
*   uC/OS-II is provided in source form for FREE evaluation, for educational use or for peaceful research.
* If you plan on using  uC/OS-II  in a commercial product you need to contact Micriµm to properly license
* its use in your product. We provide ALL the source code for your convenience and to help you experience
* uC/OS-II.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a
* licensing fee.
********************************************************************************
*/

#ifndef   OS_uCOS_II_H
#define   OS_uCOS_II_H

/*
********************************************************************************
*                                          uC/OS-II VERSION NUMBER
********************************************************************************
*/

#define  OS_VERSION                 286u
/* Version of uC/OS-II (Vx.yy mult. by 100)    */

/*
********************************************************************************
*                                           INCLUDE HEADER FILES
********************************************************************************
*/

#include <rlx/rlx_types.h>

/*
********************************************************************************
*                                             MISCELLANEOUS
********************************************************************************
*/

#ifdef   OS_GLOBALS
#define  OS_EXT
#else
#define  OS_EXT  extern
#endif

#ifndef  OS_FALSE
#define  OS_FALSE                     0u
#endif

#ifndef  OS_TRUE
#define  OS_TRUE                      1u
#endif

#define  OS_ASCII_NUL          (INT8U)0

#define  OS_PRIO_SELF              0xFFu
/* Indicate SELF priority                      */

#define OS_EVENT_NAME_SIZE       16
/* Determine the size of the name of a Sem, Mutex, Mbox or Q      */
#define OS_FLAG_NAME_SIZE        16
/*     Determine the size of the name of an event flag group      */
#define OS_TASK_NAME_SIZE        16
/*     Determine the size of a task name                          */

#define OS_MAX_EVENTS            32
/* Max. number of event control blocks in your application        */
#define OS_MAX_FLAGS              5
/* Max. number of Event Flag Groups    in your application        */
#define OS_MAX_MEM_PART          10
/* Max. number of memory partitions                               */
#define OS_MAX_QS                 8
/* Max. number of queue control blocks in your application        */
#define OS_MAX_TASKS             16
/* Max. number of tasks in your application, MUST be >= 2         */
#define OS_MSG_MAX               32
/*     Maximum number of messages                                 */

#define OS_LOWEST_PRIO           32
/* Defines the lowest priority that can be assigned ...           */

#define OS_TICKS_PER_SEC        100
/* Set the number of ticks in one second                          */

#define  OS_EVENT_TBL_SIZE ((OS_LOWEST_PRIO) / 8 + 1)
/* Size of event table                         */
#define  OS_RDY_TBL_SIZE   ((OS_LOWEST_PRIO) / 8 + 1)
/* Size of ready table                         */


#define  OS_TCB_RESERVED        ((OS_TCB *)1)

/*
********************************************************************************
*                              TASK STATUS (Bit definition for OSTCBStat)
********************************************************************************
*/
#define  OS_STAT_RDY               0x00u
/* Ready to run                                            */
#define  OS_STAT_SEM               0x01u
/* Pending on semaphore                                    */
#define  OS_STAT_MBOX              0x02u
/* Pending on mailbox                                      */
#define  OS_STAT_Q                 0x04u
/* Pending on queue                                        */
#define  OS_STAT_SUSPEND           0x08u
/* Task is suspended                                       */
#define  OS_STAT_MUTEX             0x10u
/* Pending on mutual exclusion semaphore                   */
#define  OS_STAT_FLAG              0x20u
/* Pending on event flag group                             */
#define  OS_STAT_MULTI             0x80u
/* Pending on multiple events                              */

#define  OS_STAT_PEND_ANY         (OS_STAT_SEM | OS_STAT_MBOX | OS_STAT_Q | OS_STAT_MUTEX | OS_STAT_FLAG)

/*
********************************************************************************
*                           TASK PEND STATUS (Status codes for OSTCBStatPend)
********************************************************************************
*/
#define  OS_STAT_PEND_OK              0u
/* Pending status OK, not pending, or pending complete     */
#define  OS_STAT_PEND_TO              1u
/* Pending timed out                                       */
#define  OS_STAT_PEND_ABORT           2u
/* Pending aborted                                         */

/*
********************************************************************************
*                                        OS_EVENT types
********************************************************************************
*/
#define  OS_EVENT_TYPE_UNUSED         0u
#define  OS_EVENT_TYPE_MBOX           1u
#define  OS_EVENT_TYPE_Q              2u
#define  OS_EVENT_TYPE_SEM            3u
#define  OS_EVENT_TYPE_MUTEX          4u
#define  OS_EVENT_TYPE_FLAG           5u

#define  OS_TMR_TYPE                100u
/* Used to identify Timers ...                             */
/* ... (Must be different value than OS_EVENT_TYPE_xxx)    */

/*
********************************************************************************
*                                         EVENT FLAGS
********************************************************************************
*/
#define  OS_FLAG_WAIT_CLR_ALL         0u
/* Wait for ALL    the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_AND         0u

#define  OS_FLAG_WAIT_CLR_ANY         1u
/* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define  OS_FLAG_WAIT_CLR_OR          1u

#define  OS_FLAG_WAIT_SET_ALL         2u
/* Wait for ALL    the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_AND         2u

#define  OS_FLAG_WAIT_SET_ANY         3u
/* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define  OS_FLAG_WAIT_SET_OR          3u


#define  OS_FLAG_CONSUME           0x80u
/* Consume the flags if condition(s) satisfied             */


#define  OS_FLAG_CLR                  0u
#define  OS_FLAG_SET                  1u

/*
********************************************************************************
*                                   Values for OSTickStepState
*
* Note(s): This feature is used by uC/OS-View.
********************************************************************************
*/

#define  OS_TICK_STEP_DIS             0u
/* Stepping is disabled, tick runs as mormal               */
#define  OS_TICK_STEP_WAIT            1u
/* Waiting for uC/OS-View to set OSTickStepState to _ONCE  */
#define  OS_TICK_STEP_ONCE            2u
/* Process tick once and wait for next cmd from uC/OS-View */

/*
********************************************************************************
*       Possible values for 'opt' argument of OSSemDel(), OSMboxDel(), OSQDel() and OSMutexDel()
********************************************************************************
*/
#define  OS_DEL_NO_PEND               0u
#define  OS_DEL_ALWAYS                1u

/*
********************************************************************************
*                                        OS???Pend() OPTIONS
*
* These #defines are used to establish the options for OS???PendAbort().
********************************************************************************
*/
#define  OS_PEND_OPT_NONE             0u
/* NO option selected                                      */
#define  OS_PEND_OPT_BROADCAST        1u
/* Broadcast action to ALL tasks waiting                   */

/*
********************************************************************************
*                                     OS???PostOpt() OPTIONS
*
* These #defines are used to establish the options for OSMboxPostOpt() and OSQPostOpt().
********************************************************************************
*/
#define  OS_POST_OPT_NONE          0x00u
/* NO option selected                                      */
#define  OS_POST_OPT_BROADCAST     0x01u
/* Broadcast message to ALL tasks waiting                  */
#define  OS_POST_OPT_FRONT         0x02u
/* Post to highest priority task waiting                   */
#define  OS_POST_OPT_NO_SCHED      0x04u
/* Do not call the scheduler if this option is selected    */

/*
********************************************************************************
*                                 TASK OPTIONS (see OSTaskCreateExt())
********************************************************************************
*/
#define  OS_TASK_OPT_NONE        0x0000u
/* NO option selected                                      */
#define  OS_TASK_OPT_STK_CHK     0x0001u
/* Enable stack checking for the task                      */
#define  OS_TASK_OPT_STK_CLR     0x0002u
/* Clear the stack when the task is create                 */
#define  OS_TASK_OPT_SAVE_FP     0x0004u
/* Save the contents of any floating-point registers       */

/*
********************************************************************************
*                                             ERROR CODES
********************************************************************************
*/
#define OS_ERR_NONE                   0u

#define OS_ERR_EVENT_TYPE             1u
#define OS_ERR_PEND_ISR               2u
#define OS_ERR_POST_NULL_PTR          3u
#define OS_ERR_PEVENT_NULL            4u
#define OS_ERR_POST_ISR               5u
#define OS_ERR_QUERY_ISR              6u
#define OS_ERR_INVALID_OPT            7u
#define OS_ERR_PDATA_NULL             9u

#define OS_ERR_TIMEOUT               10u
#define OS_ERR_EVENT_NAME_TOO_LONG   11u
#define OS_ERR_PNAME_NULL            12u
#define OS_ERR_PEND_LOCKED           13u
#define OS_ERR_PEND_ABORT            14u
#define OS_ERR_DEL_ISR               15u
#define OS_ERR_CREATE_ISR            16u
#define OS_ERR_NAME_GET_ISR          17u
#define OS_ERR_NAME_SET_ISR          18u

#define OS_ERR_MBOX_FULL             20u

#define OS_ERR_Q_FULL                30u
#define OS_ERR_Q_EMPTY               31u

#define OS_ERR_PRIO_EXIST            40u
#define OS_ERR_PRIO                  41u
#define OS_ERR_PRIO_INVALID          42u

#define OS_ERR_SEM_OVF               50u

#define OS_ERR_TASK_CREATE_ISR       60u
#define OS_ERR_TASK_DEL              61u
#define OS_ERR_TASK_DEL_IDLE         62u
#define OS_ERR_TASK_DEL_REQ          63u
#define OS_ERR_TASK_DEL_ISR          64u
#define OS_ERR_TASK_NAME_TOO_LONG    65u
#define OS_ERR_TASK_NO_MORE_TCB      66u
#define OS_ERR_TASK_NOT_EXIST        67u
#define OS_ERR_TASK_NOT_SUSPENDED    68u
#define OS_ERR_TASK_OPT              69u
#define OS_ERR_TASK_RESUME_PRIO      70u
#define OS_ERR_TASK_SUSPEND_IDLE     71u
#define OS_ERR_TASK_SUSPEND_PRIO     72u
#define OS_ERR_TASK_WAITING          73u

#define OS_ERR_TIME_NOT_DLY          80u
#define OS_ERR_TIME_INVALID_MINUTES  81u
#define OS_ERR_TIME_INVALID_SECONDS  82u
#define OS_ERR_TIME_INVALID_MS       83u
#define OS_ERR_TIME_ZERO_DLY         84u
#define OS_ERR_TIME_DLY_ISR          85u

#define OS_ERR_MEM_INVALID_PART      90u
#define OS_ERR_MEM_INVALID_BLKS      91u
#define OS_ERR_MEM_INVALID_SIZE      92u
#define OS_ERR_MEM_NO_FREE_BLKS      93u
#define OS_ERR_MEM_FULL              94u
#define OS_ERR_MEM_INVALID_PBLK      95u
#define OS_ERR_MEM_INVALID_PMEM      96u
#define OS_ERR_MEM_INVALID_PDATA     97u
#define OS_ERR_MEM_INVALID_ADDR      98u
#define OS_ERR_MEM_NAME_TOO_LONG     99u

#define OS_ERR_NOT_MUTEX_OWNER      100u

#define OS_ERR_FLAG_INVALID_PGRP    110u
#define OS_ERR_FLAG_WAIT_TYPE       111u
#define OS_ERR_FLAG_NOT_RDY         112u
#define OS_ERR_FLAG_INVALID_OPT     113u
#define OS_ERR_FLAG_GRP_DEPLETED    114u
#define OS_ERR_FLAG_NAME_TOO_LONG   115u

#define OS_ERR_PIP_LOWER            120u

#define OS_ERR_TMR_INVALID_DLY      130u
#define OS_ERR_TMR_INVALID_PERIOD   131u
#define OS_ERR_TMR_INVALID_OPT      132u
#define OS_ERR_TMR_INVALID_NAME     133u
#define OS_ERR_TMR_NON_AVAIL        134u
#define OS_ERR_TMR_INACTIVE         135u
#define OS_ERR_TMR_INVALID_DEST     136u
#define OS_ERR_TMR_INVALID_TYPE     137u
#define OS_ERR_TMR_INVALID          138u
#define OS_ERR_TMR_ISR              139u
#define OS_ERR_TMR_NAME_TOO_LONG    140u
#define OS_ERR_TMR_INVALID_STATE    141u
#define OS_ERR_TMR_STOPPED          142u
#define OS_ERR_TMR_NO_CALLBACK      143u


/*
********************************************************************************
*                                          EVENT CONTROL BLOCK
********************************************************************************
*/

typedef struct os_event {
    INT8U    OSEventType;
    /* Type of event control block (see OS_EVENT_TYPE_xxxx)    */
    void    *OSEventPtr;
    /* Pointer to message or queue structure                   */
    INT16U   OSEventCnt;
    /* Semaphore Count (not used if other EVENT type)          */
    INT8U    OSEventGrp;
    /* Group corresponding to tasks waiting for event to occur */
    INT8U    OSEventTbl[OS_EVENT_TBL_SIZE];
    /* List of tasks waiting for event to occur                */

    INT8U    OSEventName[OS_EVENT_NAME_SIZE];
} OS_EVENT;


/*
********************************************************************************
*                                       EVENT FLAGS CONTROL BLOCK
********************************************************************************
*/


typedef  INT16U   OS_FLAGS;

typedef struct os_flag_grp {
    /* Event Flag Group                                        */
    INT8U         OSFlagType;
    /* Should be set to OS_EVENT_TYPE_FLAG                     */
    void         *OSFlagWaitList;
    /* Pointer to first NODE of task waiting on event flag     */
    OS_FLAGS      OSFlagFlags;
    /* 8, 16 or 32 bit flags                                   */
    INT8U         OSFlagName[OS_FLAG_NAME_SIZE];
} OS_FLAG_GRP;



typedef struct os_flag_node {
    /* Event Flag Wait List Node                               */
    void         *OSFlagNodeNext;
    /* Pointer to next     NODE in wait list                   */
    void         *OSFlagNodePrev;
    /* Pointer to previous NODE in wait list                   */
    void         *OSFlagNodeTCB;
    /* Pointer to TCB of waiting task                          */
    void         *OSFlagNodeFlagGrp;
    /* Pointer to Event Flag Group                             */
    OS_FLAGS      OSFlagNodeFlags;
    /* Event flag to wait on                                   */
    INT8U         OSFlagNodeWaitType;
    /* Type of wait:                                           */

    /*      OS_FLAG_WAIT_AND                                   */

    /*      OS_FLAG_WAIT_ALL                                   */

    /*      OS_FLAG_WAIT_OR                                    */

    /*      OS_FLAG_WAIT_ANY                                   */
} OS_FLAG_NODE;

/*
********************************************************************************
*                                          MESSAGE QUEUE DATA
********************************************************************************
*/

typedef struct os_q {
    /* QUEUE CONTROL BLOCK                                         */
    struct os_q   *OSQPtr;
    /* Link to next queue control block in list of free blocks     */
    void         **OSQStart;
    /* Pointer to start of queue data                              */
    void         **OSQEnd;
    /* Pointer to end   of queue data                              */
    void         **OSQIn;
    /* Pointer to where next message will be inserted  in   the Q  */
    void         **OSQOut;
    /* Pointer to where next message will be extracted from the Q  */
    INT16U         OSQSize;
    /* Size of queue (maximum number of entries)                   */
    INT16U         OSQEntries;
    /* Current number of entries in the queue                      */
} OS_Q;


typedef struct os_q_data {
    void          *OSMsg;
    /* Pointer to next message to be extracted from queue          */
    INT16U         OSNMsgs;
    /* Number of messages in message queue                         */
    INT16U         OSQSize;
    /* Size of message queue                                       */
    INT8U          OSEventTbl[OS_EVENT_TBL_SIZE];
    /* List of tasks waiting for event to occur         */
    INT8U          OSEventGrp;
    /* Group corresponding to tasks waiting for event to occur     */
} OS_Q_DATA;

/*
********************************************************************************
*                                           SEMAPHORE DATA
********************************************************************************
*/

typedef struct os_sem_data {
    INT16U  OSCnt;
    /* Semaphore count                                         */
    INT8U   OSEventTbl[OS_EVENT_TBL_SIZE];
    /* List of tasks waiting for event to occur                */
    INT8U   OSEventGrp;
    /* Group corresponding to tasks waiting for event to occur */
} OS_SEM_DATA;

/*
********************************************************************************
*                                            TASK STACK DATA
********************************************************************************
*/

typedef struct os_stk_data {
    INT32U  OSFree;
    /* Number of free bytes on the stack                            */
    INT32U  OSUsed;
    /* Number of bytes used on the stack                            */
} OS_STK_DATA;

/*
********************************************************************************
*                                          TASK CONTROL BLOCK
********************************************************************************
*/

typedef struct os_tcb {
    OS_STK          *OSTCBStkPtr;
    /* Pointer to current top of stack                         */

    void            *OSTCBExtPtr;
    /* Pointer to user definable data for TCB extension        */
    OS_STK          *OSTCBStkBottom;
    /* Pointer to bottom of stack                              */
    INT32U           OSTCBStkSize;
    /* Size of task stack (in number of stack elements)        */
    INT16U           OSTCBOpt;
    /* Task options as passed by OSTaskCreateExt()             */
    INT16U           OSTCBId;
    /* Task ID (0..65535)                                      */

    struct os_tcb   *OSTCBNext;
    /* Pointer to next     TCB in the TCB list                 */
    struct os_tcb   *OSTCBPrev;
    /* Pointer to previous TCB in the TCB list                 */

    OS_EVENT        *OSTCBEventPtr;
    /* Pointer to          event control block                 */

    void            *OSTCBMsg;
    /* Message received from OSMboxPost() or OSQPost()         */

    OS_FLAG_NODE    *OSTCBFlagNode;
    /* Pointer to event flag node                              */
    OS_FLAGS         OSTCBFlagsRdy;
    /* Event flags that made task ready to run                 */

    INT16U           OSTCBDly;
    /* Nbr ticks to delay task or, timeout waiting for event   */
    INT8U            OSTCBStat;
    /* Task      status                                        */
    INT8U            OSTCBStatPend;
    /* Task PEND status                                        */
    INT8U            OSTCBPrio;
    /* Task priority (0 == highest)                            */

    INT8U            OSTCBX;
    /* Bit position in group  corresponding to task priority   */
    INT8U            OSTCBY;
    /* Index into ready table corresponding to task priority   */
    INT8U            OSTCBBitX;
    /* Bit mask to access bit position in ready table          */
    INT8U            OSTCBBitY;
    /* Bit mask to access bit position in ready group          */

    INT8U            OSTCBDelReq;
    /* Indicates whether a task needs to delete itself         */

    INT32U           OSTCBCtxSwCtr;
    /* Number of time the task was switched in                 */
    INT32U           OSTCBCyclesTot;
    /* Total number of clock cycles the task has been running  */
    INT32U           OSTCBCyclesStart;
    /* Snapshot of cycle counter at start of task resumption   */
    OS_STK          *OSTCBStkBase;
    /* Pointer to the beginning of the task stack              */
    INT32U           OSTCBStkUsed;
    /* Number of bytes used from the stack                     */

    INT8U            OSTCBTaskName[OS_TASK_NAME_SIZE];
} OS_TCB;

/*
********************************************************************************
*                                          FUNCTION PROTOTYPES
*                                     (Target Independent Functions)
********************************************************************************
*/

/*
********************************************************************************
*                                            MISCELLANEOUS
********************************************************************************
*/

INT8U         OSEventNameGet          (OS_EVENT        *pevent,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          OSEventNameSet          (OS_EVENT        *pevent,
                                       INT8U           *pname,
                                       INT8U           *perr);


/*
********************************************************************************
*                                         EVENT FLAGS MANAGEMENT
********************************************************************************
*/


OS_FLAGS      OSFlagAccept            (OS_FLAG_GRP     *pgrp,
                                       OS_FLAGS         flags,
                                       INT8U            wait_type,
                                       INT8U           *perr);

OS_FLAG_GRP  *OSFlagCreate            (OS_FLAGS         flags,
                                       INT8U            *perr);

OS_FLAG_GRP  *OSFlagDel               (OS_FLAG_GRP     *pgrp,
                                       INT8U            opt,
                                       INT8U           *perr);

INT8U         OSFlagNameGet           (OS_FLAG_GRP     *pgrp,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          OSFlagNameSet           (OS_FLAG_GRP     *pgrp,
                                       INT8U           *pname,
                                       INT8U           *perr);

OS_FLAGS      OSFlagPend              (OS_FLAG_GRP     *pgrp,
                                       OS_FLAGS         flags,
                                       INT8U            wait_type,
                                       INT16U           timeout,
                                       INT8U           *perr);

OS_FLAGS      OSFlagPendGetFlagsRdy   (void);
OS_FLAGS      OSFlagPost              (OS_FLAG_GRP     *pgrp,
                                       OS_FLAGS         flags,
                                       INT8U            opt,
                                       INT8U           *perr);

OS_FLAGS      OSFlagQuery             (OS_FLAG_GRP     *pgrp,
                                       INT8U           *perr);

/*
********************************************************************************
*                                         MESSAGE QUEUE MANAGEMENT
********************************************************************************
*/


void         *OSQAccept               (OS_EVENT        *pevent,
                                       INT8U           *perr);

OS_EVENT     *OSQCreate               (void           **start,
                                       INT16U           size);

OS_EVENT     *OSQDel                  (OS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);

INT8U         OSQFlush                (OS_EVENT        *pevent);

void         *OSQPend                 (OS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

INT8U         OSQPendAbort            (OS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);

INT8U         OSQPost                 (OS_EVENT        *pevent,
                                       void            *pmsg);

INT8U         OSQPostFront            (OS_EVENT        *pevent,
                                       void            *pmsg);

INT8U         OSQPostOpt              (OS_EVENT        *pevent,
                                       void            *pmsg,
                                       INT8U            opt);

INT8U         OSQQuery                (OS_EVENT        *pevent,
                                       OS_Q_DATA       *p_q_data);


/*
********************************************************************************
*                                          SEMAPHORE MANAGEMENT
********************************************************************************
*/

INT16U        OSSemAccept             (OS_EVENT        *pevent);

OS_EVENT     *OSSemCreate             (INT16U           cnt);

OS_EVENT     *OSSemDel                (OS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);

void          OSSemPend               (OS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

INT8U         OSSemPendAbort          (OS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);

INT8U         OSSemPost               (OS_EVENT        *pevent);

INT8U         OSSemQuery              (OS_EVENT        *pevent,
                                       OS_SEM_DATA     *p_sem_data);

void          OSSemSet                (OS_EVENT        *pevent,
                                       INT16U           cnt,
                                       INT8U           *perr);

//#define HOOK

#ifdef HOOK
OS_EVENT     *OSSemCreate2             (INT16U           cnt);
INT8U         OSTaskCreate2            (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       OS_STK          *ptos,
                                       INT8U            prio);

INT8U         OSTaskCreateExt2         (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       OS_STK          *ptos,
                                       INT8U            prio,
                                       INT16U           id,
                                       OS_STK          *pbos,
                                       INT32U           stk_size,
                                       void            *pext,
                                       INT16U           opt);
#endif

/*
********************************************************************************
*                                            TASK MANAGEMENT
********************************************************************************
*/
INT8U         OSTaskChangePrio        (INT8U            oldprio,
                                       INT8U            newprio);

INT8U         OSTaskCreate            (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       OS_STK          *ptos,
                                       INT8U            prio);

INT8U         OSTaskCreateExt         (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       OS_STK          *ptos,
                                       INT8U            prio,
                                       INT16U           id,
                                       OS_STK          *pbos,
                                       INT32U           stk_size,
                                       void            *pext,
                                       INT16U           opt);

INT8U         OSTaskDel               (INT8U            prio);
INT8U         OSTaskDelReq            (INT8U            prio);

INT8U         OSTaskNameGet           (INT8U            prio,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          OSTaskNameSet           (INT8U            prio,
                                       INT8U           *pname,
                                       INT8U           *perr);

INT8U         OSTaskResume            (INT8U            prio);
INT8U         OSTaskSuspend           (INT8U            prio);

INT8U         OSTaskStkChk            (INT8U            prio,
                                       OS_STK_DATA     *p_stk_data);

INT8U         OSTaskQuery             (INT8U            prio,
                                       OS_TCB          *p_task_data);

/*
********************************************************************************
*                                            TIME MANAGEMENT
********************************************************************************
*/

void          OSTimeDly               (INT16U           ticks);
INT8U         OSTimeDlyHMSM           (INT8U            hours,
                                       INT8U            minutes,
                                       INT8U            seconds,
                                       INT16U           milli);
INT8U         OSTimeDlyResume         (INT8U            prio);
INT32U        OSTimeGet               (void);
void          OSTimeSet               (INT32U           ticks);
void          OSTimeTick              (void);


/*
********************************************************************************
*                                             MISCELLANEOUS
********************************************************************************
*/

void          OSInit                  (void);

void          OSStart                 (void);

void          OSStatInit              (void);

#endif
