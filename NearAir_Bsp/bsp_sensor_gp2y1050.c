/**
  ******************************************************************************
  * @file    bsp_sensor_gp2y1050.c
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
#include "NearAir.h"
#include "bsp_sensor_gp2y1050.h"

//uint8_t uart6_data_temp[100];

#define N 12

//static u8 vaild_value_count = 0;//有效值标号计数
static uint16_t vaild_value[N+1];//取出有效值作为平均计算
volatile static uint16_t expect_final;//最终计算值
uint32_t total_vaild = 0;
static uint16_t uart_adc_value = 0;
static uint16_t display_value = 0;
static uint8_t compute_count = 0;
static uint8_t compute_rate = 0;
static void compute_display_value(void);
static mico_timer_t _gp2y_timer;
void gp2y_irq(void *arg);

void Gp2y_Uart_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
//  DMA_InitTypeDef  DMA_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;  
  
  GP2Y_GPIO_CLK_INIT(GP2Y_USARTx_RX_GPIO_CLK, ENABLE);
  GP2Y_USARTx_CLK_INIT(GP2Y_USARTx_CLK, ENABLE);
  
  NVIC_InitStructure.NVIC_IRQChannel                   = USART6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = (uint8_t) 0x7;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 0x6;
  NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
  NVIC_Init( &NVIC_InitStructure );
  /* Configure USART pin*/
  GPIO_PinAFConfig(GP2Y_USARTx_RX_GPIO_PORT, GP2Y_USARTx_RX_SOURCE, GP2Y_USARTx_RX_AF);
  
  /* Configure USART Rx as alternate function  */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Pin = GP2Y_USARTx_RX_PIN;
  GPIO_Init(GP2Y_USARTx_RX_GPIO_PORT, &GPIO_InitStructure);
  
  USART_DeInit(GP2Y_USARTx);
  USART_InitStructure.USART_BaudRate = 2400;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx;
  USART_Init(GP2Y_USARTx, &USART_InitStructure);
  
  USART_ITConfig(GP2Y_USARTx,USART_IT_RXNE,ENABLE);
    
  USART_Cmd(GP2Y_USARTx, ENABLE);
/*  USART_DMACmd(GP2Y_USARTx, USART_DMAReq_Rx, ENABLE);
  
  DMA_DeInit( GP2Y_UART_RX_DMA_Stream );
  
  DMA_InitStructure.DMA_PeripheralBaseAddr = GP2Y_USARTx_DR_Base;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;         
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_InitStructure.DMA_Channel = DMA_Channel_4;
  DMA_InitStructure.DMA_BufferSize = 0;
  
  DMA_Init(GP2Y_UART_RX_DMA_Stream, &DMA_InitStructure);*/
  
}

void bsp_sensor_init_gp2y1050(void)
{
    Gp2y_Uart_Init();
    mico_init_timer(&_gp2y_timer,100, gp2y_irq,NULL);
    mico_start_timer(&_gp2y_timer);
}


void send_uart_value(uint16_t uart_value)
{
  uart_adc_value = uart_value;
  return;
}

uint16_t filter(void)
{
  uint8_t i;
  total_vaild = 0;
  
  vaild_value[N] = uart_adc_value;
  for(i=0;i<N;i++)
  {
    vaild_value[i] = vaild_value[i+1];   
    total_vaild += vaild_value[i];
  }
  expect_final = total_vaild/N;
  uart_adc_value = 0;
  return expect_final;
}

void gp2y_irq(void *arg)
{
    (void)(arg);
    filter();
    compute_count ++;
    if(compute_count > compute_rate)
    {
      compute_count = 0;
      compute_display_value();
    }
    return;
}

static uint8_t compute_gap(uint16_t fly_past)
{
  uint8_t temp;
  if(fly_past > 100)
  {
    temp = 12;
    compute_rate = 7;
  }
  else
  {
    temp = fly_past/10 + 1;
    compute_rate = 20;
  }
  return temp;
}

static void compute_display_value(void)
{
  uint8_t gap;
  uint16_t des_value;
  des_value = (uint16_t)(700.0*expect_final/1024*5.0);
  if(des_value > 532)
    des_value = 532;
  if(display_value > des_value)
  {    
    gap = compute_gap(display_value - des_value);
    display_value -= (gap/2 + 1);
  }
  if(display_value < des_value)
  {
    gap = compute_gap(des_value - display_value);
    display_value += gap;
  }
  return;
}

void _get_dust_value(void *inContext)
{
    nearair_Context_t *airContext = inContext;

    airContext->sensorStatus.dust = display_value;
    
    if( display_value > 210) 
      airContext->airStatus.airclass = AQ_BAD;
    else if(display_value > 110)
      airContext->airStatus.airclass = AQ_SOSO;
    else
      airContext->airStatus.airclass = AQ_GOOD;
}
