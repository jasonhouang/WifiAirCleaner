/**
  ******************************************************************************
  * @file    bsp_sensor_mhtrd06.c
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
#include "bsp_sensor_mhtrd06.h"

#include "PlatformUART.h"

#define mhtrd_log(M, ...) custom_log("MHTRD", M, ##__VA_ARGS__)
#define mhtrd_log_trace() custom_log_trace("MHTRD")

uint16_t  U8FLAG;
uint8_t  U8temp;
uint8_t  temperature_value,humidity_value;

static uint8_t READ_MHTRD_DATA(void)    {if (GPIO_ReadInputDataBit(MHTRD_PORT, MHTRD_PIN) == Bit_SET) return 1; return 0;}
void MHTRD_TIM_Config(void);

void bsp_sensor_init_mhtrd06(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
		
    RCC_AHB1PeriphClockCmd(MHTRD_CLK,ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = MHTRD_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_OD;//外接上拉电阻，配置为开漏输出，输入输出通用
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;       //
    GPIO_Init(MHTRD_PORT, &GPIO_InitStructure);
    
    MHTRD_TIM_Config();
}
void MHTRD_TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);
    TIM_DeInit(TIM7);//初始化TIM7寄存器

    /*分频和周期计算公式：
    Prescaler = (TIMxCLK / TIMx counter clock) - 1;
    Period = (TIMx counter clock / TIM7 output clock) - 1 ;TIMx counter clock = 10000
    TIMx counter clock为你所需要的TXM的定时器时钟 
    */
    TIM_TimeBaseStructure.TIM_Period = 0xFFFF; //查数据手册可知，TIM7为16位自动装载
    /*在system_stm32f2xx.c中设置的APB1 Prescaler = 4 ,可知
    *APB1时钟为120M/4*2 = 60 000 000,因为如果APB1分频不为1，则定时时钟x2 
    */
    TIM_TimeBaseStructure.TIM_Prescaler = 6-1;//8400-1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
    
 //   TIM_Cmd(TIM7, ENABLE); //使能TIM7定时器

}

void delay_us(uint16_t us)
{
    TIM_Cmd(TIM7, ENABLE); 
    while(TIM7->CNT < 10*us);
    TIM_Cmd(TIM7, DISABLE);
    TIM7->CNT = 0;
}
static uint8_t  gather_th_U8data(void)
{
    uint8_t i;
    uint8_t  U8comdata;
    for(i=0;i<8;i++)
    {
        U8FLAG = 2;	
        while(!READ_MHTRD_DATA() && U8FLAG++){};
        delay_us(700);
        U8temp=0;
        if(READ_MHTRD_DATA())
            U8temp = 1;
        U8FLAG = 2;
        while(READ_MHTRD_DATA() && U8FLAG++);
        //超时则跳出for循环		  
        if(U8FLAG == 1)
            break;

        //判断数据位是0还是1	 
				 
        // 如果高电平高过预定0高电平值则数据位为 1 
        U8comdata <<= 1;
        U8comdata |= U8temp;        //0
    }//rof
    return U8comdata;
}
/*--------------------------------
//-----湿度读取子程序 ------------
//--------------------------------
//----以下变量均为全局变量--------
//----温度高8位== U8T_data_H_temp------
//----温度低8位== U8T_data_L_temp------
//----湿度高8位== U8H_data_H_temp-----
//----湿度低8位== U8H_data_L_temp-----
//----校验 8位 == U8checkdata_temp-----
//--------------------------------*/

void monitor_temperature_humidity(void)
{
    uint8_t  U8T_data_H_temp,U8T_data_L_temp,U8H_data_temp,U8checkdata_temp;   
    uint8_t  humidity_temp,temperature_temp;
    //主机拉低18ms 
    MHTRD_DATA_IO(0);
//    msleep(4);
    delay_us(5000);
    MHTRD_DATA_IO(1);
    //总线由上拉电阻拉高 主机延时20us
//    msleep(3);
    delay_us(500);
    while(READ_MHTRD_DATA()) {;}
    delay_us(50);
    //主机设为输入 判断从机响应信号 
//		DHT11_DATA_IO(1);
    //判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行	  
    if(!READ_MHTRD_DATA())		 //T !	  
    {
        U8FLAG=2;
        //判断从机是否发出 80us 的低电平响应信号是否结束	 
        while(!READ_MHTRD_DATA() && U8FLAG++){};
        U8FLAG=2;
        //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
        while(READ_MHTRD_DATA() && U8FLAG++);
        //数据接收状态		 
				
        U8H_data_temp = gather_th_U8data();
					
        U8T_data_H_temp = gather_th_U8data();

        U8T_data_L_temp = gather_th_U8data();

        U8checkdata_temp = gather_th_U8data();

        //MHTRD_DATA_IO(1);
        //数据校验 

        U8temp = (U8T_data_H_temp + U8T_data_L_temp + U8H_data_temp);
        if(U8temp == U8checkdata_temp)
        {
            humidity_temp = (U8H_data_temp>>4)*10+(U8H_data_temp&0x0f);
//              U8H_data_L = U8H_data_L_temp;//小数部分暂无
            temperature_temp = (U8T_data_H_temp&0x0f)*10+((U8T_data_L_temp&0xf0)>>4);
//              U8T_data_L = U8T_data_L_temp;//小数部分暂无
            if( humidity_temp  < 100 )
            {
              humidity_value = humidity_temp;
              temperature_value = temperature_temp;
            }
        }
    }
}

void _get_temperature_humidity(void *inContext)
{
    nearair_Context_t *airContext = inContext;

    monitor_temperature_humidity();
    airContext->sensorStatus.humidity = humidity_value;
    airContext->sensorStatus.temperature = temperature_value;
}
