/* 
 * File:   DataBlock.h
 * Author: griffonn
 *
 * Created on 24 May 2015, 22:45
 */

#ifndef DATABLOCK_H
#define	DATABLOCK_H

#include <string>

using namespace std;

class DataBlock {
    friend class DataBlockCompare;
public:
    DataBlock(const string data, const string path, const unsigned long num);

    const string data;
    string path;
    const unsigned long num;
    
    unsigned long getUseCount();
    void increaseUseCount();

private:
    unsigned long _useCount;
};

struct DataBlockCompare
{
    bool operator()(DataBlock* lhs, DataBlock* rhs);
};

#endif	/* DATABLOCK_H */

