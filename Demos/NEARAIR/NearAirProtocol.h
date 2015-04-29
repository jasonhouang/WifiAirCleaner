/**
  ******************************************************************************
  * @file    NearAirProtocol.h 
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   This file provides all the headers of NearAirProtocol data.
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


#ifndef __NEARAIRPROTOCOL_H
#define __NEARAIRPROTOCOL_H

#include "Common.h"
#include "MICODefine.h"
#include "NearAirDefine.h"

enum {
	CMD_LOGIN = 1001,
	CMD_HEAT_BEAT = 1002,
	CMD_SENOR_DATA = 1003,
	CMD_RUN_STATUS = 1004,
	CMD_START = 1005,
	CMD_SHUTDOWN = 1006,
	CMD_TO_SLEEP = 1007,
	CMD_TO_SMART = 1008,
	CMD_LOG_DATA = 1009,
	CMD_CHMOTOR_0 = 1010,
	CMD_CHMOTOR_1 = 1011,
	CMD_CHMOTOR_2 = 1012,
	CMD_CHMOTOR_3 = 1013,
	CMD_CHMOTOR_4 = 1014,
	CMD_CHMOTOR_5 = 1015,
	CMD_CHILD_LOCK = 1016,
	CMD_CHILD_UNLOCK = 1017,
	CMD_STATE_SYNCHRONIZATION = 1018,
	CMD_READ_PM2_5 = 1019,
	CMD_SEND_SN = 1020,
        CMD_SENSOR_SYNCHRONIZATION = 1021,
        CMD_ERROR_CODE = 1022,
        CMD_READ_HW = 1023,
        CMD_START_OTA = 1024,
        CMD_OTA = 1025,
        CMD_STOP_OTA = 1026,
        CMD_OTA_FAIL = 1027
};

typedef struct _nearair_cmd_head{
	char len[5];
	char time[11];
	char cmd[5];
	char ver[3];
}nearair_cmd_head_t;

typedef struct _nearair_cmd_login_body {
	char sn[11];
	char encrypt[33];
}nearair_cmd_login_body_t;

typedef struct _nearair_s2c_login_body {
		char sn[11];
		char login_time[11];
		char errno[5];
}nearair_s2c_login_body;

typedef struct _nearair_cmd_sensor_body {
    char *req_user;
    char *req_time;
    char *sn;
    int16_t odour;
    int16_t dust;
    int16_t temperature;
    int16_t humidity;
}nearair_cmd_sensor_body_t;

typedef struct _nearair_cmd_status_body {
    char *req_user;
    char *req_time;
    char *sn;
    uint8_t mode;
    uint8_t position;
    uint8_t child_lock;
}nearair_cmd_status_body_t;

typedef struct _nearair_cmd_return_body {
    char *req_user;
    char *req_time;
    char *sn;
    uint8_t ret;
}nearair_cmd_return_body_t;

typedef struct _nearair_cmd_log_body {
    char *sn;
    int16_t odour;
    int16_t dust;
    int16_t temperature;
    int16_t humidity;
    uint8_t mode;
    uint8_t position;
    uint8_t child_lock;
}nearair_cmd_log_body_t;

typedef struct _nearair_cmd_state_synchronization_body{
    char *sn;
    uint8_t mode;
    uint8_t position;
    uint8_t child_lock;
}nearair_cmd_state_synchronization_body_t;

typedef struct _nearair_cmd_sensor_synchronization_body{
    char *sn;
    int16_t odour;
    int16_t dust;
    int16_t temperature;
    int16_t humidity;
}nearair_cmd_sensor_synchronization_body_t;

typedef struct _nearair_cmd_hwinfo_body{
    char *req_user;
    char *req_time;
    char *sn;
    char *hd_version;
}nearair_cmd_hwinfo_body_t;

typedef struct _nearair_cmd_ota_return_body{
    char *sn;
    uint8_t ret;
}nearair_cmd_ota_return_body_t;
OSStatus nearairProtocolInit(mico_Context_t * const inContext);
int is_network_state(int state);
OSStatus nearairWlanCommandProcess(unsigned char const *inBuf, 
                                   int *inBufLen,
                                   int inSocketFd,
                                   mico_Context_t * const inContext,
                                   nearair_Context_t * const airContent);
OSStatus sppUartCommandProcess(uint8_t *inBuf, int inLen, mico_Context_t * const inContext);
OSStatus nearairc2sCommandProcess(uint8_t *inBuf, int inLen);
void set_network_state(int state, int on);

OSStatus nearair_login(mico_Context_t * const inContext);

OSStatus wifi_send_nearair_sn(nearair_Context_t * const airContent);
OSStatus send_sensor_data(nearair_Context_t * const airContent);
OSStatus send_nearair_status(nearair_Context_t * const airContent);
OSStatus send_nearair_start(nearair_Context_t * const airContent);
OSStatus send_nearair_shutdown(nearair_Context_t * const airContent);
OSStatus send_nearair_tosleep(nearair_Context_t * const airContent);
OSStatus send_nearair_tosmart(nearair_Context_t * const airContent);
OSStatus send_nearair_log(nearair_Context_t * const airContent);
OSStatus wifi_change_motor_gear(uint16_t cmd_motor,nearair_Context_t * const airContent);
OSStatus wifi_childlock_lock(uint16_t cmd_childlock,nearair_Context_t * const airContent);
OSStatus wifi_state_synchronization(nearair_Context_t * const airContent);
OSStatus wifi_sensor_synchronization(nearair_Context_t * const airContent);
int hex2str(char *sDest,const unsigned char *sSrc,int nSrclen);
#endif
