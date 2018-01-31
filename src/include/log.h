#ifndef __LOG_H_
#define  __LOG_H_

#define MAX_LOG_DATA 1000

struct logdata_s
{
  unsigned int data0;
  unsigned int data1;
  unsigned int data2;
  unsigned int data3;
};

#if 0

#define LOGME(d1,d2,d3) \
  logme(__LINE__, (d1), (d2), (d3))
#else
#define LOGME(d1,d2,d3) 
#endif


#if 0
#define CHECK(cond) \
do {\
  if(!(cond))  \
    dumplog(); \
} while(0)
#else
#define CHECK(cond)
#endif

#endif
