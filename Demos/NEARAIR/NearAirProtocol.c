/**
  ******************************************************************************
  * @file    NearAirProtocol.c 
  * @author  Adam Huang
  * @version V1.0.0
  * @date    05-Nov-2014
  * @brief   NearAirProtocol receive data form cloud and send data to cloud.
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

#include "MICOAppDefine.h"
#include "NearAir.h"
#include "PlatformUart.h"
#include "SocketUtils.h"
#include "debug.h"
#include "platform.h"
#include "MICONotificationCenter.h"
#include "NearAirDefine.h"
#include "PlatformRTC.h"
#include "PlatformFlash.h"
    
#include <stdio.h>

#define PROTOCOL_DEBUG

#define nearair_log(M, ...) custom_log("NEARAIR", M, ##__VA_ARGS__)
#define nearair_log_trace() custom_log_trace("NEARAIR")

#define SECRETKEY  "qwertyuiop"
#define PROTOCOL_VERSION  "01"

static mico_timer_t _heartbeat_timer;
mico_thread_t *heartbeat;
mico_semaphore_t heartbeat_sem;
static mico_mutex_t wifi_out_mutex;

static int _recved_nearair_loopback_fd = -1;
static int _nearair_status_loopback_fd = -1;

static char time_count[11] = "1234567890";
static char nearair_sn[11] = "9000000999";
static char usr_id[11] = "2085484566";
static char req_time[11] = "1234567890";
static bool heart_beat_rec = true;

//static OSStatus _ota_process(uint8_t *inBuf, int inBufLen, int *inSocketFd, mico_Context_t * const inContext);

static OSStatus start_ota_process(nearair_Context_t * const airContent);
static OSStatus _ota_process(uint8_t *inBuf, int inBufLen, mico_Context_t * const inContext);
static OSStatus stop_ota_process(uint8_t *inBuf, mico_Context_t * const inContext);
static OSStatus wifi_hardware_info(nearair_Context_t * const airContent);
//static void heart_beat_thread(void *inContext);
static void check_device_status_change_thread(void *inContent);
static void heart_beat_handle(void *inContext);

char *getstringtime(char *string_time)
{
    struct tm currentTime;
    unsigned int t_count;
    
    PlatformRTCRead( &currentTime );
    t_count = mktime(&currentTime);
    sprintf(string_time,"%d",t_count);
    return string_time;
}

OSStatus nearairProtocolInit(mico_Context_t * const inContext)
{
  nearair_log_trace();
  OSStatus err = kUnknownErr;
  (void)inContext;
  struct sockaddr_t addr;

  inContext->appStatus.isRemoteConnected = false;

  _recved_nearair_loopback_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = NEARAIR_GEN_DATA_LOOPBACK_PORT;
  err = bind(_recved_nearair_loopback_fd, &addr, sizeof(addr));
  require_noerr( err, exit );
  
  _nearair_status_loopback_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = NEARAIR_STATUS_LOOPBACK_PORT;
  err = bind(_nearair_status_loopback_fd, &addr, sizeof(addr));
  require_noerr( err, exit );
  
  mico_rtos_init_mutex(&wifi_out_mutex);
  mico_rtos_init_semaphore(&heartbeat_sem,1);
exit:
  return err;
}

OSStatus nearairWlanCommandProcess(unsigned char const*inBuf, 
                                   int *inBufLen,
                                   int inSocketFd,
                                   mico_Context_t * const inContext,
                                   nearair_Context_t * const airContent)
{
  nearair_log_trace();
  OSStatus err = kUnknownErr;
  
  nearair_cmd_head_t *head = NULL;
//  char *inBuffur
  char tempstr[11];
  unsigned int CMD_CMD = 0;
  *(char *)(inBuf + *inBufLen-1) = '\0';
#ifdef PROTOCOL_DEBUG
  nearair_log("inBuf is:%s,inBuflen is:%d",inBuf,*inBufLen);
#endif
  head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
  require_action( head, exit, err = kNoMemoryErr );
  memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
  strcpy( head->len ,strleft(tempstr,(char*)inBuf,4));
  strcpy( head->time ,strmid(tempstr,(char*)inBuf,10,4));
  strcpy( head->cmd ,strmid(tempstr,(char*)inBuf,4,14));
  strcpy( head->ver ,strmid(tempstr,(char*)inBuf,2,18));
  
#ifdef PROTOCOL_DEBUG
  nearair_log("len:%s,time:%s,cmd:%s,ver:%s",head->len,head->time,head->cmd,head->ver);
#endif
  
  CMD_CMD = atoi(head->cmd);
  if((CMD_CMD != CMD_LOGIN) && (CMD_CMD != CMD_HEAT_BEAT))
  {
	strcpy(usr_id,strmid(tempstr,(char*)inBuf,10,20));
        strcpy(req_time,head->time);
//	strncpy(nearair_sn,(const char *)buf+30,10);
  }
  
  switch(CMD_CMD)
  {
    case CMD_LOGIN:
      {
        nearair_s2c_login_body *body = NULL;
        body = ( nearair_s2c_login_body *)malloc(sizeof(nearair_s2c_login_body) );
        require_action( body, exit, err = kNoMemoryErr );
        memset(body, 0x0, sizeof(nearair_s2c_login_body));
        
        strcpy(body->sn,strmid(tempstr,(char *)inBuf,10,20));
        strcpy(body->login_time,strmid(tempstr,(char *)inBuf,10,30));
        strcpy(body->errno,strmid(tempstr,(char *)inBuf,4,40));
        
        nearair_log("sn:%s,login_time:%s,errno:%s",body->sn,body->login_time,body->errno);
        if(!strcmp(body->errno,"0000"))
        {
          nearair_log("near air login success...");
          wifi_send_nearair_sn(airContent);
          airContent->deviceStatus.isRemoteLogined = true;
          mico_rtos_init_semaphore(&airContent->deviceStatus.dev_status_change_sem, 1);
          if(kNoErr != mico_rtos_create_thread(NULL, MICO_APPLICATION_PRIORITY, \
            "Check Device",check_device_status_change_thread,0x300,(void *)airContent))
            nearair_log("create_check_device_thread error!");
          mico_init_timer(&_heartbeat_timer,5000,heart_beat_handle,(void*)inContext);
          mico_start_timer(&_heartbeat_timer);        
//         if(kNoErr != mico_rtos_create_thread(heartbeat, MICO_APPLICATION_PRIORITY, "Heart Beat", heart_beat_thread, 0x100, (void*)inContext) )
//              nearair_log("create_heart_beat_thread error!");
//         else
//           nearair_log("heart_beat_thread created success!!!!!");
        }
        else
        {
          inContext->appStatus.isRemoteConnected = false;
          airContent->deviceStatus.isRemoteLogined = false;
          nearair_log("near air login faild!!!");
        }               
      }
      break;
    case CMD_HEAT_BEAT:
      heart_beat_rec = true;
      mico_rtos_set_semaphore(&heartbeat_sem);
      break;
    case CMD_SENOR_DATA:
      send_sensor_data(airContent);
      break;
    case CMD_RUN_STATUS:
      send_nearair_status(airContent);
      break;
    case CMD_START:
      send_nearair_start(airContent);
      break;
    case CMD_SHUTDOWN:
      send_nearair_shutdown(airContent);
      break;
    case CMD_TO_SLEEP:
      send_nearair_tosleep(airContent);
      break;
    case CMD_TO_SMART:
      send_nearair_tosmart(airContent);
      break;
    case CMD_CHMOTOR_0:            
    case CMD_CHMOTOR_1:
    case CMD_CHMOTOR_2:
    case CMD_CHMOTOR_3:
    case CMD_CHMOTOR_4:
    case CMD_CHMOTOR_5:
      wifi_change_motor_gear(CMD_CMD,airContent);
      break;
    case CMD_CHILD_LOCK:
    case CMD_CHILD_UNLOCK:                
      wifi_childlock_lock(CMD_CMD,airContent);
      break;
    case CMD_READ_HW:
      wifi_hardware_info(airContent);
      break;
    case CMD_START_OTA:
      start_ota_process(airContent);
      break;
    case CMD_OTA:
      _ota_process((uint8_t *)inBuf,*inBufLen,inContext);
      break;
    case CMD_STOP_OTA:
      stop_ota_process((uint8_t *)inBuf,inContext);
      break;
    default:
      break;   
  }
  wifi_state_synchronization(airContent);
//  *inBufLen = 0;
exit:
  return err;
}

OSStatus nearairc2sCommandProcess(uint8_t *inBuf, int inLen)
{
  mico_rtos_lock_mutex(&wifi_out_mutex);
  nearair_log_trace();
  OSStatus err = kNoErr;
  mico_Context_t* inContext;
  
  struct sockaddr_t addr;

  inContext = read_mico_Context();
  if(inContext->appStatus.isRemoteConnected==true){
    addr.s_ip = IPADDR_LOOPBACK;
    addr.s_port = REMOTE_TCP_CLIENT_LOOPBACK_PORT;
    sendto(_recved_nearair_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
  }        
  mico_rtos_unlock_mutex(&wifi_out_mutex);
  return err;
}

OSStatus nearair_status_synchronization_Process(uint8_t *inBuf, int inLen)
{
  nearair_log_trace();
  OSStatus err = kNoErr;
  mico_Context_t* inContext;
  
  struct sockaddr_t addr;

  inContext = read_mico_Context();
  if(inContext->appStatus.isRemoteConnected==true){
    addr.s_ip = IPADDR_LOOPBACK;
    addr.s_port = REMOTE_TCP_CLIENT_LOOPBACK_PORT;
    sendto(_nearair_status_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
  }        
  return err;
}

OSStatus nearair_login(mico_Context_t * const inContext)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;

    md5_context *md5 = NULL;
    nearair_cmd_head_t *head = NULL;
    nearair_cmd_login_body_t *body = NULL;

    char hextostr[33];
    char *total_value = NULL;
    char *md5input = NULL;
    unsigned char md5output[16];
    char *secretkey = SECRETKEY;//"qwertyuiop";//

    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_login_body_t *)malloc(sizeof(nearair_cmd_login_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_login_body_t));
    
    total_value = (char *)malloc(63*sizeof(char));
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, sizeof(63*sizeof(char)));
    
    strcpy(head->len,"0042");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,"1001");
    strcpy(head->ver,PROTOCOL_VERSION);
		
//    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
				
    strcpy(body->sn,nearair_sn);		
				
    strcat(total_value,body->sn);
    
    md5input = malloc(30);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(md5input, 0x0, 30);    
    
    strcat(md5input,body->sn);
    strcat(md5input,secretkey);
    strcat(md5input,head->time);
				
    md5 = ( md5_context *)malloc(sizeof(md5_context) );
    require_action( md5, exit, err = kNoMemoryErr );
    memset(md5, 0x0, sizeof(md5_context));
    
    InitMd5(md5);
    Md5Update(md5,(unsigned char *)md5input,strlen(md5input));
    Md5Final(md5,md5output);	

    hex2str(hextostr,(const unsigned char*)md5output,16);
    hextostr[32] = '\0';
    strcat(total_value,hextostr);	
   
    nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));		
#ifdef PROTOCOL_DEBUG                		
    nearair_log("have_sent_login_data:%s,strlen is:%d",total_value,strlen(total_value));
#endif		
exit:
    return err;
}

static void heart_beat_handle(void *inContext)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *Context = inContext;
  
  nearair_cmd_head_t *head = NULL;
  char *total_value = NULL;
  char cmd[5];

  head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
  require_action( head, exit, err = kNoMemoryErr );
  memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
  total_value = malloc(21);
  require_action( total_value, exit, err = kNoMemoryErr );
  memset(total_value, 0x0, 21);
  
  sprintf(cmd,"%4d",CMD_HEAT_BEAT);

    if(false == heart_beat_rec)
    {
      nearair_log("heart beat display cloud connect faild!reconnect remoteTcpSever");      
      heart_beat_rec = true;      
      mico_stop_timer(&_heartbeat_timer);
      if(Context->appStatus.isRemoteConnected == true)
        nearair_login(Context);
      goto exit;
    }
    heart_beat_rec = false;
    
    strcpy(head->len,"0000");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);
  
    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);

    nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));

    nearair_log("have_sent_heat_beat:%s,length is:%d!",total_value,strlen(total_value));
exit:
  free(head);
  free(total_value);
  (void)err;
  return;
}
/*
static void heart_beat_thread(void *inContext)
{
  OSStatus err = kUnknownErr;
  mico_Context_t *Context = inContext;
  
  nearair_cmd_head_t *head = NULL;
  char *total_value = NULL;
  char cmd[5];

  head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
  require_action( head, exit, err = kNoMemoryErr );
  memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
  total_value = malloc(21);
  require_action( total_value, exit, err = kNoMemoryErr );
  memset(total_value, 0x0, 21);
  
  sprintf(cmd,"%4d",CMD_HEAT_BEAT);

   while(1)
   {
      err = mico_rtos_get_semaphore(&heartbeat_sem,5000);
      if(false == heart_beat_rec)
      {
          nearair_log("heart beat display cloud connect faild!reconnect remoteTcpSever");  
          heart_beat_rec = true;
//          nearair_login(Context);
          goto exit;
      }
      heart_beat_rec = false;
      
      strcpy(head->len,"0000");
      strcpy(head->time,getstringtime(time_count));
      strcpy(head->cmd,cmd);
      strcpy(head->ver,PROTOCOL_VERSION);
  
      memset(total_value,0,sizeof(total_value));
      strcat(total_value,head->len);
      strcat(total_value,head->time);
      strcat(total_value,head->cmd);
      strcat(total_value,head->ver);

      nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
      
      mico_thread_sleep(10);
#ifdef PROTOCOL_DEBUG
      nearair_log("have_sent_heat_beat:%s,length is:%d!",total_value,strlen(total_value));
#endif
   }
exit:
  free(head);
  free(total_value);
  (void)err;
  nearair_log("heart_beat_thread exit!!!!!");
  nearair_login(Context);
  mico_rtos_delete_thread(NULL);
  return;
}*/

