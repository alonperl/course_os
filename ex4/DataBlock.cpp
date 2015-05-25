/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.h"

DataBlock::DataBlock () { }

DataBlock::~DataBlock () { }

unsigned int DataBlock::getUseCount()
{
  return 0;
}


bool DataBlockComparator::operator()(DataBlock *lhs, DataBlock *rhs)
{
  return lhs->getUseCount() < rhs->getUseCount();
}

