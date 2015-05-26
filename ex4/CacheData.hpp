/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#ifndef FILENODE_H
#define	FILENODE_H

#include <limits.h>
#include <stdlib.h>

using namespace std;

class CacheData {
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
};

#endif	/* FILENODE_H */

