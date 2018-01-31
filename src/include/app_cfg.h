/*
********************************************************************************
* APPLICATION CONFIGURATION
*
* (c) Copyright 2006, Micrium, Weston, FL
* All Rights Reserved
********************************************************************************
*/

#ifndef _APP_CFG_H_
#define _APP_CFG_H_
#include "sys/dpdef.h"

/*
********************************************************************************
* TASK ID
********************************************************************************
*/
#define TASK_SNR_READ_ID          3
#define TASK_USB_ID               4
#define TASK_START_ID             5

#define TASK_TCP_TIMER_ID         6
#define TASK_SMBUS_ID             8
#define TASK_SNMP_ID              9
#define TASK_ASF_V1_ID           10
#define TASK_ASF_V2_ID           11
#define TASK_SOL_ID              12
#define TASK_SOL_RX_ID           13
#define TASK_WSMAN_TLS_ID        14
#define TASK_WSMAN_ID            15

#define TASK_OOBResponse_ID      16
#define TASK_WEB_SRV_ID	         17
#define TASK_WEB_SRV_THREAD_ID   18
#define TASK_DHCPv6_ID           19
#define TASK_DHCPv4_ID           20
#define TASK_TELNET_CLIENT_ID	 21
#define TASK_SOAP_ID             22
#define TASK_TELNET_SRV_ID   	 23
#define TASK_IPERF_ID            24
#define TASK_INDICATION_ID       25
#define TASK_TCP_ID              26
#define TASK_FLASH_ID            27
#define TASK_Provisioning_ID     28
#define TASK_WSMAN_IPv6_ID       29
#define TASK_WSMAN_TLS_IPv6_ID   30
#define TASK_SMBUSLOG_ID         31
#define TASK_mDNS_OFFLOAD_ID   	 36
#define TASK_8021X_ID 	 		 37

//combine test items setting @melody 20160808
#define TASK_OOBMAC_TEST_ID      41
#define TASK_PCI_DEV_DRV_ID      42
#define TASK_RKVM_TEST_ID        43
#define TASK_KCS_TEST_ID         44
#define TASK_CMAC_TEST_ID        45
#define TASK_VPD_TEST_ID         46
#define TASK_VGA_TEST_ID         47
#define TASK_EHCI_TEST_ID        48
#define TASK_TCR_TEST_ID         49
#define TASK_SMBUS_TEST_ID       52
#define TASK_MMD_ID       52

//#define TASK_START_ID            60

/*
********************************************************************************
* TASK PRIORITIES
********************************************************************************
*/
#define TASK_HIGH_PRIO            2
#define TASK_SNR_READ_PRIO        3
#define TASK_USB_PRIO             4
#define TASK_START_PRIO	          5
#define TASK_TCP_TIMER_PRIO       6
#define OS_TASK_TMR_PRIO          7
#define TASK_SMBUS_PRIO           8
#define TASK_SNMP_PRIO            9
#define TASK_ASF_V1_PRIO         10
#define TASK_ASF_V2_PRIO         11
#define TASK_SOL_PRIO            12
#define TASK_SOL_RX_PRIO         13
#define TASK_WSMAN_TLS_PRIO      14
#define TASK_WSMAN_PRIO	         15

#define TASK_OOBResponse_PRIO    16
#define TASK_WEB_SRV_PRIO        17
#define TASK_WEB_SRV_THREAD_PRIO 18
#define TASK_DHCPv6_PRIO         19
#define TASK_DHCPv4_PRIO         20
#define TASK_TELNET_CLIENT_PRIO  21
#define TASK_SOAP_PRIO           22
#define TASK_TELNET_SRV_PRIO	 23
#define TASK_IPERF_PRIO          24
#define TASK_INDICATION_PRIO     25
#define TASK_TCP_PRIO            26
#define TASK_FLASH_PRIO          27
#define TASK_Provisioning_PRIO   28
#define TASK_WSMAN_IPv6_PRIO     29
#define TASK_WSMAN_TLS_IPv6_PRIO 30
#define TASK_SMBUSLOG_PRIO       31
#define TASK_TEREDO_RX_SRV_PRIO	 32		
#define TASK_TEREDO_TX_SRV_PRIO	 33
#define TASK_mDNS_OFFLOAD_PRIO 	 36
#define TASK_8021X_PRIO 	 37
#define TASK_MMD_PRIO 	 38


//Bill
#ifdef CONFIG_MAIL_WAKEUP_ENABLED
#define TASK_MAIL_WAKE_UP_SRV_PRIO 	 38
#endif

//for Version A, PRIO should not be greater than 30
#define TASK_OOBTest_PRIO        40
#define TASK_VNC_SRV_PRIO	 50
#define TASK_VNC_CLIENT_PRIO	 51

