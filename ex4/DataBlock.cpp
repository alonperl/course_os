/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.hpp"

DataBlock::DataBlock(const string data, const string path, 
					 const unsigned long num) 
	: data(data)
    , path(path)
	, num(num)
{
	_useCount = 0;
}

unsigned long DataBlock::getUseCount()
{
	return _useCount;
}

void DataBlock::increaseUseCount()
{
	_useCount++;
}

bool DataBlockCompare::operator()(DataBlock* lhs, DataBlock* rhs)
{
  return lhs->_useCount < rhs->_useCount;
}

