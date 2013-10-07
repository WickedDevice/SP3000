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

#include <stdint.h>
/*
 * The library have a few configuration options that allows you to reduce its
 * already small footprint. For each option that you enable you will of
 * course lose some functionality. You can read more about the tiny library
 * here: http://processors.wiki.ti.com/index.php/Tiny_Driver_Support
 */

//#define CC3000_TINY_DRIVER  1

/*
 * You can also disable SmartConfig enryption whch will also save us a bunch
 * of memory but will disallow the use of encrypted SmartConfig.
 */
//#define CC3000_UNENCRYPTED_SMART_CONFIG 1

#endif
