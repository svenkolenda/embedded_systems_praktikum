#include <stm32f10x.h>   
#include <stm32f10x_gpio.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_tim.h>


enum note {c,d,e,f,g,a,h,c1,d1,e1,f1,g1,h1,c2};
// Frequenzen in Herz
int Frequenz [15] = {261,293,328,349,391,440,493,523,587,659,698,783,880,987,1046};
int AktNote = 0;

typedef struct {int note; int dauer;} Note_Type;

Note_Type Lied[50] = {  {g,150},{a,50},{g,100},{e,300},{g,150},{a,50},{g,100},{e,300},// Stille Nacht Heilige Nacht
                        {d1,200},{d1,100},{h,300},{c1,200},{c1,100},{g,300},//Alles Schläft//Einsam wacht
                        {a,200}, {a,100},{c1,150},{h,50},{a,100},// Nur das traute Hoch
                        {g,150},{a,50},{g,100},{e,300},//heilige Paar
												{a,200},{a,100},{c1,150},{h,50},{a,100}, // holder Knabe mit
                        {g,  150},{a,  50},{g,  100},{e,  300},//lockigem Haar
                        {d1,200},{d1,100},{f1,150},{d1,50},{h,100},{c1,300},{e1,200},// Schlaf in himmlischer//Ruh
                        {c1,150},{g,50},{e,100},{g,150},{f,50},{d,100},{c,300},{c,200},// Schlaf//in himmlischer  //Ruh
                        {-1, 0}};


void Init_Timer6_7(void)
{
	 //int noten_freq, noten_periode ;
	NVIC_InitTypeDef NVIC_InitStructure;	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;	
	
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);	//TIM6 im RCC anschalten:
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,ENABLE);	//TIM7 im RCC anschalten:
	TIM_TimeBaseStructure.TIM_Prescaler=12000;				// 24 MHz /12000 = 2 KHz => 0,5 ms
	TIM_TimeBaseStructure.TIM_Period= 50;
	TIM_TimeBaseStructure.TIM_ClockDivision=0;
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;									//Timer soll hochzaehlen:
	TIM_TimeBaseInit(TIM6,&TIM_TimeBaseStructure);
	
	TIM_TimeBaseStructure.TIM_Prescaler=11;				// 24 MHz /(11+1) => 2 MHz
	TIM_TimeBaseStructure.TIM_Period= 50;
	TIM_TimeBaseInit(TIM7,&TIM_TimeBaseStructure);

// NVIC

	NVIC_InitStructure.NVIC_IRQChannel=TIM6_DAC_IRQn;					//Timer6 Interrupt im NVIC Aktivieren:
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=5;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=5;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;
	NVIC_InitStructure.NVIC_IRQChannel=TIM7_IRQn;
	NVIC_Init(&NVIC_InitStructure);	
	
// IntEnable
	TIM_ITConfig (TIM6,TIM_IT_Update,ENABLE);	
  TIM_Cmd(TIM6,ENABLE);	
	TIM_ITConfig (TIM7,TIM_IT_Update,ENABLE);	
	TIM_Cmd(TIM7,ENABLE);


}



void TIM6_DAC_IRQHandler(void)
{ int noten_freq, noten_periode ;

	// Akt Note Tondauer holen
	AktNote++;
	if (Lied[AktNote].note == -1) AktNote = 0;
	TIM_SetAutoreload(TIM6,Lied[AktNote].dauer*10);

	// Note über Tim7 ausgeben
	noten_freq = Frequenz[Lied[AktNote].note];
	noten_periode = 1000000 / noten_freq;
	TIM_SetAutoreload(TIM7,noten_periode);

	TIM_ClearFlag( TIM6,TIM_FLAG_Update	) ;
	//GPIOB->ODR = GPIOB->ODR ^	0x1000; //Pin12 Toggle
}
	
void TIM7_IRQHandler(void)
	{ 	volatile int state = 1;

  GPIOB->ODR= GPIOB->ODR ^ 0x100; //Pin8
	for (state=0; state <200; state++) {};
	GPIOB->ODR= GPIOB->ODR ^ 0x100; //Pin8
	//GPIOB->ODR = GPIOB->ODR ^	0x2000; //Pin8 Toggle
	TIM_ClearFlag( TIM7,TIM_FLAG_Update	) ;	
	
}	
