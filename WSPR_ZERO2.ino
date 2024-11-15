/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - 2024

 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

// Modify the callsign and telemetry channel info in this file
#include "config.h"
#define VHF
#define SEND_INTERVAL 1 // The minimum number of minutes between transmissions


// Variables needed for SI5351 calibration processing
// #define ppsPin  0

volatile bool CalibrationDone = false;
volatile unsigned long SiCnt = 0;
volatile unsigned long mult = 0;
volatile unsigned int tcount = 0;
volatile float correction = 1;
unsigned long freq = (unsigned long)(WSPR_FREQ1); 
volatile int freqCnt;

#include <Wire.h> 
// #include <TimeLib.h>
#ifdef PICO
#include <machine.h>
RTC clock = machine.RTC();
#else
#include <RTCZero.h>
RTCZero clock;
#endif
// #include <avr/interrupt.h>
// #include <avr/io.h>
#include <si5351.h>
#include <JTEncode.h>
#include <TinyGPSPlus.h>

TinyGPSPlus gps;

// Enumerations
enum mode
{
  MODE_JT9,
  MODE_WSPR
};


Si5351 si5351;
JTEncode jtencode;
#include "NIBBBpins.h"
#define MIN_VOLTAGE 2.6

// float tempOutside, pressure; // set once in tempPress.h
int volts = 0;
double gpsAltitude = 0; // Testing value
float gpsSpeed = 0.;   // kmph
float gpsSpeedKnots =0.; // knots per hr
float gpsCourse =0.;
char call_telemetry[7]; // WSPR telemetry callsign
char loc_telemetry[5];  // WSPR telemetry locator
uint8_t dbm_telemetry;  // WSPR telemetry dbm
uint8_t dbm_standard;   // WSPR telemetry dbm

char message1[14] = ""; // Message1 (13 char limit) for JT9
char message2[14] = ""; // Message2 (13 char limit) for JT9

char loc4[5];           // 4 digit grid square locator
char loc4_telemetry[5]; // 4 digit grid square used for the telemetry message
char loc6[7];           // 6 digit grid square locator
char loc8[9];           // 8 digit grid square locator

// byte Hour, Minute, Second; // used for timing
// long lat, lon;             // used for location
// uint8_t tx_buffer[255];            // WSPR Tx buffer
uint8_t tx_buffer[165];
uint8_t symbol_count = WSPR_SYMBOL_COUNT;
uint16_t tone_delay, tone_spacing; // for digital encoding
int satellites = 0;
int alt_meters = 0;
bool telemetry_set = false;
int Sats = 0;
double latitude = 10.;
double longitude = 10.;

// Function prototypes below
void sleep();
void waitForEvenMinute();

#ifdef DEBUG
#define POUTPUT(x) Serial.print x
#else
#define POUTPUT(x)
#endif

#ifdef DEBUG
#define POUTPUTLN(x) Serial.println x
#else
#define POUTPUTLN(x)
#endif
// ** Calibration is not functional in the
//  There is code in this project that can calibrate the SI5351 frequency against the
// highly accurate gps pulse per second (pps).  This requires a pin for the clock and a pin for the pps.
// This was necessary in the past when using commericial
// SI5351 Arduino modules with inexpensive TXCOs
// When CALIBRATION is defined, The calibration code is included in the compile.
//#define CALIBRATION

// COUNTER_PIN is the PA04 of SAMD21 processor and is connected to CLK_CAL of the SI 5351
//  OF THE Si5351.  This depends on the porcessor board
// #define COUNTER_PIN 1 // Use cpu pin 1 for XIOA this corresponds to pin A1.
//#define COUNTER_PIN 18  //  Use cpu pin 18 for MKE Zero this coresponds to pin A3

// #define interruptPinPPS 2 // pin connected to the GPS pps output pin for XIAO
//#define interruptPinPPS 0 // pin connected to the GPS pps output pin for mkr zero

