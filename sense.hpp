/*
 * sense.hpp
 *
 *  Created on: 26 okt 2013
 *      Author: pontus
 */

#ifndef SENSE_HPP_
#define SENSE_HPP_

// The library can parse the json response from sen.se received when POST'ing
// data. This flag disables this functionality, saving a few bytes.
#define USE_POST_PARSING

#define SENSE_KEY_LENGTH    32
#define TIMETAG_LENGTH      29
#define ID_LENGTH           15

/**
 * This enum defines the different errors that the sen.se library can
 * generate.
 */
enum SenseErrors {
  SENSE_OK = 0x00,
  SENSE_ERR_TIMEOUT,          /**< A timeout has occured */
  SENSE_ERR_JSON_ERROR,       /**< An error was detected in the json message */
  SENSE_ERR_SERVER_NOT_FOUND, /**< The sen.se server could not be found */
  SENSE_ERR_NOT_CONNECTED,    /**< Could not connect to the sen.se server */
  SENSE_ERR_PARAMETER,        /**< There was an error with the parameters */
  SENSE_ERR_LAST_ENTRY        // Should always be last
};

/**
 * Data structure for transmitting data to sen.se. Optionally you can also
 * let the library capture the response from sen.se (see USE_POST_PARSING) and
 * store it in this structure.
 */
struct SenseData_s {
  unsigned long feed_id;    /**< The feed ID to post data to */
  char* value;               /**< The data to send */
#if defined(USE_POST_PARSING)
  char timetag[32];          /**< This is where the response timetag gets stored */
  char id[16];               /**< and this is where the event id is stored */
#endif
};
typedef struct SenseData_s SenseData;

/**
 * Sets the sen.se authentication key for all subsequent library calls.
 *
 * @param key A pointer to the key that shall be used.
 *
 * @return Nothing
 */
void sp_sense_set_key (char *key);

/**
 * A function for sending events to sen.se. This function will connect to
 * sen.se and post the supplied data channels. Data is defined in a SenseData
 * structure.
 *
 * @param size This indicates how many entries the structure contains.
 * @param data The data structure.
 *
 * @return A status code indicating if the call was successful or not.
 */
int sp_sense_send (byte size, SenseData* data);

/**
 * Polling function for getting data from a sen.se output feed. This function
 * can be used to get an event from both a device and a feed. If you are
 * polling data from a device you need to specify the device id instead of the
 * feed id and set the is_device parameter to true.
 *
 * @param feed_id The ID of the sen.se feed.
 * @param event_id The event ID of the event that you want.
 * @param output This is where this function stores the result. The caller
 *               must ensure that this array is large enough for the received
 *               data.
 * @param is_device Set this to true if you are pulling data from a device.
 * @param req_type If you set this to true you will be pulling the last data
 *                 entry from the device or feed.
 * @return A status code indicating if the call was successful or not.
 */
int sp_sense_poll (uint32_t feed_id, char *event_id, char *output,
    boolean is_device, boolean req_type);

/**
 * Polling function for getting data from a sen.se output feed. This function
 * retrieves the last event in the feed stream.
 *
 * @param feed_id The ID of the sen.se feed.
 * @param output This is where this function stores the result. The caller
 *               must ensure that this array is large enough for the received
 *               data.
 * @return A status code indicating if the call was successful or not.
 */
int sp_sense_poll (uint32_t feed_id, char *output);

/**
 * Polling function for getting data from a sen.se output feed. This function
 * retrieves a specific event in the feed stream.
 *
 * @param feed_id The ID of the sen.se feed.
 * @param event_id The event ID of the event that you want.
 * @param output This is where this function stores the result. The caller
 *               must ensure that this array is large enough for the received
 *               data.
 * @return A status code indicating if the call was successful or not.
 */
int sp_sense_poll (uint32_t feed_id, char *event_id, char *output);

#endif /* SENSE_HPP_ */
