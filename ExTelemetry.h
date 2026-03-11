#include "WsprEncoded.h"

#include <cstdint>

using namespace std;
float fracPressure = 0;  // decimal part of pressure 
void charArrayCpy(char dest[], char src[], int cnt);

void ExTelemEncode1()
{
    // Create User-Defined Telemetry object for the number of
    // fields you want, maximum of 29 1-bit fields possible.
    WsprMessageTelemetryExtendedUserDefined<5> codecGpsMsg;
    //Serial.begin(9600);
    //delay(5000);
    //Serial.println("Starting");
    /////////////////////////////////////////////////////////////////
    // Define telemetry fields
    /////////////////////////////////////////////////////////////////. 

    // name, low value, high value, resolution
    // The total number of values must be below 165
   // codecGpsMsg.DefineField("MSAltitude",    240, 15000, 30);
    codecGpsMsg.DefineField("LM75Temp", -60, 30, 1);
    codecGpsMsg.DefineField("Pressure", 1000, 5000, 1);
    codecGpsMsg.DefineField("MS5611Temp", -60, 30, 1);

// Example Message Definition -- modify then save!
/* 
{ "name": "LM75Temp",     "unit": "C",   "lowValue":   -60,    "highValue": 30,    "stepSize": 1   },
{ "name": "Pressure",      "unit": "hPa",    "lowValue":  1000,    "highValue":    5000,    "stepSize":  1   },
{ "name": "MS5611Temp",     "unit": "C",  "lowValue":   -60,    "highValue":   30,    "stepSize":  1   },
 */
    /////////////////////////////////////////////////////////////////
    // Set fields (based on GPS data sourced elsewhere)
    /////////////////////////////////////////////////////////////////


    // codecGpsMsg.Set("LM75Temp",  -45.);      
    //  codecGpsMsg.Set("Pressure",  225.);
    //  codecGpsMsg.Set("MS5611Temp",    -32.4);



    float pressure = MS5611GetPressure();
    fracPressure = pressure - (int)pressure; // extract the fractional part of the pressure
    float LM75Temp = LM75GetTemperature();
    float MS5611Temp = MS5611GetTemperature();
    //pressure = 200.; // for testing only

    codecGpsMsg.Set("LM75Temp",   LM75Temp);      
    codecGpsMsg.Set("Pressure",  pressure*10);
    codecGpsMsg.Set("MS5611Temp", MS5611Temp); 

    // Serial.print("LM75Temp: ");
    // Serial.println(LM75Temp);
    // Serial.print("Pressure: ");
    // Serial.println(pressure*10);
    // Serial.print("MS5611Temp: ");
    // Serial.println(MS5611Temp);
    /////////////////////////////////////////////////////////////////
    // Look up channel details for use in encoding
    /////////////////////////////////////////////////////////////////

    // Configure band and channel
    const char *band    = "15m";
    uint16_t    channel = 463;

    // Get channel details
    WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails(band, channel);


    /////////////////////////////////////////////////////////////////
    // slot is obtained from the chanel map time colum
    // 
    // - start minute = slot 0
    // - +2 min       = slot 1
    // - +4 min       = slot 2
    // - +6 min       = slot 3
    // - +8 min       = slot 4 
    /////////////////////////////////////////////////////////////////

    uint8_t slot = 2;
    /////////////////////////////////////////////////////////////////
    // Encode the data in preparation to transmit
    /////////////////////////////////////////////////////////////////

    //codecGpsMsg.SetId13(cd.id13);   // "Q6"
    codecGpsMsg.SetId13(std_telemID);   
    codecGpsMsg.SetHdrSlot(slot);
    codecGpsMsg.Encode();

    /////////////////////////////////////////////////////////////////
    // Extract the now-encoded WSPR message fields
    /////////////////////////////////////////////////////////////////

    const char *callsign = codecGpsMsg.GetCallsign();   // "036KVF".    
    const char *grid4    = codecGpsMsg.GetGrid4();      // "PP73"
    int         powerDbm = codecGpsMsg.GetPowerDbm();   // 30

    strcpy(call_telemetry, codecGpsMsg.GetCallsign());
    strcpy(loc4_telemetry,codecGpsMsg.GetGrid4());
    dbm_telemetry = codecGpsMsg.GetPowerDbm(); 

    // Serial.println( "Encoded data"  );
    // Serial.println("------------");
    // Serial.print("Callsign: ");
    // Serial.println(callsign);
    // Serial.print("Grid4   : ");
    // Serial.println(grid4);
    // Serial.print("PowerDbm: ");
    // Serial.println(powerDbm);
    // Serial.print("Slot: ");
    // Serial.println(slot);

    // Serial.print("Callsign: ");
    // Serial.println(call_telemetry);
    // Serial.print("Grid4   : ");
    // Serial.println(loc4_telemetry);
    // Serial.print("PowerDbm: ");
    // Serial.println(dbm_telemetry);


}

