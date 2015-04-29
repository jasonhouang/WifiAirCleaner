/**
  ******************************************************************************
  * @file    bsp_nearair.h
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
#ifndef __BSP_NEARAIR_H
#define __BSP_NEARAIR_H
#include "stdint.h"
#include "NearAir.h"
#include "bsp_led.h"
#include "bsp_adc.h"
#include "bsp_button.h"
#include "bsp_beep.h"
#include "bsp_buzzer.h"
#include "bsp_motor.h"
#include "bsp_sensor.h"
//#include "debug_config.h"
#include "bsp_menu.h"
/*
enum {
	POWER_OFF = 0,
	POWER_ON,
};
typedef enum {
	AIR_ON = 0,
	AIR_SMART = 1,
	AIR_SLEEP = 2,
}NearAir_State;
enum {
	MODE_CLOSE = 0,
	MODE_INIT,
	MODE_SMART,
	MODE_SLEEP,
	MODE_MANUAL,
};*/
void nearair_thread(void *inContext);
extern void bsp_nearair_init(nearair_Context_t * const airContent);
extern void bsp_nearair_uinit(void);
extern void nearair_start(void);
extern void nearair_shutdown(void);
extern void nearair_smart(void);
extern void nearair_sleep(void);
extern uint8_t get_nearair_mode(void);
extern void change_nearair_state(void);
extern void set_nearair_mode_manual(void);

#endif
