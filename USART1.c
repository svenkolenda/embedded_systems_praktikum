#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include <stm32f10x.h>                       /* STM32F103 definitions         */
#include <string.h>

#define USART1_PORT_CFG 0x4A0						// Pin9 Alternate Function Output, Pin 10	Floating Input
#define USART3_PORT_CFG 0x4A00					// Pin10 Alternate Function Output, Pin 11	Floating Input

/*----------------------------------------------------------------------------
  InitUSART
 *----------------------------------------------------------------------------*/
 
int cmdflag;												   // Flag für abgeschlossenes Kommando an USART1
char inputBuffer[32];									   // Puffer für eingelesene Zeichen
int bufferPos = 0;										   // Pufferposition

void Init_NVICUSART1(void)
// NVIC für USART1 einschalten
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART1_IRQn;	//USART1 Interrupt im NVIC Aktivieren:
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=5;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
 

void InitUSART1(void)
// USART1 initialisieren	
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;				   //Strukturen anlegen:
	
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPAEN,ENABLE);	//Port A im RCC anschalten:
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_9;				//Pin 9 als Alternate Function Output festlegen:
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);					//Port initialisieren:
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;				//Pin 10 als Eingang festlegen:
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);					//Port initialisieren:
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);	//USART1 im RCC anschalten:

//------------------------------------------------------------
// Ab hier die Initialisierung der USART-Schnittstelle eintragen
   
   USART_StructInit(&USART_InitStructure);               //Strukt mit Default Werten befüllen
   USART_InitStructure.USART_BaudRate = 0x00002580;      //Baudrate 9600
   
   USART_Init(USART1, &USART_InitStructure);             //Initialisierung

   USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);        //Interrupt einschalten
   
   USART_Cmd(USART1, ENABLE);                            //USART Enable
   
//------------------------------------------------------------
   
	Init_NVICUSART1();												// NVIC aktivieren
}



/*----------------------------------------------------------------------------
 ReadChar
 *----------------------------------------------------------------------------*/
char ReadChar()
{ 
   while (!USART_GetFlagStatus(USART1, USART_FLAG_RXNE)) {}
   char c = (char) USART_ReceiveData(USART1);
	return c;
}
/*----------------------------------------------------------------------------
 WriteChar
 *----------------------------------------------------------------------------*/
void WriteChar(char c)
{
   while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE)) {}
   USART_SendData(USART1, (uint16_t) c);
   while (!USART_GetFlagStatus(USART1, USART_FLAG_TXE)) {}
   return;
}



/*----------------------------------------------------------------------------
  WriteString
 *----------------------------------------------------------------------------*/
void WriteString(char *str)
{
	while (*str != 0)
	{
		WriteChar(*(str++));
	}
	
   return;
}



// ******************************************
// Interruptroutine für USART1
//
void USART1_IRQHandler (void)
{
	char inputChar;	// Eingelesenes Zeichen
	
	inputChar = USART1->DR;						      // Empfangenes Zeichen holen 
	inputBuffer[bufferPos] = inputChar;			   // und in Puffer speichern
	bufferPos++;										   // Pufferzeiger erhöhen
	WriteChar(inputChar);						      // Echo zurückschicken
	
	if (inputChar == '\r' || inputChar == '.')   // wenn Enter gedrückt
	{
		inputBuffer[bufferPos] = 0; 			      // String abschließen
		bufferPos = 0;									   // Pufferzeiger zurücksetzen
		cmdflag=1;										   // Kommandoflag setzen
		WriteString("\n\r");					         // neue Zeile beginnen
	}

}

