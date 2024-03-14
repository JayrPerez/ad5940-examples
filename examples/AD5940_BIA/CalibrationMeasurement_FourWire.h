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
#define BIT_0_TO_16_MASK                (0x1FFFF)
#define BIT_17_MASK                     (1L << 17)

/* Stages of Calibration Routine */
#define RTIA_AC_CAL_STAGE               0
#define LOAD_MEAS_STAGE                 1

/* Stages of Component to Measure */
#define RCAL_LOAD_MEASURE_STAGE         0
#define RTIA_MEASURE_STAGE              1

/* PGA Gain Index */
#define PGA_GAIN_1                      ADCPGA_1        // ADC PGA Gain of 1
#define PGA_GAIN_1P5                    ADCPGA_1P5      // ADC PGA Gain of 1.5
#define PGA_GAIN_2                      ADCPGA_2        // ADC PGA Gain of 2
#define PGA_GAIN_4                      ADCPGA_4        // ADC PGA Gain of 4
#define PGA_GAIN_9                      ADCPGA_9        // ADC PGA Gain of 9

/* PGA Gain Index */
#define PGA_GAIN_VAL_1                  1               // ADC PGA Gain of 1
#define PGA_GAIN_VAL_1P5                1.5             // ADC PGA Gain of 1.5
#define PGA_GAIN_VAL_2                  2               // ADC PGA Gain of 2
#define PGA_GAIN_VAL_4                  4               // ADC PGA Gain of 4
#define PGA_GAIN_VAL_9                  9               // ADC PGA Gain of 9

/* High Speed Oscillator Clock */
#define HSOSCCLK_32MHZ                  HPOSCOUT_32MHZ  // 32 MHz
#define HSOSCCLK_16MHZ                  HPOSCOUT_16MHZ  // 16 MHz

/* High Speed Oscillator Clock Frequency Value */
#define HSOSCCLK_VAL_32MHZ              32000000        // 32 MHz
#define HSOSCCLK_VAL_16MHZ              16000000        // 16 MHz

/* Sampling Rate for ADC Filter */
#define ADCSAMPLERATE_800KHZ            ADCRATE_800KHZ  // 800 kHz
#define ADCSAMPLERATE_1P6MHZ            ADCRATE_1P6MHZ  // 1.6 MHz

/* RTIA Values (Ohms) */
#define RTIA_200                        HSTIARTIA_200   // 200 Ohms
#define RTIA_1K                         HSTIARTIA_1K    // 1 kOhms
#define RTIA_5K                         HSTIARTIA_5K    // 5 kOhms
#define RTIA_10K                        HSTIARTIA_10K   // 10 kOhms
#define RTIA_20K                        HSTIARTIA_20K   // 20 kOhms
#define RTIA_40K                        HSTIARTIA_40K   // 40 kOhms
#define RTIA_80K                        HSTIARTIA_80K   // 80 kOhms
#define RTIA_160K                       HSTIARTIA_160K  // 160 kOhms
#define RTIA_OPEN                       HSTIARTIA_OPEN  // Open

/* InAmp Values */
#define INAMP_GAIN_2                    0       // Gain = 2
#define INAMP_GAIN_0P25                 1       // Gain = 0.25

/* InAmp Values */
#define INAMP_GAIN_VAL_2                2       // Gain = 2
#define INAMP_GAIN_VAL_0P25             0.25    // Gain = 0.25

/* InAmp Values */
#define DAC_ATTEN_DIS                   0       // Disabled -> Gain = 1
#define DAC_ATTEN_EN                    1       // Enabled -> Gain = 0.2

/* InAmp Values */
#define DAC_ATTEN_DIS_GAINVAL           1       // Disabled -> Gain = 1
#define DAC_ATTEN_EN_GAINVAL            0.2     // Enabled -> Gain = 0.2

/*  */
#define BITM_AFE_ADCFILTERCON_BIT1      (1L<<1)

/* Struct for Configurations for Measurment */
typedef struct
{
  uint8_t HighSpeedOscClock;            // Frequency value for the High Speed Oscillator Clock
  uint8_t ADCFilterSampleRate;          // Sampling rate for the ADC Filter
  uint8_t RtiaValue;                    // Set value for RTIA
  uint32_t WaveGenFreq;                 // Set Excitation Frequency of Waveform Generator
  uint32_t WaveGenAmp;
  uint8_t InAmp;
  uint8_t AttenEn;
  uint32_t PGAGain_Load;
  uint32_t PGAGain_Rtia;
  SWMatrixCfg_Type SwitchCfg;
}JuulMeasCfg_Type;

/* Struct for Values for Calculations */
typedef struct
{
  uint8_t Stage;
  double RcalValue;
  int32_t DFTLoadReal;
  int32_t DFTLoadImag;
  int32_t DFTRtiaReal;
  int32_t DFTRtiaImag;
  double RtiaMag;
  double RtiaPhase;
  double LoadMag;
  double LoadPhase;
}JuulValues_Type;

/* Function Declarations */
void AD5940Juul_Initialization(void);
void AD5940Juul_Measure(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal);
void AD5940Juul_CalculateDFTResults(JuulMeasCfg_Type *pJuulCfg, JuulValues_Type *pJuulVal);
void AD5940Juul_Revert2sComplement(JuulValues_Type *pJuulValues);
void AD5940Juul_HSOscillatorCtrl(uint8_t HSOscClk);
void AD5940Juul_ADCFilterCtrl(uint8_t ADCFltrSampleRate);
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue);
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock);
void AD5940Juul_WaveGenAmpCtrl(uint32_t WaveGenAmp, uint8_t InAmp, uint8_t AttenEn);
void AD5940Juul_DACCtrl(uint8_t InAmp, uint8_t AttenEn);
void AD5940Juul_ADCCtrl(uint8_t PGAGain, uint8_t Stage);

#endif