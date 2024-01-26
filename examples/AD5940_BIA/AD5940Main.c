/*!
 *****************************************************************************
 @file:    AD5940Main.c
 @author:  Neo Xu
 @brief:   Used to control specific application and process data.
 -----------------------------------------------------------------------------

Copyright (c) 2017-2019 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 
*****************************************************************************/
/** 
 * @addtogroup AD5940_System_Examples
 * @{
 *  @defgroup BioElec_Example
 *  @{
  */
#include "ad5940.h"
#include "AD5940.h"
#include <stdio.h>
#include "string.h"
#include "math.h"
#include "BodyImpedance.h"

#define APPBUFF_SIZE 512
uint32_t AppBuff[APPBUFF_SIZE];

/* It's your choice here how to do with the data. Here is just an example to print them to UART */
int32_t BIAShowResult(uint32_t *pData, uint32_t DataCount)
{
  float freq;

  fImpPol_Type *pImp = (fImpPol_Type*)pData;
  AppBIACtrl(BIACTRL_GETFREQ, &freq);

  printf("Freq:%.2f ", freq);
  /*Process data*/
  for(int i=0;i<DataCount;i++)
  {
    printf("RzMag: %f Ohm , RzPhase: %f \n",pImp[i].Magnitude,pImp[i].Phase*180/MATH_PI);
  }
  return 0;
}

/* Initialize AD5940 basic blocks like clock */
static int32_t AD5940PlatformCfg(void)
{
  CLKCfg_Type clk_cfg;
  FIFOCfg_Type fifo_cfg;
  AGPIOCfg_Type gpio_cfg;

  /* Use hardware reset */
  AD5940_HWReset();
  /* Platform configuration */
  AD5940_Initialize();
  /* Step1. Configure clock */
  clk_cfg.ADCClkDiv = ADCCLKDIV_1;
  clk_cfg.ADCCLkSrc = ADCCLKSRC_HFOSC;
  clk_cfg.SysClkDiv = SYSCLKDIV_1;
  clk_cfg.SysClkSrc = SYSCLKSRC_HFOSC;
  clk_cfg.HfOSC32MHzMode = bFALSE;
  clk_cfg.HFOSCEn = bTRUE;
  clk_cfg.HFXTALEn = bFALSE;
  clk_cfg.LFOSCEn = bTRUE;
  AD5940_CLKCfg(&clk_cfg);
  /* Step2. Configure FIFO and Sequencer*/
  fifo_cfg.FIFOEn = bFALSE;
  fifo_cfg.FIFOMode = FIFOMODE_FIFO;
  fifo_cfg.FIFOSize = FIFOSIZE_4KB;                       /* 4kB for FIFO, The reset 2kB for sequencer */
  fifo_cfg.FIFOSrc = FIFOSRC_DFT;
  fifo_cfg.FIFOThresh = 4;//AppBIACfg.FifoThresh;        /* DFT result. One pair for RCAL, another for Rz. One DFT result have real part and imaginary part */
  AD5940_FIFOCfg(&fifo_cfg);                             /* Disable to reset FIFO. */
  fifo_cfg.FIFOEn = bTRUE;  
  AD5940_FIFOCfg(&fifo_cfg);                             /* Enable FIFO here */
  
  /* Step3. Interrupt controller */
  
  AD5940_INTCCfg(AFEINTC_1, AFEINTSRC_ALLINT, bTRUE);           /* Enable all interrupt in Interrupt Controller 1, so we can check INTC flags */
  AD5940_INTCCfg(AFEINTC_0, AFEINTSRC_DATAFIFOTHRESH, bTRUE);   /* Interrupt Controller 0 will control GP0 to generate interrupt to MCU */
  AD5940_INTCClrFlag(AFEINTSRC_ALLINT);
  /* Step4: Reconfigure GPIO */
  gpio_cfg.FuncSet = GP6_SYNC|GP5_SYNC|GP4_SYNC|GP2_TRIG|GP1_SYNC|GP0_INT;
  gpio_cfg.InputEnSet = AGPIO_Pin2;
  gpio_cfg.OutputEnSet = AGPIO_Pin0|AGPIO_Pin1|AGPIO_Pin4|AGPIO_Pin5|AGPIO_Pin6;
  gpio_cfg.OutVal = 0;
  gpio_cfg.PullEnSet = 0;

  AD5940_AGPIOCfg(&gpio_cfg);
  AD5940_SleepKeyCtrlS(SLPKEY_UNLOCK);  /* Allow AFE to enter sleep mode. */
  return 0;
}

