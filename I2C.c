/* ------------------------------------------------------------------------------------------------
	I2C Schnittstelle
	Haunstetter (02.11.2015)
	Korrektur Till 5.4.2017
	I2C 1 (STM32vlDiscovery): SCL = PB6, SDA = PB7
	ohne Interrupt, 100kHz Takt
  Datei: i2c.c
   ------------------------------------------------------------------------------------------------
*/
#include <stm32f10x.h>
#include "I2C.h"


/* ====================================================================
   I2C Config Routine (Standard Mode, 100kHz)
   ==================================================================== */
void I2C_config(
	unsigned int clock                  // Taktrate
	,unsigned char address              // eigene 7-Bit Adresse
)
{
	unsigned int freq = (unsigned short) (clock/1000000); // Bustakt Frequenz Kennung (MHz)
	I2C1->CR1 |= I2C_CR1_SWRST;         // alles auf Anfangszustand setzen
	I2C1->CR1 &= ~I2C_CR1_SWRST;
    
    // Standards: Hardware angehalten; F_S und DUTY stehen für 100kHz Mode; 7-Bit Adressformat
	I2C1->CR2 |= freq & 0x3f;           // Bustaktfrequenz in MHz (SCL 400kHz braucht freq%10==0)
	
	I2C1->CCR |= (5*freq) & 0xfff;      // Teiler für SCL = 100kHz
	I2C1->TRISE &= ~0x3f;               // SCL Anstiegszeit 1µs
	I2C1->TRISE |= (freq+1) & 0x3f;
	
	I2C1->OAR1 |= (address & 0x7f) << 1; // eigene 7-Bit Adresse programmieren

	I2C1->CR1 |= I2C_CR1_PE;            // Hardware freigeben
}

/* ====================================================================
   I2C Init Routine
   ==================================================================== */
void I2C_init(
	unsigned int clock                  // Taktrate
	,unsigned char address              // eigene 7-Bit Adresse
)
{
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Alternative Portfunktion mit Takt versorgen
    GPIOB->CRL &= ~0xff000000;          // SCL: Pin6 SDA: Pin7
    GPIOB->CRL |= 0xee000000;           // open drain output

	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; // I2C1 Hardware Takt einschalten
	I2C_config( clock, address );       // Hardware konfigurieren
}


/* ====================================================================
   I2C Write n Byte From Buffer Routine, Master Mode
   ==================================================================== */
void I2C_write(
    const unsigned char * buf           // Puffer der zu sendenden Byte
    ,unsigned int n                     // Anzahl zu sendender Byte
    ,unsigned char dest                 // Zieladresse
)
{
    if (n == 0)                         // n = 0 -> ignorieren
        return;

    while (I2C1->SR2 & I2C_SR2_BUSY)		// warten, bis Bus frei
			;
		
		I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START; // Startbedingung setzen und Acknowledge zulassen
    while (!(I2C1->SR1 & I2C_SR1_SB));  // warten auf Startbedingung OK (I2C_SR1_TIMEOUT)
	
    // Adresse senden, mit Schreibanforderung (LSB = 0)
    I2C1->DR = (uint8_t) (dest << 1);   // Zieladresse für Senden
    while (!(I2C1->SR1 & I2C_SR1_ADDR)); // warten auf Acknowledge OK (I2C_SR1_TIMEOUT)
    while (!(I2C1->SR2 & I2C_SR2_TRA)); // warten auf Transmit Mode (I2C_SR1_TIMEOUT)
	
    // Datenbytes senden, ACK vom Slave notwendig
    for (; n != 0; n--)
    {
        while (!(I2C1->SR1 & I2C_SR1_TXE)); // warten auf freien Transmitter (I2C_SR1_TIMEOUT)
        I2C1->DR = *buf++;              // Byte senden
    }

    while (!(I2C1->SR1 & I2C_SR1_BTF)); // warten auf freien Bus (I2C_SR1_TIMEOUT)
    I2C1->CR1 |= I2C_CR1_STOP;          // wenn fertig: Stopbedingung setzen
}


/* ====================================================================
   I2C Read Byte Routine, Master Mode
   ==================================================================== */
void I2C_read(
	unsigned char * buf                 // Puffer für eingelesene Byte
	,unsigned int n                     // Anzahl zu lesender Byte
	,unsigned char source               // Adresse der Datenquelle 
)
{
    volatile unsigned int dmy;
    
    if (n == 0)                         // n = 0 -> ignorieren
        return;

    while (I2C1->SR2 & I2C_SR2_BUSY)		// warten, bis Bus frei
			;

    I2C1->CR1 |= I2C_CR1_ACK | I2C_CR1_START; // Startbedingung setzen und Acknowledge zulassen
    while (!(I2C1->SR1 & I2C_SR1_SB));  // warten auf Startbedingung OK (I2C_SR1_TIMEOUT)
	
	// Adresse senden, mit Leseanforderung (LSB = 1)
    I2C1->DR = (uint8_t)(source << 1 | 1);  // Zieladresse für Lesen
    while (!(I2C1->SR1 & I2C_SR1_ADDR));    // warten auf Acknowledge OK (I2C_SR1_TIMEOUT)

    if (n == 1)
    {
        // Einzelbyte lesen (ACK abschalten, bevor Empfangsbyte einläuft)
        DI
        I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
        dmy = I2C1->SR2;                // Adressbestätigung löschen
        I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
        EI
        while (I2C1->SR2 & I2C_SR2_TRA); // warten auf Receive Mode (I2C_SR1_TIMEOUT)
    }
    else if (n == 2)
    {
        // Doppelbyte lesen (ACK für 2. Byte abschalten: DR = Byte0, Schiebereg. = Byte1)
        DI
        I2C1->CR1 |= I2C_CR1_POS;       // verschobenes Acknowledge zulassen
        dmy = I2C1->SR2;                // Adressbestätigung löschen
        I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
        while (!(I2C1->SR1 & I2C_SR1_BTF)); // warten auf Daten im Schiebereg. (I2C_SR1_TIMEOUT)
        I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
        EI
        while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
        *buf++ = I2C1->DR;           // erstes Byte abholen
    }
    else
    {
        // Multibyte lesen
        while (I2C1->SR2 & I2C_SR2_TRA); // warten auf Receive Mode (I2C_SR1_TIMEOUT)

        // n-1 Datenbytes lesen, ACK nach jedem Byte
        while (n > 1)                   // nur (n-1) Byte in der Schleife
        {
            while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
            *buf++ = I2C1->DR;       // Byte abholen
            n--;
        }
        // letztes Datenbyte ohne ACK (Interrupts bei der Konfiguration verhindern)
        DI
        I2C1->CR1 &= ~I2C_CR1_ACK;      // Acknowledge für letztes Byte abschalten
        I2C1->CR1 |= I2C_CR1_STOP;      // Stopbedingung einstellen
        EI
    }

    while (!(I2C1->SR1 & I2C_SR1_RXNE)); // warten auf Daten im Receiver (I2C_SR1_TIMEOUT)
    *buf++ = I2C1->DR;                  // letztes Byte abholen
}
