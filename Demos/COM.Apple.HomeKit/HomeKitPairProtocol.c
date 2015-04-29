#include "MICO.h"
#include "MICODefine.h"
#include "HomeKitPairList.h"
#include "MICOAppDefine.h"
#include "SocketUtils.h"
#include "Platform.h"
#include "PlatformFlash.h" 
#include "PlatformRandomNumber.h" 
#include "HTTPUtils.h"
#include "HomeKitTLV.h"
#include "TLVUtils.h"
#include "MICOSRPServer.h"
#include "StringUtils.h"
#include "External/Curve25519/curve25519-donna.h"
#include "MICOCrypto/crypto_stream_chacha20.h"
#include "MICOCrypto/crypto_aead_chacha20poly1305.h"
#include "MICOCrypto/crypto_sign.h"
#include "external/HMAC_HKDF/sha.h"
#include "HomeKitPairProtocol.h"

#define pair_log(M, ...) custom_log("HomeKitPair", M, ##__VA_ARGS__)
#define pair_log_trace() custom_log_trace("HomeKitPair")

typedef enum
{
  eState_M1_VerifyStartRequest      = 1,
  eState_M2_VerifyStartRespond      = 2,
  eState_M3_VerifyFinishRequest     = 3,
  eState_M4_VerifyFinishRespond     = 4,
} HAPairVerifyState_t;

const char * hkdfSetupSalt  =        "Pair-Setup-Salt";
const char * hkdfVerifySalt =        "Pair-Verify-Salt";
const char * hkdfC2AKeySalt =        "Control-Salt";
const char * hkdfA2CKeySalt =        "Control-Salt";

const char * hkdfSetupInfo =        "Pair-Setup-Encryption-Key";
const char * hkdfVerifyInfo =       "Pair-Verify-Encryption-Key";
const char * hkdfC2AInfo =          "Control-Write-Info";
const char * hkdfA2CInfo =          "Control-Read-Info";

const char * AEAD_Nonce_Setup05 =   "PS-Msg05";
const char * AEAD_Nonce_Setup06 =   "PS-Msg06";
const char * AEAD_Nonce_Verify02 =  "PV-Msg02";
const char * AEAD_Nonce_Verify03 =  "PV-Msg03";

const char *stateDescription[7] = {"", "kTLVType_State = M1", "kTLVType_State = M2", "kTLVType_State = M3",
                                   "kTLVType_State = M4", "kTLVType_State = M5", "kTLVType_State = M6"};

const char *methodDescription[6] = {"Pair state ", "PIN-based pair-setup", "", "MFi+PIN-based pair-setup"
                                    "", "Pair-verify"};

static uint8_t pairErrorNum = 0;
static char * _password;
static HAPairSetupState_t haPairSetupState = eState_M1_SRPStartRequest;

OSStatus _HandleState_WaitingForSRPStartRequest( HTTPHeader_t* inHeader, pairInfo_t** inInfo, mico_Context_t * const inContext );
OSStatus _HandleState_HandleSRPStartRespond(int inFd, pairInfo_t* inInfo, mico_Context_t * const inContext);
OSStatus _HandleState_WaitingForSRPVerifyRequest(HTTPHeader_t* inHeader, pairInfo_t* inInfo, mico_Context_t * const inContext );
OSStatus _HandleState_HandleSRPVerifyRespond(int inFd, pairInfo_t* inInfo, mico_Context_t * const inContext);
OSStatus _HandleState_WaitingForExchangeRequest(HTTPHeader_t* inHeader, pairInfo_t* inInfo, mico_Context_t * const inContext );
OSStatus _HandleState_HandleExchangeRespond(int inFd, pairInfo_t** inInfo, mico_Context_t * const inContext);

OSStatus _HandleState_WaitingForVerifyStartRequest( HTTPHeader_t* inHeader, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext );
OSStatus _HandleState_WaitingForVerifyStartRespond(int inFd, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext);
OSStatus _HandleState_WaitingForVerifyFinishRequest(HTTPHeader_t* inHeader, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext );
OSStatus _HandleState_WaitingForVerifyFinishRespond(int inFd, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext);

void HKSetPassword (char * password)
{
  _password = password;
}


void HKCleanPairSetupInfo(pairInfo_t **info, mico_Context_t * const inContext){
  if(*info){
    if(inContext->appStatus.haPairSetupRunning == true){
      haPairSetupState = eState_M1_SRPStartRequest;
      inContext->appStatus.haPairSetupRunning = false;
    }
      
    srp_server_delete(&(*info)->SRPServer );
    if((*info)->SRPUser) free((*info)->SRPUser);
    if((*info)->SRPControllerPublicKey) free((*info)->SRPControllerPublicKey);
    if((*info)->SRPControllerProof) free((*info)->SRPControllerProof);
    if((*info)->HKDF_Key) free((*info)->HKDF_Key);
    free((*info));   
    *info = 0; 
  }
}

pairVerifyInfo_t* HKCreatePairVerifyInfo(void)
{
  pairVerifyInfo_t *pairVerifyInfo = NULL;
  pairVerifyInfo = calloc(1, sizeof( pairVerifyInfo_t ) );
  require( pairVerifyInfo, exit);
  pairVerifyInfo->haPairVerifyState = eState_M1_VerifyStartRequest;
exit:
  return pairVerifyInfo;
}

