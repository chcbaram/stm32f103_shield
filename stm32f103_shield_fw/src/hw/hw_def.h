/*
 * hw_def.h
 *
 *  Created on: Apr 9, 2022
 *      Author: baram
 */

#ifndef HW_HW_DEF_H_
#define HW_HW_DEF_H_

#include "def.h"
#include "bsp.h"


#define _DEF_FIRMWATRE_VERSION    "V220410R1"
#define _DEF_BOARD_NAME           "STM32F103_SHIELD"



#define _USE_HW_LED
#define      HW_LED_MAX_CH          8

#define _USE_HW_UART
#define      HW_UART_MAX_CH         1

#define _USE_HW_CLI
#define      HW_CLI_CMD_LIST_MAX    24
#define      HW_CLI_CMD_NAME_MAX    16
#define      HW_CLI_LINE_HIS_MAX    4
#define      HW_CLI_LINE_BUF_MAX    64


#endif /* HW_HW_DEF_H_ */
