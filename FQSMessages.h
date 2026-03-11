char APRSdata[257];
void HFDataInit(char dtype)
{ // Initialize the APRSdata string with the data type in the first position
  // Data type is a user defined character designating the type of data to follow
  APRSdata[0] = dtype;
  APRSdata[1] = '\0';
}

bool HFDataAppendFloat(float value)
{ // Append a comma and then float value to the end of the data string.
  String str = (String)value;
  str = (String)APRSdata + (String) "," + str;
  if (str.length() > 250)
    return false;
  strcpy(APRSdata, str.c_str());
  return true;
}

bool HFDataAppendInt(int value)
{ // Append a comma and then the int to the end of the APRSdata string.
  String str = (String)value;
  str = (String)APRSdata + (String) "," + str;
  if (str.length() > 250)
    return false;
  strcpy(APRSdata, str.c_str());
  return true;
}

bool HFDataAppendChars(char *value)
{ // Append a comma and then the char string to the end of the APRSdata string.
  String str = (String)value;
  str = (String)APRSdata + (String) "," + str;
  if (str.length() > 250)
    return false;

  strcpy(APRSdata, str.c_str());
  return true;
}

char * FQSMessage()
{ 
  // a comma will be appended betweek each data item
  HFDataInit('B');
   HFDataAppendInt((int)month()); // month
   HFDataAppendInt((int)day()); // day
   HFDataAppendInt((int)hour()); // hr
   HFDataAppendInt((int)minute()); // min
   HFDataAppendChars(loc8); // Madenhead
  HFDataAppendFloat(LM75GetTemperature()); // Temperature LM75
   HFDataAppendFloat(MS5611GetTemperature()); // temperature MS5611
   HFDataAppendFloat(MS5611GetPressure()); // pressure MS5611
   HFDataAppendFloat(MS5611GetAltitude());  // Altitude MS5611
   HFDataAppendFloat(gpsAltitude);  // Altitude GPS
   //HFDataAppendFloat(gpsCourse);  // GPS Course
   HFDataAppendFloat(gpsSpeed);  // GPS speed kph
   HFDataAppendChars(" KI9LSI AR"); 
   
  //Serial.println(APRSdata);
  return APRSdata;
}

