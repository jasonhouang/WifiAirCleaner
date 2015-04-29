/**
  ******************************************************************************
  * @file    bsp_led.c 
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   Initillize leds...
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
#include "math.h"
#include "bsp_led.h"
#include "PlatformUart.h"

#define led_log(M, ...) custom_log("LED", M, ##__VA_ARGS__)
#define led_log_trace() custom_log_trace("LED")
/*
	NEAR_MXCHIP开发板LEDRGB指示灯口线分配：
	LEDR : PB3  高电平点亮
	LEDG : PB4  高电平点亮
	LEDB : PB5  高电平点亮
*/
#define GPIO_PORT_LEDR	GPIOB
#define GPIO_PORT_LEDG	GPIOB
#define GPIO_PORT_LEDB	GPIOB

#define GPIO_PIN_LEDR	GPIO_Pin_3
#define GPIO_PIN_LEDG	GPIO_Pin_4
#define GPIO_PIN_LEDB	GPIO_Pin_5

#define GPIO_CLK_LEDR	RCC_AHB1Periph_GPIOB
#define GPIO_CLK_LEDG	RCC_AHB1Periph_GPIOB
#define GPIO_CLK_LEDB	RCC_AHB1Periph_GPIOB


static mico_timer_t _led_init_timer;
static mico_timer_t _led_sleep_timer;
/*
********************************************************************************
*	函 数 名: bsp_InitLed
*	功能说明: 初始化LED指示灯
*	形    参：无
*	返 回 值: 无
********************************************************************************
*/
void Init_Pwm_LED_GPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
	
    /* 打开GPIOB的时钟 */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    /* 配置所有的LED指示灯GPIO为推挽输出模式 */
    /* 由于将GPIO设置为输出时，GPIO输出寄存器的值缺省是0，因此会驱动LED点亮
    这是我不希望的，因此在改变GPIO为输出前，先修改输出寄存器的值为1 */
//	GPIO_SetBits(GPIO_PORT_LEDR,  GPIO_PIN_LEDR);
//	GPIO_SetBits(GPIO_PORT_LEDG,  GPIO_PIN_LEDG);
//	GPIO_SetBits(GPIO_PORT_LEDB,  GPIO_PIN_LEDB);

    //R
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;     //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_TIM2);
    //G
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;     //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_TIM3);
    //B
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;     //
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    //
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_TIM3);
}

