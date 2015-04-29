/**
  ******************************************************************************
  * @file    HAL_EMW3161.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of the EMW3161 Peripherals.
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



#ifndef __HAL_EMW3161_H
#define __HAL_EMW3161_H

/* Definition for USARTx resources **********************************************/
#define UART_RX_BUF_SIZE      2048
#define UART_DMA_MAX_BUF_SIZE 512

#define GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

#define USARTx_CLK            RCC_APB1Periph_USART2
#define USARTx_CLK_INIT       RCC_APB1PeriphClockCmd

#define USARTx_RX_PIN         GPIO_Pin_3
#define USARTx_IRQ_PIN        3
#define USARTx_RX_SOURCE      GPIO_PinSource3
#define USARTx_RX_GPIO_PORT   GPIOA
#define USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define USARTx_RX_AF          GPIO_AF_USART2

#define USARTx_TX_PIN         GPIO_Pin_2
#define USARTx_TX_SOURCE      GPIO_PinSource2
#define USARTx_TX_GPIO_PORT   GPIOA
#define USARTx_TX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define USARTx_TX_AF          GPIO_AF_USART2

#define USARTx_CTS_PIN        GPIO_Pin_0
#define USARTx_CTS_SOURCE     GPIO_PinSource0
#define USARTx_CTS_GPIO_PORT  GPIOA
#define USARTx_CTS_GPIO_CLK   RCC_AHB1Periph_GPIOA
#define USARTx_CTS_AF         GPIO_AF_USART2

#define USARTx_RTS_PIN        GPIO_Pin_1
#define USARTx_RTS_SOURCE     GPIO_PinSource1
#define USARTx_RTS_GPIO_PORT  GPIOA
#define USARTx_RTS_GPIO_CLK   RCC_AHB1Periph_GPIOA
#define USARTx_RTS_AF         GPIO_AF_USART2

#define USARTx_IRQn           USART2_IRQn
#define USARTx                USART2
#define USARTx_IRQHandler     USART2_IRQHandler

#define USARTx_DR_Base        ((uint32_t)USART2 + 0x04)

#define DMA_CLK_INIT             RCC_AHB1Periph_DMA1
#define UART_RX_DMA              DMA1
#define UART_RX_DMA_Stream       DMA1_Stream5
#define UART_RX_DMA_Stream_IRQn  DMA1_Stream5_IRQn
#define UART_RX_DMA_HTIF         DMA_FLAG_HTIF5
#define UART_RX_DMA_TCIF         DMA_FLAG_TCIF5

#define UART_TX_DMA              DMA1
#define UART_TX_DMA_Stream       DMA1_Stream6
#define UART_TX_DMA_Stream_IRQn  DMA1_Stream6_IRQn
#define UART_TX_DMA_TCIF         DMA_FLAG_TCIF6
#define UART_TX_DMA_IRQHandler   DMA1_Stream6_IRQHandler

/* Definition for EASYLINK Button resources **********************************************/
#define Button_EL_CLK_INIT   RCC_AHB1PeriphClockCmd

#define Button_EL_PIN        GPIO_Pin_10
#define Button_EL_IRQ_PIN    10
#define Button_EL_PORT       GPIOH
#define Button_EL_CLK        RCC_AHB1Periph_GPIOH
#define Button_EL_IRQ        EXTI15_10_IRQn
#define Button_EL_EXTI_Line  EXTI_Line10

/* Definition for Standby Button resources **********************************************/
#define Button_STANDBY_CLK_INIT   RCC_AHB1PeriphClockCmd

#define Button_STANDBY_PIN        GPIO_Pin_0
#define Button_STANDBY_IRQ_PIN    0
#define Button_STANDBY_PORT       GPIOF
#define Button_STANDBY_CLK        RCC_AHB1Periph_GPIOF
#define Button_STANDBY_IRQ        EXTI0_IRQn
#define Button_STANDBY_EXTI_Line  EXTI_Line0

/* Definition for SYSTEM led resources **********************************************/
#define LED_SYS_CLK_INIT          RCC_AHB1PeriphClockCmd

#define LED_SYS_PIN               GPIO_Pin_9
#define LED_SYS_PORT              GPIOI
#define LED_SYS_CLK               RCC_AHB1Periph_GPIOI

/* Definition for WiFi led resources **********************************************/
#define LED_RF_CLK_INIT           RCC_AHB1PeriphClockCmd

#define LED_RF_PIN                GPIO_Pin_1
#define LED_RF_PORT               GPIOF
#define LED_RF_CLK                RCC_AHB1Periph_GPIOF

/* Definition for Debug UART  resources **********************************************/

#define DEBUG_GPIO_CLK_INIT         RCC_AHB1PeriphClockCmd

#define DEBUG_USARTx_CLK            RCC_APB1Periph_USART2
#define DEBUG_USARTx_CLK_INIT       RCC_APB1PeriphClockCmd

#define DEBUG_USARTx_RX_PIN         GPIO_Pin_3
#define DEBUG_USARTx_RX_SOURCE      GPIO_PinSource3
#define DEBUG_USARTx_RX_GPIO_PORT   GPIOA
#define DEBUG_USARTx_RX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define DEBUG_USARTx_RX_AF          GPIO_AF_USART2

#define DEBUG_USARTx_TX_PIN         GPIO_Pin_2
#define DEBUG_USARTx_TX_SOURCE      GPIO_PinSource2
#define DEBUG_USARTx_TX_GPIO_PORT   GPIOA
#define DEBUG_USARTx_TX_GPIO_CLK    RCC_AHB1Periph_GPIOA
#define DEBUG_USARTx_TX_AF          GPIO_AF_USART2

#define DEBUG_USARTx                USART2


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

#endif /* __HAL_EMW3161_H */