/* !!Change the application parameters here if you want to change it to none-default value */
void AD5940BIAStructInit(void)
{
  AppBIACfg_Type *pBIACfg;
  
  AppBIAGetCfg(&pBIACfg);
  
  pBIACfg->SeqStartAddr = 0;
  pBIACfg->MaxSeqLen = 512; /** @todo add checker in function */
  
  pBIACfg->RcalVal = 10000.0;
  pBIACfg->DftNum = DFTNUM_8192;
  pBIACfg->NumOfData = -1;      /* Never stop until you stop it manually by AppBIACtrl() function */
  pBIACfg->BiaODR = 20;         /* ODR(Sample Rate) 20Hz */
  pBIACfg->FifoThresh = 4;      /* 4 */
  pBIACfg->ADCSinc3Osr = ADCSINC3OSR_2;
}

void AD5940_Main(void)
{
  static uint32_t IntCount;
  static uint32_t count;
  uint32_t temp;
  
  AD5940PlatformCfg();
  AD5940BIAStructInit(); /* Configure your parameters in this function */
  AppBIAInit(AppBuff, APPBUFF_SIZE);    /* Initialize BIA application. Provide a buffer, which is used to store sequencer commands */
  AppBIACtrl(BIACTRL_START, 0);         /* Control BIA measurement to start. Second parameter has no meaning with this command. */
  
  
#if 1
  AD5940_WriteReg(0x000002F0, 0x821FD );      //PMBW
  AD5940_WriteReg(0x00000014, 0x0     );      //CLKSEL
  AD5940_WriteReg(0x00000008, 0x440   );      //CLKCON0
  AD5940_WriteReg(0x000001A8, 0x0     );      //ADCCON
  AD5940_WriteReg(0x00000180, 0x37    );      //BUFSENCON
  AD5940_WriteReg(0x0000038C, 0x5F3D04);      //ADCBUFCON
  AD5940_WriteReg(0x00000000, 0x180040);      //AFECON
  AD5940_WriteReg(0x000003E4, 0x0     );      //AFECON
  AD5940_WriteReg(0x000000BC, 0x10    );      //HSOSCCON @datasheet
  AD5940_WriteReg(0x00000044, 0x1     );      //ADCFILTERCON
  AD5940_WriteReg(0x00000000, 0x180840);      //AFECON Optional
  AD5940_WriteReg(0x000000FC, 0x7C    );      //HSTIACON @datasheet
  AD5940_WriteReg(0x0000000C, 0x2FFFF );      //SWCON
  AD5940_WriteReg(0x000000F0, 0x0     );      //HSRTIACON @datasheet
  AD5940_WriteReg(0x000000E4, 0x3180  );      //LPTIASW0 @datasheet
  AD5940_WriteReg(0x0000000C, 0x28141 );      //SWCON
  AD5940_WriteReg(0x00000000, 0x380E40);      //AFECON
  AD5940_WriteReg(0x00000030, 3355440 );      //under WGFCW, bits 0-23
  AD5940_WriteReg(0x0000003C, 759     );      //WGAMPLITUDE
  AD5940_WriteReg(0x00000010, 0x80E   );      //HSDACCON @datasheet
  AD5940_WriteReg(0x00000014, 0x34    );      //WGCON
  AD5940_WriteReg(0x00000000, 0x384E40);      //AFECON
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON
  AD5940_WriteReg(0x000001A8, 0x221424);      //ADCCON
  AD5940_WriteReg(0x000000D0, 0x200091);      //DFTCON
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x39CFC0);      //AFECON
  // Measuring Across RCAL here 
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON
  AD5940_WriteReg(0x000001A8, 0x200101);      //ADCCON
  AD5940_WriteReg(0x00000000, 0x39CFC0);      //AFECON
  // Measuring Across TIA here 
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON
  // Calculation of RTIA Impedance 
  AD5940_WriteReg(0x000002F0, 0x821FD );      //PMBW Optional
  AD5940_WriteReg(0x00000014, 0x0     );      //CLKSEL Optional
  AD5940_WriteReg(0x00000008, 0x440   );      //CLKCON0 Optional
  AD5940_WriteReg(0x000001A8, 0x0     );      //ADCCON Optional
  AD5940_WriteReg(0x00000180, 0x37    );      //BUFSENCON Optional
  AD5940_WriteReg(0x0000038C, 0x5F3D04);      //ADCBUFCON Optional
  AD5940_WriteReg(0x00000000, 0x180040);      //AFECON Optional
  AD5940_WriteReg(0x000003E4, 0x0     );      //AFECON Optional
  AD5940_WriteReg(0x000000BC, 0x10    );      //HSOSCCON @datasheet
  AD5940_WriteReg(0x00000044, 0x1     );      //ADCFILTERCON Optional
  AD5940_WriteReg(0x00000000, 0x180840);      //AFECON Optional
  AD5940_WriteReg(0x000000FC, 0x7C    );      //HSTIACON @datasheet Optional
  AD5940_WriteReg(0x0000000C, 0x2FFFF );      //SWCON Optional
  AD5940_WriteReg(0x000000F0, 0x0     );      //HSRTIACON @datasheet Optional
  AD5940_WriteReg(0x000000E4, 0x3180  );      //LPTIASW0 @datasheet Optional
  AD5940_WriteReg(0x0000000C, 0x22955 );      //SWCON
  AD5940_WriteReg(0x00000000, 0x380E40);      //AFECON Optional
  AD5940_WriteReg(0x00000030, 3355440 );      //under WGFCW, bits 0-23 Optional
  AD5940_WriteReg(0x0000003C, 759     );      //WGAMPLITUDE
  AD5940_WriteReg(0x00000010, 0x180F  );      //HSDACCON @datasheet
  AD5940_WriteReg(0x00000014, 0x34    );      //WGCON Optional
  AD5940_WriteReg(0x00000000, 0x384E40);      //AFECON Optional
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON Optional
  AD5940_WriteReg(0x000001A8, 0x241424);      //ADCCON
  AD5940_WriteReg(0x000000D0, 0x200091);      //DFTCON Optional
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x39CFC0);      //AFECON
  // Measuring Across ZLoad here 
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON
  AD5940_WriteReg(0x000001A8, 0x210101);      //ADCCON
  AD5940_WriteReg(0x00000000, 0x39CFC0);      //AFECON
  // Measuring Across TIA here 
  AD5940_WriteReg(0x00000000, 0x384FC0);      //AFECON
  AD5940_WriteReg(0x00000000, 0x384EC0);      //AFECON
  // Calculation of RTIA Impedance 
  
  