void HKCleanPairVerifyInfo(pairVerifyInfo_t **verifyInfo){
    if(*verifyInfo){
    if((*verifyInfo)->pControllerCurve25519PK) free((*verifyInfo)->pControllerCurve25519PK);
    if((*verifyInfo)->pAccessoryCurve25519PK) free((*verifyInfo)->pAccessoryCurve25519PK);
    if((*verifyInfo)->pAccessoryCurve25519SK) free((*verifyInfo)->pAccessoryCurve25519SK);
    if((*verifyInfo)->pSharedSecret) free((*verifyInfo)->pSharedSecret);
    if((*verifyInfo)->pHKDFKey) free((*verifyInfo)->pHKDFKey);
    if((*verifyInfo)->A2CKey) free((*verifyInfo)->A2CKey);
    if((*verifyInfo)->C2AKey) free((*verifyInfo)->C2AKey);
    free((*verifyInfo));   
    *verifyInfo = 0; 
  }
}

OSStatus HKPairSetupEngine( int inFd, HTTPHeader_t* inHeader, pairInfo_t** inInfo, mico_Context_t * const inContext )
{
  OSStatus err = kNoErr;

  //if(pairErrorNum>=10) return kNoErr;

  switch ( haPairSetupState ){
    case eState_M1_SRPStartRequest:
      err = _HandleState_WaitingForSRPStartRequest( inHeader, inInfo, inContext );
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      err =  _HandleState_HandleSRPStartRespond( inFd , *inInfo, inContext );
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      break;

    case eState_M3_SRPVerifyRequest:
      err = _HandleState_WaitingForSRPVerifyRequest( inHeader, *inInfo, inContext );
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      err = _HandleState_HandleSRPVerifyRespond(inFd , *inInfo, inContext);
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      break;

    case eState_M5_ExchangeRequest:
      err = _HandleState_WaitingForExchangeRequest( inHeader, *inInfo, inContext );
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      err = _HandleState_HandleExchangeRespond(inFd , inInfo, inContext);
      require_noerr_action( err, exit, haPairSetupState = eState_M1_SRPStartRequest);
      break;

    default:
      pair_log("STATE ERROR");
      err = kNoErr;
  }

exit:
  if(err!=kNoErr){
    HKCleanPairSetupInfo(inInfo, inContext);
    pairErrorNum++;
  }
    
  return err;
}


OSStatus _HandleState_WaitingForSRPStartRequest( HTTPHeader_t* inHeader, pairInfo_t** inInfo, mico_Context_t * const inContext )
{
  pair_log_trace();
  const uint8_t *             src = (const uint8_t *) inHeader->extraDataPtr;
  const uint8_t * const       end = src + inHeader->extraDataLen;
  uint8_t                     eid;
  const uint8_t *             ptr;
  size_t                      len;
  char *                      tmp;

  OSStatus err = kNoErr;
  pair_log("Free memory1: %d", mico_memory_info()->free_memory);

  /*Another pair procedure is pending*/
  mico_rtos_lock_mutex(&inContext->flashContentInRam_mutex);
  if(haPairSetupState != eState_M1_SRPStartRequest){
    pairErrorNum++;
    mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
    return kNoErr;
  }

  HKCleanPairSetupInfo(inInfo, inContext); 
  *inInfo = calloc(1, sizeof(pairInfo_t));
  require_action(*inInfo, exit, err = kNoMemoryErr);
  inContext->appStatus.haPairSetupRunning = true;

  while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
  {
    tmp = calloc( len + 1, sizeof( uint8_t ) );
    require_action( tmp, exit, err = kNoMemoryErr );
    memcpy( tmp, ptr, len );

    switch( eid )
    {
      case kTLVType_State:
        pair_log("Recv: %s", stateDescription[*(uint8_t *)tmp]);
        require_action(haPairSetupState == *(uint8_t *)tmp, exit, err = kValueErr);
        free(tmp);
      break;
        case kTLVType_Method:
        pair_log("Recv: kTLVType_Method: %s", methodDescription[*(uint8_t *)tmp]);
        free(tmp);
        break;
      case kTLVType_User:
        (*inInfo)->SRPUser = tmp;
        pair_log("Recv: kTLVType_User: %s", (*inInfo)->SRPUser);
        break;
      default:
        free( tmp );
        pair_log( "Warning: Ignoring unsupported pair setup EID 0x%02X", eid );
        break;
    }
  }
  
  haPairSetupState = eState_M2_SRPStartRespond;
  
exit:
  mico_rtos_unlock_mutex(&inContext->flashContentInRam_mutex);
  return err;

}


