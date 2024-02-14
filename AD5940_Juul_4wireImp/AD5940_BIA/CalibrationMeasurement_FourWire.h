#ifndef _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#define _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#include "ad5940.h"

#include <math.h>
#include <stdio.h>

/* PGA Gain */
#define PGA_GAIN_1                      0       // ADC PGA Gain of 1
#define PGA_GAIN_1P5                    1       // ADC PGA Gain of 1.5
#define PGA_GAIN_2                      2       // ADC PGA Gain of 2
#define PGA_GAIN_4                      3       // ADC PGA Gain of 4
#define PGA_GAIN_9                      4       // ADC PGA Gain of 9

/* High Speed Oscillator Clock */
#define HSOSCCLK_32MHZ                  0x10    // 32 MHz
#define HSOSCCLK_16MHZ                  0x34    // 16 MHz

/* Sampling Rate for ADC Filter */
#define ADCSAMPLERATE_800KHZ            0x1     // 800 kHz
#define ADCSAMPLERATE_1P6MHZ            0x10    // 1.6 MHz

/* Available RTIA Values (Ohms) */
#define RTIA_VAL_200                    0x0     // 200 Ohms
#define RTIA_VAL_1K                     0x1     // 1 kOhms
#define RTIA_VAL_5K                     0x2     // 5 kOhms
#define RTIA_VAL_10K                    0x3     // 10 kOhms
#define RTIA_VAL_20K                    0x4     // 20 kOhms
#define RTIA_VAL_40K                    0x5     // 40 kOhms
#define RTIA_VAL_80K                    0x6     // 80 kOhms
#define RTIA_VAL_160K                   0x7     // 160 kOhms
#define RTIA_VAL_OPEN                   0x8     // Open

/* Available Configurable Values */
typedef struct
{
  uint32_t HighSpeedOscClock;           // Frequency value for the High Speed Oscillator Clock
  uint32_t ADCFilterSampleRate;         // Sampling rate for the ADC Filter
  double RcalValue;                     // Set value for Rcal
  uint32_t RTIAValue;                   // Set value for RTIA
  uint32_t ExcitationFrequency;         // For 100 KHz Excitation Frequency: d'3355440 at 32 MHz, d'6710890 at 16 MHz
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
void AD5940Juul_Revert2sComplement(int32_t InputValue[4]);
double AD5940Juul_GetComplexMag(double FinalMultiplier, double RealNumerator,   double ImagNumerator, double RealDenominator, double ImagDenominator);
double AD5940Juul_GetComplexPhaseCalibrate(double Real1, double Real2, double Imag1, double Imag2);
double AD5940Juul_GetComplexPhaseMeasure(double Real1, double Real2, double Imag1, double Imag2, double Theta);

#endif