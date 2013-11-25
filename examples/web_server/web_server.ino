/*
 * This is a simple example on how to implement a webserver on your arduino
 * together with the Sweet Pea WiFi Shield.
 * Note that this example does not handle client RES packets correctly
 * as the current module firmware (1.24) does not report them correctly
 * to the host. As soon as TI fixes this the example will be updated. 
 *
 */

#include <sp3000.h>
#include <SPI.h>

#define CC3000_MODE      0
#define CC3000_CS_PIN    10 
#define CC3000_EN_PIN    7
#define CC3000_IRQ_PIN   3
#define CC3000_IRQ_LEVEL 1
#define SD_CARD_CS_PIN   4
#define SRAM_CS_PIN      9

#define lSer             Serial

// Keeps track of the current connections status
byte wlan_connected = 0;
// This is the port number the server will respond on
uint16_t port = 80;
// Socket that will be used to listen on port 80
int listenSocket;
// Socket that is created by accept when a incoming connection is detected
int newsock;
// Address descriptor for the server
sockaddr_in addr;
// Address descriptor for the client
sockaddr_in clientAddr;
socklen_t addrLen = sizeof(clientAddr);
// A counter for counting the number of incoming connections
uint32_t requests = 0;
// I need a small buffer to work with
char buff[32];

//
// In setup we simply initialize the WiFi module and set the connection
// policy. In this example we connect to an open AP and start listening
// on port 80.
//
void setup(void)
{
  lSer.begin (115200);
  lSer.println (F("Simple webserver example !"));
  // The first thing we do is disable all the peripherals on the WiFi shield
  // that we are not using in this example.
  digitalWrite(SD_CARD_CS_PIN, HIGH);
  digitalWrite(SRAM_CS_PIN, HIGH);
  pinMode(SD_CARD_CS_PIN, OUTPUT);
  pinMode(SRAM_CS_PIN, OUTPUT);

  sp_wifi_init (CC3000_MODE, CC3000_CS_PIN, CC3000_EN_PIN, CC3000_IRQ_PIN, CC3000_IRQ_LEVEL);
  lSer.println(F("WiFi initialization complete."));
  
  // Connect to open access point and reconnect if needed.
  sp_set_connection_policy (SP_CONNECT_TO_OPEN_AP | SP_AUTO_START);
  // The Sweet Pea WiFi shield will now try to connect to an open AP
  // The main loop will detect this by checking the status indicators.
}

void loop(void)
{
  static byte first = 1;
  int res;

  // Wait until we have a valid connection and we have received an IP address
  if (ulCC3000Connected && ulCC3000DHCP) {
    if (first) {
      first = 0;
      wlan_connected = 1;

      // Print received IP address
      lSer.print(F("Got IP address via DHCP: "));
      lSer.print(dhcpIPAddress[0]);
      lSer.print(F("."));
      lSer.print(dhcpIPAddress[1]);
      lSer.print(F("."));
      lSer.print(dhcpIPAddress[2]);
      lSer.print(F("."));
      lSer.println(dhcpIPAddress[3]);

      // Create a listener socket
      listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
      if (listenSocket < 0) {
        lSer.println (F("Failed to created socket !"));
        while (1);
      }
      lSer.print (F("Listening on socket "));
      lSer.println (listenSocket);
      
      memset (&addr, 0, sizeof (addr));
      addr.sin_family = htons(AF_INET);
      addr.sin_port = htons(port);
      
      if (bind(listenSocket, (const sockaddr*)&addr, sizeof(addr)) != 0) {
        lSer.println (F("Failed to bind address to socket !"));
        while (1);        
      }
      if (listen(listenSocket, 10) != 0) {
        lSer.println (F("Failed to connect listener to socket !"));
        while (1);                
      }
    } else if (wlan_connected) {
      newsock = accept(listenSocket, (sockaddr*)&clientAddr, &addrLen);
      if (newsock >= 0) {
        // Make sure we have a http get request
        if (sp_read_line (newsock, buff, 32) > 0) {
          if (strncmp ((char*)&buff, "GET", 3) == 0) {
            if (read_client_http_headers (newsock) >= 0) {
              // Send http response
              sp_send (newsock, F("HTTP/1.1 200 OK\n"));
              sp_send (newsock, F("Server: Sweet Peas WiFi Server\r\n"));
              sp_send (newsock, F("Content-Type: text/html"));
              sp_send (newsock, F("\r\n\r\n"));
              // And some data to show of
              sp_send (newsock, F("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" "));
              sp_send (newsock, F("\"http://www.w3.org/TR/html4/strict.dtd\"><html>\n"));
              sp_send (newsock, F("<head><title>Sweet Pea WiFi Shield Demo</title></head><body>\n"));
              sp_send (newsock, F("<meta http-equiv=\"refresh\" content=\"5\">"));
              for (int i=0;i<5;i++)
                sp_send (newsock, F("This should be visible now !<br>"));
              sp_send (newsock, F("</body></html>"));
              lSer.print (F("Number of requests made: "));
              lSer.print (++requests);
            }
          } else lSer.println (F("No GET found"));
        } else lSer.println (F("Could not read line"));
        sp_close (newsock);
      }
    }
  } else {
    if (wlan_connected) {
      if (!ulCC3000Connected) {
        wlan_connected = 0;
        lSer.print (F("WLAN Disconnected "));
      }
    }
  }
}