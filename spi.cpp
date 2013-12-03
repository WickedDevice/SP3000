/**************************************************************************
 *
 *  ArduinoCC3000SPI.cpp - SPI functions to connect an Arduino to the TI
 *                       CC3000
 *
 *  This code uses the Arduino hardware SPI library.
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
#include <SPI.h>

#include "debug.hpp"
#include "core.hpp"
#include "hci.hpp"
#include "spi.hpp"

#define READ                    3
#define WRITE                   1

#define HI(value)               (((value) & 0xFF00) >> 8)
#define LO(value)               ((value) & 0x00FF)

#define HEADERS_SIZE_EVNT       (SPI_HEADER_SIZE + 5)

#define SPI_HEADER_SIZE			(5)

#define 	eSPI_STATE_POWERUP 				 (0)
#define 	eSPI_STATE_INITIALIZED  		 (1)
#define 	eSPI_STATE_IDLE					 (2)
#define 	eSPI_STATE_WRITE_IRQ	   		 (3)
#define 	eSPI_STATE_WRITE_FIRST_PORTION   (4)
#define 	eSPI_STATE_WRITE_EOT			 (5)
#define 	eSPI_STATE_READ_IRQ				 (6)
#define 	eSPI_STATE_READ_FIRST_PORTION	 (7)
#define 	eSPI_STATE_READ_EOT				 (8)

typedef struct {
    gcSpiHandleRx SPIRxHandler;

    unsigned short usTxPacketLength;
    unsigned short usRxPacketLength;
    unsigned long ulSpiState;
    unsigned char *pTxPacket;
    unsigned char *pRxPacket;
} tSpiInformation;

tSpiInformation sSpiInformation;

//
// Static buffer for 5 bytes of SPI HEADER
//
unsigned char tSpiReadHeader[] = { READ, 0, 0, 0, 0 };

// The magic number that resides at the end of the TX/RX buffer (1 byte after
// the allocated size) for the purpose of detection of the overrun. The location
// of the memory where the magic number resides shall never be written.
// In case it is written - the overrun occurred and either receive function
// or send function will stuck forever.
#define CC3000_BUFFER_MAGIC_NUMBER (0xDE)

char spi_buffer[CC3000_RX_BUFFER_SIZE];
unsigned char wlan_tx_buffer[CC3000_TX_BUFFER_SIZE];

//*****************************************************************************
//
//!
//!
//!  \param  SpiPauseSpi
//!
//!  \return none
//!
//!  \brief:
//
//*****************************************************************************

void SpiPauseSpi(void)
{
  EIMSK &= ~(1 << INT2);
}

//*****************************************************************************
//
//!
//!
//!  \param  SpiResumeSpi
//!
//!  \return none
//!
//!  \brief:
//
//*****************************************************************************

void SpiResumeSpi(void)
{
  EICRA = (EICRA & ~((1 << ISC20) | (1 << ISC21))) | (FALLING << ISC20);
  EIMSK |= (1 << INT2);
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  SpiTriggerRxProcessing
//!
//!  \return none
//!
//!  \brief  The function triggers a user provided callback for 
//
//*****************************************************************************
void SpiTriggerRxProcessing(void)
{
  //
  // Trigger Rx processing
  //
  SpiPauseSpi();
  negate_cs();

  // The magic number that resides at the end of the TX/RX buffer (1 byte after
  // the allocated size) for the purpose of detection of the overrun.
  // If the magic number is overriten - a buffer overrun has occurred - and we
  // will stay here forever!
  if (sSpiInformation.pRxPacket[CC3000_RX_BUFFER_SIZE - 1]
      != CC3000_BUFFER_MAGIC_NUMBER) {
    lSer.println ("Error: buffer overwritten !");
    while (1);
  }

  sSpiInformation.ulSpiState = eSPI_STATE_IDLE;
  sSpiInformation.SPIRxHandler(sSpiInformation.pRxPacket + SPI_HEADER_SIZE);
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void SpiReadDataSynchronous(unsigned char *data, unsigned short size)
{
  long i = 0;
  unsigned char *data_to_send = tSpiReadHeader;

  for (i = 0; i < size; i++) {
    data[i] = SPI.transfer(0x03);
  }
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void SpiWriteDataSynchronous(unsigned char *data, unsigned short size)
{
  while (size) {
    SPI.transfer(*data);
    size--;
    data++;
  }
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
long SpiFirstWrite(unsigned char *ucBuf, unsigned short usLength)
{

  //
  // workaround for first transaction
  //
  assert_cs();

  delayMicroseconds(50);

  // SPI writes first 4 bytes of data
  SpiWriteDataSynchronous(ucBuf, 4);

  delayMicroseconds(50);

  SpiWriteDataSynchronous(ucBuf + 4, usLength - 4);

  sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

  negate_cs();

  return (0);
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
long SpiWrite(unsigned char *pUserBuffer, unsigned short usLength)
{
  unsigned char ucPad = 0;

  //
  // Figure out the total length of the packet in order to figure out if there is padding or not
  //
  if (!(usLength & 0x0001))
    ucPad++;

  pUserBuffer[0] = WRITE;
  pUserBuffer[1] = HI(usLength + ucPad);
  pUserBuffer[2] = LO(usLength + ucPad);
  pUserBuffer[3] = 0;
  pUserBuffer[4] = 0;

  usLength += (SPI_HEADER_SIZE + ucPad);

  // The magic number that resides at the end of the TX/RX buffer (1 byte after the allocated size)
  // for the purpose of overrun detection. If the magic number is overwritten - buffer overrun
  // occurred - and we will be stuck here forever!
  if (wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] != CC3000_BUFFER_MAGIC_NUMBER) {
    while (1);
  }

  if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
    while (sSpiInformation.ulSpiState != eSPI_STATE_INITIALIZED);
  }

  if (sSpiInformation.ulSpiState == eSPI_STATE_INITIALIZED) {
    //
    // This is time for first TX/RX transactions over SPI: the IRQ is down - so need to send read buffer size command
    //
    SpiFirstWrite(pUserBuffer, usLength);
  } else {
    //
    // We need to prevent here race that can occur in case two back to back packets are sent to the
    // device, so the state will move to IDLE and once again to not IDLE due to IRQ
    //
    tSLInformation.WlanInterruptDisable();

    while (sSpiInformation.ulSpiState != eSPI_STATE_IDLE);

    sSpiInformation.ulSpiState = eSPI_STATE_WRITE_IRQ;
    sSpiInformation.pTxPacket = pUserBuffer;
    sSpiInformation.usTxPacketLength = usLength;

    //
    // Assert the CS line and wait till SSI IRQ line is active and then initialize write operation
    //
    assert_cs();

    //
    // Re-enable IRQ - if it was not disabled - this is not a problem...
    //
    tSLInformation.WlanInterruptEnable();

    //
    // check for a missing interrupt between the CS assertion and enabling back the interrupts
    //
    if (tSLInformation.ReadWlanInterruptPin() == 0) {
      SpiWriteDataSynchronous(sSpiInformation.pTxPacket,
          sSpiInformation.usTxPacketLength);

      sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

      negate_cs();
    }
  }

  //
  // Due to the fact that we are currently implementing a blocking situation
  // here we will wait till end of transaction
  //

  while (eSPI_STATE_IDLE != sSpiInformation.ulSpiState);

  return (0);
}

//*****************************************************************************
//
//! This function processes received SPI Header and in accordance with it - continues reading 
//!	the packet
//!
//!  \param  None
//!
//!  \return None
//!
//!  \brief  ...
//
//*****************************************************************************
long SpiReadDataCont(void)
{
  long data_to_recv;
  unsigned char *evnt_buff, type;

  //
  //determine what type of packet we have
  //
  evnt_buff = sSpiInformation.pRxPacket;
  data_to_recv = 0;
  STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE), HCI_PACKET_TYPE_OFFSET,
      type);

  switch (type) {
    case HCI_TYPE_DATA: {
      //
      // We need to read the rest of data..
      //
      STREAM_TO_UINT16((char *)(evnt_buff + SPI_HEADER_SIZE),
          HCI_DATA_LENGTH_OFFSET, data_to_recv);
      if (!((HEADERS_SIZE_EVNT + data_to_recv) & 1)) {
        data_to_recv++;
      }

      if (data_to_recv) {
        SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
      }
      break;
    }
    case HCI_TYPE_EVNT: {
      //
      // Calculate the rest length of the data
      //
      STREAM_TO_UINT8((char *)(evnt_buff + SPI_HEADER_SIZE),
          HCI_EVENT_LENGTH_OFFSET, data_to_recv);
      data_to_recv -= 1;

      //
      // Add padding byte if needed
      //
      if ((HEADERS_SIZE_EVNT + data_to_recv) & 1) {

        data_to_recv++;
      }

      if (data_to_recv) {
        SpiReadDataSynchronous(evnt_buff + 10, data_to_recv);
      }

      sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
      break;
    }
  }

  return (0);
}

//*****************************************************************************
//
//! This function enter point for write flow
//!
//!  \param  SSIContReadOperation
//!
//!  \return none
//!
//!  \brief  The function triggers a user provided callback for 
//
//*****************************************************************************

void SSIContReadOperation(void)
{
  //
  // The header was read - continue with  the payload read
  //
  if (!SpiReadDataCont()) {

    //
    // All the data was read - finalize handling by switching to teh task
    //	and calling from task Event Handler
    //
    SpiTriggerRxProcessing();
  }
}

//*****************************************************************************
//
//! This function enter point for read flow: first we read minimal 5 SPI header bytes and 5 Event
//!	Data bytes
//!
//!  \param  buffer
//!
//!  \return none
//!
//!  \brief  ...
//
//*****************************************************************************
void SpiReadHeader(void)
{
  SpiReadDataSynchronous(sSpiInformation.pRxPacket, 10);
}

//*****************************************************************************
// 
//!  The IntSpiGPIOHandler interrupt handler
//! 
//!  \param  none
//! 
//!  \return none
//! 
//!  \brief  GPIO A interrupt handler. When the external SSI WLAN device is
//!          ready to interact with Host CPU it generates an interrupt signal.
//!          After that Host CPU has registrated this interrupt request
//!          it set the corresponding /CS in active state.
// 
//*****************************************************************************
void CC3000InterruptHandler(void)
{
  if (sSpiInformation.ulSpiState == eSPI_STATE_POWERUP) {
    /* This means IRQ line was low call a callback of HCI Layer to inform on event */
    sSpiInformation.ulSpiState = eSPI_STATE_INITIALIZED;
  } else if (sSpiInformation.ulSpiState == eSPI_STATE_IDLE) {
    sSpiInformation.ulSpiState = eSPI_STATE_READ_IRQ;
    /* IRQ line goes down - start reception */
    assert_cs();
    SpiReadHeader();
    sSpiInformation.ulSpiState = eSPI_STATE_READ_EOT;
    SSIContReadOperation();
  } else if (sSpiInformation.ulSpiState == eSPI_STATE_WRITE_IRQ) {

    SpiWriteDataSynchronous(sSpiInformation.pTxPacket,
        sSpiInformation.usTxPacketLength);
    sSpiInformation.ulSpiState = eSPI_STATE_IDLE;

    negate_cs();
  }
}

