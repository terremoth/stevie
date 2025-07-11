/*
 * STEVIE - Simply Try this Editor for VI Enthusiasts
 *
 * Code Contributions By : Tim Thompson           twitch!tjt
 *                         Tony Andrews           onecom!wldrdg!tony 
 *                         G. R. (Fred) Walter    watmath!grwalter 
 */

/*
 * The defines in this file establish the environment we're compiling
 * in. Set these appropriately before compiling the editor.
 */

/*
 * One (and only 1) of the following defines should be uncommented. Most of
 * the code is pretty machine-independent. Machine dependent code goes in a
 * file like tos.c or unix.c. The only other place where machine dependent
 * code goes is term.h for escape sequences. 
 */

#ifndef AMIGA
# ifndef BSD
#  ifndef UNIX
/* Defined in makefile : AMIGA	Amiga */
/* Defined in makefile : BSD	BSD 4.3 */
/* Defined in makefile : UNIX	System V */
/* #define	ATARI		Atari ST */
/* #define	OS2		Microsoft OS/2 */
/* #define	DOS		MS DOS 3.3 */
#  endif
# endif
#endif

#ifdef AMIGA
# define WILD_CARDS
#endif

/*
 * If ATARI is defined, one of the following compilers must be selected. 
 */
#ifdef	ATARI
#define MWC			/* Mark William's C 3.0.9 */
/* #define	MEGAMAX		Megamax Compiler */
/* #define	ALCYON		Alcyon C compiler */

# ifdef MWC
#  define AppendNumberToUndoUndobuff 	XX1
#  define AppendPositionToUndoUndobuff	XX2
#  define FOPENB
# endif

# ifdef MEGAMAX
#  define FOPENB
# endif
#endif

/*
 * If HELP is defined, the :help command shows a vi command summary. 
 */
#define	HELP			/* enable help command */

/*
 * STRCSPN should be defined if the target system doesn't have the
 * routine strcspn() available. See regexp.c for details.
 */

#ifdef	ATARI
#define	STRCSPN
#endif
