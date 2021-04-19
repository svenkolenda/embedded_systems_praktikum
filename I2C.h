/* --------------------------------------------------------------------
	Hardware Definition STM32F100RB
	Haunstetter / 02.11.2015
	Datei: I2C.h
   --------------------------------------------------------------------
*/

/* ================================================================================ */
#define EI {unsigned int r0 = 0; __asm { msr primask,r0}} // enable configurable interrupts
#define DI {unsigned int r0 = 1; __asm { msr primask,r0}} // disable configurable interrupts
/* ================================================================================ */

// Prototypen
//void I2C_config( unsigned int clock, unsigned char address );
void I2C_init( unsigned int clock, unsigned char address );
void I2C_write( const unsigned char * buf, unsigned int n, unsigned char dest );
void I2C_read( unsigned char * buf, unsigned int n, unsigned char source );