//*****************************************************************************
//
//!  SpiClose
//!
//!  \param  none
//!
//!  \return none
//!
//!  \brief  Cofigure the SSI
//
//*****************************************************************************
void SpiOpen(gcSpiHandleRx pfRxHandler)
{
  sSpiInformation.ulSpiState = eSPI_STATE_POWERUP;

  memset(spi_buffer, 0, sizeof(spi_buffer));
  memset(wlan_tx_buffer, 0, sizeof(spi_buffer));

  sSpiInformation.SPIRxHandler = pfRxHandler;
  sSpiInformation.usTxPacketLength = 0;
  sSpiInformation.pTxPacket = NULL;
  sSpiInformation.pRxPacket = (unsigned char *) spi_buffer;
  sSpiInformation.usRxPacketLength = 0;
  spi_buffer[CC3000_RX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;
  wlan_tx_buffer[CC3000_TX_BUFFER_SIZE - 1] = CC3000_BUFFER_MAGIC_NUMBER;

  //
  // Enable interrupt on the GPIO pin of WLAN IRQ
  //
  tSLInformation.WlanInterruptEnable();
}

//*****************************************************************************
//
//!  SpiClose
//!
//!  \param  none
//!
//!  \return none
//!
//!  \brief  Cofigure the SSI
//
//*****************************************************************************
void SpiClose(void)
{
  if (sSpiInformation.pRxPacket)
    sSpiInformation.pRxPacket = 0;

  //
  //	Disable Interrupt in GPIOA module...
  //
  tSLInformation.WlanInterruptDisable();
}

//*****************************************************************************
//
//!  SpiInit
//!
//!  \param  none
//!
//!  \return Success or fail (Not implemented )
//!
//!  \brief  Configure SPI hardware for operation with the CC3000
//
//*****************************************************************************
int SpiInit(void)
{

  // Disable the CC3000 by default */
  DDRB |= _BV(3);   //pinMode(WLAN_EN, OUTPUT);
  PORTB &= ~_BV(3); //digitalWrite(WLAN_EN, 0);
  delay(500);

  /* Set CS pin to output */
  DDRB |= _BV(4); //pinMode(WLAN_CS, OUTPUT);

  /* Set interrupt pin to input */
  DDRB &= _BV(2);  //pinMode(WLAN_IRQ, INPUT);
  PORTB |= _BV(2); //digitalWrite(WLAN_IRQ, HIGH); /* Use a weak pullup */

//  SpiConfigStoreOld(); // prime ccspi_old* values for DEASSERT

  /* Initialise SPI */
  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);

  negate_cs();

  return(0);
}
