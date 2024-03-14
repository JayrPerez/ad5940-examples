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
  AD5940Juul_HSOscillatorCtrl(pJuulCfg->HighSpeedOscClock);                             // Set oscillator clock to 32 MHz or 16 MHz
  AD5940Juul_ADCFilterCtrl(pJuulCfg->ADCFilterSampleRate);                              // Sets the sampling rate to 800 kHz or 1.6 MHz
  AD5940_AFECtrlS(AFECTRL_HSTIAPWR, bTRUE);                                             // Enables high power TIA
  AD5940_WriteReg(REG_AFE_HSTIACON, 0x7C);                                              // High Power TIA
  AD5940_SWMatrixCfgS(&SWMatCfg_Init);                                                  // Initialize
  AD5940Juul_HSRtiaCtrl(pJuulCfg->RtiaValue);                                           // Use 200 Ohm RTIA, No Capacitance
  AD5940_WriteReg(REG_AFE_LPTIASW0, 0x3180);                                            // Initialize LPTIA
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_DFTRDY, bTRUE);                                   // Enables interrupt which would indicate DFT (DFTREAL & DFTIMAG) is ready for reading
  
  /* Configure AD5940 for RCAL or Load measurement */
  AD5940_SWMatrixCfgS(&pJuulCfg->SwitchCfg);                                            // Sets switches to connect to Rcal or Load
  AD5940Juul_WaveGenFreqCtrl(pJuulCfg->WaveGenFreq, pJuulCfg->HighSpeedOscClock);       // Sets excitation buffer frequency - input based on High Speed Oscillator Clock speed
  AD5940Juul_WaveGenAmpCtrl(pJuulCfg->WaveGenAmp, pJuulCfg->InAmp, pJuulCfg->AttenEn);  // Sets excitation buffer amplitude - input based on InAmp and Attenuation gains 
  AD5940Juul_DACCtrl(pJuulCfg->InAmp, pJuulCfg->AttenEn);                               // Sets InAmp and attenuation gain                          
  AD5940_WriteReg(REG_AFE_WGCON, 0x34);                                                 // Sine Wave
  AD5940Juul_ADCCtrl(pJuulCfg->PGAGain_Load, RCAL_LOAD_MEASURE_STAGE);                  // Sets PGA gain for Rcal/Load measurement
  AD5940_WriteReg(REG_AFE_DFTCON, 0x200091);                                            // Sets DFT input to ADC Raw Data and with 2048 Sampling
  AD5940_AFECtrlS(AFECTRL_WG | AFECTRL_INAMPPWR | AFECTRL_EXTBUFPWR | 
                  AFECTRL_ADCCNV | AFECTRL_ADCPWR, bTRUE);                              // ADC Idle
  AD5940_Delay10us(25);
  AD5940_AFECtrlS(AFECTRL_SINC2NOTCH | AFECTRL_DFT, bTRUE);                             // Start ADC Conversion
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);                    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_AFECtrlS(AFECTRL_SINC2NOTCH | AFECTRL_DFT, bFALSE);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                                 // Clears the recent DFT interrupt flag
  
  pJuulVal->DFTLoadReal = AD5940_ReadAfeResult(AFERESULT_DFTREAL);                      // Reads register DFTREAL to get raw DFT real value
  pJuulVal->DFTLoadImag = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);                     // Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Configure AD5940 for RTIA measurement */
  AD5940Juul_ADCCtrl(pJuulCfg->PGAGain_Rtia, RTIA_MEASURE_STAGE);                       // Sets PGA gain for RTIA measurement
  AD5940_Delay10us(25);                                                                 // Added slight delay, referenced from BIA example
  AD5940_AFECtrlS(AFECTRL_SINC2NOTCH | AFECTRL_DFT, bTRUE);                             // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);                    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_AFECtrlS(AFECTRL_SINC2NOTCH | AFECTRL_DFT, bFALSE);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                                 // Clears the recent DFT interrupt flag
  
  pJuulVal->DFTRtiaReal = AD5940_ReadAfeResult(AFERESULT_DFTREAL);                      // Reads register DFTREAL to get raw DFT real value
  pJuulVal->DFTRtiaImag = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);                     // Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Revert DFT output values which are 2's complemented */
  AD5940Juul_Revert2sComplement(pJuulVal);
  AD5940_AFECtrlS(AFECTRL_WG | AFECTRL_HSTIAPWR | AFECTRL_INAMPPWR | AFECTRL_EXTBUFPWR | AFECTRL_ADCCNV | AFECTRL_ADCPWR, bFALSE);
}

