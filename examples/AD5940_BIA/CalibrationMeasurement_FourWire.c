#include "CalibrationMeasurement_FourWire.h"

void AD5940Juul_Initialization(void)
{
  AD5940_HWReset();                                                   // Performs hardware reset
  AD5940_Initialize();                                                // According to data sheet, this is needed whenever a reset is performed
  
  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );                      // High Power Mode
  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );                      // Initialize
  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );
  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);
  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);                      // Initialize
  
  AD5940_WriteReg(REG_AFE_HPOSCCON,     0x34    );                      // 32 MHz Clk (0x10) or 16 Mhz Clk (0x34)
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, 0x1     );                      // Use 800 KHz Sampling (0x1) or 1.6 MHz (0x10)
  AD5940_WriteReg(REG_AFE_HSTIACON,     0x7C    );                      // High Power TIA
  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );                      // Initialize
  AD5940_WriteReg(REG_AFE_HSRTIACON,    0x0     );                      // Use 200 Ohm RTIA, No Capacitance
  AD5940_WriteReg(REG_AFE_LPTIASW0,     0x3180  );                      // Initialize LPTIA
  
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_DFTRDY, bTRUE);                   /* Enable SINC2 Interrupt in INTC1 */
                                                                        // Enables interrupt which would indicate DFT (DFTREAL & DFTIMAG) is ready for reading
  
  /* Removed optional lines */
  /* ADCBISCON register not present in data sheet/register library */
  /* Reset values for ADCCON, BUFSENCON and ADCBISCON are 0x0, 0x37 and 0x0 respectively */
  /* Removed the three register writes above due to input value already same with reset value */
}

void AD5940Juul_RtiaACMeasurement(int32_t RtiaAcCalibrationValues[4])
{
  AD5940_WriteReg(REG_AFE_SWCON,        0x28A11 );                      // Connect across Rcal; 0x28E11 if 2-wire (Local Sense)
  AD5940_WriteReg(REG_AFE_WGFCW,        6710890 );                      // 3355440 for 100 KHz Excitation Frequency; d’6710890 for 16 MHz Clk //SINEFCW in WGFCW, bits 0-23
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );                      // 600 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x80E   );                      // InAmp = 2, Atten = False, 0x1B Update Rate fro HS/HP
  AD5940_WriteReg(REG_AFE_WGCON,        0x34    );                      // Sine Wave
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x221424);                      // PGA Gain = 2, ADC Across Excitation Amp P-N
  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);                      // ADC Raw Data, 2048 Sampling
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_Delay10us(25);
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);                      // Start ADC Conversion
  
  /* Measuring Across RCAL */
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  RtiaAcCalibrationValues[0] = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  RtiaAcCalibrationValues[1] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x200101);                      // PGA Gain = 1, ADC Across HPTIA P-N
  AD5940_Delay10us(25);
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);                      // Start ADC Conversion
  
  /* Measuring Across TIA */
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  RtiaAcCalibrationValues[2] = AD5940_ReadAfeResult(AFERESULT_DFTREAL); // Reads register DFTREAL to get raw DFT real value
  RtiaAcCalibrationValues[3] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);// Reads register DFTIMAG to get raw DFT imaginary value
  
  for (int8_t i = 0; i < 4; i++)
  {
    int32_t Bit_18 = RtiaAcCalibrationValues[i] & (1L << 17);
    
    if (Bit_18 > 0)
    {
      RtiaAcCalibrationValues[i] = (0x1FFFF) & RtiaAcCalibrationValues[i];
      RtiaAcCalibrationValues[i] = RtiaAcCalibrationValues[i] - (1L << 17);
    }
  }
}

void AD5940Juul_LoadMeasurement(int32_t LoadMeasurementValues[4])
{
  AD5940_WriteReg(REG_AFE_SWCON,        0x22005 );                      // Connect across ZLoad
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );                      // 15 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x180F  );                      // InAmp = 0.25, Atten = True, 0x1B Update Rate for HS/HP
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x241424);                      // PGA Gain = 9, ADC Across Excitation Amp P-N
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_Delay10us(25);
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);                      // Start ADC Conversion
  
  /* Measuring Across ZLoad here */
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  LoadMeasurementValues[0] = AD5940_ReadAfeResult(AFERESULT_DFTREAL);   // Reads register DFTREAL to get raw DFT real value
  LoadMeasurementValues[1] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);  // Reads register DFTIMAG to get raw DFT imaginary value
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x210101);                      // PGA Gain = 1.5, ADC Across HPTIA P-N
  AD5940_Delay10us(25);
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);                      // Start ADC Conversion
  
  /* Measuring Across TIA here */
  while(AD5940_INTCTestFlag(AFEINTC_1, AFEINTSRC_DFTRDY) == bFALSE);    // Wait for DFT interrupt which would indicate DFT is ready to be read
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);                      // ADC Idle
  AD5940_INTCClrFlag(AFEINTSRC_DFTRDY);                                 // Clears the recent DFT interrupt flag
  LoadMeasurementValues[2] = AD5940_ReadAfeResult(AFERESULT_DFTREAL);   // Reads register DFTREAL to get raw DFT real value
  LoadMeasurementValues[3] = AD5940_ReadAfeResult(AFERESULT_DFTIMAGE);  // Reads register DFTIMAG to get raw DFT imaginary value
  
  for (int8_t i = 0; i < 4; i++)
  {
    int32_t Bit_18 = LoadMeasurementValues[i] & (1L << 17);
    
    if (Bit_18 > 0)
    {
      LoadMeasurementValues[i] = (0x1FFFF) & LoadMeasurementValues[i];
      LoadMeasurementValues[i] = LoadMeasurementValues[i] - (1L << 17);
    }
  }
}

/** Guide for Value Designation (Use Formula as Basis):
    - Final Multiplier     = for the value multiplied after the squareroot and is outside the squareroot
    - Real/ImagNumerator   = for values inside the squareroot above the fraction bar
    - Real/ImagDenominator = for values inside the squareroot below the fraction bar
*/
double AD5940Juul_GetComplexMag(double FinalMultiplier, \
                                double RealNumerator,   double ImagNumerator, \
                                double RealDenominator, double ImagDenominator)       
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
double AD5940Juul_GetComplexPhase(double Real1, double Real2, \
                                  double Imag1, double Imag2)
{
  double temp;
  
  temp = (Real1 + Real2) / (Imag1 + Imag2);
  temp = atan(temp) * 180 / MATH_PI;
  
  return temp;
}

double AD5940Juul_GetComplexPhase2(double Real1, double Real2, \
                                  double Imag1, double Imag2, double Theta)
{
  double temp;
  
  temp = (Real1 + Real2) / (Imag1 + Imag2);
  temp = atan(temp) * 180 / MATH_PI;
  temp = temp - Theta;
  
  return temp;
}

















