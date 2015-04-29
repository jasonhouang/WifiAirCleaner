
#include "MICODefine.h"
#include "MICOAppDefine.h"
#include "MICONotificationCenter.h"
#include "HaProtocol.h"
#include "PlatformUart.h"
#include "SocketUtils.h"
#include "Platform.h"
#include "PlatformFlash.h"

#include <stdio.h>

#define ha_log(M, ...) custom_log("HA Command", M, ##__VA_ARGS__)
#define ha_log_trace() custom_log_trace("HA Command")

//static u32 running_state = 0;
static u32 network_state = 0;
static mico_mutex_t _mutex;

static int _recved_uart_loopback_fd = -1;

static uint16_t _calc_sum(void *data, uint32_t len);
static OSStatus _ota_process(uint8_t *inBuf, int inBufLen, int *inSocketFd, mico_Context_t * const inContext);
static mico_thread_t    _report_status_thread_handler = NULL;
static mico_semaphore_t _report_status_sem = NULL;
static void _report_status_thread(void *inContext);

void haNotify_WifiStatusHandler(int event, mico_Context_t * const inContext)
{
  ha_log_trace();
  (void)inContext;
  switch (event) {
  case NOTIFY_STATION_UP:
    set_network_state(STA_CONNECT, 1);
    break;
  case NOTIFY_STATION_DOWN:
    set_network_state(STA_CONNECT, 0);
    break;
  default:
    break;
  }
  return;
}


int is_network_state(int state)
{
  if ((network_state & state) == 0)
    return 0;
  else
    return 1;
}


/* Get system status */
static void _get_status(mxchip_state_t *cmd, mico_Context_t * const inContext)
{
  ha_log_trace();

  u16 cksum;
  LinkStatusTypeDef ap_state;

  cmd->flag = 0x00BB;
  cmd->cmd = 0x8008;
  cmd->cmd_status = 0;
  cmd->datalen = sizeof(mxchip_state_t) - 10;

  cmd->status.uap_state = is_network_state(UAP_START);
  cmd->status.sta_state = is_network_state(STA_CONNECT);
  if (is_network_state(STA_CONNECT) == 1)
    cmd->status.tcp_client = is_network_state(REMOTE_CONNECT) + 1;
  else
    cmd->status.tcp_client = 0;

  CheckNetLink(&ap_state);
  cmd->status.signal = ap_state.wifi_strength;
  strncpy(cmd->status.ip, inContext->micoStatus.localIp, maxIpLen);
  strncpy(cmd->status.mask, inContext->micoStatus.netMask, maxIpLen);
  strncpy(cmd->status.gw, inContext->micoStatus.gateWay, maxIpLen);
  strncpy(cmd->status.dns, inContext->micoStatus.dnsServer, maxIpLen);
  strncpy(cmd->status.mac, inContext->micoStatus.mac, 18);

  cksum = _calc_sum(cmd, sizeof(mxchip_state_t) - 2);
  cmd->cksum = cksum;
}



void set_network_state(int state, int on)
{
  ha_log_trace();
  mico_rtos_lock_mutex(&_mutex);
  if (on)
    network_state |= state;
  else {
    network_state &= ~state;
    if (state == STA_CONNECT)
      network_state &= ~REMOTE_CONNECT;
  }
  
  if ((state == STA_CONNECT) || (state == REMOTE_CONNECT)){
    mico_rtos_set_semaphore(&_report_status_sem);
  }  
  mico_rtos_unlock_mutex(&_mutex);
}


OSStatus haProtocolInit(mico_Context_t * const inContext)
{
  ha_log_trace();
  OSStatus err = kUnknownErr;
  struct sockaddr_t addr;


  mico_rtos_init_mutex(&_mutex);
  mico_rtos_init_semaphore(&_report_status_sem, 1);

  _recved_uart_loopback_fd = socket(AF_INET, SOCK_DGRM, IPPROTO_UDP);
  addr.s_ip = IPADDR_LOOPBACK;
  addr.s_port = RECVED_UART_DATA_LOOPBACK_PORT;
  bind(_recved_uart_loopback_fd, &addr, sizeof(addr));
  
  err = mico_rtos_create_thread(&_report_status_thread_handler, MICO_APPLICATION_PRIORITY, "Report", _report_status_thread, 0x500, (void*)inContext );
  require_noerr_action( err, exit, ha_log("ERROR: Unable to start the status report thread.") );

  /* Regisist notifications */
  err = MICOAddNotification( mico_notify_WIFI_STATUS_CHANGED, (void *)haNotify_WifiStatusHandler );
  require_noerr( err, exit ); 
exit:
  return err;
}

