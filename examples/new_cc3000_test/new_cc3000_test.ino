/*
  Blink
  Turns on an LED on for one second, then off for one second, repeatedly.
 
  This example code is in the public domain.
 */

// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
int led = 13;
byte isInitialized = false;

// the setup routine runs once when you press reset:
#include <sp3000.h>
#include <sweetpea-board.h>
#include <SPI.h>

#define BLINKER_LED	13
#define SD_CARD_CS      4
#define SRAM_CS         9

void setup(void)
{
  setup_sweetpea_board(SWEET_PEA_WIFI_SHIELD);

  Serial1.begin(115200);

  pinMode(BLINKER_LED, OUTPUT);

  SPI.begin();
  SPI.setDataMode(SPI_MODE1);
  SPI.setBitOrder(MSBFIRST);
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  pinMode(53, OUTPUT);
}

void AsyncEventPrint(void)
{
  switch (lastAsyncEvent) 
  {
    case HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE:
      Serial1.println(F("\nCC3000 Async event: Simple config done"));
      break;

    case HCI_EVNT_WLAN_UNSOL_CONNECT:
      Serial1.println(F("\nCC3000 Async event: Unsolicited connect"));
      break;

    case HCI_EVNT_WLAN_UNSOL_DISCONNECT:
      Serial1.println(F("\nCC3000 Async event: Unsolicted disconnect"));
      break;

    case HCI_EVNT_WLAN_UNSOL_DHCP:
      Serial1.print(F("\nCC3000 Async event: Got IP address via DHCP: "));
      Serial1.print(dhcpIPAddress[0]);
      Serial1.print(F("."));
      Serial1.print(dhcpIPAddress[1]);
      Serial1.print(F("."));
      Serial1.print(dhcpIPAddress[2]);
      Serial1.print(F("."));
      Serial1.println(dhcpIPAddress[3]);
      break;

    case HCI_EVENT_CC3000_CAN_SHUT_DOWN:
      Serial1.println(F("\nCC3000 Async event: OK to shut down"));
      break;

    case HCI_EVNT_WLAN_KEEPALIVE:
      // Once initialized, the CC3000 will send these keepalive events
      // every 20 seconds.
      //Serial1.println(F("CC3000 Async event: Keepalive"));
      return;
      break;

    default:
      Serial1.print(F("\nAsyncCallback called with unhandled event! ("));
      Serial1.print(lastAsyncEvent, HEX);
      Serial1.println(F(")"));
      break;
  }
}

void loop(void)
{
  char cmd;

  Serial1.println();
  Serial1.println(F("  ----------------------------------------------"));
  Serial1.println(F("  |  Sweet Pea WiFi Sheild Test Suite V0.11    |"));
  Serial1.println(F("  ----------------------------------------------"));
  Serial1.println(F("  |                                            |"));
  Serial1.println(F("  |  1.  Initialize CC3000 chip and subsystem  |"));
  Serial1.println(F("  |  2.  Search for available AP's             |"));
  Serial1.println(F("  |  3.  Start a Smart Config sequence         |"));
  Serial1.println(F("  |  4.  Manually connect to an access point   |"));
  Serial1.println(F("  |  5.  Manually add a connection profile     |"));
  Serial1.println(F("  |  6.  Network Parameters                    |"));
  Serial1.println(F("  |  7.  Run SPI Test Sequence                 |"));
  Serial1.println(F("  |  8.  Run digitalFastWrite Test Sequence    |"));
  Serial1.println(F("  |  9.  System Information                    |"));
  Serial1.println(F("  |  A.  Set up listening socket               |"));
  Serial1.println(F("  |                                            |"));
  Serial1.println(F("  ----------------------------------------------"));
  Serial1.println();
  Serial1.print(F("              Select (1-9): "));

  for (;;) {
    while (!Serial1.available()) {
      if (asyncNotificationWaiting) {
        asyncNotificationWaiting = false;
        AsyncEventPrint();
      }
    }
    cmd = Serial1.read();
    if (cmd != '\n' && cmd != '\r') {
      break;
    }
  }
  Serial1.println();

  switch (cmd) {
    case '1':
      Initialize();
      break;
    case '9':
      ShowBufferSize();
      ShowFreeRAM();
      break;
    case '3':
      StartSmartConfig();
      break;
    case '4':
      ManualConnect();
      break;
    case '5':
      ManualAddProfile();
      break;
    case '2':
      ListAccessPoints();
      break;
    case '6':
      ShowInformation();
      break;
    case '7':
      spi_test();
      break;
    case '8':
      fastwrite_test();
      break;
    case 'A':
      setup_listening_socket();
      break;
    default:
      break;
  }

}

