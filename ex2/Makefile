CC=g++
CFLAGS = -Wall -Wextra $(INCS)

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
	$(CC) $(CFLAGS) -o test1 tests/test1.cpp $(TARGETS) > /dev/null
	./test1 > test1.out
	diff test1.out tests/test1.out > /dev/null && echo ::: Passed 1 ::: || ::: Failed 1 :::
	rm -rf test1.out test1

test2:
	$(CC) $(CFLAGS) -o test2 tests/test2.cpp $(TARGETS) > /dev/null
	./test2 > test2.out
	diff test2.out tests/test2.out > /dev/null && echo ::: Passed 2 ::: || ::: Failed 2 :::
	rm -rf test1.out test2

test3:
	$(CC) $(CFLAGS) -o test3 tests/test3.cpp $(TARGETS) > /dev/null
	./test3 > test3.out
	diff test3.out tests/test3.out > /dev/null && echo ::: Passed 3 ::: || ::: Failed 3 :::
	rm -rf test1.out test3

test4:
	$(CC) $(CFLAGS) -o test4 tests/test4.cpp $(TARGETS) > /dev/null
	./test4 > test4.out
	diff test4.out tests/test4.out > /dev/null && echo ::: Passed 4 ::: || ::: Failed 4 :::
	rm -rf test4.out test4

test5:
	$(CC) $(CFLAGS) -o test5 tests/test5.cpp $(TARGETS) > /dev/null
	./test5 > test5.out
	diff test5.out tests/test5.out > /dev/null && echo ::: Passed 5 ::: || ::: Failed 5 :::
	rm -rf test5.out test5

clean:
	rm -rf uthreads.o uthreads.a

tar:
	$(TAR) $(TARNAME) $(TARSRCS)

.PHONY: all