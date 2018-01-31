#include <stdlib.h>
#include <string.h>
//#include
//
#include "bsp.h"
#include "language.h"
#include "lib.h"


// Description : How to add a new keyword

// 1. Add keyword at language.c
//	1.1 Use Marco "KEYWORD_DEFINE" to define a new keyword parameter  
//    1.2  Add a new item at "Enum keywordIdx"
//	1.3 Add a item at "setLanguageType"
//    1.4 Add a new case at "buildKeyworkTable_foreign"
//
// 2. Add new extern define at language.h
//
// 3. Rule of  Keyword 
//	3.1. '\n' is used to separate keywords , any Keywords could not include '\n'.  
//   3.2. Space is ok.    

//=============================================================
#define LANGUAGE1_BASE_ADDR LANGUAGE_ADDRESS
#define LANGUAGE2_BASE_ADDR (LANGUAGE_ADDRESS + 0x8000)
#define TAG_E_START_LEN 3
#define TAG_F_START_LEN 3
#define TAG_E_END_LEN 4
#define TAG_F_END_LEN 4
#define TAG_E_START "<E>"
#define TAG_E_END "</E>"
#define TAG_F_START "<F>"
#define TAG_F_END "</F>"
//=============================================================
void writeflash(INT32U flashaddr,INT8U *data, INT32U length) ;
//=============================================================

#define KEYWORD_DEFINE_foregin(para)\ 
if(s_##para##_f){\
	s_##para = s_##para##_f;\
}else{\
	s_##para = s_##para##_en;\
}
	
#define KEYWORD_DEFINE_en(para) s_##para = s_##para##_en;
#define KEYWORD_DEFINE_enum(para) s_##para##_idx, 
#define KEYWORD_DEFINE(para,english,foreign) INT8U *s_##para;\
											INT8U *s_##para##_f;\
											INT8U s_##para##_en[] _ATTRIBUTE_LANGUAGE_EN = {english};\
											INT8U s_##para##_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n<E>"english"</E><F>"foreign"</F>"}; 

//INT8U s_##para##_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n"foreign}; 

#if 0
#define KEYWORD_DEFINE_reSetforegin(para)\
case s_##para##_idx: \
	s_##para##_f = sAddr;\
