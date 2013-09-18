/**************************************************************************
*
* sweetpea-board.h - C file for sweet pea boards using the CC3000
*
* This file contains knowledge about different sweet pea boards using
* the CC3000 chip. It can be used to set the board in a known state.
*
* Author: Pontus Oldberg - Electronic Sweet Peas
*
* Created: 130918
*
* "THE BEER-WARE LICENSE" (Revision 42):
* <pontus@sweetpeas.se> wrote this file. As long as you retain this notice you
* can do whatever you want with this stuff. If we meet some day, and you think
* this stuff is worth it, you can buy me a beer in return.
* Pontus Oldberg
*
****************************************************************************/

#include <Arduino.h>
#include "sweetpea-board.h"
#include "settings.hpp"

void setup_sweetpea_board(sweetpea_board_type_t type)
{
  switch (type)
  {
    case SWEET_PEA_WIFI_SHIELD:
      digitalWrite(WLAN_EN, LOW);
      pinMode(WLAN_EN, OUTPUT);
      /* Make sure SD card CS is disabled */
      digitalWrite(SD_CARD_CS, HIGH);
      pinMode(SD_CARD_CS, OUTPUT);
      /* Make sure SRAM CS is disabled */
      digitalWrite(SRAM_CS, HIGH);
      pinMode(SRAM_CS, OUTPUT);
      break;

    case SWEET_PEA_LEOFI:
      break;

    default:
      // Unknown board
      break;
  }
}
