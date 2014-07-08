NAME=libltchan
VERSION := $(shell git describe --abbrev=4 --dirty --always)

SHARED=$(NAME).so
STATIC=$(NAME).a

DESTDIR=
PREFIX=/usr/local

all: $(SHARED) $(STATIC)

OFILES=\
	ltchan.o

CC=gcc
CFLAGS=-Wall -I. -ggdb -llthread

%.o: %.c
	$(CC) $(CFLAGS) -fPIC -c $*.c

$(SHARED): $(OFILES)
	$(CC) $(CFLAGS) -shared -o $(SHARED) $(OFILES)

$(STATIC): $(OFILES)
	ar rcs $(STATIC) $(OFILES)

clean:
	rm -f *.o $(SHARED) $(STATIC)

install: $(SHARED) $(STATIC)
	cp $(SHARED) $(DESTDIR)$(PREFIX)/lib
	cp $(STATIC) $(DESTDIR)$(PREFIX)/lib
	cp *.h $(DESTDIR)$(PREFIX)/include

dist: clean
	git archive --format=tar --prefix=$(NAME)-$(VERSION)/ HEAD | xz -9v > $(NAME)-$(VERSION).tar.xz

.PHONY: all clean
