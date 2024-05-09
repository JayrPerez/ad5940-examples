#include "ad5940.h"
#include <stdio.h>
#include "CalibrationMeasurement_FourWire.h"

/** Set Configurations for RTIA AC Calibration Stage */
JuulMeasCfg_Type JuulCfg_RtiaACCal =
{
  .HighSpeedOscClock    = HPOSCOUT_16MHZ,       // Oscillator clock = 16 MHz
  .ADCFilterCfg         = 
  {
    .ADCSinc3Osr        = ADCSINC3OSR_5,        // Sinc3 oversampling rate = 5
    .ADCSinc2Osr        = ADCSINC2OSR_22,       // Sinc2 oversampling rate = 22
    .ADCAvgNum          = ADCAVGNUM_2,          // Number of samples for averaging = 2
    .ADCRate            = ADCRATE_800KHZ,       // ADC sampling rate to 800 kHz
    .BpNotch            = bFALSE,               // bFALSE to enable 50 Hz/60 Hz notch filter
    .BpSinc3            = bFALSE,               // bFALSE to enable Sinc3 filter
    .Sinc2NotchClkEnable= bFALSE,               // bFALSE to enable Sinc2 filter clock
    .Sinc2NotchEnable   = bTRUE,                // bTRUE to enable supply rejection filter for DFT
    .DFTClkEnable       = bFALSE,               // bFALSE to enable DFT clock
    .WGClkEnable        = bFALSE,               // bFALSE to enable DAC wave clock
  },
  .RtiaValue            = HSTIARTIA_200,        // RTIA = 200 Ohms
  .WaveGenFreq          = 100000,               // Wave generator freq = 100 KHz
  .WaveGenAmp           = 600,                  // Wave generator amplitude = 600 mV
  .InAmp                = INAMP_GAIN_2,         // InAmp gain = 2
  .AttenEn              = bFALSE,               // Disable attenuation, gain = 1 
  .PGAGain_Load         = ADCPGA_2,             // PGA Gain = 2
  .PGAGain_Rtia         = ADCPGA_1,             // PGA Gain = 1
  .DFTCfg               =
  {
    .DftNum             = DFTNUM_16384,         // DFT samples = 16384
    .DftSrc             = DFTSRC_ADCRAW,        // Raw ADC output for DFT
  },
  .SWMatrixCfg          = 
  {
    .Dswitch            = SWD_RCAL0,            // Excitation buffer electrode to Rcal0
    .Pswitch            = SWP_AIN3,             // HSTIA P-node to AIN3
    .Nswitch            = SWN_AIN0,             // HSTIA N-node to AIN0
    .Tswitch            = SWT_RCAL1 | SWT_TRTIA,// RTIA electrode to Rcal1, connects T9
  },
};

/** Set Configurations for Load Measurement Stage */
JuulMeasCfg_Type JuulCfg_LoadMeas =
{
  .HighSpeedOscClock    = HPOSCOUT_16MHZ,       // Oscillator clock = 16 MHz
  .ADCFilterCfg         = 
  {
    .ADCSinc3Osr        = ADCSINC3OSR_5,        // Sinc3 oversampling rate = 5
    .ADCSinc2Osr        = ADCSINC2OSR_22,       // Sinc2 oversampling rate = 22
    .ADCAvgNum          = ADCAVGNUM_2,          // Number of samples for averaging = 2
    .ADCRate            = ADCRATE_800KHZ,       // ADC sampling rate to 800 kHz
    .BpNotch            = bFALSE,               // bFALSE to enable 50 Hz/60 Hz notch filter
    .BpSinc3            = bFALSE,               // bFALSE to enable Sinc3 filter
    .Sinc2NotchClkEnable= bFALSE,               // bFALSE to enable Sinc2 filter clock
    .Sinc2NotchEnable   = bTRUE,                // bTRUE to enable supply rejection filter for DFT
    .DFTClkEnable       = bFALSE,               // bFALSE to enable DFT clock
    .WGClkEnable        = bFALSE,               // bFALSE to enable DAC wave clock
  },
  .RtiaValue            = HSTIARTIA_200,        // RTIA = 200 Ohms
  .WaveGenFreq          = 100000,               // Wave generator freq = 100 KHz
  .WaveGenAmp           = 1,                    // Wave generator amplitude = 1 mV
  .InAmp                = INAMP_GAIN_0P25,      // InAmp gain = 0.25
  .AttenEn              = bTRUE,                // Enable attenuation, gain = 0.2
  .PGAGain_Load         = ADCPGA_9,             // PGA Gain = 9
  .PGAGain_Rtia         = ADCPGA_1P5,           // PGA Gain = 1.5
  .DFTCfg               =
  {
    .DftNum             = DFTNUM_16384,         // DFT samples = 16384
    .DftSrc             = DFTSRC_ADCRAW,        // Raw ADC output for DFT
  },
  .SWMatrixCfg          = 
  {
    .Dswitch            = SWD_CE0,              // Excitation buffer electrode to CE0
    .Pswitch            = SWP_RE0,              // HSTIA P-node to RE0
    .Nswitch            = SWN_SE0,              // HSTIA N-node to SE0
    .Tswitch            = SWT_AIN1 | SWT_TRTIA, // RTIA electrode to AIN1, connects T9
  },
};

/** Set @Rcal Value to use for Calculation */
JuulValues_Type JuulValues =
{          
  .RcalValue            = 100,                 // Rcal value = 100 Ohms
};

void AD5940_Main(void)
{ 
  for (uint8_t run = 0; run < 100; run++)
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
    printf("RTIA_M\tRTIA_P\tLoad_M\tLoad_P\n");   // UART terminal outputs' header
    printf("%.2f\t%.2f\t%.2f\t%.2f\n", 
           JuulValues.RtiaMag, JuulValues.RtiaPhase, \
           JuulValues.LoadMag, JuulValues.LoadPhase);
  }
  
  AD5940Juul_Deinit();
}
 
