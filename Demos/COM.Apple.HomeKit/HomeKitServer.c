/**
  ******************************************************************************
  * @file    MICOConfigServer.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   Local TCP server for mico device configuration 
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

#include "MICO.h"
#include "MICODefine.h"
#include "HomeKitPairList.h"
#include "MICOAppDefine.h"
#include "SocketUtils.h"
#include "HomeKitHTTPUtils.h"
#include "HomeKitPairProtocol.h"
#include "HomeKitProfiles.h"

#define ha_log(M, ...) custom_log("HomeKit", M, ##__VA_ARGS__)
#define ha_log_trace() custom_log_trace("HomeKit")

#define kPAIRSETUP          "/pair-setup"
#define kPAIRVERIFY         "/pair-verify"
#define kPAIRINGS           "/pairing"
#define kReadAcc            "/accessories"
#define kAccessories        "/accessories/"
#define kServices           "/services/"
#define kCharacteristics    "/characteristics/"

#define kMIMEType_HAP_JSON   "application/hap+json"
#define min(a,b) ((a) < (b) ? (a) : (b))

static char *password = "12345678";
extern struct _hapAccessory_t hapObjects[];

typedef struct _HK_Context_t {
  pairInfo_t          *pairInfo;
  pairVerifyInfo_t    *pairVerifyInfo;
  security_session_t  *session;
} HK_Context_t;

extern  void HKCharacteristicInit(mico_Context_t * const inContext);
extern HkStatus HKReadCharacteristicValue(int accessoryID, int serviceID, int characteristicID, value_union *value, mico_Context_t * const inContext);
extern void HKWriteCharacteristicValue(int accessoryID, int serviceID, int characteristicID, value_union value, bool moreComing, mico_Context_t * const inContext);

static void homeKitClient_thread(void *inFd);
static mico_Context_t *Context;
static OSStatus HKhandleIncomeingMessage(int clientFd, HTTPHeader_t *httpHeader, HK_Context_t *inHkContext, mico_Context_t * const inContext);
static OSStatus HKCreateHAPAttriDataBase( struct _hapAccessory_t *inHapObject,  json_object **OutHapObjectJson, mico_Context_t * const inContext);
static OSStatus HKCreateHAPReadRespond( struct _hapAccessory_t inHapObject[],  json_object **OutHapObjectJson, 
                                                int accessoryID, int serviceID, int characteristicID, mico_Context_t * const inContext);
static OSStatus HKCreateHAPWriteRespond( struct _hapAccessory_t inHapObject[],  json_object *inputHapObjectJson, json_object **OutHapObjectJson,
                                                int accessoryID, int serviceID, int characteristicID, mico_Context_t * const inContext);
void _HKReadCharacteristicValue_respond(struct _hapAccessory_t inHapObject[], int accessoryID, int serviceID, 
                                        int characteristicID, json_object **OutHapObjectJson, mico_Context_t * const inContext);
void _HKWriteCharacteristicValue_respond(struct _hapAccessory_t inHapObject[], int accessoryID, int serviceID, 
                                        int characteristicID, json_object **OutHapObjectJson, mico_Context_t * const inContext);


void homeKitListener_thread(void *inContext)
{
  ha_log_trace();
  OSStatus err = kUnknownErr;
  int j;
  Context = inContext;
  struct sockaddr_t addr;
  int sockaddr_t_size;
  fd_set readfds;
  char ip_address[16];
  
  int homeKitlistener_fd = -1;
  HKSetPassword (password);
  Context->appStatus.haPairSetupRunning = false;
  HKCharacteristicInit(inContext);
  /*Establish a TCP server fd that accept the tcp clients connections*/ 
  homeKitlistener_fd = socket( AF_INET, SOCK_STREAM, IPPROTO_TCP );
  require_action(IsValidSocket( homeKitlistener_fd ), exit, err = kNoResourcesErr );
  addr.s_ip = INADDR_ANY;
  addr.s_port = HA_SERVER_PORT;
  err = bind(homeKitlistener_fd, &addr, sizeof(addr));
  require_noerr( err, exit );

  err = listen(homeKitlistener_fd, 0);
  require_noerr( err, exit );

  ha_log("HomeKit Server established at port: %d, fd: %d", HA_SERVER_PORT, homeKitlistener_fd);
  
  while(1){
    FD_ZERO(&readfds);
    FD_SET(homeKitlistener_fd, &readfds);
    select(1, &readfds, NULL, NULL, NULL);

    /*Check tcp connection requests */
    if(FD_ISSET(homeKitlistener_fd, &readfds)){
      sockaddr_t_size = sizeof(struct sockaddr_t);
      j = accept(homeKitlistener_fd, &addr, &sockaddr_t_size);
      if (j > 0) {
        inet_ntoa(ip_address, addr.s_ip );
        ha_log("HomeKit Client %s:%d connected, fd: %d", ip_address, addr.s_port, j);
        ha_log("memory>>>>>>>>: %d", mico_memory_info()->free_memory);
        err = mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, "HomeKit Client", homeKitClient_thread, 0xA00, &j);  
        if(err != kNoErr){
          ha_log("HomeKit Client for fd %d create failed", j);
          SocketClose(&j);
        }
      }
    }
   }

