/**************************************************************************
*
*  This file is part of the ArduinoCC3000 library.

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
*
*  This file is the main module for the Arduino CC3000 library.
*  Your program must call CC3000_Init() before any other API calls.
* 
****************************************************************************/
#ifndef __CORE_HPP_GUARD__
#define __CORE_HPP_GUARD__

#include <Arduino.h>
#include "settings.hpp"

/*
 * The following pin definitions define to what Arduino pin you have connected
 * each shield pin function to.
 *
 * There are some product specific limitations that need to be observed:
 *
 * Guidelines for the Sweet Pea WiFi Shield (SP3001)
 *  The Wifi shield is compatible with all Rev3 Arduino boards and have some
 *  configuration options for the interrupt pin as well as the enable and
 *  chip select pin. Please check the data sheet to see what options are
 *  available to you.
 *
 * Guidelines for Sweet Pea LeoFi (SP3011)
 *   The LeoFi is a Leonardo compatible CPU board with the CC3000 chip mounted
 *   on it. The LeoFi has a ficed configuration which can not be changed (unless
 *   you bring out your hardware hacking skillz). For the LeoFi The
 *   CC3000_init must be called like this: CC3000_Init (0, 6, 5, 7, 3)
 *
 */

extern uint8_t WLAN_IRQ_INTNUM;  // The attachInterrupt() number that corresponds
                                  // to WLAN_IRQ

extern void CC3000_Init(byte mode);
/*
 * Differently from the original author of these files we decided to include
 * the digitalFastWrite header file with this library. At least until the
 * Arduino framework actually supports this feature properly.
 */
#define irq_read()			            ((PINB & _BV(2)) >> 2)
#define negate_cs()		              (PORTB |= _BV(4))
#define assert_cs()			            (PORTB &= ~_BV(4))

#define MAC_ADDR_LEN	6
#define DISABLE	(0)
#define ENABLE	(1)

enum interrupt_state {
  INT_DISABLED = 0,
  INT_ENABLED = 1
};
extern enum interrupt_state wlan_int_status;

extern byte asyncNotificationWaiting;
extern long lastAsyncEvent;
extern byte dhcpIPAddress[];

extern volatile unsigned long ulSmartConfigFinished;
extern volatile unsigned long ulCC3000Connected;
extern volatile unsigned long ulCC3000DHCP;
extern volatile unsigned long OkToDoShutDown;
extern volatile unsigned long ulCC3000DHCP_configured;

extern volatile unsigned char ucStopSmartConfig;

/* Function Prototypes */
inline long ReadWlanInterruptPin(void);
void WriteWlanEnablePin(unsigned char val);
void WlanInterruptEnable(void);
void WlanInterruptDisable(void);
void CC3000_AsyncCallback(long lEventType, char * data, unsigned char length);
char *SendFirmwarePatch(unsigned long *Length);
char *SendDriverPatch(unsigned long *Length);
char *SendBootloaderPatch(unsigned long *Length);

void sp_core_register_event_cb (void (*cb_ptr)(uint32_t EventType,
    char *data,
    uint8_t len));

#endif
