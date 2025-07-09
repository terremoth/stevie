/*
 * System-dependent routines for BSD 4.3 UNIX 
 */

#include "stevie.h"
#include <sgtty.h>

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

static struct sgttyb ostate;
static struct sgttyb nstate;

void
set_ostate()
{
    ioctl(0, (long) TIOCSETP, (char *) &ostate);
}

void
set_nstate()
{
    ioctl(1, (long) TIOCSETP, (char *) &nstate);
}

void
windinit()
{
    char           *getenv();
#ifdef CHECK_TERM
    char           *term;

    term = getenv("TERM");
    if (!term) {
	fprintf(stderr, "Invalid terminal type '%s'\n", term);
	exit(1);
    }
    if ((strncmp(term, "vt", 2) != 0) && (strncmp(term, "kd", 2) != 0)) {
	fprintf(stderr, "Invalid terminal type '%s'\n", term);
	exit(1);
    }
#endif

    Columns = 80;
    P(P_LI) = Rows = 24;

    /*
     * Go into cbreak mode 
     */
    ioctl(1, (long) TIOCGETP, (char *) &ostate);
    nstate = ostate;
    nstate.sg_flags = nstate.sg_flags & ~(ECHO | CRMOD) | CBREAK;
    ioctl(1, (long) TIOCSETP, (char *) &nstate);
}

void
windexit(r)
    int             r;
{
    flushbuf();

    ioctl(0, (long) TIOCSETP, (char *) &ostate);

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