#define SERVER_PORT 8081

void setup_listening_socket(void)
{
  long s;
  int port;
  short nb = 0;
  long status;
  sockaddr_in sa;
  
  int clientDescriptor = -1;
  sockaddr clientaddr;
  socklen_t addrlen;

  Serial1.println(F("  **********************************************"));
  Serial1.println(F("  *            Starting a TCP server           *"));
  Serial1.println(F("  **********************************************"));

  /* Create a socket handle to use with the listening socket */
  s = socket(AF_INET, SOCK_STREAM, 0);
  if (s < 0) {
    Serial1.println(F("Failed to create socket"));
    return;
  }

  port = SERVER_PORT;

  sa.sin_family = AF_INET;
//  sa.sin_addr.s_addr = INADDR_ANY;
  sa.sin_port = port;

#if 0
  sa.sa_family = AF_INET;
  sa.sa_addr.a_addr = INADDR_ANY;
  sa.sa_data[0] = (port & 0xFF00) >> 8;
  sa.sa_data[1] = (port & 0x00FF);
#endif

//  memset(&sa.sa_data[2], 0, 4);

#if 0
if (bind(s, &sa, sizeof(sockaddr)) != 0) {
    Serial1.println(F("The bind operation failed"));
    return;
  }
#endif

  // Start Listening
  if (listen(s, 1) != 0) {
    Serial1.println(F("Failed to start listening"));
    return;
  }
  addrlen = sizeof (clientaddr);

  Serial1.print(F("Server socket started on port "));
  Serial1.println(port);
  
  clientDescriptor = accept(s, (sockaddr *) &clientaddr, &addrlen);
  
  Serial1.print (F("Accept returned "));
  Serial1.println (clientDescriptor);
}

byte fastwrite_test(void)
{
  byte a;

  Serial1.print(F("Testing digitalFastWrite returns"));

  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  digitalWrite(10, HIGH);
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

  digitalWriteFast(10, 0);
  digitalWriteFast(11, 0);
  digitalWriteFast(12, 0);
  digitalWriteFast(13, 0);
  digitalWriteFast(10, 1);
  digitalWriteFast(11, 1);
  digitalWriteFast(12, 1);
  digitalWriteFast(13, 1);

  digitalWriteFast(10, 0);
  digitalWriteFast(11, 0);
  digitalWriteFast(12, 0);
  digitalWriteFast(13, 0);

  /* Test reading */
  a = digitalReadFast(10);
  a = digitalReadFast(10);
  a = digitalReadFast(10);
  a = digitalReadFast(10);
  a = digitalReadFast(WLAN_IRQ);

  if (a)
    a = 0xff;
  else
    a = 0xaa;

  return a;
}

void spi_test(void)
{
  Serial1.print(F("SPI Test\n"));

  digitalWrite(2, LOW);
  SPI.transfer(0x00);
  digitalWrite(2, HIGH);
}

byte printStep(byte step)
{
  Serial1.print(F("  Step: "));
  Serial1.print(step);
  Serial1.print(F(" - "));
  return step + 1;
}

/* This just shows the compiled size of the transmit & recieve buffers */

void Initialize(void)
{

  unsigned char fancyBuffer[MAC_ADDR_LEN];
  byte step = 1;

  Serial1.println(F("  **********************************************"));
  Serial1.println(F("  *     Initializing CC3000 and subsystem !    *"));
  Serial1.println(F("  **********************************************"));

  if (isInitialized) {
    step = printStep(step);
    Serial1.println(F("  Warning: CC3000 system initialization already performed."));
    Serial1.println(F("           Stopping CC3000 subsystem and reinitializing !"));
    wlan_stop();
    delay(1000);
  }

  step = printStep(step);
  Serial1.println(F("Initializing CC3000..."));
  CC3000_Init (0);
  Serial1.println(F("            CC3000 init complete."));

  step = printStep(step);
  if (nvmem_read_sp_version(fancyBuffer) == 0) {
    Serial1.print(F("Firmware version: "));
    Serial1.print(fancyBuffer[0], DEC);
    Serial1.print(F("."));
    Serial1.println(fancyBuffer[1], DEC);
  } else {
    Serial1.println(F("Error - Unable to get firmware version. Can't continue."));
    return;
  }

  step = printStep(step);
  if (nvmem_get_mac_address(fancyBuffer) == 0) {
    Serial1.print(F("MAC address: "));
    for (byte i = 0; i < MAC_ADDR_LEN; i++) {
      if (i != 0) {
        Serial1.print(F(":"));
      }
      Serial1.print(fancyBuffer[i], HEX);
    }
    Serial1.println();

    isInitialized = true;
  } else {
    Serial1.println(F("Unable to get MAC address. Can't continue."));
  }
}