exit:
    ha_log("Exit: HomeKit Server exit with err = %d", err);
    mico_rtos_delete_thread(NULL);
    return;
}

void homeKitClient_thread(void *inFd)
{
  ha_log_trace();
  OSStatus err;
  int clientFd = *(int *)inFd;
  struct timeval_t t;
  HTTPHeader_t *httpHeader = NULL;
  int selectResult;
  fd_set      readfds;
  HK_Context_t hkContext;

  memset(&hkContext, 0x0, sizeof(HK_Context_t));
  hkContext.session = HKSNewSecuritySession();
  require_action(hkContext.session, exit, err = kNoMemoryErr);

  httpHeader = calloc(1, sizeof( HTTPHeader_t ) );
  require_action( httpHeader, exit, err = kNoMemoryErr );

  t.tv_sec = 60;
  t.tv_usec = 0;
  ha_log("Free memory1: %d", mico_memory_info()->free_memory);

  while(1){
    if(hkContext.session->established == true && hkContext.session->recvedDataLen > 0){
       err = HKhandleIncomeingMessage(clientFd, httpHeader, &hkContext, Context);
    }else{
      FD_ZERO(&readfds);
      FD_SET(clientFd, &readfds);
      selectResult = select(clientFd + 1, &readfds, NULL, NULL, &t);
      require( selectResult >= 0, exit );
      if(FD_ISSET(clientFd, &readfds)){
        err = HKhandleIncomeingMessage(clientFd, httpHeader, &hkContext, Context);
        require_noerr(err, exit);
      }
    }
  }

exit:
  SocketClose(&clientFd);
  HTTPHeaderClear( httpHeader );
  if(httpHeader)    free(httpHeader);

  HKCleanPairSetupInfo(&hkContext.pairInfo, Context);
  HKCleanPairVerifyInfo(&hkContext.pairVerifyInfo);
  free(hkContext.session);
  ha_log("Last Free memory1: %d", mico_memory_info()->free_memory);
  mico_rtos_delete_thread(NULL);
  return;
}

void _HKWriteCharacteristicValue_respond(struct _hapAccessory_t inHapObject[], int accessoryID, int serviceID, 
                                        int characteristicID, json_object **OutHapObjectJson, mico_Context_t * const inContext)
{
  HkStatus err;
  json_object *characteristic, *errObject;
  value_union value;
  struct _hapCharacteristic_t  pCharacteristic;

  characteristic = json_object_new_object();
  errObject = json_object_new_object();
  *OutHapObjectJson = characteristic;

  pCharacteristic = inHapObject[accessoryID-1].services[serviceID-1].characteristic[characteristicID-1];

  if(pCharacteristic.type==0){
    /*Type*/
    json_object_object_add( characteristic, "type", json_object_new_string("public.hap.characteristic.unknown"));

    /*Instance ID*/
    json_object_object_add( characteristic, "instanceID", json_object_new_int(characteristicID));

    /*Error Code*/
    json_object_object_add( errObject, "developerMessage", json_object_new_string("Write a characteristic that is not existed") ); 
    json_object_object_add( errObject, "errorCode", json_object_new_int(kHKResourceErr)); 
    json_object_object_add( characteristic, "response", errObject);

    return;
  }

  /*Type*/
  json_object_object_add( characteristic, "type", json_object_new_string(pCharacteristic.type));

  /*Instance ID*/
  json_object_object_add( characteristic, "instanceID", json_object_new_int(characteristicID));

  /*Error Code*/
  if(pCharacteristic.secureWrite == false){
    json_object_object_add( errObject, "developerMessage", json_object_new_string("Write a characteristic that is not writeable") ); 
    json_object_object_add( errObject, "errorCode", json_object_new_int(kHKWriteToROErr)); 
    json_object_object_add( characteristic, "response", errObject);
    return;
  }

  /*Value*/
  err = HKReadCharacteristicValue(accessoryID, serviceID, characteristicID, &value, inContext);

  if(err != kNoErr){
    json_object_object_add( errObject, "developerMessage", json_object_new_string("Write a characteristic err") ); 
    json_object_object_add( errObject, "errorCode", json_object_new_int(err) ); 
    json_object_object_add( characteristic, "response", errObject);
    return;
  }else{
    json_object_object_add( errObject, "developerMessage", json_object_new_string("No error occurred") ); 
    json_object_object_add( errObject, "errorCode", json_object_new_int(err) ); 
    json_object_object_add( characteristic, "response", errObject );

    switch(pCharacteristic.valueType){
      case ValueType_bool:
        json_object_object_add( characteristic, "value", json_object_new_boolean(value.boolValue));
        break;
      case ValueType_int:
        json_object_object_add( characteristic, "value", json_object_new_int(value.intValue));
        break;
      case ValueType_float:
        json_object_object_add( characteristic, "value", json_object_new_double(value.floatValue));
        break;
      case ValueType_string:
        json_object_object_add( characteristic, "value", json_object_new_string(value.stringValue));
        break;
      case ValueType_date:
        json_object_object_add( characteristic, "value", json_object_new_string(value.stringValue));
        break;
      case ValueType_null:
        break;
      default:
        break;
    }  
    return;
  }
}

