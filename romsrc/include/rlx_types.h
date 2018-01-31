#ifndef  _RLX_TYPES_H_
#define  _RLX_TYPES_H_

/*
*****************************************************************************************
*                                      DATA TYPES
*                                 (Compiler Specific)
*****************************************************************************************
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
typedef unsigned  int            OS_STK;       /* Each stack entry is 32 bits wide */
typedef unsigned  int  volatile  OS_CPU_SR;    /* The CPU Status Word is 32-bits wide.  
                                                  This variable MUST be volatile for proper 
                                                  operation.  Refer to             */
                                               /* os_cpu_a.s for more details.     */


typedef INT32U RlxIrqIdType;
/* type of IRQ handler */
typedef void (*RlxIrqHandlerType) (void);

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

/* 
 * Allow creation of procedure-like macros that are a single statement,
 * and must be followed by a semi-colon
 */
#define RT_MACRO_START do {
#define RT_MACRO_END   } while (0)


/* Macro to remove compalation warning of unused parameter */
#define RT_UNUSED_PARAM(_type, _name)       \
RT_MACRO_START                              \
  _type _tmp1 = (_name);                    \
  _type _tmp2 = _tmp1;                      \
  _tmp1 = _tmp2;                            \
RT_MACRO_END

#endif

