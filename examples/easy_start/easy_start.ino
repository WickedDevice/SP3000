/*
 * This simple sketch will simply print out the host driver version
 * as well as module firmware version and then try to connect to 
 * an open access point. It also demonstrates how to register a call back
 * function in your sketch to the WiFi framework.
 */

#include <sp3000.h>
#include <SPI.h>
#include <WildFire.h>
WildFire wf;

#define CC3000_MODE      0
#define lSer             Serial


// Keeps track of the current connections status
byte wlan_connected = 0;
// Firmware version buffer
unsigned char fw[2];
// MAC address buffer
unsigned char mac[6];
// A generic work buffer 
char buff[32];

/*
 * This is our WLAN event handler.
 * This function is registered with the WLAN framework using the
 * sp_core_register_event_cb (EventHandler) call further down in the
 * setup function.
 */
void EventHandler (uint32_t eventtype, char *data, uint8_t len)
{
  switch (eventtype)
  {
    case HCI_EVNT_WLAN_KEEPALIVE:
      lSer.println (F("Ping !"));
      break;
      
    default:
      lSer.println (F("Unhandled event !"));
      break;    
  }
}

//
// In setup we simply initialize the WiFi module, print out some vital
// information, set the connection policy and register the event handler.
// The module will then connect to an open access point and print out the
// obtained IP address.
//
void setup(void)
{
  wf.begin();
  lSer.begin (115200);

  lSer.println (F("\n\nElectronic Sweet Peas Demonstration program !"));
  lSer.println (F("Visit http://www.sweetpeas.se for more information !\n"));

  lSer.print(F("Initializing"));
  sp_wifi_init (CC3000_MODE);
  lSer.println(F(" complete."));

  lSer.print (F("Host driver version: "));
  lSer.print (DRIVER_VERSION_NUMBER / 10, DEC);
  lSer.write ('.');
  lSer.println (DRIVER_VERSION_NUMBER - ((DRIVER_VERSION_NUMBER / 10) * 10), DEC);
  
  if (!nvmem_read_sp_version(&fw[0]) == 0) {
    lSer.println (F("Error - Unable to get firmware version."));
    while (1);
  }
  lSer.print (F("Firmware version: "));
  lSer.print (fw[0], DEC);
  lSer.write ('.');
  lSer.println (fw[1], DEC);

  if (!nvmem_get_mac_address(&mac[0]) == 0) {
    lSer.println (F("Error - Unable to get MAC address."));
    while (1);
  }
  lSer.print (F("MAC address: "));
  sprintf (buff, "%02x:%02x:%02x:%02x:%02x:%02x", 
    mac[0], mac[1], mac[2], mac[3], mac[4], mac[0]);
  lSer.println(buff);
  
  // Connect to open access point and reconnect if needed.
  sp_set_connection_policy (SP_CONNECT_TO_OPEN_AP | SP_AUTO_START);
  // The Sweet Pea WiFi shield will now try to connect to an open AP
  // The main loop will detect this by checking the status indicators.
  
  // Connect our event handler to the wlan core
  sp_core_register_event_cb (EventHandler);
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
    }
  } else {
    if (wlan_connected) {
      if (!ulCC3000Connected) {
        wlan_connected = 0;
        lSer.println (F("Disconnected !"));
      }
    }
  }
}
