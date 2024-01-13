/*
   Timing Schedule - See readme file
*/

//#include <wdt_samd21.h>
void resetFunc() // Reset the Arduino
{
//wdt_init ( WDT_CONFIG_PER_8  );
}


void charArrayCpy(char dest[], char src[], int cnt)
{
  for (int i = 0; i< cnt; i++)
  {
    dest[i]= src[i];
    if(src[i]== '\0') return;
  }
  dest[cnt] = '\0';
}

#include "ConvertData.h"
#include "./src/CodeStandardMessage.h"
#include "CodeTelemetryMessage.h"

void SendMessages() // Timing
{
// run additional scripts here to generate data prior to TX if there is a large delay involved.
  code_location();  // convert latitude and longitude to grid square
  code_standard_power(); // Update WSPR power level (standard message -> coded altitude)
  POUTPUT((F("Altitude = ")));
  POUTPUTLN((gpsAltitude));
  POUTPUT((F("Standard Message ")));
  setModeWSPR(); // set WSPR standard message mode

  int stopSecond = 0;
  int curSecond = 0;
  bool sendMinute = false;

  const unsigned long period = 50;
  unsigned long time_now = 0;


  POUTPUT((F("Waiting for the send time for time slot ")));
  POUTPUTLN((send_time_slot));
// Wait for the beginning of the even minute after xtal calibration is completed
  while (!((int)minute() % 2 == 0 && (int)second() < 2 && CalibrationDone == true && sendMinute == true))
  {
      #ifdef DEBUG_SI5351
        break;
      #endif

    if((int)minute()%10 == (send_time_slot -2 )) sendMinute = true;
    if( send_time_slot == 0 && (int)minute()%10 == 8) sendMinute = true;
  
    time_now = millis();
    while(millis() < time_now + period){
        //wait 50 ms period
    }
      curSecond = (int)second();

      if (curSecond % 5 == 0 && curSecond != stopSecond)
      {
        digitalWrite(DBGPIN, HIGH);
                
        stopSecond = curSecond;
        POUTPUT((minute()));
        POUTPUT((":"));
        POUTPUTLN(((int)second()));
      }
      else
      {
        digitalWrite(DBGPIN, LOW);
      }
  }

  // Send standard WSPR message Frequency 1
 
  rf_on();  //  turn on transmitter
  POUTPUTLN(F(" Sending Standard Message "));
  digitalWrite(DBGPIN, HIGH);
  transmit();      // begin radio transmission

  // Send standard telemetry message
  code_standard_telemetry_callsign();    // Set the telemetry callsign 
  code_telemety_loc();          // Set the telemetry location
  code_telemetry_power();       // Set the telemetry power
  POUTPUTLN((F("Waiting for Standard Telemetry Message ")));
  setModeWSPR_telem(); // set WSPR telemetry message mode
  waitForEvenMinute();
  POUTPUTLN((F("Sending  Standard Telemetry Message ")));
  digitalWrite(DBGPIN, HIGH);
  transmit();            // begin radio transmission
  

  // Send additional telemetry message
  POUTPUTLN((F("Waiting for Additional Telemetry Message ")));

  code_high_precision_temp_pres_humid()
  setModeWSPR_telem(); // set WSPR telemetry message mode 

  waitForEvenMinute();
  POUTPUTLN((F("Sending Telemetry Message")));
  transmit();      // begin radio transmission
  
  
  POUTPUTLN((F("****** lOOP RESET RESET RESET RESET **********")));
  resetFunc();  // Reset Arduino - program stats from the beginning
}



void waitForEvenMinute()
{
  int stopSecond = 0;
  int curSecond = 0;

  const unsigned long period = 50;
  unsigned long time_now = 0;
 while (!((int)minute() % 2 == 0 && (int)second() < 2))
  {
    time_now = millis();
    while(millis() < time_now + period){
        //wait 50 ms
    }

      curSecond = (int)second();

      if (curSecond % 2 == 0 && curSecond != stopSecond)
      {
        digitalWrite(DBGPIN, LOW);
        stopSecond = curSecond;
        POUTPUT((minute()));
        POUTPUT((":"));
        POUTPUTLN(((int)second()));
      }
      else
      {
        digitalWrite(DBGPIN, HIGH);
      }
    
  }
}


void sleep() 
{   // Delay until the next transmission
    // shut off the power - not used
    POUTPUT((F("Sleep ")));
    int t_minutes = SEND_INTERVAL;
    POUTPUT(((t_minutes)));
    POUTPUTLN((F(" min ")));
    rf_off();
    gpsOff();
    digitalWrite(SLEEP_PIN, HIGH);  // Not used
    digitalWrite(DBGPIN, LOW);
    unsigned long duration = (unsigned long)(SEND_INTERVAL*60000);
    delay(duration);  // Wait until it is time for the next transmission
    resetFunc();  // Reset Arduino - program stats from the beginning
    
}