void ShowBufferSize(void)
{
  Serial1.print(F("Transmit buffer is "));
  Serial1.print(CC3000_TX_BUFFER_SIZE);
  Serial1.println(F(" bytes"));

  Serial1.print(F("Receive buffer is "));
  Serial1.print(CC3000_RX_BUFFER_SIZE);
  Serial1.println(F(" bytes"));
}

int freeRam()
{
  /* This routine from:
   http://www.controllerprojects.com/2011/05/23/determining-sram-usage-on-arduino/
   */

  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void ShowFreeRAM(void)
{
  Serial1.print(F("Free RAM is: "));
  Serial1.print(freeRam());
  Serial1.println(F(" bytes"));
}

void Blinker(void)
{
  digitalWrite(BLINKER_LED, HIGH);
  delay(50);
  digitalWrite(BLINKER_LED, LOW);
  delay(250);
}

/*
 Smart Config is TI's way to let you connect your device to your WiFi network
 without needing a keyboard and display to enter the network name, password,
 etc. You run a little app on your iPhone, Android device, or laptop with Java
 and it sends the config info to the CC3000 automagically, so the end user
 doesn't need to do anything complicated. More details here:

 http://processors.wiki.ti.com/index.php/CC3000_Smart_Config

 This example deletes any currently saved WiFi profiles and goes over the top
 with error checking, so it's easier to see exactly what's going on. You
 probably won't need all of this code for your own Smart Config implementation.

 This example also doesn't use any of the AES enhanced security setup API calls
 because frankly they're weirder than I want to deal with.
 */

// The Simple Config Prefix always needs to be 'TTT'
char simpleConfigPrefix[] = { 'T', 'T', 'T' };

// This is the default Device Name that TI's Smart Config app for iPhone etc. use.
// You can change it to whatever you want, but then your users will need to type
// that name into their phone or tablet when they run Smart Config.
char device_name[] = "CC3000";

void StartSmartConfig(void)
{
  long rval;
  long timeoutCounter;

  if (!isInitialized) {
    Serial1.println(F("CC3000 not initialized; can't run Smart Config."));
    return;
  }

  Serial1.println(F("Starting Smart Config..."));

  Serial1.println(F("  Disabling auto-connect policy..."));
  if ((rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE))
      != 0) {
    Serial1.print(F("    Setting auto connection policy failed, error: "));
    Serial1.println(rval, HEX);
    return;
  }

  Serial1.println(F("  Deleting all existing profiles..."));
  if ((rval = wlan_ioctl_del_profile(255)) != 0) {
    Serial1.print(F("    Deleting all profiles failed, error: "));
    Serial1.println(rval, HEX);
    return;
  }

  Serial1.println(F("  Waiting until disconnected..."));
  while (ulCC3000Connected == 1) {
    Blinker();
  }

  Serial1.println(F("  Setting smart config prefix..."));
  if ((rval = wlan_smart_config_set_prefix(simpleConfigPrefix)) != 0) {
    Serial1.print(F("    Setting smart config prefix failed, error: "));
    Serial1.println(rval, HEX);
    return;
  }

  Serial1.println(F("  Starting smart config..."));
  if ((rval = wlan_smart_config_start(0)) != 0) {
    Serial1.print(F("    Starting smart config failed, error: "));
    Serial1.println(rval, HEX);
    return;
  }

  // Wait for Smartconfig process complete, or 30 seconds, whichever
  // comes first. The Uno isn't seeing the HCI_EVNT_WLAN_ASYNC_SIMPLE_CONFIG_DONE
  // event and I can't figure out why (it works fine on the Teensy) so my
  // temporary workaround is I just stop waiting after a while
  timeoutCounter = millis();
  while (ulSmartConfigFinished == 0) {
    Blinker();
    if (millis() - timeoutCounter > 30000) {
      Serial1.println(F("    Timed out waiting for Smart Config to finish. Hopefully it did anyway"));
      break;
    }
  }

  Serial1.println(F("  Smart Config packet seen!"));

  Serial1.println(F("  Enabling auto-connect policy..."));
  if ((rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE))
      != 0) {
    Serial1.print(F("    Setting auto connection policy failed, error: "));
    Serial1.println(rval, HEX);
    return;
  }

  Serial1.println(F("  Stopping CC3000..."));
  wlan_stop();	// no error returned here, so nothing to check

  Serial1.println(F("  Pausing for 2 seconds..."));
  delay(2000);

  Serial1.println(F("  Restarting CC3000... "));
  wlan_start(0);	// no error returned here, so nothing to check

  Serial1.println(F("  Waiting for connection to AP..."));
  while (ulCC3000Connected != 1) {
    Blinker();
  }

  Serial1.println(F("  Waiting for IP address from DHCP..."));
  while (ulCC3000DHCP != 1) {
    Blinker();
  }

  Serial1.println(F("  Sending mDNS broadcast to signal we're done with Smart Config..."));
  mdnsAdvertiser(1, device_name, strlen(device_name)); // The API documentation says mdnsAdvertiser()
  // is supposed to return 0 on success and SOC_ERROR on failure, but it looks like
  // what it actually returns is the socket number it used. So we ignore it.

  Serial1.println(F("  Smart Config finished!"));
}