static HkStatus _HKReadFromOnecharacteristic( struct _hapAccessory_t inHapObject[], int accessoryID, int serviceID, 
                                              int characteristicID, json_object *characteristic, value_union *value, 
                                              uint32_t *InstanceID )
{
  HkStatus err = kNoErr;
  json_object *                 valJsonObject;
  struct _hapCharacteristic_t   pCharacteristic;
  int _instanceID = 0;

  json_object_object_foreach(characteristic, key, val) {
    if(!strcmp(key, "instanceID"))
      _instanceID = json_object_get_int(val);
    else if(!strcmp(key, "value"))
      valJsonObject = val;
  }

  if(characteristicID) _instanceID = characteristicID;
  if(InstanceID != NULL) *InstanceID = _instanceID;

  pCharacteristic = inHapObject[accessoryID-1].services[serviceID-1].characteristic[_instanceID-1];

  require_action(_instanceID&&pCharacteristic.type, exit, err = kHKResourceErr);


  switch(pCharacteristic.valueType){
    case ValueType_bool:
      (*value).boolValue = json_object_get_boolean(valJsonObject);
      break;
    case ValueType_int:
      (*value).intValue = json_object_get_int(valJsonObject);
      break;
    case ValueType_float:
      (*value).floatValue = json_object_get_double(valJsonObject);
      break;
    case ValueType_string:
      (*value).stringValue = (char *)json_object_get_string(valJsonObject);
      break;
    case ValueType_date:
      (*value).dateValue = (char *)json_object_get_string(valJsonObject);
      break;
    case ValueType_null:
      break;
    default:
      break;
  }  

exit:
  return err;
}

static HkStatus _HKReadFromOneService( json_object *inputHapObjectJson, json_object **OutHapObjectJson)
{
  HkStatus err = kNoErr;
  json_object *characteristics = NULL;

  json_object_object_foreach(inputHapObjectJson, key, val) {
    if(!strcmp(key, "characteristics"))
      characteristics = val;
  } 
  require_action(characteristics && json_object_get_type(characteristics) == json_type_array, exit, err = kHKMalformedErr);

exit:
  *OutHapObjectJson = characteristics;
  return err;
}

static HkStatus _HKReadFromOneAccessory( json_object *inputHapObjectJson, json_object **OutHapObjectJson)
{
  HkStatus err = kNoErr;
  json_object *services = NULL;

  json_object_object_foreach(inputHapObjectJson, key, val) {
    if(!strcmp(key, "characteristics"))
      services = val;
  } 
  require_action(services && json_object_get_type(services) == json_type_array, exit, err = kHKMalformedErr);

exit:
  *OutHapObjectJson = services;
  return err;
}

