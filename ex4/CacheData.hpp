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
#include <map>
#include "DataBlock.hpp"

using namespace std;

typedef map<int, DataBlock*, DataBlockComparator> BlockMap;

class BlockMapComparator
{
public:
    bool operator()(BlockMap *lhs, BlockMap *rhs);
};

typedef map<size_t, BlockMap*, BlockMapComparator> CacheMap;

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

    hash<string> hash_fn;
    CacheMap fileMaps;
   
private:
    char* _rootDir;
    char* _mountDir;
    char* _logPath;
    size_t _blockSize; // TODO make const
    unsigned int _numOfBlocks;

};

#endif	/* CACHE_DATA_H */

