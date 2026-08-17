/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019
   Modified and enhanced by K9YO Chicago IL 2019 - 2025

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
#include "pins.h" // PIN and Clock deffinitions

#define VHF
#define SEND_INTERVAL 1 // The minimum number of minutes between transmissions

// Variables needed for SI5351 calibration processing
// #define ppsPin  0

volatile bool CalibrationDone = false;
volatile unsigned long SiCnt = 0;
volatile unsigned long zeroCount = 0;
volatile unsigned int tcount = 0;
volatile float correction = 1;
unsigned long freq = (unsigned long)(WSPR_FREQ1);
volatile int freqCnt;

#include <Wire.h>


#ifdef PICO
#include <TimeLib.h>
bool setSDA(4);
bool setSCL(5);
#else
#include <RTCZero.h>
RTCZero clock;
#endif
//Wire.begin();
// #include <avr/interrupt.h>
// #include <avr/io.h>

#include <TinyGPSPlus.h>

TinyGPSPlus gps;

#define MIN_VOLTAGE 2.6

// float tempOutside, pressure; // set once in tempPress.h
int volts = 0;
double gpsAltitude = 0; 
float gpsSpeed = 0.;   // kmph
float gpsSpeedKnots =0.; // knots per hr
float gpsCourse =0.;
char call_telemetry[7]; // WSPR telemetry callsign
char loc_telemetry[5];  // WSPR telemetry locator
uint8_t dbm_telemetry;  // WSPR telemetry dbm
uint8_t dbm_standard;   // WSPR telemetry dbmnt

char loc4[5];           // 4 digit grid square locator
char loc4_telemetry[5]; // 4 digit grid square used for the telemetry message
char loc6[7];           // 6 digit grid square locator
char loc8[9];           // 8 digit grid square locator

// byte Hour, Minute, Second; // used for timing
// long lat, lon;             // used for location

int satellites = 0;
int alt_meters = 0;
bool telemetry_set = false;
int Sats = 0;
double latitude = 42.2833;
double longitude = -87.5639;


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
// ** Calibration is used only for the pico three part option
//  There is code in this project that can calibrate the SI5351 frequency against the
// highly accurate gps pulse per second (pps).  This requires a pin for the clock and a pin for the pps.
// This was necessary in the past when using commericial
// SI5351 Arduino modules with inexpensive crystals that had significant frequency error. 
// When PICO_THREE_PART is defined, The calibration code is included in the compile.
// COUNTER_PIN is the gpo 7 processor and is connected to clock 2 of the SI 5351
// The gps pps pin is connected to gpo 6
// 

#define DBGPIN LED_BUILTIN



#include "eeprom.h"  // manage EEPROM storage
#include "./src/TrackerSensors.h"  // read cpu temperature and pannel volts
#include "./src/SI5351Interface.h" // Sends messages using SI5351
#include "./src/GPS.h"             // code to interface with the gps
#include "BMP280.h" // read data from bmp280
#include "MS56zz.h" // read temperature, pressure and altitude from MS5611 sensor
#include "LM75.h" // read temperature from a LM75
#include "ExTelemetry.h"
#include "SendMessages.h" // schedules the sending of messages
#ifdef PICO_THREE_PART
#include "./src/PicoFrequencyCorrection.h"
#endif



void setup()
{
#ifdef DEBUG
  Serial.begin(9600);
#endif
delay(6000);
 
POUTPUT(F(" Version 4 "));

#ifdef NIBBB
#endif


 POUTPUTLN(F(" Starting "));
#ifndef DEBUG
  Serial.begin(9600);
  Serial.println(" Debug messages are turned off ");
#endif
  delay(2000);
#ifdef PICO
  
#else
  clock.begin();
#endif

  sensorSetup();
  pinMode(DBGPIN, OUTPUT);
  digitalWrite(DBGPIN,HIGH );
  pinMode(RF_PWR, OUTPUT);

  pinMode(GPS_PWR, OUTPUT);
  digitalWrite(GPS_PWR, HIGH);

#ifdef PICO
  pinMode(GPS_VBAT, OUTPUT);
  digitalWrite(GPS_PWR, HIGH);
#endif

  pinMode(GPS_nRESET, OUTPUT);
  digitalWrite(GPS_nRESET, HIGH);
  pinMode(PANEL_VOLTS, INPUT);
  MS5611Init();

  //ExTelemEncode2();

  #ifdef DEBUG
  //BME280Test();
  //TestEEPROM();
  //MS5611Test();
  //LM75Test();
  #endif

  rf_pwr_off();
  digitalWrite(DBGPIN, LOW);

  //rf_beep(); // Send dashes to test Si5351  - below band by up to 200 Hz
  //delay(2000);
  //code_u4b_telemetry_loc();
  // Temp.init();
  POUTPUT((F(" cpu Temperture ")));
  float cpuTemp = getTempCPU();

  POUTPUTLN((cpuTemp));
                                                                                     
  POUTPUT(F(" Panel Voltage "));
  volts = readVcc();
  POUTPUTLN((volts));

  digitalWrite(DBGPIN, HIGH);

  gpsStartTime = millis();
}


