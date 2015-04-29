/**
  ******************************************************************************
  * @file    NearAirDefine.h
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   This file provide constant definition and type declaration for Near
             Air running.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, NEAR Inc. SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2014 Near Inc.</center></h2>
  ******************************************************************************
  */ 

#ifndef __NEARAIRDEFINE_H
#define __NEARAIRDEFINE_H
#include "MICO.h"

#define UID_LENGTH                11
#define SN_LENGTH                 11
#define PROTOCOL_VER_LENGTH       2
#define HARDWARE_VER_LENGTH       5

enum {
	POWER_OFF = 0,
	POWER_ON,
};

typedef enum {
	AIR_ON = 0,
	AIR_SMART = 1,
	AIR_SLEEP = 2,
        AIR_FLY = 3,
}NearAir_State;

enum {
	MODE_CLOSE = 0,
	MODE_INIT = 1,
	MODE_SMART = 2,
	MODE_SLEEP = 3,
	MODE_MANUAL = 4,
        MODE_FLY = 5,
};

typedef enum
{
	MOTOR_HALT = 0,
	MOTOR_SLEEP = 1,
	MOTOR_LOW = 2,
	MOTOR_MIDIUM = 3,
	MOTOR_HIGH = 4,
	MOTOR_FLY = 5
}Motor_Position;

enum {
	CHILDLOCK_UNLOCK = 0,
	CHILDLOCK_LOCK,
};

enum {
	AQ_GOOD = 0,
	AQ_SOSO,
        AQ_BAD
};
enum {
        NIGHT = 0,
        DAY = 1,
};
typedef struct _flash_nearair_t 
{
  char          extra_data[UID_LENGTH];
  char          sn[SN_LENGTH];
  char          protocol_ver[PROTOCOL_VER_LENGTH];
  char          hardware_ver[HARDWARE_VER_LENGTH];
} flash_nearair_t;

typedef struct _current_power_status_t
{
  char          power;
}current_power_status_t;

typedef struct _current_device_status_t
{
  char          mode;
  char          position;
  char          child_lock;
  mico_semaphore_t      dev_status_change_sem;
  bool          isRemoteLogined;
} current_device_status_t;

typedef struct _current_sensor_status_t
{
  int16_t          odour;
  int16_t          dust;
  int16_t          temperature;
  int16_t          humidity;
  char             light;
} current_sensor_status_t;

typedef struct _current_air_quality_t
{
  char          airclass;
} current_air_quality_t;

typedef struct _temporary_setup_t
{
  char          motor_enable;
  char          led_enable;
  uint8_t       motor;
  uint8_t       led_r;
  uint8_t       led_g;
  uint8_t       led_b;
} temporary_setup_t;
typedef struct _nearair_Context_t
{
  /*Flash content*/
  flash_nearair_t           flashNearAirInRam;
  mico_mutex_t              flashNearAirInRam_mutex;

  /*Running status*/
  current_power_status_t       powerStatus;
  current_device_status_t      deviceStatus;
  current_sensor_status_t      sensorStatus;
  current_air_quality_t        airStatus;
  temporary_setup_t            setStatus;
} nearair_Context_t;

OSStatus NearAirReadConfiguration(nearair_Context_t *airContext);
OSStatus NearAirUpdateConfiguration(nearair_Context_t *airContext);

#endif
