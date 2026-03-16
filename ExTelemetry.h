#include "WsprEncoded.h"

#include <cstdint>

using namespace std;

void charArrayCpy(char dest[], char src[], int cnt);

void ExTelemEncode1()
{
    // Create User-Defined Telemetry object for the number of
    // fields you want, maximum of 29 1-bit fields possible.
    WsprMessageTelemetryExtendedUserDefined<2> codecGpsMsg;
    //Serial.begin(9600);
    //delay(5000);
    //Serial.println("Starting");
    /////////////////////////////////////////////////////////////////
    // Define telemetry fields
    /////////////////////////////////////////////////////////////////. 

    // name, low value, high value, resolution
    // The total number of values must be below 165
   // codecGpsMsg.DefineField("MSAltitude",    240, 15000, 30);
    codecGpsMsg.DefineField("LM75Temp", -600, 300, 2);
    codecGpsMsg.DefineField("Pressure", 1000, 4000, 1);

// Example Message Definition -- modify then save!
/* 
{ "name": "LM75Temp",  "unit": "C",      "lowValue":   -600,    "highValue": 300,    "stepSize": 2   },
{ "name": "Pressure",  "unit": "dPa", "lowValue":  1000,    "highValue": 4000,    "stepSize":  1   },
 */
    /////////////////////////////////////////////////////////////////
    // Set fields (based on GPS data sourced elsewhere)
    /////////////////////////////////////////////////////////////////


    // codecGpsMsg.Set("LM75Temp",  -45.);      
    //  codecGpsMsg.Set("Pressure",  225.);

    float pressure = MS5611GetPressure();
    float LM75Temp = LM75GetTemperature();
    //pressure = 200.; // for testing only

    codecGpsMsg.Set("LM75Temp",   LM75Temp*10);      
    codecGpsMsg.Set("Pressure",  pressure*10); 

    // Serial.println( "=== Telemetry Fields ==="  );
    // Serial.print("LM75Temp: ");
    // Serial.println(LM75Temp*10);
    // Serial.print("Pressure: ");
    // Serial.println(pressure*10);

    /////////////////////////////////////////////////////////////////
    // Look up channel details for use in encoding
    /////////////////////////////////////////////////////////////////

    // Configure band and channel
    const char *band    = "15m";
    uint16_t    channel = 463;

    // Get channel details
    WsprChannelMap::ChannelDetails cd = WsprChannelMap::GetChannelDetails(band, channel);


    /////////////////////////////////////////////////////////////////
    // Define HdrSlots --- 
    // HdrSlot 0 is typically callsign (Traquito Chanel Map min is the start time for this msg) 
    // HdrSlot 1 is typically Basic Telemetry (U4B or WB8ELK)
    // HdrSlot 2 is where to send the first ET. (Display slot 3 Traquito, display slot 2 for WSPRTV)
    /////////////////////////////////////////////////////////////////

    uint8_t slot = 2; // Hdrslot 
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
    // Serial.println( "=== Encoded 1 =="  );
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
    WsprMessageTelemetryExtendedUserDefined<3> codecGpsMsg;
    //Serial.begin(9600);
    //delay(5000);
    //Serial.println("Starting");
    /////////////////////////////////////////////////////////////////
    // Define telemetry fields
    /////////////////////////////////////////////////////////////////. 

    // name, low value, high value, resolution

    codecGpsMsg.DefineField("GPSAlt", 0, 20, 1);
    codecGpsMsg.DefineField("MS5611Temp", -600, 300, 2);
    codecGpsMsg.DefineField("AltDiff", -400, 400, 5);   

// Example Message Definition -- modify then save!
/* 
{ "name": "GPSAltFine",     "unit": "m",   "lowValue":   0,    "highValue": 20,    "stepSize": 1   },
{ "name": "MS5611Temp",     "unit": "C",   "lowValue":   -600,    "highValue": 300,    "stepSize": 2   },
{ "name": "AltDiff",     "unit": "m",  "lowValue":   -400,    "highValue":   400,    "stepSize":  5   },
 */
    /////////////////////////////////////////////////////////////////
    // Set fields (based on GPS data sourced elsewhere)
    /////////////////////////////////////////////////////////////////


    // codecGpsMsg.Set("GPS Alt",  13561);      
    // codecGpsMsg.Set("Pres Frac",  6);
    // codecGpsMsg.Set("Alt Diff",  151);

    // codecGpsMsg.Set("Temperature",    -32.4); 
    
   // codecGpsMsg.Set("MSAltitude",  MS5611GetAltitude());
    float temp = MS5611GetTemperature();
    int gpsAltFine = (int)gpsAltitude%20;
    float altDif = gpsAltitude - MS5611GetAltitude();
    codecGpsMsg.Set("GPSAlt",  gpsAltFine);      
    codecGpsMsg.Set("MS5611Temp",  temp*10.);
    codecGpsMsg.Set("AltDiff",  altDif);

    // Serial.println( "=== Telemetry Fields ==="  );
    // Serial.print("GPSAlt: ");
    // Serial.println(gpsAltFine);
    // Serial.print("MS5611Temp: ");
    // Serial.println(temp*10.);
    // Serial.print("AltDiff: ");
    // Serial.println(altDif);

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
    // Slot 1 is typically callsign (sent automatically)
    // Slot 2 is typically Basic Telemetry (sent automatically)
    // Slot 3+ is where most people send ET.
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
    // Serial.println( "=== Encoded 2 =="  );
    // Serial.print("Slot: ");
    // Serial.println(slot);

    // Serial.print("Callsign: ");
    // Serial.println(call_telemetry);
    // Serial.print("Grid4   : ");
    // Serial.println(loc4_telemetry);
    // Serial.print("PowerDbm: ");
    // Serial.println(dbm_telemetry);


}



