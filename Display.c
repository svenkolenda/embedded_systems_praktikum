#include "I2C.h"

#include "Display.h"

#define Display_ADR 0x38						// I2C-Bus-Adresse des LED-Displays

int Digits[]={0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};


void Display_Zahl (int Number, int dp)
{
// zeigt im Display Zahl Number zwischen 0000 und 9999 an
// dp (decimal point = Anzahl der Nachkommastellen
{
	int Ziffer, Stelle ;
	int digit[4];
	
	
	if (Number<0 || Number >9999)
		return;
	
	for (Stelle=3;Stelle>=0;Stelle--)
	{
  	Ziffer = Number % 10;
		Number=Number/10;
		digit[Stelle] = Digits[Ziffer];
	}
	if ((dp>0) && (dp<=4))
		 digit[3-dp] |= (1<<7);	
	
	// Display an
	Display_I2C(digit);
}

}

void Display_I2C( int dig[])
// Gibt ein I2C-Kommando an 7-Segment Anzeige

{
	unsigned char i2c_buf[8];
	i2c_buf[0]=0x00;				// ControlByte auswählen
	i2c_buf[1]= 0x17;//ctrl= 4 digits,3mA
	i2c_buf[2]= dig[0];
	i2c_buf[3]= dig[1];
	i2c_buf[4]= dig[2];
	i2c_buf[5]= dig[3];
	I2C_write(i2c_buf, 6, Display_ADR);

}


