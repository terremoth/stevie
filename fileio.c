/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

#include "stevie.h"
#include <stdio.h>

FILE *fopenb(const char *filename, const char *mode);

void filemess(char *s) {
    sprintf(IObuff, "\"%s\" %s", ((Filename == NULL) ? "" : Filename), s);
    msg(IObuff);
}

void renum() {
    LPtr *p;
    unsigned long   l = 0;

    for (p = Filemem; p != NULL; p = nextline(p), l += LINEINC)
	p->linep->num = l;

    Fileend->linep->num = 0xffffffffL;
}

#ifdef  MEGAMAX
overlay "fileio"
#endif

bool_t readfile(const char *fname, LPtr *fromp, bool_t nochangename/* if TRUE, don't change the Filename */) {
    FILE           *f;
    LINE           *curr;
    char            buf2[80];
    int             c;
    int             IObuffsize = 0;
    long            nchars = 0;
    int             linecnt = 0;
    bool_t          wasempty = bufempty();
    int             nonascii = 0;	/* count garbage characters */
    int             nulls = 0;	/* count nulls */
    bool_t          incomplete = FALSE;	/* was the last line incomplete? */
    bool_t          toolong = FALSE;	/* a line was too long */

    curr = fromp->linep;

    if (!nochangename)
	Filename = strsave(fname);

    f = fopen(fname, "r");
    if (f == NULL) {
	s_refresh(NOT_VALID);
	filemess("");
	return TRUE;
    }
    S_NOT_VALID;

    do {
	c = getc(f);

	if (c == EOF) {
	    if (IObuffsize == 0)/* normal loop termination */
		break;

	    /*
	     * If we get EOF in the middle of a line, note the fact and
	     * complete the line ourselves. 
	     */
	    incomplete = TRUE;
	    c = NL;
	}
	if (c >= 0x80) {
	    c -= 0x80;
	    nonascii++;
	}
	/*
	 * If we reached the end of the line, OR we ran out of space for it,
	 * then process the complete line. 
	 */
	if (c == NL || IObuffsize == (IOSIZE - 1)) {
	    LINE           *lp;

	    if (c != NL)
		toolong = TRUE;

	    IObuff[IObuffsize++] = NUL;
	    lp = newline(IObuffsize);
	    if (lp == NULL) {
		fprintf(stderr, "not enough memory - should never happen");
		getout(1);
	    }
	    strcpy(lp->s, IObuff);

	    curr->next->prev = lp;	/* new line to next one */
	    lp->next = curr->next;

	    curr->next = lp;	/* new line to prior one */
	    lp->prev = curr;

	    curr = lp;		/* new line becomes current */
	    IObuffsize = 0;
	    linecnt++;
	} else if (c == NUL) {
	    nulls++;		/* count and ignore nulls */
	} else {
	    IObuff[IObuffsize++] = (char) c;	/* normal character */
	}

	nchars++;
    } while (!incomplete && !toolong);

    fclose(f);

    /*
     * If the buffer was empty when we started, we have to go back and remove
     * the "dummy" line at Filemem and patch up the ptrs. 
     */
    if (wasempty && linecnt != 0) {
	LINE           *dummy = Filemem->linep;	/* dummy line ptr */

	Filemem->linep = Filemem->linep->next;
	Filemem->linep->prev = Filetop->linep;
	Filetop->linep->next = Filemem->linep;

	Curschar->linep = Filemem->linep;
	Topchar->linep = Filemem->linep;

	free(dummy->s);		/* free string space */
	free((char *) dummy);	/* free LINE struct */
    }
    renum();

    if (toolong) {
	s_refresh(NOT_VALID);

	sprintf(IObuff, "\"%s\" Line too long", fname);
	msg(IObuff);
	return FALSE;
    }
    s_refresh(NOT_VALID);

    sprintf(IObuff, "\"%s\" %s%d line%s, %ld character%s",
	    fname,
	    incomplete ? "[Incomplete last line] " : "",
	    linecnt, (linecnt > 1) ? "s" : "",
	    nchars, (nchars > 1) ? "s" : "");

    buf2[0] = NUL;

    if (nonascii || nulls) {
	if (nonascii) {
	    if (nulls)
		sprintf(buf2, " (%d null, %d non-ASCII)",
			nulls, nonascii);
	    else
		sprintf(buf2, " (%d non-ASCII)", nonascii);
	} else
	    sprintf(buf2, " (%d null)", nulls);
    }
    strcat(IObuff, buf2);
    msg(IObuff);

    return FALSE;
}

/*
 * writeit - write to file 'fname' lines 'start' through 'end' 
 *
 * If either 'start' or 'end' contain null line pointers, the default is to use
 * the start or end of the file respectively. 
 */
bool_t writeit(char *fname, LPtr *start, LPtr *end) {
    FILE           *f;
    char           *s;
    long            nchars;
    int             lines;
    LPtr           *p;

    sprintf(IObuff, "\"%s\"", fname);
    msg(IObuff);

    /*
     * Form the backup file name - change foo.* to foo.bak - use IObuff to
     * hold the backup file name 
     */
    strcpy(IObuff, fname);
    for (s = IObuff; *s && *s != '.'; s++);
    *s = NUL;
    strcat(IObuff, ".bak");

    /*
     * Delete any existing backup and move the current version to the backup.
     * For safety, we don't remove the backup until the write has finished
     * successfully. And if the 'backup' option is set, leave it around. 
     */
    rename(fname, IObuff);

    f = P(P_CR) ? fopen(fname, "w") : fopenb(fname, "w");
    if (f == NULL) {
	emsg("Can't open file for writing!");
	return FALSE;
    }
    /*
     * If we were given a bound, start there. Otherwise just start at the
     * beginning of the file. 
     */
    if (start == NULL || start->linep == NULL)
	p = Filemem;
    else
	p = start;

    lines = 0;
    nchars = 0;
    do {
	fprintf(f, "%s\n", p->linep->s);
	nchars += strlen(p->linep->s) + 1;
	lines++;

	/*
	 * If we were given an upper bound, and we just did that line, then
	 * bag it now. 
	 */
	if (end != NULL && end->linep != NULL) {
	    if (end->linep == p->linep)
		break;
	}
    } while ((p = nextline(p)) != NULL);

    fclose(f);

    /*
     * Remove the backup unless they want it left around 
     */
    if (!P(P_BK))
	remove(IObuff);

    sprintf(IObuff, "\"%s\" %d line%s, %ld character%s", fname,
	    lines, (lines > 1) ? "s" : "",
	    nchars, (nchars > 1) ? "s" : "");
    msg(IObuff);
    UNCHANGED;

    return TRUE;
}
