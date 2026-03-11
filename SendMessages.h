/*
   Timing Schedule - See readme file
*/

// #include <wdt_samd21.h>
#include "Morse.h"
#include "Rtty.h"
Rtty rtty(WSPR_FREQ1 WSPR_OFFSET );
Morse cw(CW,WSPR_FREQ1 WSPR_OFFSET );
//Morse qrss(QRSS,WSPR_FREQ1 + 500 );  

void resetFunc() // Reset the Arduino
{
  return;
}

void charArrayCpy(char dest[], char src[], int cnt)
{
  for (int i = 0; i < cnt; i++)
  {
    dest[i] = src[i];
    if (src[i] == '\0')
      return;
  }
  dest[cnt] = '\0';
}


#include "ConvertData.h"
#include "./src/CodeStandardMessage.h"
#include "CodeTelemetryMessage.h"
#include "FQSMessages.h"

void SendWSPRMessages() // Timing
{
  // run additional scripts here to generate data prior to TX if there is a large delay involved.
  MS5611TakeData();  
  code_location();       // convert latitude and longitude to grid square
  code_standard_power(); // Update WSPR power level (standard message -> coded altitude)
  POUTPUT((F("Altitude = ")));
  POUTPUTLN((gpsAltitude));
    POUTPUT((F("Speed (knots)= ")));
  POUTPUTLN((gpsSpeedKnots));
  POUTPUT((F("Standard Message ")));

  setModeWSPR();     // set WSPR standard message mode
  setToFrequency1(); // Initialize the transmit frequency
#ifdef DEBUG_SI5351_wo_GPS
  transmit(); // begin radio transmission
#endif

  int stopSecond = 0;
  int curSecond = 0;
  bool sendMinute = false;

  const unsigned long period = 50;
  unsigned long time_now = 0;

  POUTPUT((F("Waiting for the send time for time slot ")));
  POUTPUTLN((send_time_slot));
  int beginTime = send_time_slot-2;
  if (send_time_slot == 0)  beginTime = 8;
// Wait for the beginning of the even minute after xtal calibration is completed
  while (!((int)minute() % 2 == 0 && (int)second() < 2 && CalibrationDone == true && sendMinute == true))
  {
      #ifdef DEBUG_SI5351
        POUTPUTLN((F("DEBUG_SI5351 is Defined - Starting Transmit")));
        //waitForEvenMinute();
        break;
      #endif

    if((int)minute()%10 == beginTime) sendMinute = true;
    if((int)minute()%10 != beginTime) sendMinute = false;  // rare case if gps starts during begin minute
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

  POUTPUTLN(F(" Sending Standard Message "));

  digitalWrite(DBGPIN, HIGH);
  transmit();      // begin radio transmission

  // Second message -----------
  #ifdef WB8ELK
  // Send WB8ELK telemetry message
  code_WB8ELK_telemetry();
  POUTPUTLN((F("Waiting for WB8ELK Telemetry Message ")));
  setModeWSPR_telem(); // set WSPR telemetry message mode
  waitForEvenMinute();
  POUTPUTLN((F("Sending  Standard Telemetry Message ")));
  digitalWrite(DBGPIN, HIGH);
  transmit();            // begin radio transmission
  #endif

  #ifdef U4B
  // Send U4B telemetry message
  code_u4b_telemetry();
  POUTPUTLN((F("Waiting for u4b Telemetry Message ")));
  setModeWSPR_telem(); // set WSPR telemetry message mode
  waitForEvenMinute();
  digitalWrite(DBGPIN, HIGH);
  transmit();            // begin radio transmission
  #endif
  
  //Third Message -----------------
  ExTelemEncode1();  // Traquito extended telemetery 
  POUTPUTLN((F("Sending Extended Telemetry Message - Slot 2 ")));
  setModeWSPR_telem(); // set WSPR telemetry message mode
  waitForEvenMinute();
  digitalWrite(DBGPIN, HIGH);
  transmit();            // begin radio transmission

  //Fourth Message -----------------
  ExTelemEncode2();  // Traquito extended telemetery 
  POUTPUTLN((F("Sending Extended Telemetry Message - Slot 2 ")));
  setModeWSPR_telem(); // set WSPR telemetry message mode
  waitForEvenMinute();
  digitalWrite(DBGPIN, HIGH);
  transmit();            // begin radio transmission

  // delay(5*1000); 
  // POUTPUTLN((F("Begin CW transmission")));
  // cw.setCWSpeeed(14);
  // cw.setFrequency(WSPR_FREQ1 WSPR_OFFSET );
  // cw.sendText("... ");
  // cw.sendText(call) ;
  // cw.sendText(" HAMSCI Balloon RTTY 45 then FQS 2, 4.5, 6 AR ");
  // POUTPUTLN((F("End CW transmission")));

  delay(5*1000); 

  //POUTPUTLN((F("Begin RTTY transmission")));
  // rf_on();
  // char msgRTTY[] = "\r\n\r\n";
  // rtty.sendText(msgRTTY);
  // rtty.sendText("  ....");
  // rtty.sendText(call) ;
  //  rtty.sendText(" HAMSCI Balloon RTTY  \r\n");
  // rtty.sendText(" If you rx notify K9YO(at)aarl.net \r\n\r\n");
  // rtty.sendText(" http://bit.ly/4bt5K0s");
  //   rtty.sendText("  KE9LSI AR \r\n\r\n");
  // rf_off();
  //POUTPUTLN((F("End RTTY transmission")));

  //delay(5*1000); // delay 10 sec

 POUTPUTLN((F("Begin FSQ transmission")));

  //Serial.println(FQSMessage());
  rf_on();
  setModeFSQ(MODE_FSQ_2, FQSMessage());
  setFrequencyFQS(WSPR_FREQ1 WSPR_OFFSET);
  transmit();
  rf_off();
  delay(5 * 1000);

  // rf_on();
  // setModeFSQ(  MODE_FSQ_4_5, FQSMessage());
  // setFrequencyFQS(WSPR_FREQ1 WSPR_OFFSET);
  // transmit();
  // rf_off();

  delay(5 * 1000);

  rf_on();
  setModeFSQ(  MODE_FSQ_6, FQSMessage());
  setFrequencyFQS(WSPR_FREQ1 WSPR_OFFSET);
  transmit();
  rf_off();
  POUTPUTLN((F("End FSQ transmission")));

  // delay(10*1000); 
  // POUTPUTLN((F("Begin CW transmission")));
  // cw.setFrequency(WSPR_FREQ1 + 500 );
  // cw.sendText(" 73  de KD9TVR SK");
  // POUTPUTLN((F("End CW transmission")));
  //   rf_off();

  // Send additional telen message
  //POUTPUTLN((F("Waiting for Additional Telemetry Message ")));
  // // QW8IBY OH90 47
  // encode_telen(123890, 123456,1);
  // setModeWSPR_telem(); // set WSPR telemetry message mode
  // waitForEvenMinute();
  //   transmit();      // begin radio transmission
  // code_high_precision_temp_pres_humid();
  // setModeWSPR_telem(); // set WSPR telemetry message mode

  // waitForEvenMinute();
  // POUTPUTLN((F("Sending Pressure/Temp/Humidity Telemetry Message")));
  // transmit();      // begin radio transmission

  // code_speed_direction_message();
  // code_telemetry_power(); // Set the telemetry power
  // setModeWSPR_telem();    // set WSPR telemetry message mode
  // waitForEvenMinute();
  // POUTPUTLN((F("Sending Speed/Direction Telemetry Message")));
  // transmit();      // begin radio transmission

  POUTPUTLN((F("****** lOOP RESET RESET RESET RESET **********")));
  resetFunc(); // Reset Arduino - program stats from the beginning
}


void waitForEvenMinute()
{
  POUTPUTLN((F(" Waiting for Even Minute ")));
   int stopSecond = 0;
  int curSecond = 0;

  const unsigned long period = 50;
  unsigned long time_now = 0;
  while (!((int)minute() % 2 == 0 && (int)second() < 2))
  {
    time_now = millis();
    while (millis() < time_now + period)
    {
      // wait 50 ms
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
{ // Delay until the next transmission
  // shut off the power - not used
  POUTPUT((F("Sleep ")));
  int t_minutes = SEND_INTERVAL;
  POUTPUT(((t_minutes)));
  POUTPUTLN((F(" min ")));
  rf_off();
  gpsOff();
  digitalWrite(DBGPIN, LOW);
  //unsigned long duration = (unsigned long)(SEND_INTERVAL * 60000);
  // delay(duration);  // Wait until it is time for the next transmission
  resetFunc(); // Reset Arduino - program stats from the beginning
}