/** Calculate Magnitude and Phase using DFT Results with PGA Gain */
void AD5940Juul_CalculateDFTResults(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal)
{
  double Load_Real, Load_Imag;
  double Rtia_Real, Rtia_Imag;
  double temp;
  
  /* List of PGA Gain Values */
  double PGAGainList[5] = {PGA_GAIN_VAL_1, PGA_GAIN_VAL_1P5, PGA_GAIN_VAL_2, \
                           PGA_GAIN_VAL_4, PGA_GAIN_VAL_9};
  
  /* Calculation from DFT Value */
  Load_Real = (double) pJuulVal->DFTLoadReal / PGAGainList[pJuulCfg->PGAGain_Load];
  Load_Imag = (double) pJuulVal->DFTLoadImag / PGAGainList[pJuulCfg->PGAGain_Load];
  Rtia_Real = (double) pJuulVal->DFTRtiaReal / PGAGainList[pJuulCfg->PGAGain_Rtia];
  Rtia_Imag = (double) pJuulVal->DFTRtiaImag / PGAGainList[pJuulCfg->PGAGain_Rtia];
  
  /* Calculation for Magnitude and Phase */
  if(RTIA_AC_CAL_STAGE == pJuulVal->Stage)
  {
    /* Calculation for RTIA AC Calibration */
    temp  = sqrt((Rtia_Real * Rtia_Real) + (Rtia_Imag * Rtia_Imag));
    temp /= sqrt((Load_Real * Load_Real) + (Load_Imag * Load_Imag));
    pJuulVal->RtiaMag = pJuulVal->RcalValue * temp;   // RTIA Magnitude
    
    temp = (Load_Imag + Rtia_Imag) / (Load_Real + Rtia_Real);
    temp = atan(temp) * CONST_DEGREE_180 / MATH_PI;
    pJuulVal->RtiaPhase = temp;                       // RTIA Phase
  }
  else
  {
    /* Calculation for Load Measurement */
    temp  = sqrt((Load_Real * Load_Real) + (Load_Imag * Load_Imag));
    temp /= sqrt((Rtia_Real * Rtia_Real) + (Rtia_Imag * Rtia_Imag));
    pJuulVal->LoadMag = pJuulVal->RtiaMag * temp;     // Load Magnitude
    
    temp = (Load_Imag + Rtia_Imag) / (Load_Real + Rtia_Real);
    temp = atan(temp) * CONST_DEGREE_180 / MATH_PI;
    pJuulVal->LoadPhase = temp - pJuulVal->RtiaPhase; // Load Phase
  }
}

/** For reverting 2s complemented values read from DFT */
void AD5940Juul_Revert2sComplement(JuulValues_Type *pJuulValues)
{
  int32_t DFTRawBit18;  // Indicator for 18th bit (bit 17) for basis of 2's complement
  
  DFTRawBit18 = pJuulValues->DFTLoadReal & BIT_17_MASK;
  
  if (DFTRawBit18 > 0)
  {
    pJuulValues->DFTLoadReal &= BIT_0_TO_16_MASK;
    pJuulValues->DFTLoadReal -= BIT_17_MASK;
  }
  
  DFTRawBit18 = pJuulValues->DFTLoadImag & BIT_17_MASK;
  
  if (DFTRawBit18 > 0)
  {
    pJuulValues->DFTLoadImag &= BIT_0_TO_16_MASK;
    pJuulValues->DFTLoadImag -= BIT_17_MASK;
  }
  
  DFTRawBit18 = pJuulValues->DFTRtiaReal & BIT_17_MASK;
  
  if (DFTRawBit18 > 0)
  {
    pJuulValues->DFTRtiaReal &= BIT_0_TO_16_MASK;
    pJuulValues->DFTRtiaReal -= BIT_17_MASK;
  }
  
  DFTRawBit18 = pJuulValues->DFTRtiaImag & BIT_17_MASK;
  
  if (DFTRawBit18 > 0)
  {
    pJuulValues->DFTRtiaImag &= BIT_0_TO_16_MASK;
    pJuulValues->DFTRtiaImag -= BIT_17_MASK;
  }
}

/** For setting Oscillator Clock Frequency for High Speed Oscillator */
void AD5940Juul_HSOscillatorCtrl(uint8_t HSOscClk)
{
  uint32_t register_input = REG_AFE_HPOSCCON_RESET;
  register_input |= BITM_AFE_HPOSCCON_LOCK;
  
  if(HSOSCCLK_32MHZ == HSOscClk)
  {
    register_input &= ~(BITM_AFE_HPOSCCON_CHOPDIS | BITM_AFE_HPOSCCON_CLK32MHZEN);
  }
  
  AD5940_WriteReg(REG_AFE_HPOSCCON, register_input);
}

