/**
  ******************************************************************************
  * @file    MICODefine.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide constant definition and type declaration for MICO
             running.
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


#ifndef __MICODEFINE_H
#define __MICODEFINE_H

#include "Common.h"
#include "Debug.h"
#include "MICO.h"
#include "external/JSON-C/json.h"
#include "MICOAppDefine.h"

//#define CONFIG_MODE_EASYLINK_PLUS
#define CONFIG_MODE_EASYLINK
//#define CONFIG_MODE_EASYLINK_WITH_SOFTAP
//#define CONFIG_MODE_WPS
//#define CONFIG_MODE_WPS_WITH_SOFTAP
//#define CONFIG_MODE_WAC

#define CONFIG_SERVICE_PORT     8000

#define BUNDLE_SEED_ID          "C6P64J2MZX"  
#define EA_PROTOCOL             "com.issc.datapath"

#define APPLICATION_WATCHDOG_TIMEOUT_SECONDS  5 /**< Watch-dog enabled by MICO's main thread:
                                                     5 seconds to reload. */

#define RestoreDefault_TimeOut          3000  /**< Restore default and start easylink after 
                                                   press down EasyLink button for 3 seconds. */

#define WPS_TimeOut                     60000 /**< WPS timeout 60 seconds. */

#define maxSsidLen          32
#define maxKeyLen           64
#define maxNameLen          32
#define maxIpLen            16


typedef enum  {
  /*All settings are in default state, module will enter easylink mode when powered on. 
  Press down Easyink button for 3 seconds (defined by RestoreDefault_TimeOut) to enter this mode */
  unConfigured,
  /*Module will enter easylink mode temperaly when powered on, and go back to allConfigured
    mode after time out (Defined by EasyLink_TimeOut), This mode is used for changing wlan
    settings if module is moved to a new wlan enviroment. Press down Easyink button to
    enter this mode */                
  wLanUnConfigured,
  /*Normal working mode, module use the configured settings to connecte to wlan, and run 
    user's threads*/
  allConfigured
}Config_State_t;

typedef enum
{
    eState_Normal,
    eState_Software_Reset,
    eState_Wlan_Powerdown,
    eState_Restore_default,
    eState_Standby,
} SYS_State_t;


/* Upgrade iamge should save this table to flash */
typedef struct  _boot_table_t {
  uint32_t start_address; // the address of the bin saved on flash.
  uint32_t length; // file real length
  uint8_t version[8];
  uint8_t type; // B:bootloader, P:boot_table, A:application, D: 8782 driver
  uint8_t upgrade_type; //u:upgrade, 
  uint8_t reserved[6];
}boot_table_t;

typedef struct _mico_sys_config_t
{
  /*Device identification*/
  char            name[maxNameLen];

  /*Wi-Fi configuration*/
  char            ssid[maxSsidLen];
  char            user_key[maxKeyLen]; 
  int             user_keyLength;
  char            key[maxKeyLen]; 
  int             keyLength;
  char            bssid[6];
  int             channel;
  SECURITY_TYPE_E security;

  /*Power save configuration*/
  bool            rfPowerSaveEnable;
  bool            mcuPowerSaveEnable;

  /*Local IP configuration*/
  bool            dhcpEnable;
  char            localIp[maxIpLen];
  char            netMask[maxIpLen];
  char            gateWay[maxIpLen];
  char            dnsServer[maxIpLen];

  /*EasyLink configuration*/
  Config_State_t  configured;
  bool            easyLinkEnable;
  uint32_t        easylinkServerIP;

  /*Services in MICO system*/
  bool            bonjourEnable;
  bool            configServerEnable;

  /*Update seed number when configuration is changed*/
  int32_t         seed;
} mico_sys_config_t;

typedef struct _flash_configuration_t {

  /*OTA options*/
  boot_table_t             bootTable;
  /*MICO system core configuration*/
  mico_sys_config_t        micoSystemConfig;
  /*Application configuration*/
  application_config_t     appConfig; 
} flash_content_t;

typedef struct _current_mico_status_t 
{
  /*MICO system Running status*/
  SYS_State_t           sys_state;
  char                  localIp[maxIpLen];
  char                  netMask[maxIpLen];
  char                  gateWay[maxIpLen];
  char                  dnsServer[maxIpLen];
  char                  mac[18];
  mico_semaphore_t      sys_state_change_sem;
} current_mico_status_t;


typedef struct _mico_Context_t
{
  /*Flash content*/
  flash_content_t           flashContentInRam;
  mico_mutex_t              flashContentInRam_mutex;
  /*Running status*/
  current_mico_status_t     micoStatus;
  current_app_status_t      appStatus;    
} mico_Context_t;

/**
 *   @brief Parameters controlled by the platform to configure the WAC process. 
 */
typedef struct
{
    uint8_t macAddress[ 6 ];        /**< REQUIRED: Accessory MAC address, e.g. 00:11:22:33:44:55. */
    
    bool    isUnconfigured;         /**< TRUE/FALSE: whether the accessory is unconfigured. Should be true for current cases. */
    bool    supportsAirPlay;        /**< TRUE/FALSE: whether the accessory supports AirPlay. */
    bool    supportsAirPrint;       /**< TRUE/FALSE: whether the accessory supports AirPrint. */
    bool    supports2_4GHzWiFi;     /**< TRUE/FALSE: whether the accessory supports 2.4 GHz Wi-Fi. */
    bool    supports5GHzWiFi;       /**< TRUE/FALSE: whether the accessory supports 5 GHz Wi-Fi. */
    bool    supportsWakeOnWireless; /**< TRUE/FALSE: whether the accessory supports Wake On Wireless. */
    
    char    *firmwareRevision;      /**< REQUIRED: Version of the accessory's firmware, e.g. 1.0.0. */
    char    *hardwareRevision;      /**< REQUIRED: Version of the accessory's hardware, e.g. 1.0.0. */
    char    *serialNumber;          /**< OPTIONAL: Accessory's serial number. */
    
    char    *name;                  /**< REQUIRED: Name of the accessory. */
    char    *model;                 /**< REQUIRED: Model name of the accessory. */
    char    *manufacturer;          /**< REQUIRED: Manufacturer name of the accessory. */
    
    char    **eaProtocols;          /**< OPTIONAL: Array of EA Protocol strings. */
    uint8_t numEAProtocols;         /**< OPTIONAL: Number of EA Protocol strings contained in the eaProtocols array. */
    char    *eaBundleSeedID;        /**< OPTIONAL: Accessory manufacturer's BundleSeedID. */
    
} WACPlatformParameters_t;

#define CONFIG_DATA_SIZE (sizeof(application_config_t)-sizeof(uint32_t))

OSStatus MICOStartBonjourService        ( WiFi_Interface interface, mico_Context_t * const inContext );
OSStatus MICOStartConfigServer          ( mico_Context_t * const inContext );
OSStatus MICOStartNTPClient             ( mico_Context_t * const inContext );
OSStatus MICOStartApplication           ( mico_Context_t * const inContext );

OSStatus MICORestoreDefault             ( mico_Context_t * const inContext );
OSStatus MICOReadConfiguration          ( mico_Context_t * const inContext );
OSStatus MICOUpdateConfiguration        ( mico_Context_t * const inContext );


mico_Context_t* read_mico_Context( void );//Near add

#endif /* __MICO_DEFINE_H */
