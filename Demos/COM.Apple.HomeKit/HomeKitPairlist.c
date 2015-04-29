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

#include "HomeKitPairlist.h"
#include "Debug.h"
#include "PlatformFlash.h"

/* Update seed number every time*/

OSStatus HMClearPairList(void)
{ 
  OSStatus err = kNoErr;
  uint32_t exParaStartAddress, exParaEndAddress;
 
  exParaStartAddress = EX_PARA_START_ADDRESS;
  exParaEndAddress = EX_PARA_END_ADDRESS;
  pair_list_in_flash_t *pairList = NULL;
  pairList = calloc(1, sizeof(pair_list_in_flash_t));
  require_action(pairList, exit, err = kNoMemoryErr);

  err = PlatformFlashInitialize();
  require_noerr(err, exit);
  err = PlatformFlashErase(exParaStartAddress, exParaEndAddress);
  require_noerr(err, exit);
  err = PlatformFlashWrite(&exParaStartAddress, (uint32_t *)pairList, sizeof(pair_list_in_flash_t));
  require_noerr(err, exit);
  err = PlatformFlashFinalize();
  require_noerr(err, exit);

exit:
  if(pairList) free(pairList);
  return err;
}

OSStatus HMReadPairList(pair_list_in_flash_t *pPairList)
{
  uint32_t configInFlash;
  OSStatus err = kNoErr;
  require(pPairList, exit);

  configInFlash = EX_PARA_START_ADDRESS;
  memcpy(pPairList, (void *)configInFlash, sizeof(pair_list_in_flash_t));

exit: 
  return err;
}

OSStatus HMUpdatePairList(pair_list_in_flash_t *pPairList)
{
  OSStatus err = kNoErr;
  uint32_t exParaStartAddress, exParaEndAddress;
 
  exParaStartAddress = EX_PARA_START_ADDRESS;
  exParaEndAddress = EX_PARA_END_ADDRESS;

  err = PlatformFlashInitialize();
  require_noerr(err, exit);
  err = PlatformFlashErase(exParaStartAddress, exParaEndAddress);
  require_noerr(err, exit);
  err = PlatformFlashWrite(&exParaStartAddress, (uint32_t *)pPairList, sizeof(pair_list_in_flash_t));
  require_noerr(err, exit);
  err = PlatformFlashFinalize();
  require_noerr(err, exit);

exit:
  return err;
}


uint8_t * HMFindLTPK(char * name)
{
  int i;

  pair_list_in_flash_t *pairList = (pair_list_in_flash_t *)EX_PARA_START_ADDRESS;

  for(i=0; i<MAXPairNumber; i++){
    if(strncmp(pairList->pairInfo[i].controllerName, name, 64) == 0)
      return pairList->pairInfo[i].controllerLTPK;
  }
  return NULL;

}



