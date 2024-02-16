#ifndef _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#define _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#include "ad5940.h"

#include <math.h>
#include <stdio.h>

/* Register bit set all to zero */
#define REGISTER_ALL_ZERO               0x00000000

/* PGA Gain Index */
#define PGA_GAIN_1                      0       // ADC PGA Gain of 1
#define PGA_GAIN_1P5                    1       // ADC PGA Gain of 1.5
#define PGA_GAIN_2                      2       // ADC PGA Gain of 2
#define PGA_GAIN_4                      3       // ADC PGA Gain of 4
#define PGA_GAIN_9                      4       // ADC PGA Gain of 9

/* High Speed Oscillator Clock */
#define HSOSCCLK_32MHZ                  0       // 32 MHz
#define HSOSCCLK_16MHZ                  1       // 16 MHz

/* High Speed Oscillator Clock Frequency Value */
#define HSOSCCLK_VAL_32MHZ                32000000        // 32 MHz
#define HSOSCCLK_VAL_16MHZ                16000000        // 16 MHz

/* Sampling Rate for ADC Filter */
#define ADCSAMPLERATE_800KHZ            0       // 800 kHz
#define ADCSAMPLERATE_1P6MHZ            1       // 1.6 MHz

/* HSOSCCON/HPOSCCON Register Bit Mask */
#define HPOSCCON_OSCSTARTSEL            (1L<<0)
#define HPOSCCON_CLKOUTCON              (1L<<1)
#define HPOSCCON_CLK32MHZEN             (1L<<2)
#define HPOSCCON_FASTLOCKEN             (1L<<3)
#define HPOSCCON_LOCK                   (1L<<4)
#define HPOSCCON_CHOPDIS                (1L<<5)
#define HPOSCCON_CHOPCLKSEL             (1L<<6)

/* ADCFILTERCON Register Bit Mask */
#define ADCFILTERCON_ADCSAMPLERATE      (1L<<0)
#define ADCFILTERCON_LPFBYPEN           (1L<<4)
#define ADCFILTERCON_SINC3BYP           (1L<<6)
#define ADCFILTERCON_AVRGEN             (1L<<7)
#define ADCFILTERCON_SINC2OSR           (1111L<<8)
#define ADCFILTERCON_SINC3OSR           (11L<<12)
#define ADCFILTERCON_AVRGNUM            (11L<<14)
#define ADCFILTERCON_SINC2CLKENB        (1L<<16)
#define ADCFILTERCON_DACWAVECLKENB      (1L<<17)
#define ADCFILTERCON_DFTCLKENB          (1L<<18)

/* ADCFILTERCON SINC2OSR Values */
#define ADCFILTERCON_SINC2OSR_22        0
#define ADCFILTERCON_SINC2OSR_44        1
#define ADCFILTERCON_SINC2OSR_89        2
#define ADCFILTERCON_SINC2OSR_178       3
#define ADCFILTERCON_SINC2OSR_267       4
#define ADCFILTERCON_SINC2OSR_533       5
#define ADCFILTERCON_SINC2OSR_640       6
#define ADCFILTERCON_SINC2OSR_667       7
#define ADCFILTERCON_SINC2OSR_800       8
#define ADCFILTERCON_SINC2OSR_889       9
#define ADCFILTERCON_SINC2OSR_1067      10
#define ADCFILTERCON_SINC2OSR_1333      11

/* HSRTIACON/HPRTIACON Register Bit Mask */
#define HSRTIACON_RTIACON               (1111L<<0)
#define HSRTIACON_TIASW6CON             (1L<<4)
#define HSRTIACON_CTAICON               (11111111L<<5)

/* HSRTIACON RTIACON Values */
#define HSRTIACON_RTIACON_200           RTIA_VAL_200
#define HSRTIACON_RTIACON_1k            RTIA_VAL_1K
#define HSRTIACON_RTIACON_5k            RTIA_VAL_5K
#define HSRTIACON_RTIACON_10k           RTIA_VAL_10K
#define HSRTIACON_RTIACON_20k           RTIA_VAL_20K
#define HSRTIACON_RTIACON_40k           RTIA_VAL_40K
#define HSRTIACON_RTIACON_80k           RTIA_VAL_80K
#define HSRTIACON_RTIACON_160k          RTIA_VAL_160K
#define HSRTIACON_RTIACON_OPEN          RTIA_VAL_OPEN

