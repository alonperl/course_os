CC = g++
CFLAGS = -g -Wall -Wextra -Wvla

EXEC = -o CachingFileSystem
LIBS = -lfuse
SOURCES = CachingFileSystem.cpp

FUSE_FLAGS = -D_FILE_OFFSET_BITS=64

all:
	$(CC) $(CFLAGS) $(FUSE_FLAGS) $(EXEC) $(SOURCES) $(LIBS)