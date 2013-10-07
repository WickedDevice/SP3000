/*
  CC3000 demo program
  This demo shows how to use Sweet Pea WiFi enabled boards to connect to a 
  TCP server and to retreive data.
  
  The example code works for both the Sweet Pea WiFi Shield as well as the
  Sweet Pea LeFi board.
*/

// By enabling this macro the code is compiled for the Sweet Pea LeoFi board
//#define LEOFI

#ifdef LEOFI
  #define CC3000_MODE      0
  #define CC3000_CS_PIN    6
  #define CC3000_EN_PIN    5
  #define CC3000_IRQ_PIN   7
  #define CC3000_IRQ_LEVEL 4
  #define lSer             Serial
#else
  // The following pin definitions match the default configuration of the 
  // Sweet Pea WiFi shield.
  // Change these pin definitions to match your own configuration.
  #define CC3000_MODE      0
  #define CC3000_CS_PIN    10
  #define CC3000_EN_PIN    7
  #define CC3000_IRQ_PIN   3
  #define CC3000_IRQ_LEVEL 1
  // I normally use a USB to serial cable connected to Serial1 on a Sweet Pea Mega
  // board. You can change it to anything you want.
  #define lSer             Serial
  #define SD_CARD_CS_PIN   4
  #define SRAM_CS_PIN      9
  #define UDP_TEST
#endif

#define LED              13

#include <sp3000.h>
#include <SPI.h>
#ifndef LEOFI
#include <Wire.h>
#endif

// Keeps track of the current state of the network.
byte netStat = 0;

char *netssid = "FarmNet";
char *netkey = "CountlessHours1024";

void setup(void)
{
#ifdef LEOFI
  // Using the LED to indicate that we are connected
  pinMode (LED, OUTPUT);
  digitalWrite (LED, LOW);
#endif

  lSer.begin(115200);
  
#ifndef LEOFI
  // Disable the Sweet Pea WiFi shield on board SPI devices
  digitalWrite(SD_CARD_CS_PIN, HIGH);
  digitalWrite(SRAM_CS_PIN, HIGH);
  pinMode(SD_CARD_CS_PIN, OUTPUT);
  pinMode(SRAM_CS_PIN, OUTPUT);
#endif

  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
#ifndef LEOFI
  // If you are running this example on a Sweet Pea Mega board you should keep
  // pin 53. But if your running it on a Uno or similar board you need to change
  // this pin number to 10.
  pinMode(53, OUTPUT);
#endif
  
#ifdef LEOFI
  // Leonardo needs to wait for a serial connection.
  while (!lSer);
  lSer.println (F("Serial port connected to LeoFfi !"));
#else
  lSer.println (F("Serial port connected to Arduino board !"));
#endif

  // Initialize wlan module
  CC3000_Init (CC3000_MODE, CC3000_CS_PIN, CC3000_EN_PIN, CC3000_IRQ_PIN, CC3000_IRQ_LEVEL);
  lSer.println(F("CC3000 - Init complete."));
  
  // Make sure the module is clean from any previous connections
  wlan_ioctl_set_connection_policy(0, 0, 0);
  wlan_ioctl_del_profile(255);

  // The WiFi module needs a break before connecting.
  delay(500);

  // Now, connect to our network
  wlan_connect(WLAN_SEC_WPA2, (char *)netssid, strlen(netssid), NULL,
                             (unsigned char *)netkey, strlen(netkey));
//
// The main loop will wait for the wifi module to connect and to get an IP address via DHCP
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
    send_F (s, F("GET "));
    send_F (s, F(WEBPAGE));
    send_F (s, F(" HTTP/1.0\r\n"));
    send_F (s, F("Host: "));
    send_F (s, F(WEBSITE));
    send_F (s, F("\n"));
    send_F (s, F("Connection: close\n"));
    send_F (s, F("\n"));

    while (data_available(s)) {
      char c = read_data(s);
      lSer.write(c);
    }
    closesocket(s);
  }
}

