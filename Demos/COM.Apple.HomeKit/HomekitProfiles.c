#include "HomeKitProfiles.h"
#include "Common.h"
#include "MICODefine.h"



const struct _hapAccessory_t hapObjects[NumberofAccessories] = 
{
  { 
    .services = {
      [0] = {
        .type = "public.hap.service.accessory-information",
        .characteristic = {
          [0] = {
            .type = "public.hap.characteristic.name",
            .valueType = ValueType_string,
            .secureRead = true,
          },
          [1] = {
            .type = "public.hap.characteristic.manufacturer",
            .valueType = ValueType_string,
            .hasStaticValue = true,
            .value.stringValue = MANUFACTURER,
            .secureRead = true,
          },
          [2] = {
            .type = "public.hap.characteristic.serial-number",
            .valueType = ValueType_string,
            .hasStaticValue = true,
            .value.stringValue = SERIAL_NUMBER,
            .secureRead = true,
          },	
          [3] = {
            .type = "public.hap.characteristic.model",
            .valueType = ValueType_string,
            .hasStaticValue = true,
            .value.stringValue = MODEL,
            .secureRead = true,
          },
          [4] = {
            .type = "public.hap.characteristic.identify",
            .valueType = ValueType_null,
            .hasStaticValue = true,
            .value = NULL,
            .secureWrite = true,
          }	
        }
      },
#ifdef lightbulb
    [1] = {
        .type = "public.hap.service.lightbulb",
        .characteristic = {
          [0] = {
            .type = "public.hap.characteristic.on",
            .valueType = ValueType_bool,
            .secureRead = true,
            .secureWrite = true,
          },
          [1] = {
            .type = "public.hap.characteristic.brightness",
            .valueType = ValueType_int,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue = 0,
            .hasMaximumValue = true,
            .maximumValue = 100,
            .hasMinimumStep = true,
            .minimumStep = 1,
            .unit = "percentage",
          },
          [2] = {
            .type = "public.hap.characteristic.hue",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 360,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 1,
            .unit = "arcdegrees",
          }, 
          [3] = {
            .type = "public.hap.characteristic.saturation",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 100,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 1,
            .unit = "percentage",
          }, 
          [4] = {
            .type = "public.hap.characteristic.name",
            .valueType = ValueType_string,
            .secureRead = true,
            .secureWrite = true,
          }
        }
      },
#endif
#ifdef thermostat
    [1] = {
        .type = "public.hap.service.thermostat",
        .characteristic = {
          [0] = {
            .type = "public.hap.characteristic.heating-cooling.current",
            .valueType = ValueType_string,
            .hasStaticValue = false,
            .value.stringValue = "cool",  //cool, off, heat
            .secureRead = true,
          },
          [1] = {
            .type = "public.hap.characteristic.heating-cooling.target",
            .valueType = ValueType_string,
            .hasStaticValue = false,
            .value.stringValue = "auto", //cool, off, heat, auto
            .secureRead = true,
            .secureWrite = true,
          },
          [2] = {
            .type = "public.hap.characteristic.temperature.current",
            .valueType = ValueType_float,
            .secureRead = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 100,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 0.1,
          }, 
          [3] = {
            .type = "public.hap.characteristic.temperature.target",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 10,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 27,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 0.1,
          }, 
          [4] = {
            .type = "public.hap.characteristic.temperature.units",
            .valueType = ValueType_string,
            .hasStaticValue = false,
            .value.stringValue = "celsius", //celsius, fahrenheit, kelvin
            .secureRead = true,
            .secureWrite = true,
          }, 
          [5] = {
            .type = "public.hap.characteristic.relative-humidity.current",
            .valueType = ValueType_float,
            .secureRead = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 100,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 0.01,
          }, 
          [6] = {
            .type = "public.hap.characteristic.relative-humidity.target",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 100,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 1,
          }, 
          [7] = {
            .type = "public.hap.characteristic.temperature.heating-threshold",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 0,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 25,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 0.1
          }, 
          [8] = {
            .type = "public.hap.characteristic.temperature.cooling-threshold",
            .valueType = ValueType_float,
            .secureRead = true,
            .secureWrite = true,
            .hasMinimumValue = true,
            .minimumValue.floatValue = 10,
            .hasMaximumValue = true,
            .maximumValue.floatValue = 35,
            .hasMinimumStep = true,
            .minimumStep.floatValue = 0.1,
          }, 
          [9] = {
            .type = "public.hap.characteristic.name",
            .valueType = ValueType_string,
            .hasStaticValue = false,
            .value.stringValue = "William's thermostat",
            .secureRead = true,
            .secureWrite = true,
          }
        }
      }
#endif
    }
  }
};
