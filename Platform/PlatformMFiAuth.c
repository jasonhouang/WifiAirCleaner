/**
  ******************************************************************************
  * @file    PlatformMFiAuth.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    08-June-2014
  * @brief   This header contains function called by Apple WAC code that must be
  *          implemented by the platform. These functions are called when Apple 
  *          code needs to interact with the Apple Authentication Coprocessor. 
  *          Please refer to the relevant version of the "Auth IC" document to 
  *          obtain more details on how to interact with the Authentication 
  *          Coprocessor. This document can be found on the MFi Portal.
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


#include "PlatformLogging.h"
#include "PlatformMFiAuth.h"
#include "Platform.h"

#include "MICO.h"

#include "stm32f2xx.h"

#define CP_ADDRESS 0x20
//#define CP_ADDRESS 0xAE

#define I2C_SPEED                        100000
#define CP_LONG_TIMEOUT                 ((uint32_t)(10 * 0x1000))
#define CP_FLAG_TIMEOUT                 0x1000

#define CP_I2C_RETRY_TIMES              20

#define CP_RST_PIN                  GPIO_Pin_14                  /* PB.14 */
#define CP_PST_GPIO_PORT            GPIOB                       /* GPIOB */
#define CP_RST_GPIO_CLK             RCC_AHB1Periph_GPIOB

#define AH_DEVICE_VERSION                         0x00  // Length 1   R
#define AH_FIRMWARE_VERSON                        0x01  // Length 1   R
#define AH_AUTHENTICATION_PROTOCOL_MAJOR_VERSION  0x02  // Length 1   R
#define AH_AUTHENTICATION_PROTOCOL_MINOR_VERSION  0x03  // Length 1   R 
#define AH_DEVICE_ID                              0x04  // Length 4   R
#define AH_ERROR_CODE                             0x05  // Length 1   R

#define AH_AUTHENTICATION_CONTROL_STATUS          0x10  // Length 1
#define AH_CHALLENGE_RESPONSE_DATA_LENGTH         0x11  // Length 2   R/W
#define AH_CHALLENGE_RESPONSE_DATA                0x12  // Lenght 128 R/W
#define AH_CHALLENGE_DATA_LENGTH                  0x20  // Length 2   R/W
#define AH_CHALLENGE_DATA                         0x21  // Lenght 128 R/W

#define AH_ACCESSORY_CERTIFICATE_DATA_LENGTH      0x30  // Length 2   R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE1       0x31  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE2       0x32  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE3       0x33  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE4       0x34  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE5       0x35  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE6       0x36  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE7       0x37  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE8       0x38  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE9       0x39  // Length 128 R
#define AH_ACCESSORY_CERTIFICATE_DATA_PAGE10      0x3a  // Length 128 R

#define AH_SELF_TEST_CONTROL_STATUS               0x40  // Length 1   R/W
#define AH_SYSTEM_EVENT_COUNTER                   0x4D  // Length 1   R

#define  AH_IPOD_CERTIFICATE_DATA_LENGTH          0x50  // Length 2   R
#define  AH_IPOD_CERTIFICATE_DATA_PAGE1           0x51  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE2           0x52  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE3           0x53  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE4           0x54  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE5           0x55  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE6           0x56  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE7           0x57  // Length 128 R/W
#define  AH_IPOD_CERTIFICATE_DATA_PAGE8           0x58  // Length 128 R/W

static void CP_LowLevel_Init(void);
static OSStatus Wait_For_OPT_Finish(uint32_t Set_Status);
static OSStatus CP_ReadBuffer(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);
static OSStatus CP_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);


