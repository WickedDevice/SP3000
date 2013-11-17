/*
 * http.cpp
 *
 *  Created on: 14 nov 2013
 *      Author: pontus
 */
#include <Arduino.h>
#include "http.hpp"
#include "socket-support.hpp"
#include "sense.hpp"

#include "debug.hpp"

static const char *HTTP_200_OK = "HTTP/1.1 200 OK";
static char buff[16];

//*****************************************************************************
//*
//* Description:
//*   A reasonably fast pattern matching algorithm for finding strings
//*   on the incoming data stream.
//*
//*****************************************************************************
char sp_http_skip_to (int s, char *to)
{
  char c;
  byte clen, len;
  byte i;

  clen = len = strlen (to) - 1;

  while (data_available(s)) {
    // Get next character in stream
    c = sp_read (s);
    // Check if it matches the first character in the match string
    if (c == *to) {
      i = 1;
      len = clen;
      // Now we will start checking the rest of the characters
      while (len-- && data_available(s)) {
        if (sp_peek (s) == *(to + i++)) {
          // If we had a match, consume this character
          c = sp_read(s);
          if (!len) {
            return HTTP_OK;
          }
        } else
          break;
      }
    }
  }
  return -HTTP_EOS;
}

//*****************************************************************************
//*
//* Description:
//*   Reads the socket stream until a sequence of CR/LF/CR/LF is found.
//*   If an HTTP error is found it is returned to the caller as a negative
//*   number.
//*
//*****************************************************************************
int read_http_headers(uint32_t s)
{
  byte i;

  // First make sure there is data available, wait up to 5 seconds if necessary
  if (!data_available(s, 5)) {
    return -SENSE_ERR_TIMEOUT;
  }

  // Read the first 15 character from the socket. It must have a 200 response
  // code, otherwise we return with an error.
  PRINTLN (F("Reading HTTP response"));
  for (i=0; i<15; i++) {
    if (data_available(s))
      buff[i] = sp_read(s);
    else
      return -SENSE_ERR_TIMEOUT;
  }
  buff[i] = 0;
  PRINTLN (buff);

  if (strncmp (buff, HTTP_200_OK, 15) != 0) {
    // Parse out the error code and return it
    int error = atoi (&buff[9]);
    // Return with HTTP error
    return (error * -1);
  }

  // Look for the end of the http headers.
  sp_http_skip_to (s, "\r\n\r\n");

  return 0;
}
