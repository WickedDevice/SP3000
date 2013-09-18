/**************************************************************************
*
* sweetpea-board.h - Header file for sweet pea boards using the CC3000
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

/*
 * This enum contains the different sweet pea boards, both shields and
 * CPU board. You should simply call the function setup_sweetpea_board
 * with the specified board and the system will make a basic configuration
 * for you.
 */
enum sweetpea_board_type {
  SWEET_PEA_WIFI_SHIELD = 1,
  SWEET_PEA_LEOFI = 2
};
typedef enum sweetpea_board_type sweetpea_board_type_t;

/*
 * Function for setting up a default working condition for Sweet Pea
 * shields and CPU boards. This function should be called with a value from
 * the enum above.
 */
void setup_sweetpea_board(sweetpea_board_type_t type);
