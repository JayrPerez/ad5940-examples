#include "CalibrationMeasurement_FourWire.h"

/** Initialize Switches */
SWMatrixCfg_Type SWMatCfg_Init = 
{
  .Dswitch = SWD_OPEN,
  .Pswitch = SWP_OPEN,
  .Nswitch = SWN_OPEN,
  .Tswitch = SWT_OPEN | SWT_TRTIA,      // Connects T9 Switch
};

/** Initialize for the Calibration Routine */
void AD5940Juul_Initialization(void)
{
  AD5940_HWReset();                                     // Do hardware reset
  AD5940_Initialize();                                  // Needed after reset
  
  AD5940_WriteReg(REG_AFE_PMBW,       0x821FD );        // High Power Mode
  AD5940_WriteReg(REG_AFECON_CLKSEL,  0x0     );        // Initialize
  AD5940_WriteReg(REG_AFECON_CLKCON0, 0x440   );
  AD5940_WriteReg(REG_AFE_ADCBUFCON,  0x5F3D04);
  AD5940_WriteReg(REG_AFE_AFECON,     0x180040);        // Initialize
}

/** Measurement Process for Load (or Rcal as load) and RTIA */
void AD5940Juul_Measure(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal)
{
  int32_t temp_PGAGain = 0;                                             // Temporary storage for PGA Gain value
  
  AD5940Juul_HSOscillatorCtrl(pJuulCfg->HighSpeedOscClock);             // Set High Speed Oscillator Clock to 32 MHz or 16 MHz
  AD5940Juul_ADCFilterCtrl(pJuulCfg->ADCFilterSampleRate);              // Use 800 KHz Sampling (0x1) or 1.6 MHz (0x10)
  AD5940_WriteReg(REG_AFE_HSTIACON,     0x7C    );                      // High Power TIA
  AD5940_SWMatrixCfgS(&SWMatCfg_Init);                                  // Initialize
  AD5940Juul_HSRtiaCtrl(pJuulCfg->RtiaValue);                           // Use 200 Ohm RTIA, No Capacitance
  AD5940_WriteReg(REG_AFE_LPTIASW0,     0x3180  );                      // Initialize LPTIA
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_DFTRDY, bTRUE);                   // Enables interrupt which would indicate DFT (DFTREAL & DFTIMAG) is ready for reading
  
  /* Configure AD5940 for RCAL measurement */
  AD5940_SWMatrixCfgS(&pJuulCfg->SwitchCfg);                            // Connect across Rcal
  AD5940Juul_WaveGenFreqCtrl(pJuulCfg->WaveGenFreq, pJuulCfg->HighSpeedOscClock);                                     // 3355440 for 100 KHz Excitation Frequency; d’6710890 for 16 MHz Clk //SINEFCW in WGFCW, bits 0-23
  AD5940Juul_WaveGenAmpCtrl(pJuulCfg->WaveGenAmp, pJuulCfg->InAmp, pJuulCfg->Atten);                            // 600 mV Excitation Voltage
  
//  if(pJuulVal->Stage == RTIA_AC_CAL_STAGE)
//  {
//    AD5940_WriteReg(REG_AFE_HSDACCON,     0x80E   );                      // InAmp = 2, Atten = False, 0x1B Update Rate fro HS/HP
//  }
//  else
//  {
//    AD5940_WriteReg(REG_AFE_HSDACCON,     0x180F  );                      // InAmp = 0.25, Atten = True, 0x1B Update Rate for HS/HP
//  }
  AD5940Juul_DACCtrl(pJuulCfg->InAmp, pJuulCfg->Atten); 
  
  AD5940_WriteReg(REG_AFE_WGCON, 0x34);                                 // Sine Wave
  
  temp_PGAGain = 0x201424 | (pJuulCfg->PGAGain_Load << 16);             // Uses set .PGAGain for Rcal during RTIA AC calibration
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across Excitation Amp P-N and set register to the value
  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);                      // ADC Raw Data, 2048 Sampling
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_Delay10us(25);
  AD5940_WriteReg(REG_AFE_AFECON,       0x19CFC0);                      // Start ADC Conversion
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  pJuulVal->DFTLoadReal = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  pJuulVal->DFTLoadImag = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Configure AD5940 for RTIA measurement */
  temp_PGAGain = 0x200101 | (pJuulCfg->PGAGain_Rtia << 16);   // Uses set .PGAGain for RTIA during RTIA AC calibration
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across HPTIA P-N and set register to the value
  AD5940_Delay10us(20);                                                 // Added slight delay, referenced from BIA example
  AD5940_WriteReg(REG_AFE_AFECON, 0x19CFC0);                            // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  pJuulVal->DFTRtiaReal = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  pJuulVal->DFTRtiaImag = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Revert DFT output values which are 2's complemented */
  AD5940Juul_Revert2sComplement(pJuulVal);
}

