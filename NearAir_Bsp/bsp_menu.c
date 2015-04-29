/* Includes ------------------------------------------------------------------*/

#include "stdio.h"
#include "string.h"
#include <ctype.h>                    /* character functions                 */

#include "platform.h"
#include "bsp_menu.h"
#define menu_log(M, ...) custom_log("MENU", M, ##__VA_ARGS__)
#define menu_log_trace() custom_log_trace("MENU")

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define MAX_CMD_LEN  200
#define CNTLQ      0x11
#define CNTLS      0x13
#define DEL        0x7F
#define BACKSPACE  0x08
#define CR         0x0D
#define LF         0x0A
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const char menu[] =
   "\n"
   "+***************(C) COPYRIGHT 2014 NEAR corporation**************+\n"
   "|          EMW3162 SET and EasyLink configuration NearAir        |\n"
   "+ command ----------------+ function ----------------------------+\n"
   "| 1:SetSpeed              | Moter Speed Setup(0~100)             |\n"
   "| 2:SetRGB                | LED RGB Setup(00.00.00~100.100.100)  |\n"
   "| 3:EasyLink_V2           | One step configuration from MXCHIP   |\n"
   "| 4:REBOOT                | Reboot                               |\n"
   "| ?:HELP                  | displays this help                   |\n"
   "+-------------------------+--------------------------------------+\n"
   "|                           By Adam Huang from NEAR AIR Team     |\n"
   "+----------------------------------------------------------------+\n";
int menu_enable = 1;
#define ERROR_STR   "*** ERROR: %s"    /* ERROR message string in code   */
int  inputCount = 0;
char cmdbuf [MAX_CMD_LEN] = {0};

int SetEnable = 0;
static uint8_t motor = 0,led_r = 0,led_g = 0,led_b = 0;
//extern const char menu[];

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Analyse a command parameter
  * @param  commandBody: command string address
  * @param  para: The para we are looking for
  * @param  paraBody: A pointer to the buffer to receive the para body.
  * @param  paraBodyLength: The length, in bytes, of the buffer pointed to by the paraBody parameter.
  * @retval the actual length of the paraBody received, -1 means failed to find this paras 
  */
int findCommandPara(char *commandBody, char para, char *paraBody, int paraBodyLength)
{
  int i = 0;
  int k, j;
  int retval = -1;
  para = toupper(para);
  while(commandBody[i] != 0) {
    if(commandBody[i] == '-' && commandBody[i+1] == para){   /* para found!             */
      retval = 0;
      for (k = i+2; commandBody[k] == ' '; k++);      /* skip blanks                 */
      for(j = 0; commandBody[k] != ' ' && commandBody[k] != 0 && commandBody[k] != '-'; j++, k++){   /* para body found!             */
          paraBody[j] = commandBody[k];
          retval ++;
          if( retval == paraBodyLength)
            return retval;
        }
    }
    i++;
  }
  return retval;
}


static int getline (void)  {
  char c;
	
	if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET)
		return 0;
	else{ 
		c = USART_ReceiveData(USARTx);
		if (c  == CR)  c = LF;     /* read character                 */
		if (c == BACKSPACE  ||  c == DEL)  {    /* process backspace              */
			if (inputCount != 0)  {
				inputCount--;                              /* decrement count                */
				putchar (BACKSPACE);                /* echo backspace                 */
				putchar (' ');
				putchar (BACKSPACE);
			}
			return 0;
		}
		else if (c != CNTLQ && c != CNTLS)  {   /* ignore Control S/Q             */ 
			putchar(c);                  /* echo and store character       */
			cmdbuf[inputCount] = c;
			inputCount++; 
			if(inputCount < MAX_CMD_LEN - 1  &&  c != LF)
				return 0;
			else{
				cmdbuf[--inputCount] = 0; 
				return 1;
			}
		}
	}
	return 0;
}

