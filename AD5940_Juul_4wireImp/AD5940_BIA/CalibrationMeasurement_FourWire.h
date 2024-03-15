#ifndef _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#define _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#include "ad5940.h"

#include <math.h>
#include <stdio.h>

/* Calculation Constants */
#define CONST_DEGREE_180                180
#define CONST_WAVE_GEN_FREQ             (1L<<30)
#define CONST_WAVE_GEN_AMP              (2047/808.8)      
#define CONST_HSDACCON_ADCRATE          7
#define BIT_0_TO_16_MASK                0x0001FFFF
#define BIT_17_MASK                     0x00020000

/* Stages of Calibration Routine */
#define RTIA_AC_CAL_STAGE               0       // at measuring RTIA mag&phase
#define LOAD_MEAS_STAGE                 1       // at measuring load mag&phase

/* Stages of Component to Measure */
#define RCAL_LOAD_MEASURE_STAGE         0       // at measuring Rcal or Load component
#define RTIA_MEASURE_STAGE              1       // at measuring RTIA component

/* PGA Gain Index */
#define PGA_GAIN_VAL_1                  1       // ADC PGA Gain of 1
#define PGA_GAIN_VAL_1P5                1.5     // ADC PGA Gain of 1.5
#define PGA_GAIN_VAL_2                  2       // ADC PGA Gain of 2
#define PGA_GAIN_VAL_4                  4       // ADC PGA Gain of 4
#define PGA_GAIN_VAL_9                  9       // ADC PGA Gain of 9

/* High Speed Oscillator Clock Frequency Value */
#define HSOSCCLK_VAL_32MHZ              32000000
#define HSOSCCLK_VAL_16MHZ              16000000

/* InAmp Values */
#define INAMP_GAIN_2                    2       // Gain = 2
#define INAMP_GAIN_0P25                 0.25    // Gain = 0.25

/* InAmp Values */
#define DAC_ATTEN_DIS_GAIN              1       // Disabled -> Gain = 1
#define DAC_ATTEN_EN_GAIN               0.2     // Enabled -> Gain = 0.2

/*  */
#define BITM_AFE_ADCFILTERCON_BIT1      (1L<<1)

/* Register set all to zero */
#define REGISTER_RESET_TO_ZERO          0x00000000

/* Struct for Configurations for Measurment */
typedef struct
{
  uint8_t HighSpeedOscClock;            // Frequency value for the High Speed Oscillator Clock
  ADCFilterCfg_Type ADCFilterCfg;       // Set ADC filter configurations
  uint8_t RtiaValue;                    // Set value for RTIA
  uint32_t WaveGenFreq;                 // Set excitation frequency of waveform generator
  float WaveGenAmp;                     // Set amplitude of waveform generator
  float InAmp;                          // Set instrumentation amplifier gain
  float AttenEn;                        // Set attenuation
  uint8_t PGAGain_Load;                 // Set PGA gain for Rcal or Load
  uint8_t PGAGain_Rtia;                 // Set PGA gain for RTIA
  DFTCfg_Type DFTCfg;                   // Set DFT configurations
  SWMatrixCfg_Type SWMatrixCfg;         // Set switch matrix configuration
}JuulMeasCfg_Type;

/* Struct for Values for Calculations */
typedef struct
{
  uint8_t Stage;                        // Indicates if at calibration or load measurement stage
  float RcalValue;                      /** Set value for @Rcal */
  int32_t DFTLoadReal;                  // Raw DFT measured Rcal or Load real component
  int32_t DFTLoadImag;                  // Raw DFT measured Rcal or Load imaginary component
  int32_t DFTRtiaReal;                  // Raw DFT measured RTIA real component
  int32_t DFTRtiaImag;                  // Raw DFT measured RTIA imaginary component
  double RtiaMag;                       // Calculated RTIA magnitude
  double RtiaPhase;                     // Calculated RTIA phase
  double LoadMag;                       // Calculated Load magnitude
  double LoadPhase;                     // Calculated Load phase
}JuulValues_Type;

/* Function Declarations */
void AD5940Juul_Initialization(void);
void AD5940Juul_Deinit(void);
void AD5940Juul_Measure(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal);
void AD5940Juul_CalculateDFTResults(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal);
void AD5940Juul_Revert2sComplement(JuulValues_Type *pJuulValues);
void AD5940Juul_HSOscillatorCtrl(uint8_t HSOscClk);
void AD5940Juul_ADCFilterCtrl(ADCFilterCfg_Type *pADCFilterCfg);
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue);
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock);
void AD5940Juul_WaveGenAmpCtrl(float WaveGenAmp, float InAmp, float AttenEn);
void AD5940Juul_DACCtrl(float InAmp, float AttenEn);
void AD5940Juul_ADCCtrl(uint8_t PGAGain, uint8_t Stage);
void AD5940Juul_DFTCtrl(DFTCfg_Type *pDFTCfg);

#endif