CC=g++
RM = rm -rf

INCS=-I.
CFLAGS = -std=c++11 -Wall -g $(INCS) -lpthread
LOADLIBES = -L./

CLIENTSRC = clftp.cpp packets.cpp
CLIENTX = clftp

SERVERSRC = srftp.cpp packets.cpp
SERVERX = srftp

TAR=tar
TARFLAGS=-cvf
TARNAME=ex5.tar
TARSRCS=$(CLIENTSRC) $(SERVERSRC) Makefile README

all: sr cl

cl: $(CLIENTSRC)
	$(CC) $(CFLAGS) -o $(CLIENTX) $(CLIENTSRC)

sr: $(SERVERSRC)
	$(CC) $(CFLAGS) -o $(SERVERX) $(SERVERSRC)

clean:
	$(RM) $(CLIENTSRC) $(CLIENTX) $(SERVERSRC) $(SERVERX) *~ *core

depend:
	makedepend -- $(CFLAGS) -- $(SRC) $(LIBSRC)

tar:
	$(TAR) $(TARFLAGS) $(TARNAME) $(TARSRCS)