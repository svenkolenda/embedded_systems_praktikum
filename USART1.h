void InitUSART1(void);
void WriteChar(char c);
void WriteString(char *str);
char ReadChar(void);

extern int cmdflag;
extern char inputBuffer[];
extern int bufferPos;
