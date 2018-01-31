#include "rlx/rlx_types.h"
#include "rlx/rlx_cpu.h"
#include "log.h"

unsigned int logidx = 0;

struct logdata_s *logdata;

void loginit()
{
    logdata = malloc(sizeof(struct logdata_s)*MAX_LOG_DATA);

}

void logend()
{
   free(logdata);
}
void logme(INT32U line, INT32U data1, INT32U data2, INT32U  data3)
{
  INT32U cpu_sr;

  OS_ENTER_CRITICAL();

  logdata[logidx].data0 = line;
  logdata[logidx].data1 = data1;
  logdata[logidx].data2 = data2;
  logdata[logidx].data3 = data3;
  logidx++;

  if(logidx == MAX_LOG_DATA)
    logidx = 0;

  OS_EXIT_CRITICAL();
}

void
dumplog(void)
{
  int i;
  int idx = logidx;
  INT32U cpu_sr;


  OS_ENTER_CRITICAL();

  for(i = 0; i < MAX_LOG_DATA; i++)
    {
      if (logdata[idx].data0 != 0)
        printf("line %d: data1=0x%lx, data2=0x%lx, data3=0x%lx\n", 
             logdata[idx].data0,
             logdata[idx].data1,
             logdata[idx].data2,
             logdata[idx].data3);

      idx += i;
      if (idx == MAX_LOG_DATA)
        idx = 0;
    }

  while(1);
}
