
//#include "./src/TemperatureZero.h" // for reading the cpu internal temperature
//TemperatureZero Temp = TemperatureZero();
#include <TemperatureZero.h>  // SAMD@! temperature Lib


bool tempInit = false;

float getTempCPU()
{  // processor internal temperature
float temp = 0.;

// code for Arduino mini/uno
    // int wADC;
    // ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
    // ADCSRA |= _BV(ADEN);
    // delay(20);
    // ADCSRA |= _BV(ADSC);
    // while (bit_is_set(ADCSRA, ADSC));
    // wADC = ADCW;
    // temp = (wADC - 322.2) / 1.43;

// code for SAMD21
  //Temp.init();
  //temp = Temp.readInternalTemperature();

  TemperatureZero TempZero = TemperatureZero();
  TempZero.init();
  temp = TempZero.readInternalTemperature();

    return temp;
}

int readVcc() 
{  //Read voltage in mV 5000 = 5v
	unsigned int sensorValue = analogRead(PANEL_VOLTS);
  int result = sensorValue * 11.68;
  return result;
}
