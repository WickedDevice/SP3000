/**************************************************************************
*
*  ArduinoCC3000Core.cpp - Wrapper routines to make interfacing the Arduino
*                      and the TI CC3000 easier.
*
*  This code is based on the TI sample code "Basic WiFi Application"
*  and has the callback routines that TI expects for their library.
*
*  TI uses callback routines to make their library portable: these routines,
*  and the routines in the SPI files, will be different for an Arduino,
*  a TI MSP430, a PIC, etc. but the core library shouldn't have to be
*  changed.
*  
*  Version 1.0.1b
* 
*  Copyright (C) 2013 Chris Magagna - cmagagna@yahoo.com
*  Adapted to Sweet Pea products by Pontus Oldberg - Electronic Sweet Peas
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*  Don't sue me if my code blows up your board and burns down your house
*
****************************************************************************/

#include <Arduino.h>
#include <Wire.h>

#include "core.hpp"
#include "wlan.hpp"
#include "hci.hpp"
#include "spi.hpp"
#include "wildfire_cc3000_pins.h"

volatile unsigned long ulSmartConfigFinished,
	ulCC3000Connected,
	ulCC3000DHCP,
	OkToDoShutDown,
	ulCC3000DHCP_configured;

volatile unsigned char ucStopSmartConfig;

#define NETAPP_IPCONFIG_MAC_OFFSET     (20)
#define CC3000_APP_BUFFER_SIZE         (5)
#define CC3000_RX_BUFFER_OVERHEAD_SIZE (20)

byte asyncNotificationWaiting=false;
long lastAsyncEvent;
byte dhcpIPAddress[4];

void (*cb_ptr)(uint32_t EventType,
               char *data,
               uint8_t len) = NULL;   // Function pointer for the async callback

/*-------------------------------------------------------------------

    The TI library calls this routine when asynchronous events happen.
    
    For example you tell the CC3000 to turn itself on and connect
    to an access point then your code can go on to do its own thing.
    When the CC3000 is done configuring itself (e.g. it gets an IP
    address from the DHCP server) it will call this routine so you
    can take appropriate action.    

---------------------------------------------------------------------*/
void CC3000_AsyncCallback(long lEventType, char * data, unsigned char length) 
{

	lastAsyncEvent = lEventType;

	switch (lEventType) {
  
		case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
			ulSmartConfigFinished = 1;
			ucStopSmartConfig     = 1;
			asyncNotificationWaiting=true;
			break;
			
		case HCI_EVNT_WLAN_UNSOL_CONNECT:
		  ulCC3000Connected = 1;
			asyncNotificationWaiting=true;
			break;
			
		case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
			ulCC3000Connected = 0;
			ulCC3000DHCP      = 0;
			ulCC3000DHCP_configured = 0;			
			asyncNotificationWaiting=true;
			break;
			
		case HCI_EVNT_WLAN_UNSOL_DHCP:
			// Notes: 
			// 1) IP config parameters are received swapped
			// 2) IP config parameters are valid only if status is OK, i.e.
            //    ulCC3000DHCP becomes 1
			// only if status is OK, the flag is set to 1 and the addresses are valid
			if ( *(data + NETAPP_IPCONFIG_MAC_OFFSET) == 0) {
				ulCC3000DHCP = 1;
				dhcpIPAddress[0] = data[3];
				dhcpIPAddress[1] = data[2];
				dhcpIPAddress[2] = data[1];
				dhcpIPAddress[3] = data[0];
				}
			else {
				ulCC3000DHCP = 0;
				dhcpIPAddress[0] = 0;
				dhcpIPAddress[1] = 0;
				dhcpIPAddress[2] = 0;
				dhcpIPAddress[3] = 0;
				}
			asyncNotificationWaiting=true;
			break;
			
		case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
			OkToDoShutDown = 1;
			asyncNotificationWaiting=true;
			break;
						
		default:
			asyncNotificationWaiting=true;
			break;		
		}
	  // If we have a user defined call back make sure it is being called
	  if (cb_ptr)
	    cb_ptr(lEventType, data, length);
	}