/** Calculate Magnitude and Phase using DFT Results with PGA Gain */
void AD5940Juul_CalculateDFTResults(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal)
{
  double Load_Real, Load_Imag;
  double Rtia_Real, Rtia_Imag;
  double temp;
  
  double PGAGainList[5] = {1, 1.5, 2, 4, 9};    // List of PGA Gain Values
  
  /* Calculation from DFT Value */
  Load_Real = (double) pJuulVal->DFTLoadReal / PGAGainList[pJuulCfg->PGAGain_Load];
  Load_Imag = (double) pJuulVal->DFTLoadImag / PGAGainList[pJuulCfg->PGAGain_Load];
  Rtia_Real = (double) pJuulVal->DFTRtiaReal / PGAGainList[pJuulCfg->PGAGain_Rtia];
  Rtia_Imag = (double) pJuulVal->DFTRtiaImag / PGAGainList[pJuulCfg->PGAGain_Rtia];
  
  /* Calculation for Magnitude and Phase */
  switch(pJuulVal->Stage)
  {
    case RTIA_AC_CAL_STAGE:       // Calculation for RTIA AC Calibration
      temp  = sqrt((Rtia_Real * Rtia_Real) + (Rtia_Imag * Rtia_Imag));
      temp /= sqrt((Load_Real * Load_Real) + (Load_Imag * Load_Imag));
      pJuulVal->RtiaMag = pJuulVal->RcalValue * temp;     // RTIA Magnitude
      
      temp = (Load_Imag + Rtia_Imag) / (Load_Real + Rtia_Real);
      pJuulVal->RtiaPhase = atan(temp) * 180 / MATH_PI;   // RTIA Phase
    break;
    
    case LOAD_MEAS_STAGE:         // Calculation for Load Measurement
      temp  = sqrt((Load_Real * Load_Real) + (Load_Imag * Load_Imag));
      temp /= sqrt((Rtia_Real * Rtia_Real) + (Rtia_Imag * Rtia_Imag));
      pJuulVal->LoadMag = pJuulVal->RtiaMag * temp;       // Load Magnitude
      
      temp = (Load_Imag + Rtia_Imag) / (Load_Real + Rtia_Real);
      temp = atan(temp) * 180 / MATH_PI;
      pJuulVal->LoadPhase = temp - pJuulVal->RtiaPhase;         // Load Phase
    break;
    
    default:
      // Do nothing
    break;
  }
}

/** For reverting 2s complemented values read from DFT */
void AD5940Juul_Revert2sComplement(JuulValues_Type *pJuulValues)
{
  int32_t temp = pJuulValues->DFTLoadReal & (1L << 17);
  
  if (temp > 0)
  {
    pJuulValues->DFTLoadReal &= (0x1FFFF);
    pJuulValues->DFTLoadReal -= (1L << 17);
  }
  
  temp = pJuulValues->DFTLoadImag & (1L << 17);
  
  if (temp > 0)
  {
    pJuulValues->DFTLoadImag &= (0x1FFFF);
    pJuulValues->DFTLoadImag -= (1L << 17);
  }
  
  temp = pJuulValues->DFTRtiaReal & (1L << 17);
  
  if (temp > 0)
  {
    pJuulValues->DFTRtiaReal &= (0x1FFFF);
    pJuulValues->DFTRtiaReal -= (1L << 17);
  }
  
  temp = pJuulValues->DFTRtiaImag & (1L << 17);
  
  if (temp > 0)
  {
    pJuulValues->DFTRtiaImag &= (0x1FFFF);
    pJuulValues->DFTRtiaImag -= (1L << 17);
  }
}

