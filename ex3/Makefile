CC=g++ -Wall -Wextra -Wvla

all: libblockchain.a

tar: libblockchain.a
	tar -cvf ex3.tar blockchain.cpp Block.hpp Block.cpp \
	Chain.h Chain.cpp libblockchain.a README Makefile

libblockchain.a: Chain.o Block.o blockchain.o
	ar rcs libblockchain.a Chain.o Block.o blockchain.o libblockchain.a

Chain.o: Chain.hpp Chain.cpp Block.hpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c Chain.cpp

Block.o: Block.hpp Block.cpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c Block.cpp

blockchain.o: blockchain.h blockchain.cpp Chain.hpp Block.hpp
	g++ -Wall -Wextra -Wvla -std=c++11 -c blockchain.cpp -L. -lhash -lcrypto -lpthread

clean:
	rm -f *.o libblockchain.a

.PHONT: clean tar all

block: Block.cpp Block.hpp
	$(CC) -o block.o Block.cpp
