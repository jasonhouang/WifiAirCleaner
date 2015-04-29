/**
  ******************************************************************************
  * @file    MICOConfigDelegate.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide delegate functons from Easylink function and FTC
  *          server. 
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

#include "Common.h"
#include "debug.h"
#include "MICODefine.h"
#include "MICOAppDefine.h"
#include "MICOConfigMenu.h"

#include "HaProtocol.h"
#include "Platform.h"
#include "PlatformUART.h"
#include "EasyLink/EasyLink.h"
#include "external/JSON-C/json.h"
#include "StringUtils.h"

#define SYS_LED_TRIGGER_INTERVAL 100 
#define SYS_LED_TRIGGER_INTERVAL_AFTER_EASYLINK 500 
#define config_delegate_log(M, ...) custom_log("Config Delegate", M, ##__VA_ARGS__)
#define config_delegate_log_trace() custom_log_trace("Config Delegate")

static mico_timer_t _Led_EL_timer = NULL;

static void _led_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  Platform_LED_SYS_Set_Status(TRIGGER);
}

void ConfigWillStart( mico_Context_t * const inContext )
{
  config_delegate_log_trace();
  (void)(inContext); 
    /*Led trigger*/
  mico_init_timer(&_Led_EL_timer, SYS_LED_TRIGGER_INTERVAL, _led_EL_Timeout_handler, NULL);
  mico_start_timer(&_Led_EL_timer);
  return;
}

void ConfigWillStop( mico_Context_t * const inContext )
{
  (void)(inContext); 
  config_delegate_log_trace();
  mico_stop_timer(&_Led_EL_timer);
  mico_deinit_timer( &_Led_EL_timer );
  Platform_LED_SYS_Set_Status(OFF);
  return;
}

void ConfigSoftApWillStart(mico_Context_t * const inContext )
{
  OSStatus err;
  haProtocolInit(inContext);
  PlatformUartInitialize(inContext);

  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread, 0x500, (void*)inContext );
  require_noerr_action( err, exit, config_delegate_log("ERROR: Unable to start the uart recv thread.") );

 if(inContext->flashContentInRam.appConfig.localServerEnable == true){
   err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Local Server", localTcpServer_thread, 0x200, (void*)inContext );
   require_noerr_action( err, exit, config_delegate_log("ERROR: Unable to start the local server thread.") );
 }

exit:
  return;
}

void ConfigEasyLinkIsSuccess( mico_Context_t * const inContext )
{
  (void)(inContext); 
  config_delegate_log_trace();

  mico_stop_timer(&_Led_EL_timer);
  mico_deinit_timer( &_Led_EL_timer );
  mico_init_timer(&_Led_EL_timer, SYS_LED_TRIGGER_INTERVAL_AFTER_EASYLINK, _led_EL_Timeout_handler, NULL);
  mico_start_timer(&_Led_EL_timer);
  return;
}

OSStatus ConfigELRecvAuthData(char * anthData, mico_Context_t * const inContext )
{
  config_delegate_log_trace();
  (void)(inContext);
  (void)(anthData);
  return kNoErr;
}

