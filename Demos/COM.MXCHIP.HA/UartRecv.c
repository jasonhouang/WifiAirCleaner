
#include "MICODefine.h"
#include "MICOAppDefine.h"

#include "haProtocol.h"
#include "PlatformUart.h"
#include "MICONotificationCenter.h"

#define uart_recv_log(M, ...) custom_log("UART RECV", M, ##__VA_ARGS__)
#define uart_recv_log_trace() custom_log_trace("UART RECV")

static int _uart_get_one_packet(u8* buf, int maxlen);

void uartRecv_thread(void *inContext)
{
  uart_recv_log_trace();
  mico_Context_t *Context = inContext;
  int recvlen;
  uint8_t *inDataBuffer;
  
  inDataBuffer = malloc(UartRecvBufferLen);
  require(inDataBuffer, exit);
  
  while(1) {
    recvlen = _uart_get_one_packet(inDataBuffer, UartRecvBufferLen);
    if (recvlen <= 0)
      continue; 
    haUartCommandProcess(inDataBuffer, recvlen, Context);
  }
  
exit:
  if(inDataBuffer) free(inDataBuffer);
}

/* Packet format: BB 00 CMD(2B) Status(2B) datalen(2B) data(x) checksum(2B)
* copy to buf, return len = datalen+10
*/
int _uart_get_one_packet(uint8_t* inBuf, int inBufLen)
{
  uart_recv_log_trace();
  OSStatus err = kNoErr;
  int datalen;
  uint8_t *p;
  
  while(1) {
    p = inBuf;
    err = PlatformUartRecv(p, 1, MICO_WAIT_FOREVER);
    require_noerr(err, exit);
    require(*p == 0xBB, exit);
    
    p++;
    err = PlatformUartRecv(p, 1, 1000);
    require_noerr(err, exit);
    require(*p == 0x00, exit);
    
    p++;
    err = PlatformUartRecv(p, 6, 1000);
    require_noerr(err, exit);
    datalen = p[4] + (p[5]<<8);
    require(datalen + 10 <= inBufLen, exit);
    
    p += 6;
    err = PlatformUartRecv(p, datalen+2, 1000);
    require_noerr(err, exit);
    
    err = check_sum(inBuf, datalen + 10);
    require_noerr(err, exit);
    
    return datalen + 10;
  }
  
exit:
  return -1;
  
}


