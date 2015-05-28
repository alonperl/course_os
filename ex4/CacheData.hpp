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
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <ctime>
#include <fstream>
#include "DataBlock.hpp"

using namespace std;

typedef unordered_map<size_t, DataBlock*> CachedByPath;
typedef set<DataBlock*, DataBlockCompare> CachedBlocks;

class CacheData
{
public:
    CacheData(char* root, char* mount, string logfile, unsigned int blocksNum);
    ~CacheData();
    
    void getFullPath(char absPath[PATH_MAX], const char* path);

    void addDataBlock(size_t hash, DataBlock* block);
    size_t hashd(string absFilePath, int blockNum);

    unsigned int totalCachedBlocks;

    CachedByPath filesByHash;
    CachedBlocks filesByLFU;
   
    char* rootDir; // TODO make const
    char* mountDir; // TODO make const
    char* logPath; // TODO make const

    unsigned int maxBlocksNum; // TODO make const

    hash<string> hash_fn;
};

#endif	/* CACHE_DATA_H */

