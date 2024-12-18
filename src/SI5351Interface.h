/*
   Low level functions that perform the final message encoding and interface with the SI5351 Transmitter
*/
/*
   Mode defines
*/

#define JT9_TONE_SPACING 174  // ~1.74 Hz
#define WSPR_TONE_SPACING 146 // ~1.46 Hz

#define JT9_DELAY 576  // Delay value for JT9
#define WSPR_DELAY 683 // Delay value for WSPR

#define JT9_FREQ 14000000UL

#define CLK_CAL SI5351_CLK4 // the clock used for crystal calibration
#define XMIT_CLOCK0 SI5351_CLK0  // Must be CLKO for fanout to work
#define XMIT_CLOCK1 SI5351_CLK1

void setModeJT9_1()
{

  symbol_count = JT9_SYMBOL_COUNT;
  tone_spacing = JT9_TONE_SPACING;
  tone_delay = JT9_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.jt9_encode(message1, tx_buffer);
}

void setModeJT9_2()
{

  symbol_count = JT9_SYMBOL_COUNT;
  tone_spacing = JT9_TONE_SPACING;
  tone_delay = JT9_DELAY;
  memset(tx_buffer, 0, symbol_count); // Clears Tx buffer from previous operation.
  jtencode.jt9_encode(message2, tx_buffer);
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
  digitalWrite(RF_PWR, HIGH);
  delay(10);

  POUTPUTLN((F(" SI5351 Start Initialization "))); 
  bool checkI2C = si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL, 0);
  if (checkI2C == false)
  {
   POUTPUTLN((F("  XXXXXXXXX Si5351 i2c failure - Check wiring")));
   OLEDrotate(F("Si5351 i2c failed"),ERROR);
   delay(60000);
    return false;
  }

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


bool twoChanel = true; // set true to use two channel inverted output, set false to use only one chanel

void rf_on()
{

  si5351_init();
  si5351.set_ms_source(XMIT_CLOCK0, SI5351_PLLA);
  si5351.drive_strength(XMIT_CLOCK0, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.
  si5351.drive_strength(XMIT_CLOCK1, SI5351_DRIVE_8MA); // Set for max power if desired. Check datasheet.

  si5351.set_clock_fanout(SI5351_FANOUT_MS, 1);
  si5351.set_clock_source(XMIT_CLOCK1, SI5351_CLK_SRC_MS0); // clock 1 gets freq from clock 0
  si5351.set_clock_invert(XMIT_CLOCK1, 1);



  si5351.output_enable(XMIT_CLOCK0, 1); 
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 1);
  
}

void rf_off()
{
  si5351.output_enable(XMIT_CLOCK0, 0); 
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 0);
}

void rf_pwr_off()
{  
  digitalWrite(RF_PWR, LOW);
}



/*
   Message encoding
*/

void transmit() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  rf_on();
  POUTPUTLN((F(" SI5351 Start Transmission ")));
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), XMIT_CLOCK0); // clock 1 will follow this
    
    while (millis() < time_now + period) // Found to be more accruate than delay()
    {
    }
  }
  // Turn off the output
  rf_off();
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

// Send a beep in the beginning to test the transmitter
void rf_beep()
{    
  rf_on();
  setToFrequency1();
  POUTPUTLN((F(" Starting beep "))); 
  OLEDrotate(" Starting RF Beep ", INFO);
  OLEDbeginNoRotate();
  unsigned long freq1 = (unsigned long)((WSPR_FREQ1 * correction)-200); 
  for (int j = 10; j>=0;--j)
    {
      si5351.set_freq((freq1 * 100) + (11 * tone_spacing), XMIT_CLOCK0); 
      delay(500);
      si5351.set_freq((freq1 * 100) + (1 * tone_spacing), XMIT_CLOCK0); 
      delay(500);
      OLEDnoRotate(String(j),INFO);
    }
    // Turn off the output
    rf_off();
}

