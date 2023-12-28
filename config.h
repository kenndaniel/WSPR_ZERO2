/*
   HABalloon by KD2NDR, Miami Florida October 25 2018
   Improvements by YO3ICT, Bucharest Romania, April-May 2019 
   Modified to be simpler and work on standard Arduino by K9YO Chicago IL 2019 - March 2022
   You may use and modify the following code to suit
   your needs so long as it remains open source
   and it is for non-commercial use only.
   Please see readme file for more information.
*/
#define DEBUG // Debug output is generated if DEBUG is defined
//#define DEBUG_SI5351  // Uncomment when testing without the GPS module attached.
const char call[] = "K9YO";     // Amateur callsign
const char std_telemID[] = "Q3";  // Standard telemetry prefix e.g. Q1 is Qx1xxx
const int send_time_slot = 2;   // the time slot for the telemetry transmission : 0,2,4,6 or 8

#define WSPR_FREQ1      0  

// Reference txco frequency for SI5351
#define SI5351_XTAL 27000000  // Standard
//#define SI5351_XTAL 25000000

// Optional
const char telemID[] = "T1";   // Not implemented -- optional non-standard telemetry for additional sensors
// Custom telemetry message code modifications are required for additonal sensors
// The user is reguired to look into the code and make custom modifications. Start with the bottom of SendMessages.h
// call prefix  T1 is T1xxx  Note that no non standard telemetry transmission is implemented in the this code
// These provide slightly less space for data encoding because it must be of the format e.g. T1abc.  There are only three 
// positions available for encoding.
//If you go to Wikipedia and look up ITU prefix you will find that there are many more prefixes available. 
//For example "any letter other than A,B,F,G,I,K,M,N,W,R, + 1", "X + any number", E8, E9,J9, " letter O + any number" , T9, "U + any number"
// End of user definable information