#define SLEEP_PIN LED_BUILTIN // Not used - can be used to turn off system between transmissions see sleep()
#define DBGPIN LED_BUILTIN
#define GPS_SEARCHING LED_BUILTIN

#include "OLED.h"
// #include "./src/TemperatureZero.h" // for reading the cpu internal temperature
// TemperatureZero Temp = TemperatureZero();
#include "./src/Sensors.h"
#include "./src/SI5351Interface.h" // Sends messages using SI5351
#include "./src/GPS.h"             // code to interface with the gps
// #include "./src/SI5351Interface-16QFN.h"
#include "SendMessages.h" // schedules the sending of messages
#ifdef CALIBRATION
#include "./src/FrequencyCorrection.h"
#endif



void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  delay(2000);

  OLEDinit();
  POUTPUTLN(F(" Starting "));


#ifndef DEBUG
  Serial.begin(9600);
  Serial.println(" Debug messages are turned off ");
#endif
  clock.begin();

  // pinMode(SENSOR_PIN, INPUT);
  pinMode(DBGPIN, OUTPUT);
  pinMode(PWR_CTRL, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);

  pinMode(GPS_PWR, OUTPUT);
  digitalWrite(GPS_PWR,HIGH);

  pinMode(GPS_nRESET, OUTPUT);
  digitalWrite(GPS_nRESET, HIGH);

  pinMode(PANEL_VOLTS, INPUT);
  digitalWrite(RF_PWR, LOW);
  digitalWrite(SLEEP_PIN, LOW);
  digitalWrite(DBGPIN, LOW);


  rf_beep();  // Send dashes to test Si5351  - below band by up to 200 Hz
  // Temp.init();
  float cpuTemp = getTempCPU();
  POUTPUT((F(" Temperture ")));
  POUTPUTLN((cpuTemp));

  POUTPUT(F(" Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));
  OLEDrotate(String(" Voltage ")+String(volts), INFO);

  digitalWrite(DBGPIN, HIGH);

  gpsStartTime = millis();
}

void loop()   //*********************  Loop *********************
{
  bool getInfo = gpsGetData();
  if (getInfo == false)
  {
    gps_reset();
    return; // try again
  }
  
//calibrateFreq(); // calibrate the SI5351 frequency
#ifdef CALIBRATION
  // Calibrate Si5351 Xtal
  si5351_calibrate_init();
  pinMode(interruptPinPPS, INPUT_PULLUP);
  tcount = 0;
  // Set 1PPS pin for external interrupt input
  attachInterrupt(digitalPinToInterrupt(interruptPinPPS), PPSinterrupt, RISING);
  Si5351InterruptSetup();
  POUTPUTLN((F(" Waiting for SI5351 calibration to complete ")));
  OLEDrotate(F("Wait for Calibration "), INFO);
  OLEDbeginNoRotate();
  int ical = 14;
  CalibrationDone == false;
  Serial.print(" Init Correction ");
  Serial.println(CalibrationDone);
  while (CalibrationDone == false)
  { 
    #ifdef DEBUG_SI5351
        CalibrationDone = true;
    #endif
    delay(1000);
    ical-- ;
    POUTPUTLN((ical));
    OLEDnoRotate(String(ical),INFO);
    if (ical < 0)
    {
      POUTPUTLN((F(" Error no pps calibration signal ")));
      OLEDrotate(F("Error no PPS signal"),ERROR);
      break;
    }
  }
  Serial.println(tcount);
  Serial.print(" Correction ");
  Serial.println(CalibrationDone);
  if (correction > 1.01 || correction < .99)
  {
    POUTPUTLN((F(" Error calibration count too large or too small ")));
    OLEDrotate(F("Calibration Cnt Wrong"),ERROR);
    correction = 1;
  }
  detachInterrupt(digitalPinToInterrupt(interruptPinPPS)); // Disable the gps pps interrupt

  si5351_calibrate_off();
#else
  CalibrationDone = true;
#endif

  POUTPUTLN((F(" Starting Transmit Logic")));

  SendWSPRMessages();
}
