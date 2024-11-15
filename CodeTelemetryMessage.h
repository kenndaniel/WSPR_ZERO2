
//#include "CustomSensor.h"

code_u4b_telem_callsign()
{
// Definition of u4b encoding http://qrp-labs.com/flights/s4.html#protocol
// Example code https://traquito.github.io/pro/code/
// Encodes the callsign portion of the u4b telemetry message
  uint8_t grid5Val = loc6[4]-"A"; // 5th character of grid square
  uint8_t grid6Val = loc6[5] - 'A'; // 6th character of grid square
  double altM = gpsAltitude;
  if (gpsAltitude < 0)     { altM = 0;     }
  if (gpsAltitude > 21340) { altM = 21340; }

  uint16_t altFracM =  round(altM/ 20.);

    // convert inputs into a big number
      uint32_t val = 0;
      val *=   24; val += grid5Val;
      val *=   24; val += grid6Val;
      val *= 1068; val += altFracM;
          // extract into altered dynamic base
      uint8_t id6Val = val % 26; val = val / 26;
      uint8_t id5Val = val % 26; val = val / 26;
      uint8_t id4Val = val % 26; val = val / 26;
      uint8_t id2Val = val % 36; val = val / 36;
      // convert to encoded CallsignU4B
      char id2 = EncodeBase36(id2Val);
      char id4 = 'A' + id4Val;
      char id5 = 'A' + id5Val;
      char id6 = 'A' + id6Val;

    Callsign[0] =  std_telemID[0];   
		Callsign[1] =  id2;
		Callsign[2] =  std_telemID[1];
		Callsign[3] =  id4;
		Callsign[4] =  id5;
		Callsign[5] =  id6;
		Callsign[6] =  '\0';
}

code_u4b_telemetry_loc()
{
  // Definition of u4b encoding http://qrp-labs.com/flights/s4.html#protocol
// Encodes the locator and power value portion of the u4b telemetry message
  double tempC = getTempCPU();
  if (tempC > 39.)
    tempC = 39.;
  else if (tempC < -50.; )
  {
    tempC = 50.;
  }
  double voltage  = readVcc();
    if (voltage > 4.95)
    voltage = 4.95;
  else if (voltage < 3.; )
  {
    voltage = 3.;
  }
  double speed = gpsSpeedKnots;
  if (speed > 82 ) 
    speed = 82.;
  else if (speed < 0.)
  {
    speed = 0.
  }
  
       // map input presentations onto input radix (numbers within their stated range of possibilities)
        uint8_t tempCNum      = (uint8_t)(tempC + 50) ;
        uint8_t voltageNum    = ((uint8_t)round(((voltage * 100) - 300) / 5) + 20) / 40;
		    uint8_t speedNum = speed/2.;   //encoding # of sattelites into knots

        gpsValidNum=1; //changed sept 27 2024. because the traquito site won't show the 6 char grid if this bit is even momentarily off. Anyway, redundant cause sat count is sent as knots
		// shift inputs into a big number
        val = 0;
        val *= 90; val += tempCNum;
        val *= 40; val += voltageNum;
        val *= 42; val += speedNum;
        val *=  2; val += gpsValidNum;
        val *=  2; val += 1;          // standard telemetry (1 for the 2nd U4B packet, 0 for "Extended TELEN") - Thanks Kevin!
        // unshift big number into output radix values
        uint8_t powerVal = val % 19; val = val / 19;
        uint8_t g4Val    = val % 10; val = val / 10;
        uint8_t g3Val    = val % 10; val = val / 10;
        uint8_t g2Val    = val % 18; val = val / 18;
        uint8_t g1Val    = val % 18; val = val / 18;
        // map output radix to presentation
        char g1 = 'A' + g1Val;
        char g2 = 'A' + g2Val;
        char g3 = '0' + g3Val;
        char g4 = '0' + g4Val;
 	
		loc4_telemetry[0] = g1; 
		loc4_telemetry[1] = g2;
		loc4_telemetry[2] = g3;
		loc4_telemetry[3] = g4;
		loc4_telemetry[4] = '\0';

	dbm_telemetry=db[powerVal];

}

	void encode_telen2(telen_val1,telen_val2,telen_chars, &telen_power,int type) //converts two 32bit ints into 8 characters and one byte to be transmitted
	{
    	// TELEN packet  which has value 1 and value 2 (this same routine used for both telen#1 and telen#2). 
	// first value  gets encoded into the callsign (1st char is alphannumeric, and last three chars are alpha). Full callsign will be ID1, telen_char[0], ID3, telen_CHar[1],  telen_CHar[2], telen_CHar[3]. 
	// 2nd value gets encoded into GRID and power. grid = telen_CHar[4,5,6,7]. power = telen_power
	// max val of 1st one ~= 632k (per dave) [19 bits] i had originally thought 651,013 (if first char Z, which ~=35, times 17565(26^3). base 26 used, not base 36, because other chars must be only alpha, not alphanumeric because of Ham callsign conventions)
	// max val of 2nd one ~= 153k  (per dave) [17 bits] i had thought over 200k....

        uint32_t val = telen_val1;

		        // extract into altered dynamic base
        uint8_t id6Val = val % 26; val = val / 26;
        uint8_t id5Val = val % 26; val = val / 26;
        uint8_t id4Val = val % 26; val = val / 26;
        uint8_t id2Val = val % 36; val = val / 36;
        // convert to encoded CallsignU4B
        telen_chars[0] = EncodeBase36(id2Val);
        telen_chars[1] = 'A' + id4Val;
        telen_chars[2] = 'A' + id5Val;
        telen_chars[3] = 'A' + id6Val;
		
        val = telen_val2*4;  //(bitshift to the left twice to make room for gps bits at end)
      // unshift big number into output radix values
        uint8_t powerVal = val % 19; val = val / 19;
        uint8_t g4Val    = val % 10; val = val / 10;
        uint8_t g3Val    = val % 10; val = val / 10;
        uint8_t g2Val    = val % 18; val = val / 18;
        uint8_t g1Val    = val % 18; val = val / 18;
        // map output radix to presentation
        telen_chars[4] = 'A' + g1Val;
        telen_chars[5] = 'A' + g2Val;
        telen_chars[6] = '0' + g3Val;
        telen_chars[7] = '0' + g4Val;
        telen_chars[8]=0; //null terminate
 		

        char telemID[];
        if( type == 1 )
          telemID = TELEN1_telemID;
        else
        {  
          telemID = TELEN2_telemID;
          powerVal=powerVal+2;   //identifies it as the 2nd extended TELEN packet.  (this is the GPS-valid bit. note for extended TELEN we did NOT set the gps-sat bit)
        }

    Callsign[0] =  telemID[0];   //callsign: id13[0], telen char0, id13[1], telen char1, telen char2, telen char3
		Callsign[1] =  telen_chars[0];
		Callsign[2] =  telemID[1];	
		Callsign[3] =  telen_chars[1];
		Callsign[4] =  telen_chars[2];
		Callsign[5] =  telen_chars[3];
    Callsign[6] =  '\0';

    loc4_telemetry[0]=telen_chars[4];
    loc4_telemetry[1]=telen_chars[5];
    loc4_telemetry[2]=telen_chars[6];
    loc4_telemetry[3]=telen_chars[7];
    loc4_telemetry[4]= '\0'; 

    dbm_telemetry=db[powerVal];

  }

