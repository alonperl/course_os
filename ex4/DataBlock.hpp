/* 
 * File:   DataBlock.h
 * Author: griffonn
 *
 * Created on 24 May 2015, 22:45
 */

#ifndef DATABLOCK_H
#define	DATABLOCK_H

#include <cstring>

using namespace std;

class DataBlock {
public:
	DataBlock(char* data, size_t newHash);
    virtual ~DataBlock();
    
    size_t hash;

    char *getData();
    unsigned int getBlockNum();
    unsigned int getUseCount();
    void increaseUseCount();

    static int blockSize;
    static void setBlockSize(int size);
    
private:
    char* _data;
    unsigned int _useCount;


};

struct DataBlockCompare
{
    bool operator()(DataBlock* lhs, DataBlock* rhs);
};

#endif	/* DATABLOCK_H */

