#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_dac.h"
#include <stm32f10x.h>    
/*----------------------------------------------------------------------------
  DAC
 *----------------------------------------------------------------------------*/

void InitDAC(void)
// Initialisierung des DAC-Konverters
{
	DAC_InitTypeDef DAC_InitStructure;
	RCC_APB1PeriphClockCmd(RCC_APB1ENR_DACEN,ENABLE);
	DAC_StructInit(&DAC_InitStructure);
	DAC_Init(DAC_Channel_1,&DAC_InitStructure);
	DAC_Cmd(DAC_Channel_1,ENABLE);
}

void WriteDAC(int analog)
// Ausgabe  des Analogwertes
{
	DAC_SetChannel1Data(DAC_Align_12b_R,analog);	
}

