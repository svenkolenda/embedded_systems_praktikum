// 
// Lädt ein  Webserverprogramm in den ESP8266 
// und startet den Webserver
// nr soll die Nummer des Arbeitsplatzes sein 
// SSID des WLAN : ESP-WLAN<nr>
// Passwort: 1234567890
// Web-Browser aufrufen mit 192.168.4.1

//inputbuffer und cmdflag muessen mit uebergeben werden

//Funktion, mit der der ESP programmiert wird
	//	erster Parameter: WLAN-Nummer (ESP-WLAN##)
	//	zweiter Parameter: USART Eingabepuffer
	//	dritter Parameter: Flag fuer Ende der Eingabe

void Program_to_ESP(int nr,char *input,int *cmdflag);

