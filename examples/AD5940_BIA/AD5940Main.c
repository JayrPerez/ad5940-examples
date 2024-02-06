#include "ad5940.h"
#include <stdio.h>
#include "math.h"
#include "CalibrationMeasurement_FourWire.h"

void AD5940_Main(void)
{  
  /* For Raw DFT Values */                      // Raw DFT Values are in 2's complement form when read at registers DFTREAL and DFTIMAG
  int32_t RtiaAcCalibration_RawValues[4] = {0}; // Rcal real[0], Rcal imag[1], RTIA real[2], and RTIA imag[3]
  int32_t LoadMeasurement_RawValues[4] = {0};   // Load real[0], Load imag[1], RTIA real[2], and RTIA imag[3]
  
  /* For Calculated Values */
  int32_t Rcal_Real,      Rcal_Imag;            // Rcal real and imaginary value
  int32_t Rtia_Cal_Real,  Rtia_Cal_Imag;        // RTIA real and imaginary value for the RTIA AC Calibration stage
  int32_t Load_Real,      Load_Imag;            // Load real and imaginary value
  int32_t Rtia_Meas_Real, Rtia_Meas_Imag;       // RTIA real and imaginary value for the Load Measurement stage
  
  /* DFT Real/Imaginary Output */
  int32_t DFT_Output;                           // temporary storage for DFT Output after converting from 2's complement form to decimal 
  
  /* Initialize the setup */
  AD5940Juul_Initialization();                   // From "PMBW -> High Power Mode" to "ULPTIASW0 -> Initialize LPTIA"
  
  /** Steps Calculation for RTIA **
      1. Get raw DFT values for Rcal/Rtia/Load (still in 2's complement form)
      2. Convert from 2's complement form to decimal - done by getting its negative counterpart and temporarily stored at DFT_Output
      3. Divide the DFT output by the PGA Gain to get values to be shown as measurement results
  **/
  
  /* Rcal and RTIA Measurement */
  AD5940Juul_RtiaACMeasurement(RtiaAcCalibration_RawValues);
  
  /* Calculation of Rtia Impedance */
  DFT_Output     = -RtiaAcCalibration_RawValues[0];     //set to negative counterpart to convert from 2's complement to decimal
  Rcal_Real      = DFT_Output / PGA_GAIN_2;             //Divide DFT output by PGA Gain
  DFT_Output     = -RtiaAcCalibration_RawValues[1];
  Rcal_Imag      = DFT_Output / PGA_GAIN_2;
  DFT_Output     = -RtiaAcCalibration_RawValues[2];
  Rtia_Cal_Real  = DFT_Output / PGA_GAIN_1;
  DFT_Output     = -RtiaAcCalibration_RawValues[3];
  Rtia_Cal_Imag  = DFT_Output / PGA_GAIN_1;
  
  /* Load and RTIA Measurement */
  AD5940Juul_LoadMeasurement(LoadMeasurement_RawValues);
  
  /* Calculation of Rtia Impedance */
  DFT_Output     = -LoadMeasurement_RawValues[0];       //set to negative counterpart to convert from 2's complement to decimal
  Load_Real      = DFT_Output / PGA_GAIN_9;             //Divide DFT output by PGA Gain
  DFT_Output     = -LoadMeasurement_RawValues[1];
  Load_Imag      = DFT_Output / PGA_GAIN_9;
  DFT_Output     = -LoadMeasurement_RawValues[2];
  Rtia_Meas_Real = DFT_Output / PGA_GAIN_1P5;
  DFT_Output     = -LoadMeasurement_RawValues[3];
  Rtia_Meas_Imag = DFT_Output / PGA_GAIN_1P5;
  
#if (MAG_PHASE == 0)                                    // Show real and imaginary values only
  
  /* Show Result Values */                              // Currently uses "int32_t" as data type, thus would only show integer values
  printf("RTIA AC Calibration\r\n");
  printf("Rcal real&imag: \t%d\t%d\r\n", Rcal_Real, Rcal_Imag);
  printf("Rtia real&imag: \t%d\t%d\r\n", Rtia_Cal_Real, Rtia_Cal_Imag);
  printf("Load Measurement\r\n");
  printf("Load real&imag: \t%d\t%d\r\n", Load_Real, Load_Imag);
  printf("Rtia real&imag: \t%d\t%d\r\n", Rtia_Meas_Real, Rtia_Meas_Imag);
  
#else                                                   // Show real & imaginary value, and magnitude & phase
  
  /* Calculation for the Magnitude and Phase */
  double Rtia_Magnitude = AD5940Juul_GetComplexMag(RCAL_VALUE, Rtia_Cal_Real, Rtia_Cal_Imag, Rcal_Real, Rcal_Imag);
  double Rtia_Phase = AD5940Juul_GetComplexPhase(Rcal_Imag, Rtia_Cal_Imag, Rcal_Real, Rtia_Cal_Real);
  double Load_Magnitude = AD5940Juul_GetComplexMag(Rtia_Magnitude, Load_Real, Load_Imag, Rtia_Meas_Real, Rtia_Meas_Imag);
  double Load_Phase = AD5940Juul_GetComplexPhase(Load_Imag, Rtia_Meas_Imag, Load_Real, Rtia_Meas_Real);
  
  /* Show Result Values */                              // Uses "double" as data type for magnitude & phase - value could be outside data type range
  printf("RTIA AC Calibration\r\n");
  printf("Rcal real&imag: \t%d\t%d\r\n", Rcal_Real, Rcal_Imag);
  printf("Rtia real&imag: \t%d\t%d\r\n", Rtia_Cal_Real, Rtia_Cal_Imag);
  printf("Rtia mag&phase: \t%f\t%f\r\n", Rtia_Magnitude, Rtia_Phase);
  printf("Load Measurement\r\n");
  printf("Load real&imag: \t%d\t%d\r\n", Load_Real, Load_Imag);
  printf("Rtia real&imag: \t%d\t%d\r\n", Rtia_Meas_Real, Rtia_Meas_Imag);
  printf("Load mag&phase: \t%f\t%f\r\n", Load_Magnitude, Load_Phase);
  
#endif
  
}
 
