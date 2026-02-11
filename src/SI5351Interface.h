/*
   Low level functions that perform the final message encoding and interface with the SI5351 Transmitter
   url http://bit.ly/4bt5K0s
*/
/*
   Mode defines
*/

#include <si5351.h>
#include <JTEncode.h>
enum mode
{
  MODE_JT9,
  MODE_WSPR
};
//#include <i2cdetect.h>
enum FSQmode { MODE_FSQ_15, MODE_FSQ_2, MODE_FSQ_3,
  MODE_FSQ_4_5, MODE_FSQ_6};

Si5351 si5351;
JTEncode jtencode;

#define JT9_TONE_SPACING 174  // ~1.74 Hz
#define WSPR_TONE_SPACING 146 // ~1.46 Hz
#define FSQ_TONE_SPACING  879  // ~8.79 Hz
#define FT8_TONE_SPACING 625  // 6.25 Hz

#define JT9_DELAY 576  // Delay value for JT9
#define WSPR_DELAY 683 // Delay value for WSPR

#define FSQ_15_DELAY            685          // Delay value for 1.46 baud FSQ
#define FSQ_2_DELAY             500          // Delay value for 2 baud FSQ
#define FSQ_3_DELAY             333          // Delay value for 3 baud FSQ
#define FSQ_4_5_DELAY           222          // Delay value for 4.5 baud FSQ
#define FSQ_6_DELAY             167          // Delay value for 6 baud FSQ
#define FT8_DELAY               159          // Delay value for FT8

#define CLK_CAL SI5351_CLK4 // the clock used for crystal calibration
#define BUFFER_SIZE 325  // Nunver of tones in a message - A character can expand to up to 3 tones

char message1[14] = ""; // Message1 (13 char limit) for JT9 and FT8
char message2[14] = ""; // Message2 (13 char limit) for JT9 and FT8
uint8_t tx_buffer[BUFFER_SIZE];
uint8_t symbol_count = WSPR_SYMBOL_COUNT;
uint16_t tone_delay, tone_spacing; // for digital encoding

void setFrequency(unsigned long frequency);

void setModeJT9(char *message)
{ // 13 characters 
  // allowed characters A-Z 0-9 Space +-./=,()
  // better than ft8 by about 2 db
  memcpy(message1,message,13);
  message1[13]= '\0';
  symbol_count = JT9_SYMBOL_COUNT;
  tone_spacing = JT9_TONE_SPACING;
  tone_delay = JT9_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.jt9_encode(message1, tx_buffer);
}

void setModeFT8(char * message)
{ // 13 characters 
  // allowed characters A-Z 0-9 Space +-./=,()

  memcpy(message1,message,13);
  message1[13]= '\0';
  symbol_count = FT8_SYMBOL_COUNT;
  tone_spacing = FT8_TONE_SPACING;
  tone_delay = FT8_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.ft8_encode(message1, tx_buffer);
}


FSQmode cur_mode;

void setFrequencyFQS(unsigned long frequency)
{
  setFrequency(frequency - ((float)(33*FSQ_TONE_SPACING))/200.);
  //setFrequency(frequency);

}

void setModeFSQ(FSQmode Mode,  char *message)
{  // https://www.w1hkj.org/docs/FSQ.html
    // the callsign "allcall" is used to seed to everyone
    // 32 characters A-Z, 0-9, .  ,  ?  !  /  -  @
    // 33 tones one per character
    // tx_buffer will be the same length as the message
    // This code limits the message size to 100 characters.
  cur_mode = Mode;
  switch (Mode)
  {
      case MODE_FSQ_15:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_15_DELAY;
      break;
    case MODE_FSQ_2:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_2_DELAY;
      break;
    case MODE_FSQ_3:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_3_DELAY;
      break;
    case MODE_FSQ_4_5:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_4_5_DELAY;
      break;
    case MODE_FSQ_6:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_6_DELAY;
      break;
    default:
      tone_spacing = FSQ_TONE_SPACING;
      tone_delay = FSQ_2_DELAY;
      break;
  }

  if(sizeof(message) > 100) // for reliability, limit the message length to 100 characters
  {
    message[100] = '\n';
  }

  memset(tx_buffer, 0, BUFFER_SIZE); // Clears Tx buffer from previous operation.
  jtencode.fsq_encode(call,  message , tx_buffer);
    uint8_t j = 0;
    while(tx_buffer[j++] != 0xff)
    {
      Serial.print(tx_buffer[j]);
      Serial.print(" ");
    }; 
    symbol_count = j - 1;
    delay(1000);
}

