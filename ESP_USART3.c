#include "USART1.h"
#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------
// Lua -Programm 
// Ersetzungen: " 		--> \"
//              \ 		--> \\
//							\"		--> \\\"
//							\r\n	-->	\\r\\n

static const char lua_txt[] ={"\
w([[]]);\
w([[print(\"> ok\")]]);\
w([[s,r,g,b=50,50,50,50]]);\
w([[if srv then srv:close() srv=nil end]]);\
w([[srv=net.createServer(net.TCP) ]]);\
w([[srv:listen(80,function(conn) ]]);\
w([[    conn:on(\"receive\", function(client,request)]]);\
w([[        local buf = \"HTTP/1.1 200 OK\\r\\nContent-type: text/html\\r\\nConnection: close\\r\\n\\r\\n\";        ]]);\
w([[        --print(request)]]);\
w([[        local _, _, method, path, vars = string.find(request, \"([A-Z]+) (.+)?(.+) HTTP\");]]);\
w([[        if(method == nil)then ]]);\
w([[            _, _, method, path = string.find(request, \"([A-Z]+) (.+) HTTP\"); ]]);\
w([[        end]]);\
w([[        local _GET = {}]]);\
w([[        ]]);\
w([[        -- extract parameters out of query string]]);\
w([[        if (vars ~= nil)then             ]]);\
w([[            for k, v in string.gmatch(vars, \"(%w+)=([%w\\.]+)&*\") do ]]);\
w([[               _GET[k] = v    ]]);\
w([[            end         ]]);\
w([[        end]]);\
w([[        ]]);\
w([[        --react to HTTP-Request (TX)          ]]);\
w([[        if (_GET.cmd ~= nil) then]]);\
w([[            print(_GET.cmd)]]);\
w([[        elseif (_GET.s ~= nil) then]]);\
w([[            print(\"s\".._GET.s..\".\")]]);\
w([[            s=_GET.s]]);\
w([[        elseif ((_GET.r ~= nil) and (_GET.g ~= nil) and (_GET.b ~= nil)) then]]);\
w([[            print(\"r\".._GET.r..\"g\".._GET.g..\"b\".._GET.b..\".\")]]);\
w([[            r,g,b=_GET.r,_GET.g,_GET.b]]);\
w([[        end   ]]);\
w([[        ]]);\
w([[        ]]);\
w([[        -- html source code]]);\
w([[        buf = buf..\"<h1> MCT Steuerung</h1>\";]]);\
w([[        ]]);\
w([[        -- input text]]);\
w([[        buf = buf..\"<form name=\\\"input\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">Kommando: <input type=\\\"text\\\" name=\\\"cmd\\\" /></span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\" value=\\\"Absenden\\\" />\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        -- input RGB]]);\
w([[        buf = buf..\"<form oninput=\\\"numerisch.value=auswertung.value\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">\";]]);\
w([[        buf = buf..\"R <input name=\\\"r\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..r..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"G <input name=\\\"g\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..g..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"B <input name=\\\"b\\\" type=\\\"number\\\" min=\\\"0\\\" max=\\\"100\\\" step=\\\"1\\\" value=\\\"\"..b..\"\\\" style=\\\"width: 3.5em;\\\"> \";]]);\
w([[        buf = buf..\"</span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\" value=\\\"Absenden\\\">\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        -- input slider]]);\
w([[        buf = buf..\"<form oninput=\\\"x.value=s.value\\\">\";]]);\
w([[        buf = buf..\"<span style=\\\"width: 30%; display: inline-block;\\\">Servo: <input type=\\\"range\\\" name=\\\"s\\\"  min=\\\"0\\\" max=\\\"100\\\" value=\\\"\"..s..\"\\\"><output name=\\\"x\\\">50</output>\";]]);\
w([[        buf = buf..\"</span>\";]]);\
w([[        buf = buf..\"<input type=\\\"submit\\\"value=\\\"Absenden\\\">\";]]);\
w([[        buf = buf..\"</form>\";]]);\
w([[        ]]);\
w([[        client:send(buf);]]);\
w([[        client:close();]]);\
w([[        collectgarbage();]]);\
w([[    end)]]);\
w([[end)]]);\
"};


