/**
******************************************************************************
* @file    HomeKitHTTPUtils.h 
* @author  William Xu
* @version V1.0.0
* @date    12-July-2014
* @brief   This header contains function prototypes. These functions assist 
  with interacting with HTTP clients and servers with Homekit security session.
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


#ifndef __HOMEKITHTTPUtils_h__
#define __HOMEKITHTTPUtils_h__

#include "Common.h"

#include "HTTPUtils.h"

typedef struct _security_session_t {
  bool          established;
  uint8_t       OutputKey[32];
  uint8_t       InputKey[32];
  uint64_t      recvedDataLen;
  uint8_t*      recvedDataBuffer;
  uint64_t      outputSeqNo;
  uint64_t      inputSeqNo;
} security_session_t;

security_session_t *HKSNewSecuritySession(void);

int HKSecureSocketSend( int sockfd, void *buf, size_t len, security_session_t *session);

int HKSecureRead(security_session_t *session, int sockfd, void *buf, size_t len);

int HKSocketReadHTTPHeader( int inSock, HTTPHeader_t *inHeader, security_session_t *session );

int HKSocketReadHTTPBody  ( int inSock, HTTPHeader_t *inHeader, security_session_t *session );

#endif // __HOMEKITHTTPUtils_h__