//combine test items setting @melody 20160808
#define TASK_OOBMAC_TEST_PRIO      41
#define TASK_PCI_DEV_DRV_PRIO      42
#define TASK_RKVM_TEST_PRIO        43
#define TASK_KCS_TEST_PRIO         44
#define TASK_CMAC_TEST_PRIO        45
#define TASK_VPD_TEST_PRIO         46
#define TASK_VGA_TEST_PRIO         47
#define TASK_EHCI_TEST_PRIO        48
#define TASK_TCR_TEST_PRIO         49
#define TASK_SMBUS_TEST_PRIO   	   52



//#define TASK_START_PRIO	          60

/*
********************************************************************************
*                                            TASK STACK SIZES
********************************************************************************
*/
#define TASK_UNIT                    (128)
#define TASK_SNR_READ_STK_SIZE       (TASK_UNIT*2)
#define TASK_START_STK_SIZE          (TASK_UNIT*2)
#define TASK_SNMP_STK_SIZE           (TASK_UNIT*2)
#define TASK_TCP_STK_SIZE            (TASK_UNIT*7)

#define TASK_TCP_TIMER_STK_SIZE      (TASK_UNIT*3)

#define TASK_WSMAN_STK_SIZE          (TASK_UNIT*5)

//for SMBUSLog, it requires more memory
//temporary reduce stack size of some tasks
//be sure not to use the functions provided by these tasks
#define TASK_WSMAN_TLS_STK_SIZE      (TASK_UNIT*5)
#define TASK_SOL_STK_SIZE            (TASK_UNIT*6)
#define TASK_SOL_RX_STK_SIZE         (TASK_UNIT*2)
#define TASK_INDICATION_STK_SIZE     (TASK_UNIT*4)

#define TASK_SMBUS_STK_SIZE          (TASK_UNIT*2)
#define	TASK_TEREDO_TX_SRV_STK_SIZE  (TASK_UNIT)
#define	TASK_TEREDO_RX_SRV_STK_SIZE  (TASK_UNIT*3)

#define TASK_USB_STK_SIZE            (TASK_UNIT*3)
#define TASK_FLASH_STK_SIZE          (TASK_UNIT*2)

#define TASK_ASF_V1_STK_SIZE         (TASK_UNIT*2)
#define TASK_ASF_V2_STK_SIZE         (TASK_UNIT*2)
#define TASK_WEB_SRV_STK_SIZE        (TASK_UNIT*2)
#define TASK_OOBResponse_STK_SIZE    (TASK_UNIT*2)
#define TASK_OOBTest_STK_SIZE        (TASK_UNIT*2)
#define TASK_Iperf_STK_SIZE          (TASK_UNIT*2)
#define TASK_DHCPv4_STK_SIZE         (TASK_UNIT*2)
#define TASK_DHCPv6_STK_SIZE         (TASK_UNIT*2)
#define TASK_WebSrvThread_STK_SIZE   (TASK_UNIT*8)	
#define TASK_mDNS_SRV_STK_SIZE       (TASK_UNIT*2)	
#define TASK_TELNET_CLIENT_STK_SIZE  (TASK_UNIT*5)	
#define TASK_TELNET_SRV_STK_SIZE     (TASK_UNIT*2)
#define TASK_SMBUSLOG_STK_SIZE       (TASK_UNIT*4)
#define TASK_8021X_STK_SIZE	     (TASK_UNIT*2)
//Bill
#define TASK_MAIL_WAKE_UP_SRV_STK_SIZE  (TASK_UNIT*3)
#define TASK_VNC_CLIENT_STK_SIZE	(TASK_UNIT*2)
#define TASK_VNC_SRV_STK_SIZE    	(TASK_UNIT*1)

//combine test items @melody 20160808
#define TASK_PCI_DEV_DRV_STK_SIZE 	 (TASK_UNIT*8)
#define TASK_OOBMAC_TEST_STK_SIZE 	 (TASK_UNIT*2)
#define TASK_RKVM_TEST_STK_SIZE 	 (TASK_UNIT*2)
#define TASK_KCS_TEST_STK_SIZE 	 	 (TASK_UNIT*2)
#define TASK_CMAC_TEST_STK_SIZE 	 (TASK_UNIT*2)
#define TASK_VPD_TEST_STK_SIZE       (TASK_UNIT*2)
#define TASK_VGA_TEST_STK_SIZE    	 (TASK_UNIT*2)
#define TASK_EHCI_TEST_STK_SIZE    	 (TASK_UNIT*2)
#define TASK_TCR_TEST_STK_SIZE    	 (TASK_UNIT*2)
#define TASK_SMBUS_TEST_STK_SIZE     (TASK_UNIT*2)
#define TASK_MMD_TEST_STK_SIZE     (TASK_UNIT*2)




#endif
