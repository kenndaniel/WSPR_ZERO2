/*
Manage EEPROM Storage
*/

#ifdef NIBBB
#include <FlashStorage_SAMD.h> // https://github.com/khoih-prog/FlashStorage_SAMD
#endif

#ifdef PICO
#include <EEPROM.h> //https://arduino-pico.readthedocs.io/en/latest/eeprom.html
#endif

bool firstStore = true;
int latitudeAddress = 0;
int longitudeAddress = latitudeAddress + sizeof(latitude);
int nextAddress = longitudeAddress + sizeof(longitude);


void storeLocation()
{
// Stores the gps location into eeprom
  #ifdef PICO
  if (firstStore == true) EEPROM.begin(512);
  #endif
  EEPROM.put(latitudeAddress, latitude); 
  EEPROM.put(longitudeAddress, longitude); 
  EEPROM.commit();
  
firstStore = false;
}

bool isLocationStoredEEPROM()
{ // used to determine if a valid gps location has been stored in eeprom
  return !firstStore;}

bool getLastLocation()
{
// sets the location to the last valid value 
// returns false if no valid gps location has been stored yet
if (firstStore == true) return false;  // no location haa yet been saved

EEPROM.get(latitudeAddress, latitude); // Retrieve latitude into global variable
EEPROM.get(longitudeAddress, longitude); 

return true;
}

void TestEEPROM()
{

bool set = getLastLocation();
Serial.println(" EEPROM software test  Test 1 ");
if (set == false) Serial.print("Pass"); else Serial.print("Fail");

double testLat = latitude; // save value of global variable for testing
double testLong = longitude;
storeLocation();  // global variables have been saved
latitude = 0; // change the globle variables
longitude = 0;
set = getLastLocation(); // set the globl variable from values stored in eeprom

Serial.print(" Test 2 ");
if (set == true) Serial.print("Pass"); else Serial.print("Fail");
Serial.print(" Test 3 ");
if (testLat == latitude) Serial.print("Pass"); else Serial.print("Fail");
Serial.print(" Test 4 ");
if (testLong == longitude) Serial.print("Pass"); else Serial.print("Fail ");
}