// void code_std_telem_characters(char Callsign[], float volts, float temp, int sats)
// {                               // compose the standard WB8ELK telemtry callsign -- convert values to characters and numbers
//   Callsign[0] = std_telemID[0]; // first part of telem call e.g. Q
//   Callsign[1] = codeStdPosition2(int(temp), sats);
//   Callsign[2] = std_telemID[1]; // second part of telem call e.g. 1
//   // Standard WB8ELK uses 3.3v to 5.8v for coding.
//   Callsign[3] = codeCharacterField(33, 58, int((volts / 100.)));
//   Callsign[4] = loc6[4]; // 5th character of grid square
//   Callsign[5] = loc6[5]; // 6th character of grid square
//   Callsign[6] = '\0';
// }

void code_standard_telemetry_callsign()
{
  float tempCPU = getTempCPU();
  float volts = readVcc();
  // code telemetry callsign
  Callsign[0] = std_telemID[0]; // first part of telem call e.g. Q
  Callsign[1] = codeStdPosition2(int(temp), sats);
  Callsign[2] = std_telemID[1]; // second part of telem call e.g. 1
  // Standard WB8ELK uses 3.3v to 5.8v for coding.
  Callsign[3] = codeCharacterField(33, 58, int((volts / 100.)));
  Callsign[4] = loc6[4]; // 5th character of grid square
  Callsign[5] = loc6[5]; // 6th character of grid square
  Callsign[6] = '\0';
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

  dbm_telemetry = codeFineFineAltitude(gpsAltitude+30);
}

/**************************************************************************/
// Special programming for high precision temperature, pressure and humitity

