#include "ad5940.h"
#include <stdio.h>
#include "CalibrationMeasurement_FourWire.h"

/** Set Configurations for RTIA AC Calibration Stage */
JuulMeasCfg_Type JuulCfg_RtiaACCal =
{
  .HighSpeedOscClock    = HSOSCCLK_16MHZ,               // Oscillator clock = 16 MHz
  .ADCFilterSampleRate  = ADCSAMPLERATE_800KHZ,         // ADC Sampling rate = 800 KHz
  .RtiaValue            = RTIA_200,                     // RTIA = 200 Ohms
  .WaveGenFreq          = 100000,                       // Wave generator freq = 100 KHz
  .WaveGenAmp           = 600,                          // Wave generator amplitude = 600 mV
  .InAmp                = INAMP_GAIN_2,                 // InAmp gain = 2
  .AttenEn              = bFALSE,                       // Disable attenuation, gain = 1 
  .PGAGain_Load         = PGA_GAIN_2,                   // PGA Gain = 2
  .PGAGain_Rtia         = PGA_GAIN_1,                   // PGA Gain = 1
  .SwitchCfg            = 
  {
    .Dswitch            = SWD_RCAL0,                    // Excitation buffer electrode to Rcal0
    .Pswitch            = SWP_AIN3,                     // HSTIA P-node to AIN3
    .Nswitch            = SWN_AIN0,                     // HSTIA N-node to AIN0
    .Tswitch            = SWT_RCAL1 | SWT_TRTIA,        // RTIA electrode to Rcal1, connects T9
  },
};

/** Set Configurations for Load Measurement Stage */
JuulMeasCfg_Type JuulCfg_LoadMeas =
{
  .HighSpeedOscClock    = HSOSCCLK_16MHZ,               // Oscillator clock = 16 MHz
  .ADCFilterSampleRate  = ADCSAMPLERATE_800KHZ,         // ADC Sampling rate = 800 KHz
  .RtiaValue            = RTIA_200,                     // RTIA = 200 Ohms
  .WaveGenFreq          = 100000,                       // Wave generator freq = 100 KHz
  .WaveGenAmp           = 15,                           // Wave generator amplitude = 15 mV
  .InAmp                = INAMP_GAIN_0P25,              // InAmp gain = 0.25
  .AttenEn              = bTRUE,                        // Enable attenuation, gain = 0.2
  .PGAGain_Load         = PGA_GAIN_9,                   // PGA Gain = 9
  .PGAGain_Rtia         = PGA_GAIN_1P5,                 // PGA Gain = 1.5
  .SwitchCfg            = 
  {
    .Dswitch            = SWD_CE0,                      // Excitation buffer electrode to CE0
    .Pswitch            = SWP_RE0,                      // HSTIA P-node to RE0
    .Nswitch            = SWN_SE0,                      // HSTIA N-node to SE0
    .Tswitch            = SWT_AIN1 | SWT_TRTIA,         // RTIA electrode to AIN1, connects T9
  },
};

/** Set @Rcal Value to use for Calculation */
JuulValues_Type JuulValues =
{          
  .RcalValue            = 100,                           // Rcal value = 100 Ohms
};

void AD5940_Main(void)
{  
  printf("RTIA_M\tRTIA_P\tLoad_M\tLoad_P\n");           // Header for UART terminal outputs
  
  for (int runCount = 0; runCount < 10; runCount++)     // Temporary for getting 10 measurments
  {
    /* Initialize the setup */
    AD5940Juul_Initialization();
    
    /* Set as indicator to use configuration and formula for RTIA AC Calibration */
    JuulValues.Stage = RTIA_AC_CAL_STAGE;
    
    /* Measuring Across Rcal and TIA */
    AD5940Juul_Measure(&JuulCfg_RtiaACCal, &JuulValues);
    
    /* Calculation of RTIA Impedance */
    AD5940Juul_CalculateDFTResults(&JuulCfg_RtiaACCal, &JuulValues);
    
    /* Set as indicator to use configuration and formula for Load Measurement */
    JuulValues.Stage = LOAD_MEAS_STAGE;
    
    /* Measuring Across Load and TIA */
    AD5940Juul_Measure(&JuulCfg_LoadMeas, &JuulValues);
    
    /* Calculation of Load Impedance */
    AD5940Juul_CalculateDFTResults(&JuulCfg_LoadMeas, &JuulValues);
    
    /* Show Results to UART Terminal */
    printf("%.2f\t%.2f\t%.2f\t%.2f\n", JuulValues.RtiaMag, JuulValues.RtiaPhase, \
                                       JuulValues.LoadMag, JuulValues.LoadPhase);
  }
}
 
