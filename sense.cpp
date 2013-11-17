/*
 * sense.cpp
 *
 *  Created on: 26 oct 2013
 *      Author: pontus
 */

#include <Arduino.h>

//#define DEBUG
#include "debug.hpp"
#include "settings.hpp"
#include "sense.hpp"
#include "socket.hpp"
#include "socket-support.hpp"
#include "http.hpp"

enum parse_state {
  PARSE_NOT_STARTED = 0x00,
  PARSE_TAG = 0x01,
  PARSE_DATA = 0x02
};

#define PARSE_READ(x)  do { if (data_available(s))     \
                          x = sp_read(s);              \
                        else                            \
                          return -SENSE_ERR_JSON_ERROR; \
                        } while (0)


static char sense_key[32];
static uint32_t sense_IP = 0;

void sp_sense_set_key (char *key)
{
  strncpy (sense_key, key, SENSE_KEY_LENGTH);
}


#if defined(USE_POST_PARSING)
/**
 * Internal function
 *
 * read_and_parse
 *
 * Reads the json stream, parses the json entities and values as well as
 * updates the json data structure.
 *
 * Currently only timetag and id is supported.
 */
static int read_and_parse (uint32_t s, SenseData* data)
{
  byte parse = 0;
  byte parsing = PARSE_NOT_STARTED;
  char data_instance = -1;
  uint16_t jsontag;
  char *wptr;

  while (data_available(s)) {

    char c = sp_read(s);
    switch (c) {
      // Start indicator of json stream
      case '[':
        parse = 1;
        break;

      // Start indicator of data instance
      case '{':
        data_instance++;
        break;

      case '\"':
        if (parsing == PARSE_NOT_STARTED) parsing = PARSE_TAG;
        break;

      case ':':
        // Eat up the following "
        if (data_available(s))
          sp_read(s);
        else
          // If there's no more data something is wrong
          return -SENSE_ERR_JSON_ERROR;
        parsing = PARSE_DATA;
        break;

      // End indicator of data instance
      case '}':
        break;

      // End indicator for json stream
      case ']':
        parse = 0;
        return SENSE_OK;
        break;

      default:
        break;
    }

    // Should we parse ?
    if (parse) {
      if (parsing == PARSE_TAG) {
        jsontag = 0;
        // Read until the end of the " block
        PARSE_READ(c);
        while (data_available(s)) {
          if (c == '\"')
            break;
          jsontag += c;
          PARSE_READ(c);
        }
        PRINT (F(" : Hash "));
        PRINT (jsontag);
      } else if (parsing == PARSE_DATA) {
        switch (jsontag)
        {
          case 747: // timetag
            wptr = (char *)&data[data_instance].timetag;
            break;

          case 205: // id
            wptr = (char *)&data[data_instance].id;
            break;

          case 541: // value
          case 704: // feed_id
          case 1059: // response_id
            wptr = NULL;
            break;

          default:
            wptr = NULL;
            break;
        }
        // Only write data if we have a valid pointer
        if (wptr) {
          PRINT (" - Data ");
          PARSE_READ(c);
          while ((c != '\"') && (c != ',')) {
            *wptr++ = c;
            PARSE_READ(c);
          }
          // Terminate the string
          *wptr = 0;
          parsing = PARSE_NOT_STARTED;
        } else {
          PRINT (" - NO SAVE ");
          // Just skip to next
          PARSE_READ(c);
          while ((c != '\"') && (c != ',')) {
            PARSE_READ(c);
          }
          parsing = PARSE_NOT_STARTED;
        }
        PRINTLN ();
      }
    }
  }
  return SENSE_OK;
}
#endif

//
// The debug server is used internally at Electronic Sweet Peas for developing
// this library.
//
//#define USE_DEBUG_SERVER
#define WEBSITE "api.sen.se"

#ifdef USE_DEBUG_SERVER
#define USE_PORT    8080
#else
#define USE_PORT    80
#endif
//
// This function connects to api.sen.se and returns the socket associated
// with the connection. If something fails during the attempt it returns
// with the appropriate error code.
//

static int sp_sense_connect(void)
{
  int s;
  int res = -1;
  uint16_t port = USE_PORT;
  // Number of resolve retries.
  byte cnt = 10;
#ifdef DEBUG
  char strip[16];
#endif

#ifndef USE_DEBUG_SERVER
  // If IP have not been resolved already go ahead and do that.
  if (!sense_IP) {
    while (res < 0) {
      PRINT (F("Resolving host: "));
      PRINTLN (F(WEBSITE));
      res = gethostbyname(WEBSITE, strlen(WEBSITE), &sense_IP);
      if (res < 0 || sense_IP == 0) {
        res = -1;  // Just in case we received 0.0.0.0
        PRINT (F("Could not resolve "));
        PRINTLN (F(WEBSITE));
        delay (1000);
        if (!--cnt) {
          PRINTLN (F("Really could not resolve"));
          return -SENSE_ERR_SERVER_NOT_FOUND;
        }
      }
    }
  }
#else
  sense_IP = packIP(192,168,0,102);
#endif

#ifdef DEBUG
  sprintf (strip, "%d.%d.%d.%d", (byte)(sense_IP >> 24) & 0xff, (byte)(sense_IP >> 16) & 0xff,
                                 (byte)(sense_IP >> 8)  & 0xff, (byte)(sense_IP & 0xff));
  PRINT (F("Remote host IP address: "));
  PRINTLN (strip);
#endif

  // Try connecting four times before giving up.
  cnt = 4;
  s = -1;

  while (s < 0) {
    s = sp_connect (sense_IP, port, IPPROTO_TCP);
    if (s < 0) {
      PRINT (F("Could not connect !"));
      delay (1000);
      if (!--cnt) {
        PRINTLN (F("\r\nReally could not connect !"));
        return -SENSE_ERR_NOT_CONNECTED;
      }
      PRINTLN (F(" Trying again."));
    }
  }
  PRINTLN (F("Connected to remote server !"));
  // Return socket
  return (s);
}

