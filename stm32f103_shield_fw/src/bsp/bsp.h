/*
 * bsp.h
 *
 *  Created on: Apr 9, 2022
 *      Author: baram
 */

#ifndef BSP_BSP_H_
#define BSP_BSP_H_

#include "def.h"


#include "stm32f1xx_hal.h"


#define Error_Handler()   Error_HandlerEx(__FILE__, __LINE__)


#define _USE_LOG_PRINT    1

#if _USE_LOG_PRINT
#define logPrintf(fmt, ...)     printf(fmt, ##__VA_ARGS__)
#else
#define logPrintf(fmt, ...)
#endif



bool bspInit(void);


void delay(uint32_t delay_ms);
uint32_t millis(void);


void Error_HandlerEx(const char *file_name, int line_number);


#endif /* BSP_BSP_H_ */
