/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.hpp"
#include <malloc.h>
#include <string.h>

int DataBlock::blockSize;

void DataBlock::setBlockSize(int size)
{
	blockSize = size;
}

DataBlock::DataBlock(char* data, size_t newHash)
{
	if (data == NULL)
	{
		throw -1;
	}

	_data = (char*)malloc(sizeof(char) * blockSize);

	strcpy(_data, data);

	hash = newHash;
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

unsigned int DataBlock::getUseCount()
{
  return _useCount;
}

void DataBlock::increaseUseCount()
{
	_useCount++;
}

bool DataBlockCompare::operator()(DataBlock* lhs, DataBlock* rhs)
{
  return lhs->getUseCount() < rhs->getUseCount();
}