/** For setting Sampling Rate for ADC Filter */
void AD5940Juul_ADCFilterCtrl(uint8_t ADCFltrSampleRate)
{
  uint32_t register_input = REG_AFE_ADCFILTERCON_RESET;
  register_input &= ~BITM_AFE_ADCFILTERCON_SINC2OSR;
  
  if(ADCSAMPLERATE_1P6MHZ == ADCFltrSampleRate)
  {
    register_input &= ~BITM_AFE_ADCFILTERCON_ADCCLK;
    register_input |= BITM_AFE_ADCFILTERCON_BIT1;
  }
  
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, register_input);
}

/** For setting RTIA Values */
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue)
{
  uint32_t register_input = REG_AFE_HSRTIACON_RESET;
  
  /* Sets chosen RTIA resistance */
  register_input &= ~BITM_AFE_HSRTIACON_RTIACON;
  register_input |= (BITM_AFE_HSRTIACON_RTIACON & RTIAValue);
  
  AD5940_WriteReg(REG_AFE_HSRTIACON, register_input);
}

/** For setting Excitation Frequency of Waveform Generator */
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock)
{
  uint64_t register_input = REG_AFE_WGFCW_RESET;
  
  /* Multiplies excitation frequency by 2^30 */
  register_input = (uint64_t) WaveGenFreq * CONST_WAVE_GEN_FREQ;
  
  /* Divides by oscillator clock speed */
  if(HSOSCCLK_32MHZ == HighSpeedOscClock)
  {
    register_input /= HSOSCCLK_VAL_32MHZ;
  }
  else
  {
    register_input /= HSOSCCLK_VAL_16MHZ;
  }
  
  AD5940_WriteReg(REG_AFE_WGFCW, (uint32_t) register_input);
}

/** For setting Excitation Amplitude of Waveform Generator */
void AD5940Juul_WaveGenAmpCtrl(uint32_t WaveGenAmp, uint8_t InAmp, uint8_t AttenEn)
{
  double register_input = REG_AFE_WGAMPLITUDE_RESET;
  
  /* Multiplies amplitude by 2047 / 808.8 */
  register_input = WaveGenAmp * CONST_WAVE_GEN_AMP;
  
  /* Divides by InAmp gain */
  if(INAMP_GAIN_2 == InAmp)
  {
      register_input /= INAMP_GAIN_VAL_2;
  }
  else
  {
      register_input /= INAMP_GAIN_VAL_0P25;
  }
  
  /* Divides by attenuation gain */
  if(AttenEn)
  {
    register_input /= DAC_ATTEN_EN_GAINVAL;
  }
  else
  {
    register_input /= DAC_ATTEN_DIS_GAINVAL;
  }
  
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE, (uint32_t) register_input);
}

/** For setting InAmp and Attenuation for DAC */
void AD5940Juul_DACCtrl(uint8_t InAmp, uint8_t AttenEn)
{
  uint32_t register_input = REG_AFE_HSDACCON_RESET;
  
  /* Sets the DAC update rate and enables bandwidth of 250 kHz */
  register_input &= ~BITM_AFE_HSDACCON_RATE;
  register_input |= CONST_HSDACCON_ADCRATE << BITP_AFE_HSDACCON_RATE;
  register_input |= BITM_AFE_HSDACCON_BW250KEN;
  
  if(INAMP_GAIN_0P25 == InAmp)
  {
    /* Sets InAmp gain from 2 (default) to 0.25 */
    register_input |= BITM_AFE_HSDACCON_INAMPGNMDE;
  }
  
  if(AttenEn)
  {
    /* Sets attenuation to 0.2 gain */
    register_input |= BITM_AFE_HSDACCON_ATTENEN;
  }
  
  AD5940_WriteReg(REG_AFE_HSDACCON, register_input);
}

/** For setting PGA Gain for ADC */
void AD5940Juul_ADCCtrl(uint8_t PGAGain, uint8_t Stage)
{
  uint32_t register_input = REG_AFE_ADCCON_RESET;
  
  /* Sets Anti-alias Filter to 250 kHz */
  register_input |= AAFFMDE_250KHZ << BITP_AFE_ADCCON_AAFFMDE;
  
  if(RCAL_LOAD_MEASURE_STAGE == Stage)
  {
    /* Sets excitation nodes as input */
    register_input |= ADCMUXP_P_NODE << BITP_AFE_ADCCON_MUXSELP;
    register_input |= ADCMUXN_N_NODE << BITP_AFE_ADCCON_MUXSELN;
  }
  else
  {
    /* Sets HPTIA nodes as input */
    register_input |= ADCMUXP_HSTIA_P << BITP_AFE_ADCCON_MUXSELP;
    register_input |= ADCMUXN_HSTIA_N << BITP_AFE_ADCCON_MUXSELN;
  }
  
  /* Sets PGA Gain */
  register_input |= PGAGain << BITP_AFE_ADCCON_GNPGA;
  
  AD5940_WriteReg(REG_AFE_ADCCON, register_input);
}







