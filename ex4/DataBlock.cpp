/* 
 * File:   DataBlock.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:45
 */

#include "DataBlock.hpp"
#include <malloc.h>

unsigned int DataBlock::s_blockSize;


DataBlock::DataBlock(char* data, unsigned long num, string path) 
	: blockNum((const unsigned long)num)
	, blockPath((const string)path)
{
	_data = (char*)malloc(sizeof(char) * s_blockSize);

	if (data == NULL || _data == NULL)
	{
		throw -1; //TODO - no one catches this
	}

	strcpy(_data, data);

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