void setModeWSPR()
{ // Sends the info to the si5351 for the standard message
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  // freq = WSPR_FREQ;
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call));
  POUTPUT((F(" ")));
  POUTPUT((loc4));
  POUTPUT((F(" ")));
  POUTPUTLN((dbm_standard)); // rough altitude
  jtencode.wspr_encode(call, loc4, dbm_standard, tx_buffer);
}

void setModeWSPR_telem()
{ // sends the info to the si5351 for the telemetry message
  // This function is used for both u4b/traquito and wb8elk formats
  symbol_count = WSPR_SYMBOL_COUNT;
  tone_spacing = WSPR_TONE_SPACING;
  tone_delay = WSPR_DELAY;
  // freq = WSPR_FREQ;
  memset(tx_buffer, 0, 165); // Clears Tx buffer from previous operation.
  POUTPUT((call_telemetry));
  POUTPUT((F("-")));
  POUTPUT((loc4_telemetry));
  POUTPUT((F("-")));
  POUTPUTLN((dbm_telemetry));
  jtencode.wspr_encode(call_telemetry, loc4_telemetry, dbm_telemetry, tx_buffer);
}

bool si5351_init()
{
  POUTPUTLN(F(""));
  POUTPUTLN((F(" SI5351 Start Initialization "))); 
  //i2cdetect();
  bool checkI2C = si5351.init(SI5351_CRYSTAL_LOAD_0PF, SI5351_XTAL, 0);
  if (checkI2C == false)
  {
   POUTPUTLN((F("  XXXXXXXXX Si5351 i2c failure - Check wiring")));
   delay(60000);
    return false;
  }
  POUTPUTLN((F(" SI5351 Successful Initialization "))); 
  return true;
}

void si5351_calibrate_init()
{
  // Initialize SI5351 for gps calibration
  si5351_init();
  si5351.drive_strength(CLK_CAL, SI5351_DRIVE_2MA);
  unsigned long calfreq = 2500000UL;
  si5351.set_freq(calfreq * 100, CLK_CAL); // set calibration frequency
  si5351.output_enable(CLK_CAL, 1);        // Enable output
}

void si5351_calibrate_off()
{
  si5351.output_enable(CLK_CAL, 0);
}