void loop() //*********************  Loop *********************
{
 
  bool getInfo = gpsGetData(); // try to sync before every transmission
  if (getInfo == false)  // time out on getting a gps sync
  {       
    CalibrationDone = true  ; // skip calibration if gps no sync     
    if (clockSetOverride == true) 
    // since boot the clock has been set, 
    //use the previous information in message and get altitude from pressure calculation
    { 
      if(gpsAltitude < 8000)
     { 
      MS5611TakeData();
      gpsAltitude = MS5611GetAltitude(); // get altitude based on pressure}
      }
      else return;// try getting a sync again
    }
    else return; // try getting a sync again
  }

// calibrateFreq(); // calibrate the SI5351 frequency
#ifdef PICO_THREE_PART // This option includes code to calibrate the SI5351 frequency against the gps pulse per second signal.  This is used to improve the accuracy of the transmit frequency.  The calibration is only done once after power on and gps lock.  If there is no gps lock, the calibration is skipped and the transmit sequence starts using the uncalibrated frequency.

    CalibrationDone = false;
#ifdef DEBUG_SI5351
      CalibrationDone = true; // skip calibration for testing
#endif
// Calibrate Si5351 Xtal frequency against the gps pulse per second signal.  This is used to improve the accuracy of the transmit frequency.  The calibration is only done once after power on and gps lock.  If there is no gps lock, the calibration is skipped and the transmit sequence starts using the uncalibrated frequency.
  if (getInfo == true || CalibrationDone == false ) // no gps lock, skip calibration and start transmit sequence
  {
    POUTPUTLN((F(" Waiting for SI5351 calibration to complete ")));
    setupPinInterrupt();
    si5351_calibrate_init();
    int ical = 25;

    tcount = 0;
    attachPinInterrupt(); // enable the interrupt for the pps signal

    while (CalibrationDone == false)
    {

      delay(1000);
      ical--;
      POUTPUTLN((ical));
      if (ical < 0)
      {
        POUTPUTLN((F(" Error no pps calibration signal ")));
        break;
      }
    }
    CalibrationOff(); // disable the interrupt and end the calibration process
    si5351_calibrate_off();
    if (CalibrationDone == true)
    {
      POUTPUTLN((F(" SI5351 Calibration Completed ")));
      POUTPUT(F(" SI5351 Correction Factor "));
     POUTPUTLN((correction));
     POUTPUT(F(" SI5351 Frequency Count "));
     POUTPUTLN((SiCnt));
     POUTPUT(F(" PPS Count "));
     POUTPUTLN((tcount));
    }
    else
    {
      POUTPUTLN((F(" SI5351 Calibration Failed ")));
      CalibrationDone = true; // skip calibration if it failed
    }
  }
  if (correction > 1.01 || correction < .99)
  {
    POUTPUTLN((F(" Error calibration count too large or too small ")));
    correction = 1;
  }

#else
  CalibrationDone = true;
  correction = 1; // no calibration, use the uncorrected frequency
#endif

// calibrateFreq(); // calibrate the SI5351 frequency
#ifdef PICO_THREE_PART // This option includes code to calibrate the SI5351 frequency against the gps pulse per second signal.  This is used to improve the accuracy of the transmit frequency.  The calibration is only done once after power on and gps lock.  If there is no gps lock, the calibration is skipped and the transmit sequence starts using the uncalibrated frequency.

    CalibrationDone = false;
#ifdef DEBUG_SI5351
      CalibrationDone = true; // skip calibration for testing
#endif
// Calibrate Si5351 Xtal frequency against the gps pulse per second signal.  This is used to improve the accuracy of the transmit frequency.  The calibration is only done once after power on and gps lock.  If there is no gps lock, the calibration is skipped and the transmit sequence starts using the uncalibrated frequency.
  if (getInfo == true || CalibrationDone == false ) // no gps lock, skip calibration and start transmit sequence
  {
    POUTPUTLN((F(" Waiting for SI5351 calibration to complete ")));
    setupPinInterrupt();
    si5351_calibrate_init();
    int ical = 25;

    tcount = 0;
    attachPinInterrupt(); // enable the interrupt for the pps signal

    while (CalibrationDone == false)
    {

      delay(1000);
      ical--;
      POUTPUTLN((ical));
      if (ical < 0)
      {
        POUTPUTLN((F(" Error no pps calibration signal ")));
        break;
      }
    }
    CalibrationOff(); // disable the interrupt and end the calibration process
    si5351_calibrate_off();
    if (CalibrationDone == true)
    {
      POUTPUTLN((F(" SI5351 Calibration Completed ")));
    }
    else
    {
      POUTPUTLN((F(" SI5351 Calibration Failed ")));
      CalibrationDone = true; // skip calibration if it failed
    }

     POUTPUT(F(" SI5351 Correction Factor "));
     POUTPUTLN((correction));
     POUTPUT(F(" SI5351 Frequency Count "));
     POUTPUTLN((SiCnt));
     POUTPUT(F(" PPS Count "));
     POUTPUTLN((tcount));

  }
  if (correction > 1.01 || correction < .99)
  {
    POUTPUTLN((F(" Error calibration count too large or too small ")));
    correction = 1;
  }
#endif


  POUTPUTLN((F(" Starting Transmit Logic")));

  SendWSPRMessages();
}
