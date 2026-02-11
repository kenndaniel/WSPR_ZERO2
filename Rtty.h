// RTTY Transmitter code for Si5351
// Ken Daniel K9YO 2025
// Adapted for Si5351 from
// RTTY Transmitter for AD9851 by
// George Smart, M1GEO.
//  here: http://www,george-smart.co.uk/wiki/Arduino_RTTY
//
// RTTY Baudot Code
// Tim Zaman
//  here: http://www.timzaman.nl/?p=138&lang=en

// RTTY Output Frequency

/*
30 m	10120–10150 kHz	Most activity around 10140 kHz
20 m	14080–14100 kHz (contests up to 14150 kHz)	Avoid beacon at 14100 kHz
17 m	18100–18110 kHz
15 m	21080–21100 kHz (contests up to 21150 kHz)	Avoid beacon at 21150 kHz
12 m	24910–24930 kHz	Most activity around 24920 kHz
10 m	28080–28100 kHz (contests up to 28200 kHz)	JA activity 28070–28200 kHz
 */

// RTTY Offset Frequency
#define RTTY_OSET 170. //Hz
#define BAUD_RATE 44.45

#include <stdint.h>

#define ARRAY_LEN 32
#define LETTERS_SHIFT 31
#define FIGURES_SHIFT 27
#define LINEFEED 2
#define CARRRTN 8

#define is_lowercase(ch) ((ch) >= 'a' && (ch) <= 'z')
#define is_uppercase(ch) ((ch) >= 'A' && (ch) <= 'Z')

class Rtty
{
public:
  Rtty(unsigned long frequency);
  void sendText(char str[]);

private:
  char letters_arr[33] = "\000E\nA SIU\rDRJNFCKTZLWHYPQOBG\000MXV\000";
  char figures_arr[33] = "\0003\n- \a87\r$4',!:(5+)2#6019?&\000./;\000";
  unsigned long RTTYFrequency = 21096300UL; // Frequency in Hz

  void rtty_txbit(int bit);
  void rtty_start_tone();
  uint8_t char_to_baudot(char c, char *array);
  void rtty_txbyte(int c);
};

Rtty::Rtty(unsigned long frequency)
{
  RTTYFrequency = frequency - RTTY_OSET/2.;
}


uint8_t Rtty::char_to_baudot(char c, char *array)
{
  int i;
  for (i = 0; i < ARRAY_LEN; i++)
  {
    if (array[i] == c)
      return i;
  }

  return 0;
}

void Rtty::rtty_txbyte(int c)
{
  int8_t i;
  uint8_t b = (uint8_t) c;
  rtty_txbit(0);

  /* for (i = 4; i >= 0; i--) */
  for (i = 0; i < 5; i++)
  {
    if (b & (1 << i))
      rtty_txbit(1);
    else
      rtty_txbit(0);
  }

  rtty_txbit(1);
}

enum baudot_mode
{
  NONE,
  LETTERS,
  FIGURES
};

void Rtty::sendText(char str[])
{
  // Allowed characters in messages
  // All upper case lettere (loer case is converted to upper case
  // numbers and ',!:()+#?&./; There is no @
  rtty_start_tone();
  enum baudot_mode current_mode = NONE;
  char c;
  uint8_t b;
  rf_on();
  POUTPUTLN((F(" SI5351 Start RTTY Transmission ")));
  int i=0;
  while (str[i] != '\0')
  {
    c = str[i];
    /* some characters are available in both sets  */
    if (c == '\n')
    {
      rtty_txbyte(LINEFEED);
    }
    else if (c == '\r')
    {
      rtty_txbyte(CARRRTN);
    }
    else if (is_lowercase(str[i]) || is_uppercase(str[i]))
    {
      if (is_lowercase(str[i]))
      {
        c -= 32;
      }

      if (current_mode != LETTERS)
      {
        rtty_txbyte(LETTERS_SHIFT);
        current_mode = LETTERS;
      }

      rtty_txbyte(char_to_baudot(c, letters_arr));
    }
    else
    {
      b = char_to_baudot(c, figures_arr);

      if (b != 0 && current_mode != FIGURES)
      {
        rtty_txbyte(FIGURES_SHIFT);
        current_mode = FIGURES;
      }

      rtty_txbyte(b);
    }

    i++;
  }
  // Turn off the output
  rf_off();
}

void Rtty::rtty_start_tone()
{ // Send an inital tone for synchronization
  rf_on();
  rtty_txbit(0); // Send space
  delay(1000);
  rf_off();
}

// Transmit a bit as a mark or space
void Rtty::rtty_txbit(int bit)
{
  // Serial.println(" txbit ");
  const unsigned long duration = 1000. / BAUD_RATE;
  if (bit)
  {
    // High - mark
    digitalWrite(DBGPIN, HIGH);
    sendTone((RTTYFrequency + RTTY_OSET), duration);
  }
  else
  {
    // Low - space
    sendTone(RTTYFrequency, duration);
    digitalWrite(DBGPIN, LOW);
  }
}

