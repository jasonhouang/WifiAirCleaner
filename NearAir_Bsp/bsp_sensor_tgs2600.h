/**
  ******************************************************************************
  * @file    bsp_sensor_tgs2600.h
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
#ifndef __BSP_SENSOR_TGS2600_H
#define __BSP_SENSOR_TGS2600_H
#include "stdint.h"

typedef enum {
	AIR_OGREAT = 0,
	AIR_OSOSO = 1,
	AIR_OBAD = 2
}AIR_OQuality;


extern void bsp_sensor_init_TGS2600(void);
extern void debug_route(void *arg);

extern void _get_air_odour_quality(void *inContext);
extern AIR_OQuality read_air_odour_quality(void);
#endif
