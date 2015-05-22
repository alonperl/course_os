CC=g++ -Wall -Wextra -Wvla

all: libblockchain.a

tar: libblockchain.a
	tar -cvf ex3.tar blockchain.cpp Block.hpp Block.cpp Request.hpp \
	Request.cpp Chain.hpp Chain.cpp README Makefile

libblockchain.a: Chain.o Request.o Block.o blockchain.o
	ar rcs libblockchain.a Chain.o Request.o Block.o blockchain.o

Chain.o: Chain.hpp Chain.cpp Block.o
	g++ -Wall -Wextra -Wvla -std=c++11 -c Chain.cpp

Request.o: Request.hpp Request.cpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c Request.cpp

Block.o: Block.hpp Block.cpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c Block.cpp

blockchain.o: blockchain.h blockchain.cpp Request.o Block.o Chain.o
	g++ -Wall -Wextra -Wvla -std=c++11 -c blockchain.cpp -L. -lhash -lcrypto -lpthread

clean:
	rm -f *.o libblockchain.a

.PHONT: clean tar all