

#ifndef _LANGUAGE_H
#define _LANGUAGE_H

#include "sys/dpdef.h"




//#ifdef CONFIG_MULTILANGUAGE_ENABLED

/*
INT8U keyworksEn[2][10] _ATTRIBUTE_LANGUAGE_EN ={
	"Reltek","Welcome"
};



INT8U keyworks[2][10] _ATTRIBUTE_LANGUAGE_2={
	"·ç¬R","Åwªï"
};*/


//extern INT8U s_realtek_en[] ; 
//extern INT8U s_realtek_2[] ; 
//extern INT8U s_welcome_en[]; 
//extern INT8U s_welcome_2[] ; 
//extern INT8U  s_welstr_en[] ;
//extern INT8U  s_welstr_2[] ;
//extern INT8U  s_gigaEthControl_en[];
//extern INT8U  s_gigaEthControl_2[] ;
//extern INT8U  s_dashtech_en[];
//extern INT8U  s_dashtech_2[] ;


#define KEYWORD_DEFINE_EXTERN(para) extern INT8U *s_##para;


KEYWORD_DEFINE_EXTERN(dashtech)
KEYWORD_DEFINE_EXTERN(computerSys)
KEYWORD_DEFINE_EXTERN(opSystem)
KEYWORD_DEFINE_EXTERN(softwIdentity)	
KEYWORD_DEFINE_EXTERN(systemInfo)
KEYWORD_DEFINE_EXTERN(bios)
KEYWORD_DEFINE_EXTERN(cpu)
KEYWORD_DEFINE_EXTERN(memDevice)
KEYWORD_DEFINE_EXTERN(fan)
KEYWORD_DEFINE_EXTERN(sensorstring)

KEYWORD_DEFINE_EXTERN(asset)
KEYWORD_DEFINE_EXTERN(remoteControl)
KEYWORD_DEFINE_EXTERN(eventLog)
KEYWORD_DEFINE_EXTERN(networkSetting)	
KEYWORD_DEFINE_EXTERN(serviceSetting)
KEYWORD_DEFINE_EXTERN(mailWakeup)
KEYWORD_DEFINE_EXTERN(mailAccount)
KEYWORD_DEFINE_EXTERN(matchCondition)
KEYWORD_DEFINE_EXTERN(about)
KEYWORD_DEFINE_EXTERN(realtek)

KEYWORD_DEFINE_EXTERN(welcome)
KEYWORD_DEFINE_EXTERN(welstr)
KEYWORD_DEFINE_EXTERN(gigaEthControl)
KEYWORD_DEFINE_EXTERN(realtekCop)


//extern INT8U *s_realtek;
//extern INT8U *s_welcome;
//extern INT8U  *s_welstr;
//extern INT8U  *s_gigaEthControl;
//extern INT8U  *s_dashtech;

//void buildKeyworkTable();
	
//#endif

#endif