OSStatus _HandleState_HandleSRPStartRespond(int inFd, pairInfo_t* inInfo, mico_Context_t * const inContext)
{
  pair_log_trace();
  OSStatus err;
  int i, j;
  uint8_t *outTLVResponse = NULL;
  size_t outTLVResponseLen = 0;
  uint8_t *tlvPtr;

  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;

  inInfo->SRPServer = srp_server_setup( SRP_SHA1, SRP_NG_2048, inInfo->SRPUser, (const unsigned char *)_password, strlen(_password),0, 0);
  require(inInfo->SRPServer, exit);

  outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;
  for(i = inInfo->SRPServer->len_B/kHATLV_MaxStringSize; i>0; i--)
    outTLVResponseLen += kHATLV_MaxStringSize + kHATLV_TypeLengthSize;
  outTLVResponseLen += inInfo->SRPServer->len_B%kHATLV_MaxStringSize + kHATLV_TypeLengthSize;
  outTLVResponseLen += inInfo->SRPServer->len_s + kHATLV_TypeLengthSize;

  outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
  require_action( outTLVResponse, exit, err = kNoMemoryErr );

  tlvPtr = outTLVResponse;

  *tlvPtr++ = kTLVType_State;
  *tlvPtr++ = sizeof(uint8_t);
  *tlvPtr++ = (uint8_t)eState_M2_SRPStartRespond;
  pair_log("Send: kTLVType_State: %s", stateDescription[eState_M2_SRPStartRespond]);
  
  *tlvPtr++ = kTLVType_Salt;
  *tlvPtr++ = inInfo->SRPServer->len_s;
  memcpy( tlvPtr, inInfo->SRPServer->bytes_s, inInfo->SRPServer->len_s );
  tlvPtr += inInfo->SRPServer->len_s;

  j = inInfo->SRPServer->len_B/kHATLV_MaxStringSize;
  for(i = 0; i < j; i++){
    *tlvPtr++ = kTLVType_PublicKey;
    *tlvPtr++ = kHATLV_MaxStringSize;
    memcpy( tlvPtr, inInfo->SRPServer->bytes_B+i*kHATLV_MaxStringSize, kHATLV_MaxStringSize );
    tlvPtr += kHATLV_MaxStringSize;
  }

  *tlvPtr++ = kTLVType_PublicKey;
  *tlvPtr++ = inInfo->SRPServer->len_B%kHATLV_MaxStringSize;
  memcpy( tlvPtr, inInfo->SRPServer->bytes_B+kHATLV_MaxStringSize*j, inInfo->SRPServer->len_B%kHATLV_MaxStringSize );
  tlvPtr += inInfo->SRPServer->len_B%kHATLV_MaxStringSize;
  err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_Pairing_TLV8, outTLVResponseLen, &httpResponse, &httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, httpResponse, httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend(inFd, outTLVResponse, outTLVResponseLen);
  require_noerr( err, exit );

  haPairSetupState = eState_M3_SRPVerifyRequest;

exit:
  if(outTLVResponse) free(outTLVResponse);
  if(httpResponse) free(httpResponse);
  return err;
}

OSStatus _HandleState_WaitingForSRPVerifyRequest( HTTPHeader_t* inHeader, pairInfo_t* inInfo, mico_Context_t * const inContext )
{
  pair_log_trace();
  const uint8_t *             src = (const uint8_t *) inHeader->extraDataPtr;
  const uint8_t * const       end = src + inHeader->extraDataLen;
  uint8_t                     eid;
  const uint8_t *             ptr;
  size_t                      len;
  char *                      tmp;
  OSStatus err = kNoErr;

#ifdef DEBUG
  char *tempString = NULL;
#endif
    pair_log("Free memory1: %d", mico_memory_info()->free_memory);


  while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
  {

    switch( eid )
    {
      case kTLVType_State:
        tmp = calloc( len, sizeof( uint8_t ) );
        require_action( tmp, exit, err = kNoMemoryErr );
        memcpy( tmp, ptr, len );
        pair_log("Recv: %s", stateDescription[*(uint8_t *)tmp]);
        require_action(haPairSetupState == *(uint8_t *)tmp, exit, err = kValueErr);
        free(tmp);
      break;
        case kTLVType_PublicKey:
        if(inInfo->SRPControllerPublicKey == NULL){
          inInfo->SRPControllerPublicKey = calloc( len, sizeof( uint8_t ) );
          require_action( inInfo->SRPControllerPublicKey, exit, err = kNoMemoryErr );
          memcpy( inInfo->SRPControllerPublicKey, ptr, len );
          inInfo->SRPControllerPublicKeyLen = len;
        }else{
          inInfo->SRPControllerPublicKey = realloc( inInfo->SRPControllerPublicKey, inInfo->SRPControllerPublicKeyLen+len );
          require_action( inInfo->SRPControllerPublicKey, exit, err = kNoMemoryErr );
          memcpy( inInfo->SRPControllerPublicKey+inInfo->SRPControllerPublicKeyLen, ptr, len );
          inInfo->SRPControllerPublicKeyLen += len;
        }
        break;
      case kTLVType_Proof:
        if(inInfo->SRPControllerProof == NULL){
          inInfo->SRPControllerProof = calloc( len, sizeof( uint8_t ) );
          require_action( inInfo->SRPControllerProof, exit, err = kNoMemoryErr );
          memcpy( inInfo->SRPControllerProof, ptr, len );
          inInfo->SRPControllerProofLen = len;
        }else{
          inInfo->SRPControllerProof = realloc( inInfo->SRPControllerProof, inInfo->SRPControllerProofLen+len );
          require_action( inInfo->SRPControllerProof, exit, err = kNoMemoryErr );
          memcpy( inInfo->SRPControllerProof+inInfo->SRPControllerProofLen, ptr, len );
          inInfo->SRPControllerProofLen += len;
        }
        break;
      default:
        pair_log( "Warning: Ignoring unsupported pair setup EID 0x%02X", eid );
        break;
    }
  }

#ifdef DEBUG
  tempString = DataToHexString( inInfo->SRPControllerPublicKey, inInfo->SRPControllerPublicKeyLen );
  require_action( tempString, exit, err = kNoMemoryErr );
  pair_log("Recv: kTLVType_PublicKey length: %d: %s", inInfo->SRPControllerPublicKeyLen, tempString);
  free(tempString);
  tempString = DataToHexString( inInfo->SRPControllerProof, inInfo->SRPControllerProofLen );
  require_action( tempString, exit, err = kNoMemoryErr );
  pair_log("Recv: kTLVType_Proof kTLVType_Proof length: %d:%s", inInfo->SRPControllerProofLen, tempString);
  free(tempString);
  tempString = NULL;
#endif
  
  haPairSetupState = eState_M4_SRPVerifyRespond;
  pair_log("Free memory: %d", mico_memory_info()->free_memory);


exit:
  return err;
}

