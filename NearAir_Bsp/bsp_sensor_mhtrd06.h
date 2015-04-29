/**
  ******************************************************************************
  * @file    bsp_sensor_mhtrd06.h
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
#ifndef __BSP_SENSOR_MHTRD06_H
#define __BSP_SENSOR_MHTRD06_H

#include "stm32f2xx.h"

#define MHTRD_CLK_INIT			RCC_AHB1PeriphClockCmd

#define MHTRD_PIN         	        GPIO_Pin_14
#define MHTRD_IRQ_PIN			14
#define MHTRD_PORT   			GPIOB
#define MHTRD_CLK    			RCC_AHB1Periph_GPIOB

#define MHTRD_DATA_IO(a) if(a)  \
                 GPIO_SetBits(GPIOB,GPIO_Pin_14);  \
               else  \
                 GPIO_ResetBits(GPIOB,GPIO_Pin_14);

extern void bsp_sensor_init_mhtrd06(void);
extern void monitor_temperature_humidity(void);
extern void _get_temperature_humidity(void *inContext);
#endif
