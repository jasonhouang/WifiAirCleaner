#include "Common.h"
#include "MICODefine.h"
#include "HomeKitProfiles.h"
#include "StringUtils.h"
#include "MDNSUtils.h"

extern void HKBonjourUpdateStateNumber( mico_Context_t * const inContext );


HkStatus HKReadCharacteristicValue(int accessoryID, int serviceID, int characteristicID, value_union *value, mico_Context_t * const inContext)
{
  (void)accessoryID;
  HkStatus err; 

  if(serviceID == 1){
    if(characteristicID == 1)
      (*value).stringValue = inContext->flashContentInRam.micoSystemConfig.name;
      err = kNoErr;
  }
  else if(serviceID == 2){
#ifdef lightbulb
    switch(characteristicID){
      case 1:
        (*value).boolValue = inContext->appStatus.service.on; 
        err = inContext->appStatus.service.on_status;
        break;
      
      case 2:
        (*value).intValue = inContext->appStatus.service.brightness;    
        err = inContext->appStatus.service.brightness_status;
        break;

      case 3:
        (*value).floatValue = inContext->appStatus.service.hue;    
        err = inContext->appStatus.service.hue_status;
        break;
           
      case 4:
        (*value).floatValue = inContext->appStatus.service.saturation;    
        err = inContext->appStatus.service.saturation_status;
        break;

      case 5:
        (*value).stringValue = inContext->appStatus.service.name;
        err = inContext->appStatus.service.name_status;
        break;
      }
#endif

#ifdef thermostat
    switch(characteristicID){
      case 1:
        (*value).stringValue = inContext->appStatus.service.heating_cooling_current; 
        err = inContext->appStatus.service.heating_cooling_current_status;
        break;
      
      case 2:
        (*value).stringValue = inContext->appStatus.service.heating_cooling_target;    
        err = inContext->appStatus.service.heating_cooling_target_status;
        break;

      case 3:
         (*value).floatValue = inContext->appStatus.service.temperature_current;    
        err = inContext->appStatus.service.temperature_current_status;
        break;
           
      case 4:
        (*value).floatValue = inContext->appStatus.service.temperature_target;    
        err = inContext->appStatus.service.temperature_target_status;
        break;

      case 5:
        (*value).stringValue = inContext->appStatus.service.temperature_units; 
        err = inContext->appStatus.service.name_status;
        break;

      case 6:
        (*value).floatValue = inContext->appStatus.service.relative_humidity_current; 
        err = inContext->appStatus.service.relative_humidity_current_status;
        break;

      case 7:
        (*value).floatValue = inContext->appStatus.service.relative_humidity_target; 
        err = inContext->appStatus.service.relative_humidity_target_status;
        break;


      case 8:
        (*value).floatValue = inContext->appStatus.service.heating_threshold; 
        err = inContext->appStatus.service.heating_threshold_status;
        break;


      case 9:
        (*value).floatValue = inContext->appStatus.service.cooling_threshold; 
        err = inContext->appStatus.service.cooling_threshold_status;
        break;

      case 10:
        (*value).stringValue = inContext->appStatus.service.name;
        err = inContext->appStatus.service.name_status;
        break;
      }
#endif
  }
  return err;
}