int sp_sense_send (byte size, SenseData* data)
{
  uint32_t s;
  byte feed_id_size = 0;
  uint16_t tx_data_len = 0;
  int res = -1;
#ifdef DEBUG
  char strip[16];
#endif

  // Sanity check
  if (!size && !data)
    return -SENSE_ERR_PARAMETER;

  // Establish connection to sen.se
  if ((s = sp_sense_connect()) < 0) {
    PRINT (F("Failed to connect to Sen.se !"));
    return s;
  }

  PRINT (F("TX Socket: "));
  PRINTLN (s);

  // Calculate content length
  // TODO: Remove the pesky feed_id_size, why o why
  for (byte i=0; i<size; i++) {
    unsigned long temp = data[i].feed_id;
    feed_id_size = 1;
    // Count the number of digits in feed_id
    while ((temp = temp/10) > 0) feed_id_size++;

    tx_data_len += feed_id_size;
    tx_data_len += strlen(data[i].value);
    tx_data_len += 23;
  }
  tx_data_len += 2;
  tx_data_len += (size - 1);

  PRINTLN (F("SEND: Sending http headers !"));
  PRINT (tx_data_len);
  PRINT (" - ");
  PRINTLN (data->value);

  sp_send (s, F("POST /events/?sense_key="));
  sp_send (s, sense_key);
  sp_send (s, F(" HTTP/1.1\n"));
  sp_send (s, F("Host: api.sen.se\r\n"));
  sp_send (s, F("Content-Type: application/json\r\n"));
  sp_send (s, F("Content-Length: "));
  sp_send (s, tx_data_len);
  sp_send (s, F("\r\nConnection: Keep-Alive"));
  sp_send (s, F("\r\n\r\n"));

  PRINTLN (F("\r\nSending json data !"));
  sp_send (s, F("["));
  // Send groups of data
  for(byte i=0;i<size;i++) {
    sp_send (s, F("{\"feed_id\":"));
    sp_send (s, data[i].feed_id);
    sp_send (s, F(",\"value\":\""));
    sp_send (s, data[i].value);
    sp_send (s, F("\"}"));
    if(size > 1 && i <= (size-2)) {
      sp_send (s, F(","));
    }
  }

  sp_send (s, F("]"));
  sp_send (s, F("\r\n\r\n"));

  PRINTLN (F("Reading http headers !"));
  if ((res = read_http_headers(s)) < 0) {
    PRINT (F("Error "));
    PRINT (res);
    PRINTLN (F(" occurred, terminating sen.se communication !"));
    closesocket(s);
    return res;
  }

#if defined(USE_POST_PARSING)
  // Read and parse the reply
  PRINTLN (F("Read and parse json response !"));
  res = read_and_parse (s, data);
#endif

  PRINTLN (F("Closing socket and leaving !"));
  closesocket(s);

  return res;
}

//
// This is a simple and fault intolerant parser that will look for the third
// " character and then store the argument up to the next " character.
//
static int parse_poll_response (uint32_t s, char *buff)
{
  byte markcnt = 0;
  char c;

  // Make sure we have some data comming
  if (!data_available(s, 5))
    return -SENSE_ERR_TIMEOUT;

  while (data_available(s)) {
    c = sp_read (s);
    if ('\"' == c) {
      if (++markcnt == 3) {
        while (data_available(s)) {
          if ((c = sp_read(s)) == '\"') {
            *buff = 0;
            return SENSE_OK;
          }
          *buff++ = c;
        }
      }
    }
  }
  return -SENSE_ERR_JSON_ERROR;
}

int sp_sense_poll (uint32_t feed_id, char *event_id, char *output,
    boolean is_device, boolean req_type)
{
  uint32_t s;
  int res;

  // Sanity check
  if (!feed_id || !event_id || !output)
    return -SENSE_ERR_PARAMETER;

  // Establish connection to sen.se
  if ((s = sp_sense_connect()) < 0) {
    PRINT (F("Failed to connect to Sen.se !"));
    return s;
  }

  PRINTLN (F("POLL: Sending http headers !"));
  if (is_device)
    sp_send (s, "GET /rt/device/");
  else
    sp_send (s, "GET /rt/feed/");

  sp_send (s, feed_id);
  sp_send (s, "/events?last=");
  sp_send (s, event_id);
  sp_send (s, "&amp;sense_key=");
  sp_send (s, sense_key);
  if (req_type) {
    sp_send (s, "&amp;req_type=last");
  }
  sp_send (s, " HTTP/1.1\n");
  sp_send (s, "Host: api.sen.se\n");
  sp_send (s, "\r\n\r\n");

  res = parse_poll_response (s, output);

  PRINTLN (F("Closing socket and leaving !"));
  closesocket(s);

  return res;
}

int sp_sense_poll (uint32_t feed_id, char *output)
{
  return sp_sense_poll (feed_id, "-1", output, false, true);
}

int sp_sense_poll (uint32_t feed_id, char *event_id, char *output)
{
  return sp_sense_poll (feed_id, event_id, output, false, true);
}
