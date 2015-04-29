/**
  ******************************************************************************
  * @file    PlatformWDG.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of watch dog operation functions.
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

#ifndef __PlatformWDG_h__
#define __PlatformWDG_h__

#include "Common.h"

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformWDGInitialize
    @abstract   Performs any platform-specific initialization needed. 
    @param      timeout         Watchdag timeout, application should call PlatformWDGReload befor timeout.
*/
OSStatus PlatformWDGInitialize( uint32_t timeout );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformWDGReload
    @abstract   Reload watchdog counter.
*/
void PlatformWDGReload( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformWDGInitialize
    @abstract   Performs any platform-specific cleanup needed.
*/
OSStatus PlatformWDGFinalize( void );


#endif // __PlatformWDG_h__

