# @(#) $Id: Makefile,v 1.33 2005/07/25 19:41:06 alex Exp $
# Makefile for EXIFPROBE image file probe

# Any compiler should do
CC?=cc
CFLAGS=-DCOLOR -O -std=c99

# You probably don't need these
# CFLAGS=-DCOLOR -g -O -Dlint -Wall  -Wno-long-long 
# SPLINTCFLAGS=-DCOLOR -Dlint -showsummary

# If your system has strcasestr(3) you can use this if you like.
# If not defined a fake routine is used (which should be good enough,
# this is used only for the -C option)
# CFLAGS+=-DSTRCASESTR=strcasestr

#Solaris; cultural differences
#CFLAGS += -Du_int32_t=uint32_t

# You'll need this on big-endian machines if (and ONLY if) "byteorder.h"
# is not created properly on first invocation.
#CFLAGS += -DNATIVE_BYTEORDER_BIGENDIAN     # or add to ./byteorder.h

# On Solaris, you may need to use 'gmake', or edit the next line.
PREFIX ?= /usr/local
BINDIR=$(PREFIX)/bin
MANDIR=$(PREFIX)/man/man1


INCLUDES=defs.h extern.h misc.h tifftags.h exiftags.h \
	tiffeptags.h jpegtags.h global.h

MAKERSRCS=maker_nikon.c maker_epson.c maker_olympus.c maker_fujifilm.c \
	maker_casio.c maker_minolta.c maker_sanyo.c maker_canon.c \
    maker_leica.c maker_agfa.c maker_asahi.c maker_panasonic.c \
    maker_kyocera.c maker_toshiba.c maker_sigma.c maker_sony.c \
    maker_konica.c maker_hp.c maker_kodak.c maker_ricoh.c \
    maker_traveler.c

MAKEROBJS=maker_nikon.o maker_epson.o maker_olympus.o maker_fujifilm.o \
	maker_casio.o maker_minolta.o maker_sanyo.o maker_canon.o \
    maker_leica.o maker_agfa.o maker_asahi.o maker_panasonic.o \
    maker_kyocera.o maker_toshiba.o maker_sigma.o maker_sony.o \
    maker_konica.o maker_hp.o maker_kodak.o maker_ricoh.o \
    maker_traveler.o

SRCS=main.c readfile.c print.c tagnames.c process.c misc.c interpret.c \
	print_maker.c maker_generic.c maker_tagnames.c options.c ciff.c\
    jp2000.c mrw.c raf.c x3f.c \
	$(MAKERSRCS)

OBJS=main.o readfile.o print.o tagnames.o process.o misc.o interpret.o \
	print_maker.o maker_generic.o maker_tagnames.o options.o ciff.o \
    jp2000.o mrw.o veclib.o raf.o x3f.o \
	$(MAKEROBJS)

all: byteorder.h exifprobe

byteorder.h:
	@echo -n "Creating byteorder.h: "
	@sh makebyteorder.sh
	@cat byteorder.h
	@echo 

veclib.o: lib/veclib.c
	$(CC) -o veclib.o -c $(CFLAGS) lib/veclib.c

# Need the math library to calculate some APEX things...
exifprobe: $(OBJS) $(INCLUDES) Makefile
	./mkcomptime > comptime.c
	$(CC) -o exifprobe $(CFLAGS) $(OBJS) comptime.c -lm

install: all
	cp exifprobe $(BINDIR)
	cp exifgrep $(BINDIR)
	cp exifprobe.1 $(MANDIR)
	cp exifgrep.1 $(MANDIR)

# if you want HTML man pages, pick a target; I prefer man2html
rman: exifprobe.1 exifgrep.1
    # edit at <blockquote> & APP0...APPn; rman gets it wrong
	rman -f HTML exifprobe.1 > exifprobe-rman.html
	rman -f HTML exifgrep.1 > exifgrep-rman.html

man: exifprobe.1 exifgrep.1
	groff -man -Thtml exifprobe.1 > exifprobe-man.html
	groff -man -Thtml exifgrep.1 > exifgrep-man.html

man2html:
	nroff -man exifprobe.1 | man2html -pgsize 1024 -topm 0 -botm 2 -noheads > exifprobe.1.html
	nroff -man exifgrep.1 | man2html -pgsize 1024 -topm 0 -botm 2 -noheads > exifgrep.1.html

clean:
	rm -f exifprobe *.o comptime.c

distclean: clean undepend
	rm -f byteorder.h MANIFEST.* Makefile.bak exifprobe-*.tar.gz

depend: byteorder.h
	makedepend -- $(CFLAGS) -- $(SRCS)

undepend:
	makedepend

splint: $(SRCS) $(INCLUDES) Makefile
	splint $(SPLINTCFLAGS) $(SRCS)

# DO NOT DELETE THIS LINE -- make depend depends on it.