OSStatus wifi_send_nearair_sn(nearair_Context_t * const airContent)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    char *body_data = NULL;	
    char *total_value = NULL;
    char cmd[5];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
    body_data = malloc(21);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 21);
    
    total_value = malloc(41);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 41);
    
    sprintf(cmd,"%4d",CMD_SEND_SN);
    strcpy(head->len,"0020");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
	
    if(10 == strlen(airContent->flashNearAirInRam.extra_data))
       strcpy(usr_id,airContent->flashNearAirInRam.extra_data);
//    nearair_log("usr_id is:%s,extradata is:%s",usr_id,airContent->flashNearAirInRam.extra_data);
    sprintf(body_data,"%s%s",usr_id,nearair_sn);
    strcat(total_value,body_data);
		
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_sn:%s,length is:%d!",total_value,strlen(total_value));        
#endif
exit:
  if(head) free(head);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus send_sensor_data(nearair_Context_t * const airContent)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    nearair_cmd_sensor_body_t *body;
    char *body_data;
    char *total_value;
    char cmd[5];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_sensor_body_t *)malloc(sizeof(nearair_cmd_sensor_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_sensor_body_t));
  
    body_data = malloc(43);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 43);
    
    total_value = malloc(63);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 63);
    
    sprintf(cmd,"%4d",CMD_SENOR_DATA);
    strcpy(head->len,"0042");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
		
    body->req_user = usr_id;
    body->req_time = req_time;
    body->sn = (char *)nearair_sn;//airContent->flashNearAirInRam.sn;
    body->odour = airContent->sensorStatus.odour;
    body->dust = airContent->sensorStatus.dust;
    body->temperature = airContent->sensorStatus.temperature;
    body->humidity = airContent->sensorStatus.humidity;
    sprintf(body_data,"%s%s%s%04d%04d%02d%02d",body->req_user,body->req_time,body->sn
            ,body->odour,body->dust,body->temperature,body->humidity);
    strcat(total_value,body_data);
    
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_sensor_data:%s,length is:%d!",total_value,strlen(total_value));
#endif
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus send_nearair_status(nearair_Context_t * const airContent)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_status_body_t body;
    
    char body_data[33];	
    char total_value[54];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_RUN_STATUS);
    strncpy(head.len,"0033",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    body.mode = airContent->deviceStatus.mode;
    body.position = airContent->deviceStatus.position;
    body.child_lock = airContent->deviceStatus.child_lock;
    sprintf(body_data,"%s%s%s%1d%1d%1d",body.req_user,body.req_time,body.sn,body.mode,body.position,body.child_lock);
    strcat(total_value,body_data);
		
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_status:%s,length is:%d!",total_value,strlen(total_value));
#endif	
    return err;	
}

