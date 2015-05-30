/**
 * @file DataBlock.hpp
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

#ifndef DATABLOCK_H
#define	DATABLOCK_H

#include <string>

using namespace std;

/**
 * @brief DataBlock class represents single data block of a certain file, consisting
 * of data, path of file it was originated from and number of block it represents.
 */
class DataBlock {
    friend class DataBlockCompare;

public:
	/**
	 * @brief Block Constructor
	 * @param data - The data which the block contains
	 * @param path - The path of the file the block is related to
	 * @param num - The num of dataBlock from the file
	 */
    DataBlock(const string data, const string path, const unsigned long num);

    const string data;

    /* The path of the file this Block's data is from */
    string path;

    /* Absolute number of this block in the file it represents */
    const unsigned long num;
    
    /**
 	 * @return block's use count
     */
    unsigned long getUseCount();
    
    /**
 	 * Increases the use count of this block
 	 */
 	void increaseUseCount();

private:
    unsigned long _useCount;
};

/**
 * @brief DataBlockCompare Struct. contains a single comparator function
 * in order to be used in order to order the data blocks.
 */
struct DataBlockCompare
{
    bool operator()(DataBlock* lhs, DataBlock* rhs);
};

#endif	/* DATABLOCK_H */

