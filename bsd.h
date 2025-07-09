/*
 * BSD 4.3 Machine-dependent routines. 
 */

void set_ostate();
void set_nstate();

int  inchar();

#ifdef OLD_IO
void flushbuf();
void outchar();
void outstr();
#else
# define flushbuf() fflush(stdout)
# define outchar(C) putchar(C)
# define outstr(S)  fputs((S), stdout)
#endif

void beep();
#define remove(path) unlink(path)
int rename();
void windinit();
void windexit();
void windgoto();
void delay();
