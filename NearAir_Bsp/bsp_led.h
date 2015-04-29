/**
  ******************************************************************************
  * @file    bsp_led.h
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   the headers of led handle.
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
#ifndef __BSP_LED_H
#define __BSP_LED_H

typedef enum{
	COLOR_NONE = 0,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_BLUE
}Color_Led;
typedef enum{
	LED_RED = 0,
	LED_GREEN,
	LED_BLUE
}LED_Color;
typedef enum{
	LED_OFF = 0,
	LED_START,
	LED_SMART,
	LED_SLEEP,
}LED_State;

/* 供外部调用的函数声明 */
extern void bsp_InitLed(void);
extern void bsp_LedRed(void);
extern void bsp_LedGreen(void);
extern void bsp_LedBlue(void);
extern void bsp_LedOff(void);
extern void set_led_color(uint16_t R_value,uint16_t G_value,uint16_t B_value);
extern void led_thread(void *inContext);
extern void easylink_led_thread(void *inContext);
#endif
