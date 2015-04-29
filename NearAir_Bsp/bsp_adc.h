/**
  ******************************************************************************
  * @file    bsp_adc.h
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   headers of drive GP2Y1010 TGS2600 sensor for nearair.
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
#ifndef __BSP_ADC_H
#define __BSP_ADC_H

#include "stm32f2xx.h"



extern void bsp_InitAdc(void);

extern void ad_getVoltage(void *arg);

//extern float get_odour_value(void);

//extern float get_dust_value(void);

extern uint16_t get_tgs2600_adc_new_value(void);

extern uint16_t get_lightsensor_adc_new_value(void);



#endif