#elif 0
  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );      //PMBW
  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );      //CLKSEL
  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );      //CLKCON0
  AD5940_WriteReg(REG_AFE_ADCCON,       0x0     );      //ADCCON
  AD5940_WriteReg(REG_AFE_BUFSENCON,    0x37    );      //BUFSENCON
  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);      //ADCBUFCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);      //AFECON
  AD5940_WriteReg(/*ADCBISCCON*/,       0x0     );      //AFECON
  AD5940_WriteReg(REG_AFE_HPOSCCON,     0x10    );      //HSOSCCON @datasheet
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, 0x1     );      //ADCFILTERCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x180840);      //AFECON Optional
  AD5940_WriteReg(REG_AFE_HSTIACON,     0x7C    );      //HSTIACON @datasheet
  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );      //SWCON
  AD5940_WriteReg(REG_AFE_HSRTIACON,    0x0     );      //HSRTIACON @datasheet
  AD5940_WriteReg(REG_AFE_LPTIASW0,     0x3180  );      //LPTIASW0 @datasheet
  AD5940_WriteReg(REG_AFE_SWCON,        0x28141 );      //SWCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x380E40);      //AFECON
  AD5940_WriteReg(REG_AFE_WGFCW,        3355440 );      //under WGFCW, bits 0-23
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );      //WGAMPLITUDE
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x80E   );      //HSDACCON @datasheet
  AD5940_WriteReg(REG_AFE_WGCON,        0x34    );      //WGCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384E40);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON
  AD5940_WriteReg(REG_AFE_ADCCON,       0x221424);      //ADCCON
  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);      //DFTCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //AFECON
  // Measuring Across RCAL here 
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON
  AD5940_WriteReg(REG_AFE_ADCCON,       0x200101);      //ADCCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //AFECON
  // Measuring Across TIA here 
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON
  // Calculation of RTIA Impedance 
  AD5940_WriteReg(REG_AFE_PMBW,         0x821FD );      //PMBW Optional
  AD5940_WriteReg(REG_AFECON_CLKSEL,    0x0     );      //CLKSEL Optional
  AD5940_WriteReg(REG_AFECON_CLKCON0,   0x440   );      //CLKCON0 Optional
  AD5940_WriteReg(REG_AFE_ADCCON,       0x0     );      //ADCCON Optional
  AD5940_WriteReg(REG_AFE_BUFSENCON,    0x37    );      //BUFSENCON Optional
  AD5940_WriteReg(REG_AFE_ADCBUFCON,    0x5F3D04);      //ADCBUFCON Optional
  AD5940_WriteReg(REG_AFE_AFECON,       0x180040);      //AFECON Optional
  AD5940_WriteReg(/*ADCBISCCON*/,       0x0     );      //AFECON Optional
  AD5940_WriteReg(REG_AFE_HPOSCCON,     0x10    );      //HSOSCCON @datasheet Optional
  AD5940_WriteReg(REG_AFE_ADCFILTERCON, 0x1     );      //ADCFILTERCON Optional
  AD5940_WriteReg(REG_AFE_AFECON,       0x180840);      //AFECON Optional Optional
  AD5940_WriteReg(REG_AFE_HSTIACON,     0x7C    );      //HSTIACON @datasheet Optional
  AD5940_WriteReg(REG_AFE_SWCON,        0x2FFFF );      //SWCON Optional
  AD5940_WriteReg(REG_AFE_HSRTIACON,    0x0     );      //HSRTIACON @datasheet Optional
  AD5940_WriteReg(REG_AFE_LPTIASW0,     0x3180  );      //LPTIASW0 @datasheet Optional
  AD5940_WriteReg(REG_AFE_SWCON,        0x22955 );      //SWCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x380E40);      //AFECON Optional
  AD5940_WriteReg(REG_AFE_WGFCW,        3355440 );      //under WGFCW, bits 0-23 Optional
  AD5940_WriteReg(REG_AFE_WGAMPLITUDE,  759     );      //WGAMPLITUDE
  AD5940_WriteReg(REG_AFE_HSDACCON,     0x180F  );      //HSDACCON @datasheet
  AD5940_WriteReg(REG_AFE_WGCON,        0x34    );      //WGCON Optional
  AD5940_WriteReg(REG_AFE_AFECON,       0x384E40);      //AFECON Optional
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON Optional
  AD5940_WriteReg(REG_AFE_ADCCON,       0x241424);      //ADCCON
  AD5940_WriteReg(REG_AFE_DFTCON,       0x200091);      //DFTCON Optional
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //AFECON
  // Measuring Across ZLoad here 
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON
  AD5940_WriteReg(REG_AFE_ADCCON,       0x210101);      //ADCCON
  AD5940_WriteReg(REG_AFE_AFECON,       0x39CFC0);      //AFECON
  // Measuring Across TIA here 
  AD5940_WriteReg(REG_AFE_AFECON,       0x384FC0);      //AFECON
  AD5940_WriteReg(REG_AFE_AFECON,       0x384EC0);      //AFECON
  // Calculation of RTIA Impedance 
#endif
  
  
  
  
  while(1)
  {
    /* Check if interrupt flag which will be set when interrupt occurred. */
//    if(AD5940_GetMCUIntFlag())
//    {
//      IntCount++;
//      AD5940_ClrMCUIntFlag(); /* Clear this flag */
//      temp = APPBUFF_SIZE;
//      AppBIAISR(AppBuff, &temp); /* Deal with it and provide a buffer to store data we got */
//      BIAShowResult(AppBuff, temp); /* Show the results to UART */
//    }
//    count++;
//    if(count > 1000000)
//    {
//      count = 0;
//      //AppBIAInit(0, 0);    /* Re-initialize BIA application. Because sequences are ready, no need to provide a buffer, which is used to store sequencer commands */
//      //AppBIACtrl(BIACTRL_START, 0);          /* Control BIA measurement to start. Second parameter has no meaning with this command. */
//    }
  }
}

/**
 * @}
 * @}
 * */
 
