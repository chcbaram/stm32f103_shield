/*
 * hw.c
 *
 *  Created on: Apr 9, 2022
 *      Author: baram
 */


#include "hw.h"





bool hwInit(void)
{
  bspInit();

  cliInit();
  ledInit();
  uartInit();

  logPrintf("\r\n[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", _DEF_BOARD_NAME);
  logPrintf("Booting..Ver  \t\t: %s\r\n", _DEF_FIRMWATRE_VERSION);
  logPrintf("Sys Clk       \t\t: %d Mhz\r\n", (int)HAL_RCC_GetSysClockFreq()/1000000);

  return true;
}
