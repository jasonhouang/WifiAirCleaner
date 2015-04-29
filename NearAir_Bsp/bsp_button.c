#include "stdio.h"
#include "stm32f2xx.h"
#include "Platform.h"
#include "bsp_button.h"
#include "NearAirDefine.h"
//#include "NearAirProtocol.h"
#include "PlatformUART.h"

#define button_log(M, ...) custom_log("Button", M, ##__VA_ARGS__)
#define button_log_trace() custom_log_trace("Button")
static mico_timer_t _button_Usr_timer;

static void _button_Usr_irq_handler(void *arg);
static void _button_Usr_Timeout_handler( void* arg );
static uint32_t _default_start_time = 0;

void bsp_InitButton(nearair_Context_t * const airContent)
{
    GPIO_InitTypeDef   GPIO_InitStructure;
    NVIC_InitTypeDef   NVIC_InitStructure;
    
    ButtonUsr_CLK_INIT(ButtonUsr_CLK, ENABLE);
	
    GPIO_InitStructure.GPIO_Pin = ButtonUsr_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;//GPIO_PuPd_NOPULL;			
    GPIO_Init(ButtonUsr_PORT, &GPIO_InitStructure);
	
    mico_init_timer(&_button_Usr_timer, ShutDown_TimeOut, _button_Usr_Timeout_handler, airContent);
    gpio_irq_enable(ButtonUsr_PORT, ButtonUsr_IRQ_PIN, IRQ_TRIGGER_BOTH_EDGES, _button_Usr_irq_handler, airContent);
    
    NVIC_InitStructure.NVIC_IRQChannel = ButtonUsr_IRQ;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

static void _button_Usr_irq_handler(void *arg)
{
  nearair_Context_t *airContent = arg;
  int interval = -1;

  if (GPIO_ReadInputDataBit(ButtonUsr_PORT, ButtonUsr_PIN) == 0) {
    _default_start_time = mico_get_time()+1;
    mico_start_timer(&_button_Usr_timer);
  } else {
    interval = mico_get_time() + 1 - _default_start_time;
    if ( (_default_start_time != 0) && interval > 50 && interval < ShutDown_TimeOut){
      /* UsrKey button clicked once */
      if(CHILD_LOCKED == airContent->deviceStatus.child_lock  )
        return;      
      switch( airContent->deviceStatus.mode )
      {
      case MODE_CLOSE:
        airContent->powerStatus.power = POWER_ON;
//        airContent->deviceStatus.mode = MODE_INIT;
        airContent->deviceStatus.mode = MODE_SMART;
        break;
      case MODE_INIT:
        airContent->deviceStatus.mode = MODE_SMART;
        break;
      case MODE_SMART:
        airContent->deviceStatus.mode = MODE_FLY;
        break;
      case MODE_SLEEP:
        airContent->deviceStatus.mode = MODE_SMART;
        break;
      case MODE_FLY:
        airContent->deviceStatus.mode = MODE_SLEEP;
        break;
      default:
        break;
      }
      if( airContent->deviceStatus.isRemoteLogined == true )
          mico_rtos_set_semaphore(&airContent->deviceStatus.dev_status_change_sem);
//      button_log("airContent->deviceStatus.mode is:%d!",airContent->deviceStatus.mode);
//      button_log("airContent->powerStatus.power is:%d!",airContent->powerStatus.power);
    }
    mico_stop_timer(&_button_Usr_timer);
    _default_start_time = 0;
  }
}

static void _button_Usr_Timeout_handler(void* arg )
{
  nearair_Context_t *airContent = arg;
  _default_start_time = 0;
      
  if( CHILD_LOCKED == airContent->deviceStatus.child_lock )
  {
    airContent->deviceStatus.child_lock = CHILD_UNLOCK;
    goto exit;
  }
  if(POWER_ON == airContent->powerStatus.power)
  {
    airContent->powerStatus.power = POWER_OFF;
    airContent->deviceStatus.mode = MODE_CLOSE;
  }
  goto exit;
exit:
  if( airContent->deviceStatus.isRemoteLogined == true )
          mico_rtos_set_semaphore(&airContent->deviceStatus.dev_status_change_sem);
//  button_log("airContent->deviceStatus.mode is:%d!",airContent->deviceStatus.mode);
//  button_log("airContent->powerStatus.power is:%d!",airContent->powerStatus.power);
}
