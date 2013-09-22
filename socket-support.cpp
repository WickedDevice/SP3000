/*
 * socket-support.cpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#include "sp3000.h"

// Enable the DEBUG macro to get debug prints
//#define DEBUG
#ifdef DEBUG
#define PRINT(x) Serial1.print(x)
#define PRINT_LN(x) Serial1.println(x)
#else
#define PRINT(x)
#define PRINT_LN(x)
#endif

uint8_t txbuf[BUFFER_SIZE];
uint8_t rxbuf[BUFFER_SIZE];

#if BUFFER_SIZE > 255
uint16_t buf_siz = 0;
uint16_t rx_ptr = 0;
#else
uint8_t buf_siz = 0;
uint8_t rx_ptr = 0;
#endif


//*****************************************************************************
//*
//* Description:
//*   A short debug function for printing an IP address in a proper format.
//*   This function is not exported or even built when the DEBUG flag is not
//*   defined.
//*
//*****************************************************************************
#ifdef DEBUG
void dbgIP (uint32_t IP)
{
  char buff[16];
  sprintf (buff, "%d.%d.%d.%d",
      (unsigned char)(IP >> 24), (unsigned char)(IP >> 16) & 0xff,
      (unsigned char)(IP >> 8) & 0xff, (unsigned char)IP & 0xff);
  Serial1.print (buff);
}
#endif

//*****************************************************************************
//*
//* Description:
//*   This function attempts to create a TCP connection to the provided
//*   destination host and port. If successful it returns the socket handle
//*   associated with the connection and if it fails it simply returns 0.
//*
//*****************************************************************************
int32_t sp_connect (uint32_t destIP, uint16_t destPort, uint32_t type)
{
  sockaddr      socketAddress;
  uint32_t      sock;

  if (type != IPPROTO_TCP && type != IPPROTO_UDP) {
    PRINT_LN (F("Invalid socket type !"));
    return -1;
  }
  // Create a socket for the connection
  PRINT_LN (F("Creating socket !"));
  sock = socket(AF_INET, SOCK_STREAM, type);
  if (sock < 0)
  {
    PRINT_LN(F("Failed to open socket"));
    return sock;
  }

  // Try to open the socket
  memset(&socketAddress, 0x00, sizeof(socketAddress));
  socketAddress.sa_family = AF_INET;
  socketAddress.sa_data[0] = (destPort & 0xFF00) >> 8;  // Set the Port Number
  socketAddress.sa_data[1] = (destPort & 0x00FF);
  socketAddress.sa_data[2] = destIP >> 24;
  socketAddress.sa_data[3] = destIP >> 16;
  socketAddress.sa_data[4] = destIP >> 8;
  socketAddress.sa_data[5] = destIP;

#ifdef DEBUG
    PRINT(F("Trying to connect to "));
    dbgIP (destIP);
    PRINT(':');
    PRINT_LN(destPort);
#endif

  PRINT_LN (F("Connecting socket."));
  if (-1 == connect(sock, &socketAddress, sizeof(socketAddress)))
  {
    PRINT_LN(F("Connection error"));
    closesocket(sock);
    return -1;
  }
  PRINT_LN (F("Successfully connected !"));
  return sock;
}

//*****************************************************************************
//*
//* Description:
//*   Sends the content of a PROGMEM string over a socket.
//*
//*****************************************************************************
size_t send_F(uint32_t s, const __FlashStringHelper *string)
{
  uint8_t i = 0;
  size_t n = 0;

  PRINT_LN (F("Entered send_F"));
  const char PROGMEM *p = (const char PROGMEM *)string;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (!c) break;
    txbuf[i++] = c;
    if (i >= BUFFER_SIZE) {
      // Only reached when there is more in the buffer to send
      n += send(s, txbuf, BUFFER_SIZE, 0);
      i = 0;
    }
  }
  n += send(s, txbuf, i, 0);

  PRINT_LN (F("Leaving send_F"));
  return n;
}

//*****************************************************************************
//*
//* Description:
//*   Reads data from an incoming socket stream
//*
//*****************************************************************************
uint8_t read_data(int16_t s)
{
  PRINT_LN (F("Entered read_data"));

      while ((buf_siz <= 0) || (buf_siz == rx_ptr)) {
    buf_siz = recv(s, rxbuf, sizeof(rxbuf), 0);
    if (buf_siz == -57) {
      closesocket(s);
      return 0;
    }
    rx_ptr = 0;
  }

  PRINT_LN (F("Leaving read_data"));
  return rxbuf[rx_ptr++];
}

//*****************************************************************************
//*
//* Description:
//*   Checks if there is data available in the supplied socket stream
//*
//*****************************************************************************
uint8_t data_available(int16_t s)
{
  timeval timeout;
  fd_set fd_read;

  PRINT_LN (F("Entered data_available"));

  // Make sure the bloody socket is open
  if (s < 0) {
    PRINT_LN(F("Socket is closed."));
    return 0;
  }

  // Check if we already have som data that can be read from the buffer
  if ((buf_siz > 0) && (rx_ptr < buf_siz))
    return (buf_siz - rx_ptr);

  // Prepare to do select
  memset(&fd_read, 0, sizeof(fd_read));
  FD_SET(s, &fd_read);

  timeout.tv_sec = 0;
  timeout.tv_usec = 500000; // 500 millisec, My Raspberry Pi based test system
                            // takes a while to reply.

  PRINT_LN (F("Leaving data_available"));
  return ((select(s+1, &fd_read, NULL, NULL, &timeout) == 1) ? 1 : 0);
}
