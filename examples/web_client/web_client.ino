/*
  CC3000 demo program
  This demo shows how to use Sweet Pea WiFi enabled boards to connect to a 
  TCP server and to retreive data.
  
  The example code works for both the Sweet Pea WiFi Shield as well as the
  Sweet Pea LeFi board.
*/

#include <sp3000.h>
#include <SPI.h>
#include <WildFire.h>
WildFire wf;

#define CC3000_MODE      0
#define lSer             Serial

// Keeps track of the current state of the network.
byte netStat = 0;

char *netssid = "FarmNet";
char *netkey = "CountlessHours1024";

void setup(void)
{
  wf.begin();

  lSer.begin(115200);

  lSer.println (F("\n\nElectronic Sweet Peas Demonstration program !"));
  lSer.println (F("Visit http://www.sweetpeas.se for more information !\n"));   
  lSer.println (F("Serial port connected to Arduino board !"));

  // Initialize wlan module
  sp_wifi_init (CC3000_MODE);
  lSer.println(F("CC3000 - Init complete."));
  
  // Make sure the module is clean from any previous connections
  wlan_ioctl_set_connection_policy(0, 0, 0);
  wlan_ioctl_del_profile(255);

  // The WiFi module needs a break before connecting.
  delay(500);

  // Now, connect to our network
  wlan_connect(WLAN_SEC_WPA2, (char *)netssid, strlen(netssid), NULL,
                             (unsigned char *)netkey, strlen(netkey));

// The main loop will wait for the wifi module to connect and to get an IP 
// address via DHCP
}

void loop(void)
{
  // Wait for something to happen on the network
  if (asyncNotificationWaiting) {
    asyncNotificationWaiting = false;
      AsyncEventPrint();
    if (netStat) {
      client_connect();
      netStat = 0;
    }
  }
}

void AsyncEventPrint(void)
{
  switch (lastAsyncEvent) 
  {
    case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
      lSer.println(F("\nCC3000 - Simple config done"));
      break;

    case HCI_EVNT_WLAN_UNSOL_CONNECT:
      lSer.println(F("CC3000 - Unsolicited connect"));
      break;

    case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
      lSer.println(F("CC3000 - Unsolicted disconnect"));
      break;

    case HCI_EVNT_WLAN_UNSOL_DHCP:
      lSer.print(F("CC3000 - Got IP address via DHCP: "));
      lSer.print(dhcpIPAddress[0]);
      lSer.print(F("."));
      lSer.print(dhcpIPAddress[1]);
      lSer.print(F("."));
      lSer.print(dhcpIPAddress[2]);
      lSer.print(F("."));
      lSer.println(dhcpIPAddress[3]);
      digitalWrite (LED, HIGH);
      netStat = 1;
      break;

    case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
      lSer.println(F("\nCC3000 - OK to shut down"));
      break;

    case HCI_EVNT_WLAN_KEEPALIVE:
      // Once initialized, the CC3000 will send these keepalive events
      // every 20 seconds.
      //lSer.println(F("\nCC3000 - Keepalive"));
      return;
      break;

    default:
      lSer.print(F("\nCC3000 - Unknown event! ("));
      lSer.print(lastAsyncEvent, HEX);
      lSer.println(F(")"));
      break;
  }
}

#define WEBSITE      "www.sweetpeas.se"
#define WEBPAGE      "testdata/test.txt"
void client_connect(void)
{
  uint32_t s;
  uint32_t IP;
  uint16_t port = 80;

  if (gethostbyname(WEBSITE, strlen(WEBSITE), &IP) < 0) {
    lSer.print (F("Could not resolve "));
    lSer.println (F(WEBSITE));
    return;
  }
  
  s = sp_connect(IP, port, IPPROTO_TCP);
  if (s < 0) {
    lSer.println (F("Error while trying to connect to TCP port on remote server !"));
  } else {
    lSer.println (F("Successfully connected !!!"));
    
    // Make a http get request
    sp_send (s, F("GET "));
    sp_send (s, F(WEBPAGE));
    sp_send (s, F(" HTTP/1.0\r\n"));
    sp_send (s, F("Host: "));
    sp_send (s, F(WEBSITE));
    sp_send (s, F("\n"));
    sp_send (s, F("Connection: close\n"));
    sp_send (s, F("\n"));

    while (data_available(s)) {
      char c = sp_read(s);
      lSer.write(c);
    }
    sp_close (s);
  }
}

