/**
  ******************************************************************************
  * @file    MICOPARASTORAGE.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide operations on nonvolatile memory.
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
#include "MICO.h"
#include "PlatformFlash.h"
#include "Platform.h"

/* Update seed number every time*/
static int32_t seedNum = 0;

__weak void appRestoreDefault_callback(mico_Context_t *inContext)
{

}

OSStatus MICORestoreDefault(mico_Context_t *inContext)
{ 
  OSStatus err = kNoErr;
  uint32_t paraStartAddress, paraEndAddress;
 
  paraStartAddress = PARA_START_ADDRESS;
  paraEndAddress = PARA_END_ADDRESS;

  /*wlan configration is not need to change to a default state, use easylink to do that*/
  sprintf(inContext->flashContentInRam.micoSystemConfig.name, DEFAULT_NAME);
  inContext->flashContentInRam.micoSystemConfig.configured = unConfigured;
  inContext->flashContentInRam.micoSystemConfig.rfPowerSaveEnable = false;
  inContext->flashContentInRam.micoSystemConfig.mcuPowerSaveEnable = false;
  inContext->flashContentInRam.micoSystemConfig.bonjourEnable = true;
  inContext->flashContentInRam.micoSystemConfig.configServerEnable = true;
  inContext->flashContentInRam.micoSystemConfig.seed = seedNum;

  /*Application's default configuration*/
  appRestoreDefault_callback(inContext);

  err = PlatformFlashInitialize();
  require_noerr(err, exit);
  err = PlatformFlashErase(paraStartAddress, paraEndAddress);
  require_noerr(err, exit);
  err = PlatformFlashWrite(&paraStartAddress, (void *)inContext, sizeof(flash_content_t));
  require_noerr(err, exit);
  err = PlatformFlashFinalize();
  require_noerr(err, exit);

exit:
  return err;
}

OSStatus MICOReadConfiguration(mico_Context_t *inContext)
{
  uint32_t configInFlash;
  OSStatus err = kNoErr;
  configInFlash = PARA_START_ADDRESS;
  memcpy(&inContext->flashContentInRam, (void *)configInFlash, sizeof(flash_content_t));
  seedNum = inContext->flashContentInRam.micoSystemConfig.seed;
  if(seedNum == -1) seedNum = 0;

  if(inContext->flashContentInRam.appConfig.configDataVer != CONFIGURATION_VERSION){
    err = MICORestoreDefault(inContext);
    require_noerr(err, exit);
    PlatformSoftReboot();
  }

  if(inContext->flashContentInRam.micoSystemConfig.dhcpEnable == DHCP_Disable){
    strcpy((char *)inContext->micoStatus.localIp, inContext->flashContentInRam.micoSystemConfig.localIp);
    strcpy((char *)inContext->micoStatus.netMask, inContext->flashContentInRam.micoSystemConfig.netMask);
    strcpy((char *)inContext->micoStatus.gateWay, inContext->flashContentInRam.micoSystemConfig.gateWay);
    strcpy((char *)inContext->micoStatus.dnsServer, inContext->flashContentInRam.micoSystemConfig.dnsServer);
  }

exit: 
  return err;
}

OSStatus MICOUpdateConfiguration(mico_Context_t *inContext)
{
  OSStatus err = kNoErr;
  uint32_t paraStartAddress, paraEndAddress;
 
  paraStartAddress = PARA_START_ADDRESS;
  paraEndAddress = PARA_END_ADDRESS;

  inContext->flashContentInRam.micoSystemConfig.seed = ++seedNum;
  err = PlatformFlashInitialize();
  require_noerr(err, exit);
  err = PlatformFlashErase(paraStartAddress, paraEndAddress);
  require_noerr(err, exit);
  err = PlatformFlashWrite(&paraStartAddress, (u32 *)&inContext->flashContentInRam, sizeof(flash_content_t));
  require_noerr(err, exit);
  err = PlatformFlashFinalize();
  require_noerr(err, exit);

exit:
  return err;
}


