#ifndef __HOMEKITPROFILES_h__
#define __HOMEKITPROFILES_h__

#include "Common.h"
#include "HTTPUtils.h"

#include "external/JSON-C/json.h"

#define MAXCharacteristicPerService       20
#define MAXServicePerAccessory            10
#define NumberofAccessories                1

#define lightbulb
//#define thermostat

// ==== HKtatus ====
typedef int32_t         HkStatus;

#define kHKURLErr                     -70401   //! .
#define kHKParamErr                   -70402   //! .
#define kHKBusyErr                    -70403   //! .
#define kHKWriteToROErr               -70404   //! .
#define kHKReadFromWOErr              -70405   //! .
#define kHKResourceErr                -70407   //! .
#define kHKTimeOutErr                 -70408   //! .
#define kHKCommunicateErr             -70409   //! .
#define kHKProgressingErr             -70410   //! .
#define kHKMalformedErr               -70411   //! .
#define kHKPrivilegeErr               -70412   //! .
#define kHKInterpretErr               -70413   //! .
#define kHKMethodErr                  -70413   //! .



#define kGenericErrorEnd            -6779   //! Last generic error code (inclusive)

typedef enum _valueType{
  ValueType_bool,
  ValueType_int,
  ValueType_float,
  ValueType_string,
  ValueType_date,
  ValueType_array,
  ValueType_object,
  ValueType_null,
} valueType;

typedef union {
    bool        boolValue;
    int         intValue;
    float       floatValue;
    char        *stringValue;
    char        *dateValue;
    json_object *array;
    json_object *object;
  } value_union;


struct _hapCharacteristic_t {
  char   *type;

  bool   hasStaticValue;
  valueType valueType;
  value_union value;

  bool   secureRead;
  bool   secureWrite;

  bool   hasMinimumValue;
  union {
    int         intValue;
    float       floatValue;
  }      minimumValue;

  bool   hasMaximumValue;
  union {
    int         intValue;
    float       floatValue;
  }      maximumValue;

  bool   hasMinimumStep;
  union {
    int         intValue;
    float       floatValue;
  }      minimumStep;

  bool   hasPrecision;
  float  precision;
  bool   hasMaxLength;
  int    maxLength;
  char   *description;
  char   *format;
  char   *unit;
};

struct _hapService_t {
  char    *type;
  struct  _hapCharacteristic_t                characteristic[MAXCharacteristicPerService];
};

struct _hapAccessory_t {
  struct _hapService_t  services[MAXServicePerAccessory];
};


#endif

