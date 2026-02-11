
#include "Morse.h"
#include "Arduino.h"

void rf_on();
void rf_off();
void sendTone(unsigned long freq, float duration);
void sendDitDah(unsigned long freq, float duration);
#define DBGPIN 13

// QRSS Standard Offset Frequency Hz
#define QRSS_OSET 10. 

#define DEFAULT_WPM 13 // default cw wpm -- see setCWSpeed 

Morse::Morse(morseMode mMode, unsigned long frequency)
{
  mode = mMode;
  if (mode == QRSS) ditLength = 3000.;  // +++ should be 3000
  else setCWSpeeed(DEFAULT_WPM);
  cwFrequency = frequency;
}

void Morse::setFrequency(unsigned long frequency)
{   
  cwFrequency = frequency;
}

// Set the speed for the cw mode
void Morse::setCWSpeeed(int wpm)
{

  ditLength = 1200. / wpm;
  
}

// Set the speed for the QRSS mode
void Morse::setQRSSpeed(QRSSMode Qmode)
{
switch (Qmode){
case QRSS3:
  ditLength = 3000.;  
  break;
case QRSS6:
  ditLength = 6000.;
  break;
case QRSS10:
  ditLength = 10000.;
  break;
default:
  ditLength = 60000.;
}
}

// send a full message
void Morse::sendText( char msg[])
{
  rf_on();  // initialize Si5351
  if (mode == CW) rf_off(); // Disable Si5351 outputs
  for (int i = 0; msg[i] != '\0'; i++)
  {
    if (msg[i] == ' ')
    {
      wait(ditLength * 7); // Word gap
      continue;
    }
    const char *morse = lookupCharacter(msg[i]);
    if (morse != "")
    {
      sendCharacter(morse);
      wait(ditLength * 3); // Letter gap
    }
  }
  rf_off();
}
const char *Morse::lookupCharacter(char c)
{
  c = toupper(c);
  //Serial.print(c);
  for (int i = 0; i < MORSE_TABLE_SIZE; i++)
  {
    if (morseTable[i].letter == c)
    {
      return morseTable[i].morse;
    }
  }
  return ""; // Not found
}

// send a full character
void Morse::sendCharacter(const char *morse)
{
  for (int i = 0; morse[i] != '\0'; i++)
  {
    sendSymbol(morse[i]);
  }
}


// send a dot or dash
void Morse::sendSymbol(char symbol)
{

  if (symbol == '.')
  {
    digitalWrite(DBGPIN, HIGH);
    
    if (mode == QRSS)
      sendTone((cwFrequency + QRSS_OSET / 2.), ditLength); // call the Si5351 interface

    else
      sendDitDah(cwFrequency, ditLength); // call the Si5351 interface

    digitalWrite(DBGPIN, LOW);
    //Serial.print(".");
  }
  else if (symbol == '-')
  {
    digitalWrite(DBGPIN, HIGH);

    if (mode == QRSS)
      sendTone((cwFrequency + QRSS_OSET / 2.), ditLength * 3);

    else
      sendDitDah(cwFrequency, ditLength * 3);

    digitalWrite(DBGPIN, LOW);
    //Serial.print("-");
  }
  wait(ditLength); // Intra-character gap
}

// Provide delays between dits dahs letters and words
void Morse::wait(int duration)
{
  // spaces between letters and words
  unsigned long start_time = millis();
    if (mode == QRSS)
    {
      sendTone((cwFrequency - QRSS_OSET / 2.), duration); // call the Si5351 interface
    }
  while (millis() - start_time < duration)
  {
    // Wait
    yield(); // Allow other tasks to run
  }

}


