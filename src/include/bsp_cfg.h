#ifndef  _BSP_CFG_H_ALL
#define  _BSP_CFG_H_ALL

#include "sys/autoconf.h"
#include "sys/dpdef.h"



//DP Revf
//Hw Feature:Port Filter, Support Single/Dual IP mode but do not fixed DHCP issue
//Rom Code: lib.c arp.c ip.c tcp.c rtskt.c

#if CONFIG_VERSION >= IC_VERSION_FP_RevA
#include "bsp_cfg_fp_revA.h"
#elif CONFIG_VERSION >= IC_VERSION_EP_RevA
#include "bsp_cfg_ep.h"
#elif CONFIG_VERSION == IC_VERSION_DP_RevF
#include "bsp_cfg_dp_revF.h"
#elif CONFIG_VERSION < IC_VERSION_DP_RevF
#include "bsp_cfg_dp.h"
#endif

#endif