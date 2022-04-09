/*
 * ap.c
 *
 *  Created on: Apr 9, 2022
 *      Author: baram
 */


#include "ap.h"





void apInit(void)
{
  cliOpen(_DEF_UART1, 115200);
}

void apMain(void)
{
  uint32_t pre_time;
  uint8_t led_st = 0;
  int8_t  led_pos = 0;
  int8_t  led_dir = 1;


  pre_time = millis();
  while(1)
  {
    if (millis()-pre_time >= 50)
    {
      pre_time = millis();

      if (led_st == 0)
        ledOn(led_pos);
      else
        ledOff(led_pos);

      if (led_st%2 == 1)
      {
        led_pos += led_dir;
        if (led_pos >= LED_MAX_CH || led_pos < 0)
        {
          led_dir *= -1;
          led_pos += led_dir;
        }
      }
      led_st++;
      led_st %= 2;
    }

    cliMain();
  }
}
