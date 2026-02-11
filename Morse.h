
#ifndef MORSE
#define MORSE

enum QRSSMode 
{
  QRSS3, // QRSS3 = ditLength 3 sec 
  QRSS6,
  QRSS10  // slowest
}; 

enum  morseMode 
{
  CW,
  QRSS
};

class Morse
{
public:
  Morse(morseMode mMode, unsigned long frequency);


  // Set the speed for the cw mode
  void setCWSpeeed(int wpm);

  // Set the speed for the QRSS mode
  void setQRSSpeed(QRSSMode Qmode);

  // send a full message
  void sendText(char msg[]);

  void setFrequency(unsigned long frequency);  // in Hz

private:
  morseMode mode = CW; 
  unsigned long cwFrequency = 21096350UL;     // Frequency in Hz 
  int ditLength = 3000;

  typedef struct
  {
    char letter;
    const char *morse;
  } MorseMap;

  // Lookup Morse representation for a character
  const char *lookupCharacter(char c);

  // Provide delays between dits dahs letters and words
  void wait(int duration);

  // send a dot or dash
  void sendSymbol(char symbol);

  // send a full character
  void sendCharacter(const char *morse);

  const int MORSE_TABLE_SIZE = 54;

    const MorseMap morseTable[54] = {
      {'A', ".-"}, {'B', "-..."}, {'C', "-.-."}, {'D', "-.."}, {'E', "."}, {'F', "..-."}, {'G', "--."}, {'H', "...."}, {'I', ".."}, {'J', ".---"}, {'K', "-.-"}, {'L', ".-.."}, {'M', "--"}, 
      {'N', "-."}, {'O', "---"}, {'P', ".--."}, {'Q', "--.-"}, {'R', ".-."}, {'S', "..."}, {'T', "-"}, {'U', "..-"}, {'V', "...-"}, {'W', ".--"}, {'X', "-..-"}, {'Y', "-.--"}, {'Z', "--.."}, 
      {'1', ".----"}, {'2', "..---"}, {'3', "...--"}, {'4', "....-"}, {'5', "....."}, {'6', "-...."}, {'7', "--..."}, {'8', "---.."}, {'9', "----."}, {'0', "-----"}, {'.', ".-.-.-"}, // Period
      {',', "--..--"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Comma
      {'?', "..--.."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Question mark
      {'\'', ".----."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                // Apostrophe
      {'!', "-.-.--"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Exclamation mark
      {'/', "-..-."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // Slash
      {'(', "-.--."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // Left parenthesis
      {')', "-.--.-"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Right parenthesis
      {'&', ".-..."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // Ampersand
      {':', "---..."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Colon
      {';', "-.-.-."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Semicolon
      {'=', "-...-"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // Equals
      {'+', ".-.-."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // Plus
      {'-', "-....-"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Hyphen / minus
      {'_', "..--.-"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Underscore
      {'"', ".-..-."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 // Quotation mark
      {'$', "...-..-"},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                // Dollar sign
      {'@', ".--.-."},                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  // At sign
  };
};

#endif
