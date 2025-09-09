// Contains pin definitions for the NIBBB tracker and the Traquito (PICO)tracker


#ifdef PICO
//#define CLOCKCAL 18  // 2.5 MHz signal from Si5351
//#define COUNTER_PIN CLOCKCAL
#define PANEL_VOLTS A3  // Solar Cell voltage
//#define CSO A5
//#define CSI A6
#define SReset // System  reset
#define PWR_CTRL SReset
#define PROG_RESET LED_BUILTIN // Resets the SAMD21 
#define SI5351_PWR_CTRL 28 // Used to turn off SI5351
#define RF_PWR SI5351_PWR_CTRL
#define GPS_RX 9
#define GPS_TX 8
#define ON_OFF LED_BUILTIN 
#define GPS_PWR  2 // High for gps operation
#define GPS_VBAT 3
#define GPS_nRESET 6   // Low for gps operation

#endif
#ifdef NIBBB
// NIBBB424V1 pin definitions
// Tracability to cpu schematic definitions dated 9/8/24

#define CLOCKCAL 18  // 2.5 MHz signal from Si5351
#define COUNTER_PIN CLOCKCAL
#define PANEL_VOLTS A4  // Solar Cell voltage
#define CSO A5
#define CSI A6
#define PROG_RESET 4 // Resets the SAMD21
#define VCO_CTRL D3  
#define PWR_CTRL D5 // Can be used to turn off si5351 
#define RF_PWR PWR_CTRL
#define GPS_PPS D0 // GPS pulse per second
#define interruptPinPPS GPS_PPS
//#define GPS_RX D13
//#define GPS_TX D14
#define GPS_nRESET D7   // Low for gps operation
#define ON_OFF D6 
#define GPS_PWR  ON_OFF // High for gps operation
#endif
