
#include "PlatformFlash.h"
#include "NearAirDefine.h"

OSStatus NearAirReadConfiguration(nearair_Context_t *airContext)
{
  uint32_t configInFlash;
  OSStatus err = kNoErr;
  configInFlash = EX_PARA_START_ADDRESS;
  memcpy(&airContext->flashNearAirInRam, (void *)configInFlash, sizeof(flash_nearair_t));

  return err;
}

OSStatus NearAirUpdateConfiguration(nearair_Context_t *airContext)
{
  OSStatus err = kNoErr;
  uint32_t paraStartAddress, paraEndAddress;
 
  paraStartAddress = EX_PARA_START_ADDRESS;
  paraEndAddress = EX_PARA_END_ADDRESS;

//  inContext->flashContentInRam.micoSystemConfig.seed = ++seedNum;
  err = PlatformFlashInitialize();
  require_noerr(err, exit);
  err = PlatformFlashErase(paraStartAddress, paraEndAddress);
  require_noerr(err, exit);
  err = PlatformFlashWrite(&paraStartAddress, (u32 *)&airContext->flashNearAirInRam, sizeof(flash_nearair_t));
  require_noerr(err, exit);
  err = PlatformFlashFinalize();
  require_noerr(err, exit);
  
exit:
  return err;
}
