/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.hpp"
#include <malloc.h>
#include <cstring>

DataBlock::DataBlock(char* data, long blockNum, char* path)
{
	if (data == NULL)
	{
		throw -1; //TODO - no one catches this
	}

	strcpy(_data, data);
	strcpy(_path, path);

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

char* DataBlock::getPath()
{
  return _path;
}

unsigned int DataBlock::getBlockNum()
{
  return _blockNum;
}

unsigned int DataBlock::getUseCount()
{
  return _useCount;
}
void DataBlock::increaseUseCount()
{
	_useCount++;
}

// bool DataBlockMapComparator::operator()(int lhs, int rhs)
// {
//   return lhs < rhs;
// }