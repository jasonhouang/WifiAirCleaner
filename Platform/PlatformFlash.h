/**
  ******************************************************************************
  * @file    PlatformFlash.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of flash operation functions.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PLATFORM_FLASH_H
#define __PLATFORM_FLASH_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx.h"
#include "Common.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Base address of the Flash sectors */
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 16 Kbyte */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08004000) /* Base @ of Sector 1, 16 Kbyte */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08008000) /* Base @ of Sector 2, 16 Kbyte */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x0800C000) /* Base @ of Sector 3, 16 Kbyte */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08010000) /* Base @ of Sector 4, 64 Kbyte */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08020000) /* Base @ of Sector 5, 128 Kbyte */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08040000) /* Base @ of Sector 6, 128 Kbyte */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x08060000) /* Base @ of Sector 7, 128 Kbyte */
#define ADDR_FLASH_SECTOR_8     ((uint32_t)0x08080000) /* Base @ of Sector 8, 128 Kbyte */
#define ADDR_FLASH_SECTOR_9     ((uint32_t)0x080A0000) /* Base @ of Sector 9, 128 Kbyte */
#define ADDR_FLASH_SECTOR_10    ((uint32_t)0x080C0000) /* Base @ of Sector 10, 128 Kbyte */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbyte */

/* End of the Flash address */
#define FLASH_START_ADDRESS     (uint32_t)0x08000000  
#define FLASH_END_ADDRESS       (uint32_t)0x080FFFFF
#define FLASH_SIZE        			(FLASH_END_ADDRESS -  FLASH_START_ADDRESS + 1)


/* Define the address from where user application will be loaded.
   Note: the 1st sector 0x08000000-0x08003FFF is reserved for the IAP code */
#define APPLICATION_START_ADDRESS   (uint32_t)0x0800C200
#define APPLICATION_END_ADDRESS     (uint32_t)0x0805FFFF
#define USER_FLASH_SIZE             (APPLICATION_END_ADDRESS - APPLICATION_START_ADDRESS + 1)

#define UPDATE_START_ADDRESS        (uint32_t)0x08060000 
#define UPDATE_END_ADDRESS          (uint32_t)0x080BFFFF 
#define UPDATE_FLASH_SIZE           (UPDATE_END_ADDRESS - UPDATE_START_ADDRESS + 1)

#define BOOT_START_ADDRESS          (uint32_t)0x08000000 
#define BOOT_END_ADDRESS            (uint32_t)0x08003FFF 
#define BOOT_FLASH_SIZE             (BOOT_END_ADDRESS - BOOT_START_ADDRESS + 1)

#define DRIVER_START_ADDRESS        (uint32_t)0x080C0000 
#define DRIVER_END_ADDRESS          (uint32_t)0x080FFFFF 
#define DRIVER_FLASH_SIZE           (DRIVER_END_ADDRESS - DRIVER_START_ADDRESS + 1)

#define PARA_START_ADDRESS          (uint32_t)0x08004000 
#define PARA_END_ADDRESS            (uint32_t)0x08007FFF
#define PARA_FLASH_SIZE             (PARA_END_ADDRESS - PARA_START_ADDRESS + 1)  
/*
#define NEAR_PARA_START_ADDRESS     (uint32_t)0x08006000 
#define NEAR_PARA_END_ADDRESS       (uint32_t)0x08007FFF
#define NEAR_PARA_FLASH_SIZE        (PARA_END_ADDRESS - PARA_START_ADDRESS + 1)  
*/
#define EX_PARA_START_ADDRESS       (uint32_t)0x08008000 
#define EX_PARA_END_ADDRESS         (uint32_t)0x0800BFFF
#define EX_PARA_FLASH_SIZE          (PARA_END_ADDRESS - PARA_START_ADDRESS + 1)  

#define BACKUP_PARA_START_ADDRESS   (uint32_t)0x08008000 
#define BACKUP_PARA_END_ADDRESS     (uint32_t)0x0800BFFF
#define BACKUP_PARA_FLASH_SIZE      (BACKUP_PARA_END_ADDRESS - BACKUP_PARA_START_ADDRESS + 1)

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformWDGInitialize
    @abstract   Performs any platform-specific initialization needed. 
*/
OSStatus PlatformFlashInitialize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformFlashErase
    @abstract   This function does an erase of all user flash area
    @param      StartSector: start of user flash area
    @param      EndAddress:  end of user flash area
*/
OSStatus PlatformFlashErase(uint32_t StartAddress, uint32_t EndAddress);

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformFlashWrite
    @abstract   This function writes a data buffer in flash (data are 32-bit aligned).
    @note       After writing data buffer, the flash content is checked.
  * @param      FlashAddress: pointer on start address for writing data buffer
  * @param      Data: pointer on data buffer
  * @param      DataLength: length of data buffer (unit is 8-bit bytes) 
*/
OSStatus PlatformFlashWrite(__IO uint32_t* FlashAddress, uint32_t* Data, uint32_t DataLength);

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformFlashFinalize
    @abstract   Performs any platform-specific cleanup needed.
*/
OSStatus PlatformFlashFinalize( void );

#endif  /* __PLATFORM_FLASH_H */