void _report_status_thread(void *inContext)
{
  mxchip_state_t cmd;

  while(1){
    mico_rtos_get_semaphore(&_report_status_sem, MICO_WAIT_FOREVER);
    _get_status(&cmd, inContext);
    PlatformUartSend((uint8_t *)&cmd, sizeof(mxchip_state_t));
  }
}

OSStatus haWlanCommandProcess(unsigned char *inBuf, int *inBufLen, int inSocketFd, mico_Context_t * const inContext)
{
  ha_log_trace();
  OSStatus err = kUnknownErr;
  mxchip_cmd_head_t *p_reply;
  uint16_t cmd;
  uint16_t cmdLen;
  int idx;

  for(idx = 0; idx < *inBufLen; idx += cmdLen){
    if((uint16_t)(*inBufLen - idx) < HA_CMD_HEAD_SIZE) goto needsMoreData;
    require_action(inBuf[idx] == CONTROL_FLAG, exit, err = kFormatErr);
    require_action(inBuf[idx+1] == 0x0, exit, err = kFormatErr);
    cmdLen  = inBuf[idx+6] + (inBuf[idx+7]<<8) + HA_CMD_HEAD_SIZE + 2;
    if(cmdLen > *inBufLen - idx) goto needsMoreData;
    err = check_sum(inBuf+idx+HA_CMD_HEAD_SIZE, cmdLen);
    require_noerr(err, exit);

    p_reply = (mxchip_cmd_head_t *)(inBuf+idx);
    p_reply->cmd_status = CMD_OK;
    cmd = p_reply->cmd;
    p_reply->cmd |= 0x8000;
    switch (cmd) {
      case CMD_READ_VERSION:
      case CMD_READ_CONFIG:
      case CMD_WRITE_CONFIG:
      case CMD_SCAN:
        break;

      case CMD_OTA:
        err = _ota_process(inBuf+idx, cmdLen, &inSocketFd, inContext);
        break;

      case CMD_NET2COM:
        err = PlatformUartSend(inBuf+idx, cmdLen);
        break;

      default:
        break;
    }
  }

needsMoreData:
  memmove(inBuf, inBuf+idx, *inBufLen - idx);
  *inBufLen = *inBufLen - idx;
  return kNoErr;

exit:
  *inBufLen = 0;
  //if(err != kNoErr) ha_log("Exit with err: %d", err);
  return err;
}


OSStatus _ota_process(uint8_t *inBuf, int inBufLen, int *inSocketFd, mico_Context_t * const inContext)
{
  OSStatus err = kNoErr;
  mxchip_cmd_head_t *p_control_cmd;
  ota_upgrate_t *p_upgrade;
  uint8_t * p_bin;
  int bin_len, total_len, head_len;
  uint32_t flash_addr = UPDATE_START_ADDRESS;
  uint8_t md5_ret[16];
  mxchip_cmd_head_t cmd_ack;
  fd_set readfds;
  struct timeval_t t;
  md5_context ctx;

  memset(&cmd_ack, 0, sizeof(cmd_ack));
  cmd_ack.cmd_status = CMD_FAIL;
  p_control_cmd = (mxchip_cmd_head_t *)inBuf;
  cmd_ack.flag = p_control_cmd->flag;
  cmd_ack.cmd = p_control_cmd->cmd | 0x8000;
  head_len = sizeof(mxchip_cmd_head_t) + sizeof(ota_upgrate_t) - 2;
  if (inBufLen < head_len){
    goto CMD_REPLY;
  }
  PlatformFlashInitialize();
  p_upgrade = (ota_upgrate_t*)(p_control_cmd->data);

  p_bin = p_upgrade->data;
  total_len = p_upgrade->len;
  bin_len = inBufLen - head_len;
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
  cmd_ack.cmd_status = CMD_OK;
  
CMD_REPLY:
  err =  SocketSend( *inSocketFd, (u8 *)&cmd_ack, sizeof(cmd_ack) + 1 + cmd_ack.datalen );
  require_noerr(err, exit);
  return kNoErr;

exit:
  SocketClose(inSocketFd);
  inContext->micoStatus.sys_state = eState_Software_Reset;
  mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
  return err;
}

