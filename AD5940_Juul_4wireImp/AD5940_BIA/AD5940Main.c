#include "ad5940.h"
#include <stdio.h>
#include "CalibrationMeasurement_FourWire.h"

/* Configurations for Calibration Routine */
AppJuulCfg_Type AppJuulCfg = 
{
  .HighSpeedOscClock            = HSOSCCLK_16MHZ,       // 
  .ADCFilterSampleRate          = ADCSAMPLERATE_800KHZ, // 
  .RcalValue                    = 100,                  // 
  .RTIAValue                    = RTIA_200,             // 
  .WaveGenFreq                  = 100000,               // 
  .WaveGenAmp_Calibrate         = 600,                  // 
  .WaveGenAmp_LoadMeas          = 15,                   // 
  .InAmp_Calibrate              = INAMP_GAIN_2,         // 
  .InAmp_LoadMeas               = INAMP_GAIN_0P25,      // 
  .Atten_Calibrate              = DAC_ATTEN_DIS,        // 
  .Atten_LoadMeas               = DAC_ATTEN_EN,         // 
  .PGAGain_Calibrate_Rcal       = PGA_GAIN_2,           // 
  .PGAGain_Calibrate_Rtia       = PGA_GAIN_1,           // 
  .PGAGain_LoadMeas_Load        = PGA_GAIN_9,           // 
  .PGAGain_LoadMeas_Rtia        = PGA_GAIN_1P5,         // 
};

void AD5940_Main(void)
{  
  /* For DFT Output Values */
  int32_t RtiaAcCalibration_DFTOutput[4] = {0};         // Rcal real[0], Rcal imag[1], RTIA real[2], and RTIA imag[3]
  int32_t LoadMeasurement_DFTOutput[4] = {0};           // Load real[0], Load imag[1], RTIA real[2], and RTIA imag[3]
  
  printf("RTIA_M\tRTIA_P\tLoad_M\tLoad_P\n");           // Header for values to be sent on the terminal
  
  for (int i = 0; i < 10; i++)                          // Added temporarily to repeat the calibration routine 10 times during code testing
  {
    /* Initialize the setup */
    AD5940Juul_Initialization(&AppJuulCfg);
    
    /* Rcal and RTIA Measurement */
    AD5940Juul_RtiaACMeasurement(RtiaAcCalibration_DFTOutput, &AppJuulCfg);
    
    /* Load and RTIA Measurement */
    AD5940Juul_LoadMeasurement(LoadMeasurement_DFTOutput, &AppJuulCfg);
    
    /* Calculation of RTIA & Load Impedance with Showing of Results */
    AD5940Juul_CalculateShowResult(RtiaAcCalibration_DFTOutput, LoadMeasurement_DFTOutput, &AppJuulCfg);
  }
}
 
