CC = g++
CFLAGS = -std=c++0x -g -Wall

EXEC = CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp DataBlock.cpp CacheData.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) -o $(EXEC) $(SOURCES) $(LIBS)
	
tar:
	tar -cvf ex4.tar CachingFileSystem.cpp DataBlock.hpp DataBlock.cpp \
	CacheData.cpp CacheData.hpp README Makefile

clean:
	rm -rf ./CachingFileSystem ex4.tar

.PHONY = all clean