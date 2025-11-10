// For BME 280 sensor of temperature, pressure and altitude

#include <BMP280_DEV.h> // Include the BMP280_DEV.h library
float fbmeTemp = 0.0;

float fbmePressure = 0.0;

float fbmeAltitude = 0.0;

float BMEGetPressurehPa()
{ // Pressure in hPa
	return fbmePressure;
}

float BMEGetTempC()
{ // Temperature in C
	return fbmeTemp;
}

float BMEGetAltitudeMeters()
{ // Altitude in m
	return fbmeAltitude;
}

float temperature, pressure, altitude; // Create the temperature, pressure and altitude variables
BMP280_DEV bmp280;					   // Instantiate (create) a BMP280_DEV object and set-up for I2C operation (address 0x77)

void BME280TakeData()
{						// Initialise the serial port
	bmp280.begin(0x76); // Default initialisation, place the BMP280 into SLEEP_MODE
	// bmp280.setPresOversampling(OVERSAMPLING_X4);    // Set the pressure oversampling to X4
	// bmp280.setTempOversampling(OVERSAMPLING_X1);    // Set the temperature oversampling to X1
	// bmp280.setIIRFilter(IIR_FILTER_4);              // Set the IIR filter to setting 4
	bmp280.startForcedConversion(); // Start BMP280 forced conversion (if we're in SLEEP_MODE)
	int period = 50;				// From trial and error. Usually takes 32

	for (int i = 0; i < period; i++)
	{
		delay(1);
		if (bmp280.getMeasurements(fbmeTemp, fbmePressure, fbmeAltitude)) // Check if the measurement is complete
		{
			return;
		}
	}
	Serial.println(" BMP NO Data ****** ");
}

void TestBME()
{
	Serial.println(F(" BME280 Sensor Test"));

	BME280TakeData();

	Serial.print(" Temp C ");
	Serial.print(BMEGetTempC());
	Serial.print(" Pressure hPa ");
	Serial.print(BMEGetPressurehPa());
	Serial.print(" altitude m ");
	Serial.println(BMEGetAltitudeMeters());
	if ((BMEGetPressurehPa() - 970.) < 100. && BMEGetTempC() - 21. < 10)
	{
		Serial.println(" BMP Test Passes");
	}
	else
	{
		Serial.println(" BMP Test Failed *************");
	}
}
