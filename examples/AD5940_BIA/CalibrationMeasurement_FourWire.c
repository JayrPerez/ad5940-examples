#include "CalibrationMeasurement_FourWire.h"

void AD5940JuulCalibration()
{
  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );      //High Power Mode
  
  /* Initialize */
  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );      
  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );      
  AD5940_WriteReg(REG_AFE_ADCCON,       0x0     );      
  AD5940_WriteReg(REG_AFE_BUFSENCON,    0x37    );      
  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);      
  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);      
  AD5940_WriteReg(REG_AFE_ADCBISCCON,   0x0     );      //Not present @datasheet
  
  AD5940_WriteReg(REG_AFE_HPOSCCON,     0x10    );      //32 MHz Clk (0x10) or 16 Mhz Clk (0x34)
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, 0x1     );      //Use 800 KHz Sampling (0x1) or 1.6 MHz (0x10)
//  AD5940_WriteReg(REG_AFE_AFECON,       0x180840);      //Optional
  
  AD5940_WriteReg(REG_AFE_HPTIACON,     0x7C    );      //High Power TIA
  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );      //Initialize
  AD5940_WriteReg(REG_AFE_HPRTIACON,    0x0     );      //Use 200 Ohm RTIA, No Capacitance
  AD5940_WriteReg(REG_AFE_ULPTIASW0,    0x3180  );      //Initialize LPTIA
  AD5940_WriteReg(REG_AFE_SWCON,        0x28141 );      //Connect across Rcal; 0x28E11 if 2-wire (Local Sense)
//  AD5940_WriteReg(REG_AFE_AFECON,       0x380E40);      //Optional
  
  AD5940_WriteReg(REG_AFE_WGFCW,        3355440 );      //100 KHz Excitation Frequency; d’6710890 for 16 MHz Clk //SINEFCW in WGFCW, bits 0-23
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );      //600 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x80E   );      //InAmp = 2, Atten = False, 0x1B Update Rate fro HS/HP
  AD5940_WriteReg(REG_AFE_WGCON,        0x34    );      //Sine Wave
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384E40);      //Optional
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x221424);      //PGA Gain = 2, ADC Across Excitation Amp P-N
  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);      //ADC Raw Data, 2048 Sampling
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //Start ADC Conversion
  
  /* Measuring Across RCAL here */
  
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x200101);      //PGA Gain = 1, ADC Across HPTIA P-N
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //Start ADC Conversion
  
  /* Measuring Across TIA here */
  
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  /* Calculation of RTIA Impedance here*/
  
}

void AD5940JuulMeasurement()
{
  /* All these are optional, start here */
//  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );      //High Power Mode
//  
//  /* Initialize */
//  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );      
//  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );      
//  AD5940_WriteReg(REG_AFE_ADCCON,       0x0     );      
//  AD5940_WriteReg(REG_AFE_BUFSENCON,    0x37    );      
//  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);      
//  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);      
//  AD5940_WriteReg(REG_AFE_ADCBISCCON,   0x0     );      //Not present @datasheet
//  
//  AD5940_WriteReg(REG_AFE_HPOSCCON,     0x10    );      //32 MHz Clk (0x10) or 16 Mhz Clk (0x34)
//  AD5940_WriteReg(REG_AFE_ADCFILTERCON, 0x1     );      //Use 800 KHz Sampling (0x1) or 1.6 MHz (0x10)
//  AD5940_WriteReg(REG_AFE_AFECON,       0x180840);      //Optional
//  
//  AD5940_WriteReg(REG_AFE_HPTIACON,     0x7C    );      //High Power TIA
//  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );      //Initialize
//  AD5940_WriteReg(REG_AFE_HPRTIACON,    0x0     );      //Use 200 Ohm RTIA, No Capacitance
//  AD5940_WriteReg(REG_AFE_ULPTIASW0,    0x3180  );      //Initialize LPTIA
  /* All these are optional, end here */
  
  AD5940_WriteReg(REG_AFE_SWCON,        0x22955 );      //Connect across ZLoad
//  AD5940_WriteReg(REG_AFE_AFECON,       0x380E40);      //Optional
//  AD5940_WriteReg(REG_AFE_WGFCW,        3355440 );      //Connect across Rcal; 0x28E11 if 2-wire (Local Sense) //under WGFCW, bits 0-23 //Optional
  
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );      //15 mV Excitation Voltage
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x180F  );      //InAmp = 0.25, Atten = True, 0x1B Update Rate for HS/HP
//  AD5940_WriteReg(REG_AFE_WGCON,        0x34    );      //Sine Wave //Optional
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384E40);      //Optional
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x241424);      //PGA Gain = 9, ADC Across Excitation Amp P-N
//  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);      //ADC Raw Data, 2048 Sampling //Optional
  
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //Start ADC Conversion
  
  /* Measuring Across ZLoad here */
  
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  AD5940_WriteReg(REG_AFE_ADCCON,       0x210101);      //PGA Gain = 1.5, ADC Across HPTIA P-N
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //Start ADC Conversion
  
  /* Measuring Across TIA here */
  
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //ADC Idle
//  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //Optional
  
  /* Calculation of RTIA Impedance here */
  
}