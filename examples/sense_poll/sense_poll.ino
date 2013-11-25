//
// Sweet Pea WiFi Shield Demo
//
// This is a very simple sketch that demonstrates how to connect to
// an access point and start getting data from Sen.se
//

#include <sp3000.h>   // The Sweet Pea WiFi Library
#include <SPI.h>      // Required

#if defined(__AVR_ATmega32U4__)  // Pins on LeoFi are fixed
  #define CC3000_MODE      0
  #define CC3000_CS_PIN    6
  #define CC3000_EN_PIN    5
  #define CC3000_IRQ_PIN   7
  #define CC3000_IRQ_LEVEL 4
  #define lSer             Serial
#else
  #define CC3000_MODE      0
  #define CC3000_CS_PIN    10
  #define CC3000_EN_PIN    7
  #define CC3000_IRQ_PIN   3
  #define CC3000_IRQ_LEVEL 1
  #define SD_CARD_CS_PIN   4
  #define SRAM_CS_PIN      9
  #define FEED_ID          <insert your feed id here>
  #define lSer             Serial
#endif

// This specifies how often we should poll sen.se for new data
#define INTERVAL        10000

// Timer for keeping track on when to send new data
uint32_t tRef;
// An array where the reference temperature from sen.se will be stored.
char temperature[16];

//
// In setup we simply initialize the WiFi module and set the connection
// policy. In this example we connect to an open AP and start polling for
// data immediatly.
//
void setup() {
  lSer.begin (115200);
  lSer.println (F("Example on how to retrieve feed data from Sen.se with the Sweet Pea WiFi shield !"));

  // Initialize WiFi module
  sp_wifi_init (CC3000_MODE, CC3000_CS_PIN, CC3000_EN_PIN, CC3000_IRQ_PIN, CC3000_IRQ_LEVEL);
  lSer.println(F("WiFi initialization complete."));

  // Connect to open access point and reconnect if needed.
  sp_set_connection_policy (SP_CONNECT_TO_OPEN_AP | SP_AUTO_START);
  // The Sweet Pea WiFi shield will now try to connect to an open AP
  // The main loop will detect this by checking the status indicators.

  // Set the access key to sen.se. You need to use your own key here
  // Goto sen.se to get one if you haven't already.
  sp_sense_set_key ("0123456789012345678901");
}

// the loop routine runs over and over again forever:
void loop() {
  static byte first = 1;

  // Wait until we have a valid connection and we have received an IP address
  if (ulCC3000Connected && ulCC3000DHCP) {
    if (first) {
      first = 0;
      tRef = millis();
    }
    if (millis() > tRef) {
      lSer.println ("Retrieving");
      tRef = millis() + INTERVAL;
      
      // Simply poll sen.se for new data. In this example I have set up a feed
      // on sen.se holding a reference temperature.
      sp_sense_poll (FEED_ID, (char *)&temperature);
      lSer.print (F("Reference temperature is: "));
      lSer.println (temperature);
    }
  }
}
