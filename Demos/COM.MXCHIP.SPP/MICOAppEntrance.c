/**
  ******************************************************************************
  * @file    MICOAppEntrance.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Mico application entrance, addd user application functons and threads.
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

#include "MICODefine.h"
#include "MICOAppDefine.h"

#include "StringUtils.h"
//#include "SppProtocol.h"

#include "Platform.h"
#include "PlatformUART.h"

#include "NearAir.h"

extern nearair_Context_t *airContext = NULL;//near add

#define app_log(M, ...) custom_log("APP", M, ##__VA_ARGS__)
#define app_log_trace() custom_log_trace("APP")

static void read_icid_thread(void *arg);

/* MICO system callback: Restore default configuration provided by application */
void appRestoreDefault_callback(mico_Context_t *inContext)
{
  inContext->flashContentInRam.appConfig.configDataVer = CONFIGURATION_VERSION;
  inContext->flashContentInRam.appConfig.localServerPort = LOCAL_PORT;
  inContext->flashContentInRam.appConfig.localServerEnable = true;
  inContext->flashContentInRam.appConfig.USART_BaudRate = 115200;
  inContext->flashContentInRam.appConfig.remoteServerEnable = true;
  sprintf(inContext->flashContentInRam.appConfig.remoteServerDomain, DEAFULT_REMOTE_SERVER);
  inContext->flashContentInRam.appConfig.remoteServerPort = DEFAULT_REMOTE_SERVER_PORT;
}

OSStatus MICOStartApplication( mico_Context_t * const inContext )
{
  app_log_trace();

  OSStatus err = kNoErr;
  require_action(inContext, exit, err = kParamErr);

//  sppProtocolInit(inContext);//near -
  nearairProtocolInit(inContext);//near + 
  PlatformUartInitialize(inContext);
  //near + begin
  airContext = ( nearair_Context_t *)malloc(sizeof(nearair_Context_t) );
  require_action( airContext, exit, err = kNoMemoryErr );
  memset(airContext, 0x0, sizeof(nearair_Context_t));
  mico_rtos_init_mutex(&airContext->flashNearAirInRam_mutex);
  NearAirReadConfiguration(airContext);
  
//  airContext->sensorStatus.dust = 1111;
//  airContext->sensorStatus.humidity = 22;
//  airContext->sensorStatus.odour = 3333;
//  airContext->sensorStatus.temperature = 44;
//  airContext->airStatus.airclass = AQ_GOOD;
  airContext->sensorStatus.light = DAY;
//  strcpy(airContext->flashNearAirInRam.extra_data,"9130756763");
  bsp_nearair_init(airContext);//near + end
      
 err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Near Air", nearair_thread, 0x100, (void*)airContext );
  require_noerr_action( err, exit, app_log("ERROR: Unable to start the Near Air thread.") );
  
  /*Bonjour for service searching*/
//  if(inContext->flashContentInRam.micoSystemConfig.bonjourEnable == true)
//    MICOStartBonjourService( Station, inContext );
  
  err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "UART Recv", uartRecv_thread, 0x200, (void*)airContext );
  require_noerr_action( err, exit, app_log("ERROR: Unable to start the uart recv thread.") );

 if(inContext->flashContentInRam.appConfig.localServerEnable == true){
   err = mico_rtos_create_thread( NULL, MICO_APPLICATION_PRIORITY, "Local Server", localTcpServer_thread, 0x200, (void*)inContext );
   require_noerr_action( err, exit, app_log("ERROR: Unable to start the local server thread.") );
 }

 if(inContext->flashContentInRam.appConfig.remoteServerEnable == true){
   err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY-1, "Remote Client", remoteTcpClient_thread, 0x600, (void*)inContext );
   require_noerr_action( err, exit, app_log("ERROR: Unable to start the remote client thread.") );
 }
 
 err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "Read ICID", read_icid_thread, 0x200, (void*)inContext );
   require_noerr_action( err, exit, app_log("ERROR: Unable to start the test_icid_thread.") );

exit:
  return err;
}

static void read_icid_thread(void *arg)
{
  static u32 CpuID[3];
  static u32 Lock_Code;
  int flash_size;
  (void)arg;
  //获取CPU唯一ID
  while(1)
  {
    flash_size = (*(vu16 *)0x1ffff7e0);
    CpuID[0] = *(vu32*)(0x1ffff7e8);
    CpuID[1] = *(vu32*)(0x1ffff7ec);
    CpuID[2] = *(vu32*)(0x1ffff7f0);
    //加密算法,很简单的加密算法
    Lock_Code = (CpuID[0]>>1)+(CpuID[1]>>2)+(CpuID[2]>>3);
    app_log("flash_size is:%d,CpuID[0] is:%ld \nCpuID[1] is:%ld \nCpuID[2] is:%ld \
          \nLock_Code is:%ld",flash_size,CpuID[0],CpuID[1],CpuID[2],Lock_Code);
    sleep(2);
    goto exit;
  }
exit:
  mico_rtos_delete_thread(NULL);
}