void ExTelemEncode2()
{
    // Create User-Defined Telemetry object for the number of
    // fields you want, maximum of 29 1-bit fields possible.
    WsprMessageTelemetryExtendedUserDefined<5> codecGpsMsg;
    //Serial.begin(9600);
    //delay(5000);
    //Serial.println("Starting");
    /////////////////////////////////////////////////////////////////
    // Define telemetry fields
    /////////////////////////////////////////////////////////////////. 

    // name, low value, high value, resolution

    codecGpsMsg.DefineField("GPSAlt", 8500, 14000, 1);
    codecGpsMsg.DefineField("PresFrac", 0, 100, 5);
    codecGpsMsg.DefineField("AltDiff", -3000, 3000, 5);   

// Example Message Definition -- modify then save!
/* 
{ "name": "GPSAlt",     "unit": "m",   "lowValue":   8500,    "highValue": 14000,    "stepSize": 1   },
{ "name": "PresFrac",      "unit": "hPa",    "lowValue":   0,    "highValue":    100,    "stepSize":  5   },
{ "name": "AltDiff",     "unit": "m",  "lowValue":   -3000,    "highValue":   3000,    "stepSize":  5   },
 */
    /////////////////////////////////////////////////////////////////
    // Set fields (based on GPS data sourced elsewhere)
    /////////////////////////////////////////////////////////////////


    // codecGpsMsg.Set("GPS Alt",  13561);      
    // codecGpsMsg.Set("Pres Frac",  6);
    // codecGpsMsg.Set("Alt Diff",  151);

    // codecGpsMsg.Set("Temperature",    -32.4); 
    
   // codecGpsMsg.Set("MSAltitude",  MS5611GetAltitude());

  float altDif = gpsAltitude - MS5611GetAltitude();
    codecGpsMsg.Set("GPSAlt",  gpsAltitude);      
    codecGpsMsg.Set("PresFrac",  100*fracPressure);
    codecGpsMsg.Set("AltDiff",  altDif);

    /////////////////////////////////////////////////////////////////
    // Look up channel details for use in encoding
    /////////////////////////////////////////////////////////////////

    // Configure band and channel
    const char *band    = "15m";
    uint16_t    channel = 462;

    // Get channel details
    WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails(band, channel);


    /////////////////////////////////////////////////////////////////
    // slot is obtained from the chanel map time colum
    // 
    // - start minute = slot 0
    // - +2 min       = slot 1
    // - +4 min       = slot 2
    // - +6 min       = slot 3
    // - +8 min       = slot 4 
    /////////////////////////////////////////////////////////////////

    uint8_t slot = 3;
    /////////////////////////////////////////////////////////////////
    // Encode the data in preparation to transmit          
    /////////////////////////////////////////////////////////////////

    //codecGpsMsg.SetId13(cd.id13);   // "Q6"
    codecGpsMsg.SetId13(std_telemID);   
    codecGpsMsg.SetHdrSlot(slot);
    codecGpsMsg.Encode();

    /////////////////////////////////////////////////////////////////
    // Extract the now-encoded WSPR message fields
    /////////////////////////////////////////////////////////////////

    const char *callsign = codecGpsMsg.GetCallsign();   // "036KVF"
    const char *grid4    = codecGpsMsg.GetGrid4();      // "PP73"
    int         powerDbm = codecGpsMsg.GetPowerDbm();   // 30

    strcpy(call_telemetry, codecGpsMsg.GetCallsign());
    strcpy(loc4_telemetry,codecGpsMsg.GetGrid4());
    dbm_telemetry = codecGpsMsg.GetPowerDbm(); 

    // Serial.println( "Encoded data"  );
    // Serial.println("------------");
    // Serial.print("Callsign: ");
    // Serial.println(callsign);
    // Serial.print("Grid4   : ");
    // Serial.println(grid4);
    // Serial.print("PowerDbm: ");
    // Serial.println(powerDbm);
    // Serial.print("Slot: ");
    // Serial.println(slot);

    // Serial.print("Callsign: ");
    // Serial.println(call_telemetry);
    // Serial.print("Grid4   : ");
    // Serial.println(loc4_telemetry);
    // Serial.print("PowerDbm: ");
    // Serial.println(dbm_telemetry);


}



