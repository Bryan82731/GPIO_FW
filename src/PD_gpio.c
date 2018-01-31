#include "lib.h"
#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <bsp_cfg.h>

#include "OOBMAC_COMMON.h"
#include "usb.h"
#inlcude "PD_message.h"

typedef struct __pd_source_port {
	bool hw_goodcrc_tx;
	bool hw_goodcrc_rx;
	bool support_gotomin;
	//bool usb_comm_capable;
	INT8U pd_rev;
	INT16U msg_id;
	UINT state;
	bool waiting_goodcrc;
	//struct pd_source_cap source_caps[MAX_NR_SOURCE_CAPS];
	int nr_source_caps;
	//enum usb_spec usb_spec;
	//struct sink_ps ps_reqs[MAX_NR_SINK_PS_REQS];
	//int nr_ps;
	//int active_ps;
	//int port;
	//u8 last_sent_msg;
	//bool last_msg_ctrl;
	
	//struct workqueue_struct *rx_wq;
	//struct work_struct rx_work;
	//spinlock_t rx_lock;
	//struct list_head rx_list;
	//struct hrtimer tx_timer;
	//int (*tx)(int port, void *tx_priv, void *buf, int len,
	//				enum sop_type sop_type);
	//void *tx_priv;
}pd_source_port,*ppd_source_port;

ppd_source_port PD_DFP;

void roll_msg_id(ppd_source_port port)
{
	if ( port->msg_id > 7)
		port->msg_id = 0;
	else
		port->msg_id += 1;
}

#define MAKE_SOURCE_HEADER(port, header, msg, nu_objs) \
do { \
	header->type = msg; \
	header->data_role = PD_DATA_ROLE_DFP; \
	header->revision = port->pd_rev; \
	header->power_role = PD_POWER_ROLE_SOURCE; \
	header->id = roll_msg_id(port); \
	header->nr_objs = nu_objs; \
	header->extended = PD_MSG_NOT_EXTENDED; \
} while (0)



void Init_PD_Source_Port(ppd_source_port port)
{
	port =(struct __pd_source_port*) mallocx(sizeof(pd_source_port));
	pPD_DFP->msg_id = 0;
	pPD_DFP->pd_rev = PD_REVISION_3;
}

void PD_Send_Source_Cap()
{

}

void PD_Negotiation_Start()
{
	struct pd_msg_header *pPD_MSG_HDR;
	MAKE_SOURCE_HEADER(PD_DFP, pPD_MSG_HDR, PD_DMSG_SOURCE_CAP, objs)
	/* 1. Send Source Capabilities to sink */
	
}
