/**
******************************************************************************
* @file    HomeKitHTTPUtils.c 
* @author  William Xu
* @version V1.0.0
* @date    12-July-2014
* @brief   These functions assist with interacting with HTTP clients and servers
*          under Homekit security session.
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

#include "HomeKitHTTPUtils.h"
#include "Debug.h"
#include "StringUtils.h"
#include "PlatformFlash.h"
#include "MICOSocket.h"
#include "SocketUtils.h"
#include "MICOCrypto/crypto_aead_chacha20poly1305.h"

#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

extern bool verify_otp(void);

#define hkhttp_utils_log(M, ...) custom_log("HKHTTPUtils", M, ##__VA_ARGS__)

static __IO uint32_t flashStorageAddress = UPDATE_START_ADDRESS;

security_session_t *HKSNewSecuritySession(void)
{
  security_session_t *session;
  session = calloc(1, sizeof(security_session_t));
  session->established = false;
  return session;
}

int HKSecureSocketSend( int sockfd, void *buf, size_t len, security_session_t *session)
{
  OSStatus       err = kNoErr;
  uint8_t*       encryptedData = NULL;
  ssize_t        sentLength = 0;
  uint64_t       encryptedDataLen;

  encryptedData = malloc(len + crypto_aead_chacha20poly1305_ABYTES + 4);
  require_action(encryptedData, exit, err = kNoMemoryErr);
  *(int *)encryptedData = len;
  err =  crypto_aead_chacha20poly1305_encrypt(encryptedData+4, &encryptedDataLen, (uint8_t *)buf, len,
                                              (const uint8_t *)encryptedData, 4, NULL, (uint8_t *)(&session->outputSeqNo),
                                              (const unsigned char *)session->OutputKey);
  session->outputSeqNo++;
  require_noerr_string(err, exit, "crypto_aead_chacha20poly1305_encrypt failed");
  require_string(encryptedDataLen - crypto_aead_chacha20poly1305_ABYTES == len, exit, "encryptedDataLen is not properly set");

  err = SocketSend( sockfd, encryptedData, encryptedDataLen + 4 );
  require_noerr( err, exit );

  exit:
    if(encryptedData) free(encryptedData);
    return sentLength;
}

int HKSecureRead(security_session_t *session, int sockfd, void *buf, size_t len)
{
  OSStatus    err = kNoErr;
  int         packageLength;
  uint32_t         length = 0;
  uint32_t         recvLength = 0;
  uint32_t         recvLengthTmp = 0;
  uint8_t*       *encryptedData = NULL;
  uint64_t           decryptedDataLen = 0;

  fd_set      readfds;
  int         returnLength = 0;
  int         selectResult;
  bool        moreData;
  struct      timeval_t t;
  t.tv_sec  =  20;
  t.tv_usec =  0;

  if(session->established == true){
    if(session->recvedDataLen){
      returnLength += min(len, session->recvedDataLen);
      memcpy(buf, session->recvedDataBuffer, returnLength);
      session->recvedDataLen -= returnLength;
      if(session->recvedDataLen)
        memmove(session->recvedDataBuffer, session->recvedDataBuffer+returnLength, session->recvedDataLen);
      else
        free(session->recvedDataBuffer);
      goto exit;
    }else{
      while( recvLengthTmp < sizeof(int)){
        FD_ZERO( &readfds );
        FD_SET( sockfd, &readfds );
        selectResult = select( sockfd + 1, &readfds, NULL, NULL, &t );
        require(selectResult >= 1, exit);
        length = read( sockfd, (uint8_t *)&packageLength + recvLengthTmp, sizeof(int) - recvLengthTmp);
        if( length > 0 ) recvLengthTmp += length;
        else return 0;
      }

      recvLength = packageLength + crypto_aead_chacha20poly1305_ABYTES;
      recvLengthTmp = 0;

      encryptedData = malloc(recvLength);
      require(encryptedData, exit);

      while( recvLengthTmp < recvLength){
        FD_ZERO( &readfds );
        FD_SET( sockfd, &readfds );
        selectResult = select( sockfd + 1, &readfds, NULL, NULL, &t );
        require(selectResult >= 1, exit);

        length = read( sockfd, encryptedData + recvLengthTmp, recvLength - recvLengthTmp );
        if( length > 0 ) recvLengthTmp += length;
        else return 0;
      }

      session->recvedDataBuffer = malloc(packageLength);
      require(session->recvedDataBuffer, exit);


      err =  crypto_aead_chacha20poly1305_decrypt(session->recvedDataBuffer, &session->recvedDataLen, NULL, 
                                                     (const unsigned char *)encryptedData, recvLength, (uint8_t *)&packageLength, 4,  
                                                     (uint8_t *)(&session->inputSeqNo), (const unsigned char *)session->InputKey);

      session->inputSeqNo++;

      require_noerr(err, exit);
      require(session->recvedDataLen == recvLength - crypto_aead_chacha20poly1305_ABYTES, exit);

      free(encryptedData);
      encryptedData = NULL;

      returnLength += min(len, session->recvedDataLen);
      memcpy(buf, session->recvedDataBuffer, returnLength);
      session->recvedDataLen -= returnLength;
      if(session->recvedDataLen)
        memmove(session->recvedDataBuffer, session->recvedDataBuffer+returnLength, session->recvedDataLen);
      else
        free(session->recvedDataBuffer);

      goto exit;

    }

    exit:
      if(err == kNoErr)
        return returnLength;
      else{
        if(encryptedData) free(encryptedData);
        return 0;
      }
  }
  else
    return read( sockfd, buf, len);
}


int HKSocketReadHTTPHeader( int inSock, HTTPHeader_t *inHeader, security_session_t *session )
{
  int        err =0;
  char *          buf;
  char *          dst;
  char *          lim;
  char *          end;
  size_t          len;
  ssize_t         n;
  const char *    value;
  size_t          valueSize;
  
  
  buf = inHeader->buf;
  dst = buf + inHeader->len;
  lim = buf + sizeof( inHeader->buf );
  for( ;; )
  {
    // If there's data from a previous read, move it to the front to search it first.
    len = inHeader->extraDataLen;
    if( len > 0 )
    {
      require_action( len <= (size_t)( lim - dst ), exit, err = kParamErr );
      memmove( dst, inHeader->extraDataPtr, len );
      inHeader->extraDataLen = 0;
    }
    else
    {
      n = HKSecureRead( session, inSock, dst, (size_t)( lim - dst ) );
      if(      n  > 0 ) len = (size_t) n;
      else if( n == 0 ) { err = kConnectionErr; goto exit; }
      else goto exit;
    }
    dst += len;
    inHeader->len += len;
    
    if(findHeader( inHeader,  &end ))
      break ;
  }
  
  inHeader->len = (size_t)( end - buf );
  err = HTTPHeaderParse( inHeader );
  require_noerr( err, exit );
  inHeader->extraDataLen = (size_t)( dst - end );
  if(inHeader->extraDataPtr) {
    free((uint8_t *)inHeader->extraDataPtr);
    inHeader->extraDataPtr = 0;
  }
  
  if(inHeader->otaDataPtr) {
    free((uint8_t *)inHeader->otaDataPtr);
    inHeader->otaDataPtr = 0;
  }
  
  err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
  
  if(err == kNoErr && strnicmpx( value, valueSize, kMIMEType_MXCHIP_OTA ) == 0){
    hkhttp_utils_log("Receive OTA data!");        
    err = PlatformFlashInitialize();
    require_noerr(err, exit);
    err = PlatformFlashWrite(&flashStorageAddress, (uint32_t *)end, inHeader->extraDataLen);
    require_noerr(err, exit);
  }else{
    inHeader->extraDataPtr = calloc(inHeader->contentLength, sizeof(uint8_t));
    require_action(inHeader->extraDataPtr, exit, err = kNoMemoryErr);
    memcpy((uint8_t *)inHeader->extraDataPtr, end, inHeader->extraDataLen);
    err = kNoErr;
  }
  
exit:   
  return err;
}



int HKSocketReadHTTPBody  ( int inSock, HTTPHeader_t *inHeader, security_session_t *session )
{
  OSStatus err = kParamErr;
  ssize_t readResult;
  int selectResult;
  fd_set readSet;
  const char *    value;
  size_t          valueSize;
  
  require( inHeader, exit );
  
  err = kNotReadableErr;
  
  
  while ( inHeader->extraDataLen < inHeader->contentLength )
  {
    if(session->recvedDataLen == 0){
      FD_ZERO( &readSet );
      FD_SET( inSock, &readSet );
      selectResult = select( inSock + 1, &readSet, NULL, NULL, NULL );
      require( selectResult >= 1, exit );      
    }
    
    err = HTTPGetHeaderField( inHeader->buf, inHeader->len, "Content-Type", NULL, NULL, &value, &valueSize, NULL );
    require_noerr(err, exit);
    if( strnicmpx( value, valueSize, kMIMEType_MXCHIP_OTA ) == 0 ){
      inHeader->otaDataPtr = calloc(OTA_Data_Length_per_read, sizeof(uint8_t)); 
      require_action(inHeader->otaDataPtr, exit, err = kNoMemoryErr);
      if((inHeader->contentLength - inHeader->extraDataLen)<OTA_Data_Length_per_read){
        readResult = HKSecureRead(  session, inSock,
                                    (uint8_t*)( inHeader->otaDataPtr ),
                                    ( inHeader->contentLength - inHeader->extraDataLen ) );
      }else{
        readResult = HKSecureRead(  session, inSock,
                                    (uint8_t*)( inHeader->otaDataPtr ),
                                    OTA_Data_Length_per_read);
      }
      
      if( readResult  > 0 ) inHeader->extraDataLen += readResult;
      else if( readResult == 0 ) { err = kConnectionErr; goto exit; }
      else goto exit;
      
      err = PlatformFlashWrite(&flashStorageAddress, (uint32_t *)inHeader->otaDataPtr, readResult);
      require_noerr(err, exit);
      
      free(inHeader->otaDataPtr);
      inHeader->otaDataPtr = 0;
    }else{
      readResult = HKSecureRead( session, inSock,
                                (uint8_t*)( inHeader->extraDataPtr + inHeader->extraDataLen ),
                                ( inHeader->contentLength - inHeader->extraDataLen ) );
      
      if( readResult  > 0 ) inHeader->extraDataLen += readResult;
      else if( readResult == 0 ) { err = kConnectionErr; goto exit; }
      else goto exit;
    }
  }
  
  err = kNoErr;
  
exit:
  if(inHeader->otaDataPtr) {
    free(inHeader->otaDataPtr);
    inHeader->otaDataPtr = 0;
  }
  return err;
}


