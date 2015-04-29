/**
  ******************************************************************************
  * @file    MICOAppDefine.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file create a TCP listener thread, accept every TCP client
  *          connection and create thread for them.
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


#ifndef __MICOAPPDEFINE_H
#define __MICOAPPDEFINE_H

#include "Common.h"
#include "HomeKitProfiles.h"

#define APP_INFO   "Apple HomeKit Demo based on MICO OS"

#ifdef EMW3162
#define HARDWARE_REVISION   "3162"
#define DEFAULT_NAME        "EMW3162 Module"
#define MODEL               "EMW3162"
#endif

#ifdef EMW3161
#define HARDWARE_REVISION   "3161"
#define DEFAULT_NAME        "EMW3161 Module"
#define MODEL               "EMW3161"
#endif

#ifdef Open1081
#define HARDWARE_REVISION   "1081"
#define DEFAULT_NAME        "Open1081 DevBoard"
#define MODEL               "Open1081"
#endif

#define FIRMWARE_REVISION   "MICO_HOMEKIT_1"
#define MANUFACTURER        "MXCHIP Inc."
#define SERIAL_NUMBER       "20140606"
#define PROTOCOL            "com.apple.homekit"
#define LOCAL_PORT          8080
#define CONFIGURATION_VERSION    0x00010001 // if changed default configuration, add this num

/*User provided application configurations*/
#define MAX_Local_Client_Num                8
#define DEAFULT_REMOTE_SERVER               "192.168.2.254"
#define DEFAULT_REMOTE_SERVER_PORT          8080
#define UART_RECV_TIMEOUT                   500
#define UART_ONE_PACKAGE_LENGTH             1024
#define wlanBufferLen                       1024

#define LOCAL_TCP_SERVER_LOOPBACK_PORT      1000
#define REMOTE_TCP_CLIENT_LOOPBACK_PORT     1002
#define RECVED_UART_DATA_LOOPBACK_PORT      1003

#define BONJOUR_SERVICE         "_hap._tcp.local."

/*HomeKit definition*/
#define HA_PV               "1.0"
#define HA_SV               "1.0"
#define HA_SERVER_PORT      1200  

typedef enum
{
    eState_M1_SRPStartRequest      = 1,
    eState_M2_SRPStartRespond      = 2,
    eState_M3_SRPVerifyRequest     = 3,
    eState_M4_SRPVerifyRespond     = 4,
    eState_M5_ExchangeRequest      = 5,
    eState_M6_ExchangeRespond      = 6,
} HAPairSetupState_t;

/*Application's configuration stores in flash, and loaded to ram when system boots up*/
typedef struct
{
  uint32_t          configDataVer;
  uint32_t          localServerPort;

  /*local services*/
  bool              localServerEnable;
  bool              remoteServerEnable;
  char              remoteServerDomain[64];
  int               remoteServerPort;

  /*Homekit*/
  bool              haPairSetupFinished;
  uint8_t           LTSK[64]; 

  /*IO settings*/
  uint32_t          USART_BaudRate;
} application_config_t;



#ifdef thermostat
typedef struct _HKServiceStatus_t {
  char              heating_cooling_current[16];
  HkStatus          heating_cooling_current_status;

  char              heating_cooling_target[16];
  char              heating_cooling_target_new[16];
  HkStatus          heating_cooling_target_status;

  float             temperature_current;
  HkStatus          temperature_current_status;

  float             temperature_target;
  float             temperature_target_new;
  HkStatus          temperature_target_status;

  char              temperature_units[16];
  char              temperature_units_new[16];
  HkStatus          temperature_units_status;

  float             relative_humidity_current;
  HkStatus          relative_humidity_current_status;

  float             relative_humidity_target;
  float             relative_humidity_target_new;
  HkStatus          relative_humidity_target_status;

  float             heating_threshold;
  float             heating_threshold_new;
  HkStatus          heating_threshold_status;

  float             cooling_threshold;
  float             cooling_threshold_new;
  HkStatus          cooling_threshold_status;

  char              name[64];
  HkStatus          name_status;
} HKServiceStatus;
#endif  

#ifdef lightbulb
typedef struct _HKServiceStatus_t {

  bool              on;
  bool              on_new;
  HkStatus          on_status;

  int               brightness;
  int               brightness_new;
  HkStatus          brightness_status;

  float             hue;
  float             hue_new;
  HkStatus          hue_status;

  float             saturation;
  float             saturation_new;
  HkStatus          saturation_status;

  char              name[64];
  HkStatus          name_status;

} HKServiceStatus;
#endif  



/*Running status*/
typedef struct _current_app_status_t {
  /*Local clients port list*/
  uint32_t          loopBack_PortList[MAX_Local_Client_Num];
  /*Remote TCP client connecte*/
  bool              isRemoteConnected;
  /*Homekit*/
  bool              haPairSetupRunning;
  int               statusNumber;

  //mico_semaphore_t  write
  HKServiceStatus   service;


#ifdef thermostat
  
  
#endif 
} current_app_status_t;


void localTcpServer_thread(void *inContext);
void remoteTcpClient_thread(void *inContext);
void uartRecv_thread(void *inContext);
void homeKitListener_thread(void *inContext);

#endif