json_object* ConfigCreateReportJsonMessage( mico_Context_t * const inContext )
{
  OSStatus err = kNoErr;
  config_delegate_log_trace();
  char name[50], *tempString;
  OTA_Versions_t versions;
  char rfVersion[50];
  char *rfVer = NULL, *rfVerTemp = NULL;
  json_object *sectors, *sector, *subMenuSectors, *subMenuSector, *mainObject = NULL;

  wlan_driver_version( rfVersion, 50 );
  rfVer = strstr(rfVersion, "version ");
  config_delegate_log("RF version=%s", rfVersion);
  if(rfVer) rfVer = rfVer + strlen("version ");
  rfVerTemp = rfVer;

  for(rfVerTemp = rfVer; *rfVerTemp != ' '; rfVerTemp++);
  *rfVerTemp = 0x0;

  if(inContext->flashContentInRam.micoSystemConfig.configured == wLanUnConfigured){
    /*You can upload a specific menu*/
  }

  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  snprintf(name, 50, "%s(%c%c%c%c%c%c)",MODEL, 
                                        inContext->micoStatus.mac[9],  inContext->micoStatus.mac[10], 
                                        inContext->micoStatus.mac[12], inContext->micoStatus.mac[13],
                                        inContext->micoStatus.mac[15], inContext->micoStatus.mac[16]);

  versions.fwVersion = FIRMWARE_REVISION;
  versions.hdVersion = HARDWARE_REVISION;
  versions.protocol  = PROTOCOL;
  versions.rfVersion = NULL;

  sectors = json_object_new_array();
  require( sectors, exit );

  err = MICOAddTopMenu(&mainObject, name, sectors, versions);
  require_noerr(err, exit);

  /*Sector 1*/
  sector = json_object_new_array();
  require( sector, exit );
  err = MICOAddSector(sectors, "MICO SYSTEM",    sector);
  require_noerr(err, exit);

    /*name cell*/
    err = MICOAddStringCellToSector(sector, "Device Name",    inContext->flashContentInRam.micoSystemConfig.name,               "RW", NULL);
    require_noerr(err, exit);

    //Bonjour switcher cell
    err = MICOAddSwitchCellToSector(sector, "Bonjour",        inContext->flashContentInRam.micoSystemConfig.bonjourEnable,      "RW");
    require_noerr(err, exit);

    //RF power save switcher cell
    err = MICOAddSwitchCellToSector(sector, "RF power save",  inContext->flashContentInRam.micoSystemConfig.rfPowerSaveEnable,  "RW");
    require_noerr(err, exit);

    //MCU power save switcher cell
    err = MICOAddSwitchCellToSector(sector, "MCU power save", inContext->flashContentInRam.micoSystemConfig.mcuPowerSaveEnable, "RW");
    require_noerr(err, exit);

    /*sub menu*/
    subMenuSectors = json_object_new_array();
    require( subMenuSectors, exit );
    err = MICOAddMenuCellToSector(sector, "Detail", subMenuSectors);
    require_noerr(err, exit);
      
      subMenuSector = json_object_new_array();
      require( subMenuSector, exit );
      err = MICOAddSector(subMenuSectors,  "",    subMenuSector);
      require_noerr(err, exit);

        err = MICOAddStringCellToSector(subMenuSector, "Firmware Rev.",  FIRMWARE_REVISION,     "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "Hardware Rev.",  HARDWARE_REVISION,     "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "MICO OS Rev.",   system_lib_version(),  "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "RF Driver Rev.", rfVer,                 "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "Model",          MODEL,                 "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "Manufacturer",   MANUFACTURER,          "RO", NULL);
        require_noerr(err, exit);
        err = MICOAddStringCellToSector(subMenuSector, "Protocol",       PROTOCOL,              "RO", NULL);
        require_noerr(err, exit);

      subMenuSector = json_object_new_array();
      err = MICOAddSector(subMenuSectors,  "WLAN",    subMenuSector);
      require_noerr(err, exit);

        err = MICOAddStringCellToSector(subMenuSector, "Wi-Fi",        inContext->flashContentInRam.micoSystemConfig.ssid,     "RO", NULL);
        require_noerr(err, exit);

        err = MICOAddStringCellToSector(subMenuSector, "Password",     inContext->flashContentInRam.micoSystemConfig.user_key, "RO", NULL);
        require_noerr(err, exit);

        tempString = DataToHexStringWithColons( (uint8_t *)inContext->flashContentInRam.micoSystemConfig.bssid, 6 );
        err = MICOAddStringCellToSector(subMenuSector, "BSSID",        tempString, "RO", NULL);
        require_noerr(err, exit);
        free(tempString);

        err = MICOAddNumberCellToSector(subMenuSector, "Channel",      inContext->flashContentInRam.micoSystemConfig.channel, "RO", NULL);
        require_noerr(err, exit);

        switch(inContext->flashContentInRam.micoSystemConfig.security){
          case SECURITY_TYPE_NONE:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "Open system", "RO", NULL); 
            break;
          case SECURITY_TYPE_WEP:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WEP",         "RO", NULL); 
            break;
          case SECURITY_TYPE_WPA_TKIP:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WPA TKIP",    "RO", NULL); 
            break;
          case SECURITY_TYPE_WPA_AES:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WPA AES",     "RO", NULL); 
            break;
          case SECURITY_TYPE_WPA2_TKIP:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WPA2 TKIP",   "RO", NULL); 
            break;
          case SECURITY_TYPE_WPA2_AES:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WPA2 AES",    "RO", NULL); 
            break;
          case SECURITY_TYPE_WPA2_MIXED:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "WPA2 MIXED",  "RO", NULL); 
            break;
          default:
            err = MICOAddStringCellToSector(subMenuSector, "Security",   "Auto",      "RO", NULL); 
            break;
        }
        require_noerr(err, exit); 

        if(inContext->flashContentInRam.micoSystemConfig.keyLength == maxKeyLen){ /*This is a PMK key, generated by user key in WPA security type*/
          tempString = calloc(maxKeyLen+1, 1);
          require_action(tempString, exit, err=kNoMemoryErr);
          memcpy(tempString, inContext->flashContentInRam.micoSystemConfig.key, maxKeyLen);
          err = MICOAddStringCellToSector(subMenuSector, "PMK",          tempString, "RO", NULL);
          require_noerr(err, exit);
          free(tempString);
        }
        else{
          err = MICOAddStringCellToSector(subMenuSector, "KEY",          inContext->flashContentInRam.micoSystemConfig.user_key,  "RO", NULL);
          require_noerr(err, exit);
        }

        /*DHCP cell*/
        err = MICOAddSwitchCellToSector(subMenuSector, "DHCP",        inContext->flashContentInRam.micoSystemConfig.dhcpEnable,   "RO");
        require_noerr(err, exit);
        /*Local cell*/
        err = MICOAddStringCellToSector(subMenuSector, "IP address",  inContext->micoStatus.localIp,   "RO", NULL);
        require_noerr(err, exit);
        /*Netmask cell*/
        err = MICOAddStringCellToSector(subMenuSector, "Net Mask",    inContext->micoStatus.netMask,   "RO", NULL);
        require_noerr(err, exit);
        /*Gateway cell*/
        err = MICOAddStringCellToSector(subMenuSector, "Gateway",     inContext->micoStatus.gateWay,   "RO", NULL);
        require_noerr(err, exit);
        /*DNS server cell*/
        err = MICOAddStringCellToSector(subMenuSector, "DNS Server",  inContext->micoStatus.dnsServer, "RO", NULL);
        require_noerr(err, exit);

  /*Sector 3*/
  sector = json_object_new_array();
  require( sector, exit );
  err = MICOAddSector(sectors, "SPP Remote Server",           sector);
  require_noerr(err, exit);


    // SPP protocol remote server connection enable
    err = MICOAddSwitchCellToSector(sector, "Connect SPP Server",   inContext->flashContentInRam.appConfig.remoteServerEnable,   "RW");
    require_noerr(err, exit);

    //Seerver address cell
    err = MICOAddStringCellToSector(sector, "SPP Server",           inContext->flashContentInRam.appConfig.remoteServerDomain,   "RW", NULL);
    require_noerr(err, exit);

    //Seerver port cell
    err = MICOAddNumberCellToSector(sector, "SPP Server Port",      inContext->flashContentInRam.appConfig.remoteServerPort,   "RW", NULL);
    require_noerr(err, exit);

  /*Sector 5*/
  sector = json_object_new_array();
  require( sector, exit );
  err = MICOAddSector(sectors, "MCU IOs",            sector);
  require_noerr(err, exit);

    /*UART Baurdrate cell*/
    json_object *selectArray;
    selectArray = json_object_new_array();
    require( selectArray, exit );
    json_object_array_add(selectArray, json_object_new_int(9600));
    json_object_array_add(selectArray, json_object_new_int(19200));
    json_object_array_add(selectArray, json_object_new_int(38400));
    json_object_array_add(selectArray, json_object_new_int(57600));
    json_object_array_add(selectArray, json_object_new_int(115200));
    err = MICOAddNumberCellToSector(sector, "Baurdrate", 115200, "RW", selectArray);
    require_noerr(err, exit);

  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  
