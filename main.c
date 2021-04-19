// V5.3 CMSIS
// 18.3.2018
// V4.3 mit CMSIS

#include <stm32f10x.h>                       /* STM32F100 definitions         */
#include <stdio.h>
#include "USART1.h"
#include "DAC.h"
#include "TIM2_PWM.h"
#include "I2C.h"
#include "Temperatur.h"
#include "Display.h"
#include "Stepper.h"
#include "ESP_USART3.h"

// Positionierungsmodi
#define STEPMODE_NONE 0		// Schrittmotor aus
#define STEPMODE_FWD 1		// Schrittmotor dreht vorwärts
#define STEPMODE_BWD 2		// Schrittmotor dreht rückwärts
#define STEPMODE_POS 3		// Schrittmotor im Positionierungsmodus

//Sinustabelle
int sinus[32] = { 2048, 2419, 2775, 3104, 3392, 3628, 3803, 3911, 3948,
									3911, 3803, 3628, 3392, 3104, 2775, 2419, 2048, 1677,
									1321, 992, 704, 468, 293, 185, 148, 185, 293, 468,
									704, 992, 1321, 1677};

int stepdir = 0;  		// Richtung des Schrittmotors
int akt_pos=0;				// aktuelle Position des Schrittmotors
int ziel_pos=0;				// Zielposition des Schrittmotors
int stepmode;					// Modus des Schrittmotors
int tempon=0;					// Modus Temperaturmessung

/*----------------------------------------------------------------------------
   ExecuteCmd
 *----------------------------------------------------------------------------*/
void ExecuteCmd (void)
// Ausführung des eingegebenen Befehls
{
	  char led;
		int pos, wert,r,g,b,dp;
	  int puls;
	  int t;
	  char out_str[15]; 
	  int digit[4];

	switch (inputBuffer[0])
		{
			// Sinuswert ändern
			case 'p': 	sscanf(inputBuffer,"p%dw%d",&pos,&wert);
		            if ((pos>=0 && pos<32) && (wert>=0 && wert<4096))
									sinus[pos] = wert;
								break;
			// Servoposition anfahren
			case 's':  sscanf(inputBuffer,"s%d.",&puls);
								if (puls>=0 && puls<=100)
									TIM2_servo(100-puls);
								break;
			// RGB-LED setzen
			case 'r':  sscanf(inputBuffer,"r%dg%db%d",&r,&g,&b);
			          if ((r>=0 && r<256) && (g>=0 && g<256) && (b>=0 && b<256))
									TIM2_RGB(r,g,b);
								break;
			// einzelne Farbe der RGB-LED setzen
			case 'l':  sscanf(inputBuffer,"l%c%d",&led,&puls);
								if (puls>=0 && puls<256)
									switch (led)
										{ case 'r': TIM2_RGB(puls,-1,-1);	break;
											case 'g': TIM2_RGB(-1,puls,-1);	break;
											case 'b': TIM2_RGB(-1,-1,puls); break;
										}
								break;
			// Schrittmotor vorwärts
			case '+':	stepmode = STEPMODE_FWD;
                break;			
			// Schrittmotor rückwärts
			case '-':	stepmode = STEPMODE_BWD;
                break;
			// Schrittmotorposition anfahren
			case 'm':	stepmode = STEPMODE_POS;
								sscanf(inputBuffer,"m%d", &wert);
								if (wert>=0 && wert<=400)
	                ziel_pos =wert;
								break;
			// Anzeige Kommandos
			case 'h': 
								WriteString("Kommandos:\n\r");
								WriteString("\tp[pos]w[wert]:\t\tSinustabelle: an Stelle pos den Wert wert setzen\n\r");
								WriteString("\ts[pos]:\t\t\tServo auf Position pos fahren\n\r");
								WriteString("\tr[r]g[g]b[b]\t\tRGB-LED auf Farbe r/g/b setzen [0...255]\n\r");
								WriteString("\tl[r/g/b][wert]:\t\tBei RGB-LED Farbe r/g/b auf Wert wert setzen\n\r");
								WriteString("\t+:\t\t\tSchrittmotor dreht vorwaerts\n\r");
								WriteString("\t-:\t\t\tSchrittmotor dreht rueckwaerts\n\r");
								WriteString("\tm[pos]\t\t\tSchrittmotor auf Position pos fahren\n\r");
								WriteString("\ttr\t\t\tTemperatur messen\n\r");
								WriteString("\tta[0|1]\t\t\tTemperaturanzeige aus/an\n\r");
								WriteString("\tts[0/1/2/3]\t\tAufloesung Temperatur einstellen\n\r");
								WriteString("\tdb[b1],[b2],[b3],[b4]:\tBitmuster b1, b2, b3, b4 auf Digits anzeigen\n\r");
							  WriteString("\tdz[wert],[dezpos]:\tZahl Wert mit Dezimalpunktposition dezpos anzeigen\n\r\n\r");
								break;	
			//Temperatur
			case 't':	
								// auslesen
								if (inputBuffer[1] == 'r')
			          {
									t = read_Temp();
			            sprintf(out_str," T = %f C\n\r",(float)t/256.0);
			            WriteString(out_str);
								}
								// Auflösung setzen
								if (inputBuffer[1] == 's')
								 {
									 set_TempRes( (int)(inputBuffer[2]-'0'));
								 }
								 // Temperaturanzeige an/aus
								 if (inputBuffer[1] == 'a')
								 {
									 if (inputBuffer[2] == '0') {
											tempon=0;
											stepmode = STEPMODE_NONE;
									 } else if (inputBuffer[2] == '1') {
											tempon=1;
										 stepmode = STEPMODE_POS;
									 }
								 }
								break;
				//Display
			 case 'd':					
								// Bitmuster anzeigen
				        if (inputBuffer[1] == 'b') // Display Bits
			          { 
									sscanf(inputBuffer,"db%x,%x,%x,%x",&digit[0],&digit[1],&digit[2],&digit[3]);
									Display_I2C(digit);
								}
								 
								// Zahl anzeigen
								if (inputBuffer[1] == 'z')
								{ 
									 sscanf(inputBuffer,"dz%d,%d", &wert, &dp);
									 Display_Zahl(wert,dp);
								}
								break;										
		}
}

