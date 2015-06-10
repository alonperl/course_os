CC=g++
RM = rm -rf

INCS=-I.
CFLAGS = -Wall -g $(INCS)
LOADLIBES = -L./

CLIENTSRC = clftp.cpp
CLIENTX = clftp

SERVERSRC = srftp.cpp
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