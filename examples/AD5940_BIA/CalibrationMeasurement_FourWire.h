#ifndef _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#define _CALIBRATIONMEASUREMENT_FOURWIRE_H_
#include "ad5940.h"

#include <math.h>

#define MAG_PHASE                       1     /* set to 1 to include calculate Magnitude and Phase */

#define MATH_PI                         3.1416

#define RCAL_VALUE                      100

#define PGA_GAIN_1                      1     /* ADC PGA Gain of 1   */
#define PGA_GAIN_1P5                    1.5   /* ADC PGA Gain of 1.5 */
#define PGA_GAIN_2                      2     /* ADC PGA Gain of 2   */
#define PGA_GAIN_4                      4     /* ADC PGA Gain of 4   */
#define PGA_GAIN_9                      9     /* ADC PGA Gain of 9   */

void AD5940Juul_Initialization(void);

void AD5940Juul_RtiaACMeasurement(int32_t RtiaAcCalibration_RawValues[4]);
void AD5940Juul_LoadMeasurement(int32_t LoadMeasurement_RawValues[4]);

double AD5940Juul_GetComplexMag(int32_t FinalMultiplier, \
                                int32_t RealNumerator,   int32_t ImagNumerator, \
                                int32_t RealDenominator, int32_t ImagDenominator);

double AD5940Juul_GetComplexPhase(int32_t Real1, int32_t Real2, \
                                  int32_t Imag1, int32_t Imag2);

#endif