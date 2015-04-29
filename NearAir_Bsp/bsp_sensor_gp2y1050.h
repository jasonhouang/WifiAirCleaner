/**
  ******************************************************************************
  * @file    bsp_sensor_gp2y1050.h
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
#ifndef __BSP_SENSOR_GP2Y1050_H
#define __BSP_SENSOR_GP2Y1050_H
#include "stm32f2xx.h"

#define GP2Y_GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

#define GP2Y_USARTx_CLK            RCC_APB2Periph_USART6
#define GP2Y_USARTx_CLK_INIT       RCC_APB2PeriphClockCmd

#define GP2Y_USARTx_RX_PIN         GPIO_Pin_7
#define GP2Y_USARTx_RX_SOURCE      GPIO_PinSource7
#define GP2Y_USARTx_RX_GPIO_PORT   GPIOC
#define GP2Y_USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOC
#define GP2Y_USARTx_RX_AF          GPIO_AF_USART6

#define GP2Y_USARTx                USART6

#define GP2Y_UART_RX_DMA_Stream    DMA2_Stream3//not sure
#define GP2Y_USARTx_DR_Base        ((uint32_t)USART6 + 0x04)

void bsp_sensor_init_gp2y1050(void);
void send_uart_value(uint16_t uart_value);
void _get_dust_value(void *inContext);
#endif
