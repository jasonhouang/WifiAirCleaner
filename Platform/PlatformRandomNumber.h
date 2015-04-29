/**
  ******************************************************************************
  * @file    PlatformRandomNumber.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    08-June-2014
  * @brief   This header contains function prototypes called by the WAC engine 
  *          that must be implemented by the platform. These functions are called
  *          when the platform needs random numbers or bytes.
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

#ifndef __PlatformRandomNumber_h__
#define __PlatformRandomNumber_h__

#include "Common.h"
//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformRandomBytes
    @abstract   This function is called by the WAC engine when it needs cryptographically
                strong random bytes. This function should fill the inBuffer with inByteCount
                cryptographically strong bytes.

    @param      inBuffer    The buffer to fill with random bytes. inBuffer must be malloc'd by the caller.
    @param      inByteCount The length of the buffer to fill.

    @return kNoErr if successful or an error code indicating failure.
*/
OSStatus PlatformRandomBytes( void *inBuffer, size_t inByteCount );

#endif // 


