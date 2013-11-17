/*
 * leds.cpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */


#include <Arduino.h>
#include <Wire.h>
#include "settings.hpp"

#ifndef SP_DISABLE_LEDS
#include "pca9536.hpp"
#include "leds.hpp"

void setled (uint8_t led, uint8_t value)
{
  // The system always starts with all LED's off
  static byte led_shadow = 0x00;

  led_shadow = (value) ? led_shadow & ~led : led_shadow | led;
  Wire.beginTransmission(PCA9536_BASE_ADDRESS);
  Wire.write(PCA9536_REG_OUT);
  Wire.write(led_shadow);
  Wire.endTransmission();
}

void initled (void)
{
  Wire.begin();
  // Make all IO pins outputs
  Wire.beginTransmission(PCA9536_BASE_ADDRESS);
  Wire.write(PCA9536_REG_CTR);
  Wire.write(0x00);
  Wire.endTransmission();

  // Make sure all pins are non inverted
  Wire.beginTransmission(PCA9536_BASE_ADDRESS);
  Wire.write(PCA9536_REG_CTR);
  Wire.write(0x00);
  Wire.endTransmission();

  // And that all LED's are off
  Wire.beginTransmission(PCA9536_BASE_ADDRESS);
  Wire.write(PCA9536_REG_OUT);
  Wire.write(0xff);
  Wire.endTransmission();
}

#endif // SP_DISABLE_LEDS
