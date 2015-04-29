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
#include "MICO.h"
#include "MICOSystemMonitor.h"
#include "PlatformWDG.h"


#ifdef APPLICATION_WATCHDOG_TIMEOUT_SECONDS
#define DEFAULT_SYSTEM_MONITOR_PERIOD   (APPLICATION_WATCHDOG_TIMEOUT_SECONDS*1000 - 100)
#else
#ifndef DEFAULT_SYSTEM_MONITOR_PERIOD
#define DEFAULT_SYSTEM_MONITOR_PERIOD   (5000)
#endif
#endif

#ifndef MAXIMUM_NUMBER_OF_SYSTEM_MONITORS
#define MAXIMUM_NUMBER_OF_SYSTEM_MONITORS    (5)
#endif

static mico_system_monitor_t* system_monitors[MAXIMUM_NUMBER_OF_SYSTEM_MONITORS];
void mico_system_monitor_thread_main( void* arg );


OSStatus MICOStartSystemMonitor (mico_Context_t * const inContext)
{
   return mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "SYS MONITOR", mico_system_monitor_thread_main, 0x500, (void*)inContext );
}

void mico_system_monitor_thread_main( void* arg )
{
    (void)arg;
    PlatformWDGInitialize(2*DEFAULT_SYSTEM_MONITOR_PERIOD);

    memset(system_monitors, 0, sizeof(system_monitors));

    while (1)
    {
        int a;
        uint32_t current_time = mico_get_time();

        for (a = 0; a < MAXIMUM_NUMBER_OF_SYSTEM_MONITORS; ++a)
        {
            if (system_monitors[a] != NULL)
            {
                if ((current_time - system_monitors[a]->last_update) > system_monitors[a]->longest_permitted_delay)
                {
                    /* A system monitor update period has been missed */
                    while(1);
                }
            }
        }

        PlatformWDGReload();
        msleep(DEFAULT_SYSTEM_MONITOR_PERIOD);
    }

    mico_rtos_delete_thread(NULL);
}

OSStatus MICORegisterSystemMonitor(mico_system_monitor_t* system_monitor, uint32_t initial_permitted_delay)
{
    int a;

    /* Find spare entry and add the new system monitor */
    for ( a = 0; a < MAXIMUM_NUMBER_OF_SYSTEM_MONITORS; ++a )
    {
        if (system_monitors[a] == NULL)
        {
            system_monitor->last_update = mico_get_time();
            system_monitor->longest_permitted_delay = initial_permitted_delay;
            system_monitors[a] = system_monitor;
            return kNoErr;
        }
    }

    return kUnknownErr;
}

OSStatus MICOUpdateSystemMonitor(mico_system_monitor_t* system_monitor, uint32_t permitted_delay)
{
    uint32_t current_time = mico_get_time();
    /* Update the system monitor if it hasn't already passed it's permitted delay */
    if ((current_time - system_monitor->last_update) <= system_monitor->longest_permitted_delay)
    {
        system_monitor->last_update             = current_time;
        system_monitor->longest_permitted_delay = permitted_delay;
    }

    return kNoErr;
}
