#include "CalibrationMeasurement_FourWire.h"

void AD5940Juul_Initialization(AppJuulCfg_Type *pJuulCfg)
{
  AD5940_HWReset();                                                     // Performs hardware reset
  AD5940_Initialize();                                                  // According to data sheet, this is needed whenever a reset is performed
  
  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );                      // High Power Mode
  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );                      // Initialize
  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );
  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);
  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);                      // Initialize
  AD5940Juul_HSOscillatorCtrl(pJuulCfg->HighSpeedOscClock);             // Set High Speed Oscillator Clock to 32 MHz or 16 MHz
  AD5940Juul_ADCFilterCtrl(pJuulCfg->ADCFilterSampleRate);              // Use 800 KHz Sampling (0x1) or 1.6 MHz (0x10)
  AD5940_WriteReg(REG_AFE_HSTIACON,     0x7C    );                      // High Power TIA
  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );                      // Initialize
  AD5940Juul_HSRtiaCtrl(pJuulCfg->RTIAValue);                           // Use 200 Ohm RTIA, No Capacitance
  AD5940_WriteReg(REG_AFE_LPTIASW0,     0x3180  );                      // Initialize LPTIA
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_DFTRDY, bTRUE);                   // Enables interrupt which would indicate DFT (DFTREAL & DFTIMAG) is ready for reading
  
  /* Removed optional lines */
  /* ADCBISCON register not present in data sheet/register library */
  /* Reset values for ADCCON, BUFSENCON and ADCBISCON are 0x0, 0x37 and 0x0 respectively */
  /* Removed the three register writes above due to input value already same with reset value */
}

void AD5940Juul_RtiaACMeasurement(int32_t RtiaAcCalibrationValues[4], AppJuulCfg_Type *pJuulCfg)
{
  int32_t temp_PGAGain = 0;                                             // Temporary storage for PGA Gain value

  /* Configure AD5940 for RCAL measurement */
  AD5940_WriteReg(REG_AFE_SWCON, 0x28A11 );                             // Connect across Rcal
  AD5940Juul_WaveGenFreqCtrl(pJuulCfg->WaveGenFreq, pJuulCfg->HighSpeedOscClock);                                     // 3355440 for 100 KHz Excitation Frequency; d’6710890 for 16 MHz Clk //SINEFCW in WGFCW, bits 0-23
  AD5940Juul_WaveGenAmpCtrl(pJuulCfg->WaveGenAmp_Calibrate, pJuulCfg->InAmp_Calibrate, pJuulCfg->Atten_Calibrate);                            // 600 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON, 0x80E);                             // InAmp = 2, Atten = False, 0x1B Update Rate fro HS/HP
  AD5940_WriteReg(REG_AFE_WGCON, 0x34);                                 // Sine Wave
  
  temp_PGAGain = 0x201424 | (pJuulCfg->PGAGain_Calibrate_Rcal << 16);   // Uses set .PGAGain for Rcal during RTIA AC calibration
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across Excitation Amp P-N and set register to the value
  AD5940_WriteReg(REG_AFE_DFTCON, 0x200091);                            // ADC Raw Data, 2048 Sampling
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_Delay10us(20);                                                 // Added slight delay, referenced from BIA example
  AD5940_WriteReg(REG_AFE_AFECON, 0x19CFC0);                            // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  RtiaAcCalibrationValues[0] = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  RtiaAcCalibrationValues[1] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Configure AD5940 for RTIA measurement */
  temp_PGAGain = 0x200101 | (pJuulCfg->PGAGain_Calibrate_Rtia << 16);   // Uses set .PGAGain for RTIA during RTIA AC calibration
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across HPTIA P-N and set register to the value
  AD5940_Delay10us(20);                                                 // Added slight delay, referenced from BIA example
  AD5940_WriteReg(REG_AFE_AFECON, 0x19CFC0);                            // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  RtiaAcCalibrationValues[2] = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  RtiaAcCalibrationValues[3] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Revert DFT output values which are 2's complemented */
  AD5940Juul_Revert2sComplement(RtiaAcCalibrationValues);
}

