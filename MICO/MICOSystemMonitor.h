/**
  ******************************************************************************
  * @file    MICOSystemMonitor.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   System monitor function, create a system monitor thread, and user 
  *          can add own monitor events.
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

#ifndef __MICOSYSTEMMONITOR_H__
#define __MICOSYSTEMMONITOR_H__

#include "Common.h"
#include "MICODefine.h"

/** Structure to hold information about a system monitor item */
typedef struct
{
    uint32_t last_update;              /**< Time of the last system monitor update */
    uint32_t longest_permitted_delay;  /**< Longest permitted delay between checkins with the system monitor */
} mico_system_monitor_t;


OSStatus MICOStartSystemMonitor (mico_Context_t * const inContext);

OSStatus MICOUpdateSystemMonitor( mico_system_monitor_t* system_monitor, uint32_t permitted_delay );

OSStatus MICORegisterSystemMonitor( mico_system_monitor_t* system_monitor, uint32_t initial_permitted_delay );


#endif //__MICO_SYSTEM_MONITOR_H__



