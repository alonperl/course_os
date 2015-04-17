CC=g++
CFLAGS=-Wall -Wextra $(INCS)

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
	./test1 > test1.out
	diff test1.out tests/test1.out && (printf "\033[32m" && \
	echo ::: Passed 1 ::: && printf "\033[m") || (printf "\033[31m" && echo ::: Failed 1 ::: && printf "\033[m")
	rm -rf test1.out test1

test2:
	$(CC) $(CFLAGS) -o test2 tests/test2.cpp $(TARGETS)
	./test2 > test2.out
	diff test2.out tests/test2.out && (printf "\033[32m" && \
	echo ::: Passed 2 ::: && printf "\033[m") || (printf "\033[31m" && echo ::: Failed 2 ::: && printf "\033[m")
	rm -rf test2.out test2

test3:
	$(CC) $(CFLAGS) -o test3 tests/test3.cpp $(TARGETS)
	./test3 > test3.out
	diff test3.out tests/test3.out && (printf "\033[32m" && \
	echo ::: Passed 3 ::: && printf "\033[m") || (printf "\033[31m" && echo ::: Failed 3 ::: && printf "\033[m")
	rm -rf test3.out test3

test4:
	$(CC) $(CFLAGS) -o test4 tests/test4.cpp $(TARGETS)
	./test4 > test4.out
	diff test4.out tests/test4.out && (printf "\033[32m" && \
	echo ::: Passed 4 ::: && printf "\033[m") || (printf "\033[31m" && echo ::: Failed 4 ::: && printf "\033[m")
	rm -rf test4.out test4

test5:
	$(CC) $(CFLAGS) -o test5 tests/test5.cpp $(TARGETS)
	./test5 > test5.out
	diff test5.out tests/test5.out && (printf "\033[32m" && \
	echo ::: Passed 5 ::: && printf "\033[m") || (printf "\033[31m" && echo ::: Failed 5 ::: && printf "\033[m")
	rm -rf test5.out test5

clean:
	rm -rf uthreads.o uthreads.a

tar:
	$(TAR) $(TARNAME) $(TARSRCS)

.PHONY: all