OSStatus send_nearair_start(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_START);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    airContent->powerStatus.power = POWER_ON;
    
    if(POWER_ON == airContent->powerStatus.power)
    {
//      airContent->deviceStatus.mode = MODE_INIT;
      airContent->deviceStatus.mode = MODE_SMART;
//      airContent->deviceStatus.position = MOTOR_FLY;
      body.ret = SUCCESS;
    }
    else
      body.ret = ERROR;
    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_start:%s,length is:%d!",total_value,strlen(total_value));
#endif		
    return err;	
}

OSStatus send_nearair_shutdown(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_SHUTDOWN);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    airContent->powerStatus.power = POWER_OFF;
    if(POWER_OFF == airContent->powerStatus.power)
    {
      airContent->deviceStatus.mode = MODE_CLOSE;
      airContent->deviceStatus.position = MOTOR_HALT;
      body.ret = SUCCESS;
    }
    else
      body.ret = ERROR;

    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_shutdown:%s,length is:%d!",total_value,strlen(total_value));
#endif		
    return err;	
}

OSStatus send_nearair_tosleep(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_TO_SLEEP);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    airContent->deviceStatus.mode = MODE_SLEEP;
    if(MODE_SLEEP == airContent->deviceStatus.mode)
    {
      airContent->deviceStatus.position = MOTOR_SLEEP;
      body.ret = SUCCESS;
    }
    else
      body.ret = ERROR;
    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_tosleep:%s,length is:%d!",total_value,strlen(total_value));