/*
 This is an example of how you'd connect the CC3000 to an AP without using
 Smart Config or a stored profile.

 All the code above wlan_connect() is just for this demo program; if you're
 always going to connect to your network this way you wouldn't need it.
 */

void ManualConnect(void)
{

  char ssidName[] = "PotatoTron";
  char AP_KEY[] = "cromulent";
  byte rval;

  if (!isInitialized) {
    Serial1.println(F("CC3000 not initialized; can't run manual connect."));
    return;
  }

  Serial1.println(F("Starting manual connect..."));

  Serial1.println(F("  Disabling auto-connect policy..."));
  rval = wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

  Serial1.println(F("  Deleting all existing profiles..."));
  rval = wlan_ioctl_del_profile(255);

  Serial1.println(F("  Waiting until disconnected..."));
  while (ulCC3000Connected == 1) {
    delay(100);
  }

  Serial1.println(F("  Manually connecting..."));

  // Parameter 1 is the security type: WLAN_SEC_UNSEC, WLAN_SEC_WEP,
  //				WLAN_SEC_WPA or WLAN_SEC_WPA2
  // Parameter 3 is the MAC adddress of the AP. All the TI examples
  //				use NULL. I suppose you would want to specify this
  //				if you were security paranoid.
  rval = wlan_connect(WLAN_SEC_WPA2, ssidName, strlen(ssidName), NULL,
      (unsigned char *) AP_KEY, strlen(AP_KEY));

  if (rval == 0) {
    Serial1.println(F("  Manual connect success."));
  } else {
    Serial1.print(F("  Unusual return value: "));
    Serial1.println(rval);
  }
}

/*
 This is an example of manually adding a WLAN profile to the CC3000. See
 wlan_ioctl_set_connection_policy() for more details of how profiles are
 used but basically there's 7 slots where you can store AP info and if
 the connection policy is set to auto_start then the CC3000 will go
 through its profile table and try to auto-connect to something it knows
 about after it boots up.

 Note the API documentation for wlan_add_profile is wrong. It says it
 returns 0 on success and -1 on failure. What it really returns is
 the stored profile number (0-6, since the CC3000 can store 7) or
 255 on failure.

 Unfortunately the API doesn't give you any way to see how many profiles
 are in use or which profile is stored in which slot, so if you want to
 manage multiple profiles you'll need to do that yourself.
 */