static HkStatus HKCreateHAPWriteRespond(  struct _hapAccessory_t inHapObject[], json_object *inputHapObjectJson, json_object **OutHapObjectJson,
                                          int accessoryID, int serviceID, int characteristicID, mico_Context_t * const inContext)
{
  HkStatus err = kNoErr;
  uint32_t characteristicIndex;
  value_union value;
  int idx, idxService, length, serviceLength;
  bool moreComing;
  int _serviceID;

  json_object *services = NULL, *service, *characteristics = NULL, *outCharacteristics = NULL, *characteristic, *outCharacteristic;
  json_object *outServices = NULL, *outService = NULL;

  *OutHapObjectJson = NULL;

  if(serviceID == 0 && characteristicID == 0){
    err = _HKReadFromOneAccessory( inputHapObjectJson, &services);
    require_noerr(err, exit);

    outServices = json_object_new_array();
    *OutHapObjectJson = outServices;

    serviceLength = json_object_array_length(services);
    
    for(idxService = 0; idxService < serviceLength; idxService ++){
      service = json_object_array_get_idx(services, idxService);

      /*Read service's instance id*/
      _serviceID = 0;
      json_object_object_foreach(service, key, val) {
        if(!strcmp(key, "type"))
          _serviceID = json_object_get_int(val);
      }
      require_action(_serviceID, exit, err = kHKMalformedErr);

      outService = json_object_new_object();
      json_object_object_add( outService, "type",        json_object_new_string(inHapObject[accessoryID-1].services[_serviceID-1].type));
      json_object_object_add( outService, "instanceID",  json_object_new_int(_serviceID));
      outCharacteristics = json_object_new_array();
      json_object_object_add( outService, "characteristics",  outCharacteristics);

      /*read characteristics array from a service*/
      err = _HKReadFromOneService( service, &characteristics);
      require_noerr(err, exit);

      length = json_object_array_length(characteristics);
      /*Write to characteristics*/
      for(idx = 0; idx < length; idx ++){
        characteristic = json_object_array_get_idx(characteristics, idx);
        err = _HKReadFromOnecharacteristic( inHapObject, accessoryID, _serviceID, 0, characteristic, &value, &characteristicIndex );
        require_noerr(err, exit);
        if(idx < length - 1) moreComing = true;
        else moreComing = false;
        HKWriteCharacteristicValue(accessoryID, _serviceID, characteristicIndex, value, moreComing, inContext);
      }

      /*Read operation result*/
      for(idx = 0; idx < length; idx ++){
        characteristic = json_object_array_get_idx(outCharacteristics, idx);
        err = _HKReadFromOnecharacteristic( inHapObject, accessoryID, _serviceID, 0, characteristic, &value, &characteristicIndex );
        require_noerr_action(err, exit, json_object_put(outCharacteristics));
        _HKWriteCharacteristicValue_respond(inHapObject, accessoryID, _serviceID, characteristicIndex, &outCharacteristic, inContext);
        if(outCharacteristic)
          json_object_array_add (outCharacteristics, outCharacteristic);
      }
    }

  }

  else if (characteristicID == 0){
    err = _HKReadFromOneService( inputHapObjectJson, &characteristics);
    require_noerr(err, exit);

    length = json_object_array_length(characteristics);
    /*Write to characteristics*/
    for(idx = 0; idx < length; idx ++){
      characteristic = json_object_array_get_idx(characteristics, idx);
      err = _HKReadFromOnecharacteristic( inHapObject, accessoryID, serviceID, 0, characteristic, &value, &characteristicIndex );
      require_noerr(err, exit);
      if(idx < length - 1) moreComing = true;
      else moreComing = false;
      HKWriteCharacteristicValue(accessoryID, serviceID, characteristicIndex, value, moreComing, inContext);
    }

    /*Read operation result*/
    outCharacteristics = json_object_new_array();
    *OutHapObjectJson = outCharacteristics;

    for(idx = 0; idx < length; idx ++){
      characteristic = json_object_array_get_idx(outCharacteristics, idx);
      err = _HKReadFromOnecharacteristic( inHapObject, accessoryID, serviceID, 0, characteristic, &value, &characteristicIndex );
      require_noerr_action(err, exit, json_object_put(outCharacteristics));
      _HKWriteCharacteristicValue_respond(inHapObject, accessoryID, serviceID, characteristicIndex, &outCharacteristic, inContext);
      if(outCharacteristic)
        json_object_array_add (outCharacteristics, outCharacteristic);
    }

  }
  else{
    err = _HKReadFromOnecharacteristic( inHapObject, accessoryID, serviceID, characteristicID, inputHapObjectJson, &value, NULL );
    require_noerr(err, exit);
    /*Write to characteristic*/
    HKWriteCharacteristicValue(accessoryID, serviceID, characteristicID, value, false, inContext);
    /*Read operation result*/
    _HKWriteCharacteristicValue_respond(inHapObject, accessoryID, serviceID, characteristicID, OutHapObjectJson, inContext);

  }

exit:
  if(err!=kNoErr && *OutHapObjectJson){
    json_object_put(*OutHapObjectJson);
    *OutHapObjectJson = NULL;
  }
  return err;
}

