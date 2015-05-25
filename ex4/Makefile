CC = g++
CFLAGS = -std=c++11 -g -Wall -Wextra -Wvla

EXEC = -o CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp FileNode.cpp DataBlock.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) $(EXEC) $(SOURCES) $(LIBS)