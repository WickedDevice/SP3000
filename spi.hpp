/**************************************************************************
*
*  ArduinoCC3000SPI.h - SPI functions to connect an Arduino to the TI
*                       CC3000
*
*  This code uses the Arduino hardware SPI library (There is also a bit
*  banged version available) to send & receive data between the library
*  API calls and the CC3000 hardware.
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
#ifndef __SPI_H_GUARD__
#define __SPI_H_GUARD__

typedef void (*gcSpiHandleRx)(void *p);

//*****************************************************************************
//
// Prototypes for the APIs.
//
//*****************************************************************************

extern short SPIInterruptsEnabled;
extern unsigned char wlan_tx_buffer[];

extern void SpiOpen(gcSpiHandleRx pfRxHandler);
extern void SpiClose(void);
extern long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength);
extern void SpiResumeSpi(void);
extern void CC3000InterruptHandler(void);
extern int SpiInit (void);

#endif