void _HKReadCharacteristicValue_respond(struct _hapAccessory_t inHapObject[], int accessoryID, int serviceID, 
                                        int characteristicID, json_object **OutHapObjectJson, mico_Context_t * const inContext)
{
  json_object *characteristic, *errObject;
  value_union value;
  struct _hapCharacteristic_t  pCharacteristic;

  pCharacteristic = inHapObject[accessoryID-1].services[serviceID-1].characteristic[characteristicID-1];
  require_action(pCharacteristic.type, exit, *OutHapObjectJson = NULL);

  characteristic = json_object_new_object();
  *OutHapObjectJson = characteristic;

  if(pCharacteristic.secureRead == false){
    errObject = json_object_new_object();
    json_object_object_add( errObject, "developerMessage", json_object_new_string("Read a characteristic that is not readable") ); 
    json_object_object_add( errObject, "errorCode", json_object_new_int(kHKWriteToROErr)); 
    json_object_object_add( characteristic, "response", errObject);
    return;
  }

  /*Type*/
  json_object_object_add( characteristic, "type", json_object_new_string(pCharacteristic.type));

  /*Instance ID*/
  json_object_object_add( characteristic, "instanceID", json_object_new_int(characteristicID));

  if(pCharacteristic.hasStaticValue){
    value = pCharacteristic.value;
  }
  else
    HKReadCharacteristicValue(accessoryID, serviceID, characteristicID, &value, inContext);

  switch(pCharacteristic.valueType){
    case ValueType_bool:
      json_object_object_add( characteristic, "value", json_object_new_boolean(value.boolValue));
      break;
    case ValueType_int:
      json_object_object_add( characteristic, "value", json_object_new_int(value.intValue));
      break;
    case ValueType_float:
      json_object_object_add( characteristic, "value", json_object_new_double(value.floatValue));
      break;
    case ValueType_string:
      json_object_object_add( characteristic, "value", json_object_new_string(value.stringValue));
      break;
    case ValueType_date:
      json_object_object_add( characteristic, "value", json_object_new_string(value.stringValue));
      break;
    case ValueType_null:
      break;
    default:
      break;
  }  

exit:
  return;

}

static HkStatus HKCreateHAPReadRespond( struct _hapAccessory_t inHapObject[],  json_object **OutHapObjectJson, 
                                        int accessoryID, int serviceID, int characteristicID, mico_Context_t * const inContext)
{
  HkStatus err = kNoErr;
  uint32_t characteristicIndex;

  json_object *characteristics, *characteristic;

  if(characteristicID == 0){
    characteristics = json_object_new_array();
    *OutHapObjectJson = characteristics;

    for(characteristicIndex = 0; characteristicIndex < MAXCharacteristicPerService; characteristicIndex++){
      _HKReadCharacteristicValue_respond(inHapObject, accessoryID, serviceID, characteristicIndex+1, &characteristic, inContext); 
      require(characteristic, exit);  
      json_object_array_add( characteristics, characteristic ); 
    }
  }
  else
    _HKReadCharacteristicValue_respond(inHapObject, accessoryID, serviceID, characteristicID, OutHapObjectJson, inContext);   

exit:
  return err;

}