#endif		
    return err;	
}

OSStatus send_nearair_tosmart(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_TO_SMART);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    airContent->deviceStatus.mode = MODE_SMART;
    if(MODE_SMART == airContent->deviceStatus.mode)
    {
      airContent->deviceStatus.position = MOTOR_MIDIUM;
      body.ret = SUCCESS;
    }
    else
      body.ret = ERROR;
    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_tosmart:%s,length is:%d!",total_value,strlen(total_value));
#endif		
    wifi_state_synchronization(airContent);
    return err;	
}

OSStatus send_nearair_log(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_log_body_t body;
    char body_data[26];
    char total_value[46];
    char cmd[5];
    
    sprintf(cmd,"%4d",CMD_LOG_DATA);
    strncpy(head.len,"0025",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.sn = (char *)nearair_sn;
    body.odour = airContent->sensorStatus.odour;
    body.dust = airContent->sensorStatus.dust;
    body.temperature = airContent->sensorStatus.temperature;
    body.humidity = airContent->sensorStatus.humidity;
    body.mode = airContent->deviceStatus.mode;
    body.position = airContent->deviceStatus.position;
    body.child_lock = airContent->deviceStatus.child_lock;
    sprintf(body_data,"%s%04d%04d%02d%02d%1d%1d%1d",body.sn,body.odour,body.dust,
            body.temperature,body.humidity,body.mode,body.position,body.child_lock);
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_log:%s,length is:%d!",total_value,strlen(total_value));
#endif
    return err;	
}

OSStatus wifi_change_motor_gear(uint16_t cmd_motor,nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",cmd_motor);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    airContent->deviceStatus.mode = MODE_MANUAL;
    switch(cmd_motor)
    {
    case CMD_CHMOTOR_0:
      {
        airContent->deviceStatus.position = MOTOR_HALT;
        if(MOTOR_HALT == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    case CMD_CHMOTOR_1:
      {
        airContent->deviceStatus.position = MOTOR_SLEEP;
        if(MOTOR_SLEEP == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    case CMD_CHMOTOR_2:
      {
        airContent->deviceStatus.position = MOTOR_LOW;
        if(MOTOR_LOW == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    case CMD_CHMOTOR_3:
      {
        airContent->deviceStatus.position = MOTOR_MIDIUM;
        if(MOTOR_MIDIUM == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    case CMD_CHMOTOR_4:
      {
        airContent->deviceStatus.position = MOTOR_HIGH;
        if(MOTOR_HIGH == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    case CMD_CHMOTOR_5:
      {
        airContent->deviceStatus.mode = MODE_FLY;
        airContent->deviceStatus.position = MOTOR_FLY;
        if(MOTOR_FLY == airContent->deviceStatus.position)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;
      }
      break;
    default:
      body.ret = ERROR;
      break;
    }
//    if(SUCCESS == body.ret)
//      airContent->deviceStatus.mode = MODE_MANUAL;
    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_motor_gear[%d]:%s,length is:%d!",airContent->deviceStatus.position,
                total_value,strlen(total_value));
#endif		
    return err;	
}

OSStatus wifi_childlock_lock(uint16_t cmd_childlock,nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t head;
    nearair_cmd_return_body_t body;
    char body_data[32];
    char total_value[52];
    char cmd[5];
    
    sprintf(cmd,"%4d",cmd_childlock);
    strncpy(head.len,"0031",4);
    head.len[4] = '\0';
    strncpy(head.time,getstringtime(time_count),10);
    head.time[10] = '\0';
    strncpy(head.cmd,cmd,4);
    head.cmd[4] = '\0';
    strncpy(head.ver,PROTOCOL_VERSION,2);
    head.ver[2] = '\0';

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head.len);
    strcat(total_value,head.time);
    strcat(total_value,head.cmd);
    strcat(total_value,head.ver);
	
    body.req_user = usr_id;
    body.req_time = req_time;
    body.sn = (char *)nearair_sn;
    switch( cmd_childlock )
    {
    case CMD_CHILD_LOCK:
      {
        airContent->deviceStatus.child_lock = CHILDLOCK_LOCK;
        if(CHILDLOCK_LOCK == airContent->deviceStatus.child_lock)
          body.ret = SUCCESS;
        else
          body.ret = ERROR;       
      }
      break;
    case CMD_CHILD_UNLOCK:
      {
        airContent->deviceStatus.child_lock = CHILDLOCK_UNLOCK;
        if(CHILDLOCK_UNLOCK == airContent->deviceStatus.child_lock)
          body.ret = SUCCESS;
        else
          body.ret = ERROR; 
      }
      break;
    default:
      body.ret = ERROR;
      break;
    }
    
    sprintf(body_data,"%s%s%s%1d",body.req_user,body.req_time,body.sn,body.ret);
    
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_childlock[%d]:%s,length is:%d!",airContent->deviceStatus.child_lock,
                total_value,strlen(total_value));
#endif		
    return err;	
}

OSStatus wifi_state_synchronization(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    nearair_cmd_state_synchronization_body_t *body;
    char *body_data;
    char *total_value;
    char cmd[4+1];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_state_synchronization_body_t *)malloc(\
                             sizeof(nearair_cmd_state_synchronization_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_state_synchronization_body_t));
  
    body_data = malloc(14);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 14);
    
    total_value = malloc(34);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 34);
    
    sprintf(cmd,"%4d",CMD_STATE_SYNCHRONIZATION);
    strcpy(head->len,"0013");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
	
    body->sn = (char *)nearair_sn;
    body->mode = airContent->deviceStatus.mode;
    if(MODE_FLY == body->mode)
      body->mode = MODE_MANUAL;
    body->position = airContent->deviceStatus.position;
    body->child_lock = airContent->deviceStatus.child_lock;
    
    sprintf(body_data,"%s%1d%1d%1d",body->sn,body->mode,body->position,body->child_lock);
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_state_synchronization:%s,length is:%d!",total_value,strlen(total_value));
#endif	
    wifi_sensor_synchronization(airContent);
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus wifi_sensor_synchronization(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    nearair_cmd_sensor_synchronization_body_t *body;
    char *body_data;
    char *total_value;
    char cmd[4+1];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_sensor_synchronization_body_t *)malloc(\
                             sizeof(nearair_cmd_sensor_synchronization_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_sensor_synchronization_body_t));
  
    body_data = malloc(23);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 23);
    
    total_value = malloc(43);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 43);
    
    sprintf(cmd,"%4d",CMD_SENSOR_SYNCHRONIZATION);
    strcpy(head->len,"0022");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
	
    body->sn = (char *)nearair_sn;
    body->odour = airContent->sensorStatus.odour;
    body->dust = airContent->sensorStatus.dust;
    body->temperature = airContent->sensorStatus.temperature;
    body->humidity = airContent->sensorStatus.humidity;
    
    sprintf(body_data,"%s%04d%04d%02d%02d",body->sn,body->odour,body->dust,
            body->temperature,body->humidity);
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_sensor_synchronization:%s,length is:%d!",total_value,strlen(total_value));
#endif		
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus wifi_hardware_info(nearair_Context_t * const airContent)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    nearair_cmd_hwinfo_body_t *body;
    char *body_data = NULL;	
    char *total_value = NULL;
    char cmd[5];
    
    body = ( nearair_cmd_hwinfo_body_t *)malloc(sizeof(nearair_cmd_hwinfo_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_hwinfo_body_t));
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
    body_data = malloc(35);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 35);
    
    total_value = malloc(55);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 55);
    
    sprintf(cmd,"%4d",CMD_READ_HW);
    strcpy(head->len,"0035");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
    
    body->req_user = usr_id;
    body->req_time = req_time;
    body->sn = (char *)nearair_sn;
    body->hd_version = (char *)airContent->flashNearAirInRam.hardware_ver;
    sprintf(body_data,"%s%s%s%s",body->req_user,body->req_time,body->sn,body->hd_version);
    strcat(total_value,body_data);
		
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
    nearair_log("have_sent_nearair_hardware_ver:%s,length is:%d!",total_value,strlen(total_value));        
    
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus status_changed_synchronization(nearair_Context_t * const airContent)
{ 
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    nearair_cmd_head_t *head;
    nearair_cmd_state_synchronization_body_t *body;
    char *body_data;
    char *total_value;
    char cmd[4+1];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_state_synchronization_body_t *)malloc(\
                             sizeof(nearair_cmd_state_synchronization_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_state_synchronization_body_t));
  
    body_data = malloc(14);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 14);
    
    total_value = malloc(34);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 34);
    
    sprintf(cmd,"%4d",CMD_STATE_SYNCHRONIZATION);
    strcpy(head->len,"0013");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
	
    body->sn = (char *)nearair_sn;
    body->mode = airContent->deviceStatus.mode;
    if(MODE_FLY == body->mode)
      body->mode = MODE_MANUAL;
    body->position = airContent->deviceStatus.position;
    body->child_lock = airContent->deviceStatus.child_lock;
    
    sprintf(body_data,"%s%1d%1d%1d",body->sn,body->mode,body->position,body->child_lock);
    strcat(total_value,body_data);
	
    err = nearair_status_synchronization_Process((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_state_synchronization:%s,length is:%d!",total_value,strlen(total_value));
#endif	
//    wifi_sensor_synchronization(airContent);
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

static void check_device_status_change_thread(void *inContent)
{
    nearair_log_trace();
//    OSStatus err = kUnknownErr;
    nearair_Context_t *airContent = inContent;
/*    
    nearair_cmd_head_t *head;
    nearair_cmd_state_synchronization_body_t *body;
    char *body_data;
    char *total_value;
    char cmd[4+1];
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
    
    body = ( nearair_cmd_state_synchronization_body_t *)malloc(\
                             sizeof(nearair_cmd_state_synchronization_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_state_synchronization_body_t));
  
    body_data = malloc(14);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 14);
    
    total_value = malloc(34);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 34);*/
    
    while(1)
    {
        mico_rtos_get_semaphore(&airContent->deviceStatus.dev_status_change_sem,MICO_NEVER_TIMEOUT);
        nearair_log("this is test: dev_status_change_sem changed!");
        status_changed_synchronization(airContent);
/*        sprintf(cmd,"%4d",CMD_STATE_SYNCHRONIZATION);
    strcpy(head->len,"0013");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
	
    body->sn = (char *)nearair_sn;
    body->mode = airContent->deviceStatus.mode;
    if(MODE_FLY == body->mode)
      body->mode = MODE_MANUAL;
    body->position = airContent->deviceStatus.position;
    body->child_lock = airContent->deviceStatus.child_lock;
    
    sprintf(body_data,"%s%1d%1d%1d",body->sn,body->mode,body->position,body->child_lock);
    strcat(total_value,body_data);
	
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_state_synchronization:%s,length is:%d!",total_value,strlen(total_value));
#endif	*/
    }
/*    if( airContent->deviceStatus.mode_changed_flag )
    {      
      wifi_state_synchronization(airContent);
      airContent->deviceStatus.mode_changed_flag = 0;
    }
    return;*/
/*    
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
  mico_rtos_delete_thread(NULL);
    return;	*/
}

int hex2str(char *sDest,const unsigned char *sSrc,int nSrclen)
{
    int i;
    char szTmp[3];
    for(i=0;i<nSrclen;i++)
    {
            sprintf(szTmp,"%02x",sSrc[i]);
            memcpy(&sDest[i*2],szTmp,2);
    }
    return 0;
}

uint32_t flash_addr;
OSStatus start_ota_process(nearair_Context_t * const airContent)
{
    nearair_log_trace();
    OSStatus err = kUnknownErr;
    
    flash_addr = UPDATE_START_ADDRESS;
    nearair_cmd_head_t *head;
    nearair_cmd_return_body_t *body;
    char *body_data = NULL;	
    char *total_value = NULL;
    char cmd[5];
    
    body = ( nearair_cmd_return_body_t *)malloc(sizeof(nearair_cmd_return_body_t) );
    require_action( body, exit, err = kNoMemoryErr );
    memset(body, 0x0, sizeof(nearair_cmd_return_body_t));
    
    head = ( nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t) );
    require_action( head, exit, err = kNoMemoryErr );
    memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
    body_data = malloc(32);
    require_action( body_data, exit, err = kNoMemoryErr );
    memset(body_data, 0x0, 32);
    
    total_value = malloc(52);
    require_action( total_value, exit, err = kNoMemoryErr );
    memset(total_value, 0x0, 52);
    
    sprintf(cmd,"%4d",CMD_START_OTA);
    strcpy(head->len,"0031");
    strcpy(head->time,getstringtime(time_count));
    strcpy(head->cmd,cmd);
    strcpy(head->ver,PROTOCOL_VERSION);

    memset(total_value,0,sizeof(total_value));
    strcat(total_value,head->len);
    strcat(total_value,head->time);
    strcat(total_value,head->cmd);
    strcat(total_value,head->ver);
    
    body->req_user = usr_id;
    body->req_time = req_time;
    body->sn = (char *)nearair_sn;
    
    if(kNoErr == PlatformFlashInitialize())
      body->ret = SUCCESS;
    else
      body->ret = ERROR;
    
    sprintf(body_data,"%s%s%s%1d",body->req_user,body->req_time,body->sn,body->ret);
    strcat(total_value,body_data);
		
    err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
    nearair_log("have_sent_nearair_start_ota:%s,length is:%d!",total_value,strlen(total_value));        
#endif
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}

OSStatus _ota_process(uint8_t *inBuf, int inBufLen, mico_Context_t * const inContext)
{
  OSStatus err = kNoErr;
  uint8_t * p_bin;
  int bin_len;
  
  nearair_cmd_head_t *head;
  nearair_cmd_ota_return_body_t *body;
  char *body_data = NULL;	
  char *total_value = NULL;
  char cmd[5];
    
  head = (nearair_cmd_head_t *)malloc(sizeof(nearair_cmd_head_t));
  require_action( head, exit, err = kNoMemoryErr );
  memset(head, 0x0, sizeof(nearair_cmd_head_t));
  
  body = (nearair_cmd_ota_return_body_t *)malloc(sizeof(nearair_cmd_ota_return_body_t));
  require_action( body, exit, err = kNoMemoryErr );
  memset(body, 0x0, sizeof(nearair_cmd_ota_return_body_t));
  
  body_data = malloc(12);
  require_action( body_data, exit, err = kNoMemoryErr );
  memset(body_data, 0x0, 12);
    
  total_value = malloc(32);
  require_action( total_value, exit, err = kNoMemoryErr );
  memset(total_value, 0x0, 32);
  
  
  p_bin = (uint8_t *)inBuf + 30;
  bin_len = inBufLen - 30;  
  
  err = PlatformFlashWrite(&flash_addr, (uint32_t *)p_bin, bin_len);
  if(kNoErr == err )
    body->ret = SUCCESS;
  else
    body->ret = ERROR;
  
  sprintf(cmd,"%4d",CMD_OTA);
  strcpy(head->len,"0011");
  strcpy(head->time,getstringtime(time_count));
  strcpy(head->cmd,cmd);
  strcpy(head->ver,PROTOCOL_VERSION);
  
  strcat(total_value,head->len);
  strcat(total_value,head->time);
  strcat(total_value,head->cmd);
  strcat(total_value,head->ver);
    
  body->sn = (char *)nearair_sn;
  
  sprintf(body_data,"%s%1d",body->sn,body->ret);
  
  strcat(total_value,body_data);
    
  err = nearairc2sCommandProcess((unsigned char *)total_value,strlen(total_value));
#ifdef PROTOCOL_DEBUG
  nearair_log("have_sent_ota_return:%s,length is:%d!",total_value,strlen(total_value));
#endif
  return err;
exit:
  if(head) free(head);
  if(body) free(body);
  if(total_value) free(total_value);
  if(body_data) free(body_data);
    return err;	
}
/*
OSStatus _ota_process(uint8_t *inBuf, int inBufLen, int *inSocketFd, mico_Context_t * const inContext)
{
  OSStatus err = kNoErr;
  mxchip_cmd_head_t *p_control_cmd;
  ota_upgrate_t *p_upgrade;
  uint8_t * p_bin;
  int bin_len, total_len, head_len;
  uint32_t flash_addr = UPDATE_START_ADDRESS;
  uint8_t md5_ret[16];
//  mxchip_cmd_head_t cmd_ack;
  fd_set readfds;
  struct timeval_t t;
  md5_context ctx;

//  memset(&cmd_ack, 0, sizeof(cmd_ack));
//  cmd_ack.cmd_status = CMD_FAIL;
  p_control_cmd = (mxchip_cmd_head_t *)inBuf;
//  cmd_ack.flag = p_control_cmd->flag;
//  cmd_ack.cmd = p_control_cmd->cmd | 0x8000;
//  head_len = sizeof(mxchip_cmd_head_t) + sizeof(ota_upgrate_t) - 2;
  if (inBufLen < head_len){
    goto CMD_REPLY;
  }
  PlatformFlashInitialize();
  p_upgrade = (ota_upgrate_t*)(p_control_cmd->data);

//  p_bin = p_upgrade->data;
//  total_len = p_upgrade->len;
//  bin_len = inBufLen - head_len;
  total_len -= bin_len;

  if (bin_len>0)
    PlatformFlashWrite(&flash_addr, (uint32_t *)p_bin, bin_len);

  while (total_len>0) {
    FD_ZERO(&readfds);
    t.tv_sec = 10;
    t.tv_usec = 0;
    FD_SET(*inSocketFd, &readfds);
    select(1, &readfds, NULL, NULL, &t);

    if (FD_ISSET(*inSocketFd, &readfds)) {
      bin_len = recv(*inSocketFd, (char*)p_bin, 1024, 0);
      require_action(bin_len >= 0, exit, err = kConnectionErr);
      PlatformFlashWrite(&flash_addr, (uint32_t *)p_bin, bin_len);
      total_len-=bin_len;
    }
  }


  InitMd5( &ctx );
  Md5Update( &ctx, (u8 *)UPDATE_START_ADDRESS, flash_addr - UPDATE_START_ADDRESS);
  Md5Final( &ctx, md5_ret );

  if(memcmp(md5_ret, p_upgrade->md5, 16) != 0) {
    PlatformFlashFinalize();
    goto CMD_REPLY;
  }

  memset(&inContext->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
  inContext->flashContentInRam.bootTable.length = flash_addr - UPDATE_START_ADDRESS;
  inContext->flashContentInRam.bootTable.start_address = UPDATE_START_ADDRESS;
  inContext->flashContentInRam.bootTable.type = 'A';
  inContext->flashContentInRam.bootTable.upgrade_type = 'U';
  MICOUpdateConfiguration(inContext);
//  cmd_ack.cmd_status = CMD_OK;
  
CMD_REPLY:
//  err =  SocketSend( *inSocketFd, (u8 *)&cmd_ack, sizeof(cmd_ack) + 1 + cmd_ack.datalen );
  require_noerr(err, exit);
  return kNoErr;

exit:
  SocketClose(inSocketFd);
  inContext->micoStatus.sys_state = eState_Software_Reset;
  mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
  return err;
}*/

OSStatus stop_ota_process(uint8_t *inBuf, mico_Context_t * const inContext)
{
  nearair_log_trace();
  OSStatus err = kNoErr;
  
  uint8_t md5_ret[16];
  char hextostr[33];
  md5_context ctx;
  char md5str[33]; 
  
  InitMd5( &ctx );
  Md5Update( &ctx, (u8 *)UPDATE_START_ADDRESS, flash_addr - UPDATE_START_ADDRESS);
  Md5Final( &ctx, md5_ret );
  hex2str(hextostr,(const unsigned char*)md5_ret,16);
    hextostr[32] = '\0';
    
  strmid( md5str, (char*)inBuf,30,32);
  if(strcmp( hextostr, md5str ) != 0) {
     PlatformFlashFinalize();
     goto exit;
  }
    
  memset(&inContext->flashContentInRam.bootTable, 0, sizeof(boot_table_t));
  inContext->flashContentInRam.bootTable.length = flash_addr - UPDATE_START_ADDRESS;
  inContext->flashContentInRam.bootTable.start_address = UPDATE_START_ADDRESS;
  inContext->flashContentInRam.bootTable.type = 'A';
  inContext->flashContentInRam.bootTable.upgrade_type = 'U';
  MICOUpdateConfiguration(inContext);
  
exit:
  inContext->micoStatus.sys_state = eState_Software_Reset;
  mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
  return err;
}
