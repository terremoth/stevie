/*
 * System-dependent routines for OS9 v2.2
 */

#include "stevie.h"
#include <sgstat.h>

#ifdef OLD_IO
#define BSIZE   2048
static char     outbuf[BSIZE];
static int      bpos = 0;

void
flushbuf()
{
    if (bpos != 0)
	fwrite(outbuf, sizeof(*outbuf), bpos, stdout);
    fflush(stdout);
    bpos = 0;
}

void
outchar(c)
    char            c;
{
    outbuf[bpos++] = c;
    if (bpos >= BSIZE)
	flushbuf();
}

void
outstr(s)
    char           *s;
{
    while (*s) {
	outbuf[bpos++] = *s++;
	if (bpos >= BSIZE)
	    flushbuf();
    }
}
#endif

/*
 * inchar() - get a character from the keyboard 
 */
int
inchar()
{
    int             c;

    flushbuf();			/* flush any pending output */

    c = getchar();

    return c;
}

void
beep()
{
    if (RedrawingDisabled)
	return;

    outchar('\007');
}

void
delay()
{
    sleep(1);
}

static struct sgbuf ostate;
static struct sgbuf nstate;

void
windinit()
{
    char           *getenv();

    Columns = 80;
    P(P_LI) = Rows = 24;

    /*
     * Go into cbreak mode 
     */
	_gs_opt(1, &ostate);
	_gs_opt(1, &nstate);
	nstate.sg_echo = 0;   /* no echo */
/*	nstate.sg_alf = 0;    /* auto line feed */
	nstate.sg_pause = 0;  /* no end of page pause */
	nstate.sg_dlnch = 0;  /* delete line char */
	nstate.sg_eorch = 0;  /* end of record char */
	nstate.sg_eofch = 0;  /* end of file char */
	nstate.sg_rlnch = 0;  /* reprint line char */
	nstate.sg_dulnch = 0; /* duplicate last line char */
	nstate.sg_psch = 0;   /* pause char */
	nstate.sg_kbich = 0;  /* interrupt char */
	nstate.sg_kbach = 0;  /* abort char */
	_ss_opt(1, &nstate);

	stdin->_flag |= _UNBUF;

}

void
windexit(r)
    int             r;
{
    flushbuf();

	stdin->_flag &= ~_UNBUF;
	_ss_opt(1, &ostate);

    exit(r);
}

void
windgoto(r, c)
    int             c;
    int             r;
{
    r++;
    c++;

    outstr("\033[");
    if (r >= 10)
	outchar((char) (r / 10 + '0'));
    outchar((char) (r % 10 + '0'));
    outchar(';');
    if (c >= 10)
	outchar((char) (c / 10 + '0'));
    outchar((char) (c % 10 + '0'));
    outchar('H');
}

FILE           *
fopenb(fname, mode)
    char           *fname;
    char           *mode;
{
    return fopen(fname, mode);
}

void rename(of,nf)
char *of, *nf;
{
char buffer[128];

	sprintf(buffer, "rename >>>/nil %s %s", of, nf);
	system (buffer);
}

