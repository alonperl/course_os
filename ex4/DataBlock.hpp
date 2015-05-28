/* 
 * File:   DataBlock.h
 * Author: griffonn
 *
 * Created on 24 May 2015, 22:45
 */

#ifndef DATABLOCK_H
#define	DATABLOCK_H

#include <string>
#include <string.h>

using namespace std;

class DataBlock {
public:
	DataBlock(char* data, unsigned long num, string path);
    virtual ~DataBlock();
    
    const unsigned long blockNum;
    const string blockPath;

    char *getData();
    unsigned int getUseCount();
    void increaseUseCount();

    static unsigned int s_blockSize;

private:
    char *_data;
    unsigned int _useCount;
};

struct DataBlockCompare
{
    bool operator()(DataBlock* lhs, DataBlock* rhs);
};

#endif	/* DATABLOCK_H */