static HkStatus HKCreateHAPAttriDataBase( struct _hapAccessory_t inHapObject[],  json_object **OutHapObjectJson, mico_Context_t * const inContext)
{
  HkStatus err = kNoErr;
  uint32_t accessoryIndex, serviceIndex, characteristicIndex;
  struct _hapCharacteristic_t             pCharacteristic;
  bool hasConstraint = false;
  value_union value;

  json_object *hapJsonObject, *accessories, *accessory, *services, *service, *characteristics, *characteristic, *properties;
  json_object *constraints, *metaData;

  hapJsonObject = json_object_new_object();

  for(accessoryIndex = 0; accessoryIndex < NumberofAccessories; accessoryIndex++){
    accessories = json_object_new_array();
    json_object_object_add( hapJsonObject, "accessories", accessories ); 

    accessory = json_object_new_object();
    json_object_array_add (accessories, accessory);

    json_object_object_add( accessory, "instanceID", json_object_new_int(accessoryIndex+1) );   
    services = json_object_new_array();
    json_object_object_add( accessory, "services", services);

    for(serviceIndex = 0; serviceIndex < MAXServicePerAccessory; serviceIndex++){
      if(inHapObject[0].services[serviceIndex].type == 0)
        break;
      service = json_object_new_object();

      json_object_object_add( service, "type",        json_object_new_string(inHapObject[0].services[serviceIndex].type));
      json_object_object_add( service, "instanceID",  json_object_new_int(serviceIndex+1));

      characteristics = json_object_new_array();

      json_object_object_add( service, "characteristics",  characteristics);

      for(characteristicIndex = 0; characteristicIndex < MAXCharacteristicPerService; characteristicIndex++){
        pCharacteristic = inHapObject[0].services[serviceIndex].characteristic[characteristicIndex];
        if(pCharacteristic.type){
          characteristic = json_object_new_object();
          /*Type*/
          json_object_object_add( characteristic, "type", json_object_new_string(pCharacteristic.type));

          /*Instance ID*/
          json_object_object_add( characteristic, "instanceID", json_object_new_int(characteristicIndex+1));

          /*Value*/
          if(pCharacteristic.hasStaticValue)
            value = pCharacteristic.value;
          else
            HKReadCharacteristicValue(accessoryIndex+1, serviceIndex+1, characteristicIndex+1, &value, inContext);

          switch(pCharacteristic.valueType){
            case ValueType_bool:
              json_object_object_add( characteristic, "value", json_object_new_boolean(value.boolValue));
              break;
            case ValueType_int:
              json_object_object_add( characteristic, "value", json_object_new_int(value.intValue));
              break;
            case ValueType_float:
              json_object_object_add( characteristic, "value", json_object_new_double(value.floatValue));
              break;
            case ValueType_string:
              json_object_object_add( characteristic, "value", json_object_new_string(value.stringValue));
              break;
            case ValueType_date:
              json_object_object_add( characteristic, "value", json_object_new_string(value.dateValue));
              break;
            case ValueType_null:
              break;
            default:
              break;
          }

          /*Properties*/
          properties = json_object_new_array();
          if(pCharacteristic.secureRead)
            json_object_array_add( properties, json_object_new_string("secureRead") ); 
          if(pCharacteristic.secureWrite)
            json_object_array_add( properties, json_object_new_string("secureWrite") ); 
          json_object_object_add( characteristic, "properties", properties);

          /*Metadata*/
          hasConstraint = false;
          if(pCharacteristic.hasMinimumValue || pCharacteristic.hasMaximumValue || pCharacteristic.hasMinimumStep ||
             pCharacteristic.precision || pCharacteristic.maxLength )
            hasConstraint = true;

          if(hasConstraint || pCharacteristic.description || pCharacteristic.format || pCharacteristic.unit){
            metaData = json_object_new_object();
            json_object_object_add( characteristic, "metaData", metaData);

            if(hasConstraint){
              constraints = json_object_new_object();
              json_object_object_add( metaData, "constraints",  constraints);

              if(pCharacteristic.hasMinimumValue){
                switch(pCharacteristic.valueType){
                  case ValueType_int:
                  json_object_object_add( constraints, "minimumValue",  json_object_new_int(pCharacteristic.minimumValue.intValue) );
                  break;
                case ValueType_float:
                  json_object_object_add( constraints, "minimumValue",  json_object_new_double(pCharacteristic.minimumValue.floatValue) );
                  break;
                default:
                  break;
                }
              }

              if(pCharacteristic.hasMaximumValue){
                switch(pCharacteristic.valueType){
                  case ValueType_int:
                    json_object_object_add( constraints, "maximumValue",  json_object_new_int(pCharacteristic.maximumValue.intValue) );
                    break;
                  case ValueType_float:
                    json_object_object_add( constraints, "maximumValue",  json_object_new_double(pCharacteristic.maximumValue.floatValue) );
                    break;
                  default:
                    break;
                }
              }

              if(pCharacteristic.hasMinimumStep){
                switch(pCharacteristic.valueType){
                  case ValueType_int:
                    json_object_object_add( constraints, "minimumStep",  json_object_new_int(pCharacteristic.minimumStep.intValue) );
                    break;
                  case ValueType_float:
                    json_object_object_add( constraints, "minimumStep",  json_object_new_double(pCharacteristic.minimumStep.floatValue) );
                    break;
                  default:
                    break;
                }
              }

              if(pCharacteristic.hasPrecision)
                json_object_object_add( constraints, "precision",     json_object_new_double(pCharacteristic.precision)    );
                
              if(pCharacteristic.hasMaxLength){
                json_object_object_add( constraints, "maxLength",     json_object_new_int(pCharacteristic.maxLength)    );
              }
            }

            if(pCharacteristic.description)
              json_object_object_add( metaData, "description", json_object_new_string(pCharacteristic.description));

            if(pCharacteristic.format)
              json_object_object_add( metaData, "format", json_object_new_string(pCharacteristic.format));

            if(pCharacteristic.unit)
              json_object_object_add( metaData, "unit", json_object_new_string(pCharacteristic.unit));
          } 

          json_object_array_add( characteristics, characteristic ); 
        }
      }
      
      json_object_array_add( services, service ); 
    }    
  }
  
  *OutHapObjectJson = hapJsonObject;

//exit:
  return err;

}

