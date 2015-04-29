/**
  ******************************************************************************
  * @file    bsp_sensor.c
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   .
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, NEAR Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 Near Inc.</center></h2>
  ******************************************************************************
  */

#include "bsp_sensor.h"
#include "PlatformUart.h"

#define sensor_log(M, ...) custom_log("SENSOR", M, ##__VA_ARGS__)
#define sensor_log_trace() custom_log_trace("SENSOR")

//static mico_timer_t _get_TH_timer;
//static mico_timer_t _refresh_timer;
//static mico_timer_t _debug_timer;

void bsp_InitSensor(void)
{	
    bsp_sensor_init_mhtrd06();
	
    bsp_sensor_init_TGS2600();
	
//    bsp_sensor_init_gp2y1010();
    
    bsp_sensor_init_gp2y1050();
}

void sensor_thread(void *inContext)
{
    sensor_log_trace();
    nearair_Context_t *airContext = inContext;
    
    bsp_InitAdc();
    bsp_InitSensor();
        
//    mico_init_timer(&_get_TH_timer,1000,_get_temperature_humidity,airContext);
//    mico_start_timer(&_get_TH_timer);
    
//    mico_init_timer(&_refresh_timer,10,ad_getVoltage,NULL);
//    mico_start_timer(&_test_timer);
    
//    mico_init_timer(&_debug_timer,2000,debug_route,NULL);
 //   mico_start_timer(&_debug_timer);
    
    while(1)
    {
      _get_temperature_humidity(airContext);
//      sensor_log("Humidity is:%d,Temperture is:%d",airContext->sensorStatus.humidity,
//                 airContext->sensorStatus.temperature);
//      ad_getVoltage(NULL);
      _get_air_odour_quality(airContext);
      _get_dust_value(airContext);
      sleep(1);      
    }
}
