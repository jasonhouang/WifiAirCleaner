/**
  ******************************************************************************
  * @file    PlatformRTC.c
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides RTC operation functions.
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


#include "PlatformLogging.h"
#include "PlatformRTC.h"
#include "stm32f2xx.h"

#define RTC_log(M, ...) custom_log("Platform RTC", M, ##__VA_ARGS__)
#define RTC_log_trace() custom_log_trace("Platform RTC")


OSStatus PlatformRTCInitialize( void )
{
  return kNoErr;
}

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformRTCReadTime
    @abstract   
*/
OSStatus PlatformRTCRead( struct tm *time )
{
  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;

  RTC_GetDate(RTC_Format_BIN, &RTC_DateStructure);
  RTC_GetTime(RTC_Format_BIN, &RTC_TimeStructure);
  
  time->tm_sec = RTC_TimeStructure.RTC_Seconds;
  time->tm_min = RTC_TimeStructure.RTC_Minutes;
  time->tm_hour = RTC_TimeStructure.RTC_Hours;

  time->tm_mday = RTC_DateStructure.RTC_Date;
  time->tm_wday = RTC_DateStructure.RTC_WeekDay;
  time->tm_mon = RTC_DateStructure.RTC_Month - 1;
  time->tm_year = RTC_DateStructure.RTC_Year + 100;
  return kNoErr;
}

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformRTCWriteTime
    @abstract   
*/
OSStatus PlatformRTCWrite( struct tm *time )
{
  RTC_TimeTypeDef RTC_TimeStructure;
  RTC_DateTypeDef RTC_DateStructure;

  RTC_TimeStructure.RTC_Seconds = time->tm_sec;
  RTC_TimeStructure.RTC_Minutes = time->tm_min ;
  RTC_TimeStructure.RTC_Hours = time->tm_hour;

  RTC_DateStructure.RTC_Date = time->tm_mday;
  RTC_DateStructure.RTC_WeekDay = time->tm_wday;
  RTC_DateStructure.RTC_Month = time->tm_mon + 1;
  RTC_DateStructure.RTC_Year = (time->tm_year + 1900)%100;

  RTC_SetTime(RTC_Format_BIN, &RTC_TimeStructure);
  RTC_SetDate(RTC_Format_BIN, &RTC_DateStructure);

  return kNoErr;
}






