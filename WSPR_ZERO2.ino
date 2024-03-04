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
// volatile unsigned long XtalFreq=25000000UL;
volatile int32_t FreqCorrection_ppb = 0;
volatile float correction = 1;
unsigned long freq = (unsigned long)(WSPR_FREQ1);

#include <Wire.h> // For aprs
// #include <TimeLib.h>
#include <RTCZero.h>
// #include <avr/interrupt.h>
// #include <avr/io.h>
// #include "src/si5351-16QFN.h"
#include <si5351.h>
#include <JTEncode.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;

// Enumerations
enum mode
{
  MODE_JT9,
  MODE_WSPR
};

Si5351 si5351;
JTEncode jtencode;
RTCZero clock;

#define MIN_VOLTAGE 2.6

// float tempOutside, pressure; // set once in tempPress.h
int volts = 0;
double gpsAltitude = 0; // Testing value
double gpsSpeed = 0.;   // Testing values
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
int gps_speed = 0;
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
// #define CALIBRATION

// COUNTER_PIN is the PA04 of SAMD21 processor and is connected to CLK_CAL of the SI 5351
//  OF THE Si5351.  This depends on the porcessor board
#define COUNTER_PIN 1 // Use cpu pin 1 for XIOA this corresponds to pin A1.
// #define COUNTER_PIN 18  //  Use cpu pin 18 for MKE Zero this coresponds to pin A3

#define interruptPinPPS 2 // pin connected to the GPS pps output pin.

#define RANDOM_PIN 0 // used to generate a seed for the random number gerator
#define RFPIN 13     // Can be used to turn off si5351 see rf_off() and sleep()
#define SLEEP_PIN 13 // Not used - can be used to turn off system between transmissions see sleep()
#define DBGPIN 13
#define SENSOR_PIN0 0 // Generic analog sensor -- can be changed to any unused pin
#define SENSOR_PIN1 0
#define GPS_SEARCHING 13
#define INPUT_VOLTAGE 2 // measures solar panel voltage
#define GPS_POWER 13    // Pull down to turn on GPS module (not used) see sleep()
static const uint32_t GPSBaud = 9600;

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

// gps must lock position within 15 minutes or system will sleep or use the default location if the clock was set
const unsigned long gpsTimeout = 900000; // in milliseconds
unsigned long gpsStartTime = 0;

void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
  delay(1000);
  POUTPUT(F(" Starting "));

#ifndef DEBUG
  Serial.begin(9600);
  Serial.println(" Debug messages are turned off ");
#endif
  clock.begin();
  //HF_init();  
  // for (int j = 1; j<5;++j)
  //   {
  //     beep();
  //     delay(3000);
  //   }

  // pinMode(SENSOR_PIN, INPUT);
  pinMode(RFPIN, OUTPUT);
  pinMode(SLEEP_PIN, OUTPUT);
  pinMode(GPS_POWER, OUTPUT);
  pinMode(INPUT_VOLTAGE, INPUT);
  digitalWrite(RFPIN, LOW);
  digitalWrite(SLEEP_PIN, LOW);
#ifdef CALIBRATION
  si5351_calibrate_init();
#endif
  // Temp.init();
  // float cpuTemp = Temp.readInternalTemperature();
  float cpuTemp = 0;
  POUTPUT((F(" Temperture ")));
  POUTPUTLN((cpuTemp));

  POUTPUT(F(" Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));

  digitalWrite(DBGPIN, HIGH);

  gpsStartTime = millis();
}

long loopi = 0;
int loopj = 0;
bool rfpinon = false;
void loop()
{
  bool getInfo = gpsGetInfo();
  if (getInfo == false)
  {
    gps_reset();
    return; // try again
  }

#ifdef CALIBRATION
  // Calibrate Si5351 Xtal
  pinMode(interruptPinPPS, INPUT_PULLUP);
  // Set 1PPS pin for external interrupt input
  attachInterrupt(digitalPinToInterrupt(interruptPinPPS), PPSinterrupt, RISING);
  Si5351InterruptSetup();
  POUTPUTLN((F(" Waiting for SI5351 calibration to complete ")));
  while (CalibrationDone == false)
  {
#ifdef DEBUG_SI5351
    CalibrationDone = true;
#endif
    delay(1000);
    POUTPUTLN((tcount));
  }
  detachInterrupt(digitalPinToInterrupt(interruptPinPPS)); // Disable the gps pps interrupt
#else
  CalibrationDone = true;
#endif

  POUTPUTLN((F(" Starting Transmit Logic")));

  SendMessages();
}

bool gpsGetInfo()
{
  bool clockSet = false, locSet = false, altitudeSet = false, speedSet = false;
  Serial1.begin(GPSBaud);
  delay(100);
  gpsStartTime = millis();
  bool hiAltitudeSet = false;
  POUTPUTLN((F("Waiting for GPS to find satellites - 5-10 min")));
  while (millis() < gpsStartTime + gpsTimeout)
  {
    // wdt_reset();
    while (Serial1.available() > 0)
      gps.encode(Serial1.read());

    if (gps.charsProcessed() > 10 && hiAltitudeSet == false)
    { // put the gps module into high altitude mode
      SetHighAltitude();
      // ss.write("$PMTK886,3*2B\r\n");
      hiAltitudeSet = true;
    }

    if (gps.time.isUpdated() && gps.satellites.value() > 0 && clockSet == false)
    {
      clockSet = true;
    }
    if (gps.altitude.isUpdated())
    {
      gpsAltitude = gps.altitude.meters();
      altitudeSet = true;
    }
    if (gps.speed.isUpdated())
    {
      gpsSpeed = gps.speed.kmph();
      speedSet = true;
    }
    if (gps.location.isUpdated())
    {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      locSet = true;
    }

    if (locSet && speedSet && altitudeSet && clockSet)
    {
      satellites = gps.satellites.value();
      POUTPUT((F(" Number of satellites found ")));
      POUTPUTLN((satellites));
      SetCPUClock(gps);
      // start transmission loop
      return true;
    }

    loopi++;

    if (loopi % 5000 == 0)
      beep(); // still looking for satellites

    if (gps.charsProcessed() < 10 && millis() % 1500 < 5)
    {

      POUTPUTLN((F("WARNING: No GPS data.  Check wiring.")));
      // blink moris code "w" for wiring
      digitalWrite(DBGPIN, LOW);
      delay(50);
      digitalWrite(DBGPIN, HIGH);
      delay(50);
      digitalWrite(DBGPIN, LOW);
      delay(50);
      digitalWrite(DBGPIN, HIGH);
      delay(150);
      digitalWrite(DBGPIN, LOW);
      delay(50);
      digitalWrite(DBGPIN, HIGH);
      delay(150);
      digitalWrite(DBGPIN, LOW);
    }
// If DEBUG_SI5351 is defined, the system will transmit, but not on the correct minute
// Use this for unit testing when there is no gps attached.
#ifdef DEBUG_SI5351
    return true;
#endif
  }
  POUTPUTLN((F(" GPS Timeout - no satellites found ")));

  return false;
  /*  if(clockSet==true)
   {
     // Send report anyway if only the clock has been set
     clockSet = false; // needed for testing only
     locSet = false;
     altitudeSet = false;
     speedSet = false;
     return true;
   }
   else
   {
     return false;
   } */
}