/* RTIA Values (Ohms) */
#define RTIA_200                        0       // 200 Ohms
#define RTIA_1K                         1       // 1 kOhms
#define RTIA_5K                         2       // 5 kOhms
#define RTIA_10K                        3       // 10 kOhms
#define RTIA_20K                        4       // 20 kOhms
#define RTIA_40K                        5       // 40 kOhms
#define RTIA_80K                        6       // 80 kOhms
#define RTIA_160K                       7       // 160 kOhms
#define RTIA_OPEN                       8       // Open

/* InAmp Values */
#define INAMP_GAIN_2                    0       // Gain = 2
#define INAMP_GAIN_0P25                 1       // Gain = 0.25

/* InAmp Values */
#define INAMP_GAIN_VAL_2                2       // Gain = 2
#define INAMP_GAIN_VAL_0P25             0.25       // Gain = 0.25

/* InAmp Values */
#define DAC_ATTEN_DIS                   0       // Disabled -> Gain = 1
#define DAC_ATTEN_EN                    1       // Enabled -> Gain = 0.2

/* InAmp Values */
#define DAC_ATTEN_DIS_GAINVAL           1       // Disabled -> Gain = 1
#define DAC_ATTEN_EN_GAINVAL            0.2       // Enabled -> Gain = 0.2

/* Available Configurable Values */
typedef struct
{
  uint8_t HighSpeedOscClock;            // Frequency value for the High Speed Oscillator Clock
  uint8_t ADCFilterSampleRate;          // Sampling rate for the ADC Filter
  uint32_t RcalValue;                   // Set value for Rcal
  uint8_t RTIAValue;                    // Set value for RTIA
  uint32_t WaveGenFreq;                 // Set Excitation Frequency of Waveform Generator
  uint32_t WaveGenAmp_Calibrate;        // Set Excitation Amplitude of Waveform Generator during RTIA AC Calibration
  uint32_t WaveGenAmp_LoadMeas;         // Set Excitation Amplitude of Waveform Generator during Load Measurement
  uint8_t InAmp_Calibrate;              // Set InAmp Gain value during RTIA AC Calibration
  uint8_t InAmp_LoadMeas;               // Set InAmp Gain value during Load Measurement
  uint8_t Atten_Calibrate;              // Set DAC Attenuator value during RTIA AC Calibration
  uint8_t Atten_LoadMeas;               // Set DAC Attenuator value during Load Measurement
  uint32_t PGAGain_Calibrate_Rcal;      // Set PGA Gain for Rcal measurement during RTIA AC Calibration
  uint32_t PGAGain_Calibrate_Rtia;      // Set PGA Gain for RTIA measurement during RTIA AC Calibration
  uint32_t PGAGain_LoadMeas_Load;       // Set PGA Gain for Load measurement during Load Measurement
  uint32_t PGAGain_LoadMeas_Rtia;       // Set PGA Gain for RTIA measurement during Load Measurement
}AppJuulCfg_Type;

/* Function Declarations */
void AD5940Juul_Initialization(AppJuulCfg_Type *pJuulCfg);
void AD5940Juul_RtiaACMeasurement(int32_t RtiaAcCalibration_RawValues[4], AppJuulCfg_Type *pJuulCfg);
void AD5940Juul_LoadMeasurement(int32_t LoadMeasurement_RawValues[4], AppJuulCfg_Type *pJuulCfg);
void AD5940Juul_CalculateShowResult(int32_t RtiaAcCalibration_RawValues[4], int32_t LoadMeasurement_RawValues[4], AppJuulCfg_Type *pJuulCfg);

void AD5940Juul_HSOscillatorCtrl(uint8_t HSOscClk);
void AD5940Juul_ADCFilterCtrl(uint8_t ADCFltrSampleRate);
void AD5940Juul_HSRtiaCtrl(uint8_t RTIAValue);
void AD5940Juul_WaveGenFreqCtrl(uint32_t WaveGenFreq, uint8_t HighSpeedOscClock);
void AD5940Juul_WaveGenAmpCtrl(uint32_t WaveGenAmp, uint8_t InAmp, uint8_t Atten);

void AD5940Juul_Revert2sComplement(int32_t InputValue[4]);
double AD5940Juul_GetComplexMag(double FinalMultiplier, double RealNumerator,   double ImagNumerator, double RealDenominator, double ImagDenominator);
double AD5940Juul_GetComplexPhaseCalibrate(double Real1, double Real2, double Imag1, double Imag2);
double AD5940Juul_GetComplexPhaseMeasure(double Real1, double Real2, double Imag1, double Imag2, double Theta);

#endif