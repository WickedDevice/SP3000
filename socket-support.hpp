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

enum sp_socket_errors {
  SP_SOCK_OK = 0x00,            /**< Return code for a successful socket op */
  SP_SOCK_CREATE_FAILED,        /**< Failed to create a socket */
  SP_SOCK_BIND_FAILED,          /**< Failed to bind a socket */
  SP_SOCK_LISTEN_FAILED,        /**< Failed to create a listener */
  SP_SOCK_LAST_ENTRY,
};
/**
 * Connection policies
 */
enum sp_connection_policies {
  SP_CONNECT_TO_OPEN_AP = 0x01, /**< Connect to any open AP available */
  SP_FAST_CONNECT = 0x02,       /**< Use previous connection */
  SP_AUTO_START = 0x04          /**< Autostart  */
};

/**
 * Scan values
 */
enum sp_scan_ssid {
  SP_SCAN_STOP  = 0x00,         /**< Stop the device from scanning */
  SP_SCAN_SSID_10 = 0x01        /**< Use default 10 min scanning period */
};

//
// Function prototypes
//
/**
 * A support function that simplifies connecting to other hosts. Simply forward
 * the IP address, the destination port and the connection type (UDP/TCP) to
 * this function and it will return the socket associated with this connection.
 * The method returns -1 if something goes wrong during the connection attempt.
 *
 * @param destIP The IP address of the host you are attempting to connect to.
 *               Use the macro packIP to set the address or the result from
 *               resolvehost.
 * @param destPort The destination port of the host.
 * @param type This must be set to either IPPROTO_TCP or IPPROTO_UDP depending
 *               on what connection method you are using.
 *
 * @return Returns a socket descriptor if the call was successfull, otherwise
 *               it returns -1.
 */
int sp_connect (uint32_t destIP, uint16_t destPort, uint32_t type);

/**
 * Closes a socket. This function differs from the closesocket(sd) method in
 * that it will ensure that all data have been sent from the CC3000 module
 * before it closes any sockets. This is crucial if you are closing a socket
 * immediatly after sending many packets which might not have been sent out
 * yet.
 *
 * @param sd The socket descriptor of the socket you want to close.
 *
 * @return Returns 0 if successful otherwise it returns -1.
 */
int sp_close(int sd);

/**
 * This method sends a string that is stored in the flash memory of an AVR
 * device. The function must be called like this:
 *
 *   sp_send (socket, F("Some string));
 *
 * @param sd The socket descriptor of the socket associated with the write.
 *
 * @return Returns 0 if successful otherwise it returns -1.
 */
int sp_send (uint32_t sd, const __FlashStringHelper *string);

/**
 * This method sends a long value as a character based number.
 * The function must be called like this:
 *
 *   sp_send (socket, (long)value);
 *
 * @param sd The socket descriptor of the socket associated with the write.
 *
 * @return Returns 0 if successful otherwise it returns -1.
 */
int sp_send (uint32_t sd, long value);

/**
 * This method sends a ram based character string.
 * The function must be called like this:
 *
 *   sp_send (socket, (char *)string);
 *
 * @param sd The socket descriptor of the socket associated with the write.
 *
 * @return Returns 0 if successful otherwise it returns -1.
 */
int sp_send (uint32_t sd, char *str);

/**
 * This method uses select to see if there is any data available on the
 * associated socket. It will wait up to the specified number of seconds and
 * micro seconds before returning. If there is data available it will return
 * with 1, otherwise 0 will be returned.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 * @param secs The number of seconds to wait for data before returning.
 * @param usecs The number of micro seconds to wait for data before returning.
 *
 * @return Returns 1 if there is data available otherwise it will return 0.
 */
uint8_t data_available(int16_t sd, long secs, long usecs);

/**
 * This method uses select to see if there is any data available on the
 * associated socket. It will wait up to the specified number of seconds
 * before returning. If there is data available it will return with 1,
 * otherwise 0 will be returned.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 * @param secs The number of seconds to wait for data before returning.
 *
 * @return Returns 1 if there is data available otherwise it will return 0.
 */
uint8_t data_available(int16_t sd, long secs);

/**
 * This method uses select to see if there is any data available on the
 * associated socket. It will wait up to 1 second before returning.
 * If there is data available it will return with 1, otherwise 0 will be
 * returned.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 *
 * @return Returns 1 if there is data available otherwise it will return 0.
 */
uint8_t data_available(int16_t sd);

/**
 * Peeks into the socket stream without advancing the read pointer. The caller
 * must ensure that there is data available to read from the socket by calling
 * the appropriate data_available method before calling sp_read.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 *
 * @return Returns the data from the socket.
 *
 * @see data_available
 */
uint8_t sp_peek(int16_t sd);

/**
 * A method for reading one byte from a socket. The caller must ensure that
 * there is data available to read from the socket by calling the appropriate
 * data_available method before calling sp_read.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 *
 * @return Returns the data from the socket.
 *
 * @see data_available
 */
uint8_t sp_read(int16_t sd);

/**
 * A method for reading one full line from a socket. Note that it is not
 * necessary for the caller to ensure that there is data available prior to
 * calling this method. It uses its own select method and will return with
 * a negative status code if there is no data available.
 *
 * @param sd The socket descriptor of the socket associated with the data to
 *           be read.
 * @param output An array where the result will be stored.
 * @param len The length of the array.
 *
 * @return Returns the line from the socket or -1 if there is no data available.
 *
 * @see sp_read, sp_peek
 */
int sp_read_line (int16_t sd, char *output, int len);

/**
 * Creates a listening socket on the specified port. This method will create
 * a socket, bind it to the local address and the specified port and then
 * start a listening instance. The listener socket is returned to the caller.
 *
 * @param port The port associated with the listener
 * @param blocking Indicates whether the associated socket should block
 *                 subsequent calls to accept or not. 0 = non blocking, 1 =
 *                 blocking.
 *
 * @return Returns the socket associated with the listener. If an error is
 *         detected during the setup of the listener this error is returns
 *         as a negative value.
 *
 * @see sp_socket_errors
 */
int sp_create_listener (uint16_t port, boolean blocking);

/**
 * To be called when an ssid scan is required. Calling this method with
 * SP_SCAN_SSID_10 will start the CC3000 scanning for networks every
 * 10 minutes. using SP_SCAN_STOP will stop the module from performing
 * any further scans. Using any other value will set the scan period to
 * that value in milliseconds.
 *
 * @param time The time in milliseconds to scan for ssid's.
 *
 * @return Returns 0 if the call was successfull or -1 if it failed for some
 *         reason.
 *
 */
int sp_scan_ssid(uint32_t time);

/**
 * This function performs a complete smart configuration sequence. The method
 * will wait for a smartconfig packet to arrive, then continue to wait for
 * a valid connection and a DHCP negotiation to be finished. It will then
 * advertise itself on the network to indicate that the process is done.
 *
 * @return Returns 0 if the call was successful or -1 if it failed for some
 *         reason.
 *
 * @see sp_scan_ssid
 *
 */
int sp_smart_config(void);

/**
 * This function sets the connection policy that shall be used.
 *
 * @param policy The policy that shall be used
 *
 * @see sp_connection_policies
 */
void sp_set_connection_policy (int policy);


#endif /* SOCKET_SUPPORT_HPP_ */
