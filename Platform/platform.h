/**
  ******************************************************************************
  * @file    Platform.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers to some basic Peripherals.
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

#ifndef __PLATFORM_H
#define __PLATFORM_H


#include "Common.h"
#include "stm32f2xx.h"
#include "MICO.h"
                                     
void Platform_Init(void);          

typedef enum
{
  IRQ_TRIGGER_RISING_EDGE  = 0x1, /* Interrupt triggered at input signal's rising edge  */
  IRQ_TRIGGER_FALLING_EDGE = 0x2, /* Interrupt triggered at input signal's falling edge */
  IRQ_TRIGGER_BOTH_EDGES   = IRQ_TRIGGER_RISING_EDGE | IRQ_TRIGGER_FALLING_EDGE,
} gpio_irq_trigger_t;

typedef void (*gpio_irq_handler_t)( void* arg);

#ifdef EMW3162
#include "HAL_EMW3162.h"
#endif

#ifdef EMW3161
#include "HAL_EMW3161.h"
#endif

#ifdef Open1081
#include "HAL_Open1081.h"
#endif

typedef enum _led_index {
  MICO_LED_SYS,
  MICO_LED_RF
} led_index;

typedef enum _led_operation {
  ON,
  OFF,
  TRIGGER
} led_operation;

/*Leds*/
void Platform_LED_SYS_Init(void);
void Platform_LED_RF_Init(void);
void Platform_LED_SYS_Set_Status(led_operation opperation);
void Platform_LED_RF_Set_Status(led_operation opperation);

/*Buttons*/
void Platform_Button_EL_Init(void);
void Platform_Button_STANDBY_Init(void);

/*Low power mode*/
void Platform_Enter_STANDBY(void);

/*Debug UART*/
void Platform_Debug_UART_Init(void);

/*nearair add MOTOR Pin lower*/
void Platform_Motor_Pin_Low(void);

int gpio_irq_enable (GPIO_TypeDef* gpio_port, uint8_t gpio_pin_number, gpio_irq_trigger_t trigger, gpio_irq_handler_t handler, void* arg);

int gpio_irq_disable(GPIO_TypeDef* gpio_port, uint8_t gpio_pin_number);

void PlatformSoftReboot(void);

extern void PlatformEasyLinkButtonClickedCallback(void);
#endif /* __PLATFORM_H */
