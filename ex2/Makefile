CC=gcc
RANLIB=ranlib

LIBSRC=uthreads.cpp
LIBOBJ=$(LIBSRC:.c=.o)

INCS=-I.
CFLAGS = -Wall -Wextra -g $(INCS)
LOADLIBES = -L./

UTHREADSLIB = libuthreads.a
TARGETS = $(UTHREADSLIBLIB)

TAR=tar
TARFLAGS=-cvf
TARNAME=ex2.tar
TARSRCS=$(LIBSRC) Makefile README

all: thread.cpp statesManager.cpp uthreads.cpp
	g++ -g -o uthreads uthreads.cpp thread.cpp statesManager.cpp

$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(OBJ) $(LIBOBJ)

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)