void AD5940Juul_LoadMeasurement(int32_t LoadMeasurementValues[4], AppJuulCfg_Type *pJuulCfg)
{
  int32_t temp_PGAGain = 0;                                             // Temporary storage for PGA Gain value
  
  /* Configure AD5940 for Load measurement */
  AD5940_WriteReg(REG_AFE_SWCON, 0x22955);                              // Connect across ZLoad
  AD5940Juul_WaveGenAmpCtrl(pJuulCfg->WaveGenAmp_Calibrate, pJuulCfg->InAmp_Calibrate, pJuulCfg->Atten_Calibrate);                            // 15 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON, 0x180F);                            // InAmp = 0.25, Atten = True, 0x1B Update Rate for HS/HP
  
  temp_PGAGain = 0x201424 | (pJuulCfg->PGAGain_LoadMeas_Load << 16);    // Uses set .PGAGain for Load during load measurement
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across Excitation Amp P-N and set register to the value
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_Delay10us(20);                                                 // Added slight delay, referenced from BIA example
  AD5940_WriteReg(REG_AFE_AFECON, 0x19CFC0);                            // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  LoadMeasurementValues[0] = AD5940_ReadAfeResult(AFERESULT_DFTREAL);   // Reads register DFTREAL to get raw DFT real value
  LoadMeasurementValues[1] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);  // Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Configure AD5940 for RTIA measurement */
  temp_PGAGain = 0x200101 | (pJuulCfg->PGAGain_LoadMeas_Rtia << 16);    // Uses set .PGAGain for RTIA during load measurement
  AD5940_WriteReg(REG_AFE_ADCCON, temp_PGAGain);                        // Adds PGA Gain to fixed config of ADC Across HPTIA P-N and set register to the value
  AD5940_Delay10us(20);                                                 // Added slight delay, referenced from BIA example
  AD5940_WriteReg(REG_AFE_AFECON, 0x19CFC0);                            // Start ADC Conversion (has a disabled dc DAC buffer)
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON, 0x184FC0);                            // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  
  LoadMeasurementValues[2] = AD5940_ReadAfeResult(AFERESULT_DFTREAL);   // Reads register DFTREAL to get raw DFT real value
  LoadMeasurementValues[3] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);  // Reads register DFTIMAG to get raw DFT imaginary value
  
  /* Revert DFT output values which are 2's complemented */
  AD5940Juul_Revert2sComplement(LoadMeasurementValues);
}

void AD5940Juul_CalculateShowResult(int32_t RtiaAcCalibration_RawValues[4], int32_t LoadMeasurement_RawValues[4], AppJuulCfg_Type *pJuulCfg)
{
  /* For Calculated Values */
  double Rcal_Real,      Rcal_Imag;             // Rcal real and imaginary value
  double Rtia_Cal_Real,  Rtia_Cal_Imag;         // RTIA real and imaginary value for the RTIA AC Calibration stage
  double Load_Real,      Load_Imag;             // Load real and imaginary value
  double Rtia_Meas_Real, Rtia_Meas_Imag;        // RTIA real and imaginary value for the Load Measurement stage
  
  double Rtia_Magnitude;                        // RTIA magnitude during RTIA AC Calibration
  double Rtia_Phase;                            // RTIA phase during RTIA AC Calibration
  double Load_Magnitude;                        // Load magnitude during Load Measurement
  double Load_Phase;                            // Load phase during Load Measurement
  
  double PGA_Gain_List[5] = {1, 1.5, 2, 4, 9};  // List of available PGA Gains
                                                // "pJuulCfg.PGAGain_x_x" are index values for register, "PGA_Gain_List" are the actual values
  
  /* Calculation of Rtia Impedance */           //Divide DFT output by PGA Gain set at ".PGAGain_x_x"
  Rcal_Real      = (double) RtiaAcCalibration_RawValues[0] / PGA_Gain_List[pJuulCfg->PGAGain_Calibrate_Rcal];
  Rcal_Imag      = (double) RtiaAcCalibration_RawValues[1] / PGA_Gain_List[pJuulCfg->PGAGain_Calibrate_Rcal];
  Rtia_Cal_Real  = (double) RtiaAcCalibration_RawValues[2] / PGA_Gain_List[pJuulCfg->PGAGain_Calibrate_Rtia];
  Rtia_Cal_Imag  = (double) RtiaAcCalibration_RawValues[3] / PGA_Gain_List[pJuulCfg->PGAGain_Calibrate_Rtia];
  
  /* Calculation of Rtia Impedance */           //Divide DFT output by PGA Gain set at ".PGAGain_x_x"
  Load_Real      = (double) LoadMeasurement_RawValues[0] / PGA_Gain_List[pJuulCfg->PGAGain_LoadMeas_Load];
  Load_Imag      = (double) LoadMeasurement_RawValues[1] / PGA_Gain_List[pJuulCfg->PGAGain_LoadMeas_Load];
  Rtia_Meas_Real = (double) LoadMeasurement_RawValues[2] / PGA_Gain_List[pJuulCfg->PGAGain_LoadMeas_Rtia];
  Rtia_Meas_Imag = (double) LoadMeasurement_RawValues[3] / PGA_Gain_List[pJuulCfg->PGAGain_LoadMeas_Rtia];
  
  /* Calculation for the Magnitude and Phase */
  Rtia_Magnitude = AD5940Juul_GetComplexMag(pJuulCfg->RcalValue, Rtia_Cal_Real, Rtia_Cal_Imag, Rcal_Real, Rcal_Imag);
  Rtia_Phase     = AD5940Juul_GetComplexPhaseCalibrate(Rcal_Imag, Rtia_Cal_Imag, Rcal_Real, Rtia_Cal_Real);
  Load_Magnitude = AD5940Juul_GetComplexMag(Rtia_Magnitude, Load_Real, Load_Imag, Rtia_Meas_Real, Rtia_Meas_Imag);
  Load_Phase     = AD5940Juul_GetComplexPhaseMeasure(Load_Imag, Rtia_Meas_Imag, Load_Real, Rtia_Meas_Real, Rtia_Phase);
  
  /* Show Results through UART terminal */
  printf("%.2f\t%.2f\t%.2f\t%.2f\n", Rtia_Magnitude, Rtia_Phase, Load_Magnitude, Load_Phase);
}

