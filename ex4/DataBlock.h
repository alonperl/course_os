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
    DataBlock(const DataBlock& orig);
    virtual ~DataBlock();
    
    char *getData();
    unsigned int getOffset();
    unsigned int getUseCount();
    
    bool operator<(DataBlock other);
private:
    char *_data;
    unsigned int _offset;
    unsigned int _useCount;
};

struct DataBlockComparator
{
    bool operator()(const DataBlock *lhs, const DataBlock *rhs);
};

#endif	/* DATABLOCK_H */