static const char lua_cmd1[] ={"wifi.setmode(wifi.SOFTAP)\r\n"};
static const char lua_cmd2[] ={"wifi.ap.config({ssid=\"ESP-WLAN%i\",pwd=\"%s\"})\r\n"};
static const char lua_cmd3[] ={"=file.open(\"V5_neu.lua\",\"r\")\r\n"};
static const char lua_cmd4[] ={"=file.open(\"V5_neu.lua\",\"w+\")\r\n"};
static const char lua_cmd5[] ={"w = file.writeline\r\n"};
static const char lua_cmd6[] ={"=file.close()\r\n"};
static const char lua_cmd7[] ={"=dofile(\"V5_neu.lua\")\r\n"};
static const char lua_cmd8[] ={"=file.remove(\"V5_neu.lua\")\r\n"};

//===========================================================================
// USART3 zur Kommunikation mit ESP
void Init_NVICUSART3(void)
// NVIC für USART3 einschalten
{
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=USART3_IRQn;					//USART3 Interrupt im NVIC Aktivieren:
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=4;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}
 
void InitUSART3(void)
// USART3 initialisieren
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;									//Strukturen anlegen:
	
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_AFIOEN,ENABLE);			//AFIO im RCC anschalten:
	GPIO_PinRemapConfig(GPIO_PartialRemap_USART3,ENABLE);		//Pin remapping für USART3

	RCC_APB2PeriphClockCmd(RCC_APB2ENR_IOPCEN,ENABLE);			//Port C im RCC anschalten:
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_10;								//Pin 10 als Alternate Function Output festlegen:
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);										//Port initialisieren:
	
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_11;								//Pin 11 als Eingang festlegen:
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOC,&GPIO_InitStructure);										//Port initialisieren:
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);		//USART3 im RCC anschalten:

	USART_InitStructure.USART_BaudRate=9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USART3,&USART_InitStructure);
	
	USART_Cmd(USART3,ENABLE);																//USART1 anschalten:
	USART_ITConfig(USART3,USART_IT_RXNE,ENABLE);						//USART1 RXNE-Interrupt einschalten:
	Init_NVICUSART3();																			// NVIC aktivieren
}

void WriteCharUSART3(char c){
	while(!USART_GetFlagStatus(USART3,USART_FLAG_TXE));			//Auf TXE Bit warten:
	USART_SendData(USART3,c);																//Daten ins Data-Register schreiben:
}

void WriteStringUSART3(char *str)
{
	while (*str != 0)
	{
		WriteCharUSART3(*(str++));
	}
}

// Interruptroutine für USART3
// optimiert für ESP
void USART3_IRQHandler (void)
{
	char inputChar;								// Eingelesenes Zeichen
	
	inputChar = USART3->DR;								// Empfangenes Zeichen holen 
	inputBuffer[bufferPos] = inputChar;			// und in Puffer speichern
	bufferPos++;													// Pufferzeiger erhöhen
	WriteChar(inputChar);						// Echo an Terminal schicken

	if (inputChar == '\n') 								// wenn Zeilenenderückmeldung von ESP
	{
		inputBuffer[bufferPos] = 0; 			    // String abschließen
		bufferPos = 0;											// Pufferzeiger zurücksetzen
		cmdflag=1;													// Kommandoflag setzen
		WriteString("\n\r");					// neue Zeile beginnen
	}
}
//===========================================================================

void wait(int ms){
	int i;
	for (i=0;i< 10000 *ms; i++)
	{};
}

/*Der ESP schickt immer ein Echo zurueck, gefolgt von CR+LF
*in der naechsten Zeile schickt er die Zeichen "> ",
*dadurch wird signalisiert, dass die naechste Eingabe
*erfolgen kann. Auf diesen Zustand wird mit diesem
*Programm gewartet.
*Das ganze hat den Nachteil, dass der USART-Interrupt
*die Eingabe bei LF als beendet ansieht und wieder an
*den Anfang des Puffers springt, der ESP schickt dann
*aber noch die Zeichen "> " hinterher und macht damit
*die ersten zwei Zeichen im Puffer platt.*/
void wait_for_esp(char *input,int *cmdflag){
	while(1){
		//Pruefen, ob die Zeichen "> " im Puffer stehen
		if(*cmdflag) {
			if (input[0]=='>') {
				if(input[1]==' '){
					//Flag loeschen
					*cmdflag=0;
					//Kurz warten
					wait(100);
					return ;

				}
			}
		}
	}
}

