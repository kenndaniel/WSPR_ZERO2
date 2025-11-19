#include "WsprEncoded.h"

#include <cstdint>

using namespace std;

void charArrayCpy(char dest[], char src[], int cnt);

void ExTelemEncode2()
{
    // Create User-Defined Telemetry object for the number of
    // fields you want, maximum of 29 1-bit fields possible.
    WsprMessageTelemetryExtendedUserDefined<5> codecGpsMsg;
    Serial.begin(9600);
    delay(5000);
    Serial.println("Starting");
    /////////////////////////////////////////////////////////////////
    // Define telemetry fields
    /////////////////////////////////////////////////////////////////

    // name, low value, high value, resoluton
    codecGpsMsg.DefineField("MSAltitude",    240, 15000, 30);
    codecGpsMsg.DefineField("GPSAltitude",  240, 15000, 30);
    codecGpsMsg.DefineField("Pressure", 200, 1000, 20);
    codecGpsMsg.DefineField("Temperature",    -40, 20, 1);



    /////////////////////////////////////////////////////////////////
    // Set fields (based on GPS data sourced elsewhere)
    /////////////////////////////////////////////////////////////////


    // codecGpsMsg.Set("MSAltitude",  12330.);
    // codecGpsMsg.Set("GPSAltitude",  12680.);      
    // codecGpsMsg.Set("Pressure",  225.);
    // codecGpsMsg.Set("Temperature",    -32.4); 
    
    codecGpsMsg.Set("MSAltitude",  MS5611GetAltitude());
    codecGpsMsg.Set("GPSAltitude",  gpsAltitude);      
    codecGpsMsg.Set("Pressure",  MS5611GetPressure());
    codecGpsMsg.Set("Temperature",  MS5611GetTemperature()); 

    /////////////////////////////////////////////////////////////////
    // Look up channel details for use in encoding
    /////////////////////////////////////////////////////////////////

    // Configure band and channel
    const char *band    = "20m";
    uint16_t    channel = 123;

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
    codecGpsMsg.SetId13(std_telemID);   // "06"
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

    Serial.println( "Encoded data"  );
    Serial.println("------------");
    Serial.print("Callsign: ");
    Serial.println(callsign);
    Serial.print("Grid4   : ");
    Serial.println(grid4);
    Serial.print("PowerDbm: ");
    Serial.println(powerDbm);
    Serial.print("Slot: ");
    Serial.println(slot);

    Serial.print("Callsign: ");
    Serial.println(call_telemetry);
    Serial.print("Grid4   : ");
    Serial.println(loc4_telemetry);
    Serial.print("PowerDbm: ");
    Serial.println(dbm_telemetry);


}



