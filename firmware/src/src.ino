/*

  Transmit and receive values via RFM69CW radio

  Author: Andy Pomfret

  Forked from openenergymonitor/emonpi, part of the openenergymonitor.org project
  Authors: Glyn Hudson & Trystan Lea
  Builds upon JCW JeeLabs RF12 library and Arduino

  Licence: GNU GPL V3

*/

/*Recommended node ID allocation
------------------------------------------------------------------------------------------------------------
-ID-	-Node Type-
0	- Special allocation in JeeLib RFM12 driver - reserved for OOK use
1-4     - Control nodes
5-10	- Energy monitoring nodes
11-14	--Un-assigned --
15-16	- Base Station & logging nodes
17-30	- Environmental sensing nodes (temperature humidity etc.)
31	- Special allocation in JeeLib RFM12 driver - Node31 can communicate with nodes on any network group
-------------------------------------------------------b------------------------------------------------------

*/

#define emonTxV3                                                      // Tell emonLib this is the emonPi V3 - don't read Vcc assume Vcc = 3.3V as is always the case on emonPi eliates bandgap error and need for calibration http://harizanov.com/2013/09/thoughts-on-avr-adc-accuracy/
#define RF69_COMPAT 1                                                 // Set to 1 if using RFM69CW or 0 is using RFM12B

#include <JeeLib.h>                                                   // https://github.com/openenergymonitor/jeelib
#include <avr/pgmspace.h>
#include <util/parity.h>
// ISR(WDT_vect) { Sleepy::watchdogEvent(); }                            // Attached JeeLib sleep function to Atmega328 watchdog -enables MCU to be put into sleep mode inbetween readings to reduce power consumption
// #FIXME

//----------------------------emonRX Firmware Version---------------------------------------------------------------------------------------------------------------
// Changelog: https://github.com/openenergymonitor/emonpi/blob/master/firmware/readme.md
const int firmware_version = 100;                                     //firmware version x 100 e.g 100 = V1.00

//----------------------------emonPi Settings---------------------------------------------------------------------------------------------------------------
bool debug =                   true;
const unsigned long BAUD_RATE=    38400;

const int TIME_BETWEEN_READINGS=  5000;                             // Time between readings (ms)
const unsigned long RF_RESET_PERIOD=        60000;                            // Time (ms) between RF resets (hack to keep RFM60CW alive)

bool RF_STATUS=                 true;                                  // Turn RF on and off

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------emonPi V3 hard-wired connections---------------------------------------------------------------------------------------------------------------
const byte LEDpin=                     9;              // emonPi LED - on when HIGH
const byte shutdown_switch_pin =       8;              // Push-to-make - Low when pressed
const byte emonpi_GPIO_pin=            5;              // Connected to Pi GPIO 17, used to activate Pi Shutdown when HIGH
//-------------------------------------------------------------------------------------------------------------------------------------------

//-----------------------RFM12B / RFM69CW SETTINGS----------------------------------------------------------------------------------------------------
byte RF_freq=RF12_433MHZ;                                        // Frequency of RF69CW module can be RF12_433MHZ, RF12_868MHZ or RF12_915MHZ. You should use the one matching the module you have.
byte nodeID = 5;                                                 // emonpi node ID
int networkGroup = 210;

//-------------------------------------------------------------------------------------------------------------------------------------------

unsigned long now =0;
unsigned long last_rf_rest=0;                                  // Record time of last RF reset

// RF Global Variables
static byte stack[RF12_MAXDATA+4], top, sendLen, dest;           // RF variables
static char cmd;
static word value;                                               // Used to store serial input
long unsigned int start_press=0;                                 // Record time emonPi shutdown push switch is pressed
bool quiet_mode = true;

const char helpText1[] PROGMEM =                                 // Available Serial Commands
"\n"
"Available commands:\n"
"  <nn> i     - set node IDs (standard node ids are 1..30)\n"
"  <n> b      - set MHz band (4 = 433, 8 = 868, 9 = 915)\n"
"  <nnn> g    - set network group (RFM12 only allows 212, 0 = any)\n"
"  <n> c      - set collect mode (advanced, normally 0)\n"
"  ...,<nn> a - send data packet to node <nn>, request ack\n"
"  ...,<nn> s - send data packet to node <nn>, no ack\n"
"  v          - Show firmware version\n"
"  <n> q      - set quiet mode (1 = don't report bad packets)\n"
;

//-------------------------------------------------------------------------------------------------------------------------------------------
// SETUP ********************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------
void setup()
{

  delay(100);

  emonPi_startup();                                                     // emonPi startup proceadure, check for AC waveform and print out debug
  RF_Setup();
  
  delay(2000);
  
} //end setup


//-------------------------------------------------------------------------------------------------------------------------------------------
// LOOP ********************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------
void loop()
{
  now = millis();

  if (digitalRead(shutdown_switch_pin) == 0 )
    digitalWrite(emonpi_GPIO_pin, HIGH);                                          // if emonPi shutdown butten pressed then send signal to the Pi on GPIO 11
  else
    digitalWrite(emonpi_GPIO_pin, LOW);

  if (Serial.available()){
    handleInput(Serial.read());                                                   // If serial input is received
    double_LED_flash();
  }


  if (RF_Rx_Handle()==1) {                                                      // Returns true if RF packet is received
     double_LED_flash();
  }

  send_RF();                                                                    // Transmitt data packets if needed

  if ((now - last_rf_rest) > RF_RESET_PERIOD) {
    rf12_initialize(nodeID, RF_freq, networkGroup);                             // Periodically reset RFM69CW to keep it alive :-(
    last_rf_rest = now;
  }


} // end loop---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void single_LED_flash()
{
  digitalWrite(LEDpin, HIGH);  delay(50); digitalWrite(LEDpin, LOW);
}

void double_LED_flash()
{
  digitalWrite(LEDpin, HIGH);  delay(25); digitalWrite(LEDpin, LOW);
  digitalWrite(LEDpin, HIGH);  delay(25); digitalWrite(LEDpin, LOW);
}
