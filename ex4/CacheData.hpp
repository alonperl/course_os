/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#ifndef CACHE_DATA_H
#define	CACHE_DATA_H

#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <set>
#include <unordered_map>
#include "DataBlock.hpp"
#include "FileNode.hpp"

using namespace std;

class CacheData
{
public:
    CacheData(char* root, char* mount, string logfile, unsigned int size, unsigned int blocksNum);
    ~CacheData();
    char* getRoot();
    char* getMount();
    char* getLogPath();
    unsigned int getBlockSize();
    unsigned int getNumOfBlocks();
    char* getFullPath(const char* path);
   
private:
	char* _rootDir;
    char* _mountDir;
	char* _logPath;
	unsigned int _blockSize;
	unsigned int _numOfBlocks;

    set<DataBlock*, DataBlockComparator> _cache;
    unordered_map<size_t, FileNode*> _files;
    hash<string> _hash_fn;

};

#endif	/* CACHE_DATA_H */

