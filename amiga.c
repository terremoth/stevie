/*
 * Amiga system-dependent routines. 
 */

#include <proto/exec.h>
#include <proto/dos.h>
#include <exec/memory.h>
#include <devices/conunit.h>
#include <stdio.h>
#include <ios1.h>
#include <error.h>

#include "stevie.h"

/* Globals initialized by get_ConUnit() */
struct Window  *conWindow;
struct ConUnit *conUnit;

extern int      errno;		/* The error variable */

long            raw_in = 0;
long            raw_out = 0;

#define BSIZE   2048
static char     outbuf[BSIZE];
static int      bpos = 0;

void
flushbuf()
{
    if (bpos != 0)
	Write(raw_out, outbuf, bpos);
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

int
get_Rows_and_Columns()
{
    if (get_ConUnit(raw_in) != 0)
	return (FALSE);
    Rows = conUnit->cu_YMax + 1;
    Columns = conUnit->cu_XMax + 1;
    if (Rows < 0 || Rows > 200) {	/* AUX: device (I hope) */
	Columns = 80;
	Rows = 24;
	Aux_Device = TRUE;
    }
    if (Columns < 5)
	Columns = 5;
    if (Columns > MAX_COLUMNS)
	Columns = MAX_COLUMNS;
    if (Rows < 2)
	Rows = 2;
    P(P_LI) = Rows;

    return (TRUE);
}

int
GetCharacter()
{
    char            c;

    Read(raw_in, &c, sizeof(c));
    return ((int) c);
}

/*
 * getCSIsequence - get a CSI sequence
 *                - either cursor keys, help, or function keys
 */

int
getCSIsequence()
{
    int             c;
    int             tmp;


    c = GetCharacter();
    if (isdigit(c)) {
	tmp = 0;
	while (isdigit(c)) {
	    tmp = tmp * 10 + c - '0';
	    c = GetCharacter();
	}
	if (c == '~')		/* function key */
	    return ((char) (K_F1 + tmp));
    }
    switch (c) {
      case 'A':		/* cursor up */
	return K_UARROW;
      case 'B':		/* cursor down */
	return K_DARROW;
      case 'C':		/* cursor right */
	return K_RARROW;
      case 'D':		/* cursor left */
	return K_LARROW;
      case 'T':		/* shift cursor up */
	return K_SUARROW;
      case 'S':		/* shift cursor down */
	return K_SDARROW;
      case ' ':		/* shift cursor left or right */
	c = GetCharacter();
	if (c == 'A')		/* shift cursor left */
	    return K_SLARROW;
	if (c == '@')		/* shift cursor right */
	    return K_SRARROW;
	break;
      case '?':		/* help */
	c = GetCharacter();
	if (c == '~')
	    return K_HELP;
	break;
    }
    while ((c != '|') && (c != '~')) {
	if (WaitForChar(raw_in, 500L) == 0)
	    break;
	c = GetCharacter();
    }

    /* must have been screen resize event */
    s_clear();
    flushbuf();
    if (get_Rows_and_Columns() == FALSE) {	/* hopefully never exit .... */
	emsg("STEVIE: can't get ConUnit info ?!?!?!?\n");
	sleep(5);
	return 0;
    }
    screenalloc();
    tmp = RedrawingDisabled;
    RedrawingDisabled = TRUE;
    S_NOT_VALID;
    cursupdate(UPDATE_ALL);	/* make sure not below Botchar */
    RedrawingDisabled = FALSE;
    s_refresh(NOT_VALID);	/* draw it */
    RedrawingDisabled = tmp;
    windgoto(Cursrow, Curscol);
    flushbuf();

    return 0;
}

/*
 * inchar() - get a character from the keyboard 
 */
int
inchar()
{
    int             c;

    flushbuf();

    for (;;) {
	c = GetCharacter();
	if (c == 0x9b)
	    c = getCSIsequence();
	if (c != 0)
	    break;
    }

    return c;
}

void
beep()
{
    if (RedrawingDisabled)
	return;

    outbuf[bpos++] = '\007';
    if (bpos >= BSIZE)
	flushbuf();
}

void
sleep(n)
    int             n;
{
    void            Delay();

    if (n > 0)
	Delay(50L * n);
}

void
delay()
{
    void            Delay();

    Delay(25L);
}

void
windinit()
{
    raw_in = Input();
    if (!IsInteractive(raw_in)) {
	raw_in = Open("RAW:0/0/640/200/STEVIE", MODE_NEWFILE);
	if (raw_in == NULL) {
	    raw_in = Open("RAW:0/0/480/200/STEVIE", MODE_NEWFILE);
	    if (raw_in == NULL) {
		fprintf(stderr, "STEVIE: Can't open window ?!?!?!?\n");
		exit(2);
	    }
	}
	raw_out = raw_in;
    } else {
	raw_out = Output();
	if (raw(raw_in) != 0) {
	    perror("STEVIE: Can't change to raw mode ?!?!?!?");
	    exit(2);
	}
    }

    if (get_Rows_and_Columns() == FALSE) {
	fprintf(stderr, "STEVIE: can't get ConUnit info ?!?!?!?\n");
	windexit(3);
    }
    if (!Aux_Device)
	outstr("\033[12{");	/* window resize events activated */
    flushbuf();
}

void
windexit(r)
    int             r;
{
    if (!Aux_Device)
	outstr("\033[12}");	/* window resize events de-activated */
    flushbuf();

    if (raw_in != raw_out) {
	if (cooked(raw_in) != 0)
	    perror("STEVIE: Can't change to cooked mode ?!?!?!?");
    } else {
	Close(raw_in);
    }

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
    FILE           *fopen();
    char            modestr[16];

    sprintf(modestr, "%sb", mode);
    return fopen(fname, modestr);
}

/*
 * raw() & cooked()
 *
 * These are routines for setting a given stream to raw or cooked mode on the
 * Amiga. This is useful when you are using Lattice C to produce programs
 * that want to read single characters with the "getch()" or "fgetc" call. 
 *
 * Written : 18-Jun-87 By Chuck McManis.
 */

/*
 * Function raw() - Convert the specified File Handle to 'raw' mode. This
 * only works on TTY's and essentially keeps DOS from translating keys for
 * you.
 */

long
raw(afh)
    struct FileHandle *afh;
{
    struct MsgPort *mp;		/* The File Handle message port */
    long            Arg[1], res;

    mp = ((struct FileHandle *) (BADDR(afh)))->fh_Type;
    Arg[0] = -1L;
    res = send_packet(mp, ACTION_SCREEN_MODE, Arg, 1);
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

/*
 * Function - cooked() this function returns the designate file pointer to
 * it's normal, wait for a <CR> mode. This is exactly like raw() except that
 * it sends a 0 to the console to make it back into a CON: from a RAW: 
 */

long
cooked(afh)
    struct FileHandle *afh;
{
    struct MsgPort *mp;		/* The File Handle message port */
    long            Arg[1], res;

    mp = ((struct FileHandle *) (BADDR(afh)))->fh_Type;
    Arg[0] = 0L;
    res = send_packet(mp, ACTION_SCREEN_MODE, Arg, 1);
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

/*
 * Code for this routine came from the following :
 *
 * ConPackets.c -  C. Scheppner, A. Finkel, P. Lindsay  CBM
 *   DOS packet example
 *   Requires 1.2
 *
 * which I found on Fish Disk 56.
 */

/* initializes conWindow and conUnit (global vars) */
long
get_ConUnit(afh)
    struct FileHandle *afh;
{
    struct MsgPort *mp;		/* The File Handle message port */
    struct InfoData *id;
    long            Arg[8], res;

    if (!IsInteractive((BPTR) afh)) {
	errno = ENOTTY;
	return (-1);
    }
    mp = ((struct FileHandle *) (BADDR(afh)))->fh_Type;

    /* Alloc to insure longword alignment */
    id = (struct InfoData *) AllocMem(sizeof(struct InfoData),
				      MEMF_PUBLIC | MEMF_CLEAR);
    if (!id) {
	errno = ENOMEM;
	return (-1);
    }
    Arg[0] = ((ULONG) id) >> 2;
    res = send_packet(mp, ACTION_DISK_INFO, Arg, 1);
    conWindow = (struct Window *) id->id_VolumeNode;
    conUnit = (struct ConUnit *) ((struct IOStdReq *) id->id_InUse)->io_Unit;
    FreeMem(id, sizeof(struct InfoData));
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

/*
 * send_packet() - written by Phil Lindsay, Carolyn Scheppner, and Andy
 * Finkel. This function will send a packet of the given type to the Message
 * Port supplied. 
 */

long
send_packet(pid, action, args, nargs)
    struct MsgPort *pid;	/* process indentifier ... (handlers message
				 * port ) */
    long            action,	/* packet type ... (what you want handler to
				 * do )   */
                    args[],	/* a pointer to a argument list */
                    nargs;	/* number of arguments in list  */
{
    struct MsgPort *replyport;
    struct StandardPacket *packet;

    long            count, *pargs, res1;

    replyport = (struct MsgPort *) CreatePort(NULL, 0);
    if (!replyport)
	return (0);

    /* Allocate space for a packet, make it public and clear it */
    packet = (struct StandardPacket *)
	AllocMem((long) sizeof(struct StandardPacket),
		 MEMF_PUBLIC | MEMF_CLEAR);
    if (!packet) {
	DeletePort(replyport);
	return (0);
    }
    packet->sp_Msg.mn_Node.ln_Name = (char *) &(packet->sp_Pkt);
    packet->sp_Pkt.dp_Link = &(packet->sp_Msg);
    packet->sp_Pkt.dp_Port = replyport;
    packet->sp_Pkt.dp_Type = action;

    /* copy the args into the packet */
    pargs = &(packet->sp_Pkt.dp_Arg1);	/* address of first argument */
    for (count = 0; count < nargs; count++)
	pargs[count] = args[count];

    PutMsg(pid, packet);	/* send packet */

    WaitPort(replyport);
    GetMsg(replyport);

    res1 = packet->sp_Pkt.dp_Res1;

    FreeMem(packet, (long) sizeof(struct StandardPacket));
    DeletePort(replyport);

    return (res1);
}

#ifdef WILD_CARDS
/*
 * ExpandWildCard() - this code does wild-card pattern matching using the arp
 *                    routines. This is based on WildDemo2.c (found in arp1.1
 *                    distribution). That code's copyright follows :
 *-------------------------------------------------------------------------
 * WildDemo2.c - Search filesystem for patterns, and separate into directories
 *    	 and files, sorting each separately using DA lists.
 *
 * -+=SDB=+-
 *
 * Copyright (c) 1987, Scott Ballantyne
 * Use and abuse as you please.
 *-------------------------------------------------------------------------
 */

#include <libraries/arpbase.h>
#include <arpfunctions.h>

#define ANCHOR_BUF_SIZE (512)
#define ANCHOR_SIZE (sizeof(struct AnchorPath) + ANCHOR_BUF_SIZE)

struct ArpBase *ArpBase;

void
ExpandWildCards(num_pat, pat, num_file, file)
    int             num_pat;
    char          **pat;
    int            *num_file;
    char         ***file;
{
    int             i;

    struct DirectoryEntry *FileList = NULL;
    struct DirectoryEntry *de;
    struct AnchorPath *Anchor;
    LONG            Result;

    *num_file = 0;
    ***file = NULL;

    if (!(ArpBase = (struct ArpBase *) OpenLibrary(ArpName, ArpVersion))) {
	*num_file = num_pat;
	*file = pat;
	return;
    }
    /* Get our AnchorBase */
    Anchor = (struct AnchorPath *) calloc(1, ANCHOR_SIZE);
    if (!Anchor) {
OUT_OF_MEMORY:
	fprintf(stderr, "Out of memory!\n");
	exit(20);
    }
    Anchor->ap_Length = ANCHOR_BUF_SIZE;

    if (num_pat > 0) {
	for (i = 0; i < num_pat; i++) {
	    Result = FindFirst(pat[i], Anchor);
	    while (Result == 0 || Result == ERROR_OBJECT_NOT_FOUND) {
		if (Anchor->ap_Info.fib_DirEntryType < 0) {
		    (*num_file)++;
		    if (!AddDANode(Anchor->ap_Buf, &FileList, 0L, i)) {
			FreeAnchorChain(Anchor);
			FreeDAList(FileList);
			goto OUT_OF_MEMORY;
		    }
		} else if (Result == ERROR_OBJECT_NOT_FOUND) {
		    (*num_file)++;
		    if (!AddDANode(pat[i], &FileList, 0L, i)) {
			FreeAnchorChain(Anchor);
			FreeDAList(FileList);
			goto OUT_OF_MEMORY;
		    }
		}
		Result = FindNext(Anchor);
	    }
	    if (Result == ERROR_BUFFER_OVERFLOW) {
		fprintf(stderr, "ANCHOR_BUF_SIZE too small.\n");
		FreeAnchorChain(Anchor);
		FreeDAList(FileList);
		exit(20);
	    } else if (Result != ERROR_NO_MORE_ENTRIES) {
		fprintf(stderr, "With %s: I/O ERROR #%ld\n", pat[i], Result);
		FreeAnchorChain(Anchor);
		FreeDAList(FileList);
		exit(20);
	    }
	}
	FreeAnchorChain(Anchor);

	de = FileList;
	if (de) {
	    *file = (char **) malloc(sizeof(char *) * (*num_file));
	    if (*file == NULL)
		goto OUT_OF_MEMORY;
	    for (i = 0; de; de = de->de_Next, i++) {
		(*file)[i] = (char *) malloc(strlen(de->de_Name) + 1);
		if ((*file)[i] == NULL)
		    goto OUT_OF_MEMORY;
		strcpy((*file)[i], de->de_Name);
	    }
	}
	FreeDAList(FileList);
    }
    CloseLibrary((struct Library *) ArpBase);
}
#endif