void rf_on()
{  // Turns power on and initializes clocks

  digitalWrite(RF_PWR, RF_ON);

  delay(10);

  si5351_init();
  si5351.set_ms_source(SI5351_CLK0, SI5351_PLLA);
  si5351.drive_strength(XMIT_CLOCK0, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
  si5351.drive_strength(XMIT_CLOCK1, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.

  si5351.set_clock_fanout(SI5351_FANOUT_MS, 1);
  #ifdef PICO
  si5351.set_clock_source(XMIT_CLOCK1, SI5351_CLK_SRC_MS0); // clock 1 gets freq from clock 0
  si5351.set_clock_invert(XMIT_CLOCK1, 1);
  #endif
  #ifdef NIBBB
  si5351.set_clock_source(XMIT_CLOCK1, SI5351_CLK_SRC_MS0); // clock 1 gets freq from clock 0
  si5351.set_clock_invert(XMIT_CLOCK1, 1);
  si5351.set_clock_source(XMIT_CLOCK0, SI5351_CLK_SRC_MS0); // clock 1 gets freq from clock 0
  #endif



  si5351.output_enable(XMIT_CLOCK0, 1); 
 
  si5351.output_enable(XMIT_CLOCK1, 1);
  
}

void rf_off()
{
  //Disable output
  si5351.output_enable(XMIT_CLOCK0, 0); 
  si5351.output_enable(XMIT_CLOCK1, 0);
  POUTPUTLN((F(" SI5351 End Transmission ")));
}

void rf_pwr_off()
{  

  digitalWrite(RF_PWR, RF_OFF);

}

/*
   Message encoding
*/
// send a signal of a given frequency and duration 
void sendDitDah(unsigned long frequency, float duration)
{ //frequency in Hz , duration in miliseconds
  // Used for sending Morse code
  static unsigned long lastFrequency = 0;
  unsigned long millisec = duration;
  //Serial.print(duration);
  //Serial.print(" ");
  //Serial.println(frequency);
    //Inable output
  si5351.output_enable(SI5351_CLK0, 1); 
  //si5351.output_enable(XMIT_CLOCK0, 1); 
  si5351.output_enable(XMIT_CLOCK1, 1);
  unsigned long time_now = 0;
    //Serial.println("sendBit");
    time_now = millis();
    if (lastFrequency != frequency){
    si5351.set_freq(frequency * 100 , SI5351_CLK0); // clock 1 will follow this
    lastFrequency = frequency;
    }
    
    while ((millis() - time_now) <= millisec) // Found to be more accruate than delay()
    { yield();}
      //Disable output
  si5351.output_enable(SI5351_CLK0, 0); 
  //si5351.output_enable(XMIT_CLOCK0, 0); 
  si5351.output_enable(XMIT_CLOCK1, 0);
  
}

// send DominoEX message
void sendMessage(unsigned long frequency, unsigned long duration, float tone_spacing,
                 int tx_buff[], int symbol_cnt)
{ // frequency in  Hz , duration in microseconds
  // tone_spacing in Hz, tx_buff int array of tones offsets, symbol_cnt number of tones to send
  // Used to transmit DominoEX messages

  unsigned long microsec = duration;
  unsigned long time_now = 0;
  time_now = micros();
  for (int i = 0; i < symbol_cnt; i++) // Now transmit the channel symbols
  {
    time_now = micros();
    si5351.set_freq((frequency  + ((tx_buff[i]+.5) * tone_spacing))*100 , SI5351_CLK0); // clock 1 will follow this

    while ((micros() - time_now) <= microsec) // Found to be more accruate than delay()
    { yield();
    }
  }
}

// send a signal of a given frequency and duration
void sendTone(unsigned long frequency, float duration)
{ //frequency in Hz , duration in miliseconds
  uint8_t i;
  unsigned long millsec = duration;
  unsigned long time_now = 0;

    time_now = millis();
    si5351.set_freq((frequency ) , SI5351_CLK0); // clock 1 will follow this
    
    while ((millis() - time_now) <= millsec) // Found to be more accruate than delay()
    {yield();}
  
}
void transmit() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  rf_on();
  POUTPUTLN((F(" SI5351 Start Transmission ")));
  const unsigned long period = tone_delay*1000;
  unsigned long time_now = 0;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = micros();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), SI5351_CLK0); // clock 1 will follow this
    
    while ((micros() - time_now) <= period) // Found to be more accruate than delay()
    { 
      yield();
    }
  }
  // Turn off the output
  rf_off();
}

void setFrequency(unsigned long frequency)
{
  freq = frequency;
}

void setToFrequency1()
{

  freq = (unsigned long)(WSPR_FREQ1 * (correction));
  POUTPUT(F("Corrected Frequency "));
  POUTPUTLN((freq));
  // random number to create random frequency -spread spectrum
  float randomChange = random(-SPREAD_SPECTRUM, SPREAD_SPECTRUM);
  freq = freq + randomChange + FREQ_BIAS; // freq in middle 150 Hz of wspr band
#ifdef CALIBRATION
  POUTPUT(F(" Correction fraction from Band Center "));
  POUTPUTLN((String(correction, 7)));
#endif
  POUTPUT(F(" Random Change "));
  POUTPUTLN((randomChange));
  POUTPUT(F(" Bias from band center "));
  POUTPUTLN((FREQ_BIAS));
  POUTPUT(F(" Frequency "));
  POUTPUTLN((WSPR_FREQ1 + randomChange + FREQ_BIAS));
  POUTPUT(F(" Final Frequency "));
  POUTPUTLN((freq));

}

// Send a beep to test the transmitter
void rf_beep()
{    
  rf_on();
  //setToFrequency1();
  POUTPUTLN((F(" Starting beep on band + 200HZ "))); ;
  unsigned long freq1 = (unsigned long)((WSPR_FREQ1 * correction)+200); 

  int beepCount = 10;

  #ifdef DEBUG_SI5351
  beepCount = 5;
  #endif

  for (int j = beepCount; j>=0;--j)
    {
      si5351.set_freq((freq1 + 200UL) * 100., XMIT_CLOCK0); 
      delay(500);
      si5351.set_freq((freq1 + 300UL) * 100., XMIT_CLOCK0); 
      delay(500);
    }
    // Turn off the output
    rf_off();
}

