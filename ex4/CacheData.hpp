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
#include <string>
#include <set>
#include <unordered_map>
#include "DataBlock.hpp"
#include "FileNode.hpp"

using namespace std;

class CacheData
{
public:
    CacheData(char* root, char* mount, unsigned int size, unsigned int blocksNum);
    ~CacheData();
    char* getRoot();
    char* getMount();
    unsigned int getBlockSize();
    unsigned int getNumOfBlocks();
   
private:
	char* rootDir;
	char* mountDir;
	unsigned int blockSize;
	unsigned int numOfBlocks;

    set<DataBlock*, DataBlockComparator> cache;
    unordered_map<size_t, FileNode*> files;
    hash<string> hash_fn;

};

#endif	/* CACHE_DATA_H */

