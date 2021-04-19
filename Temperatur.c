
#include "I2C.h"


int read_Temp()
{
	 #define TEMP_ADR    0x48             // Slave Adresse Temperaturfühler
    unsigned char i2c_buf[10];					// Buffer für I2C-Kommunikation
	  short int temp_value;								// temp_value als 16-Bit Wert
																				// dadurch werden negative Werte richtig behandelt
				
	
 
    i2c_buf[0] = 0x00;                  // Register-Pointer =>Temp-Register 
    I2C_write( i2c_buf, 1, TEMP_ADR );  // Temperatur Register adressieren
	
    I2C_read( i2c_buf, 2, TEMP_ADR );   // Temperatur einlesen
    temp_value = i2c_buf[0]*256 + i2c_buf[1]; 
	
	  return (int) temp_value;						// Typecast erweitert vorzeichenrichtig

}

void set_TempRes(int resolution)
{ 
	#define TEMP_ADR    0x48                    // Slave Adresse Temperaturfühler
    unsigned char i2c_buf[10];
	
 
    i2c_buf[0] = 0x01;                  // Register-Pointer =>Config-Register 
	  i2c_buf[1] = resolution<<5;         //  Auf Bit 5-6 schieben 
    I2C_write( i2c_buf, 2, TEMP_ADR );  // Temperatur Register adressieren
}