/** For setting Oscillator Clock Frequency for High Speed Oscillator */
void AD5940Juul_HSOscillatorCtrl(uint8_t HSOscClk)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  register_input = HPOSCCON_LOCK;
  
  if(HSOSCCLK_16MHZ == HSOscClk)
  {
    register_input |= HPOSCCON_CLK32MHZEN | HPOSCCON_CHOPDIS;
  }
  
  AD5940_WriteReg(REG_AFE_HPOSCCON, register_input);
}

/** For setting Sampling Rate for ADC Filter */
void AD5940Juul_ADCFilterCtrl(uint8_t ADCFltrSampleRate)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  
  switch(ADCFltrSampleRate)
  {
    case ADCSAMPLERATE_800KHZ:
      register_input = ADCFILTERCON_ADCSAMPLERATE;
    break;
    
    case ADCSAMPLERATE_1P6MHZ:
      register_input = (1L<<1);
    break;
    
    default:
      // Do nothing
    break;
  }
  
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, register_input);
}

/** For setting RTIA Values */
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  register_input = (HSRTIACON_RTIACON & RTIAValue);
  
  AD5940_WriteReg(REG_AFE_HSRTIACON, register_input);
}

/** For setting Excitation Frequency of Waveform Generator */
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  
  switch(HighSpeedOscClock)
  {
    case HSOSCCLK_32MHZ:
      register_input = (uint64_t) WaveGenFreq * (1L<<30) / HSOSCCLK_VAL_32MHZ;
    break;
    
    case HSOSCCLK_16MHZ:
      register_input = (uint64_t) WaveGenFreq * (1L<<30) / HSOSCCLK_VAL_16MHZ;
    break;
    
    default:
      // Do nothing
    break;
  }
  
  AD5940_WriteReg(REG_AFE_WGFCW, register_input);
}

/** For setting Excitation Amplitude of Waveform Generator */
void AD5940Juul_WaveGenAmpCtrl(uint32_t WaveGenAmp, uint8_t InAmp, uint8_t Atten)
{
  double register_input = REGISTER_ALL_ZERO;
  
  register_input = WaveGenAmp * 2047 / 808.8;
  
  switch(InAmp)
  {
    case INAMP_GAIN_2:
      register_input /= INAMP_GAIN_VAL_2;
    break;
    
    case INAMP_GAIN_0P25:
      register_input /= INAMP_GAIN_VAL_0P25;
    break;
    
    default:
      // Do nothing
    break;
  }
  
  switch(Atten)
  {
    case DAC_ATTEN_DIS:
      register_input /= DAC_ATTEN_DIS_GAINVAL;
    break;
    
    case DAC_ATTEN_EN:
      register_input /= DAC_ATTEN_EN_GAINVAL;
    break;
    
    default:
      // Do nothing
    break;
  }
  
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE, (uint32_t) register_input);
}

void AD5940Juul_DACCtrl(uint8_t InAmp, uint8_t Atten)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  
  register_input |= 7 << HSDACCON_RATE;
  register_input |= bTRUE << HSDACCON_BW250KEN;
  
  if(INAMP_GAIN_0P25 == InAmp)
  {
    register_input |= bTRUE << HSDACCON_INAMPGNMDE;
  }
  
  if(DAC_ATTEN_EN == Atten)
  {
    register_input |= bTRUE << HSDACCON_ATTENEN;
  }
  
  AD5940_WriteReg(REG_AFE_HSDACCON, register_input);
}









