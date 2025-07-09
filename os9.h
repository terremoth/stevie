/*
 * OS9 v2.2
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
void rename();
void windinit();
void windexit();
void windgoto();
void delay();
#define remove(path) unlink(path)
#ifndef strchr
# define strchr index
#endif