void ManualAddProfile(void)
{
  char ssidName[] = "PotatoTron";
  char AP_KEY[] = "cromulent";

  if (!isInitialized) {
    Serial1.println(F("CC3000 not initialized; can't run manual add profile."));
    return;
  }

  Serial1.println(F("Starting manual add profile..."));

  Serial1.println(F("  Disabling auto connection..."));
  wlan_ioctl_set_connection_policy(DISABLE, DISABLE, DISABLE);

  Serial1.println(F("  Adding profile..."));
  byte rval = wlan_add_profile(WLAN_SEC_WPA2,	// WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
      (unsigned char *) ssidName, strlen(ssidName), NULL,	// BSSID, TI always uses NULL
      0,					// profile priority
      0x18,	// key length for WEP security, undocumented why this needs to be 0x18
      0x1e,				// key index, undocumented why this needs to be 0x1e
      0x2,				// key management, undocumented why this needs to be 2
      (unsigned char *) AP_KEY,	// WPA security key
      strlen(AP_KEY)		// WPA security key length
          );

  if (rval != 255) {

    // This code is lifted from http://e2e.ti.com/support/low_power_rf/f/851/p/180859/672551.aspx;
    // the actual API documentation on wlan_add_profile doesn't specify any of this....

    Serial1.print(F("  Manual add profile success, stored in profile: "));
    Serial1.println(rval, DEC);

    Serial1.println(F("  Enabling auto connection..."));
    wlan_ioctl_set_connection_policy(DISABLE, DISABLE, ENABLE);

    Serial1.println(F("  Stopping CC3000..."));
    wlan_stop();

    Serial1.println(F("  Stopping for 5 seconds..."));
    delay(5000);

    Serial1.println(F("  Restarting CC3000..."));
    wlan_start(0);

    Serial1.println(F("  Manual add profile done!"));

  } else {
    Serial1.print(F("  Manual add profile failured (all profiles full?)."));
  }
}

/*
 The call wlan_ioctl_get_scan_results returns this structure. I couldn't
 find it in the TI library so it's defined here. It's 50 bytes with
 a semi weird arrangement but fortunately it's not as bad as it looks.

 numNetworksFound - 4 bytes - On the first call to wlan_ioctl_get_scan_results
 this will be set to how many APs the CC3000 sees. Although
 with 4 bytes the CC3000 could see 4 billion APs in my testing
 this number was always 20 or less so there's probably an
 internal memory limit.

 results - 4 bytes - 0=aged results, 1=results valid, 2=no results. Why TI
 used 32 bits to store something that could be done in 2,
 and how this field is different than isValid below, is
 a mystery to me so I just igore this field completely.

 isValid & rssi - 1 byte - a packed structure. The top bit (isValid)
 indicates whether or not this structure has valid data,
 the bottom 7 bits (rssi) are the signal strength of this AP.

 securityMode & ssidLength - 1 byte - another packed structure. The top 2
 bits (securityMode) show how the AP is configured:
 0 - open / no security
 1 - WEP
 2 - WPA
 3 - WPA2
 ssidLength is the lower 6 bytes and shows how many characters
 (up to 32) of the ssid_name field are valid

 frameTime - 2 bytes - how long, in seconds, since the CC3000 saw this AP
 beacon

 ssid_name - 32 bytes - The ssid name for this AP. Note that this isn't a
 regular null-terminated C string so you can't use it
 directly with a strcpy() or Serial1.println() etc. and you'll
 need a 33-byte string to store it (32 valid characters +
 null terminator)

 bssid - 6 bytes - the MAC address of this AP
 */

typedef struct scanResults
{
  unsigned long numNetworksFound;
  unsigned long results;
  unsigned isValid :1;
  unsigned rssi :7;
  unsigned securityMode :2;
  unsigned ssidLength :6;
  unsigned short frameTime;
  unsigned char ssid_name[32];
  unsigned char bssid[6];
} scanResults;

#define NUM_CHANNELS	16

unsigned long aiIntervalList[NUM_CHANNELS];

