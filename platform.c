/**
  ******************************************************************************
  * @file    Platform.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides some operation function to somw basic Peripherals.
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

#include "stdio.h"
#include "stm32f2xx.h"
#include "Platform.h"
#include "PlatformWDG.h"
#include "MICO.h"
#include "MICODefine.h"

static void _button_EL_irq_handler( void * arg );
static void _button_EL_Timeout_handler( void* arg );
static mico_timer_t _button_EL_timer;
mico_mutex_t printf_mutex;
void Debug_UART_Init(void);

#define platform_log(M, ...) custom_log("Platform", M, ##__VA_ARGS__)
#define platform_log_trace() custom_log_trace("Platform")

static uint32_t _default_start_time = 0;

void Platform_Init(void)
{
  /*STM32 wakeup by watchdog in standby mode, re-enter standby mode in this situation*/
  PlatformWDGReload();
  if ( (PWR_GetFlagStatus(PWR_FLAG_SB) != RESET) && RCC_GetFlagStatus(RCC_FLAG_IWDGRST) != RESET)
  {
    RCC_ClearFlag();
    Platform_Enter_STANDBY();
  }
  PWR_ClearFlag(PWR_FLAG_SB);

  mico_rtos_init_mutex(&printf_mutex);
  Platform_Button_EL_Init();
  Platform_Button_STANDBY_Init();
  Platform_LED_SYS_Init();
  Platform_LED_RF_Init();
  Platform_Debug_UART_Init();
}

__weak void PlatformEasyLinkButtonClickedCallback(void){

}

__weak void PlatformEasyLinkButtonLongPressedCallback(void){
  
}

__weak void PlatformStandbyButtonClickedCallback(void){
  
}

static void _button_EL_irq_handler( void* arg )
{
  (void)(arg);
  int interval = -1;

  if (GPIO_ReadInputDataBit(Button_EL_PORT, Button_EL_PIN) == 0) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_EL_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < RestoreDefault_TimeOut){
      /* EasyLink button clicked once */
      PlatformEasyLinkButtonClickedCallback();
    }
    mico_stop_timer(&_button_EL_timer);
    _default_start_time = 0;
  }
}

static void _button_EL_Timeout_handler( void* arg )
{
  (void)(arg);
  _default_start_time = 0;
  PlatformEasyLinkButtonLongPressedCallback();
}

static void _button_STANDBY_irq_handler( void* arg )
{
  (void)(arg);
  PlatformStandbyButtonClickedCallback();
}

void Platform_LED_SYS_Init()
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  LED_SYS_CLK_INIT(LED_SYS_CLK, ENABLE);  

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = LED_SYS_PIN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(LED_SYS_PORT, &GPIO_InitStructure);
}

void Platform_LED_RF_Init(void){
  GPIO_InitTypeDef   GPIO_InitStructure;
  LED_RF_CLK_INIT(LED_RF_CLK, ENABLE);  

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = LED_RF_PIN;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_Init(LED_RF_PORT, &GPIO_InitStructure);
  Platform_LED_RF_Set_Status(OFF);
}

void Platform_LED_SYS_Set_Status(led_operation opperation)
{
  switch(opperation){
    case ON:
      GPIO_WriteBit(LED_SYS_PORT, LED_SYS_PIN, Bit_SET);
    break;
    case OFF:
      GPIO_WriteBit(LED_SYS_PORT, LED_SYS_PIN, Bit_RESET);
    break;
    case TRIGGER:
      GPIO_ToggleBits(LED_SYS_PORT, LED_SYS_PIN);
    break;
  }
}

void Platform_LED_RF_Set_Status(led_operation opperation)
{
  switch(opperation){
    case ON:
      GPIO_WriteBit(LED_RF_PORT, LED_RF_PIN, Bit_RESET);
    break;
    case OFF:
      GPIO_WriteBit(LED_RF_PORT, LED_RF_PIN, Bit_SET);
    break;
    case TRIGGER:
      GPIO_ToggleBits(LED_RF_PORT, LED_RF_PIN);
    break;
  }
}

void Platform_Enter_STANDBY(void)
{
  PWR_WakeUpPinCmd(ENABLE);
  PWR_EnterSTANDBYMode();
}

void Platform_Button_EL_Init(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  Button_EL_CLK_INIT(Button_EL_CLK, ENABLE);	
  GPIO_InitStructure.GPIO_Pin = Button_EL_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;			
  GPIO_Init(Button_EL_PORT, &GPIO_InitStructure);

  mico_init_timer(&_button_EL_timer, RestoreDefault_TimeOut, _button_EL_Timeout_handler, NULL);
  gpio_irq_enable(Button_EL_PORT, Button_EL_IRQ_PIN, IRQ_TRIGGER_BOTH_EDGES, _button_EL_irq_handler, 0);
  
  NVIC_InitStructure.NVIC_IRQChannel = Button_EL_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}	


void Platform_Button_STANDBY_Init(void)
{
  GPIO_InitTypeDef   GPIO_InitStructure;
  NVIC_InitTypeDef   NVIC_InitStructure;

  Button_EL_CLK_INIT(Button_STANDBY_CLK, ENABLE);  
  GPIO_InitStructure.GPIO_Pin = Button_STANDBY_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;     
  GPIO_Init(Button_STANDBY_PORT, &GPIO_InitStructure);

  gpio_irq_enable(Button_STANDBY_PORT, Button_STANDBY_IRQ_PIN, IRQ_TRIGGER_FALLING_EDGE, _button_STANDBY_irq_handler, 0);
  
  NVIC_InitStructure.NVIC_IRQChannel = Button_STANDBY_IRQ;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void Platform_Debug_UART_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;

   
  mico_rtos_init_mutex( &printf_mutex );
  DEBUG_GPIO_CLK_INIT(DEBUG_USARTx_RX_GPIO_CLK, ENABLE);
  DEBUG_USARTx_CLK_INIT(DEBUG_USARTx_CLK, ENABLE);
  
  /* Configure USART pin*/
  GPIO_PinAFConfig(DEBUG_USARTx_TX_GPIO_PORT, DEBUG_USARTx_TX_SOURCE, DEBUG_USARTx_TX_AF);
  GPIO_PinAFConfig(DEBUG_USARTx_RX_GPIO_PORT, DEBUG_USARTx_RX_SOURCE, DEBUG_USARTx_RX_AF);
  
  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = DEBUG_USARTx_TX_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(DEBUG_USARTx_TX_GPIO_PORT, &GPIO_InitStructure);

  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = DEBUG_USARTx_RX_PIN;
  GPIO_Init(DEBUG_USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
  
  USART_DeInit(DEBUG_USARTx);
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(DEBUG_USARTx, &USART_InitStructure);
  
  USART_Cmd(DEBUG_USARTx, ENABLE);
}

void PlatformSoftReboot(void)
{ 
  NVIC_SystemReset();
}


/* Retarget the C library printf function to the UART. 
  * All printf output will print from the uart.
  */
int fputc(int ch, FILE *f)
{
  if (ch == '\n')  {
    while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(DEBUG_USARTx, 0x0D);
  }
  while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);
  USART_SendData(DEBUG_USARTx, (uint8_t) ch);
  return ch;
}
