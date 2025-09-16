//#define PICO


#ifdef PICO

void sensorSetup()
{
analogReadResolution(12);
}

#else
#include <TemperatureZero.h>  // SAMD21 temperature Lib
void sensorSetup() 
{return;}
#endif

bool tempInit = false;

float getTempCPU()
{  // Return temperature in C

float tempC = 0.;


  #ifdef PICO
  // processor internal temperature in C
  tempC = analogReadTemp(3.3f);


  #else // SAMD21

  TemperatureZero TempZero = TemperatureZero();
  TempZero.init();
  tempC = TempZero.readInternalTemperature();
  #endif

    return tempC;
}

int readVcc() 
{  
// Return the solar panel volts in mV e.g. 3700
  int result = 0.;
  #ifdef PICO
  //analogReadResolution(12);
  unsigned int sensorValue = analogRead(PANEL_VOLTS);
  result = 100*1000*sensorValue*3.3 / 65535;

  #else // SAMD21

	unsigned int sensorValue = analogRead(PANEL_VOLTS);
  result = sensorValue * 11.68;
  #endif

  return result;
}