static int setup_enable(nearair_Context_t *inContext)
{
   char c;
        if( SetEnable )
        {	
            if(USART_GetFlagStatus(USARTx, USART_FLAG_RXNE) == RESET)
		return 0;
            else 
		c = USART_ReceiveData(USARTx);
                
            if(c == 'q')//c = q,exit
            {
                SetEnable = 0;
                inContext->setStatus.motor_enable = 0;
                inContext->setStatus.led_enable = 0;
                if(menu_enable)
                    menu_log("NEAR> ");
                return 1;
            }
           if(1 == SetEnable)
           {
              if(c == 'u')//'u',up
              {
                if( motor < 100)
                  motor ++;
              }
              if(c == 'j')//'j',down
              {
                if( motor > 0)
                  motor --;
              }
              inContext->setStatus.motor = motor;
              menu_log("motor %d",motor);
              return 0;
           }
           if(2 == SetEnable)
           {
             //led_r setup
              if(c == 'u')//'u',up
              {
                if( led_r < 100)
                  led_r ++;
              }
              if(c == 'j')//'j',down
              {
                if( led_r > 0)
                  led_r --;
              }
              //led_g setup
              if(c == 'i')//'i',up
              {
                if( led_g < 100)
                  led_g ++;
              }
              if(c == 'k')//'k',down
              {
                if( led_g > 0)
                  led_g --;
              }
              //led_b setup
              if(c == 'o')//'o',up
              {
                if( led_b < 100)
                  led_b ++;
              }
              if(c == 'l')//'l',down
              {
                if( led_b > 0)
                  led_b --;
              }
              inContext->setStatus.led_r = led_r;
              inContext->setStatus.led_g = led_g;
              inContext->setStatus.led_b = led_b;
              menu_log("led_r %d,led_g %d,led_b %d",led_r,led_g,led_b);
              return 0;
           }
           return 0;
        }
        else
          return 1;
}
/**
  * @brief  Display the Main Menu on HyperTerminal
  * @param  None
  * @retval None
  */
void Main_Menu(nearair_Context_t *inContext)
{
  char cmdname[15] = {0};                            /* command input buffer        */
  int i, j;                                       /* index for command buffer    */
  
        if(setup_enable(inContext) == 0)
          return;
	if(getline()==0)                              /* input command line          */
		return;

	for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks on head         */
	for (; cmdbuf[i] != 0; i++)  {             /* convert to upper characters */
		cmdbuf[i] = toupper(cmdbuf[i]);
	}

	for (i = 0; cmdbuf[i] == ' '; i++);        /* skip blanks on head         */
	for(j=0; cmdbuf[i] != ' '&&cmdbuf[i] != 0; i++,j++)  {         /* find command name       */
		cmdname[j] = cmdbuf[i];
	}
	cmdname[j] = '\0';
        //strlen(cmdname)        
	/***************** Command: WPS configuration  *************************/
	if(strcmp(cmdname, "SetSpeed") == 0 || strcmp(cmdname, "1") == 0) {
		menu_log ("Moter Speed Setup(00~99)"); 
                SetEnable = 1;
                inContext->setStatus.motor_enable = 1;
	}
	/***************** Command: Easylink configuration  *************************/
	else if(strcmp(cmdname, "SetRGB") == 0 || strcmp(cmdname, "2") == 0)	{
		menu_log ("LED RGB Setup(000000~999999)");
                SetEnable = 2;
                inContext->setStatus.led_enable = 1;
	}
	/***************** Command: Easylink configuration  v2*************************/
	else if(strcmp(cmdname, "EasyLink") == 0 || strcmp(cmdname, "3") == 0)	{
		menu_log ("EasyLink  started......, start your easylink function on iOS/Android APP");
                PlatformEasyLinkButtonClickedCallback(); 
	}
 /***************** Command: Reboot *************************/
	else if(strcmp(cmdname, "REBOOT") == 0 || strcmp(cmdname, "4") == 0)  {
		NVIC_SystemReset();                            
	}

	else if(strcmp(cmdname, "HELP") == 0 || strcmp(cmdname, "?") == 0)	{
		menu_log ("%s", menu);                         /* display command menu        */
	}

	else if(strcmp(cmdname, "") == 0 )	{                         
	}
	
	else{
		menu_log (ERROR_STR, "UNKNOWN COMMAND");
	}
	
	memset(cmdbuf, 0x0, MAX_CMD_LEN);
	inputCount = 0;

	if(menu_enable && !SetEnable)
            menu_log("NEAR> ");
}

/**
  * @}
  */

/*******************(C)COPYRIGHT 2011 STMicroelectronics *****END OF FILE******/
