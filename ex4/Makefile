CC = g++
CFLAGS = -std=c++11 -g -Wall -Wvla

EXEC = -o CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp DataBlock.cpp CacheData.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64 `pkg-config fuse --cflags --libs`

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) $(EXEC) $(SOURCES) $(LIBS)