/*-------------------------------------------------------------------

    Stores a user function pointer which will be called when an
    asynchronous even occurs.

---------------------------------------------------------------------*/
void sp_core_register_event_cb (void (*f_ptr)(uint32_t EventType,
    char *data,
    uint8_t len))
{
  cb_ptr = f_ptr;
}

/*-------------------------------------------------------------------

    The TI library calls these routines on CC3000 startup.
    
    This library does not send firmware, driver, or bootloader patches
    so we do nothing and we return NULL.

---------------------------------------------------------------------*/

char *SendFirmwarePatch(unsigned long *Length)
{
	*Length = 0;
	return NULL;
}



char *SendDriverPatch(unsigned long *Length) 
{
	*Length = 0;
	return NULL;
}


char *SendBootloaderPatch(unsigned long *Length)
{
	*Length = 0;
	return NULL;
}

/*-------------------------------------------------------------------

    The TI library calls this routine to get the status of the
    WLAN_IRQ pin.
    
    The CC3000 will set this pin LOW when it needs to send data
    to the Arduino, and ordinarily the interrupt routine will
    be called via attachInterrupt(), but on some synchronous
    events (e.g. startup) the interrupt routine is disabled
    and the library reads it directly.

---------------------------------------------------------------------*/
inline long ReadWlanInterruptPin(void) 
{
	return(irq_read());
}

/*-------------------------------------------------------------------

    The TI library calls this routine to set the WLAN_EN pin,
    which tells the CC3000 to turn itself on (HIGH) or off (LOW).
    
 --------------------------------------------------------------------*/
inline void WriteWlanEnablePin(unsigned char val)
{

//  digitalWriteFast (WLAN_EN, (val) ? HIGH : LOW);
	if (val) {
		WLAN_EN_PORT |= _BV(WLAN_EN_PIN); //digitalWriteFast (WLAN_EN, HIGH);
	}
	else {
		WLAN_EN_PORT &= ~_BV(WLAN_EN_PIN); //digitalWriteFast (WLAN_EN, LOW);
	}
}

/*-------------------------------------------------------------------

    The TI library calls these routines to enable or disable interrupts
    on the WLAN_IRQ pin.
    
 --------------------------------------------------------------------*/
void WlanInterruptEnable(void) 
{
  attachInterrupt(WLAN_IRQ_INTNUM, CC3000InterruptHandler, FALLING);
}


void WlanInterruptDisable(void) 
{
  detachInterrupt(WLAN_IRQ_INTNUM);
}

/*-------------------------------------------------------------------

    This is my routine to simplify CC3000 startup.
    
    It sets the Arduino pins then calls the normal CC3000 routines
    wlan_init() with all the callbacks and wlan_start() with 0
    to indicate we're not sending any patches.
    
 --------------------------------------------------------------------*/
void sp_wifi_init (byte startReqest)
{
  /*
   * Initialize pins used
   */


  // Initialize the SPI library
  SpiInit();

	/* 
	 * Set the initial state of the output pins before
	 * enabling them as outputs in order to avoid
	 * glitches. Also the hw need to have pull up/down
	 * resistors on the signals pulling them to their
	 * inactive state.
	 */
	negate_cs();	// turn off CS until we're ready

	/* Set the modes for the control pins */
	WLAN_IRQ_DDR &= ~_BV(WLAN_IRQ_PIN); // pinMode(WLAN_IRQ, INPUT);
	WLAN_EN_DDR |= _BV(WLAN_EN_PIN);    // pinMode(WLAN_EN, OUTPUT);
	WLAN_CS_DDR |= _BV(WLAN_CS_PIN);    // pinMode(WLAN_CS, OUTPUT);

	//setled (LED_CON | LED_ACT | LED_ERR | LED_AUX, LED_ON);
	//delay (100);
	//setled (LED_CON | LED_ACT | LED_ERR | LED_AUX, LED_OFF);

	wlan_init( CC3000_AsyncCallback,
		SendFirmwarePatch,
		SendDriverPatch,
		SendBootloaderPatch,
		ReadWlanInterruptPin,
		SpiResumeSpi, // WlanInterruptEnable,
		SpiPauseSpi, // WlanInterruptDisable,
		WriteWlanEnablePin);
	
	WlanInterruptEnable();

	wlan_start(startReqest);
}

