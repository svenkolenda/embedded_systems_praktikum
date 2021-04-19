#include <stm32f10x.h>   
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>
#include "Timer6.h"

#define ms20  20000 // 20 ms
#define ms02  200  //  0,2 ms  
#define ms1  1000  //  1 ms
#define ms001 10  //  0,01 ms

void InitTIM2_PWM(void)
// Initialisierung des Timer2
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;														//Strukturen anlegen:
	TIM_OCInitTypeDef TIM_OCInitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPAEN,ENABLE);												//Port A im RCC anschalten:
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;	//Pins PA0-PA3 als Alternate Function Output festlegen:
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);												//TIM2 im RCC anschalten:
	TIM_TimeBaseStructure.TIM_Prescaler=239;																	//Prescaler, Clockteiler und Reload Wert festlegen:
	TIM_TimeBaseStructure.TIM_Period=2000;
	TIM_TimeBaseStructure.TIM_ClockDivision=0;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;									//Timer soll hochzaehlen:
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);
	TIM_Cmd(TIM2,ENABLE);																											//Timer 2 anschalten:
	
	TIM_OCInitStructure.TIM_OCMode=TIM_OCMode_PWM1;														//Timer 2 Modus auf PWM1 festlegen:
	TIM_OCInitStructure.TIM_OutputState=TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Enable;
	TIM_OCInitStructure.TIM_Pulse=0;
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_Low;										//Polarity Low, da die RGB-LED Low-aktiv ist
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_Low;									//(keine Werteumkehr in Executecmd() noetig)!:
	TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;
	TIM_OCInitStructure.TIM_OCNIdleState=TIM_OCNIdleState_Reset;
	TIM_OC2Init(TIM2,&TIM_OCInitStructure);
	TIM_OC3Init(TIM2,&TIM_OCInitStructure);
	TIM_OC4Init(TIM2,&TIM_OCInitStructure);
	
	TIM_OCInitStructure.TIM_OCPolarity=TIM_OCPolarity_High;										//Polarity High, da das Servo High-aktiv ist:
	TIM_OCInitStructure.TIM_OCNPolarity=TIM_OCNPolarity_High;
	TIM_OC1Init(TIM2,&TIM_OCInitStructure);
	//Init_Timer6_7();
}


void TIM2_servo( int puls)
{
  if (puls >=0 && puls <=100)
   TIM_SetCompare1(TIM2,puls+100);
}


void TIM2_RGB(int red, int green, int blue)
{
    if ((red >=0)&& (red <=100)) 			TIM_SetCompare2(TIM2,(red*20));	
		if ((green >=0)&& (green <=100))	TIM_SetCompare3(TIM2,(green*20));	
		if ((blue >=0)&& (blue <=100))		TIM_SetCompare4(TIM2,(blue*20)); 
}
