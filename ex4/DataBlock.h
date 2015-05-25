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
    DataBlock();
    virtual ~DataBlock();
    
    char *getData();
    unsigned int getOffset();
    unsigned int getUseCount();
    
private:
    char *_data;
    unsigned int _offset;
    unsigned int _useCount;
};

class DataBlockComparator
{
public:
    bool operator()(DataBlock *lhs, DataBlock *rhs);
};

#endif	/* DATABLOCK_H */