OSStatus PlatformMFiAuthInitialize( void )
{
    OSStatus err = kNoErr;
//    GPIO_InitTypeDef  GPIO_InitStructure; 

    // PLATFORM_TO_DO
    plat_log_trace();
    RCC_AHB1PeriphClockCmd(CP_RST_GPIO_CLK, ENABLE);
/*
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Pin = CP_RST_PIN;*/
    //GPIO_Init(CP_PST_GPIO_PORT, &GPIO_InitStructure);
          // GPIO_ResetBits(CP_PST_GPIO_PORT, CP_RST_PIN);
  // msleep(50);
  // GPIO_SetBits(CP_PST_GPIO_PORT, CP_RST_PIN);
  // msleep(50);

    return err;
}

void PlatformMFiAuthFinalize( void )
{
    // PLATFORM_TO_DO
    plat_log_trace();
    return;
}

OSStatus PlatformMFiAuthCreateSignature( const void *inDigestPtr,
                                         size_t     inDigestLen,
                                         uint8_t    **outSignaturePtr,
                                         size_t     *outSignatureLen )
{
    OSStatus err = kNoErr;
      uint8_t deviceVersion;

    plat_log_trace();


    // PLATFORM_TO_DO
    uint16_t Len_big = ((inDigestLen&0xFF00)>>8) + ((inDigestLen&0xFF)<<8);
    uint8_t control_status;

    err =  CP_BufferWrite((uint8_t *)(&Len_big), AH_CHALLENGE_DATA_LENGTH, 2);
    require( err == kNoErr, exit ) ;
          
    deviceVersion = 0;
    err = CP_ReadBuffer(&deviceVersion, AH_ERROR_CODE, 1);
    require( err == kNoErr, exit ) ;
    //wac_log("Write Digest data lengthstatus: %d", deviceVersion);

///////////////////////////////////////////////////////////////////////////////////////

    err =  CP_BufferWrite((uint8_t *)inDigestPtr, AH_CHALLENGE_DATA, inDigestLen);
    require( err == kNoErr, exit ) ;

    deviceVersion = 0;
    err = CP_ReadBuffer(&deviceVersion, AH_ERROR_CODE, 1);
    require( err == kNoErr, exit ) ;
    //wac_log("Write Digest data status: %d", deviceVersion);

///////////////////////////////////////////////////////////////////////////////////////

    control_status = 1;
    err =  CP_BufferWrite(&control_status, AH_AUTHENTICATION_CONTROL_STATUS, 1);
    require( err == kNoErr, exit ) ;

    deviceVersion = 0;
    err = CP_ReadBuffer(&deviceVersion, AH_ERROR_CODE, 1);
    require( err == kNoErr, exit ) ;
    //wac_log("Write control data status: %d", deviceVersion);
///////////////////////////////////////////////////////////////////////////////////////

    do{
      control_status = 0;
      err = CP_ReadBuffer(&control_status, AH_AUTHENTICATION_CONTROL_STATUS, 1);
      require( err == kNoErr, exit ) ;
      //wac_log("Read control: %d", control_status);


          deviceVersion = 0;
    err = CP_ReadBuffer(&deviceVersion, AH_ERROR_CODE, 1);
    require( err == kNoErr, exit ) ;
    //wac_log("Read control status: %d", deviceVersion);

    }while((control_status>>4)!=0x1);

///////////////////////////////////////////////////////////////////////////////////////

    err = CP_ReadBuffer((uint8_t *)&Len_big, AH_CHALLENGE_RESPONSE_DATA_LENGTH, 2);
    require( err == kNoErr, exit ) ;

    *outSignatureLen = (size_t)(((Len_big&0xFF00)>>8) + ((Len_big&0xFF)<<8));

    //wac_log("Response length: %d", *outSignatureLen);

    *outSignaturePtr = malloc(*outSignatureLen);

    err = CP_ReadBuffer(*outSignaturePtr, AH_CHALLENGE_RESPONSE_DATA, *outSignatureLen);
    require( err == kNoErr, exit ) ;

    return err;

exit:
    return kNotPreparedErr;
}

