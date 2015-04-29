/**
  ******************************************************************************
  * @file    PlatformRandomNumber.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    08-June-2014
  * @brief   This header contains function called by the WAC engine that must be
  *          implemented by the platform. These functions are called when the 
  *          platform needs random numbers or bytes.
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

#include "PlatformRandomNumber.h"
#include "PlatformLogging.h"
#include "stm32f2xx.h"

OSStatus PlatformRandomBytes( void *inBuffer, size_t inByteCount )
{
    // PLATFORM_TO_DO
    int idx;
    uint32_t *pWord = inBuffer;
    uint32_t tempRDM;
    uint8_t *pByte = NULL;
    size_t inWordCount;
    size_t remainByteCount;

    inWordCount = inByteCount/4;
    remainByteCount = inByteCount%4;
    pByte = (uint8_t *)pWord+inWordCount*4;

    plat_log_trace();
    RNG_DeInit();
    RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_RNG, ENABLE);
    RNG_Cmd(ENABLE);

    for(idx = 0; idx<inWordCount; idx++, pWord++){
        while(RNG_GetFlagStatus(RNG_FLAG_DRDY)!=SET);
        *pWord = RNG_GetRandomNumber();
    }

    if(remainByteCount){
        while(RNG_GetFlagStatus(RNG_FLAG_DRDY)!=SET);
        tempRDM = RNG_GetRandomNumber();
        memcpy(pByte, &tempRDM, remainByteCount);
    }
    
    RNG_DeInit();
    return kNoErr;
}


