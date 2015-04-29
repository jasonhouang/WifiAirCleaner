/**
  ******************************************************************************
  * @file    StringUtils.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This header contains function prototypes for manipulating strings..
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

#ifndef __StringUtils_h__
#define __StringUtils_h__

#include <stdarg.h>
#include "Common.h"

// ==== STRING SIZE UTILS ====
//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    sizeof_string
    @abstract   Determines the size of a constant C string, excluding the null terminator.
*/
#define sizeof_string( X )      ( sizeof( ( X ) ) - 1 )

//---------------------------------------------------------------------------------------------------------------------------
/*! @defined    kSizeCString
    @abstract   Constant for describing the size of a null terminated string
*/
#define kSizeCString ( (size_t) -1 )

// ==== MAC String Tools ====   
void formatMACAddr(char *destAddr, char *srcAddr);

/* transform src string to hex mode 
* example: "aabbccddee" => 0xaabbccddee
* each char in the string must 0~9 a~f A~F, otherwise return 0
* return the real obuf length
*/
unsigned int str2hex(unsigned char *ibuf, unsigned char *obuf, unsigned int olen);

/*Allocate a memory to store the string*/
char *__strdup(char *src);

/*Allocate a memory to store the string, and transfer '.' to '\.'*/
char *__strdup_trans_dot(char *src);

// ==== NETWORKING STRING UTILS ====
#define TextToMACAddress( TEXT, SIZE, ADDR )    TextToHardwareAddress( TEXT, SIZE, 6, ADDR )

int TextToHardwareAddress( const void *inText, size_t inTextSize, size_t inAddrSize, void *outAddr );

// ==== BYTE BUFFER TO STRING CONVERSION UTILS ====
char* DataToCString( const uint8_t *inBuf, size_t inBufLen );

char* DataToHexString( const uint8_t *inBuf, size_t inBufLen );

char* DataToHexStringWithSpaces( const uint8_t *inBuf, size_t inBufLen );

char* DataToHexStringWithColons( const uint8_t *inBuf, size_t inBufLen );

// ==== STRING COMPARE UTILS ====
int strnicmp_suffix( const void *inStr, size_t inMaxLen, const char *inSuffix );

int strnicmp( const char *inS1, const char *inS2, size_t inMax );

int strnicmpx( const void *inS1, size_t inN, const char *inS2 );

char * strnstr_suffix( const char *inStr, size_t inMaxLen, const char *inSuffix);

int VSNScanF( const void *inString, size_t inSize, const char *inFormat, va_list inArgs );

size_t  memrlen( const void *inSrc, size_t inMaxLen );




#endif // __StringUtils_h__

