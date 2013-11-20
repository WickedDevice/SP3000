/*
 * http.hpp
 *
 *  Created on: 14 nov 2013
 *      Author: pontus
 */

#ifndef HTTP_HPP_
#define HTTP_HPP_

#include <stdint.h>

enum http_status {
  HTTP_OK,
  HTTP_EOS,
};

int read_http_headers(uint32_t s);
int read_client_http_headers(uint32_t);

#endif /* HTTP_HPP_ */
