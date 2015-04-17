CC=g++
RANLIB=ranlib

TARGETS=uthreads.cpp Thread.cpp Scheduler.cpp SignalManager.cpp PriorityQueue.cpp

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

all: uthreads.cpp Thread.cpp Scheduler.cpp SignalManager.cpp PriorityQueue.cpp
	$(CC) $(CFLAGS) -o uthreads uthreads.cpp Thread.cpp Scheduler.cpp SignalManager.cpp PriorityQueue.cpp

$(TARGETS): $(LIBOBJ)
	$(AR) $(ARFLAGS) $@ $^
	$(RANLIB) $@

clean:
	$(RM) $(TARGETS) $(OBJ) $(LIBOBJ)

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)