void LedR_TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
    TIM_DeInit(TIM2);//初始化TIM2寄存器

    /*分频和周期计算公式：
    Prescaler = (TIMxCLK / TIMx counter clock) - 1;
    Period = (TIMx counter clock / TIM3 output clock) - 1 ;TIMx counter clock = 10000
    TIMx counter clock为你所需要的TXM的定时器时钟 
    */
    TIM_TimeBaseStructure.TIM_Period = 100-1; //查数据手册可知，TIM2为32位自动装载
    /*在system_stm32f2xx.c中设置的APB1 Prescaler = 4 ,可知
    *APB1时钟为120M/4*2 = 60 000 000,因为如果APB1分频不为1，则定时时钟x2 
    */
    TIM_TimeBaseStructure.TIM_Prescaler = 6-1;//
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /*配置输出比较，产生占空比为20%的PWM方波*/
    TIM_OCStructInit(&TIM_OCInitStructure);  //填入缺省值，其中后几项只对TIM1和TIM8有效
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //PWM1为正常占空比模式，PWM2为反极性模式
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; //输入CCR（占空比数值）
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //High为占空比高极性，此时占空比为20%；Low则为反极性，占空比为80%
    /*Red Tim_Config*/
    TIM_OC2Init(TIM2, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的
    /*Motor Tim_Config*/
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的
  
    TIM_ARRPreloadConfig(TIM2, ENABLE); //ARR自动装载默认是打开的，可以不设置

    TIM_Cmd(TIM2, ENABLE); //使能TIM2定时器
    TIM_CtrlPWMOutputs(TIM2,ENABLE);
}
/*************************************************
Function:    void LedG_B_TIM_Config(void)  
Description: 定时器配置函数       
Input:       无
Output:      无                            
*************************************************/
void LedG_B_TIM_Config(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3,ENABLE);
    TIM_DeInit(TIM3);//初始化TIM3寄存器

    /*分频和周期计算公式：
    Prescaler = (TIMxCLK / TIMx counter clock) - 1;
    Period = (TIMx counter clock / TIM3 output clock) - 1 ;TIMx counter clock = 10000
    TIMx counter clock为你所需要的TXM的定时器时钟 
    */
    TIM_TimeBaseStructure.TIM_Period = 100-1; //查数据手册可知，TIM3为16位自动装载
    /*在system_stm32f2xx.c中设置的APB1 Prescaler = 4 ,可知
    *APB1时钟为120M/4*2 = 60 000 000,因为如果APB1分频不为1，则定时时钟x2 
    */
    TIM_TimeBaseStructure.TIM_Prescaler = 6-1;//
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;//向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /*配置输出比较，产生占空比为20%的PWM方波*/
    TIM_OCStructInit(&TIM_OCInitStructure);  //填入缺省值，其中后几项只对TIM1和TIM8有效
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //PWM1为正常占空比模式，PWM2为反极性模式
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0; //输入CCR（占空比数值）
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //High为占空比高极性，此时占空比为20%；Low则为反极性，占空比为80%
    /*Green Tim_Config*/
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的
    /*Blue Tim_Config*/
    TIM_OC2Init(TIM3, &TIM_OCInitStructure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);//CCR自动装载默认也是打开的
				
    TIM_ARRPreloadConfig(TIM3, ENABLE); //ARR自动装载默认是打开的，可以不设置

    TIM_Cmd(TIM3, ENABLE); //使能TIM3定时器
    TIM_CtrlPWMOutputs(TIM3,ENABLE);
}

