/**
  ******************************************************************************
  * @file    PlatformFlash.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides flash operation functions.
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


/* Includes ------------------------------------------------------------------*/
#include "PlatformLogging.h"
#include "PlatformFlash.h"
#include "stdio.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint32_t _GetSector(uint32_t Address);
static OSStatus _PlatformFlashByteWrite(__IO uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength);

OSStatus PlatformFlashInitialize( void )
{ 
  plat_log_trace();
  FLASH_Unlock(); 
  /* Clear pending flags (if any) */  
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | 
                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
  return kNoErr;
}

OSStatus PlatformFlashErase(uint32_t StartAddress, uint32_t EndAddress)
{
  plat_log_trace();
  OSStatus err = kNoErr;
  uint32_t StartSector, EndSector, i = 0;

  /* Get the sector where start the user flash area */
  StartSector = _GetSector(StartAddress);
  EndSector = _GetSector(EndAddress);

  for(i = StartSector; i <= EndSector; i += 8)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */
    require_action(FLASH_EraseSector(i, VoltageRange_3) == FLASH_COMPLETE, exit, err = kWriteErr); 
  }
  
exit:
  return err;
}

OSStatus PlatformFlashWrite(__IO uint32_t* FlashAddress, uint32_t* Data ,uint32_t DataLength)
{
  plat_log_trace();
  OSStatus err = kNoErr;
  uint32_t i = 0;
  uint32_t dataInRam;
	u8 startNumber;
	uint32_t DataLength32 = DataLength;
	
	/*First bytes that are not 32bit align*/
	if(*FlashAddress%4){
		startNumber = 4-(*FlashAddress)%4;
		err = _PlatformFlashByteWrite(FlashAddress, (uint8_t *)Data, startNumber);
    require_noerr(err, exit);
		DataLength32 = DataLength - startNumber;
		Data = (uint32_t *)((u32)Data + startNumber);
	}
	
	/*Program flash by words*/
  for (i = 0; (i < DataLength32/4) && (*FlashAddress <= (FLASH_END_ADDRESS-3)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    dataInRam = *(Data+i);
    require_action(FLASH_ProgramWord(*FlashAddress, dataInRam) == FLASH_COMPLETE, exit, err = kWriteErr); 
    require_action(*(uint32_t*)*FlashAddress == dataInRam, exit, err = kChecksumErr); 
    /* Increment FLASH destination address */
    *FlashAddress += 4;
  }
	
	/*Last bytes that cannot be write by a 32 bit word*/
	err = _PlatformFlashByteWrite(FlashAddress, (uint8_t *)Data + i*4, DataLength32-i*4);
  require_noerr(err, exit);

exit:
  return err;
}

OSStatus PlatformFlashFinalize( void )
{
  FLASH_Lock();
  return kNoErr;
}


OSStatus _PlatformFlashByteWrite(__IO uint32_t* FlashAddress, uint8_t* Data ,uint32_t DataLength)
{
  uint32_t i = 0;
  uint32_t dataInRam;
  OSStatus err = kNoErr;

  for (i = 0; (i < DataLength) && (*FlashAddress <= (FLASH_END_ADDRESS)); i++)
  {
    /* Device voltage range supposed to be [2.7V to 3.6V], the operation will
       be done by word */ 
    dataInRam = *(uint8_t*)(Data+i);
    
    require_action(FLASH_ProgramByte(*FlashAddress, dataInRam) == FLASH_COMPLETE, exit, err = kWriteErr); 
    require_action(*(uint8_t*)*FlashAddress == dataInRam, exit, err = kChecksumErr); 
    *FlashAddress +=1;
  }

exit:
  return err;
}

/**
  * @brief  Returns the write protection status of user flash area.
  * @param  None
  * @retval 0: No write protected sectors inside the user flash area
  *         1: Some sectors inside the user flash area are write protected
  */
uint16_t _PlatformFlashGetWriteProtectionStatus(void)
{
  uint32_t UserStartSector = FLASH_Sector_1;

  /* Get the sector where start the user flash area */
  UserStartSector = _GetSector(APPLICATION_START_ADDRESS);

  /* Check if there are write protected sectors inside the user flash area */
  if ((FLASH_OB_GetWRP() >> (UserStartSector/8)) == (0xFFF >> (UserStartSector/8)))
  { /* No write protected sectors inside the user flash area */
    return 1;
  }
  else
  { /* Some sectors inside the user flash area are write protected */
    return 0;
  }
}

/**
  * @brief  Disables the write protection of user flash area.
  * @param  None
  * @retval 1: Write Protection successfully disabled
  *         2: Error: Flash write unprotection failed
  */
uint32_t _PlatformFlashDisableWriteProtection(void)
{
  __IO uint32_t UserStartSector = FLASH_Sector_1, UserWrpSectors = OB_WRP_Sector_1;

  /* Get the sector where start the user flash area */
  UserStartSector = _GetSector(APPLICATION_START_ADDRESS);

  /* Mark all sectors inside the user flash area as non protected */  
  UserWrpSectors = 0xFFF-((1 << (UserStartSector/8))-1);
   
  /* Unlock the Option Bytes */
  FLASH_OB_Unlock();

  /* Disable the write protection for all sectors inside the user flash area */
  FLASH_OB_WRPConfig(UserWrpSectors, DISABLE);

  /* Start the Option Bytes programming process. */  
  if (FLASH_OB_Launch() != FLASH_COMPLETE)
  {
    /* Error: Flash write unprotection failed */
    return (2);
  }

  /* Write Protection successfully disabled */
  return (1);
}

/**
  * @brief  Gets the sector of a given address
  * @param  Address: Flash address
  * @retval The sector of a given address
  */
static uint32_t _GetSector(uint32_t Address)
{
  uint32_t sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_Sector_0;  
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_Sector_1;  
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_Sector_2;  
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_Sector_3;  
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_Sector_4;  
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_Sector_5;  
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_Sector_6;  
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_Sector_7;  
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_Sector_8;  
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_Sector_9;  
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_Sector_10;  
  }
  else/*(Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11))*/
  {
    sector = FLASH_Sector_11;  
  }
    return sector;
}
