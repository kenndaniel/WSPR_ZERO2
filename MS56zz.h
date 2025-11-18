
#include <MS5611.h>

MS5611 MS5611(0x77);

float MS5611Pressure = 0.;
float MS5611Temperatue = 0.;
float MS5611Altitude = 0.;
int MS5611ErrorNo = 0;
bool MS5611OK = true;

float MS5611GetTemperature()
{   // Temperatue in C
    return MS5611Temperatue;
}

float MS5611GetPressure()
{   // pressure in hPa
    return MS5611Pressure;
}

float MS5611GetAltitude()
{   // Altitude in m
    return MS5611Altitude;
}


bool MS5611Init()
{
     Wire.begin();
  if (MS5611.begin() == true)
  {
    //Serial.print("MS5611 found: ");
    // Serial.println(MS5611.getAddress());
  }
  else
  {
    //Serial.println("MS5611 not found. halt.");
    MS5611ErrorNo = false;  // once set the code will not try to access the MS5611
    return false;

  }
  //  use adjusted math for pressure.
    MS5611.reset();
    MS5611.setOversampling(OSR_STANDARD);
    MS5611.setTemperatureOffset(-.71);
    MS5611.setPressureOffset(3.);
    return true;
}


bool MS5611TakeData()
{
  if (MS5611OK == false) return false;
    uint32_t start, stop;
    start = micros();
    MS5611ErrorNo = MS5611.read();
    stop = micros();

  if (MS5611ErrorNo != MS5611_READ_OK)
  {
    //Serial.print("Error in read: ");
    //Serial.println(MS5611ErrorNo);
    return false;
  }
  else
  {
    MS5611Temperatue = MS5611.getTemperature();
    MS5611Pressure = MS5611.getPressure();
    MS5611Altitude = MS5611.getAltitude();
    return true;
  }
}

void MS5611Test()
{
  MS5611Init();
  MS5611TakeData();

    float SeaLevel = MS5611.getSeaLevelPressure(MS5611Pressure,MS5611Altitude);
    Serial.print("T:\t");
    Serial.print(MS5611Temperatue, 2);
    Serial.print("\tP:\t");
    Serial.print(MS5611Pressure, 2);
    Serial.print("\t Altitude :\t");
    Serial.print(MS5611Altitude, 2);
    Serial.print("\t Sea Level :\t");
    Serial.print(SeaLevel, 2);
    Serial.print('\t');
    Serial.println();

}