void Lua_to_ESP(char *lua,char *input,int *cmdflag){
	char c;
	
	while (*lua != 0)
	{
		c = *lua;
		//nicht mehr noetig, Zeichen schon im Text ersetzt
		//if (c == '§') c = '"';
		//if (c == '$') c = '\\'; // ¬ in Backslash
		WriteCharUSART3(c);
		
		//Saubere Variante, dauert aber ewig
		//while(ReadCharUSART1()!=c);
		
		//_DEUTLICH_ kuerzere Hochladezeit
		wait(2);
		if ((c == ';')
			   && (*(lua-1)==')')
			   && (*(lua-2)==']')
		     && (*(lua-3)==']'))
		{
		  WriteStringUSART3("\r\n");
			//warten auf ESP, ginge auch mit wait(40),
			//ist so aber schoener
			wait_for_esp(input,cmdflag);
		}
		lua++;		
	}
	
}

void Program_to_ESP( int nr,char *input,int *cmdflag){ 
	char cmd[100];
	
	InitUSART3();		// USART3 fuer ESP-Verbindung 

	//USART1 RXNE-Interrupt abschalten, damit während ESP-Programmierung keine Eingaben ueber putty moeglich
	USART_ITConfig(USART1,USART_IT_RXNE,DISABLE);		

	WriteString("*** Start ESP-Programmierung\n\r");
	wait(100);
	
	//Sekunde warten, bis der ESP gestartet ist
	wait(1000);
	//Flag zurücksetzen
	*cmdflag=0;
	
	//WLAN-Modus festlegen
	//=wifi.setmode(wifi.STATIONAP)\r\n"
	WriteStringUSART3((char*)lua_cmd1);
	//Warten auf Rueckmeldung
	wait_for_esp(input,cmdflag);
	
	//WLAN-Namen und Passwort festlegen
	//=wifi.ap.config({ssid=\"ESP-WLAN%i\",pwd=\"%s\"})\r\n
	sprintf(cmd,(char*)lua_cmd2,nr,"1234567890");
	WriteStringUSART3(cmd);
	//Warten auf Rueckmeldung
	wait_for_esp(input,cmdflag);
	
	//Testen, ob Skript vorhanden ist, falls ja wird vom
	//ESP "true" zurueckgegeben, falls nein "false"
	//=file.open(\"V5_neu.lua\",\"r\")\r\n
	WriteStringUSART3((char*)lua_cmd3);
	//Warten auf Rueckmeldung
	wait_for_esp(input,cmdflag);
	
	//Testen, ob das dritte Zeichen im Puffer kein 'u' ist,
	//siehe Kommentar bei wait_for_ESP(),
	//falls zutreffend -> Skript hochladen
	if(input[2]!='u'){
		//Datei neu anlegen
		//=file.open(\"V5_neu.lua\",\"w+\")\r\n
		WriteStringUSART3((char*)lua_cmd4);
		wait_for_esp(input,cmdflag);
		
		//w = file.writeline\r\n
		WriteStringUSART3((char*)lua_cmd5);
		wait_for_esp(input,cmdflag);
		
		Lua_to_ESP((char*)lua_txt,input,cmdflag);
		//Warten schon integriert
	}
	//Datei wieder schliessen
	//=file.close()\r\n
	WriteStringUSART3((char*)lua_cmd6);
	wait_for_esp(input,cmdflag);
	
	//Skript ausfuehren
	//=dofile(\"V5_neu.lua\")\r\n
	WriteStringUSART3((char*)lua_cmd7);
	wait_for_esp(input,cmdflag);
	
	wait(10);

	if(!(input[2]=='o'&&input[3]=='k')){
		//Alte Datei loeschen
		//=file.remove(\"V5_neu.lua\")\r\n
		WriteString((char*)lua_cmd8);
		wait_for_esp(input,cmdflag);
		
		//Datei neu anlegen
		//=file.open(\"V5_neu.lua\",\"w+\")\r\n
		WriteString((char*)lua_cmd4);
		wait_for_esp(input,cmdflag);
		
		//w = file.writeline\r\n
		WriteString((char*)lua_cmd5);
		wait_for_esp(input,cmdflag);
		
		Lua_to_ESP((char*)lua_txt,input,cmdflag);
		//Warten schon integriert
		
		//Skript ausfuehren
		//=dofile(\"V5_neu.lua\")\r\n
		WriteString((char*)lua_cmd7);
		wait_for_esp(input,cmdflag);
	}
	
	bufferPos=0;		// Puffer zurücksetzen, um das Kommandoprompt "> " aus Puffer zu entfernen
	
	//USART1 RXNE-Interrupt wieder zulassen
	USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);		

	WriteString("*** Ende ESP-Programmierung\n\r");
	
}

