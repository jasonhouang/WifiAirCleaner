/**
  ******************************************************************************
  * @file    MICOPARASTORAGE.c 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide operations on nonvolatile memory.
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
#include "Common.h"

#define MAXPairNumber       50
/*Pair Info flash content*/
typedef struct _pair_t {
  char             controllerName[64];
  uint8_t          controllerLTPK[32];
} _pair_t;

typedef struct _pair_list_in_flash_t {
  _pair_t          pairInfo[MAXPairNumber];
} pair_list_in_flash_t;


OSStatus HMClearPairList(void);
OSStatus HMReadPairList(pair_list_in_flash_t *pPairList);
OSStatus HMUpdatePairList(pair_list_in_flash_t *pPairList);
uint8_t * HMFindLTPK(char * name);


