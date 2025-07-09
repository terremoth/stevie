#
# Makefile for Lattice C 5.0 on Amiga
#

.c.o:
	lc $(CFLAGS) $<

#CFLAGS = -cu -ma -DAMIGA
CFLAGS = -cu -ma -O -DAMIGA
LINKFLAGS = NODEBUG
LIBS = lib:a.lib+lib:lc.lib

MACH=	amiga.o

OBJ1=	main.o edit.o linefunc.o normal.o cmdline.o charset.o
OBJ2=	format_l.o misccmds.o help.o dec.o inc.o search.o alloc.o
OBJ3=	mk.o regexp.o regsub.o version.o
OBJ4=	s_io.o mark.o screen.o fileio.o param.o

OBJ= $(OBJ1) $(OBJ2) $(OBJ3) $(OBJ4) $(MACH)

SRC1=	main.c edit.c linefunc.c normal.c cmdline.c charset.c
SRC2=	format_l.c misccmds.c help.c dec.c inc.c search.c alloc.c
SRC3=	mk.c regexp.c regsub.c version.c
SRC4=	s_io.c mark.c screen.c fileio.c param.c

all: stevie
	say "done all"

stevie: $(OBJ) Makefile
	BLINK TO stevie FROM lib:cres.o $(OBJ) LIBRARY $(LIBS) $(LINKFLAGS)

clean:
	delete $(OBJ1)
	delete $(OBJ2)
	delete $(OBJ3)
	delete $(OBJ4)
	delete $(MACH)
	delete stevie
	delete tags

tags:
	ctags -t    $(SRC1)
	ctags -t -a $(SRC2)
	ctags -t -a $(SRC3)
	ctags -t -a $(SRC4)
	ctags -t -a ascii.h keymap.h macros.h param.h stevie.h term.h
	ctags -t -a amiga.c
	ctags -t -a amiga.h
