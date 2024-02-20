/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - March 2022
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/

//#define DEBUG // Debug output is generated if DEBUG is defined
//#define DEBUG_SI5351  // Uncomment when testing without the GPS module attached.
const char call[] = "KD9TPM";     // Amateur callsign
const char std_telemID[] = "Q7";  // Standard telemetry prefix e.g. Q1 is Qx1xxx
const int send_time_slot = 4;   // the time slot for the telemetry transmission : 0,2,4,6 or 8

// WSPR Band Center Frequencies (Do not change)
#define WSPR_30m      10140200UL  // Center of WSPR 30m band - actual frequency is random each transmission
#define WSPR_20m      14097100UL  // Center of WSPR 20m band - actual frequency is random each transmission
#define WSPR_17m      18106100UL  // Center of WSPR 17m band - actual frequency is random each transmission
#define WSPR_15m      21096100UL  // Center of WSPR 15m band - actual frequency is random each transmission
#define WSPR_12m      24926100UL  // Center of WSPR 12m band - actual frequency is random each transmission
#define WSPR_10m      28126100UL  // Center of WSPR 10m band - actual frequency is random each transmission

// Technician license holders should set both WSPR_FREQ to WSPR_10m
// First band to transmit on
#define WSPR_FREQ1      WSPR_20m  

// Reference txco frequency for SI5351
#define SI5351_XTAL 27000000  // Standard
//#define SI5351_XTAL 25000000

// APRS Variables included in case of combination of WSPR with APRS
#define APRS_SID  7
//#define VCXO_GAIN 52 // 45 should be ok - Adjustment of the difference between the low and high tone frequencies 9-9.5MHz Critical


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
const int telemTrkID = 6; // Single digit (0-9) to indicate the tracker (BDD). 
const char telemID[] = "T1";   // Message identifier - first two characters of telemetery call 

