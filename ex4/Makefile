CC = g++
CFLAGS = -std=c++0x -g -Wall -Wvla

EXEC = CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp DataBlock.cpp CacheData.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) -o $(EXEC) $(SOURCES) $(LIBS)
	
run: all
	mkdir /tmp/td
	mkdir /tmp/mp
	touch /tmp/td/1
	touch /tmp/td/2
	echo "test text" > /tmp/td/2
	./$(EXEC) /tmp/td /tmp/mp 1024 1024

clean:
	rm -rf /tmp/td
	rm -rf /tmp/mp