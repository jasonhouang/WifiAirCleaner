/**
  ******************************************************************************
  * @file    EasyLink.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide the easylink function and FTC server for quick 
  *          provisioning and first time configuration.
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

#include "MICO.h"
#include "MICONotificationCenter.h"

#include "Platform.h"
#include "PlatformFlash.h"
#include "StringUtils.h"
#include "HTTPUtils.h"

#include "WPS.h"

#define wps_log(M, ...) custom_log("WPS", M, ##__VA_ARGS__)
#define wps_log_trace() custom_log_trace("WPS")

static mico_semaphore_t  wps_sem = NULL;

static void wps_thread(void *inContext);

extern void       ConfigWillStart           ( mico_Context_t * const inContext );
extern void       ConfigWillStop            ( mico_Context_t * const inContext );
extern void       ConfigSoftApWillStart     ( mico_Context_t * const inContext );
extern OSStatus   MICOStartBonjourService   ( WiFi_Interface interface, mico_Context_t * const inContext );


void WPSNotify_WPSCompleteHandler(network_InitTypeDef_st *nwkpara, mico_Context_t * const inContext)
{
  OSStatus err;
  wps_log_trace();
  require_action(inContext, exit, err = kParamErr);
  require_action(nwkpara, exit, err = kTimeoutErr);
  
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  memcpy(inContext->flashContentInRam.micoSystemConfig.ssid, nwkpara->wifi_ssid, maxSsidLen);
  inContext->flashContentInRam.micoSystemConfig.channel = 0;
  inContext->flashContentInRam.micoSystemConfig.security = SECURITY_TYPE_AUTO;
  memcpy(inContext->flashContentInRam.micoSystemConfig.user_key, nwkpara->wifi_key, maxKeyLen);
  inContext->flashContentInRam.micoSystemConfig.user_keyLength = strlen(nwkpara->wifi_key);
  memcpy(inContext->flashContentInRam.micoSystemConfig.key, nwkpara->wifi_key, maxKeyLen);
  inContext->flashContentInRam.micoSystemConfig.keyLength = strlen(nwkpara->wifi_key);
  inContext->flashContentInRam.micoSystemConfig.configured = allConfigured;
  MICOUpdateConfiguration(inContext);
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  wps_log("Get SSID: %s, Key: %s", inContext->flashContentInRam.micoSystemConfig.ssid, inContext->flashContentInRam.micoSystemConfig.user_key);
  ConfigWillStop(inContext);
  PlatformSoftReboot();
  return;

/*EasyLink is not start*/    
exit:
  wps_log("ERROR, err: %d", err);
#if defined (CONFIG_MODE_WPS_WITH_SOFTAP)
  mico_rtos_set_semaphore(&wps_sem);
#else
  ConfigWillStop(inContext);
  /*so roll back to previous settings  (if it has) and reboot*/
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  if(inContext->flashContentInRam.micoSystemConfig.configured != unConfigured){
    inContext->flashContentInRam.micoSystemConfig.configured = allConfigured;
    MICOUpdateConfiguration(inContext);
    PlatformSoftReboot();
  }
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  /*module should powd down in default setting*/ 
  micoWlanPowerOff();
#endif
  return;
}

void WPSNotify_SYSWillPowerOffHandler(mico_Context_t * const inContext)
{
  stopWPS(inContext);
}

OSStatus startWPS( mico_Context_t * const inContext)
{
  wps_log_trace();
  OSStatus err = kUnknownErr;

  err = MICOAddNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)WPSNotify_WPSCompleteHandler );
  require_noerr(err, exit);
  err = MICOAddNotification( mico_notify_SYS_WILL_POWER_OFF, (void *)WPSNotify_SYSWillPowerOffHandler );
  require_noerr( err, exit ); 

  // Start the WPS thread
  ConfigWillStart(inContext);
  mico_rtos_init_semaphore(&wps_sem, 1);
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "WPS", wps_thread, 0x1000, (void*)inContext );
  require_noerr_string( err, exit, "ERROR: Unable to start the WPS thread." );

exit:
  return err;
}


void _wpsStartSoftAp( mico_Context_t * const inContext)
{
  OSStatus err;
  wps_log_trace();
  network_InitTypeDef_st wNetConfig;

  memset(&wNetConfig, 0, sizeof(network_InitTypeDef_st));
  wNetConfig.wifi_mode = Soft_AP;
  snprintf(wNetConfig.wifi_ssid, 32, "MXCHIP_%c%c%c%c%c%c", inContext->micoStatus.mac[9],  inContext->micoStatus.mac[10], \
                                                            inContext->micoStatus.mac[12], inContext->micoStatus.mac[13],
                                                            inContext->micoStatus.mac[15], inContext->micoStatus.mac[16] );
  strcpy((char*)wNetConfig.wifi_key, "");
  strcpy((char*)wNetConfig.local_ip_addr, "10.10.10.1");
  strcpy((char*)wNetConfig.net_mask, "255.255.255.0");
  strcpy((char*)wNetConfig.gateway_ip_addr, "10.10.10.1");
  wNetConfig.dhcpMode = DHCP_Server;
  micoWlanStart(&wNetConfig);
  wps_log("Establish soft ap: %s.....", wNetConfig.wifi_ssid);

  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true){
    err = MICOStartBonjourService( Soft_AP , inContext );
    require_noerr(err, exit);
  }
  
  if(inContext->flashContentInRam.micoSystemConfig.configServerEnable == true){
    err = MICOStartConfigServer  ( inContext );
    require_noerr(err, exit);
  }
  ConfigSoftApWillStart( inContext );

exit:
  return;
}


OSStatus stopWPS( mico_Context_t * const inContext)
{
  (void)inContext;

  MICORemoveNotification( mico_notify_EASYLINK_WPS_COMPLETED, (void *)WPSNotify_WPSCompleteHandler );
  MICORemoveNotification( mico_notify_SYS_WILL_POWER_OFF, (void *)WPSNotify_SYSWillPowerOffHandler );

  if(wps_sem){
    mico_rtos_deinit_semaphore(&wps_sem);
    wps_sem = NULL;
  }

  micoWlanStopWPS();
  return kNoErr;
}

void wps_thread(void *inContext)
{
  
  wps_log_trace();
  OSStatus err = kNoErr;
  mico_Context_t *Context = inContext;

  require_action(wps_sem, exit, err = kNotPreparedErr);

  micoWlanStartWPS(WPS_TimeOut/1000); 
  wps_log("Start WPS configuration");
  mico_rtos_get_semaphore(&wps_sem, MICO_WAIT_FOREVER);

  /* If WPS is failed and needs to start soft ap mode */
  msleep(20);
  _wpsStartSoftAp(Context);

exit:
  if(err) wps_log("WPS thread exit, err = %d", err);
  stopWPS(Context);
  mico_rtos_delete_thread(NULL);
    
  return;

}



