/**************************************************************************
*
* settings.hpp - Global settings for the SP3000 library
*
* This file contains the settings required to control the compile time
* behavior of the library.
*
* Author: Pontus Oldberg - Electronic Sweet Peas
*
* Created: 130918
*
****************************************************************************/
#ifndef __SP3000_SETTINGS_H__
#define __SP3000_SETTINGS_H__

/*
 * The following pin definitions define to what Arduino pin you have connected
 * each shield pin function to.
 *
 * There are some product specific limitations that need to be observed:
 * This are some guidelines for the Sweet Pea WiFi Shield (SP3001)
 *  The Wifi shield is compatible with all Rev3 Arduino boards and have some
 *  configuration options for the interrupt pin as well as the enable and
 *  chip select pin. Please check the data sheet to see what options are
 *  available to you.
 */

#define WLAN_CS           10 // Arduino pin connected to CC3000 WLAN_SPI_CS
#define WLAN_EN           7  // Arduino pin connected to CC3000 VBAT_SW_EN
#define WLAN_IRQ          3  // Arduino pin connected to CC3000 WLAN_SPI_IRQ
#define WLAN_IRQ_INTNUM   1  // The attachInterrupt() number that corresponds
                              // to WLAN_IRQ
#define SD_CARD_CS        4  // Pin connected to the CS signal of the SD Card
#define SRAM_CS           9  // Pin connected to the CS signal of the SRAM

/*
 * The library have a few configuration options that allows you to reduce its
 * already small footprint. For each option that you enable you will of
 * course lose some functionality. You can read more about the tiny library
 * here: http://processors.wiki.ti.com/index.php/Tiny_Driver_Support
 */

#undef CC3000_TINY_DRIVER  1
/*
 * You can also disable SmartConfig enryption whch will also save us a bunch
 * of memory but will disallow the use of encrypted SmartConfig.
 */
#undef CC3000_UNENCRYPTED_SMART_CONFIG 1

#endif
