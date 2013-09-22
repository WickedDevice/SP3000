/*
 * leds.hpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#ifndef LEDS_HPP_
#define LEDS_HPP_

enum available_leds {
  LED_CON = 0x08,
  LED_ACT = 0x04,
  LED_ERR = 0x02,
  LED_AUX = 0x01,
};

enum led_state {
  LED_OFF = 0x00,
  LED_ON = 0x01
};
typedef enum led_state led_state_t;

//
// Function prototypes
//
void setled (unsigned char led, led_state_t value);
void initled (void);

#endif /* LEDS_HPP_ */
