CC=g++ -Wall -Wextra -Wvla

all: libblockchain.a

tar: libblockchain.a
	tar -cvf ex3.tar blockchain.cpp Block.hpp Block.cpp AddRequest.hpp \
	AddRequest.cpp Chain.h Chain.cpp README Makefile

libblockchain.a: Chain.o AddRequest.o Block.o blockchain.o
	ar rcs libblockchain.a Chain.o AddRequest.o Block.o blockchain.o

Chain.o: Chain.hpp Chain.cpp Block.o
	g++ -Wall -Wextra -Wvla -std=c++11 -c Chain.cpp

AddRequest.o: AddRequest.hpp AddRequest.cpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c AddRequest.cpp

Block.o: Block.hpp Block.cpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c Block.cpp

blockchain.o: blockchain.h blockchain.cpp AddRequest.o Block.o Chain.o
	g++ -Wall -Wextra -Wvla -std=c++11 -c blockchain.cpp -L. -lhash -lcrypto -lpthread

test:
	g++ -g -std=c++11 -o runt test1.cpp -L. -lhash -lcrypto -lpthread blockchain.cpp Block.cpp Chain.cpp AddRequest.cpp

clean:
	rm -f *.o libblockchain.a

.PHONT: clean tar all