void Init_NoPwm_LED(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;	
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_StructInit(&GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LEDR;
    GPIO_Init(GPIO_PORT_LEDR, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LEDG;
    GPIO_Init(GPIO_PORT_LEDG, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_PIN_LEDB;
    GPIO_Init(GPIO_PORT_LEDB, &GPIO_InitStructure);
}

void bsp_InitLed(void)
{
    Init_Pwm_LED_GPIO();
//   Init_NoPwm_LED();
    LedR_TIM_Config();	
    LedG_B_TIM_Config();
}

void bsp_LedRed(void)
{
    GPIO_PORT_LEDR->BSRRL = GPIO_PIN_LEDR;	
    GPIO_PORT_LEDG->BSRRH = GPIO_PIN_LEDG;	
    GPIO_PORT_LEDB->BSRRH = GPIO_PIN_LEDB;
}

void bsp_LedGreen(void)
{
    GPIO_PORT_LEDR->BSRRH = GPIO_PIN_LEDR;	
    GPIO_PORT_LEDG->BSRRL = GPIO_PIN_LEDG;	
    GPIO_PORT_LEDB->BSRRH = GPIO_PIN_LEDB;
}

void bsp_LedBlue(void)
{
    GPIO_PORT_LEDR->BSRRH = GPIO_PIN_LEDR;	
    GPIO_PORT_LEDG->BSRRH = GPIO_PIN_LEDG;	
    GPIO_PORT_LEDB->BSRRL = GPIO_PIN_LEDB;
}

void bsp_LedOff(void)
{
    GPIO_PORT_LEDR->BSRRH = GPIO_PIN_LEDR;	
    GPIO_PORT_LEDG->BSRRH = GPIO_PIN_LEDG;	
    GPIO_PORT_LEDB->BSRRH = GPIO_PIN_LEDB;
}

void set_led_color(uint16_t R_value,uint16_t G_value,uint16_t B_value)
{
  if(R_value > 99 || G_value > 99 || B_value > 99)
    return;
  
    TIM2->CCR2 = R_value;
    TIM3->CCR1 = G_value;
    TIM3->CCR2 = B_value;  
}

static void led_init_handle(void *inContext)
{
    nearair_Context_t *airContent = inContext;
    static char color_count = 0;
    if( MODE_INIT != airContent->deviceStatus.mode )
    {
      mico_stop_timer(&_led_init_timer);
      mico_deinit_timer(&_led_init_timer);
    }
    switch(color_count)
    {
    case 0:
      set_led_color(99,0,0);
      color_count = 1;
      break;
    case 1:
      set_led_color(99,50,0);
      color_count = 2;
      break;
    case 2:
      set_led_color(0,0,99);
      color_count = 0;
      break;
    default:
      break;
    }
}

static void led_sleep_handle(void *inContext)
{
    nearair_Context_t *airContent = inContext;
    static uint8_t count = 0;
    static uint8_t R;
    uint8_t pluse_count = 0;
    if(MODE_SLEEP != airContent->deviceStatus.mode)
    {
        mico_stop_timer(&_led_sleep_timer);
        mico_deinit_timer(&_led_sleep_timer);
    }
    switch(airContent->sensorStatus.light)
    {
    case NIGHT:
      R = 130;
      break;
    case DAY:
      R = 100;
      break;
    default:
      break;
    }
    count += 1;
    if(count > 200)
    {
      count = 0;
      return;
    }
    if(count < 100)
    {
      pluse_count = R - (int)sqrt(R*R - count*count);
    }
    if(count > 100)
    {
      pluse_count = R - (int)sqrt(R*R - (200 - count)*(200 - count));
      
    }
    if(pluse_count < 3)//避免灯灭导致闪烁感
      return;
    set_led_color(0,0,pluse_count);
/*    switch(airContext->airStatus.airclass)
    {
    case AQ_GOOD:
      set_led_color(0,0,pluse_count);
      break;
    case AQ_SOSO:
      set_led_color(pluse_count,2+(int)(0.3*pluse_count),0);
      break;
    case AQ_BAD:
      set_led_color(pluse_count,0,0);
      break;
    default:
      break;
    }*/
}

void led_thread(void *inContext)
{
    led_log_trace();
    nearair_Context_t *airContext = inContext;
    char mode_old;
    char airclass_old;
    bsp_InitLed();
    mode_old = airContext->deviceStatus.mode;
    airclass_old = airContext->airStatus.airclass;
    
    mico_init_timer(&_led_init_timer, 500,led_init_handle, inContext);
    mico_init_timer(&_led_sleep_timer,20, led_sleep_handle,inContext);
    while(1)
    {
        if(mode_old != airContext->deviceStatus.mode)
        {
            if(mode_old == MODE_INIT)
               mico_stop_timer(&_led_init_timer);
            if(mode_old == MODE_SLEEP)
               mico_stop_timer(&_led_sleep_timer);
            
            mode_old = airContext->deviceStatus.mode;
            
            switch(mode_old)
            {
              case MODE_CLOSE:
                set_led_color(0,0,0);
                break;
              case MODE_INIT:
                mico_start_timer(&_led_init_timer);          
                break;
              case MODE_SMART:
                switch( airContext->airStatus.airclass )
                {
                  case AQ_GOOD:
                    set_led_color(0,0,99);
                    break;
                  case AQ_SOSO:
                    set_led_color(99,33,0);
                    break;
                  case AQ_BAD:
                    set_led_color(99,0,0);
                    break;
                  default:
                    break;            
                }          
                break;
              case MODE_SLEEP:
                mico_start_timer(&_led_sleep_timer);    
                break;
              case MODE_FLY:
                set_led_color(0,99,0);
                break;
              default:
                break;
            }
        }
        if(mode_old == MODE_SMART)
        {
            if(airclass_old != airContext->airStatus.airclass)
            {
                airclass_old = airContext->airStatus.airclass;
                switch( airclass_old )
                {
                  case AQ_GOOD:
                    set_led_color(0,0,99);
                    break;
                  case AQ_SOSO:
                    set_led_color(99,33,0);
                    break;
                  case AQ_BAD:
                    set_led_color(99,0,0);
                    break;
                  default:
                    break;            
                }
            }
        }
        if(airContext->setStatus.led_enable)
        {
          set_led_color(airContext->setStatus.led_r,airContext->setStatus.led_g,\
            airContext->setStatus.led_b);
        }
    }
}