OSStatus PlatformMFiAuthCopyCertificate( uint8_t **outCertificatePtr, size_t *outCertificateLen )
{
    // PLATFORM_TO_DO
    OSStatus err = kNoErr;
    uint8_t page, pageNumber;
    uint16_t Len_big;
    plat_log_trace();
    uint8_t *obj;

    err = CP_ReadBuffer((uint8_t *)&Len_big, AH_ACCESSORY_CERTIFICATE_DATA_LENGTH, 2);  
    require( err == kNoErr, exit ) ;

    *outCertificateLen = (size_t)(((Len_big&0xFF00)>>8) + ((Len_big&0xFF)<<8));
    //wac_log("Certificate length: %d", *outCertificateLen);

     *outCertificatePtr = malloc(*outCertificateLen);
     obj = *outCertificatePtr;

    pageNumber = *outCertificateLen/128;

    for(page = AH_ACCESSORY_CERTIFICATE_DATA_PAGE1; page < AH_ACCESSORY_CERTIFICATE_DATA_PAGE1 + pageNumber; page++){
      err = CP_ReadBuffer(obj, page, 128);
      require( err == kNoErr, exit ) ;
      obj+=128;
      //wac_log("Read page number: %x", page);
    }

    pageNumber = (*outCertificateLen)%128;
    err = CP_ReadBuffer(obj, page, pageNumber);
    //wac_log("Read page number: %x,bytes: %d", page, pageNumber);
    
    return err;

exit: 
    return kNotPreparedErr;
}



void CP_LowLevel_Init(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure; 
  I2C_InitTypeDef  I2C_InitStructure;
   
  /*!< sEE_I2C Periph clock enable */
  RCC_APB1PeriphClockCmd(CP_I2C_CLK, ENABLE);
  
  /*!< sEE_I2C_SCL_GPIO_CLK and sEE_I2C_SDA_GPIO_CLK Periph clock enable */
  RCC_AHB1PeriphClockCmd(CP_I2C_SCL_GPIO_CLK | CP_I2C_SDA_GPIO_CLK, ENABLE);

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
  
  /* Reset sEE_I2C IP */
  RCC_APB1PeriphResetCmd(CP_I2C_CLK, ENABLE);
  
  /* Release reset signal of sEE_I2C IP */
  RCC_APB1PeriphResetCmd(CP_I2C_CLK, DISABLE);
    
  /*!< GPIO configuration */
  /* Connect PXx to I2C_SCL*/
  GPIO_PinAFConfig(CP_I2C_SCL_GPIO_PORT, CP_I2C_SCL_SOURCE, CP_I2C_SCL_AF);
  /* Connect PXx to I2C_SDA*/
  GPIO_PinAFConfig(CP_I2C_SDA_GPIO_PORT, CP_I2C_SDA_SOURCE, CP_I2C_SDA_AF);  
  
  /*!< Configure sEE_I2C pins: SCL */   
  GPIO_InitStructure.GPIO_Pin = CP_I2C_SCL_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CP_I2C_SCL_GPIO_PORT, &GPIO_InitStructure);
  /*!< Configure sEE_I2C pins: SDA */
  GPIO_InitStructure.GPIO_Pin = CP_I2C_SDA_PIN;
  GPIO_Init(CP_I2C_SDA_GPIO_PORT, &GPIO_InitStructure);

  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0xA0;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  
  /* sEE_I2C Peripheral Enable */
  I2C_Cmd(CP_I2C, ENABLE);
  /* Apply sEE_I2C configuration after enabling it */
  I2C_Init(CP_I2C, &I2C_InitStructure);
 
}



