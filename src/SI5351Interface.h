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

#define CLK_CAL SI5351_CLK2 // the clock used for crystal calibration
#define XMIT_CLOCK0 SI5351_CLK0
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
  digitalWrite(RFPIN, HIGH);

  POUTPUTLN((F(" SI5351 Start Initialization ")));
  bool siInit = si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_XTAL, 0);
  delay(2000);
  if (siInit == false)
  {
    Serial.println(" XXXXXXXXX Si5351 initializaton failure - Check wiring");
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

/*
   Message encoding
*/
bool twoChanel = true; // set true to use channel 0 and 1 set false to use only channel 0

void transmit() // Loop through the string, transmitting one character at a time
{
  uint8_t i;
  POUTPUTLN((F(" SI5351 Start Transmission ")));
  digitalWrite(RFPIN, HIGH);
  si5351.output_enable(XMIT_CLOCK0, 1); // Reset the tone to the base frequency and turn on the output
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 1);
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;
  uint8_t one = 1;

  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();
    si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), XMIT_CLOCK0); // not needed for inverted output on CLK!
    if (twoChanel)
    {
      si5351.set_freq((freq * 100) + (tx_buffer[i] * tone_spacing), XMIT_CLOCK1);
      si5351.set_clock_invert(XMIT_CLOCK1, one);
      si5351.pll_reset(SI5351_PLLA);
    }
    while (millis() < time_now + period) // Found to be more accruate than delay()
    {
    }
  }
   // Turn off the output
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 0);
  si5351.output_enable(XMIT_CLOCK0, 0);

  digitalWrite(RFPIN, LOW);
}

void rf_on()
{
  digitalWrite(RFPIN, HIGH);
  si5351_init();
  si5351.output_enable(CLK_CAL, 0); // Disable calibration signal
  si5351.pll_reset(SI5351_PLLA);
  si5351.pll_reset(SI5351_PLLB);

  si5351.drive_strength(XMIT_CLOCK0, SI5351_DRIVE_6MA); // Set for max power if desired. Check datasheet.
  si5351.drive_strength(XMIT_CLOCK1, SI5351_DRIVE_6MA); // Set for max power if desired. Check datasheet.

  si5351.output_enable(XMIT_CLOCK0, 0); // Disable the clock initially
  si5351.output_enable(XMIT_CLOCK1, 0);
}

void rf_off()
{
  if (twoChanel)
    si5351.output_enable(XMIT_CLOCK1, 0);
  si5351.output_enable(XMIT_CLOCK0, 0);
  digitalWrite(RFPIN, LOW);
}

// void setToFrequency2()
// {
//   si5351.pll_reset(SI5351_PLLA);
//   si5351.pll_reset(SI5351_PLLB);
//   freq = (unsigned long) (WSPR_FREQ2*(correction));
//   float randomChang = random(-75,75);
//   freq = freq +(unsigned long) randomChang;
//   POUTPUT(F(" Random Change from Band Center "));
//   POUTPUTLN((randomChang));
//   POUTPUT(F(" Frequency "));
//   POUTPUTLN((WSPR_FREQ1+randomChang));
// }

bool firstFreq = true;
void setToFrequency1()
{

  freq = (unsigned long)(WSPR_FREQ1 * (correction));
  // random number to create random frequency -spread spectrum
  // float randomChange = random(-70, 70);
  float randomChange = 0;

  freq = freq + (unsigned long)randomChange; // random freq in middle 150 Hz of wspr band
#ifdef CALIBRATION
  POUTPUT(F(" Correction fraction from Band Center "));
  POUTPUTLN((String(correction, 7)));
#endif
  POUTPUT(F(" Random Change from Band Center "));
  POUTPUTLN((randomChange));
  POUTPUT(F(" Frequency "));
  POUTPUTLN((WSPR_FREQ1 + randomChange));
  firstFreq = false;
}
void RF_init()
{
  rf_on();
  setToFrequency1();
}
