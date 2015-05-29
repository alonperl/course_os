/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#ifndef CACHE_DATA_H
#define	CACHE_DATA_H

#include <climits>
#include <cstdlib>
#include <string>
#include <set>
#include <unordered_map>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <ctime>
#include <fstream>
#include "DataBlock.hpp"

using namespace std;

typedef unordered_map<string, DataBlock*> CachedPathBlocksMap;
typedef set<DataBlock*, DataBlockCompare> CachedBlocksSet;

class CacheData
{
public:
    CacheData(const string rootDir, const string mountDir, const string logFile, 
              const unsigned long maxBlocksCount, const unsigned int blockSize);
    
    string absolutePath(const char* path);
    void pushDataBlock(DataBlock* block);
    DataBlock* readBlockFromDisk(uint64_t  fh, const string path, unsigned long blockNum);
    void deleteLeastUsedBlock();
    void renameCachedBlocks(const string oldPath, const string newPath);
    void log(string action);

    CachedPathBlocksMap cachePathMap;
    CachedBlocksSet cacheFreqSet;
   
    const string rootDir; // TODO make const
    const string mountDir; // TODO make const
    const string logFile; // TODO make const
    const unsigned long maxBlocksCount;
    const unsigned int blockSize;
    
    unsigned long cacheSize();
private:
    unsigned long _cacheSize;
};

#endif	/* CACHE_DATA_H */

