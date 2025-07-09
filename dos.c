/*
 * DOS System-dependent routines.
 *
 * System-specific code for MS-DOS. This has been tested with
 * MSDOS 3.3 on an AT. Also, the console driver "nansi.sys" is
 * required.
 *
 */

#include <dos.h>
#include "stevie.h"

/*
 * inchar() - get a character from the keyboard
 */
int
inchar()
{
    int             c;

    for (;; beep()) {		/* loop until we get a valid character */

	flushbuf();		/* flush any pending output */

	switch (c = getch()) {
	  case 0x1e:
	    return K_CGRAVE;
	  case 0:		/* special key */
	    if (State != NORMAL) {
		c = getch();	/* throw away next char */
		continue;	/* and loop for another char */
	    }
	    switch (c = getch()) {
	      case 0x50:
		return K_DARROW;
	      case 0x48:
		return K_UARROW;
	      case 0x4b:
		return K_LARROW;
	      case 0x4d:
		return K_RARROW;
	      case 0x52:
		return K_INSERT;
	      case 0x47:
		stuffReadbuff("1G");
		return -1;
	      case 0x4f:
		stuffReadbuff("G");
		return -1;
	      case 0x51:
		stuffReadbuff(mkstr(CTRL('F')));
		return -1;
	      case 0x49:
		stuffReadbuff(mkstr(CTRL('B')));
		return -1;
		/*
		 * Hard-code some useful function key macros. 
		 */
	      case 0x3b:	/* F1 */
		stuffReadbuff(":p\n");
		return -1;
	      case 0x54:	/* SF1 */
		stuffReadbuff(":p!\n");
		return -1;
	      case 0x3c:	/* F2 */
		stuffReadbuff(":n\n");
		return -1;
	      case 0x55:	/* SF2 */
		stuffReadbuff(":n!\n");
		return -1;
	      case 0x3d:	/* F3 */
		stuffReadbuff(":e #\n");
		return -1;
	      case 0x3e:	/* F4 */
		stuffReadbuff(":rew\n");
		return -1;
	      case 0x57:	/* SF4 */
		stuffReadbuff(":rew!\n");
		return -1;
	      case 0x3f:	/* F5 */
		stuffReadbuff("[[");
		return -1;
	      case 0x40:	/* F6 */
		stuffReadbuff("]]");
		return -1;
	      case 0x41:	/* F7 */
		stuffReadbuff("<<");
		return -1;
	      case 0x42:	/* F8 */
		stuffReadbuff(">>");
		return -1;
	      case 0x43:	/* F9 */
		stuffReadbuff(":x\n");
		return -1;
	      case 0x44:	/* F10 */
		stuffReadbuff(":help\n");
		return -1;
	      default:
		break;
	    }
	    break;

	  default:
	    return c;
	}
    }
}

#define BSIZE   2048
static char     outbuf[BSIZE];
static int      bpos = 0;

flushbuf()
{
    if (bpos != 0)
	write(1, outbuf, bpos);
    bpos = 0;
}

/*
 * Macro to output a character. Used within this file for speed.
 */
#define outone(c)       outbuf[bpos++] = c; if (bpos >= BSIZE) flushbuf()

/*
 * Function version for use outside this file.
 */
void
outchar(c)
    register char   c;
{
    outbuf[bpos++] = c;
    if (bpos >= BSIZE)
	flushbuf();
}


/*
 * outstr(s) - write a string to the console
 */
void
outstr(s)
    register char  *s;
{
    if (strcmp(s, T_DL) == 0) {	/* delete line */
	union REGS      rr;

	flushbuf();
	rr.x.ax = 0x0300;	/* get cursor position */
	rr.x.bx = 0;
	int86(0x10, &rr, &rr);
	rr.h.ch = rr.h.dh;	/* scroll from current row to bot dn 1 */
	rr.h.cl = rr.h.dl;
	rr.h.dh = Rows - rr.h.ch;
	rr.h.dl = Columns;
	rr.x.bx = 07;
	rr.x.ax = 0x0601;
	int86(0x10, &rr, &rr);
	return;
    }
    if (strcmp(s, T_IL) == 0) {	/* insert line */
	union REGS      rr;

	flushbuf();
	rr.x.ax = 0x0300;	/* get cursor position */
	rr.x.bx = 0;
	int86(0x10, &rr, &rr);
	rr.h.ch = rr.h.dh;	/* scroll from current row to bot dn 1 */
	rr.h.cl = rr.h.dl;
	rr.h.dh = Rows - rr.h.ch;
	rr.h.dl = Columns;
	rr.x.bx = 07;
	rr.x.ax = 0x0701;
	int86(0x10, &rr, &rr);
	return;
    }
    while (*s) {
	outone(*s++);
    }
}

void
beep()
{
    if (RedrawingDisabled)
	return;

    outone('\007');
}

void
sleep(n)
    int             n;
{
    /*
     * Should do something reasonable here. 
     */
}

void
delay()
{
    long            l;

    flushbuf();
    /*
     * Should do something better here... 
     */
    for (l = 0; l < 5000; l++);
}

void
windinit()
{
    Columns = 80;
    P(P_LI) = Rows = 25;
}

void
windexit(r)
    int             r;
{
    flushbuf();
    exit(r);
}

void
windgoto(r, c)
    register int    r, c;
{
    union REGS      rr;

    flushbuf();
    rr.h.dh = r;
    rr.h.dl = c;
    rr.x.bx = 0;
    rr.x.ax = 0x0200;
    int86(0x10, &rr, &rr);
}

FILE           *
fopenb(fname, mode)
    char           *fname;
    char           *mode;
{
    FILE           *fopen();
    char            modestr[16];

    sprintf(modestr, "%sb", mode);
    return fopen(fname, modestr);
}