/*----------------------------------------------------------------------------
   Temp_Display
 *----------------------------------------------------------------------------*/

void Temp_Display()
// Temperatur auslesen und anzeigen, Schrittmotor und Servo positionieren
{
	int Temp;
	int puls;	
	#define dp 0	
	
	Temp = read_Temp();
  Temp = (float)Temp / 2.56; // Grad * 100
	Display_Zahl(Temp,2);
	ziel_pos = (Temp-2000)/5;
	puls = 100-(Temp-2000)/10;
	TIM2_servo(puls);
}	

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void)
{ 	
   int i;
	int displaycount = 0;
	
	//------- Init ----------
	InitDAC();
	InitUSART1();		// Terminal
	
   InitTIM2_PWM();
	InitStepper();
	I2C_init( 24000000, 0x08 );
   WriteString("\n\r\n\rV5\n\r");	

	while (1) 
	{
		// nur bei jedem 10000ten Durchlauf wird die Temperatur abgefragt und angezeigt
		displaycount = (displaycount)% 10000;
		if (displaycount==1 && tempon)
			Temp_Display();
		displaycount++;
		
      // Sinus ausgeben
		for (i=0;i<32;i++)
		{
			WriteDAC(sinus[i]);
			if (cmdflag)			// anliegendes Eingabekommando
			{
				ExecuteCmd();
				cmdflag = 0;
			}
      }
	}// while
}

/*----------------------------------------------------------------------------
  Interrupt-Handler SysTick
 *----------------------------------------------------------------------------*/
void SysTick_Handler (void)
{
	static int steps[8] = {0x01, 0x03, 0x02, 0x06, 0x04, 0x0c, 0x08, 0x09};
	static int stepnr = 0;

	if (stepmode == STEPMODE_NONE)	// keine Schrittmotorbewegung -> zurück
		return;
	
	Step_Out(steps[stepnr]);
	stepnr = stepnr + stepdir;
	akt_pos = (akt_pos + stepdir) % 401; 	// aktuelle Position erhöhen und auf Werte zwischen 0...400 begrenzen
	if (stepnr >7) stepnr = 0;
	if (stepnr < 0) stepnr = 7;
	if (stepmode == STEPMODE_POS) {				// Modus Positionierung
		if (ziel_pos > akt_pos) stepdir = 1;
		if (ziel_pos < akt_pos) stepdir = -1;
		if (ziel_pos == akt_pos) stepdir = 0;   
	} else if (stepmode == STEPMODE_FWD)	// Modus Vorwärtsdrehen
		stepdir = 1;
	else																	// Modus Rückwärtsdrehen
		stepdir = -1;
}

