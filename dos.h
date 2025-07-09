/*
 * MS DOS Machine-dependent routines. 
 */

void flushbuf();

int  inchar();
void outchar(register char);
void outstr(register char *);
void beep();
void windinit();
void windexit(int r);
void windgoto(register int, register int);
void delay();
void sleep(int);