OSStatus HKSendResponseMessage(int sockfd, HkStatus hkErr, char * errorMessage, uint8_t *payload, int payloadLen, HK_Context_t *inHkContext )
{
  OSStatus err;
  json_object *respondErrObject = NULL;
  uint8_t *httpResponse = NULL;
  size_t httpResponseLen = 0;
  int status = 200;
  const char *buffer = NULL;
  int bufferLen;

  if(hkErr == kNoErr){
    buffer = (const char *)payload;
    bufferLen = payloadLen;
    err = CreateHTTPRespondMessageNoCopy( status, kMIMEType_HAP_JSON, bufferLen, &httpResponse, &httpResponseLen );
  }else{
    respondErrObject = json_object_new_object();
    json_object_object_add( respondErrObject, "developerMessage", json_object_new_string(errorMessage) ); 
    json_object_object_add( respondErrObject, "errorCode", json_object_new_int(hkErr) ); 

    buffer = json_object_to_json_string(respondErrObject);
    bufferLen = strlen(buffer);
    ha_log("Json cstring generated, memory remains %d", mico_memory_info()->free_memory);

    if(hkErr == kHKURLErr || hkErr == kHKMalformedErr || hkErr == kHKWriteToROErr || hkErr == kHKReadFromWOErr || hkErr == kHKParamErr )
      err = CreateHTTPRespondMessageNoCopy( kStatusForbidden, kMIMEType_HAP_JSON, bufferLen, &httpResponse, &httpResponseLen );
    else
      err = CreateHTTPRespondMessageNoCopy( kStatusInternalServerErr, kMIMEType_HAP_JSON, bufferLen, &httpResponse, &httpResponseLen );
  }
  require_noerr( err, exit );
  require( httpResponse, exit );

  err = HKSecureSocketSend( sockfd, httpResponse, httpResponseLen, inHkContext->session );
  require_noerr( err, exit );
  err = HKSecureSocketSend( sockfd, (uint8_t *)buffer, bufferLen, inHkContext->session );
  require_noerr( err, exit ); 

exit:
  if(respondErrObject) json_object_put(respondErrObject);
  if(httpResponse) free(httpResponse);
  return err;
}



