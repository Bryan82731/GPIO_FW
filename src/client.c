#include <stdlib.h>
#include <string.h>
#include "tcp.h"
#include "rtskt.h"
#include "client.h"
#include "wsman-names.h"
#include "dash.h"
#include "lib.h"
#include "smbus.h"
#include "eventlog.h"
#include "bsp.h"

extern char change_sub;
extern SubEvent *subhead;
char dest[16],uri[64],source[16];
char *EData,*buf;
plist *EventStartOff=NULL;
SubEvent *c_subhead;
int port;
extern OS_EVENT *AlertQ;

XMLElt _EventAlertIndication[]=
{
    {"IndicationTime",""},
    {"MessageID","PET111",1},
    {"Message",""},
    {"SystemName",source},
    {0,0}
};
void c_insertlen(send_list *sendlist, INT16U headerlen)
{
    send_list *current = sendlist;
    INT16U total = 0;
    INT8U *ptr;

    while (current)
    {
        //assert(sendlist->len < ETH_PAYLOAD_LEN);
        total += current->len;
        current = current->next;
    }
    
    total -= headerlen;

    ptr = strstr(sendlist->addr + PKT_HDR_SIZE,"xxxxx");
    sprintf(ptr,"%5d",total);

    *(ptr+5) = '\r';
    //remove the null character
}
char  c_sendpacket(send_list *sendlist,RTSkt* cs)
{
    send_list *freehead;
    PKT* txPkt;
    char status=0;

    while (sendlist)
    {
        txPkt = (PKT*)setPkt(sendlist->addr,sendlist->len);

        freehead = sendlist;
        sendlist = sendlist->next;
        free(freehead);
        status = rtSktSend_F(cs, txPkt, 0);
        if (status==SKTDISCONNECT)
            return -1;

    }
    return 0;

}

send_list* c_new_send_list()
{
    send_list *ptr = malloc(sizeof(send_list));
    memset(ptr, 0 , sizeof(send_list));
    ptr->addr = malloc(ETH_PKT_LEN);
    memset(ptr->addr, 0 , PKT_HDR_SIZE+1);
    return ptr;
}
void parser_subinfo(SubEvent *subinfo)
{
    char *ptr,*ptr1,*ptr_end;
    char ch;
    int ret;
    port=80;
    strcpy(dest,"127.0.0.1");
    ptr=strstr(subinfo->notifyto,"http://");
    if(ptr)
    {
      ptr=ptr+strlen("http://");
    
      ptr_end=strchr(ptr,'/');
      if(ptr_end)
      {
        ch=*ptr_end;
        *ptr_end = NULLCHR;
        ptr1=strchr(ptr,':');
        if(ptr1)
          *ptr1=0x20;
        sscanf(ptr,"%s %d",dest,&port);
        *ptr_end=ch;
        if(ptr1)
          *ptr1=':';
        strcpy(uri,ptr_end+1);  
      }
      else 
      { 
      	strcpy(dest,ptr);    
        strcpy(uri,ptr);
      }  
    }      
}

send_list* send_heartbeat(SubEvent *subinfo)
{
    INT8U *buf;
    INT16U tmplen;

    send_list *current,*head;

    current = c_new_send_list();
    head = current;

    if (current->addr == NULL)
    {
        free(head);
        return NULL;
    }

    buf = current->addr+PKT_HDR_SIZE;

    sprintf(buf,"POST /%s HTTP/1.1\r\n",uri);
    sprintf(buf+strlen(buf),"Host: %s:%d\r\n",dest,port);
    sprintf(buf+strlen(buf),"Accept: */*\r\n");
    sprintf(buf+strlen(buf),"Content-Type: application/soap+xml;charset=UTF-8\r\n");
    sprintf(buf+strlen(buf),"Content-Length: xxxxx\r\n\r\n");

    tmplen = strlen(buf);

    sprintf(buf+strlen(buf),"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsman=\"http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd\" xmlns:n1=\"http://schemas.xmlsoap.org/wbmem/wsman/1/wsman\">");
    sprintf(buf+strlen(buf),"<s:Header>");
    sprintf(buf+strlen(buf),"<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Heartbeat</wsa:Action>");
    sprintf(buf+strlen(buf),"<wsa:To s:mustUnderstand=\"true\">%s</wsa:To>",subinfo->notifyto);
    sprintf(buf+strlen(buf),"<wsman:ResourceURI s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Heartbeat</wsman:ResourceURI>");

    sprintf(buf+strlen(buf),"<wsa:MessageID s:mustUnderstand=\"true\">");
    sprintf(buf+strlen(buf),"uuid:00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</wsa:MessageID>");
    sprintf(buf+strlen(buf),"<wsa:ReplyTo>");
    sprintf(buf+strlen(buf),"<wsa:Address>%s</wsa:Address>",WSA_TO_ANONYMOUS);
    sprintf(buf+strlen(buf),"</wsa:ReplyTo>");
    sprintf(buf+strlen(buf),"</s:Header><s:Body><n1:Heartbeat/>");
    sprintf(buf+strlen(buf),"</s:Body></s:Envelope>");

    current->len = strlen(buf);
    // DEBUGMSG(DASH_DEBUG,"%d get_buf=%s\n",current->len,current->addr);

    c_insertlen(head, tmplen);
    DEBUGMSG(CLIENT_DEBUG,"33\n");

    return head;
}
void EgenXML(char *buf)
{
    plist *p;
    p=EventStartOff;
    while (p)
    {

        sprintf(buf+strlen(buf),"<p:");
        sprintf(buf+strlen(buf),p->eltname);
        sprintf(buf+strlen(buf),">");
        sprintf(buf+strlen(buf),p->data);
        sprintf(buf+strlen(buf),"</p:");
        sprintf(buf+strlen(buf),p->eltname);
        sprintf(buf+strlen(buf),">");

        p=p->next;

    }

    return;

}

