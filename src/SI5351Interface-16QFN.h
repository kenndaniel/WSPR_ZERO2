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

#define CLK_CAL SI5351_CLK3 // the clock used for crystal calibration
// This is not used in the latest version of the hardware
void si5351_calibrate_init()
{
  // Initialize SI5351 for gps calibration
  si5351_init();
  si5351.drive_strength(CLK_CAL, SI5351_DRIVE_2MA);
  unsigned long calfreq = 2500000UL;
  si5351.set_freq(calfreq * 100, CLK_CAL); // set calibration frequency
  si5351.output_enable(CLK_CAL, 1);        // Enable output
}

typedef struct
{
  unsigned int address; /* 16-bit register address */
  unsigned char value;  /* 8-bit register data */

} si5351b_revb_register_t;


void rf_on()
{
  digitalWrite(RFPIN, HIGH);
  // si5351_init(SI5351_DRIVE_8MA, SI5351_XTAL, 0);
    uint8_t regStatus = si5351.si5351_write(0x0003, 0x00);  // Turn on
}

void rf_off()
{
  Serial.println("HF Off");
  uint8_t regStatus = si5351.si5351_write(0x0003, 0x06);
  digitalWrite(RFPIN, LOW);
}


void HF_init()
{
  // Start I2C comms
  Wire.begin();

  // Check for a device on the bus, bail out if it is not there
  Wire.beginTransmission(SI5351_BUS_BASE_ADDR);
  uint8_t reg_val;
  reg_val = Wire.endTransmission();

  if (reg_val == 0)
  {
    // Wait for SYS_INIT flag to be clear, indicating that device is ready
    uint8_t status_reg = 0;
    do
    {
      status_reg = si5351.si5351_read(SI5351_DEVICE_STATUS);
    } while (status_reg >> 7 == 1);
  }
  const int num_reg = 52;

  si5351b_revb_register_t const si5351b_revb_registers[num_reg] =
      {
          {0x0002, 0x53},
          {0x0003, 0x00},
          {0x0004, 0x20},
          {0x0007, 0x01},
          {0x000F, 0x00},
          {0x0010, 0x8C},
          {0x0011, 0x0F},
          {0x0012, 0x1F},
          {0x0013, 0x8C},
          {0x0014, 0x8C},
          {0x0015, 0x8C},
          {0x0016, 0x8C},
          {0x0017, 0x8C},
          {0x001A, 0x5F},
          {0x001B, 0x90},
          {0x001C, 0x00},
          {0x001D, 0x0E},
          {0x001E, 0xA4},
          {0x001F, 0x10},
          {0x0020, 0x9A},
          {0x0021, 0x40},
          {0x0032, 0x00},
          {0x0033, 0x04},
          {0x0034, 0x00},
          {0x0035, 0x1D},
          {0x0036, 0xE0},
          {0x0037, 0x00},
          {0x0038, 0x00},
          {0x0039, 0x00},
          {0x003A, 0x00},
          {0x003B, 0x04},
          {0x003C, 0x00},
          {0x003D, 0x1D},
          {0x003E, 0xE0},
          {0x003F, 0x00},
          {0x0040, 0x00},
          {0x0041, 0x00},
          {0x005A, 0x00},
          {0x005B, 0x00},
          {0x0095, 0x00},
          {0x0096, 0x00},
          {0x0097, 0x00},
          {0x0098, 0x00},
          {0x0099, 0x00},
          {0x009A, 0x00},
          {0x009B, 0x00},
          {0x00A2, 0x00},
          {0x00A3, 0x00},
          {0x00A4, 0x00},
          {0x00A6, 0x00},
          {0x00A7, 0x00},
          {0x00B7, 0x92}

      };

  for (int i = 0; i < num_reg; i++)
  {
    uint8_t regStatus = si5351.si5351_write(si5351b_revb_registers[i].address, si5351b_revb_registers[i].value);
    if (regStatus != 0)
    {
      Serial.print(" XXX Register Write error HF_init ");
      Serial.println((int)regStatus);
    }

    delay(1);
  }
  si5351.pll_reset(SI5351_PLLA);
  rf_off();
  // si5351.pll_reset(SI5351_PLLB);
  return;
}

void Set_WSPR_frequency(int tone)
{

  int num_reg = 5;
  si5351b_revb_register_t WSPRFreqRegisters[4][num_reg] =
      // si5351-16QFN does not propely set frequency.  Consequently regs are set manually

      // 0

      {{{0x001A, 0x5F},
        {0x001B, 0x90},
        {0x001F, 0x10},
        {0x0020, 0x9A},
        {0x0021, 0x40}},

       // 1

       {{0x001A, 0xC4},
        {0x001B, 0xB4},
        {0x001F, 0x42},
        {0x0020, 0x18},
        {0x0021, 0x30}},

       // 2
       {{0x001A, 0x31},
        {0x001B, 0x2D},
        {0x001F, 0x10},
        {0x0020, 0x86},
        {0x0021, 0x2C}},

       // 3
       {{0x001A, 0xF4},
        {0x001B, 0x24},
        {0x001F, 0x00},
        {0x0020, 0x6B},
        {0x0021, 0x70}}};

  uint8_t regStatus = si5351.si5351_write(0x0003, 0x06); // turn off
  for (int i = 0; i < num_reg; i++)
  {
    regStatus = si5351.si5351_write(WSPRFreqRegisters[tone][i].address, WSPRFreqRegisters[tone][i].value);
    if (regStatus != 0)
    {
      Serial.print(" XXX HF FreqRegisters Write error Set_ frequency ");
      Serial.println((int)regStatus);
    }
  }
  regStatus = si5351.si5351_write(0x0003, 0x00);  // Turn on


  return;
}

void beep()
{
  unsigned long period = 3000;
  unsigned long time_now = 0;
  rf_on();
  time_now = millis();
    Set_WSPR_frequency(1);
    while (millis() < time_now + period) // Found to be more accruate than delay()
    {
    }
  
  rf_off();
}

/*
   Message encoding
*/
void transmit() // Loop through the string, transmitting one character at a time
{

  uint8_t i;
  const unsigned long period = tone_delay;
  unsigned long time_now = 0;
  uint8_t one = 1;
  rf_on();
  for (i = 0; i < symbol_count; i++) // Now transmit the channel symbols
  {
    time_now = millis();

    Set_WSPR_frequency(tx_buffer[i]);

    while (millis() < time_now + period) // Found to be more accruate than delay()
    {
    }
  }
  rf_off();

}

void rf_enable()
{
  digitalWrite(RFPIN, HIGH);
  uint8_t regStatus = si5351.si5351_write(0x0003, 0x00);
}



