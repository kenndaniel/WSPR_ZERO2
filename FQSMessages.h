char FSQdata[130];
int FXQMsgMaxLength = 130; // maximum length of FSQ message
void HFDataInit()
{ // Initialize the FSQdata string with the data type in the first position
  // Data type is a user defined character designating the type of data to follow
  FSQdata[0] = '\0';
}

bool HFDataAppendFloat(float value)
{ // Append a comma and then float value to the end of the data string.
  String str = (String)value;
      if (str.length()+strlen(FSQdata) > FXQMsgMaxLength)
    return false;
  str = (String)FSQdata + (String) "," + str;
  strcpy(FSQdata, str.c_str());
  return true;
}

bool HFDataAppendInt(int value)
{ // Append a comma and then the int to the end of the FSQdata string.
  String str = (String)value;
    if (str.length()+strlen(FSQdata) > FXQMsgMaxLength)
    return false;
  str = (String)FSQdata + (String) "," + str;
  strcpy(FSQdata, str.c_str());
  return true;
}

bool HFDataAppendChars(char *value)
{ // Append a comma and then the char string to the end of the FSQdata string.
  String str = (String)value;
      if (str.length()+strlen(FSQdata) > FXQMsgMaxLength)
    return false;
  str = (String)FSQdata + (String) "," + str;
  strcpy(FSQdata, str.c_str());
  return true;
}

char * FQSMessage(float fsqType)
{ 
  // a comma will be appended betweek each data item
  HFDataInit();
   HFDataAppendInt((int)month()); // month
   HFDataAppendInt((int)day()); // day
   HFDataAppendInt((int)hour()); // hr
   HFDataAppendInt((int)minute()); // min
   HFDataAppendChars(" FSQ");
   HFDataAppendFloat(fsqType); // FSQ type
  //HFDataAppendFloat(LM75GetTemperature()); // Temperature LM75
   HFDataAppendChars(" HI from Sunset Ridge Elementary School"); // Greeting
   HFDataAppendFloat(gpsAltitude);  // Altitude GPS
   //HFDataAppendFloat(gpsCourse);  // GPS Course
   HFDataAppendFloat(gpsSpeed);  // GPS speed kph
   HFDataAppendChars(loc8); // Madenhead
   HFDataAppendChars(" KD9TVR SK"); 
   
  //Serial.println(FSQdata);
  return FSQdata;
}