break;
#else
#define KEYWORD_DEFINE_reSetforegin(para)\
else if(memcmp(s_##para##_en ,engStart ,  keyword_len) == 0){\
	if(strlen(s_##para##_en) == keyword_len){\
		s_##para##_f = fgStart;\		
	}\
}\

//
//
	
#endif

//----------------------------------------------------------------------
//INT8U s_realtek_en[] _ATTRIBUTE_LANGUAGE_EN = {"\nReltek"}; 
//INT8U s_realtek_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n瑞昱"}; 
//INT8U *s_realtek;
//INT8U *s_realtek_f;
//----------------------------------------------------------------------
//INT8U s_welcome_en[] _ATTRIBUTE_LANGUAGE_EN = {"\nWelcome"}; 
//INT8U s_welcome_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n歡迎"}; 
//INT8U *s_welcome;
//INT8U *s_welcome_f;
//----------------------------------------------------------------------
//INT8U  s_welstr_en[] _ATTRIBUTE_LANGUAGE_EN = {"\nWelcome to use Realtek Gigabit"};
//INT8U  s_welstr_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n歡迎使用瑞昱科技"};
//INT8U  *s_welstr;
//INT8U  *s_welstr_f;
//----------------------------------------------------------------------
//INT8U  s_gigaEthControl_en[] _ATTRIBUTE_LANGUAGE_EN = {"\nGigabit Ethernet Controller"};
//INT8U  s_gigaEthControl_2[] _ATTRIBUTE_LANGUAGE_2 = {"\n超高速乙太網路"};
//INT8U  *s_gigaEthControl;
//INT8U  *s_gigaEthControl_f;
//----------------------------------------------------------------------

//Flash End -----------------------------------------------------------------
//!!Add New Item Here !!Add New Item Here!!Add New Item Here!!Add New Item Here

#if 0
KEYWORD_DEFINE(realtekCop,"Realtek Semiconductor Corp.\n","瑞昱半導體\n")
KEYWORD_DEFINE(gigaEthControl,"Gigabit Ethernet Controller\n","超高速乙太網路\n")
KEYWORD_DEFINE(welstr,"Welcome to use Realtek Gigabit\n","歡迎使用瑞昱科技\n")
KEYWORD_DEFINE(welcome,"Welcome\n","歡迎\n")

KEYWORD_DEFINE(realtek,"Reltek\n","瑞昱\n")
KEYWORD_DEFINE(about,"About\n","關於\n")
KEYWORD_DEFINE(matchCondition,"Match Condition\n","Match Condition\n")
KEYWORD_DEFINE(mailAccount,"Mail Account\n","Mail Account\n")
KEYWORD_DEFINE(mailWakeup,"Mail Wake Up\n","Mail Wakeup\n")
KEYWORD_DEFINE(serviceSetting,"Service setting\n","Service Setting\n")
KEYWORD_DEFINE(networkSetting,"Network setting\n","Network Setting\n")
KEYWORD_DEFINE(eventLog,"Event Log\n","Event Log\n")
KEYWORD_DEFINE(remoteControl,"Remote Control\n","Remote Control\n")
KEYWORD_DEFINE(asset,"Asset\n","Asset\n")

KEYWORD_DEFINE(sensorstring,"Sensor\n","Sensor\n")
KEYWORD_DEFINE(fan,"Fan\n","風扇\n")
KEYWORD_DEFINE(memDevice,"Memory Device\n","記憶體裝置\n")
KEYWORD_DEFINE(cpu,"CPU\n","CPU\n")
KEYWORD_DEFINE(bios,"BIOS\n","BIOS\n")
KEYWORD_DEFINE(systemInfo,"System information\n","系統資訊\n")
KEYWORD_DEFINE(softwIdentity,"Software Identity\n","軟體清單\n")
KEYWORD_DEFINE(opSystem,"Operation System\n","作業系統\n")
KEYWORD_DEFINE(computerSys,"Computer System\n","電腦系統\n")

INT8U  s_dashtech_en[] _ATTRIBUTE_LANGUAGE_EN = {"With Dash Technology\n"};
INT8U  s_dashtech_2[] _ATTRIBUTE_LANGUAGE_2 = {"與DASH技術\n"};
INT8U  *s_dashtech;
INT8U  *s_dashtech_f;
KEYWORD_DEFINE(startxxx,"XXX\n","XXX\n") //This is use to be the head
#else
KEYWORD_DEFINE(realtekCop,"Realtek Semiconductor Corp.","瑞昱半導體")
KEYWORD_DEFINE(gigaEthControl,"Gigabit Ethernet Controller","超高速乙太網路")
KEYWORD_DEFINE(welstr,"Welcome to use Realtek Gigabit","歡迎使用瑞昱科技")
KEYWORD_DEFINE(welcome,"Welcome","歡迎")

KEYWORD_DEFINE(realtek,"Reltek","瑞昱")
KEYWORD_DEFINE(about,"About","關於")
KEYWORD_DEFINE(matchCondition,"Match Condition","Match Condition")
KEYWORD_DEFINE(mailAccount,"Mail Account","Mail Account")
KEYWORD_DEFINE(mailWakeup,"Mail Wake Up","Mail Wakeup")
KEYWORD_DEFINE(serviceSetting,"Service setting","Service Setting")
KEYWORD_DEFINE(networkSetting,"Network setting","Network Setting")
KEYWORD_DEFINE(eventLog,"Event Log","Event Log")
KEYWORD_DEFINE(remoteControl,"Remote Control","Remote Control")
KEYWORD_DEFINE(asset,"Asset","Asset")

KEYWORD_DEFINE(sensorstring,"Sensor","Sensor")
KEYWORD_DEFINE(fan,"Fan","風扇")
KEYWORD_DEFINE(memDevice,"Memory Device","記憶體裝置")
KEYWORD_DEFINE(cpu,"CPU","CPU")
KEYWORD_DEFINE(bios,"BIOS","BIOS")
KEYWORD_DEFINE(systemInfo,"System information","系統資訊")
KEYWORD_DEFINE(softwIdentity,"Software Identity","軟體清單")
KEYWORD_DEFINE(opSystem,"Operation System","作業系統")
KEYWORD_DEFINE(computerSys,"Computer System","電腦系統")
KEYWORD_DEFINE(dashtech,"With Dash Technology","與DASH技術")

//INT8U  s_dashtech_en[] _ATTRIBUTE_LANGUAGE_EN = {"With Dash Technology"};
//INT8U  s_dashtech_2[] _ATTRIBUTE_LANGUAGE_2 = {"與DASH技術"};
//INT8U  *s_dashtech;
//INT8U  *s_dashtech_f;
KEYWORD_DEFINE(startxxx,"XXX","XXX") //This is use to be the head
#endif
//XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

//Flash Start -------------------------------------------------------------

//=============================================================
//!!Add to the End!!Add to the End!!Add to the End!!Add to the End!!Add to the End!!
enum keywordIdx{
//!!Add to the End!!Add to the End!!Add to the End!!Add to the End!!Add to the End!!
	s_keywowrdstart_idx,			
	KEYWORD_DEFINE_enum(dashtech)
	KEYWORD_DEFINE_enum(computerSys)
	KEYWORD_DEFINE_enum(opSystem)
	KEYWORD_DEFINE_enum(softwIdentity)	
	KEYWORD_DEFINE_enum(systemInfo)
	KEYWORD_DEFINE_enum(bios)
	KEYWORD_DEFINE_enum(cpu)
	KEYWORD_DEFINE_enum(memDevice)
	KEYWORD_DEFINE_enum(fan)	
	KEYWORD_DEFINE_enum(sensorstring)
	
	KEYWORD_DEFINE_enum(asset)
	KEYWORD_DEFINE_enum(remoteControl)
	KEYWORD_DEFINE_enum(eventLog)
	KEYWORD_DEFINE_enum(networkSetting)	
	KEYWORD_DEFINE_enum(serviceSetting)
	KEYWORD_DEFINE_enum(mailWakeup)
	KEYWORD_DEFINE_enum(mailAccount)
	KEYWORD_DEFINE_enum(matchCondition)
	KEYWORD_DEFINE_enum(about)	
	KEYWORD_DEFINE_enum(realtek)
	
	KEYWORD_DEFINE_enum(welcome)
	KEYWORD_DEFINE_enum(welstr)
	KEYWORD_DEFINE_enum(gigaEthControl)
	KEYWORD_DEFINE_enum(realtekCop)	
	//add at Here
	s_keywowrdend_idx
	//s_realtek_idx
	//start of flash--------------------------
};
//=============================================================

void setLanguageType(enum LanguageType type){

	#ifndef CONFIG_MULTILANGUAGE_ENABLED
	type = en; 
	#endif	

	if(type != en){
		printf("[RTK] Language Type != EN\n");
		//s_dashtech = s_dashtech_f;
		KEYWORD_DEFINE_foregin(dashtech)	
		KEYWORD_DEFINE_foregin(computerSys)
		KEYWORD_DEFINE_foregin(opSystem)		
		KEYWORD_DEFINE_foregin(softwIdentity)		
		KEYWORD_DEFINE_foregin(systemInfo)
		KEYWORD_DEFINE_foregin(bios)
		KEYWORD_DEFINE_foregin(cpu)
		KEYWORD_DEFINE_foregin(memDevice)
		KEYWORD_DEFINE_foregin(fan)
		KEYWORD_DEFINE_foregin(sensorstring)
		
		KEYWORD_DEFINE_foregin(asset)
		KEYWORD_DEFINE_foregin(remoteControl)
		KEYWORD_DEFINE_foregin(eventLog)	
		KEYWORD_DEFINE_foregin(networkSetting)		
		KEYWORD_DEFINE_foregin(serviceSetting)
		KEYWORD_DEFINE_foregin(mailWakeup)
		KEYWORD_DEFINE_foregin(mailAccount)
		KEYWORD_DEFINE_foregin(matchCondition)
		KEYWORD_DEFINE_foregin(about)
		KEYWORD_DEFINE_foregin(realtek)
		
		KEYWORD_DEFINE_foregin(welcome)
		KEYWORD_DEFINE_foregin(welstr)
		KEYWORD_DEFINE_foregin(gigaEthControl)
		KEYWORD_DEFINE_foregin(realtekCop)	
			
	}else{
		printf("[RTK] Language Type is EN\n");
		//s_dashtech = s_dashtech_en;
		KEYWORD_DEFINE_en(dashtech)
		KEYWORD_DEFINE_en(computerSys)
		KEYWORD_DEFINE_en(opSystem)		
		KEYWORD_DEFINE_en(softwIdentity)		
		KEYWORD_DEFINE_en(systemInfo)
		KEYWORD_DEFINE_en(bios)
		KEYWORD_DEFINE_en(cpu)
		KEYWORD_DEFINE_en(memDevice)
		KEYWORD_DEFINE_en(fan)
		KEYWORD_DEFINE_en(sensorstring)
		
		KEYWORD_DEFINE_en(asset)
		KEYWORD_DEFINE_en(remoteControl)
		KEYWORD_DEFINE_en(eventLog)	
		KEYWORD_DEFINE_en(networkSetting)		
		KEYWORD_DEFINE_en(serviceSetting)
		KEYWORD_DEFINE_en(mailWakeup)
		KEYWORD_DEFINE_en(mailAccount)
		KEYWORD_DEFINE_en(matchCondition)
		KEYWORD_DEFINE_en(about)
		KEYWORD_DEFINE_en(realtek)
		
		KEYWORD_DEFINE_en(welcome)
		KEYWORD_DEFINE_en(welstr)
		KEYWORD_DEFINE_en(gigaEthControl)
		KEYWORD_DEFINE_en(realtekCop)	
	}
	//printf("[RTK](setLanguagetype)s_realtek_en=%x , s_welcome_en = %x , s_welstr_en = %x\n",s_realtek_en,s_welcome_en,s_welstr_en);	
	//printf("[RTK](setLanguagetype)[0x%x][0x%x][0x%x]\n",s_dashtech_en,s_dashtech,s_welstr);
	
}


//Function Name : buildKeyworkTable_foreign
//Description: Empty , do not let customer to modify English keywords for now.
//void buildKeyworkTable_en(){
//if do not modify keyword
//}


//Function Name : buildKeyworkTable_foreign
//Description : 
void buildKeyworkTable_foreign(){
	INT8U *sAddr;
	INT8U cflag;
	INT32U idx = 0;
	//INT8U tmp[16];
	INT8U *flashBuf=NULL;
	INT8U *engStart,*fgStart;
	INT8U *start,*end;
	int keyword_len = 0;
	INT8U tmpKey[64];
	//int *EnglishkeyLen;
	
	
	//----------------------------------
	//EnglishkeyLen = malloc(sizeof(int) * );
		
	
	//reset the flash ----------------------	
	//Use 0x00 to replace 0x0A ('\n')

	printf("[RTK]+ buildKeyworkTable_foreign\n");
	
	flashBuf = malloc(4096);
	
	cflag = 0;
	//--------------------------------------------------------------
	if(flashBuf != NULL){
		memcpy(flashBuf,(INT8U *)LANGUAGE2_BASE_ADDR,4096);		
		sAddr = flashBuf;
		
		do{
			//sAddr =strchr(sAddr , '\n');
			sAddr =strstr(sAddr , TAG_F_END);
			if(sAddr){
				cflag = 0x01;//This means the data must be writed back to flash
				memset(sAddr,0x00,TAG_F_END_LEN);
				sAddr+=TAG_F_END_LEN;
			}
			
		}while(sAddr!=NULL);
		idx = 0;
		sAddr = flashBuf;
		printf("[RTK] start to remove 0x0a\n");;
		do{
			if(*sAddr == '\n'){
				*sAddr = 0x00;	
				printf("[RTK] remove one 0x0a , (0x%x)\n",sAddr);;
				cflag = 0x01;//This means the data must be writed back to flash
			}
			sAddr++;			
			idx++;
		}while((sAddr!=0xff) &&(idx<4096));
		

	
		if(cflag){//write back to Flash				
			writeflash(LANGUAGE2_BASE_ADDR,flashBuf,4096);
		}		
		if(flashBuf != NULL){
			free(flashBuf);
			flashBuf = NULL;
		}
	

		
	}else{	
		//(to be continued....)
		//not enough memory , do not support multi-language ; just  English	
		printf("[RTK]  - buildKeyworkTable_foreign : No Memory\n");
		return;
	}
	//----------------------------------------
	

	


	printf("[RTK] parsing keyword\n");
	
	idx = s_keywowrdstart_idx;
	sAddr = (INT8U *)LANGUAGE2_BASE_ADDR;



	while(idx < s_keywowrdend_idx){
			
		#if 1
		
		
		while((*sAddr == 0x00)&&(sAddr < (INT8U *)(LANGUAGE2_BASE_ADDR+4096) )){
			sAddr++;	
		}
		if((*sAddr == 0xFF) || (sAddr >=  (INT8U *)(LANGUAGE2_BASE_ADDR+4096))){
			break;
		}
		
		
		start = strstr(sAddr , TAG_E_START);
		if(start != NULL){
			engStart = start+TAG_E_START_LEN;//Got English
			end = strstr(sAddr , TAG_E_END);	
			if(end){				
				keyword_len = end-engStart;//Got English Length
				
				start = strstr(engStart , TAG_F_START);

				if(start){
					fgStart = start+TAG_F_START_LEN;//Got Foregin
				}else{
					printf("Error !! Could not find \"<F>\"");
					sAddr = sAddr + strlen(sAddr) ;
					continue;
				}
				
				//end = strstr(engStart , TAG_F_END);				
				//if(end){
				//	memset(end,0x00,TAG_F_END_LEN);//Make 0x00 NULL
				//}
			}else{
				//Error !! Could not find "</E>"
				printf("Error !! Could not find \"</E>\"");
				sAddr = sAddr + strlen(sAddr) ;
				continue;
			}
			
		}else{
			//Error !! Could not find "<E>"
			printf("Error !! Could not find \"<E>\"");
			sAddr = sAddr + strlen(sAddr) ;
			continue;
		}

	memset(tmpKey,0x00,64);
	memcpy(tmpKey,engStart,keyword_len);
		
		

		//printf("[RTK](%d,%d)\n",strlen(s_startxxx_en) , keyword_len);
		if(memcmp(s_startxxx_en ,engStart ,  keyword_len) == 0){
			if(strlen(s_startxxx_en) == keyword_len){
				s_startxxx_f = fgStart;				
			}
		}		
		KEYWORD_DEFINE_reSetforegin(dashtech)		
		KEYWORD_DEFINE_reSetforegin(computerSys)
		KEYWORD_DEFINE_reSetforegin(opSystem)		
		KEYWORD_DEFINE_reSetforegin(softwIdentity)			
		KEYWORD_DEFINE_reSetforegin(systemInfo)
		KEYWORD_DEFINE_reSetforegin(bios)
		KEYWORD_DEFINE_reSetforegin(cpu)
		KEYWORD_DEFINE_reSetforegin(memDevice)
		KEYWORD_DEFINE_reSetforegin(fan)
		KEYWORD_DEFINE_reSetforegin(sensorstring)
			
		KEYWORD_DEFINE_reSetforegin(asset)
		KEYWORD_DEFINE_reSetforegin(remoteControl)
		KEYWORD_DEFINE_reSetforegin(eventLog)	
		KEYWORD_DEFINE_reSetforegin(networkSetting)			
		KEYWORD_DEFINE_reSetforegin(serviceSetting)
		KEYWORD_DEFINE_reSetforegin(mailWakeup)
		KEYWORD_DEFINE_reSetforegin(mailAccount)
		KEYWORD_DEFINE_reSetforegin(matchCondition)
		KEYWORD_DEFINE_reSetforegin(about)
		KEYWORD_DEFINE_reSetforegin(realtek)
			
		KEYWORD_DEFINE_reSetforegin(welcome)
		KEYWORD_DEFINE_reSetforegin(welstr)
		KEYWORD_DEFINE_reSetforegin(gigaEthControl)	
		KEYWORD_DEFINE_reSetforegin(realtekCop)
	
		
		sAddr = sAddr + strlen(sAddr) ;
		idx++;


		
		#else
		//Without Tag
		/*
		while((*sAddr == 0x00)&&(sAddr < (INT8U *)(LANGUAGE2_BASE_ADDR+4096) )){
			sAddr++;	
		}
		if((*sAddr == 0xFF) || (sAddr >=  (INT8U *)(LANGUAGE2_BASE_ADDR+4096))){
			break;
		}
		
		//printf("[RTK](buildKeyworkTable_foreign) idx = %d\n ",idx);		
		switch(idx){
			
			case s_dashtech_idx:  // case 1:
				s_dashtech_f = sAddr;				
			break;			
			KEYWORD_DEFINE_reSetforegin(computerSys)
			KEYWORD_DEFINE_reSetforegin(opSystem)		
			KEYWORD_DEFINE_reSetforegin(softwIdentity)			
			KEYWORD_DEFINE_reSetforegin(systemInfo)
			KEYWORD_DEFINE_reSetforegin(bios)
			KEYWORD_DEFINE_reSetforegin(cpu)
			KEYWORD_DEFINE_reSetforegin(memDevice)
			KEYWORD_DEFINE_reSetforegin(fan)
			KEYWORD_DEFINE_reSetforegin(sensorstring)
			
			KEYWORD_DEFINE_reSetforegin(asset)
			KEYWORD_DEFINE_reSetforegin(remoteControl)
			KEYWORD_DEFINE_reSetforegin(eventLog)	
			KEYWORD_DEFINE_reSetforegin(networkSetting)			
			KEYWORD_DEFINE_reSetforegin(serviceSetting)
			KEYWORD_DEFINE_reSetforegin(mailWakeup)
			KEYWORD_DEFINE_reSetforegin(mailAccount)
			KEYWORD_DEFINE_reSetforegin(matchCondition)
			KEYWORD_DEFINE_reSetforegin(about)
			KEYWORD_DEFINE_reSetforegin(realtek)
			
			KEYWORD_DEFINE_reSetforegin(welcome)
			KEYWORD_DEFINE_reSetforegin(welstr)
			KEYWORD_DEFINE_reSetforegin(gigaEthControl)	
			KEYWORD_DEFINE_reSetforegin(realtekCop)			
		}

		sAddr = sAddr + strlen(sAddr) ;
		idx++;
		*/
		#endif	
		
	}
	
	
	printf("[RTK](buildKeywordTable)[0x%x][0x%x][0x%x]\n",s_dashtech_f,s_gigaEthControl_f,s_welstr_f);
	//printf("[RTK](buildKeywordTable)[%s][%s][%s]\n",s_dashtech_f,s_gigaEthControl_f,s_welstr_f);
	printf("[RTK] (buildKeywordTable) : (%x)(%x)(%x)\n",s_computerSys_f , s_opSystem_f , s_softwIdentity_f );

	
}
//#define MACRO_BUILD_TABLE(s) KeywordTable[s_idx] = s;
//Function Name : buildKeyworkTable
//Description : Because the language2.img could be modify by Customer .
//                  The keyord point may be modeify (Because the length of keyword could not be forecasted)
//                  Must use this function to search all keyword again.

void buildKeyworkTable(){
	//-----------

	//buildKeyworkTable_en();
	#ifdef CONFIG_MULTILANGUAGE_ENABLED
	buildKeyworkTable_foreign();
	#endif

	printf("[RTK] -buildKeyworkTable\n ");
	
	return;
}


