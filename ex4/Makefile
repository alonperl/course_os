CC = g++
CFLAGS = -std=c++0x -g -Wall

EXEC = CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp DataBlock.cpp CacheData.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) -o $(EXEC) $(SOURCES) $(LIBS)
	
run: all
	./$(EXEC) /tmp/td /tmp/mp 1 1024