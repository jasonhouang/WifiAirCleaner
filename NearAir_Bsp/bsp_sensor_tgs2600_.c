/**
  ******************************************************************************
  * @file    bsp_sensor_tgs2600.c
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
#include "bsp_sensor_tgs2600.h"
#include "stdint.h"
#include "bsp_adc.h"
#include "stdio.h"
#include "string.h"

#include "PlatformUART.h"

#define tgs_log(M, ...) custom_log("TGS", M, ##__VA_ARGS__)
#define tgs_log_trace() custom_log_trace("TGS")

#define TIME_SECTION 30000//1200000 //20min一个区间

static float Rmax = 0;//传感器基准值
static float This_Rmax = 0;//传感器在新的20min内的最大值
char AQBAD_ON = 0;//ON=1:空气质量越来越差;ON=0:空气质量越来越好
float L;//L = Rs/Rmax
char  D;//污染等级,0到3为空气质量逐渐变差
uint16_t TGS_ADC_Value[8][8];//滤波250ms取8次，2s进行一次平均

volatile static uint16_t TGS_ADC_Mean_Value;
static float Rs = 0;//当前2s计算得到的传感器阻值
static float Last_Rs = 0;//前2s计算得到的传感器阻值
static uint32_t time_begin = 0;
float get_transform_Res_value(const uint16_t ADC_Value);
static AIR_OQuality air_quality = AIR_OGREAT;
static void judge_AQ_dgree(void);
static mico_timer_t _tgstick_timer;

void get_TGS_ADC_Value(void *arg);
void renew_MAXR(void);
uint32_t get_time_interval(void);

AIR_OQuality read_air_odour_quality(void)
{
    return air_quality;
}
/*
void debug_route(void *arg)
{
    (void)(arg);
    air_quality = tgs2600_monitor_routine();
	
    tgs_log("TGS_RES_Value is:%d,air_quality is:%d",(int)(100*Rs),air_quality);
	
}*/

void bsp_sensor_init_TGS2600(void)
{
	memset(TGS_ADC_Value,0x0,sizeof(TGS_ADC_Value));
	time_begin = mico_get_time();
        
        mico_init_timer(&_tgstick_timer,250,get_TGS_ADC_Value,NULL);
        mico_start_timer(&_tgstick_timer);
}
//每250ms采集一次数据
void get_TGS_ADC_Value(void *arg)
{
    (void)(arg);
    static uint32_t sum = 0;
    static int j = 0;	
        
    for(int i=0;i<8;i++)
    {
        TGS_ADC_Value[j][i] = get_tgs2600_adc_new_value();
    }
    if(8 == (++j))//每2s将8次采集平均一次
    {
        j = 0;
        for(int m=0;m<8;m++)
        {
            for(int n=0;n<8;n++)
            {
                sum+=TGS_ADC_Value[m][n];
            }
        }
        TGS_ADC_Mean_Value = sum/64;
        Rs = get_transform_Res_value(TGS_ADC_Mean_Value);
        renew_MAXR();
        sum = 0;
    }
}

/*******************************************
*函数原型：float get_transform_Res_value(uint16_t ADC_Value)
*功能：把测量的ADC值转化为传感器阻值，单位：K
*输入参数：ADC_Value:测得的ADC值
*输出参数：传感器的阻值,单位：K
*******************************************/
float get_transform_Res_value(const uint16_t ADC_Value)
{
	float tgs_rs_value;
	tgs_rs_value = ((5.0*10)/(ADC_Value*(3.3/((1<<12)-1)))-10);//VCC = 5.0V;RL = 10K;Vref = 3.3V
	return tgs_rs_value;
}

void renew_MAXR(void)
{
	if(TIME_SECTION < get_time_interval())
	{
		time_begin = mico_get_time();
		Rmax = This_Rmax;
		This_Rmax = 0;
		tgs_log("new Rmax is:%d",(int)Rmax);
	}
	else
	{
		if(Rs > This_Rmax)
		{
			This_Rmax = Rs;
			if(This_Rmax > Rmax)
				Rmax = This_Rmax;
		}
	}
        if(Last_Rs > Rs)
          AQBAD_ON = 1;
        else
          AQBAD_ON = 0;
        Last_Rs = Rs;
        judge_AQ_dgree();
}

static void judge_AQ_dgree(void)
{
    L = Rs/Rmax;
    if(1 == AQBAD_ON)
    {
        if(L > 0.85)
          D = 0;
        else if(L > 0.77 && L <= 0.85)
          D = 1;
        else if(L > 0.69 && L <=0.77)
          D = 2;
        else
          D = 3;
    }
    else
    {
        if(L < 0.72)
          D = 3;
        else if(L >= 0.72 && L < 0.8)
          if(D > 2)
            D --;
        else if(L >= 0.8 && L < 0.88)
          if(D > 1)
            D --;
        else
          if(D > 0)
            D --;         
    }
}

void _get_air_odour_quality(void *inContext)
{
    nearair_Context_t *airContext = inContext;
    
    airContext->sensorStatus.odour = (int)Rs;
    
    airContext->airStatus.airclass = D;
}

uint32_t get_time_interval(void)
{
    return (mico_get_time() - time_begin);
}
