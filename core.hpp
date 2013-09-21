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

#include "digitalWriteFast.hpp"
#include "settings.hpp"

/*
 * Differently from the original author of these files we decided to include
 * the digitalFastWrite header file with this library. At least until the
 * Arduino framework actually supports this feature properly.
 */
#define irq_read()			            digitalReadFast(WLAN_IRQ)
#define negate_cs()		              digitalWriteFast(WLAN_CS, HIGH)
#define assert_cs()			            digitalWriteFast(WLAN_CS, LOW)

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
extern void CC3000_Init(byte);

/* Function Prototypes */
inline long ReadWlanInterruptPin(void);
void WriteWlanEnablePin(unsigned char val);
void WlanInterruptEnable(void);
void WlanInterruptDisable(void);
void CC3000_AsyncCallback(long lEventType, char * data, unsigned char length);
char *SendFirmwarePatch(unsigned long *Length);
char *SendDriverPatch(unsigned long *Length);
char *SendBootloaderPatch(unsigned long *Length);

#endif
