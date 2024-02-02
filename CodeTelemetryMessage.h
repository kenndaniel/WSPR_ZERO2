
#include "CustomSensor.h"

void code_std_telem_characters(char Callsign[], float volts, float temp, int sats)
{                               // compose the standard WB8ELK telemtry callsign -- convert values to characters and numbers
  Callsign[0] = std_telemID[0]; // first part of telem call e.g. Q
  Callsign[1] = codeStdPosition2(int(temp), sats);
  Callsign[2] = std_telemID[1]; // second part of telem call e.g. 1
  // Standard WB8ELK uses 3.3v to 5.8v for coding.
  Callsign[3] = codeCharacterField(33, 58, int((volts / 100.)));
  Callsign[4] = loc6[4]; // 5th character of grid square
  Callsign[5] = loc6[5]; // 6th character of grid square
  Callsign[6] = '\0';
}

void code_standard_telemetry_callsign()
{
  // float tempCPU = getTempCPU();
  //float tempCPU = Temp.readInternalTemperature();
  float tempCPU = 0;
  float volts = readVcc();
  // code telemetry callsign
  code_std_telem_characters(call_telemetry, volts, tempCPU, satellites);
}

void code_telemety_loc()
{ // loc4_telemetry is coded into the 4 characters of the location field e.g. EN62
  // This can be changed to code sensor data.
  // The letters can take the value A-R the numbers 0-9
  charArrayCpy(loc4_telemetry, loc4, 4);
}

void code_telemetry_power()
{ // Altitude coded into the power field of the telemetry field

  dbm_telemetry = codeFineAltitude(gpsAltitude);
}

void code_characters(char Callsign[], float gpsSpeed)
{                           // compose the custom telemtry callsign and convert values to characters and number
  Callsign[0] = telemID[0]; // first part of telem call  e.g. T
  Callsign[1] = telemID[1]; // second part of telem call e.g. 1
  // The remainder of this message can be customized for different sensors
  int fineSpeed = int(gpsSpeed) % 8;
  Callsign[2] = codeNumberField(0, 9, fineSpeed); // third position must be a number
  Callsign[3] = codeCharacterField(0, 200, int(gpsSpeed));

  Callsign[4] = loc6[4]; // Example 5th character of grid square
  Callsign[5] = loc6[5]; // Example 6th character of grid square
  Callsign[6] = '\0';
  POUTPUTLN((F(Callsign)));
}

void code_custom_telemetry_callsign()
{
  float tempCPU = getTempCPU();

  // code telemetry callsign
  code_characters(call_telemetry, gpsSpeed);
}

void code_custom_telemetry_power()
{ // Altitude 0-60 is coded into the power field of the telemetry field

  dbm_telemetry = codeFineFineAltitude(gpsAltitude);
}

/**************************************************************************/
// Special programming for high precision temperature, pressure and humitity

void code_high_precision_temp_pres_humid()
{
  float temp, pres, humidity;
  // temp = get_temperature();
  // pres = get_pressure();
  // humidity = get_humidity();
  temp = -15.8;
  pres = 256.3;
  humidity = 3.6;

  int valueT = (temp + 80.) * 10;
  int valueP = (pres * 10);
  int valueh = (humidity * 10);

  valueT = 777;
  valueP = 9999;
  valueh = 888;

  int divid1 = valueP;
  int divid2 = valueh * 1000 + valueT;
  int remainder1 = 0;
  int remainder2 = 0;

  int divisor[] = {19, 10, 10, 18, 18, 26, 26, 26, 10};
  int msg[9];
  char cmsg[] = "000000000";
  for (int i = 0; i < 9; i++)
  {
    // because of the restricted size of int, division must be done in two parts
    remainder1 = divid1 % divisor[i];
    divid1 = divid1 / divisor[i];

    remainder2 = (remainder1 * 1000000 + divid2) % divisor[i];
    divid2 = (remainder1 * 1000000 + divid2) / divisor[i];

    msg[i] = remainder2;
  }

  uint8_t power = code_dbField(0, 18, msg[0]);

  cmsg[1] = codeNumberField(0, 9, msg[1]);
  cmsg[2] = codeNumberField(0, 9, msg[2]);
  cmsg[3] = codeCharacterField(0, 25, msg[3]);
  cmsg[4] = codeCharacterField(0, 25, msg[4]);
  cmsg[5] = codeCharacterField(0, 25, msg[5]);
  cmsg[6] = codeCharacterField(0, 25, msg[6]);
  cmsg[7] = codeCharacterField(0, 25, msg[7]);
  cmsg[8] = codeNumberField(0, 9, msg[8]);

  call_telemetry[0] = telemID[0]; // first part of telem call  e.g. T
  call_telemetry[1] = telemID[1]; // second part of telem call e.g. 1
  call_telemetry[2] = cmsg[8];
  call_telemetry[3] = cmsg[7];
  call_telemetry[4] = cmsg[6];
  call_telemetry[5] = cmsg[5];
  call_telemetry[6] = '\0';

  dbm_telemetry = power;

  loc4_telemetry[0] = cmsg[4];
  loc4_telemetry[1] = cmsg[3];
  loc4_telemetry[2] = cmsg[2];
  loc4_telemetry[3] = cmsg[1];
  loc4_telemetry[4] = '\0';
}
