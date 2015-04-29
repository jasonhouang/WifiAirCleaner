/**
  ******************************************************************************
  * @file    MICOAlgorithm.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provides all the headers of Algorithms provided by MICO.
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

#ifndef __MICO_ALGORITHM_H_
#define __MICO_ALGORITHM_H_

#include "Common.h"

enum {
    MD5             =  0,      /* hash type unique */
    MD5_BLOCK_SIZE  = 64,
    MD5_DIGEST_SIZE = 16,
    MD5_PAD_SIZE    = 56
};


/* MD5 digest */
typedef struct Md5 {
    uint32_t  buffLen;   /* in bytes          */
    uint32_t  loLen;     /* length in bytes   */
    uint32_t  hiLen;     /* length in bytes   */
    uint32_t  digest[MD5_DIGEST_SIZE / sizeof(uint32_t)];
    uint32_t  buffer[MD5_BLOCK_SIZE  / sizeof(uint32_t)];
} md5_context;

/**
 * @brief          MD5 context setup
 *
 * @param ctx      context to be initialized
 */
void InitMd5(md5_context *ctx);

/**
 * @brief          MD5 process buffer
 *
 * @param ctx      MD5 context
 * @param input    buffer holding the  data
 * @param ilen     length of the input data
 */
void Md5Update(md5_context *ctx, unsigned char *input, int ilen);

/**
 * @brief          MD5 final digest
 *
 * @param ctx      MD5 context
 * @param output   MD5 checksum result
 */
void Md5Final(md5_context *ctx, unsigned char output[16]);



#endif /* __MICO_ALGORITHM_H_ */