OSStatus CP_ReadBuffer(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
    OSStatus err = kNoErr;

    uint16_t  sCPAddress = CP_ADDRESS;   
    int retry = CP_I2C_RETRY_TIMES; 

    do{
      CP_LowLevel_Init();

      I2C_GenerateSTART(CP_I2C, ENABLE);                     // Send START condition
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_MODE_SELECT);     // Test on EV5 and clear it
      //require( err == kNoErr, exit ) ;

      I2C_Send7bitAddress(CP_I2C, sCPAddress, I2C_Direction_Transmitter);        // Send EEPROM address for write
      msleep(10);
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);   // Test on EV6 and clear it
      if(err == kNoErr) break;
    }while(retry--);

    require( retry, exit ) ;
    //wac_log("retry times: %d", CP_I2C_RETRY_TIMES - retry);

    I2C_SendData(CP_I2C, ReadAddr);          // Send the EEPROM's internal address to read from: MSB of the address first
    err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_BYTE_TRANSMITTED);    // Test on EV8 and clear it
    require( err == kNoErr, exit ) ;
    
    I2C_GenerateSTOP(CP_I2C, ENABLE);              // Send STOP Condition

    do{
      CP_LowLevel_Init();

      I2C_GenerateSTART(CP_I2C, ENABLE);                     // Send START condition
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_MODE_SELECT);     // Test on EV5 and clear it
      //require( err == kNoErr, exit ) ;

      I2C_Send7bitAddress(CP_I2C, sCPAddress, I2C_Direction_Receiver);        // Send EEPROM address for write
      msleep(10);
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);   // Test on EV6 and clear it
      if(err == kNoErr) break;
    }while(retry--);

    while(NumByteToRead)                            // While there is data to be read
    {
        if(NumByteToRead == 1)
        {
            //I2C_AcknowledgeConfig(CP_I2C, DISABLE);        // Disable Acknowledgement
        }
        if(I2C_CheckEvent(CP_I2C, I2C_EVENT_MASTER_BYTE_RECEIVED)) // Test on EV7 and clear it
        {
            *pBuffer = I2C_ReceiveData(CP_I2C);        // Read a byte from the EEPROM
            pBuffer++;                              // Point to the next location where the byte read will be saved
            NumByteToRead--;                        // Decrement the read bytes counter
        }
    }
    I2C_GenerateSTOP(CP_I2C, ENABLE);              // Send STOP Condition
    I2C_AcknowledgeConfig(CP_I2C, ENABLE);         // Enable Acknowledgement to be ready for another reception

    return kNoErr;
exit:
    return kNotPreparedErr;
}


static OSStatus CP_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    OSStatus err = kNoErr;

    uint16_t  sCPAddress = CP_ADDRESS;   
    int retry = CP_I2C_RETRY_TIMES; 

    do{
      CP_LowLevel_Init();

      I2C_GenerateSTART(CP_I2C, ENABLE);                     // Send START condition
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_MODE_SELECT);     // Test on EV5 and clear it
      //require( err == kNoErr, exit ) ;

      I2C_Send7bitAddress(CP_I2C, sCPAddress, I2C_Direction_Transmitter);        // Send EEPROM address for write
      msleep(10);
      err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);   // Test on EV6 and clear it
      if(err == kNoErr) break;
    }while(retry--);

    require( retry, exit ) ;

    I2C_SendData(CP_I2C, WriteAddr);          // Send the EEPROM's internal address to read from: MSB of the address first
    err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_BYTE_TRANSMITTED);    // Test on EV8 and clear it
    require( err == kNoErr, exit ) ;

    while(NumByteToWrite--)                 // While there is data to be written
    {
        I2C_SendData(CP_I2C, *pBuffer);        // Send the current byte
        pBuffer++;                          // Point to the next byte to be written
        err = Wait_For_OPT_Finish(I2C_EVENT_MASTER_BYTE_TRANSMITTED);// Test on EV8 and clear it
        require( err == kNoErr, exit ) ;
    }
    
    I2C_GenerateSTOP(CP_I2C, ENABLE);      // Send STOP condition

    return kNoErr;
exit:
    return kNotPreparedErr;
}


OSStatus Wait_For_OPT_Finish(uint32_t Set_Status)
{
    volatile u32 I2C_Status;

    msleep(2);
    I2C_Status = I2C_Read_Flag_Status(CP_I2C);
    require_quiet(I2C_Status == Set_Status, exit);
    return kNoErr;
exit:
    return kNotPreparedErr;
}

