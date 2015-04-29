/**
  ******************************************************************************
  * @file    bsp_beep.c
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
#include "stm32f2xx.h"
#include "bsp_beep.h"
#include "bsp_buzzer.h"
#include "MICODefine.h"
#include "PlatformUart.h"

#define beep_log(M, ...) custom_log("BEEP", M, ##__VA_ARGS__)
#define beep_log_trace() custom_log_trace("BEEP")

static mico_timer_t _buzzer_timer;

void bsp_InitBeep(void)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
	
    BEEP_CLK_INIT(BEEP_CLK, ENABLE);	
		
    GPIO_InitStructure.GPIO_Pin = BEEP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;        //
    GPIO_Init(BEEP_PORT, &GPIO_InitStructure);
//    GPIO_SetBits(BEEP_PORT,BEEP_PIN);
    GPIO_PinAFConfig(BEEP_PORT, BEEP_PIN_SOURCE, GPIO_AF_TIM1);
	
    SWBEEP_CLK_INIT(BEEP_CLK, ENABLE);	
		
    GPIO_InitStructure.GPIO_Pin = SWBEEP_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode  =  GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType  = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;        //
    GPIO_Init(SWBEEP_PORT, &GPIO_InitStructure);
		
    GPIO_ResetBits(SWBEEP_PORT,SWBEEP_PIN);
    
}

void BeepPwrOn(void)
{
    GPIO_SetBits(SWBEEP_PORT,SWBEEP_PIN);
}

void BeepPwrOff(void)
{
    GPIO_ResetBits(SWBEEP_PORT,SWBEEP_PIN);
}

void BEEP_SetFreq(uint32_t iFreq)          //定时器PWM设置
{
    TIM_OCInitTypeDef    TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
		
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);            //开启定时器1的时钟。
		
    TIM_DeInit(TIM1);        //复位定时器4寄存器值。                                       
    TIM_TimeBaseStructure.TIM_Prescaler= 20-1;                   //12分频
    TIM_TimeBaseStructure.TIM_CounterMode= TIM_CounterMode_Up;           //TIM向上计数模式
    TIM_TimeBaseStructure.TIM_Period= 6000000 / iFreq - 1;//0xFFF ;       //设置自动重装载周期值
    TIM_TimeBaseStructure.TIM_ClockDivision= 0x0;       //设置时钟分割:TDTS = Tck_tim
    TIM_TimeBaseStructure.TIM_RepetitionCounter = TIM_CKD_DIV1;//0x0;//
    TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);                //根据指定的参数初始化TIMx

    TIM_OCStructInit(&TIM_OCInitStructure);
    TIM_OCInitStructure.TIM_OCMode= TIM_OCMode_PWM1;                                   //CH4PWM2模式
    TIM_OCInitStructure.TIM_OutputState= TIM_OutputState_Enable;               //比较输出使能
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;                   //OC4 低电平有效
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_Low;//
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set; 
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Reset;
	
    TIM_OC4Init(TIM1,&TIM_OCInitStructure);                                     //根据指定的参数初始化外设TIMx
    TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);  //CH4 预装载使能
    TIM_ARRPreloadConfig(TIM1,ENABLE);
    TIM1->CCR4 = (u16)((TIM1->ARR) * 50 / 100);          //pwm占空比设置
 //   TIM_Cmd(TIM1,ENABLE);                              //开户定时器4
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
}

void BEEP_On(void)
{
    TIM_Cmd(TIM1,ENABLE);
}

void BEEP_Off(void)
{
    TIM_Cmd(TIM1,DISABLE);
    TIM1->CNT = 0;
}

void beep_thread(void *inContext)
{
    beep_log_trace();
    nearair_Context_t *airContent = inContext;
    char temp_mode,temp_position,temp_lock;
    
    temp_mode = airContent->deviceStatus.mode;
    temp_position = airContent->deviceStatus.position;
    temp_lock = airContent->deviceStatus.child_lock;
    
    bsp_InitBeep();
    mico_init_timer(&_buzzer_timer,10,BuzzerCtrl,NULL);
    mico_start_timer(&_buzzer_timer);
    while(1)
    {
        if(temp_mode != airContent->deviceStatus.mode)
        {            
            if( MODE_CLOSE == temp_mode )
            {
              BuzzerStart( POLY_ON );           
              temp_mode = airContent->deviceStatus.mode;   
            }
            else
            {
              temp_mode = airContent->deviceStatus.mode;
              switch( temp_mode )
              {
                case MODE_CLOSE:
                  BuzzerStart( POLY_OFF );
                  break;
                case MODE_INIT:
                  BuzzerStart( POLY_ON );
                  break;
                case MODE_SMART:
                  BuzzerStart( MONO );
                  break;
                case MODE_SLEEP:
                  BuzzerStart( MONO );
                  break;
                case MODE_FLY:
                  BuzzerStart( MONO );
                  break;
                default:
                  break;
              }
            }
        }
        if( MODE_MANUAL == airContent->deviceStatus.mode )
        {
          
          if(temp_position != airContent->deviceStatus.position)
          {
            beep_log("mode_manual");
            temp_position = airContent->deviceStatus.position;
            switch(temp_position)
            {
            case MOTOR_LOW:
            case MOTOR_MIDIUM:
            case MOTOR_HIGH:
            case MODE_FLY:
              BuzzerStart( MONO );
              break;
            default:
              break;
            }            
          }         
        }
        if(temp_lock != airContent->deviceStatus.child_lock  )
        {
          temp_lock = airContent->deviceStatus.child_lock;
          BuzzerStart( MONO );
        }
    }
}