OSStatus _HandleState_HandleSRPVerifyRespond(int inFd, pairInfo_t* inInfo, mico_Context_t * const inContext)
{
  pair_log_trace();
  OSStatus err = kNoErr;
  uint8_t *outTLVResponse = NULL;
  size_t outTLVResponseLen = 0;
  uint8_t *tlvPtr;

  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;

  const uint8_t * bytes_HAMK = 0;
  pair_log("Free memory1: %d", mico_memory_info()->free_memory);

  err = srp_server_generate_session_key( inInfo->SRPServer, inInfo->SRPControllerPublicKey, inInfo->SRPControllerPublicKeyLen );
  require_noerr(err, exit);

  srp_server_verify_session( inInfo->SRPServer,  inInfo->SRPControllerProof,  &bytes_HAMK );

  if ( !bytes_HAMK ){
    pair_log("User authentication failed!");
    outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;

    outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
    require_action( outTLVResponse, exit, err = kNoMemoryErr );

    tlvPtr = outTLVResponse;
    *tlvPtr++ = kTLVType_Status;
    *tlvPtr++ = sizeof(uint8_t);
    *tlvPtr++ = kTLVStatus_AuthenticationErr;
    pair_log("Send: kTLVType_Status: 0x%x", kTLVStatus_AuthenticationErr);
    haPairSetupState = eState_M1_SRPStartRequest;
  }
  else{
    pair_log("User authentication success!");
    outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;
    outTLVResponseLen += inInfo->SRPServer->len_AMK + kHATLV_TypeLengthSize;

    outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
    require_action( outTLVResponse, exit, err = kNoMemoryErr );

    tlvPtr = outTLVResponse;
    *tlvPtr++ = kTLVType_State;
    *tlvPtr++ = sizeof(uint8_t);
    *tlvPtr++ = eState_M4_SRPVerifyRespond;

    *tlvPtr++ = kTLVType_Proof;
    *tlvPtr++ = inInfo->SRPServer->len_AMK;
    memcpy( tlvPtr, bytes_HAMK, inInfo->SRPServer->len_AMK );
    tlvPtr += inInfo->SRPServer->len_AMK;

    haPairSetupState = eState_M5_ExchangeRequest;
  }

  err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_Pairing_TLV8, outTLVResponseLen, &httpResponse, &httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, httpResponse, httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend(inFd, outTLVResponse, outTLVResponseLen);
  require_noerr( err, exit );

exit:
  if(outTLVResponse) free(outTLVResponse);
  if(httpResponse) free(httpResponse);

  return err;

}

