/*
 * socket-support.cpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#include "sp3000.h"
#include "debug.hpp"
#include "spi.hpp"

uint8_t txbuf[BUFFER_SIZE];
uint8_t rxbuf[BUFFER_SIZE];

#if BUFFER_SIZE > 255
uint16_t buf_siz = 0;
uint16_t rx_ptr = 0;
#else
uint8_t buf_siz = 0;
uint8_t rx_ptr = 0;
#endif

static char *device_name = "CC3000";
// The Simple Config Prefix always needs to be 'TTT'
static char simpleConfigPrefix[] = { 'T', 'T', 'T' };

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
  lSer.print (buff);
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
int sp_connect(uint32_t destIP, uint16_t destPort, uint32_t type)
{
  sockaddr socketAddress;
  uint32_t sock;

  buf_siz = 0;
  rx_ptr = 0;

  if (type != IPPROTO_TCP && type != IPPROTO_UDP) {
    PRINTLN (F("Invalid socket type !"));
    return -1;
  }
  // Create a socket for the connection
  PRINTLN (F("Creating socket !"));
  sock = socket(AF_INET, SOCK_STREAM, type);
  if (sock < 0) {
    PRINTLN(F("Failed to open socket"));
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
  PRINTLN(destPort);
#endif

  PRINTLN (F("Connecting socket."));
  if (-1 == connect(sock, &socketAddress, sizeof(socketAddress))) {
    PRINTLN(F("Connection error"));
    closesocket(sock);
    return -1;
  } PRINTLN (F("Successfully connected !"));
  return sock;
}

//*****************************************************************************
//*
//* Description:
//*   Sends the content of the supplied string
//*
//*****************************************************************************
int sp_send (uint32_t s, char *str)
{
  uint8_t i = 0;
  size_t n = 0;
  int ret;

  PRINTLN (F("Entered send_c"));
  while (1) {
    if (!str[i])
      break;
    txbuf[i] = str[i];
    i++;
    if (i >= BUFFER_SIZE) {
      // Only reached when there is more in the buffer to send
      n += send(s, txbuf, BUFFER_SIZE, 0);
      i = 0;
    }
  }
  if (0 > (ret = send(s, txbuf, i, 0))) return ret;
  n += ret;

  PRINTLN (F("Leaving send_c"));
  return n;
}

//*****************************************************************************
//*
//* Description:
//*   Sends the content of the supplied long int over a socket
//*
//*****************************************************************************
int sp_send (uint32_t s, long value)
{
  uint8_t i = 0;
  size_t n = 0;
  int ret

  PRINTLN (F("Entered send_l"));
  ltoa (value, (char *)txbuf, 10);
  i = strlen((char*)txbuf);

  txbuf[i] = 0;
  if (0 > (ret = send(s, txbuf, i, 0))) return ret;
  n += ret;
  PRINTLN (F("Leaving send_l"));
  return n;
}

//*****************************************************************************
//*
//* Description:
//*   Sends the content of a PROGMEM string over a socket.
//*
//*****************************************************************************
int sp_send (uint32_t s, const __FlashStringHelper *string)
{
  uint8_t i = 0;
  size_t n = 0;
  int ret;

  PRINTLN (F("Entered send_F"));

  const char PROGMEM *p = (const char PROGMEM *) string;
  while (1) {
    unsigned char c = pgm_read_byte(p++);
    if (!c)
      break;
    txbuf[i++] = c;
    if (i >= BUFFER_SIZE) {
      if (0 > (ret = send (s, txbuf, BUFFER_SIZE, 0))) return ret;
      n += ret;
      i = 0;
    }
  }
  if ((ret = send(s, txbuf, i, 0)) < 0) return ret;
  n += ret;

  PRINTLN (F("Leaving send_F"));
  return n;
}

//*****************************************************************************
//*
//* Description:
//*   Peeks at the data from an incoming socket stream
//*
//*****************************************************************************
uint8_t sp_peek(int16_t s)
{
  PRINTLN (F("Entered sp_peek"));

  while ((buf_siz <= 0) || (buf_siz == rx_ptr)) {
    buf_siz = recv(s, rxbuf, sizeof(rxbuf), 0);
    if (buf_siz == -57) {
      closesocket(s);
      lSer.println (F("Oops, nasty things happen"));
      return 0;
    }
    rx_ptr = 0;
  }

  PRINTLN (F("Leaving sp_peek"));
  return rxbuf[rx_ptr];
}

//*****************************************************************************
//*
//* Description:
//*   Reads data from an incoming socket stream
//*
//*****************************************************************************
uint8_t sp_read(int16_t s)
{
  uint8_t c;

  PRINTLN (F("Entered sp_read"));
  c = sp_peek(s);
  rx_ptr++;

  PRINTLN (F("Leaving sp_read"));
  return c;
}

//*****************************************************************************
//*
//* Description:
//*   Checks if there is data available in the supplied socket stream
//*
//*****************************************************************************
uint8_t data_available(int16_t s,  uint16_t time)
{
  timeval timeout;
  fd_set fd_read;

  PRINTLN (F("Entered data_available"));

  // Make sure the bloody socket is open
  if (s < 0) {
    PRINTLN(F("Socket is closed."));
    return 0;
  }

  // Check if we already have some data that can be read from the buffer
  if ((buf_siz > 0) && (rx_ptr < buf_siz))
    return (buf_siz - rx_ptr);

  // Prepare to do select
  memset(&fd_read, 0, sizeof(fd_read));
  FD_SET(s, &fd_read);

  timeout.tv_sec = time;
  timeout.tv_usec = 0;

  PRINTLN (F("Leaving data_available"));
  return ((select(s + 1, &fd_read, NULL, NULL, &timeout) == 1) ? 1 : 0);
}

//*****************************************************************************
//*
//* Description:
//*   Checks if there is data available in the supplied socket stream
//*
//*****************************************************************************
uint8_t data_available(int16_t s)
{
  return data_available (s, 1);
}

//*****************************************************************************
//*
//* Description:
//*   A helper function for scanning CC3000 neighborhood
//*
//*****************************************************************************
#ifndef CC3000_TINY_DRIVER
void scan_ssid(uint32_t time)
{
  const unsigned long intervalTime[16] = { 2000, 2000, 2000, 2000, 2000, 2000,
      2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000, 2000 };

  PRINTLN (F("Entered scanSSIDs"));

  wlan_ioctl_set_scan_params(time, 20, 100, 5, 0x7FF, -120, 0, 300,
      (unsigned long *) &intervalTime);

  PRINTLN (F("Leaving scanSSIDs"));
}
#endif

//*****************************************************************************
//*
//* Description:
//*   Performs a smart config sequence
//*
//*****************************************************************************
char do_smart_config(void)
{
  long rval;
  long timeoutCounter;

  PRINTLN (F("Entered do_smart_config"));

  PRINTLN (F("Disabling auto-connect policy."));
  if ((rval = wlan_ioctl_set_connection_policy(0, 0, 0)) != 0) {
    PRINT (F("Setting auto connection policy failed, error: ")); PRINTLN (rval);
    return -1;
  }

  PRINTLN (F("Deleting all existing profiles."));
  if ((rval = wlan_ioctl_del_profile(255)) != 0) {
    PRINT (F("Deleting all profiles failed, error: ")); PRINTLN (rval);
    return -1;
  }

  PRINTLN (F("Waiting until disconnected..."));
  while (ulCC3000Connected == 1)
    ;

  PRINTLN (F("Setting smart config prefix."));
  if ((rval = wlan_smart_config_set_prefix(simpleConfigPrefix)) != 0) {
    PRINT (F("Setting smart config prefix failed, error: ")); PRINTLN (rval);
    return -1;
  }

  PRINTLN (F("Starting smart config !"));
  if ((rval = wlan_smart_config_start(0)) != 0) {
    PRINT (F("Starting smart config failed, error: ")); PRINTLN (rval);
    return -1;
  }

  // Wait for Smartconfig process complete or return an error if we have
  // timed out.
  timeoutCounter = millis();
  while (ulSmartConfigFinished == 0) {
    if (millis() - timeoutCounter > 30000) {
      PRINTLN (F("Timed out waiting for Smart Config to finish!"));
      return -1;
    }
  }

  PRINTLN (F("Enabling auto-connect policy..."));
  if ((rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE))
      != 0) {
    PRINT (F("Setting auto connection policy failed, error: ")); PRINTLN (rval);
    return -1;
  }

  PRINTLN (F("Stopping CC3000..."));
  wlan_stop();  // no error returned here, so nothing to check

  PRINTLN (F("Pausing for 2 seconds..."));
  delay(2000);

  PRINTLN (F("Restarting CC3000... "));
  wlan_start(0);  // no error returned here, so nothing to check

  PRINTLN (F("Waiting for connection to AP..."));
  while (ulCC3000Connected != 1)
    ;

  PRINTLN (F("Waiting for IP address from DHCP..."));
  while (ulCC3000DHCP != 1)
    ;

  PRINTLN (F("Sending mDNS broadcast to signal we're done with Smart Config..."));
  mdnsAdvertiser(1, device_name, strlen(device_name));
  // The API documentation says mdnsAdvertiser()
  // is supposed to return 0 on success and SOC_ERROR on failure, but it looks like
  // what it actually returns is the socket number it used. So we ignore it.

  PRINTLN (F("Smart Config finished!"));

  return 0;
}

void sp_set_connection_policy (byte policy)
{
  byte open_ap = 0;
  byte fast_connect = 0;
  byte use_profiles = 0;

  PRINT ("Connection policy: ");
  PRINTLN (policy);

  if (policy & SP_CONNECT_TO_OPEN_AP)
    open_ap = 1;
  if (policy & SP_FAST_CONNECT)
    fast_connect = 1;
  if (policy & SP_AUTO_START)
    use_profiles = 1;

  wlan_ioctl_set_connection_policy  ((uint32_t)open_ap,
      (uint32_t)fast_connect, (uint32_t)use_profiles);
}
