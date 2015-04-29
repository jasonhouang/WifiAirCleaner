/**
  ******************************************************************************
  * @file    bsp_beep.h
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
#ifndef __BSP_BEEP_H
#define __BSP_BEEP_H

#include "stm32f2xx.h"
#include "bsp_buzzer.h"

#define BEEP_CLK_INIT          RCC_AHB1PeriphClockCmd
#define BEEP_PIN_SOURCE        GPIO_PinSource11
#define BEEP_PIN               GPIO_Pin_11
#define BEEP_PORT              GPIOA
#define BEEP_CLK               RCC_AHB1Periph_GPIOA

#define SWBEEP_CLK_INIT        RCC_AHB1PeriphClockCmd
//#define SWBEEP_PIN_SOURCE     GPIO_PinSource12
#define SWBEEP_PIN             GPIO_Pin_12
#define SWBEEP_PORT            GPIOA
#define SWBEEP_CLK             RCC_AHB1Periph_GPIOA

extern void bsp_InitBeep(void);
extern void BEEP_SetFreq(uint32_t iFreq);
extern void BeepPwrOn(void);
extern void BeepPwrOff(void);
extern void BEEP_On(void);
extern void BEEP_Off(void);
extern void beep_thread(void *inContext);

#endif
