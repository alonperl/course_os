CC=g++
CFLAGS = -Wall -Wextra -g $(INCS)

AR=ar rvs
TAR=tar -cvs
TARNAME=ex2.tar

TARGETS=uthreads.cpp Thread.cpp Scheduler.cpp SignalManager.cpp PriorityQueue.cpp
LIBUTHREADS = libuthreads.a

TARSRCS=$(TARGETS) Makefile README

all: uthreads.a

uthreads.o: $(TARGETS)
	$(CC) $(CFLAGS) -c $(TARGETS)

uthreads.a: uthreads.o
	$(AR) uthreads.a uthreads.o

tests: test1 test2 test3 test4 test5

test1:
	$(CC) $(CFLAGS) -o test1 tests/test1.cpp $(TARGETS)

test2:
	$(CC) $(CFLAGS) -o test2 tests/test2.cpp $(TARGETS)

test3:
	$(CC) $(CFLAGS) -o test3 tests/test3.cpp $(TARGETS)

test4:
	$(CC) $(CFLAGS) -o test4 tests/test4.cpp $(TARGETS)

test5:
	$(CC) $(CFLAGS) -o test5 tests/test5.cpp $(TARGETS)

clean:
	rm -rf uthreads.o uthreads.a test1 test2 test3 test4 test5

tar:
	$(TAR) $(TARNAME) $(TARSRCS)

.PHONY: all