OSStatus haUartCommandProcess(uint8_t *inBuf, int inLen, mico_Context_t * const inContext)
{
  ha_log_trace();
  OSStatus err = kNoErr;
  int i, control;
  mxchip_cmd_head_t *cmd_header;
  uint16_t cksum;
    struct sockaddr_t addr;

  cmd_header = (mxchip_cmd_head_t *)inBuf;

  switch(cmd_header->cmd) {
    case CMD_COM2NET:
        cmd_header->cmd |= 0x8000;

        addr.s_ip = IPADDR_LOOPBACK;


        for(i=0; i < MAX_Local_Client_Num; i++) {
          if( inContext->appStatus.loopBack_PortList[i] != 0 ){
            addr.s_port = inContext->appStatus.loopBack_PortList[i];
            sendto(_recved_uart_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
          }
        }

        if(is_network_state(REMOTE_CONNECT)==1){
          addr.s_ip = IPADDR_LOOPBACK;
          addr.s_port = REMOTE_TCP_CLIENT_LOOPBACK_PORT;
          sendto(_recved_uart_loopback_fd, inBuf, inLen, 0, &addr, sizeof(addr));
        }
        
        break;
        
    case CMD_GET_STATUS:
        _get_status((mxchip_state_t*)inBuf, inContext);
        err = PlatformUartSend(inBuf, sizeof(mxchip_state_t));

        break;
    case CMD_CONTROL:
        if (cmd_header->datalen != 1)
            break;
        control = inBuf[8];
        switch(control) {
        case 1: 
            inContext->micoStatus.sys_state = eState_Software_Reset;
            require(inContext->micoStatus.sys_state_change_sem, exit);
            mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
            break;
        case 2:
            MICORestoreDefault(inContext);
            inContext->micoStatus.sys_state = eState_Software_Reset;
            require(inContext->micoStatus.sys_state_change_sem, exit);
            mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
            break;
        case 3:
            inContext->micoStatus.sys_state = eState_Wlan_Powerdown;
            require(inContext->micoStatus.sys_state_change_sem, exit);
            mico_rtos_set_semaphore(&inContext->micoStatus.sys_state_change_sem);
            break;
        case 5: 
            micoWlanStartEasyLink(120);
            break;
        default:
            break;
        }
        cmd_header->cmd |= 0x8000;
        cmd_header->cmd_status = 1;
        cmd_header->datalen = 0;
        cksum = _calc_sum(inBuf, 8);
        inBuf[8] = cksum & 0x00ff;
        inBuf[9] = (cksum & 0x0ff00) >> 8;
        err = PlatformUartSend(inBuf, 10);
        break;
    default:
        break;
    }
exit:
    return err;
}


OSStatus check_sum(void *inData, uint32_t inLen)  
{
  ha_log_trace();

  uint16_t *sum;
  uint8_t *p = (u8 *)inData;

  return kNoErr; 
  // TODO: real cksum
  p += inLen - 2;

  sum = (u16 *)p;

  if (_calc_sum(inData, inLen - 2) != *sum) {  // check sum error    
    return kChecksumErr;
  }
  return kNoErr;
}

uint16_t _calc_sum(void *inData, uint32_t inLen)
{
  ha_log_trace();
  uint32_t cksum=0;
  uint16_t *p=inData;

  while (inLen > 1)
  {
    cksum += *p++;
    inLen -=2;
  }
  if (inLen)
  {
    cksum += *(u8 *)p;
  }
  cksum = (cksum >> 16) + (cksum & 0xffff);
  cksum += (cksum >>16);

  return ~cksum;
}




