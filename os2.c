/*
 * OS/2 System-dependent routines.
 *
 * $Log:        os2.c,v $
 * Revision 1.2  88/04/25  16:50:19  tony
 * Minor changes for OS/2 version 1.1; also fixed up the RCS header.
 * 
 * Revision 1.1  88/03/21  12:04:23  tony
 * Initial revision
 * 
 *
 */

#define INCL_BASE
#include <os2.h>
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
	  case 0xe0:		/* special key */
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

static char     cell[2] = {0, 7};

/*
 * outstr(s) - write a string to the console
 *
 * We implement insert/delete line escape sequences here. This is kind
 * of a kludge, but at least it's localized to a single point.
 */
void
outstr(s)
    register char  *s;
{
    if (strcmp(s, T_DL) == 0) {	/* delete line */
	int             r, c;

	flushbuf();
	VioGetCurPos(&r, &c, 0);
	VioScrollUp(r, 0, 100, 100, 1, cell, 0);
	return;
    }
    if (strcmp(s, T_IL) == 0) {	/* insert line */
	int             r, c;

	flushbuf();
	VioGetCurPos(&r, &c, 0);
	VioScrollDn(r, 0, 100, 100, 1, cell, 0);
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
    DosSleep(1000L * n);
}

void
delay()
{
    flushbuf();
    DosSleep(300L);
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
    flushbuf();
    VioSetCurPos(r, c, 0);
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