/* For setting Oscillator Clock Frequency for High Speed Oscillator */
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

/* For setting Sampling Rate for ADC Filter */
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
  }
  
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, register_input);
}

/* For setting RTIA Values */
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  register_input = (HSRTIACON_RTIACON & RTIAValue);
  
  AD5940_WriteReg(REG_AFE_HSRTIACON, register_input);
}

/* For setting Excitation Frequency of Waveform Generator */
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock)
{
  uint32_t register_input = REGISTER_ALL_ZERO;
  
  switch(HighSpeedOscClock)
  {
  case HSOSCCLK_32MHZ:
    register_input = WaveGenFreq * (1L<<30) / HSOSCCLK_VAL_32MHZ;
    break;
    
  case HSOSCCLK_16MHZ:
    register_input = WaveGenFreq * (1L<<30) / HSOSCCLK_VAL_16MHZ;
    break;
  }
  
  AD5940_WriteReg(REG_AFE_WGFCW, register_input);
}

/* For setting Excitation Amplitude of Waveform Generator */
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
  }
  
  switch(Atten)
  {
  case DAC_ATTEN_DIS:
    register_input /= DAC_ATTEN_DIS_GAINVAL;
    break;
    
  case DAC_ATTEN_EN:
    register_input /= DAC_ATTEN_EN_GAINVAL;
    break;
  }
  
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE, (uint32_t) register_input);
}



void AD5940Juul_Revert2sComplement(int32_t InputValue[4])
{
  for (int8_t i = 0; i < 4; i++)
  {
    int32_t Bit_18 = InputValue[i] & (1L << 17);
    
    if (Bit_18 > 0)
    {
      InputValue[i] = (0x1FFFF) & InputValue[i];
      InputValue[i] = InputValue[i] - (1L << 17);
    }
  }
}

/** Guide for Value Designation (Use Formula as Basis):
    - Final Multiplier     = for the value multiplied after the squareroot and is outside the squareroot
    - Real/ImagNumerator   = for values inside the squareroot above the fraction bar
    - Real/ImagDenominator = for values inside the squareroot below the fraction bar
*/
double AD5940Juul_GetComplexMag(double FinalMultiplier, double RealNumerator,   double ImagNumerator, double RealDenominator, double ImagDenominator)       
{
  double numerator;
  double denominator;
  double result;
  
  numerator   = sqrt((RealNumerator   * RealNumerator  ) + (ImagNumerator   * ImagNumerator  ));
  denominator = sqrt((RealDenominator * RealDenominator) + (ImagDenominator * ImagDenominator));
  result = FinalMultiplier * numerator / denominator;
  
  return result;
}

/** Guide for Value Designation (Use Formula as Basis):
    - Real1/2 = for the real values found below the fraction bar, can be interchanged
    - Imag1/2 = for the imaginary values found above the fraction bar, can be interchanged
*/
double AD5940Juul_GetComplexPhaseCalibrate(double Real1, double Real2, double Imag1, double Imag2)
{
  double temp;
  
  temp = (Real1 + Real2) / (Imag1 + Imag2);
  temp = atan(temp) * 180 / MATH_PI;
  
  return temp;
}

double AD5940Juul_GetComplexPhaseMeasure(double Real1, double Real2, double Imag1, double Imag2, double Theta)
{
  double temp;
  
  temp = (Real1 + Real2) / (Imag1 + Imag2);
  temp = atan(temp) * 180 / MATH_PI;
  temp = temp - Theta;
  
  return temp;
}

















