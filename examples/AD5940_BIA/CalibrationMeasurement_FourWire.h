#ifndef _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#define _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#include "ad5940.h"

#include <math.h>
#include <stdio.h>

#define MAG_PHASE                       1     /* set to 1 to include calculate Magnitude and Phase */

#define RCAL_VALUE                      96.36

#define PGA_GAIN_1                      1     /* ADC PGA Gain of 1   */
#define PGA_GAIN_1P5                    1.5   /* ADC PGA Gain of 1.5 */
#define PGA_GAIN_2                      2     /* ADC PGA Gain of 2   */
#define PGA_GAIN_4                      4     /* ADC PGA Gain of 4   */
#define PGA_GAIN_9                      9     /* ADC PGA Gain of 9   */

void AD5940Juul_Initialization(void);

void AD5940Juul_RtiaACMeasurement(int32_t RtiaAcCalibration_RawValues[4]);
void AD5940Juul_LoadMeasurement(int32_t LoadMeasurement_RawValues[4]);

double AD5940Juul_GetComplexMag(double FinalMultiplier, \
                                double RealNumerator,   double ImagNumerator, \
                                double RealDenominator, double ImagDenominator);

double AD5940Juul_GetComplexPhase(double Real1, double Real2, \
                                  double Imag1, double Imag2);

double AD5940Juul_GetComplexPhase2(double Real1, double Real2, \
                                  double Imag1, double Imag2, double Theta);

#endif