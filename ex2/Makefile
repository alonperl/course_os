CC=g++
CFLAGS=-Wall -Wextra $(INCS)

AR=ar rvs
TAR=tar -cvf
TARNAME=ex2.tar

TARGETS=uthreads.cpp Thread.cpp Scheduler.cpp SignalManager.cpp PriorityQueue.cpp
HEADERS=uthreads.h Thread.hpp Scheduler.hpp SignalManager.hpp PriorityQueue.hpp
LIBUTHREADS = libuthreads.a

TARSRCS=$(TARGETS) Makefile README

all: uthreads.a

uthreads.a: uthreads.o Thread.o Scheduler.o SignalManager.o PriorityQueue.o
	$(AR) $(LIBUTHREADS) $^
%.o: %.cpp
	$(CC) $(CFLAGS) -c $^

clean:
	rm -rf uthreads.o uthreads.a

tar:
	$(TAR) $(TARNAME) $(TARSRCS) $(HEADERS)

.PHONY: all
