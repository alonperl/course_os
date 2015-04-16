CC=gcc
RANLIB=ranlib

TARGETS=thread.cpp statesManager.cpp uthreads.cpp signalManager.cpp

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

all: thread.cpp statesManager.cpp uthreads.cpp signalManager.cpp
	g++ -std=c++11 -g -o uthreads thread.cpp statesManager.cpp uthreads.cpp signalManager.cpp

$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(OBJ) $(LIBOBJ)

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)