#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H
#include "stm32f2xx.h"
#include "NearAir.h"
/* Definition for ButtonUsr resources **********************************************/
#define ButtonUsr_CLK_INIT		        	RCC_AHB1PeriphClockCmd

#define ButtonUsr_PIN         	                        GPIO_Pin_3
#define ButtonUsr_IRQ_PIN				3
#define ButtonUsr_PORT   				GPIOA
#define ButtonUsr_CLK    				RCC_AHB1Periph_GPIOA
#define ButtonUsr_IRQ                                   EXTI2_IRQn
#define ButtonUsr_EXTI_Line                             EXTI_Line2

#define ShutDown_TimeOut                                3000/*press down usrkey to shutdown for 3sec*/

enum {
	CHILD_UNLOCK = 0,
	CHILD_LOCKED,
};

//__weak void _button_Usr_irq_handler(void *arg);
extern void bsp_InitButton(nearair_Context_t * const airContent);
extern void button_cmd(FunctionalState NewState);
//extern uint8_t read_button_state(void);

#endif
