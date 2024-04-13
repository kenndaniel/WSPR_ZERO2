


void PPSinterrupt()
{
   // Calibration function that counts SI5351 clock 2 for 10 seconds
  // Called on every pulse per second after gps sat lock
 
  tcount++;
 
/*   if (tcount % 2 == 0)
  {
   digitalWrite(DBGPIN, HIGH);
  }
  else
  {digitalWrite(DBGPIN, LOW);} */

  if (CalibrationDone == true) return;
  if (tcount == 4)  // Start counting the 2.5 MHz signal from Si5351A CLK
  {
    TC4->COUNT32.CTRLBSET.reg = TC_CTRLBSET_CMD_RETRIGGER;   // Retrigger the TC4 timer
    while (TC4->COUNT32.STATUS.bit.SYNCBUSY);                // Wait for synchronization
  }
  else if (tcount == 5) {SiCnt=TC4->COUNT32.COUNT.reg;}
  else if (tcount == 15)  //The 10 second counting time has elapsed - stop counting
  {     

    XtalFreq=TC4->COUNT32.COUNT.reg -SiCnt; 
    correction = ((float)SI5351_XTAL/(float)XtalFreq) ;     
    // I found that adjusting the transmit freq gives a cleaner signal than setting ppb

    CalibrationDone = true;                  
  }
             
}


#define EIC_EVCtrl EIC_EVCTRL_EXTINTEO4 // Enable event output on external interrupt 4 
#define EIC_Config EIC_CONFIG_SENSE4_HIGH // Set event detecting a HIGH level on interrupt 4
#define EIC_IntenClr EIC_INTENCLR_EXTINT4 // Disable interrupts on interrupt 4 

void Si5351InterruptSetup()
{
   // Generic Clock 
  GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |                 // Enable the generic clock
                      GCLK_CLKCTRL_GEN_GCLK0 |             // on GCLK0 at 48MHz
                      GCLK_CLKCTRL_ID_TC4_TC5;             // Route GCLK0 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);                       // Wait for synchronization

  // Enable the port multiplexer on digital pin COUNTER_PIN
  PORT->Group[g_APinDescription[COUNTER_PIN].ulPort].PINCFG[g_APinDescription[COUNTER_PIN].ulPin].bit.PMUXEN = 1;
  // Set-up the pin as an EIC (interrupt) peripheral on D1
  PORT->Group[g_APinDescription[COUNTER_PIN].ulPort].PMUX[g_APinDescription[COUNTER_PIN].ulPin >> 1].reg |= PORT_PMUX_PMUXO_A;

  // External Interrupt Controller (EIC)
  EIC->EVCTRL.reg |= EIC_EVCTRL_EXTINTEO4;                 // Enable event output on external interrupt
  EIC->CONFIG[0].reg |= EIC_CONFIG_SENSE4_HIGH;            // Set event detecting a HIGH level on interrupt 
  EIC->INTENCLR.reg = EIC_INTENCLR_EXTINT4;                // Disable interrupts on interrupt x
  EIC->CTRL.bit.ENABLE = 1;                                // Enable the EIC peripheral
  while (EIC->STATUS.bit.SYNCBUSY);                        // Wait for synchronization

  // Event System
  PM->APBCMASK.reg |= PM_APBCMASK_EVSYS;                                  // Switch on the event system peripheral
  EVSYS->USER.reg = EVSYS_USER_CHANNEL(1) |                               // Attach the event user (receiver) to channel 0 (n + 1)
                    EVSYS_USER_USER(EVSYS_ID_USER_TC4_EVU);               // Set the event user (receiver) as timer TC4
  EVSYS->CHANNEL.reg = EVSYS_CHANNEL_EDGSEL_NO_EVT_OUTPUT |               // No event edge detection
                       EVSYS_CHANNEL_PATH_ASYNCHRONOUS |                  // Set event path as asynchronous
                       EVSYS_CHANNEL_EVGEN(EVSYS_ID_GEN_EIC_EXTINT_4) |   // Set event generator (sender) as external interrupt 4
                       EVSYS_CHANNEL_CHANNEL(0);                          // Attach the generator (sender) to channel 0                                 
 
  // Timer Counter TC4
  TC4->COUNT32.EVCTRL.reg |= TC_EVCTRL_TCEI |              // Enable asynchronous events on the TC timer
                             TC_EVCTRL_EVACT_COUNT;        // Increment the TC timer each time an event is received
  TC4->COUNT32.CTRLA.reg = TC_CTRLA_MODE_COUNT32;          // Configure TC4 together with TC5 to operate in 32-bit mode
  TC4->COUNT32.CTRLA.bit.ENABLE = 1;                       // Enable TC4
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY);                // Wait for synchronization
  TC4->COUNT32.READREQ.reg = TC_READREQ_RCONT |            // Enable a continuous read request
                             TC_READREQ_ADDR(0x10);        // Offset of the 32-bit COUNT register
  while (TC4->COUNT32.STATUS.bit.SYNCBUSY); 

  // setup pulse per second GPS interrupt
  //pinMode(interruptPinPPS, INPUT_PULLUP); 
  //attachInterrupt(digitalPinToInterrupt(interruptPinPPS), PPSinterrupt, RISING);               // Wait for synchronization
  //si5351_calibrate_init();
  //Serial.println(" Si5351 calibration setup complete ");

}
  
