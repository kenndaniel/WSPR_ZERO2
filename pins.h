// Contains pin definitions for the NIBBB tracker and the Traquito (PICO)tracker


#ifdef PICO
//#define CLOCKCAL 18  // 2.5 MHz signal from Si5351
//#define COUNTER_PIN CLOCKCAL
//#define CSO A5
//#define CSI A6
#define SReset // System  reset
#define PWR_CTRL SReset
#define PROG_RESET LED_BUILTIN // Resets the SAMD21 
#define PANEL_VOLTS 29  // Solar Cell voltage
#define RF_PWR 28 // pin used to turn off SI5351
#define RF_ON  LOW
#define RF_OFF  HIGH
#define GPS_RX 9
#define GPS_TX 8
#define ON_OFF LED_BUILTIN 
#define GPS_PWR  2 // High for gps operation
#define GPS_VBAT 3
#define GPS_nRESET 6   // Low for gps operation
#define SI5351_XTAL 26000000UL  // use for Arduino Adafruit module or equivalent
#define XMIT_CLOCK0 SI5351_CLK0  // Must be CLKO for fanout to work
#define XMIT_CLOCK1 SI5351_CLK1

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
#define VCO_CTRL 3  
#define PWR_CTRL 5 // Can be used to turn off si5351 
#define RF_ON  HIGH
#define RF_OFF  LOW
#define RF_PWR PWR_CTRL
#define GPS_PPS 0 // GPS pulse per second
#define interruptPinPPS GPS_PPS
//#define GPS_RX D13
//#define GPS_TX D14
#define GPS_nRESET 7   // Low for gps operation
#define ON_OFF 6 // GPS on off
#define GPS_PWR  ON_OFF // High for gps operation
// Reference txco frequency for SI5351
#define SI5351_XTAL 27000000UL  // Standard
// HF Clock
#define XMIT_CLOCK0 SI5351_CLK2  // VHF must be CLKO for fanout to work
#define XMIT_CLOCK1 SI5351_CLK3

#endif