void ListAccessPoints(void) 
{
  byte rval;
  scanResults sr;
  int apCounter;
  char localB[33];
  byte step = 1;
  byte i, j;

  Serial1.println(F("  **********************************************"));
  Serial1.println(F("  *   Search for available wireless networks   *"));
  Serial1.println(F("  **********************************************"));

  if (!isInitialized) {
    step = printStep(step);
    Serial1.println(F("Warning - System not initialized, can't access WiFi Chip."));
    return;
  }

  step = printStep(step);
  Serial1.print(F("Starting to scan for access points"));

  for (int i = 0; i < NUM_CHANNELS; i++) {
    aiIntervalList[i] = 2000;
  }

  rval = wlan_ioctl_set_scan_params(
      1000,      // enable start application scan
      100,       // minimum dwell time on each channel
      200,       // maximum dwell time on each channel
      10,         // number of probe requests
      0x7ff,     // channel mask
      -80,       // RSSI threshold
      0,         // SNR threshold
      205,       // probe TX power
      aiIntervalList	// table of scan intervals per channel
      );
  if (rval != 0) {
    Serial1.println();
    Serial1.print(F("Got back unusual result from wlan_ioctl_set_scan_params, can't continue: "));
    Serial1.println(rval);
    return;
  }

  for (i = 0; i < 10; i++) {
    delay(500);
    Serial1.print(".");
  }

  // On the first call to get_scan_results, sr.numNetworksFound will return the
  // actual # of APs currently seen. Get that # then loop through and print
  // out what's found.

  if ((rval = wlan_ioctl_get_scan_results(2000, (unsigned char *) &sr)) != 0) {
    Serial1.println();
    Serial1.print(F("Got back unusual result from wlan_ioctl_get scan results, can't continue: "));
    Serial1.println(rval);
    return;
  }

  apCounter = sr.numNetworksFound;
  Serial1.println();
  Serial1.println();
  Serial1.print(F("            Number of access points found: "));
  Serial1.println(apCounter);

  Serial1.println();
  Serial1.println(F("              Encrypt   Rssi  SSID"));
  Serial1.println(F("            ----------------------------------------------"));

  do {
    if (sr.isValid) {
      Serial1.print(F("              "));
      switch (sr.securityMode) {
        case WLAN_SEC_UNSEC:	// 0
          Serial1.print(F("OPEN "));
          break;
        case WLAN_SEC_WEP:		// 1
          Serial1.print(F("WEP  "));
          break;
        case WLAN_SEC_WPA:		// 2
          Serial1.print(F("WPA  "));
          break;
        case WLAN_SEC_WPA2:		// 3
          Serial1.print(F("WPA2 "));
          break;
      }
      sprintf(localB, "%3d  ", sr.rssi);
      Serial1.print(F("      "));
      Serial1.print(localB);
      memset(localB, 0, 33);
      memcpy(localB, sr.ssid_name, sr.ssidLength);
      Serial1.println(localB);
    }

    if (--apCounter > 0) {
      if ((rval = wlan_ioctl_get_scan_results(2000, (unsigned char *) &sr))
          != 0) {
        Serial1.print(F("  Got back unusual result from wlan_ioctl_get scan results, can't continue: "));
        Serial1.println(rval);
        return;
      }
    }
  } while (apCounter > 0);
}

void PrintIPBytes(unsigned char *ipBytes)
{
  Serial1.print(ipBytes[3]);
  Serial1.print(F("."));
  Serial1.print(ipBytes[2]);
  Serial1.print(F("."));
  Serial1.print(ipBytes[1]);
  Serial1.print(F("."));
  Serial1.println(ipBytes[0]);
}

/*
 All the data in all the fields from netapp_ipconfig() are reversed,
 e.g. an IP address is read via bytes 3,2,1,0 instead of bytes
 0,1,2,3 and the MAC address is read via bytes 5,4,3,2,1,0 instead
 of 0,1,2,3,4,5.

 N.B. TI is inconsistent here; nvmem_get_mac_address() returns them in
 the right order etc.
 */

void ShowInformation(void)
{

  tNetappIpconfigRetArgs inf;
  char localB[33];

  if (!isInitialized) {
    Serial1.println(F("CC3000 not initialized; can't get information."));
    return;
  }

  Serial1.println(F("CC3000 information:"));

  netapp_ipconfig(&inf);

  Serial1.print(F("  IP address: "));
  PrintIPBytes(inf.aucIP);

  Serial1.print(F("  Subnet mask: "));
  PrintIPBytes(inf.aucSubnetMask);

  Serial1.print(F("  Gateway: "));
  PrintIPBytes(inf.aucDefaultGateway);

  Serial1.print(F("  DHCP server: "));
  PrintIPBytes(inf.aucDHCPServer);

  Serial1.print(F("  DNS server: "));
  PrintIPBytes(inf.aucDNSServer);

  Serial1.print(F("  MAC address: "));
  for (int i = (MAC_ADDR_LEN - 1); i >= 0; i--) {
    if (i != (MAC_ADDR_LEN - 1)) {
      Serial1.print(F(":"));
    }
    Serial1.print(inf.uaMacAddr[i], HEX);
  }
  Serial1.println();

  memset(localB, 0, 32);
  memcpy(localB, inf.uaSSID, 32);

  Serial1.print(F("  Connected to SSID: "));
  Serial1.println(localB);
}

