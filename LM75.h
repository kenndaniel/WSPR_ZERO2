/* 
Read the temperature from an LM75-derived temperature sensor, 
in Celcius. Any LM75-derived sensor should work.
https://github.com/jeremycole/Temperature_LM75_Derived/blob/master/doc/Sensors.md
 */

#include <Temperature_LM75_Derived.h>


Generic_LM75_11Bit LM75B; // provides .125 C accuracy - 11bit


float LM75GetTemperature()
{   // Temperatue in C
    return LM75B.readTemperatureC();
}

void LM75Test() {
  Serial.print(" LM75 Temperature = ");
  Serial.print(LM75B.readTemperatureC());
  Serial.println(" C");

  delay(250);
}