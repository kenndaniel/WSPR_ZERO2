/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - March 2022
   Modified to work on the traquino architecture by K9YO Chicago IL 2024 - March 2025
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/
// Define the tracker
//#define PICO
#define NIBBB

#include "pins.h" // PIN and Clock deffinitions

// Debug flags
#define DEBUG // Serial.println debug output is generated if DEBUG is defined
// Normally when the program boots, a 10 sec signal will be generated (beep) 200 Hz below band center
//#define DEBUG_SI5351  // Send on first even minute regardless of send_time_slot & Beep is 30 sec long
//#define DEBUG_SI5351_wo_GPS // sends first message immediately after beep ends 

//const char call[] = "K9YO";     // Amateur callsign
const char call[] = "KF8IA";     // Amateur callsign

// WSPR Band Center Frequencies (Do not change)
#define WSPR_30m      10140200UL  // Center of WSPR 30m band 
#define WSPR_20m      14097100UL  // Center of WSPR 20m band 
#define WSPR_17m      18106100UL  // Center of WSPR 17m band 
#define WSPR_15m      21096100UL  // Center of WSPR 15m band 
#define WSPR_12m      24926100UL  // Center of WSPR 12m band 
#define WSPR_10m      28126100UL  // Center of WSPR 10m band 
// Technician license holders should set WSPR_FREQ to WSPR_10m

// Band to transmit on
#define WSPR_FREQ1      WSPR_15m 

const char std_telemID[] = "Q1";  // Standard telemetry prefix e.g. Q1 is Qx1xxx
// the time slot for the telemetry transmission : 0,2,4,6 or 8 corresponds to the minute value on LU7AA or Traquito website
// The standard WSPR transmission will start two minutes before the lu7aa send_time_slot
// lu7aa Time-Slot below    
const int send_time_slot = 8;  

// Standard Telemetry Type
#define WB8ELK
//#define U4B

// FREQ_BIAS is added to the center frequency
// For U4B telemetry messages this will be 80, -80, 40 or -40 -- Note do not use a plus sign for positive numbers 
// For U4B channel conversion information see https://traquito.github.io/channelmap/
// For WB8ELK FREQ_BIAS should be 0
#define FREQ_BIAS   0   // added to WSPR_FREQ1 e.g. -40 or 40 (no + sign) This can also be used to correct txco error e.g. 71 instead of 80
// SPREAD_SPECTRUM will randomly change the transmit frequenc per set of transmission bu a random amount
// For U4B protocol this should be 0
// For WB8ELK protocol this should be 20
#define SPREAD_SPECTRUM 20  // random +- change to frequency

// gps must lock position within 15 minutes or system will sleep or use the default location if the clock was set
#define GPS_TIMEOUT 900000 

// APRS Variables included in case of combination of WSPR with APRS
#define APRS_SID  8

// Optional
// Not implemented -- optional non-standard telemetry for additional sensors
// Custom telemetry message code modifications are required for additonal sensors
// The user is reguired to look into the code and make custom modifications. Start with the bottom of SendMessages.h
// call prefix  T1 is T1nxxx  Note that no non standard telemetry transmission is implemented in the this code
// These provide slightly less space for data encoding because it must be of the format e.g. T1abc.  There are only three 
// positions available for encoding.
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,B,F,G,I,K,M,N,W,R, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
// End of user definable information
const int telemTrkID = 5; // Single digit (0-9) to indicate the balloon being tracked (BD). 
const char telemID[] = "T1";   // Message identifier - first two characters of telemetery call 
const char telemID2[] = "T9";  // Message identifier for second custom message
