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
#define BUFFER_SIZE    32

#define packIP(a,b,c,d)   ((uint32_t)((uint32_t)a << 24) | \
                            (uint32_t)((uint32_t)b << 16) | \
                            (uint32_t)((uint32_t)c << 8) | (uint32_t)d)

//
// Function prototypes
//
int32_t sp_connect (uint32_t destIP, uint16_t destPort, uint32_t type);
size_t send_F(uint32_t s, const __FlashStringHelper *string);
uint8_t data_available(int16_t s);
uint8_t read_data(int16_t s);

#endif /* SOCKET_SUPPORT_HPP_ */
