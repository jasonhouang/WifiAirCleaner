/**
  ******************************************************************************
  * @file    PlatformMFiAuth.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    08-June-2014
  * @brief   This header contains function prototypes called by WAC code that 
  *          must be implemented by the platform. These functions are called when
  *          Apple code needs to interact with the Apple Authentication 
  *          Coprocessor. Please refer to the relevant version of the "Auth IC" 
  *          document to obtain more details on how to interact with the 
  *          Authentication Coprocessor. This document can be found on the MFi 
  *          Portal.
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

#ifndef __PlatformMFiAuth_h__
#define __PlatformMFiAuth_h__

#include "Common.h"

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthInitialize
    @abstract   Performs any platform-specific initialization needed. Example: Bring up I2C interface for communication with
                the Apple Authentication Coprocessor.
*/
OSStatus PlatformMFiAuthInitialize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthFinalize
    @abstract   Performs any platform-specific cleanup needed. Example: Bringing down the I2C interface for communication with
                the Apple Authentication Coprocessor.
*/
void PlatformMFiAuthFinalize( void );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthCreateSignature
    @abstract   Create an RSA signature from the specified SHA-1 digest using the Apple Authentication Coprocessor.

    @param      inDigestPtr         Pointer to 20-byte SHA-1 digest.
    @param      inDigestLen         Number of bytes in the digest. Must be 20.
    @param      outSignaturePtr     Receives malloc()'d ptr to RSA signature. Caller must free() on success.
    @param      outSignatureLen     Receives number of bytes in RSA signature.
*/
OSStatus PlatformMFiAuthCreateSignature( const void *inDigestPtr,
                                         size_t     inDigestLen,
                                         uint8_t    **outSignaturePtr,
                                         size_t     *outSignatureLen );

//---------------------------------------------------------------------------------------------------------------------------
/*! @function   PlatformMFiAuthCopyCertificate
    @abstract   Copy the certificate from the Apple Authentication Coprocessor.

    @param      outCertificatePtr   Receives malloc()'d ptr to a DER-encoded PKCS#7 message containing the certificate.
                                    Caller must free() on success.
    @param      outCertificateLen   Number of bytes in the DER-encoded certificate.
*/
OSStatus PlatformMFiAuthCopyCertificate( uint8_t **outCertificatePtr, size_t *outCertificateLen );


#endif // __PlatformMFiAuth_h__


