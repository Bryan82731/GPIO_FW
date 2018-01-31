#ifndef  _RLX_TYPES_H_
#define  _RLX_TYPES_H_

/*
********************************************************************************
*                                      DATA TYPES
*                                 (Compiler Specific)
********************************************************************************
*/


typedef           void           VOID;
typedef unsigned  char           BOOLEAN;
typedef unsigned  char           INT8U;        /* Unsigned  8-bit quantity */
typedef signed    char           INT8S;        /* Signed    8-bit quantity */
typedef unsigned  short          INT16U;       /* Unsigned 16-bit quantity */
typedef signed    short          INT16S;       /* Signed   16-bit quantity */
typedef unsigned  int            INT32U;       /* Unsigned 32-bit quantity */
typedef signed    int            INT32S;       /* Signed   32-bit quantity */
typedef float                    FP32;
typedef double                   FP64;
typedef unsigned  int            OS_REG;
typedef unsigned  int            OS_STK;
/* Each stack entry is 32 bits wide */
typedef unsigned  int  volatile  OS_CPU_SR;

/* The CPU Status Word is 32-bits wide.
   This variable MUST be volatile for proper operation.
*/

/* Definiton of null pointer */
#ifndef NULL
#define NULL  ((void *)0)
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true (!false)
#endif

#endif