OSStatus HKhandleIncomeingMessage(int sockfd, HTTPHeader_t *httpHeader, HK_Context_t *inHkContext, mico_Context_t * const inContext)
{
  OSStatus err = kNoErr;
  HkStatus hkErr = kNoErr;
  printbuf *buffer = NULL;
  char *pos1, *pos2, *pos3;
  err = HKSocketReadHTTPHeader( sockfd, httpHeader, inHkContext->session );
  int accessoryID, serviceID, characteristicID;
  static json_object *outhapJsonObject, *inhapJsonObject;

  switch ( err )
  {
    case kNoErr:
        err = HKSocketReadHTTPBody( sockfd, httpHeader, inHkContext->session );
        require_noerr(err, exit);
        /*Pair set engine*/
        if(HTTPHeaderMatchURL( httpHeader, kPAIRSETUP ) == kNoErr) {
          err = HKPairSetupEngine( sockfd, httpHeader, &inHkContext->pairInfo, inContext );
          require_noerr( err, exit );
          if(inContext->appStatus.haPairSetupRunning == false){err = kConnectionErr; goto exit;};
        }
        /*Pair verify engine*/ 
        else if(HTTPHeaderMatchURL( httpHeader, kPAIRVERIFY ) == kNoErr){
          if(inHkContext->pairVerifyInfo == NULL){
            inHkContext->pairVerifyInfo = HKCreatePairVerifyInfo();
            require_action( inHkContext->pairVerifyInfo, exit, err = kNoMemoryErr );
          }
          err = HKPairVerifyEngine( sockfd, httpHeader, inHkContext->pairVerifyInfo, inContext );
          require_noerr_action( err, exit, HKCleanPairVerifyInfo(&inHkContext->pairVerifyInfo));
          if(inHkContext->pairVerifyInfo->verifySuccess){
            inHkContext->session->established = true;
            memcpy(inHkContext->session->InputKey,  inHkContext->pairVerifyInfo->C2AKey, 32);
            memcpy(inHkContext->session->OutputKey, inHkContext->pairVerifyInfo->A2CKey, 32);
            HKCleanPairVerifyInfo(&inHkContext->pairVerifyInfo);
          }
        }
        /*Read accessories database*/
        else if(HTTPHeaderMatchURL( httpHeader, kReadAcc ) == kNoErr){
          err = HKCreateHAPAttriDataBase(hapObjects, &outhapJsonObject, inContext);
          require_noerr( err, exit );
          buffer = json_object_to_json_string_ex(outhapJsonObject);
          ha_log("Json cstring generated, memory remains %d, %s", mico_memory_info()->free_memory, buffer->buf);
          json_object_put(outhapJsonObject);
          outhapJsonObject = NULL;
          err = HKSendResponseMessage(sockfd, hkErr, "Read Data base Error", (uint8_t *)buffer->buf, strlen(buffer->buf), inHkContext);
          require_noerr(err, exit);
        }
        /*Read or write accessories characteristics*/
        else if (HTTPHeaderMatchPartialURL( httpHeader, kAccessories ) != NULL){
          pos1 = HTTPHeaderMatchPartialURL( httpHeader, kAccessories ) ;
          pos2 = HTTPHeaderMatchPartialURL( httpHeader, kServices ) ;
          pos3 = HTTPHeaderMatchPartialURL( httpHeader, kCharacteristics ) ;

          accessoryID = atoi(pos1+strlen(kAccessories));
          
          if( pos2+strlen(kServices) ==  httpHeader->url.pathPtr+httpHeader->url.pathLen ){
            serviceID = 0; 
            characteristicID = 0; 
          }
          else{
            if( pos3+strlen(kCharacteristics) ==  httpHeader->url.pathPtr+httpHeader->url.pathLen )
              characteristicID = 0; 
            else
            characteristicID = atoi(pos3+strlen(kCharacteristics));
          }
            serviceID = atoi(pos2+strlen(kServices));
          
          ha_log("Accessory: %d, service: %d, characteristic: %d", accessoryID, serviceID,characteristicID);

          /*Read characteristic*/
          if(HTTPHeaderMatchMethod( httpHeader, "GET")!=kNotFoundErr){
            hkErr = HKCreateHAPReadRespond(hapObjects, &outhapJsonObject, accessoryID, serviceID, characteristicID, inContext);
            buffer = json_object_to_json_string_ex(outhapJsonObject);
            ha_log("Json cstring generated, memory remains %d", mico_memory_info()->free_memory);
            json_object_put(outhapJsonObject);
            outhapJsonObject = NULL;
            err = HKSendResponseMessage(sockfd, hkErr, "Read characteristic Error", (uint8_t *)buffer->buf, strlen(buffer->buf), inHkContext);
            require_noerr(err, exit);
          }
          /*Write characteristic*/
          else if(HTTPHeaderMatchMethod( httpHeader, "PUT")!=kNotFoundErr){
            inhapJsonObject = json_tokener_parse(httpHeader->extraDataPtr);
            require_string(inhapJsonObject, exit, "json_tokener_parse error");
            hkErr = HKCreateHAPWriteRespond(hapObjects, inhapJsonObject,  &outhapJsonObject, accessoryID, serviceID, characteristicID, inContext);
            json_object_put(inhapJsonObject);
            if(outhapJsonObject){
              buffer = json_object_to_json_string_ex(outhapJsonObject);
              ha_log("Json cstring generated, memory remains %d", mico_memory_info()->free_memory);
              json_object_put(outhapJsonObject);
              outhapJsonObject = NULL;
              err = HKSendResponseMessage(sockfd, hkErr, "Read characteristic Error", (uint8_t *)buffer->buf, strlen(buffer->buf), inHkContext);
              require_noerr(err, exit);             
            }else{
              err = HKSendResponseMessage(sockfd, hkErr, "Write characteristic Error", NULL, 0, inHkContext);
              require_noerr(err, exit);
            }

          }else
          /*Unkown Methold*/
            err = HKSendResponseMessage(sockfd, kHKMethodErr, "Unsupport HTTP method", NULL, 0, inHkContext);
            require_noerr(err, exit);
        }
        /*Unknow URL path*/
      else{
        err = HKSendResponseMessage(sockfd, kHKURLErr, "Unsupport HTTP url", NULL, 0, inHkContext);
        require_noerr(err, exit);
      }

    break;

    case EWOULDBLOCK:
        // NO-OP, keep reading
    break;

    case kNoSpaceErr:
      ha_log("ERROR: Cannot fit HTTPHeader.");
      goto exit;
    break;

    case kConnectionErr:
      // NOTE: kConnectionErr from SocketReadHTTPHeader means it's closed
      ha_log("ERROR: Connection closed.");
      goto exit;
    break;
    default:
      ha_log("ERROR: HTTP Header parse internal error: %d", err);
      goto exit;
  }
exit:
  HTTPHeaderClear( httpHeader );
  if(outhapJsonObject) json_object_put(outhapJsonObject);
  if(buffer) printbuf_free(buffer);
  return err;

}