send_list* send_event(SubEvent *subinfo)
{
    INT8U *buf;
    INT16U tmplen;

    send_list *current,*head;

    current = c_new_send_list();
    head = current;

    if (current->addr == NULL)
    {
        free(head);
        return NULL;
    }

    buf = current->addr+PKT_HDR_SIZE;

    sprintf(buf,"POST /%s HTTP/1.1\r\n",uri);
    sprintf(buf+strlen(buf),"Host: %s:%d\r\n",dest,port);
    sprintf(buf+strlen(buf),"Accept: */*\r\n");
    sprintf(buf+strlen(buf),"Content-Type: application/soap+xml;charset=UTF-8\r\n");
    sprintf(buf+strlen(buf),"Content-Length: xxxxx\r\n\r\n");

    tmplen = strlen(buf);

    sprintf(buf+strlen(buf),"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" xmlns:wsman=\"http://schemas.dmtf.org/wbem/wsman/1/wsman.xsd\">");
    sprintf(buf+strlen(buf),"<s:Header>");
    sprintf(buf+strlen(buf),"<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Heartbeat</wsa:Action>");

    sprintf(buf+strlen(buf),"<wsa:To s:mustUnderstand=\"true\">%s</wsa:To>",subinfo->notifyto);

    sprintf(buf+strlen(buf),"<wsman:ResourceURI s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/wbmem/wsman/1/wsman/Heartbeat</wsman:ResourceURI>");

    sprintf(buf+strlen(buf),"<wsa:MessageID s:mustUnderstand=\"true\">");
    sprintf(buf+strlen(buf),"uuid:00000000-0000-0000-0000-000000000000");
    sprintf(buf+strlen(buf),"</wsa:MessageID>");
    sprintf(buf+strlen(buf),"<wsa:ReplyTo>");
    sprintf(buf+strlen(buf),"<wsa:Address>%s</wsa:Address>",WSA_TO_ANONYMOUS);
    sprintf(buf+strlen(buf),"</wsa:ReplyTo>");
    sprintf(buf+strlen(buf),"</s:Header><s:Body><p:CIM_AlertIndication xmlns:p=\"http://schemas.dmtf.org/wbem/wscim/1/cim-schema/2/CIM_AlertIndication\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\">");
    //  current->len = strlen(buf);

    EgenXML(buf);

    sprintf(buf+strlen(buf),"</p:CIM_AlertIndication></s:Body></s:Envelope>");

    current->len = strlen(buf);
    // DEBUGMSG(CLIENT_DEBUG, "%d get_buf=%s\n",current->len,current->addr);

    c_insertlen(head, tmplen);

    return head;
}
plist* ReleaseEvent(plist *ptr)
{
    plist *freeptr;
    while (ptr)
    {
        freeptr = ptr;
        ptr = ptr->next;
        free(freeptr);
    }
    if (EData)
        free(EData);
    return NULL;
}

plist *EventDefaultInstance(XMLElt *p,INT8U *pdata, eventdata *ed) // Alert Indicator 
{
    INT8U i=0, offset = 0;
    plist *head=NULL,*current=NULL,*prev=NULL;
    //struct _elist *ptr;
    //ptr = eventGet();

    while (p[i].eltname)
    {
        current=malloc(sizeof(plist));
        memset(current, 0 , sizeof(plist));
        current->eltname=p[i].eltname;
        current->data=p[i].data;

        if (strcmp(current->eltname,"Message")==0)

	//printf("[RTK] EventDefaultInstance\n");

	#if CONFIG_FWLOG_ENABLED	
	if(ed->Event_Sensor_Type == 0x7F && ed->Event_Type == 0x1F ){				
      		offset = getFwLogParser(ed, pdata);
	}else
	#endif
            offset = getEventLogParser(ed, pdata);

        if (offset)
        {
            current->data = pdata;
            pdata = pdata + offset + 1;
            offset = 0;
        }

        current->key=p[i].key;
        current->used=p[i].used;
        current->idx=p[i].idx;

        if (head == NULL)
            head = current;
        else
            prev->next = current;

        prev = current;
        i++;
    }

    i=0;

    return head;

}
void c_subscribe(SubEvent *NewSub)
{
    SubEvent *tmp=c_subhead;
    SubEvent *now=NULL;
    while (tmp)
    {
        now=tmp;
        tmp=tmp->next;
    }
    NewSub->next=NULL;
    if (now==NULL)
        c_subhead=NewSub;
    else
        now->next = NewSub;

}
void ReleaseSub()
{
    SubEvent *subptr,*freeptr;

    subptr=c_subhead;
    while (subptr)
    {
        freeptr = subptr;
        subptr = subptr->next;
        free(freeptr);
    }
    c_subhead=NULL;
}

