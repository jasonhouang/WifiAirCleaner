/**
  ******************************************************************************
  * @file    PlatformUart.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of UART operation functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_UART_H
#define __PLATFORM_UART_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "Common.h"
#include "MICODefine.h"

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformUartInitialize
    @abstract   Performs any platform-specific initialization needed. 
*/
OSStatus PlatformUartInitialize( mico_Context_t * const inContext );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformUartSend
    @abstract   Send data from UART interface 
    @param      inSendBuf: start address of data
    @param      inBufLen:  data length
*/
OSStatus PlatformUartSend(uint8_t *inSendBuf, uint32_t inBufLen);

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformFlashWrite
    @abstract   This function writes a data buffer in flash (data are 32-bit aligned).
    @note       After writing data buffer, the flash content is checked.
  * @param      FlashAddress: pointer on start address for writing data buffer
  * @param      Data: pointer on data buffer
  * @param      DataLength: length of data buffer (unit is 8-bit bytes) 
*/
OSStatus PlatformUartRecv(uint8_t *inRecvBuf, uint32_t inBufLen, uint32_t inTimeOut);

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformUartRecvedDataLen
    @abstract   This function returns the data length received by UART in buffer.
  * @return     DataLength: length of data in buffer (unit is 8-bit bytes) 
*/
size_t PlatformUartRecvedDataLen(void);

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformFlashFinalize
    @abstract   Performs any platform-specific cleanup needed.
*/
OSStatus PlatformFlashFinalize( void );

#endif  /* __PLATFORM_UART_H */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
