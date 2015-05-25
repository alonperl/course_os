/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.h"

DataBlock::DataBlock () { }

DataBlock::DataBlock (const DataBlock& orig) { }

DataBlock::~DataBlock () { }

struct DataBlockComparator
{
    bool operator()(const DataBlock *lhs, const DataBlock *rhs)
    {
      return lhs->getUseCount() < rhs->getUseCount();
    }
};

