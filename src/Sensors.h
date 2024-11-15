//#define PICO


//#ifdef PICO
// #include <PicoAnalogCorrection.h>
// PicoAnalogCorrection pico;
// const uint8_t GND_PIN = A1;  // GND meas pin
// const uint8_t VCC_PIN = A0;  // VCC meas pin
// const uint8_t ADC_RES = 12;  // ADC bits
// const float VREF = 3.0;      // Analog reference voltage

// bool sensorFirstTime = true;
// void sensorSetup()
// {
//   if (sensorFirstTime == false) return;
//   sensorFirstTime = false;

//   pinMode(GND_PIN, INPUT);
//   pinMode(VCC_PIN, INPUT);

//   analogReadResolution(ADC_RES);

  // Needs to be called only if ADC_RES changes after the initial declaration of PicoAnalogCorrection.
  // You can also use this method instead of the stock analogReadResolution() version if you want to change the
  // resolution on the go. This will automatically call analogReadResolution(ADC_RES) too.
  // pico.analogReadResolution(ADC_RES);

  // Calibrate ADC using an average of 5000 measurements
  //pico.calibrateAdc(GND_PIN, VCC_PIN, 5000);

// }
// PicoAnalogCorrection pico(ADC_RES, VREF);
// #else
// #include <TemperatureZero.h>  // SAMD@! temperature Lib
// void sensorSetup() 
// {return;}
// #endif

bool tempInit = false;

float getTempCPU()
{  // processor internal temperature

float temp = 0.;


  #ifdef PICO
  //temp = analogReadTemp() ;
  #else

  TemperatureZero TempZero = TemperatureZero();
  TempZero.init();
  temp = TempZero.readInternalTemperature();
  #endif

    return temp;
}

int readVcc() 
{  

  int result = 0.;
  #ifdef PICO

  unsigned int sensorValue = analogRead(29);
  result = 3*sensorValue*3.3/(1<<12);
  #else
	unsigned int sensorValue = analogRead(PANEL_VOLTS);
  result = sensorValue * 11.68;
  #endif
  return result;
}
