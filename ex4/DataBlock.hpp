/* 
 * File:   DataBlock.h
 * Author: griffonn
 *
 * Created on 24 May 2015, 22:45
 */

#ifndef DATABLOCK_H
#define	DATABLOCK_H

using namespace std;

class DataBlock {
public:
	DataBlock(char* data, long blockNum);
    virtual ~DataBlock();
    
    char *getData();
    unsigned int getBlockNum();
    unsigned int getUseCount();
    
private:
    char *_data;
    long _blockNum;
    unsigned int _useCount;
};

struct DataBlockComparator
{
    bool operator()(DataBlock* lhs, DataBlock* rhs)
    {
    	return lhs->getUseCount() < rhs->getUseCount();
    }
};

#endif	/* DATABLOCK_H */

