/*
   GPS functions
*/
static const uint32_t GPSBaud = 9600;



void atgm336h_SingleGPSworkingMode()
{
  Serial1.write("$PCAS04,1*18\r\n"); //Sets navsystem of the ATGM to GPS only new code for ATGM335H 021922 BB
}

void SetHighAltitude()
{
  // put the gps in high altitude mode
  Serial1.write("$PMTK886,3*2B\r\n");
}

bool SetCPUClock(TinyGPSPlus gps)
{
  // Sets the cpu clock with the latest available GPS time

  byte h, m, s, mon, d;
  int y;
  unsigned long age;
  mon = gps.date.month();
  d = gps.date.day();
  y = gps.date.year();
  h = gps.time.hour();
  m = gps.time.minute();
  s = gps.time.second();
  // Serial.println(" d" + String(d) + " m" + String(mon) + " y" + String(y) + " a" + String(age));
  age = gps.time.age(); // Time since sat syncronization - add to sat time

  if (age > 1000 && age < (195000)) // limited to size of byte 255-60
  {
    s += (byte)(age / 1000);
    if (s >= 60)
    {
      byte mins = s / 60;
      s = s % 60;
      m += mins;
      if (m >= 60)
      { // not good for more than 1 hr
        m = m - 60;
        h += 1;
        if (h > 24)
          h = 0; // day might be off, but not important
      }
    }
  }
  // Conversion to real time clock on SAMD21 processor
  clock.setTime(h, m, s);
  clock.setDate(d, mon, y % 2000);

  // setTime((int)h, (int)m, (int)s, (int)d, (int)mon, (int)y);
  // if (timeStatus() != timeSet)
  // {
  //   POUTPUTLN((F(" Time not set")));
  //   return false;

  // }
  POUTPUTLN((s));
  POUTPUTLN((F("cpu time set - waiting for position")));
  return true;
}

int minute()
{
  // Serial.println(clock.getMinutes());
  return clock.getMinutes();
}

int second()
{
  // Serial.println(clock.getSeconds());
  return clock.getSeconds();
}

bool gpsSearch = true;
void beep()
{ // turn led on and off while searching for satellites
  if (gpsSearch)
  {
    digitalWrite(DBGPIN, HIGH);
    gpsSearch = false;
  }
  else
  {
    digitalWrite(DBGPIN, LOW);
    gpsSearch = true;
  }
}

// not needed for newer gps units
void gpsOn()
{
  rf_pwr_off();  // RF sometimes gets stuck on - make sure it is off
  digitalWrite(GPS_PWR, HIGH);
  digitalWrite(GPS_nRESET, HIGH);
}

void gpsOff()
{
  digitalWrite(GPS_PWR, LOW);
  rf_on();
}

void gpsBounce()
{
  //Turn GPS off and on
  gpsOff();
  delay(100);
  gpsOn();
}

void gps_reset()
{
  digitalWrite(GPS_nRESET, LOW);
  delay(1);
  digitalWrite(GPS_nRESET, HIGH);
  
  Serial1.write("$PCAS10,0*1D\r\n"); // software reset
}
// gps must lock position within 15 minutes or system will sleep or use the default location if the clock was set
unsigned long gpsTimeout = GPS_TIMEOUT; // in milliseconds
unsigned long gpsStartTime = 0;
long loopi = 0;
/*
  This function get gps data and makes it ready for transmission
*/
bool gpsGetData()
{
  gpsOn();
  bool clockSet = false, locSet = false, altitudeSet = false, speedSet = false;
#ifdef PICO
  UART ss(8, 9, NC, NC);
#endif

  Serial1.begin(GPSBaud);
  delay(100);
  gpsStartTime = millis();
  bool hiAltitudeSet = false;
  POUTPUTLN((F("Waiting for GPS to find satellites - 5-10 min")));
  OLEDrotate("Waiting for GPS Lock ", INFO);
  while (millis() < gpsStartTime + gpsTimeout)
  {
    // wdt_reset();
    while (Serial1.available() > 0)
      gps.encode(Serial1.read());

    if (gps.charsProcessed() > 10 && hiAltitudeSet == false)
    { // put the gps module into high altitude mode
      SetHighAltitude();
      // ss.write("$PMTK886,3*2B\r\n");
      hiAltitudeSet = true;
    }

    if (gps.time.isUpdated() && gps.satellites.value() > 0 && clockSet == false)
    {
      clockSet = true;
    }
    if (gps.altitude.isUpdated())
    {
      gpsAltitude = gps.altitude.meters();
      altitudeSet = true;
    }
    if (gps.speed.isUpdated())
    {
      gpsSpeed = gps.speed.kmph();
      gpsCourse = gps.course.deg();
      speedSet = true;
    }
    if (gps.location.isUpdated())
    {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
      locSet = true;
    }

    if (locSet && speedSet && altitudeSet && clockSet)
    {
      satellites = gps.satellites.value();
      POUTPUT((F(" Number of satellites found ")));
      POUTPUTLN((satellites));
      SetCPUClock(gps);
      randomSeed(millis() % 1000);
      // start transmission loop
      return true;
    }

    loopi++;

    if (loopi % 5000 == 0)
      beep(); // still looking for satellites

    if (gps.charsProcessed() < 10 && millis() % 1500 < 5)
    {

      POUTPUTLN((F("WARNING: No GPS data.  Check wiring.")));
      OLEDnoRotate(F("No Comm with GPS "),ERROR);
      // blink moris code "w" for wiring
      digitalWrite(DBGPIN, LOW);
      delay(150);
      digitalWrite(DBGPIN, HIGH);
      delay(150);
      digitalWrite(DBGPIN, LOW);
      delay(150);
      digitalWrite(DBGPIN, HIGH);
      delay(350);
      digitalWrite(DBGPIN, LOW);
      delay(150);
      digitalWrite(DBGPIN, HIGH);
      delay(350);
      digitalWrite(DBGPIN, LOW);
    }
    // If DEBUG_SI5351 is defined, the system will transmit, but not on the correct minute
    // Use this for unit testing when there is no gps attached.
#ifdef DEBUG_SI5351
    return true;
#endif
  }
  POUTPUTLN((F(" GPS Timeout - no satellites found ")));

  return false;
  /*  if(clockSet==true)
 {
   // Send report anyway if only the clock has been set
   clockSet = false; // needed for testing only
   locSet = false;
   altitudeSet = false;
   speedSet = false;
   return true;
 }
 else
 {
   return false;
 } */
}