exit:
  if(err != kNoErr && mainObject){
    json_object_put(mainObject);
    mainObject = NULL;
  }
  return mainObject;
}

OSStatus ConfigIncommingJsonMessage( const char *input, mico_Context_t * const inContext )
{
  OSStatus err = kNoErr;
  json_object *new_obj;
  config_delegate_log_trace();

  new_obj = json_tokener_parse(input);
  require_action(new_obj, exit, err = kUnknownErr);
  config_delegate_log("Recv config object=%s", json_object_to_json_string(new_obj));
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  json_object_object_foreach(new_obj, key, val) {
    if(!strcmp(key, "Device Name")){
      strncpy(inContext->flashContentInRam.micoSystemConfig.name, json_object_get_string(val), maxNameLen);
    }else if(!strcmp(key, "RF power save")){
      inContext->flashContentInRam.micoSystemConfig.rfPowerSaveEnable = json_object_get_boolean(val);
    }else if(!strcmp(key, "MCU power save")){
      inContext->flashContentInRam.micoSystemConfig.mcuPowerSaveEnable = json_object_get_boolean(val);
    }else if(!strcmp(key, "Bonjour")){
      inContext->flashContentInRam.micoSystemConfig.bonjourEnable = json_object_get_boolean(val);
    }else if(!strcmp(key, "Connect SPP Server")){
      inContext->flashContentInRam.appConfig.remoteServerEnable = json_object_get_boolean(val);
    }else if(!strcmp(key, "SPP Server")){
      strncpy(inContext->flashContentInRam.appConfig.remoteServerDomain, json_object_get_string(val), 64);
    }else if(!strcmp(key, "SPP Server Port")){
      inContext->flashContentInRam.appConfig.remoteServerPort = json_object_get_int(val);
    }else if(!strcmp(key, "Baurdrate")){
      inContext->flashContentInRam.appConfig.USART_BaudRate = json_object_get_int(val);
    }
  }
  json_object_put(new_obj);
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);

  inContext->flashContentInRam.micoSystemConfig.configured = allConfigured;
  MICOUpdateConfiguration(inContext);

exit:
  return err; 
}