void clientTask1(void *data)
{
    send_list *sendlist = NULL;
    RTSkt* s = rtSkt_F(IPv4, IP_PROTO_TCP);
    PKT* rxPkt;
    int xip[4],filter;
    unsigned char ip[4];

    int status=0;
    unsigned char err = 0;
    SubEvent *subinfo,*c_subinfo;
    eventdata *evt_ptr=NULL;
    eventdata *tmpeptr=NULL;
    unsigned int address;
    OS_CPU_SR  cpu_sr = 0;
    if(subhead!=NULL)
      change_sub=1;

    while (OS_TRUE)
    {
        address=getIPAddress_F(eth0);
        sprintf(source,"%d.%d.%d.%d",address&0xff,(address>>8) & 0xff,(address>>16)&0xff,address>>24);

        if (change_sub==1)
        {
            OS_ENTER_CRITICAL();
            ReleaseSub();
            subinfo=subhead;
            while (subinfo)
            {
                c_subinfo = malloc(sizeof(SubEvent));
                memset(c_subinfo, 0, sizeof(SubEvent));
                strcpy(c_subinfo->subid,subinfo->subid);
                strcpy(c_subinfo->notifyto,subinfo->notifyto);
                strcpy(c_subinfo->query,subinfo->query);
                c_subscribe(c_subinfo);
                subinfo=subinfo->next;
            }
            change_sub=0;
            OS_EXIT_CRITICAL();
        }


        subinfo = c_subhead;

        if (subinfo == NULL)
        {
            OSTimeDly(OS_TICKS_PER_SEC);
            continue;
        }

        if (EventStartOff)
            EventStartOff=ReleaseEvent(EventStartOff);

        evt_ptr = (eventdata *) OSQPend(AlertQ, OS_TICKS_PER_SEC*10 ,&err);

        if (evt_ptr)
        {
            EData=malloc(1024);
            memset(EData, 0 , 1024);
            EventStartOff=EventDefaultInstance((void *) &_EventAlertIndication,EData,evt_ptr);
        }


        while (subinfo)
        {
            if (evt_ptr)
            {
                tmpeptr = evt_ptr;
                if (strstr(subinfo->query,"FilterCollection:"))
                {
                    sscanf(subinfo->query,"FilterCollection:%d",&filter);
                    if (filter!=tmpeptr->logtype && filter != 5)
                        tmpeptr=NULL;

                }
            }

            parser_subinfo(subinfo);

            sscanf(dest,"%d.%d.%d.%d", &xip[0], &xip[1], &xip[2], &xip[3]);
            ip[0]=xip[0];
            ip[1]=xip[1];
            ip[2]=xip[2];
            ip[3]=xip[3];
            if (s==NULL)
                s = rtSkt_F(IPv4, IP_PROTO_TCP);
#if CONFIG_VERSION >= IC_VERSION_DP_RevF
            rtSktEnableIBPortFilter_F(s);
#endif
            if (rtSktConnect_F(s, ip, port) == -1)
            {
                rtSktClose_F(s);
                OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
                OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
                s->rxSem=0;
                s->txSem=0;
                if (s)
                    free(s);
                s=NULL;
                subinfo=subinfo->next;
                continue;
            }

            if (tmpeptr == NULL)
            {
                if (err == OS_ERR_TIMEOUT){
					
                    
			#ifdef CONFIG_DISABLE_HEARBEAT		
                   sendlist=NULL; //Disable send_heartbeat for "HP NoteBook" project , 20140318.briankuo                   
                   //printf("[RTK] Send Heart beat?\n");
			#else
			sendlist=send_heartbeat(subinfo);
			#endif
                }
                else
                    sendlist=NULL;
            }
            else{
		#ifdef CONFIG_DISABLE_HEARBEAT				
                if(EventStartOff){
                	sendlist=send_event(subinfo);
                }else{
                	//printf("[RTK] Send Heart beat by event\n");
                }
		#else
			sendlist=send_event(subinfo);
		#endif
            }

            if (sendlist)
                status = c_sendpacket(sendlist,s);

            while (s)
            {
                while ((rxPkt = rtSktRx_F(s,100,&status)) == 0)
                {
                    if (status == SKTDISCONNECT || status == OS_ERR_TIMEOUT)
                    {
                        rtSktClose_F(s);
                        OSSemDel(s->rxSem, OS_DEL_ALWAYS, &err);
                        OSSemDel(s->txSem, OS_DEL_ALWAYS, &err);
                        s->rxSem=0;
                        s->txSem=0;
                        if (s)
                            free(s);
                        s=NULL;
                        break;
                    }

                }

                if (status == SKTDISCONNECT || status == OS_ERR_TIMEOUT)
                    break;

                freePkt_F(rxPkt);
            }

            subinfo=subinfo->next;

        }

    }

}