OSStatus _HandleState_WaitingForExchangeRequest( HTTPHeader_t* inHeader, pairInfo_t* inInfo, mico_Context_t * const inContext )
{
  pair_log_trace();
  const uint8_t *             src = (const uint8_t *) inHeader->extraDataPtr;
  const uint8_t * const       end = src + inHeader->extraDataLen;
  uint8_t                     eid;
  const uint8_t *             ptr;
  size_t                      len;
  char *                      tmp;
  int                         i;
  OSStatus                    err = kNoErr;
  unsigned char *             encryptedData = NULL;
  unsigned long               encryptedDataLen;
  unsigned char *             authTag = NULL;
  unsigned long long          authTagLen;

  unsigned char               decryptedData[32];
  unsigned long long          decryptedDataLen;
  pair_list_in_flash_t        *pairList = NULL;
  pair_log("Free memory1: %d", mico_memory_info()->free_memory);

  inInfo->HKDF_Key = malloc(32);
  require_action(inInfo->HKDF_Key, exit, err = kNoMemoryErr);
  err = hkdf(SHA512,  (const unsigned char *) hkdfSetupSalt, strlen(hkdfSetupSalt),
                            inInfo->SRPServer->session_key, inInfo->SRPServer->len_session_key,
                            (const unsigned char *)hkdfSetupInfo, strlen(hkdfSetupInfo), inInfo->HKDF_Key, 32);
  require_noerr(err, exit);

  while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
  {

    switch( eid )
    {
      case kTLVType_State:
        tmp = calloc( len, sizeof( uint8_t ) );
        require_action( tmp, exit, err = kNoMemoryErr );
        memcpy( tmp, ptr, len );
        pair_log("Recv: %s", stateDescription[*(uint8_t *)tmp]);
        require_action(haPairSetupState == *(uint8_t *)tmp, exit, err = kValueErr);
        free(tmp);
      break;
        case kTLVType_EncryptedData:
        encryptedData = calloc( len, sizeof( uint8_t ) );
        require_action( encryptedData, exit, err = kNoMemoryErr );
        memcpy( encryptedData, ptr, len );
        encryptedDataLen = len;
        pair_log("Recv: kTLVType_EncryptedData");
        break;
      case kTLVType_AuthTag:
        require(len == crypto_aead_chacha20poly1305_ABYTES, exit);
        authTag = calloc( len, sizeof( uint8_t ) );
        require_action( authTag, exit, err = kNoMemoryErr );
        memcpy( authTag, ptr, len );
        authTagLen = len;
        pair_log("Recv: kTLVType_AuthTag");
        break;
      default:
        pair_log( "Warning: Ignoring unsupported pair setup EID 0x%02X", eid );
        break;
    }
  }

  encryptedData = realloc(encryptedData, encryptedDataLen + authTagLen);
  memcpy(encryptedData+encryptedDataLen, authTag, authTagLen);
  err =  crypto_aead_chacha20poly1305_decrypt(decryptedData, &decryptedDataLen, NULL, 
                                              (const unsigned char *)encryptedData, encryptedDataLen + authTagLen,NULL, 0,  
                                              (const unsigned char *)AEAD_Nonce_Setup05, (const unsigned char *)inInfo->HKDF_Key);
  require_noerr_action(err, exit, pair_log("crypto_aead_chacha20poly1305_decrypt failed"));
  require_action(decryptedDataLen == 32, exit, pair_log("decryptedDataLen is not properly set"));
  pair_log("crypto_aead_chacha20poly1305 decrypt success");

  pairList = calloc(1, sizeof(pair_list_in_flash_t));
  require_action(pairList, exit, err = kNoMemoryErr);
  err = HMReadPairList(pairList);
  require_noerr(err, exit);

  
  /*Save controller's LTPK*/
  for(i=0; i<MAXPairNumber; i++){
    if(strncmp(pairList->pairInfo[i].controllerName, inInfo->SRPUser, 64)==0)
      break;
  }

  /*This is a new record*/
  if(i == MAXPairNumber){
    for(i=0; i<MAXPairNumber; i++){
      if(pairList->pairInfo[i].controllerName[0] == 0x0)
      break;
    }
  }
  
  /*No space for new record*/
  require_action(i < MAXPairNumber, exit, inInfo->pairListFull = true);

  strcpy(pairList->pairInfo[i].controllerName, inInfo->SRPUser);
  memcpy(pairList->pairInfo[i].controllerLTPK, decryptedData, decryptedDataLen);
  err = HMUpdatePairList(pairList);

  haPairSetupState = eState_M6_ExchangeRespond;

exit:
  if(encryptedData) free(encryptedData);
  if(authTag) free(authTag);
  if(pairList) free(pairList);
  return err; 
}

