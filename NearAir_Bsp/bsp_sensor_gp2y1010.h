/**
  ******************************************************************************
  * @file    bsp_sensor_gp2y1010.h
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
#ifndef __BSP_SENSOR_GP2Y1010_H
#define __BSP_SENSOR_GP2Y1010_H
#include "stm32f2xx.h"

#define GPLED_CLK_INIT          RCC_AHB1PeriphClockCmd
#define GPLED_PIN_SOURCE        GPIO_PinSource7
#define GPLED_PIN               GPIO_Pin_7
#define GPLED_PORT              GPIOC
#define GPLED_CLK               RCC_AHB1Periph_GPIOC

void bsp_sensor_init_gp2y1010(void);

#endif

