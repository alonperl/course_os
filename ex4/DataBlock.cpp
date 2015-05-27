/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.hpp"
#include <malloc.h>

DataBlock::DataBlock(char* data, long blockNum)
{
	if (data == NULL)
	{
		throw -1;
	}

	strcpy(_data, data);

	_blockNum = blockNum;
	_useCount = 0;
}

DataBlock::~DataBlock()
{
	free(_data);
	_data = NULL;
}

char* DataBlock::getData()
{
  return _data;
}

unsigned int DataBlock::getBlockNum()
{
  return _blockNum;
}

unsigned int DataBlock::getUseCount()
{
  return _useCount;
}

bool DataBlockComparator::operator()(DataBlock *lhs, DataBlock *rhs)
{
  return lhs->getUseCount() < rhs->getUseCount();
}

