

#include <Wire.h>
#include <MS5611.h>
#include "Adafruit_SHT4x.h"

// ADC Constants
#define MCP3221_ADDR 0x4D     // MCP3221A5T I2C address
#define VOLTAGE_REFERENCE 3.3 // Reference voltage in volts
//#define MS5611_Temperature  // define this if using the 5611 to read temperature

// Pressure Sensor
MS5611 MS5611(0x77);
uint32_t start, stop;

// Humidity Sensor
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

float mHumidity = 0.;
float mPressure = 0.;
float mTemperature = 0.;

void initSensors()
{
  // Prints Header File
  // Serial.println("Pressure(hPa),Humidity(%),Temperature(C)");
  Wire.begin();

  // Pressure Sensor Startup
  MS5611.begin();
  MS5611.setOversampling(OSR_ULTRA_HIGH);

  // Adafruit below Humidity Sensor Startup
  sht4.begin();
  sht4.setPrecision(SHT4X_HIGH_PRECISION);
  sht4.setHeater(SHT4X_MED_HEATER_100MS);
}

void readSensors()
{
  mHumidity = 0.;
  mPressure = 0.;
  mTemperature = 0.;

  float sHumidity = 0.;
  float sPressure = 0.;
  float sTemperature = 0.;

  int numMesurments = 10; // number of measurements to be averaged
  int duration = 500;     // wait between measurements

  for (int i = 0; i < numMesurments; ++i)
  {

    // Request data from MS5611
    MS5611.read();
    sPressure = MS5611.getPressure();
    if (sPressure > 999.9)
    {
      sPressure = 999.9;
    }
    else if (sPressure < 0)
    {
      sPressure = 0;
    }
    //Serial.print(sPressure, 2);
    //Serial.print(',');

    // Request data from SHT42
    sensors_event_t humidity, temp;
    sht4.getEvent(&humidity, &temp);
    sHumidity = humidity.relative_humidity;
    if (sHumidity > 99.9)
    {
      sHumidity = 99.9;
    }
    else if (sHumidity < 0)
    {
      sHumidity = 0;
    }
    //Serial.print(sHumidity, 1);
    //Serial.print(',');

    float solarVolts = readVcc();
    // Request data from MCP3221A5T
    Wire.beginTransmission(MCP3221_ADDR);
    Wire.endTransmission();
    Wire.requestFrom(MCP3221_ADDR, 2);
    if (Wire.available() >= 2 and solarVolts > 3.5)
    {
      byte msb = Wire.read();
      byte lsb = Wire.read();
      // Combine the MSB and LSB to get the 12-bit ADC value
      int adcValue = ((msb & 0x0F) << 8) | lsb;
      // Convert ADC value to voltage
      float voltage = (adcValue * VOLTAGE_REFERENCE) / 4095.0;
      // Convert voltage to resistance
      float r = 10000 * ((VOLTAGE_REFERENCE / voltage) - 1);
      // Convert resistance to temperature
      float thermtemp = 1.0 / (0.8237883661e-3 + 2.642550066e-4 * log(r) + 1.24004296e-7 * pow(log(r), 3)) - 273.15;
      sTemperature = thermtemp;

      #ifdef MS5611_Temperature
      sTemperature = MS5611.getTemperature();
      #endif

      if (sTemperature > 25.)
      {
        sTemperature = 25.;
      }
      else if (sTemperature < -75.)
      {
        sTemperature = -75.;
      }
      //Serial.println(sTemperature, 1);
    }
    else
    {
      sTemperature = 25.; // Sensor or voltage reference is bad
    }

    mHumidity += sHumidity;
    mPressure += sPressure;
    mTemperature += sTemperature;
    delay(duration);
  }
  mHumidity = mHumidity / numMesurments;
  mPressure = mPressure / numMesurments;
  mTemperature = mTemperature / numMesurments;
}

float getTemperature()
{ // get the value when converting data to callsign in CodeTelemetryMessage.h
  return mTemperature;
}

float getPressure()
{ // get the value when converting data to callsign in CodeTelemetryMessage.h
  return mPressure;
}

float getHumidity()
{ // get the value when converting data to callsign in CodeTelemetryMessage.h
  return mHumidity;
}