OSStatus _HandleState_HandleExchangeRespond(int inFd, pairInfo_t** inInfo, mico_Context_t * const inContext)
{
  pair_log_trace();
  OSStatus err = kNoErr;
  uint8_t *outTLVResponse = NULL;
  size_t outTLVResponseLen = 0;
  uint8_t *tlvPtr;

  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;
  uint8_t LTPK[32];
  unsigned char       encryptedData[100];
  unsigned long long  encryptedDataLen;
  char *accessoryName;
  pair_log("Free memory1: %d", mico_memory_info()->free_memory);

  memset(encryptedData, 0x0, 100);

  if((*inInfo)->pairListFull == true){
    pair_log("Pair list is full!");

    outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;
    outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
    require_action( outTLVResponse, exit, err = kNoMemoryErr );

    tlvPtr = outTLVResponse;
    *tlvPtr++ = kTLVType_Status;
    *tlvPtr++ = sizeof(uint8_t);
    *tlvPtr++ = kTLVStatus_MaxPeerErr;
    pair_log("Send: kTLVType_Status: 0x%x", kTLVStatus_MaxPeerErr);
  }else{

    if(inContext->flashContentInRam.appConfig.haPairSetupFinished){
      memcpy(LTPK, inContext->flashContentInRam.appConfig.LTSK + 32, 32);
    }else{
      err = crypto_sign_keypair(LTPK, inContext->flashContentInRam.appConfig.LTSK);
      require_noerr(err, exit);     
    }

    accessoryName = __strdup_trans_dot(inContext->micoStatus.mac);
    outTLVResponseLen += strlen(accessoryName) + kHATLV_TypeLengthSize;
    outTLVResponseLen += 32 + kHATLV_TypeLengthSize;

    outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
    require_action( outTLVResponse, exit, err = kNoMemoryErr );

    tlvPtr = outTLVResponse;
    *tlvPtr++ = kTLVType_User;
    *tlvPtr++ = strlen(accessoryName);
    memcpy( tlvPtr, accessoryName, strlen(accessoryName) );
    tlvPtr += strlen(accessoryName);
    free(accessoryName);

    *tlvPtr++ = kTLVType_PublicKey;
    *tlvPtr++ = 32;
    memcpy( tlvPtr, LTPK, 32 );

    require_action((*inInfo)->HKDF_Key, exit, err = kParamErr);
    err =  crypto_aead_chacha20poly1305_encrypt(encryptedData, &encryptedDataLen, outTLVResponse, outTLVResponseLen,
                                                NULL, 0, NULL, (const unsigned char *)AEAD_Nonce_Setup06,
                                                (const unsigned char *)(*inInfo)->HKDF_Key);

    require_noerr_action(err, exit, pair_log("crypto_aead_chacha20poly1305_encrypt failed"));
    require_action(encryptedDataLen - crypto_aead_chacha20poly1305_ABYTES == outTLVResponseLen, exit, pair_log("encryptedDataLen is not properly set"));
    free(outTLVResponse);
    outTLVResponse = NULL;

    outTLVResponseLen = 0;
    outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;
    outTLVResponseLen += encryptedDataLen - crypto_aead_chacha20poly1305_ABYTES + kHATLV_TypeLengthSize;
    outTLVResponseLen += crypto_aead_chacha20poly1305_ABYTES + kHATLV_TypeLengthSize;

    outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
    require_action( outTLVResponse, exit, err = kNoMemoryErr );

    tlvPtr = outTLVResponse;
    *tlvPtr++ = kTLVType_State;
    *tlvPtr++ = sizeof(uint8_t);
    *tlvPtr++ = eState_M6_ExchangeRespond;

    *tlvPtr++ = kTLVType_EncryptedData;
    *tlvPtr++ = encryptedDataLen -  crypto_aead_chacha20poly1305_ABYTES;
    memcpy( tlvPtr, encryptedData, encryptedDataLen -  crypto_aead_chacha20poly1305_ABYTES);
    tlvPtr += encryptedDataLen -  crypto_aead_chacha20poly1305_ABYTES;

    *tlvPtr++ = kTLVType_AuthTag;
    *tlvPtr++ = crypto_aead_chacha20poly1305_ABYTES;
    memcpy( tlvPtr, encryptedData + encryptedDataLen -  crypto_aead_chacha20poly1305_ABYTES, crypto_aead_chacha20poly1305_ABYTES );
  }

  haPairSetupState = eState_M1_SRPStartRequest;

  err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_Pairing_TLV8, outTLVResponseLen, &httpResponse, &httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, httpResponse, httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend(inFd, outTLVResponse, outTLVResponseLen);
  require_noerr( err, exit );

  /*Save accessory's LPSK*/
  if((*inInfo)->pairListFull != true){
    inContext->flashContentInRam.appConfig.haPairSetupFinished = true;
    err = MICOUpdateConfiguration(inContext);
    require_noerr(err, exit);
  }
  haPairSetupState = eState_M1_SRPStartRequest;
  HKCleanPairSetupInfo(inInfo, inContext);
  pair_log("Free memory1: %d", mico_memory_info()->free_memory);
  inContext->appStatus.haPairSetupRunning = false;

exit:
  if(outTLVResponse) free(outTLVResponse);
  if(httpResponse) free(httpResponse);
  return err;

}


OSStatus HKPairVerifyEngine( int inFd, HTTPHeader_t* inHeader, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext )
{
  OSStatus err = kNoErr;
  require_action(inInfo, exit, err = kNotPreparedErr);

  switch ( inInfo->haPairVerifyState ){
    case eState_M1_VerifyStartRequest:
      err = _HandleState_WaitingForVerifyStartRequest( inHeader, inInfo, inContext );
      require_noerr_action( err, exit, inInfo->haPairVerifyState = eState_M1_VerifyStartRequest);
      err =  _HandleState_WaitingForVerifyStartRespond( inFd , inInfo, inContext );
      require_noerr_action( err, exit, inInfo->haPairVerifyState = eState_M1_VerifyStartRequest);
      break;

    case eState_M3_VerifyFinishRequest:
      err = _HandleState_WaitingForVerifyFinishRequest( inHeader, inInfo, inContext );
      require_noerr_action( err, exit, inInfo->haPairVerifyState = eState_M1_VerifyStartRequest);
      err = _HandleState_WaitingForVerifyFinishRespond(inFd , inInfo, inContext);
      require_noerr_action( err, exit, inInfo->haPairVerifyState = eState_M1_VerifyStartRequest);
      break;

    default:
      pair_log("STATE ERROR");
      err = kStateErr;
  }

exit:
  return err;
}