void HKWriteCharacteristicValue(int accessoryID, int serviceID, int characteristicID, value_union value, bool moreComing, mico_Context_t * const inContext)
{
  (void)accessoryID;  

  /*You can store the new value in xxxx_new and operate target later when moreComing == false
    or write to target right now!*/
  if(serviceID == 1){
    if(characteristicID == 1)
      strncpy(inContext->flashContentInRam.micoSystemConfig.name, value.stringValue, 64);
  }
  else if(serviceID == 2){
#ifdef lightbulb
    switch(characteristicID){
      case 1:
        inContext->appStatus.service.on_new = value.boolValue;
        inContext->appStatus.service.on_status = kHKBusyErr;
        break;
      
      case 2:
        inContext->appStatus.service.brightness_new = value.intValue;
        inContext->appStatus.service.brightness_status = kHKBusyErr;
        break;

      case 3:
        inContext->appStatus.service.hue_new = value.floatValue; 
        inContext->appStatus.service.hue_status = kHKBusyErr;
        break;
           
      case 4:
        inContext->appStatus.service.saturation_new = value.floatValue;
        inContext->appStatus.service.saturation_status = kHKBusyErr;
        break;

      case 5:
        strncpy(inContext->appStatus.service.name, value.stringValue, 64);
        inContext->appStatus.service.name_status = kNoErr;
        break;
      }
#endif

#ifdef thermostat
    switch(characteristicID){
      case 2:
        strncpy(inContext->appStatus.service.heating_cooling_target_new, value.stringValue, 16);
        inContext->appStatus.service.heating_cooling_target_status = kHKBusyErr;
        break;
      case 4:
        inContext->appStatus.service.temperature_target_new = value.floatValue;  
        inContext->appStatus.service.temperature_target_status = kHKBusyErr;
        break;  
      case 5:
        strncpy(inContext->appStatus.service.temperature_units_new, value.stringValue, 16);
        inContext->appStatus.service.temperature_units_status = kHKBusyErr;
        break;  
      case 7:
        inContext->appStatus.service.relative_humidity_target_new = value.floatValue;  
        inContext->appStatus.service.relative_humidity_target_status = kHKBusyErr;
        break;  
      case 8:
        inContext->appStatus.service.heating_threshold_new = value.floatValue;  
        inContext->appStatus.service.heating_threshold_status = kHKBusyErr;
        break;  
      case 9:
        inContext->appStatus.service.cooling_threshold_new = value.floatValue;  
        inContext->appStatus.service.cooling_threshold_status = kHKBusyErr;
        break;  
      case 10:
        strncpy(inContext->appStatus.service.name, value.stringValue, 64);
        inContext->appStatus.service.name_status = kNoErr;
        break;
    }
#endif
  }

    /*Operate hardware to write all Characteristic in one service in one time, this is useful when 
      one command send to taget device can set multiple Characteristic*/
  if(!moreComing){
#ifdef lightbulb
    /*Control lightbulb*/
    /*.................*/

    if(inContext->appStatus.service.on_status == kHKBusyErr){
      inContext->appStatus.service.on = inContext->appStatus.service.on_new;
      inContext->appStatus.service.on_status = kNoErr;      
    }

    if(inContext->appStatus.service.brightness_status == kHKBusyErr){
      inContext->appStatus.service.brightness = inContext->appStatus.service.brightness_new;
      inContext->appStatus.service.brightness_status = kNoErr;
    }

    if(inContext->appStatus.service.hue_status == kHKBusyErr){
      inContext->appStatus.service.hue = inContext->appStatus.service.hue_new; 
      inContext->appStatus.service.hue_status = kNoErr;
    }

    if(inContext->appStatus.service.saturation_status == kHKBusyErr){
      inContext->appStatus.service.saturation = inContext->appStatus.service.saturation_new;
      inContext->appStatus.service.saturation_status = kNoErr;
    }
#endif

#ifdef thermostat
    /*Control thermostat*/
    /*.................*/

    if(inContext->appStatus.service.heating_cooling_target_status == kHKBusyErr){
      strncpy(inContext->appStatus.service.heating_cooling_target, inContext->appStatus.service.heating_cooling_target_new, 16);
      inContext->appStatus.service.heating_cooling_target_status = kNoErr;
    }

    if(inContext->appStatus.service.temperature_target_status == kHKBusyErr){
      inContext->appStatus.service.temperature_target = inContext->appStatus.service.temperature_target_new;  
      inContext->appStatus.service.temperature_target_status = kNoErr;
    }

    if(inContext->appStatus.service.temperature_units_status == kHKBusyErr){
      strncpy(inContext->appStatus.service.temperature_units, inContext->appStatus.service.temperature_units_new, 16);
      inContext->appStatus.service.temperature_units_status = kNoErr;
    }

    if(inContext->appStatus.service.relative_humidity_target_status == kHKBusyErr){
      inContext->appStatus.service.relative_humidity_target = inContext->appStatus.service.relative_humidity_target_new;  
      inContext->appStatus.service.relative_humidity_target_status = kNoErr;
    }

    if(inContext->appStatus.service.heating_threshold_status == kHKBusyErr){
      inContext->appStatus.service.heating_threshold = inContext->appStatus.service.heating_threshold_new;  
      inContext->appStatus.service.heating_threshold_status = kNoErr;
    }

    if(inContext->appStatus.service.cooling_threshold_status == kHKBusyErr){
      inContext->appStatus.service.cooling_threshold = inContext->appStatus.service.cooling_threshold_new;  
      inContext->appStatus.service.cooling_threshold_status = kNoErr;
      
    }
#endif   
  }
    HKBonjourUpdateStateNumber( inContext );
  return;
}

void HKCharacteristicInit(mico_Context_t * const inContext)
{
#ifdef lightbulb
  inContext->appStatus.service.on                               = true;
  inContext->appStatus.service.brightness                       = 80;
  inContext->appStatus.service.hue                              = 180;
  inContext->appStatus.service.saturation                       = 80;
  strncpy(inContext->appStatus.service.name,                    "Wiliam's lightbulb", 64);
#endif  

#ifdef thermostat
  strncpy(inContext->appStatus.service.heating_cooling_current, "cool", 16); //cool, off, heat
  strncpy(inContext->appStatus.service.heating_cooling_target,  "cool", 16);  //cool, off, heat, auto
  inContext->appStatus.service.temperature_current              = 30;
  inContext->appStatus.service.temperature_target               = 26;
  strncpy(inContext->appStatus.service.temperature_units,       "celsius", 16);  //celsius, fahrenheit, kelvin
  inContext->appStatus.service.relative_humidity_current        = 75;
  inContext->appStatus.service.relative_humidity_target         = 40;
  inContext->appStatus.service.heating_threshold                = 16;
  inContext->appStatus.service.cooling_threshold                = 28;
  strncpy(inContext->appStatus.service.name,                    "Wiliam's thermostat", 64);
#endif
}