// Funktionen zum Betrieb des Schrittmotors
#include "Stepper.h"

#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/*----------------------------------------------------------------------------
  Init_SystemTick function
 *----------------------------------------------------------------------------*/
void InitSysTick (int load) {
 
	SysTick->LOAD = load;    // Timer laden
	SysTick->CTRL = 0x3 ;	//Timer starten + Interrupt Enable
}


/*----------------------------------------------------------------------------
  Init_Stepper function
 *----------------------------------------------------------------------------*/
void InitStepper(void)
	{	
	GPIO_InitTypeDef GPIO_InitStructure;
	//Port B im RCC anschalten:
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPBEN,ENABLE);
	//Werte in CRL und CRH schreiben:
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	//Port initialisieren:
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	InitSysTick (25000); // SysTick starten mit 10 ms Takt (30000 / (24MHz/8))  = 100 Hz )
}

/*----------------------------------------------------------------------------
  Step_Out(int step_pattern)
 *----------------------------------------------------------------------------*/
void Step_Out(int step_pattern)
{	
	GPIO_Write(GPIOB,step_pattern<<8); // step_pattern um 8 Stellen schieben und ausgeben	
}