void code_high_precision_temp_pres_humid()
{
  float temp, pres, humidity;
  // temp = get_temperature();
  // pres = get_pressure();
  // humidity = get_humidity();
  // temp = -10.5;
  // pres = 233.1;
  // humidity = 2.1;

  temp = getTemperature();
  pres = getPressure();
  humidity = getHumidity();

  long int valueT = (temp + 75.) * 10.;
  long int valueP = (pres * 10.);
  long int valueh = (humidity * 10.);
  POUTPUTLN((F("Temperature Humidity Pressure  ")));
  POUTPUT((valueT));POUTPUT((F(", ")));POUTPUT((valueh));POUTPUT((F(", ")));POUTPUTLN((valueP));

  long int divid1 = valueh;
  //int divid1 =  valueP;

  long int divid2 = valueP * 1000 + valueT;
  long int remainder1 = 0;
  long int remainder2 = 0;
  int divisor[] = {19, 10, 10, 18, 18, 26, 26, 26, 10};
  int msg[9];
  char cmsg[] = "000000000";
  for (int i = 0; i < 9; i++)
  {

    // because of the restricted size of long int, division must be done in two parts
    remainder1 = divid1 % divisor[i];
    divid1 = divid1 / divisor[i];

    remainder2 = (remainder1 * 10000000 + divid2) % divisor[i];
    divid2 = (remainder1 * 10000000 + divid2) / divisor[i];

    msg[i] = remainder2;
  }

  uint8_t power = code_dbField(0, 18, msg[0]);

  cmsg[1] = codeNumberField(0, 9, msg[1]);
  cmsg[2] = codeNumberField(0, 9, msg[2]);
  cmsg[3] = codeCharacterField(0, 25, msg[3]);  // 25 is correct
  cmsg[4] = codeCharacterField(0, 25, msg[4]);
  cmsg[5] = codeCharacterField(0, 25, msg[5]);
  cmsg[6] = codeCharacterField(0, 25, msg[6]);
  cmsg[7] = codeCharacterField(0, 25, msg[7]);
  cmsg[8] = codeNumberField(0, 9, msg[8]);

  call_telemetry[0] = telemID[0]; // first part of telem message id  e.g. T
  call_telemetry[1] = telemID[1]; // second part of telem message id e.g. 1
  //call_telemetry[2] = cmsg[8]; 
  call_telemetry[2] = codeNumberField(0, 9, telemTrkID);  // Tracker identifier
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


int encodeSD(unsigned int a, unsigned int b) {
  // create a number using 2 bits from each number
  // numbers must be between 0 and 3
  if (a > 3) a = 3;
  if ( a < 0 ) a = 0;
  if (b > 3) b = 3;
  if ( b < 0 ) b = 0;
  
    // Mask the first two bits of a and b
    a = a & 0b00000011;
    b = b & 0b00000011;
    // Shift a left by 2 bits
    a = a << 2;
    //b = b << 2;
    // Combine a and b with bitwise OR
    return a | b;
}

void code_speed_direction_message()
{

char telemID[] = "T9";
//int telemTrkID = 9;  // balloon designator
//float speed = 143.9;  // Speed maximum value is 150.0 m/s
float speed = gps.speed.mps();
//float direction = 355.;  // degrees
float direction = gps.course.deg();

POUTPUTLN((F(" Speed, Direction ")));
 POUTPUT((speed));POUTPUT((F(", ")));POUTPUTLN((direction));


  long int divid1 = direction;

  unsigned int remainder1 = 0;

  int divisor[] = { 10, 10, 4, 26, 18, 4};
  int dmsg[3];
  char cmsg[] = "000000000";
  
 for (int i = 0; i < 3; i++)
  {
    // code direction 
    remainder1 = divid1 % divisor[i];
    divid1 = divid1 / divisor[i];

    dmsg[i] = remainder1;
  }

  int smsg[3];

  divid1 = speed*10;
 for (int i = 3; i < 6; i++)
  {
    // cpde speed
    remainder1 = divid1 % divisor[i];
    //Serial.println(divid1);
    divid1 = divid1 / divisor[i];

    smsg[i-3] = remainder1;
    //Serial.println(smsg[i-3]);
  }
int grid2;
// left two bits speed, right two bits direction
grid2 = encodeSD(smsg[2],dmsg[2]);

  //uint8_t power = code_dbField(0, 18, msg[0]);
  cmsg[0] = '>';
  cmsg[1] = codeNumberField(0, 9, dmsg[0]);
  cmsg[2] = codeNumberField(0, 9, dmsg[1]);
  cmsg[3] = codeCharacterField(0, 25, grid2);  // 25 is correct - max is 16
  cmsg[4] = codeCharacterField(0, 25, smsg[1]); // max is 18
  cmsg[5] = codeCharacterField(0, 25, smsg[0]);
  cmsg[6] = '\0';
  //Serial.println(cmsg);

  call_telemetry[0] = telemID2[0]; // first part of telem message id  e.g. T
  call_telemetry[1] = telemID2[1]; // second part of telem message id e.g. 1
  call_telemetry[2] = codeNumberField(0, 9, telemTrkID);  // Tracker identifier
  call_telemetry[3] = loc6[4];
  call_telemetry[4] = loc6[5];
  call_telemetry[5] = cmsg[5];
  call_telemetry[6] = '\0';


  loc4_telemetry[0] = cmsg[4];
  loc4_telemetry[1] = cmsg[3];
  loc4_telemetry[2] = cmsg[2];
  loc4_telemetry[3] = cmsg[1];
  loc4_telemetry[4] = '\0';
  
}

