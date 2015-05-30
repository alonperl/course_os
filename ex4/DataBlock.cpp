/**
 * @file DataBlock.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 30 May 2015
 * 
 * @brief A single DataBlock class
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * Implementation of the dataBlock data structure
 */

#include "DataBlock.hpp"

/**
 * @brief Block Constructor
 * 
 * @param data - The data which the block contains
 * @param path - The path of the file the block is related to
 * @param num - The num of dataBlock from the file
 */
DataBlock::DataBlock(const string data, const string path, 
					 const unsigned long num) 
	: data(data)
    , path(path)
	, num(num)
{
	_useCount = 0;
}

/**
 * @return block's use count
 */
unsigned long DataBlock::getUseCount()
{
	return _useCount;
}

/**
 * Increases the use count of this block
 */
void DataBlock::increaseUseCount()
{
	_useCount++;
}

/**
 * @brief Block Comparator
 * @return True is left hand side Block's use count is smaller than the right hand side block
 */
bool DataBlockCompare::operator()(DataBlock* lhs, DataBlock* rhs)
{
  return lhs->_useCount < rhs->_useCount;
}