OSStatus _HandleState_WaitingForVerifyStartRequest( HTTPHeader_t* inHeader, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext )
{
  pair_log_trace();
  OSStatus                    err = kNoErr;
  const uint8_t *             src = (const uint8_t *) inHeader->extraDataPtr;
  const uint8_t * const       end = src + inHeader->extraDataLen;
  uint8_t                     eid;
  const uint8_t *             ptr;
  size_t                      len;
  char *                      tmp = NULL;

  while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
  {
    tmp = calloc( len+1, sizeof( uint8_t ) );
    require_action( tmp, exit, err = kNoMemoryErr );
    memcpy( tmp, ptr, len );

    switch( eid )
    {
      case kTLVType_State:
        pair_log("Recv: %s", stateDescription[*(uint8_t *)tmp]);
        require_action(inInfo->haPairVerifyState == *(uint8_t *)tmp, exit, err = kValueErr);
        free(tmp);
      break;
        case kTLVType_User:
        inInfo->pControllerLTPK =  HMFindLTPK(tmp);
        free(tmp);
        require_action(inInfo->pControllerLTPK, exit, err = kNotFoundErr);
        break;
      case kTLVType_PublicKey:
        inInfo->pControllerCurve25519PK = (uint8_t *)tmp;
        break;
      default:
        pair_log( "Warning: Ignoring unsupported pair setup EID 0x%02X", eid );
        break;
    }
  }
  inInfo->haPairVerifyState = eState_M2_VerifyStartRespond;

exit:
  return err;
}

OSStatus _HandleState_WaitingForVerifyStartRespond(int inFd, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext)
{
  pair_log_trace();
  OSStatus            err = kNoErr;
  uint8_t             *outTLVResponse = NULL;
  size_t              outTLVResponseLen = 0;
  uint8_t             *tlvPtr;
  uint8_t             *httpResponse = NULL;
  size_t              httpResponseLen = 0;
  uint8_t             YX[64];
  uint8_t             signature[128];
  unsigned long long  signatureLen;
  uint8_t             *pAccessoryProof = NULL;
  char                *accessoryName;

  inInfo->pAccessoryCurve25519PK = malloc(32);
  require_action(inInfo->pAccessoryCurve25519PK, exit, err = kNoMemoryErr);
  inInfo->pAccessoryCurve25519SK = malloc(32);
  require_action(inInfo->pAccessoryCurve25519SK, exit, err = kNoMemoryErr);
  inInfo->pSharedSecret = malloc(32);
  require_action(inInfo->pSharedSecret, exit, err = kNoMemoryErr);
  inInfo->pHKDFKey = malloc(32);
  require_action(inInfo->pHKDFKey, exit, err = kNoMemoryErr);
  pAccessoryProof = malloc(128 + crypto_aead_chacha20poly1305_ABYTES);
  require_action(pAccessoryProof, exit, err = kNoMemoryErr);

  err = PlatformRandomBytes( inInfo->pAccessoryCurve25519SK, 32 );
  require_noerr( err, exit );

  curve25519_donna( inInfo->pAccessoryCurve25519PK, inInfo->pAccessoryCurve25519SK, NULL );
  curve25519_donna( inInfo->pSharedSecret, inInfo->pAccessoryCurve25519SK, inInfo->pControllerCurve25519PK );

  memcpy(YX,    inInfo->pAccessoryCurve25519PK,   32);
  memcpy(YX+32, inInfo->pControllerCurve25519PK,  32);

  err = crypto_sign(signature,&signatureLen, YX, 64, inContext->flashContentInRam.appConfig.LTSK );
  require_noerr_string(err, exit, "crypto sign failed");

  err = hkdf(SHA512,  (const unsigned char *) hkdfVerifySalt, strlen(hkdfVerifySalt),
                      inInfo->pSharedSecret, 32,
                      (const unsigned char *)hkdfVerifyInfo, strlen(hkdfVerifyInfo), inInfo->pHKDFKey, 32);
  require_noerr_string(err, exit, "Generate HKDK key failed");

  err = crypto_stream_chacha20_xor_ic(pAccessoryProof, signature, 64, 
                                (const unsigned char *)AEAD_Nonce_Verify02, 0U, (const unsigned char *)inInfo->pHKDFKey);

  require_noerr_string(err, exit, "crypto_chacha20 failed");

  accessoryName = __strdup_trans_dot(inContext->micoStatus.mac);

  outTLVResponseLen = 0;
  outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;
  outTLVResponseLen += strlen(accessoryName) + kHATLV_TypeLengthSize;
  outTLVResponseLen += 32 + kHATLV_TypeLengthSize;
  outTLVResponseLen += 64 + kHATLV_TypeLengthSize;

  outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
  require_action( outTLVResponse, exit, err = kNoMemoryErr );

  tlvPtr = outTLVResponse;
  *tlvPtr++ = kTLVType_State;
  *tlvPtr++ = sizeof(uint8_t);
  *tlvPtr++ = eState_M2_VerifyStartRespond;

  *tlvPtr++ = kTLVType_User;
  *tlvPtr++ = strlen(accessoryName);
  memcpy( tlvPtr, accessoryName, strlen(accessoryName));
  tlvPtr += strlen(accessoryName);
  free(accessoryName);

  *tlvPtr++ = kTLVType_PublicKey;
  *tlvPtr++ = 32;
  memcpy( tlvPtr, inInfo->pAccessoryCurve25519PK, 32 );
  tlvPtr += 32;

  *tlvPtr++ = kTLVType_Proof;
  *tlvPtr++ = 64;
  memcpy( tlvPtr, pAccessoryProof, 64);

  err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_Pairing_TLV8, outTLVResponseLen, &httpResponse, &httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, httpResponse, httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend(inFd, outTLVResponse, outTLVResponseLen);
  require_noerr( err, exit );
  inInfo->haPairVerifyState = eState_M3_VerifyFinishRequest;

exit:
  if(pAccessoryProof) free(pAccessoryProof);
  if(outTLVResponse) free(outTLVResponse);
  if(httpResponse) free(httpResponse);
  return err;
}

