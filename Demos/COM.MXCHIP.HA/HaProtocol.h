#include "Common.h"
#include "MICODefine.h"

#define UART_FRAM_START     0xAA
#define UART_FRAM_END       0x55
#define CONTROL_FLAG        0xBB
#define FRAM_FLAG           0x00BB


enum {
  CMD_OK = 0,
  CMD_FAIL,
};

enum {
  CMD_READ_VERSION = 1,    //Deprecated in MICO, use bonjour to read device related info
  CMD_READ_CONFIG,         //Deprecated in MICO, use Easylink to R/W configuration
  CMD_WRITE_CONFIG,        //Deprecated in MICO, use Easylink to R/W configuration
  CMD_SCAN,                //Deprecated in MICO, that is no use for HA application
  CMD_OTA, 
  CMD_NET2COM,             //Deprecated in MICO, use Easylink to R/W configuration
  CMD_COM2NET, 
  CMD_GET_STATUS, 
  CMD_CONTROL,    
  CMD_SEARCH, 
};

enum {
  STA_CONNECT = 1<<0,
  UAP_START = 1<<2,       //Deprecated in MICO, Nerver setup a Soft AP under MICO system
  REMOTE_CONNECT = 1<<3,
};

typedef struct _mxchip_cmd_head {
  uint16_t flag; // Allways BB 00
  uint16_t cmd; // commands, return cmd=cmd|0x8000
  uint16_t cmd_status; //return result
  uint16_t datalen; 
  uint8_t data[1]; 
}mxchip_cmd_head_t;

#define  HA_CMD_HEAD_SIZE 8

typedef struct _upgrade_t {
  uint8_t md5[16];
  uint32_t len;
  uint8_t data[1];
}ota_upgrate_t;

typedef struct _current_state_ {
  uint32_t uap_state;
  uint32_t sta_state;
  uint32_t tcp_client;
  uint32_t signal;
  char ip[16];
  char mask[16];
  char gw[16];
  char dns[16];
  char mac[18];
}current_state_t;

typedef struct _mxchip_state_ {
  uint16_t flag; 
  uint16_t cmd; 
  uint16_t cmd_status; 
  uint16_t datalen; 
  current_state_t status;
  uint16_t cksum;
}mxchip_state_t;

OSStatus haProtocolInit(mico_Context_t * const inContext);
int is_network_state(int state);
OSStatus haWlanCommandProcess(unsigned char *inBuf, int *inBufLen, int inSocketFd, mico_Context_t * const inContext);
OSStatus haUartCommandProcess(uint8_t *inBuf, int inLen, mico_Context_t * const inContext);
OSStatus check_sum(void *inData, uint32_t inLen);  


void set_network_state(int state, int on);

