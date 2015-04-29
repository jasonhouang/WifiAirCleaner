/**
  ******************************************************************************
  * @file    bsp_adc.c
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   drive Light TGS2600 sensor of nearair.
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
#include "stm32f2xx.h"

#include "bsp_adc.h"

#include "PlatformUART.h"

#define adc_log(M, ...) custom_log("ADC", M, ##__VA_ARGS__)
#define adc_log_trace() custom_log_trace("ADC")

#define DUST_CLK_INIT			RCC_AHB1PeriphClockCmd

#define GPIO_PORT_DUST	GPIOC

#define GPIO_PIN_DUST	GPIO_Pin_7

#define GPIO_CLK_DUST	RCC_AHB1Periph_GPIOC

#define N 50   //每通道采集50次
#define M 2		//为3个通道

vu16 AD_Value[N][M];
vu16 After_filter[M];
void adc_filter(void);
/*
float get_odour_value(void)
{
    adc_filter();
    return After_filter[0]*(3.3/((1<<12)-1));	
}

float get_dust_value(void)
{
    adc_filter();
    return After_filter[1]*(3.3/((1<<12)-1));	
}
*/
uint16_t get_tgs2600_adc_new_value(void)
{
    adc_filter();
    return After_filter[0];
}

uint16_t get_lightsensor_adc_new_value(void)
{
    adc_filter();
    return After_filter[1];
}

static void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
/*	
    DUST_CLK_INIT(GPIO_CLK_DUST,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_DUST;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIO_PORT_DUST, &GPIO_InitStructure);*/

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC ,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
}

static void ADC1_Configuration(void)
{
    ADC_InitTypeDef ADC_InitStructure;
	
    ADC_CommonInitTypeDef ADC_CommonInitStructure;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);//60MHz

    ADC_DeInit();


    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b; //
    ADC_InitStructure.ADC_ScanConvMode = ENABLE; //
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConvEdge_None;//
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; //
    ADC_InitStructure.ADC_NbrOfConversion  = M; //
    ADC_Init(ADC1, &ADC_InitStructure); //
	
    ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;    //
    ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div6; //10MHz
    ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_1;//ADC_DMAAccessMode_Disabled; //
    ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;//
    ADC_CommonInit(&ADC_CommonInitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_144Cycles );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, ADC_SampleTime_144Cycles );
    ADC_RegularChannelConfig(ADC1, ADC_Channel_12, 3, ADC_SampleTime_144Cycles );

    ADC_Cmd(ADC1, ENABLE); //
		
    ADC_DMARequestAfterLastTransferCmd(ADC1,ENABLE); //
    ADC_DMACmd(ADC1, ENABLE);
}

static void NVIC_Configuration(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //DMA 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);        //
    NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream0_IRQn;   //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 10; //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;    //
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     //
    NVIC_Init(&NVIC_InitStructure);
    // ADC 
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);        //
    NVIC_InitStructure.NVIC_IRQChannel = ADC_IRQn;           //
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 9; //
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 8;    //
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     //
    NVIC_Init(&NVIC_InitStructure);
}

void adc_dma_irq(void)
{
  if( DMA_GetITStatus(DMA2_Stream0,DMA_IT_TCIF0))
  {
    DMA_ClearITPendingBit(DMA2_Stream0,DMA_IT_TCIF0);
  }
    adc_log("this a adc_dma_irq");
//  DMA_Cmd(DMA2_Stream0, DISABLE);
  return;
}

static void DMA_Configuration(void)
{
    DMA_InitTypeDef DMA_InitStructure;
	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE); 
	
    DMA_DeInit(DMA2_Stream0); //将DMA的通道1寄存器重设为缺省值
    DMA_InitStructure.DMA_Channel = DMA_Channel_0;
    DMA_InitStructure.DMA_PeripheralBaseAddr = (u32)&ADC1->DR; //DMA外设ADC基地址
    DMA_InitStructure.DMA_Memory0BaseAddr = (u32)&AD_Value; //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;//DMA_DIR_PeripheralSRC; //内存作为数据传输的目的地
    DMA_InitStructure.DMA_BufferSize = N*M; //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable; //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable; //内存地址寄存器不变
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; //数据宽度为16位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //工作在循环缓存模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Low; //DMA通道 x拥有高优先级
    DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
    DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
    DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
    DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
    DMA_Init(DMA2_Stream0, &DMA_InitStructure);
		
//    DMA_ITConfig(DMA2_Stream0,DMA_IT_TC,ENABLE);
    DMA_Cmd(DMA2_Stream0, ENABLE);
}

void bsp_InitAdc(void)
{
    GPIO_Configuration();
	
    ADC1_Configuration();
	
    DMA_Configuration();
		
    NVIC_Configuration();
	
    ADC_SoftwareStartConv(ADC1);
}

void adc_filter(void)
{
    int sum = 0;
    u8 count;
    for(int i=0;i<M;i++)
    {
        for(count=0;count<N;count++)
        {
            sum += AD_Value[count][i];
        }
        After_filter[i]=sum/N;
        sum=0;
    }
}
/*delete if done*/
void ad_getVoltage(void *arg)
{
  (void)(arg);
    adc_filter();
    adc_log("TGS value is:%d",(int)(100*After_filter[0]*(3.3/((1<<12)-1))));
//    adc_log("GP2Y value is:%d",(int)(100*After_filter[1]*(3.3/((1<<12)-1))));
    adc_log("LSE value is:%d",(int)(100*After_filter[1]*(3.3/((1<<12)-1))));

}
