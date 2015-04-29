/**
  ******************************************************************************
  * @file    HAL_EMW3162.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of the EMW3162 Peripherals.
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

#ifndef __HAL_EMW3162_H
#define __HAL_EMW3162_H

/* Definition for USARTx resources **********************************************/
#define UART_RX_BUF_SIZE      2048
#define UART_DMA_MAX_BUF_SIZE 512

#define GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

#define USARTx_CLK            RCC_APB2Periph_USART1
#define USARTx_CLK_INIT       RCC_APB2PeriphClockCmd

#define USARTx_RX_PIN         GPIO_Pin_10
#define USARTx_IRQ_PIN        10
#define USARTx_RX_SOURCE      GPIO_PinSource10
#define USARTx_RX_GPIO_PORT   GPIOA
#define USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define USARTx_RX_AF          GPIO_AF_USART1

#define USARTx_TX_PIN         GPIO_Pin_9
#define USARTx_TX_SOURCE      GPIO_PinSource9
#define USARTx_TX_GPIO_PORT   GPIOA
#define USARTx_TX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define USARTx_TX_AF          GPIO_AF_USART1
/*
#define USARTx_CTS_PIN        GPIO_Pin_11
#define USARTx_CTS_SOURCE     GPIO_PinSource11
#define USARTx_CTS_GPIO_PORT  GPIOA
#define USARTx_CTS_GPIO_CLK   RCC_AHB1Periph_GPIOA
#define USARTx_CTS_AF         GPIO_AF_USART1

#define USARTx_RTS_PIN        GPIO_Pin_12
#define USARTx_RTS_SOURCE     GPIO_PinSource12
#define USARTx_RTS_GPIO_PORT  GPIOA
#define USARTx_RTS_GPIO_CLK   RCC_AHB1Periph_GPIOA
#define USARTx_RTS_AF         GPIO_AF_USART1
*/
#define USARTx_IRQn           USART1_IRQn
#define USARTx                USART1
#define USARTx_IRQHandler     USART1_IRQHandler

#define USARTx_DR_Base        ((uint32_t)USART1 + 0x04)

#define DMA_CLK_INIT             RCC_AHB1Periph_DMA2
#define UART_RX_DMA              DMA2
#define UART_RX_DMA_Stream       DMA2_Stream2
#define UART_RX_DMA_Stream_IRQn  DMA2_Stream2_IRQn
#define UART_RX_DMA_HTIF         DMA_FLAG_HTIF2
#define UART_RX_DMA_TCIF         DMA_FLAG_TCIF2

#define UART_TX_DMA              DMA2
#define UART_TX_DMA_Stream       DMA2_Stream7
#define UART_TX_DMA_Stream_IRQn  DMA2_Stream7_IRQn
#define UART_TX_DMA_TCIF         DMA_FLAG_TCIF7
#define UART_TX_DMA_IRQHandler   DMA2_Stream7_IRQHandler

/* Definition for EASYLINK Button resources **********************************************/
#define Button_EL_CLK_INIT   RCC_AHB1PeriphClockCmd

#define Button_EL_PIN        GPIO_Pin_15
#define Button_EL_IRQ_PIN    15
#define Button_EL_PORT       GPIOA
#define Button_EL_CLK        RCC_AHB1Periph_GPIOA
#define Button_EL_IRQ        EXTI1_IRQn
#define Button_EL_EXTI_Line  EXTI_Line1

/* Definition for Standby Button resources **********************************************/
#define Button_STANDBY_CLK_INIT   RCC_AHB1PeriphClockCmd

#define Button_STANDBY_PIN        GPIO_Pin_0
#define Button_STANDBY_IRQ_PIN    0
#define Button_STANDBY_PORT       GPIOA
#define Button_STANDBY_CLK        RCC_AHB1Periph_GPIOA
#define Button_STANDBY_IRQ        EXTI0_IRQn
#define Button_STANDBY_EXTI_Line  EXTI_Line0

/* Definition for SYSTEM led resources **********************************************/
#define LED_SYS_CLK_INIT            RCC_AHB1PeriphClockCmd

#define LED_SYS_PIN        GPIO_Pin_0
#define LED_SYS_PORT       GPIOB
#define LED_SYS_CLK        RCC_AHB1Periph_GPIOB

/* Definition for WiFi led resources **********************************************/
#define LED_RF_CLK_INIT             RCC_AHB1PeriphClockCmd

#define LED_RF_PIN                  GPIO_Pin_1
#define LED_RF_PORT                 GPIOB
#define LED_RF_CLK                  RCC_AHB1Periph_GPIOB

/* Definition for Debug UART  resources **********************************************/

#define DEBUG_GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

#define DEBUG_USARTx_CLK            RCC_APB2Periph_USART1
#define DEBUG_USARTx_CLK_INIT       RCC_APB2PeriphClockCmd

#define DEBUG_USARTx_RX_PIN         GPIO_Pin_9
#define DEBUG_USARTx_RX_SOURCE      GPIO_PinSource9
#define DEBUG_USARTx_RX_GPIO_PORT   GPIOA
#define DEBUG_USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define DEBUG_USARTx_RX_AF          GPIO_AF_USART1

#define DEBUG_USARTx_TX_PIN         GPIO_Pin_10
#define DEBUG_USARTx_TX_SOURCE      GPIO_PinSource10
#define DEBUG_USARTx_TX_GPIO_PORT   GPIOA
#define DEBUG_USARTx_TX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define DEBUG_USARTx_TX_AF          GPIO_AF_USART1

#define DEBUG_USARTx                USART1

// #define DEBUG_GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

// #define DEBUG_USARTx_CLK            RCC_APB2Periph_USART6
// #define DEBUG_USARTx_CLK_INIT       RCC_APB2PeriphClockCmd

// #define DEBUG_USARTx_RX_PIN         GPIO_Pin_7
// #define DEBUG_USARTx_RX_SOURCE      GPIO_PinSource7
// #define DEBUG_USARTx_RX_GPIO_PORT   GPIOC
// #define DEBUG_USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOC
// #define DEBUG_USARTx_RX_AF          GPIO_AF_USART6

// #define DEBUG_USARTx_TX_PIN         GPIO_Pin_6
// #define DEBUG_USARTx_TX_SOURCE      GPIO_PinSource6
// #define DEBUG_USARTx_TX_GPIO_PORT   GPIOC
// #define DEBUG_USARTx_TX_GPIO_CLK    RCC_AHB1Periph_GPIOC
// #define DEBUG_USARTx_TX_AF          GPIO_AF_USART6

// #define DEBUG_USARTx                USART6

/* Definition for I2C resources that connected to apple CP chip***************/
#define CP_I2C                          I2C1
#define CP_I2C_CLK                      RCC_APB1Periph_I2C1
#define CP_I2C_SCL_PIN                  GPIO_Pin_6                  /* PB.06 */
#define CP_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOB */
#define CP_I2C_SCL_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define CP_I2C_SCL_SOURCE               GPIO_PinSource6
#define CP_I2C_SCL_AF                   GPIO_AF_I2C1
#define CP_I2C_SDA_PIN                  GPIO_Pin_7                  /* PB.07 */
#define CP_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOB */
#define CP_I2C_SDA_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define CP_I2C_SDA_SOURCE               GPIO_PinSource7
#define CP_I2C_SDA_AF                   GPIO_AF_I2C1

#endif /* __HAL_EMW3162_H */