OSStatus _HandleState_WaitingForVerifyFinishRequest(HTTPHeader_t* inHeader, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext )
{
  pair_log_trace();
  OSStatus                    err = kNoErr;
  (void)                      inContext;
  const uint8_t *             src = (const uint8_t *) inHeader->extraDataPtr;
  const uint8_t * const       end = src + inHeader->extraDataLen;
  uint8_t                     eid;
  const uint8_t *             ptr;
  size_t                      len;
  char *                      tmp = NULL;
  uint8_t *                   pControllerProof = NULL;
  uint8_t                     decryptedControllerProof[128]; 
  uint8_t                     XY[128];
  unsigned long long          XYLen;

  while( TLVGetNext( src, end, &eid, &ptr, &len, &src ) == kNoErr )
  {
    tmp = calloc( len+1, sizeof( uint8_t ) );
    require_action( tmp, exit, err = kNoMemoryErr );
    memcpy( tmp, ptr, len );

    switch( eid )
    {
      case kTLVType_State:
        pair_log("Recv: %s", stateDescription[*(uint8_t *)tmp]);
        require_action(inInfo->haPairVerifyState == *(uint8_t *)tmp, exit, err = kValueErr);
        free(tmp);
        break;
      case kTLVType_Proof:
        pControllerProof = (uint8_t *)tmp;
        break;
      default:
        pair_log( "Warning: Ignoring unsupported pair setup EID 0x%02X", eid );
        break;
    }
  }

  err = crypto_stream_chacha20_xor_ic(decryptedControllerProof, pControllerProof, 64, 
                                (const unsigned char *)AEAD_Nonce_Verify03, 0U, (const unsigned char *)inInfo->pHKDFKey);

  memcpy(decryptedControllerProof+64, inInfo->pControllerCurve25519PK, 32);
  memcpy(decryptedControllerProof+96, inInfo->pAccessoryCurve25519PK, 32);
  err = crypto_sign_open(XY, &XYLen, decryptedControllerProof,128, inInfo->pControllerLTPK);
  require_noerr_string(err, exit, "Signature verify failed");

exit:
  if(pControllerProof) free(pControllerProof);
  return err;
}

OSStatus _HandleState_WaitingForVerifyFinishRespond(int inFd, pairVerifyInfo_t* inInfo, mico_Context_t * const inContext)
{
  pair_log_trace();
  OSStatus err = kNoErr;
  (void)inContext;
  uint8_t *outTLVResponse = NULL;
  size_t outTLVResponseLen = 0;
  uint8_t *tlvPtr;

  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;

  outTLVResponseLen += sizeof(uint8_t) + kHATLV_TypeLengthSize;

  outTLVResponse = calloc( outTLVResponseLen, sizeof( uint8_t ) );
  require_action( outTLVResponse, exit, err = kNoMemoryErr );

  tlvPtr = outTLVResponse;
  *tlvPtr++ = kTLVType_State;
  *tlvPtr++ = sizeof(uint8_t);
  *tlvPtr++ = eState_M4_SRPVerifyRespond;

  inInfo->verifySuccess = true;
  inInfo->A2CKey = malloc(32);
  require_action(inInfo->A2CKey, exit, err = kNoMemoryErr);
  err = hkdf(SHA512,  (const unsigned char *) hkdfA2CKeySalt, strlen(hkdfA2CKeySalt),
                            inInfo->pSharedSecret, 32,
                            (const unsigned char *)hkdfA2CInfo, strlen(hkdfA2CInfo), inInfo->A2CKey, 32);
  require_noerr(err, exit);

  inInfo->C2AKey = malloc(32);
  require_action(inInfo->C2AKey, exit, err = kNoMemoryErr);
  err = hkdf(SHA512,  (const unsigned char *) hkdfC2AKeySalt, strlen(hkdfC2AKeySalt),
                            inInfo->pSharedSecret, 32,
                            (const unsigned char *)hkdfC2AInfo, strlen(hkdfC2AInfo), inInfo->C2AKey, 32);
  require_noerr(err, exit);

  err =  CreateSimpleHTTPMessageNoCopy( kMIMEType_Pairing_TLV8, outTLVResponseLen, &httpResponse, &httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, httpResponse, httpResponseLen );
  require_noerr( err, exit );
  err = SocketSend( inFd, outTLVResponse, outTLVResponseLen );
  require_noerr( err, exit );

exit:
  if(outTLVResponse) free(outTLVResponse);
  if(httpResponse) free(httpResponse);
  return err;
}

