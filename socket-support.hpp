/*
 * socket-support.hpp
 *
 *  Created on: 22 sep 2013
 *      Author: pontus
 */

#ifndef SOCKET_SUPPORT_HPP_
#define SOCKET_SUPPORT_HPP_

//
// The size of the tx and rx buffers for the support library
//
#define BUFFER_SIZE    64

#define packIP(a,b,c,d)   ((uint32_t)((uint32_t)a << 24) | \
                            (uint32_t)((uint32_t)b << 16) | \
                            (uint32_t)((uint32_t)c << 8) | (uint32_t)d)

#define rpackIP(a,b,c,d)  ((uint32_t)((uint32_t)d << 24) | \
                            (uint32_t)((uint32_t)c << 16) | \
                            (uint32_t)((uint32_t)b << 8) | (uint32_t)a)

enum sp_connection_policies {
  SP_CONNECT_TO_OPEN_AP = 0x01,
  SP_FAST_CONNECT = 0x02,
  SP_AUTO_START = 0x04
};

//
// Function prototypes
//
int sp_connect (uint32_t destIP, uint16_t destPort, uint32_t type);
int sp_close(int sd);
int sp_send (uint32_t s, const __FlashStringHelper *string);
int sp_send (uint32_t s, long value);
int sp_send (uint32_t s, char *str);

uint8_t data_available(int16_t s);
uint8_t data_available(int16_t s, long secs);
uint8_t data_available(int16_t s, long secs, long usecs);
uint8_t sp_read(int16_t s);
int sp_read_line (int16_t s, char *output, int len);
uint8_t sp_peek(int16_t s);
void scan_ssid(uint32_t time);
char sp_smart_config(void);
void sp_set_connection_policy (byte policy);


#endif /* SOCKET_SUPPORT_HPP_ */
