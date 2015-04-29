/**
  ******************************************************************************
  * @file    MICOConfigMenu.h 
  * @author  William Xu
  * @version V1.0.0
  * @date    05-May-2014
  * @brief   This file provide function prototypes for createing configration 
  * 		 menu that can be displayed on EasyLink APP on iOS or Android.
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
#ifndef __MICOCONFIGMENU_H
#define __MICOCONFIGMENU_H

#include "Common.h"
#include "external/JSON-C/json.h"

typedef struct {
  char*  protocol;
  char*  hdVersion;
  char*  fwVersion;
  char*  rfVersion;
} OTA_Versions_t;


OSStatus MICOAddSector(json_object* sectors, char* const name,  json_object *menus);

OSStatus MICOAddStringCellToSector(json_object* menus, char* const name,  char* const content, char* const privilege, json_object* secectionArray);

OSStatus MICOAddNumberCellToSector(json_object* menus, char* const name,  int content, char* const privilege, json_object* secectionArray);

OSStatus MICOAddFloatCellToSector(json_object* menus, char* const name,  float content, char* const privilege, json_object* secectionArray);

OSStatus MICOAddSwitchCellToSector(json_object* menus, char* const name,  boolean content, char* const privilege);

OSStatus MICOAddMenuCellToSector(json_object* menus, char* const name, json_object* lowerSectors);

OSStatus MICOAddTopMenu(json_object **deviceInfo, char* const name, json_object* sectors, OTA_Versions_t versions);

#endif
