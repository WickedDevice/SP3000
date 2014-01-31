//
// Sweet Pea WiFi Shield Demo 01
//
// This is a very simple scetch that demonstrates how to connect to
// an access point and start sending data to Sen.se
//

#include <sp3000.h>   // The Sweet Pea WiFi Library
#include <SPI.h>      // Required
#include <WildFire.h>
WildFire wf;

#define CC3000_MODE      0
#define lSer             Serial

// This specifies how often we should send data
#define INTERVAL        10000

// This array will hold the data that we send to Sen.se
char sendat[32];
// Create the Sen.se data structure that we need to send data to Sen.se
SenseData sd[1] = {
  XXXXX,    // This is the Sen.se Feed ID for your data channel
  sendat    // Point out where the data to send is located
};

// Timer for keeping track on when to send new data
uint32_t tRef;

//
// In setup we simply initialize the WiFi module and set the connection
// policy. In this example we connect to an open AP and start sending
// data immediatly.
//
void setup() {
  wf.begin();
  
  lSer.begin (115200);
  lSer.println (F("Example on how to publish sensor data to Sen.se with the Sweet Pea WiFi shield !"));

  // Initialize WiFi module
  sp_wifi_init (CC3000_MODE);
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
      lSer.println ("Sending");
      tRef = millis() + INTERVAL;
      // Prepare the data that we want to send.
      // This can be any data of course, in this example I'm just using a fixed
      // string to represent a temperature.
      sprintf (sd[0].value, "19.25");
      sp_sense_send (1, sd);
    }
  }
}
