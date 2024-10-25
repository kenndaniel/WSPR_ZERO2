// NIBBB424V1 pin definitions
// Tracability to cpu schematic definitions dated 9/8/24

#define CLOCKCAL 18  // 2.5 MHz signal from Si5351
#define COUNTER_PIN CLOCKCAL
#define PANEL_VOLTS A4  // Solar Cell voltage
#define CSO A5
#define CSI A6
#define PROG_RESET 4 // Resets the SAMD21
#define VCO_CTRL D3  
#define PWR_CTRL 5 // Can be used to turn off si5351 
#define RF_PWR PWR_CTRL
#define GPS_PPS 0 // GPS pulse per second
#define interruptPinPPS GPS_PPS
//#define GPS_RX D13
//#define GPS_TX D14
#define GPS_nRESET 7   // Low for gps operation
#define ON_OFF 6 
#define GPS_PWR  ON_OFF // High for gps operation
