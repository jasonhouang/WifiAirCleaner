/**
  ******************************************************************************
  * @file    bsp_nearair.c
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   provide partform handles for nearair.
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
#include "bsp_nearair.h"
#include "NearAir.h"

#include "PlatformUART.h"

#define bsp_log(M, ...) custom_log("BSP", M, ##__VA_ARGS__)
#define bsp_log_trace() custom_log_trace("BSP")

void bsp_nearair_init(nearair_Context_t * const airContent)
{
//    bsp_InitLed();
	
//    bsp_InitAdc();
	
    bsp_InitButton(airContent);
	
//    bsp_InitBeep();
	
//    bsp_InitSensor();
	
//    bsp_InitMotor();
}

void nearair_thread(void *inContext)
{
    OSStatus err;
    bsp_log_trace();
    nearair_Context_t *airContent = inContext;    

    bsp_nearair_init(airContent);
    
    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "LED", led_thread, 0x200, (void *)airContent );
      require_noerr_action( err, exit, bsp_log("ERROR: Unable to start the led thread.") );
//    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "BEEP", beep_thread, 0x200, (void *)airContent );
//      require_noerr_action( err, exit, bsp_log("ERROR: Unable to start the beep thread.") );
    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY-1, "SENSOR", sensor_thread, 0x200, (void *)airContent );
      require_noerr_action( err, exit, bsp_log("ERROR: Unable to start the sensor thread.") );
    //!!!cantion:motor_thread must create after led_thread,because TIM configration.!!!
    err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "MOTOR", motor_thread, 0x200, (void *)airContent );
      require_noerr_action( err, exit, bsp_log("ERROR: Unable to start the motor thread.") );
      
    while(1)
    {      
        mico_thread_sleep(1);
//        bsp_log("Humidity is:%d,temperture is:%d",airContext->sensorStatus.humidity,\
//          airContext->sensorStatus.temperature);              
    }
exit:
  bsp_log("Exit: nearair thread exit with err = %d", err);
}
