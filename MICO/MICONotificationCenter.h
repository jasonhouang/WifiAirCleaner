/**
  ******************************************************************************
  * @file    MICONotificationCenter.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide function prototypes for operations on MICO's 
  *          notification center
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, MXCHIP Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 MXCHIP Inc.</center></h2>
  ******************************************************************************
  */



#ifndef __MICONOTIFICATIONCENTER_H__
#define __MICONOTIFICATIONCENTER_H__

#include "MICODefine.h"

typedef enum {
  NOTIFY_STATION_UP = 1,
  NOTIFY_STATION_DOWN,

  NOTIFY_AP_UP,
  NOTIFY_AP_DOWN,
} WiFiEvent;

typedef enum{
  /* MICO system defined notifications */
  mico_notify_WIFI_SCAN_COMPLETED,          //void (*function)(ScanResult *pApList, mico_Context_t * const inContext);
  mico_notify_WIFI_STATUS_CHANGED,          //void (*function)(WiFiEvent status, mico_Context_t * const inContext);
  mico_notify_WiFI_PARA_CHANGED,            //void (*function)(apinfo_adv_t *ap_info, char *key, int key_len, mico_Context_t * const inContext);
  mico_notify_DHCP_COMPLETED,               //void (*function)(IPStatusTypedef *pnet, mico_Context_t * const inContext);
  mico_notify_EASYLINK_WPS_COMPLETED,       //void (*function)(network_InitTypeDef_st *nwkpara, mico_Context_t * const inContext);
  mico_notify_EASYLINK_GET_EXTRA_DATA,      //void (*function)(int datalen, char*data, mico_Context_t * const inContext);
  mico_notify_TCP_CLIENT_CONNECTED,         //void (*function)(char *str, int len, mico_Context_t * const inContext);
  mico_notify_DNS_RESOLVE_COMPLETED,        //void (*function)(char *str, int len, mico_Context_t * const inContext);
  mico_notify_READ_APP_INFO,                //void (*function)(int fd, mico_Context_t * const inContext);
  mico_notify_SYS_WILL_POWER_OFF,           //void (*function)(mico_Context_t * const inContext);
  mico_notify_WIFI_CONNECT_FAILED,          //void join_fail(OSStatus err, mico_Context_t * const inContext);
  mico_notify_WIFI_SCAN_ADV_COMPLETED,      //void (*function)(ScanResult_adv *pApList, mico_Context_t * const inContext);
  mico_notify_WIFI_Fatal_ERROR,             //void (*function)(mico_Context_t * const inContext);
  
  /* User defined notifications */_

} mico_notify_types_t;

OSStatus MICOInitNotificationCenter   ( void * const inContext );

OSStatus MICOAddNotification          ( mico_notify_types_t notify_type, void *functionAddress );

OSStatus MICORemoveNotification       ( mico_notify_types_t notify_type, void *functionAddress );


#endif


