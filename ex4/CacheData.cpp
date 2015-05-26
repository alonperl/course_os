/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#include "CacheData.hpp"

CacheData::CacheData(char* root, char* mount, unsigned int size, unsigned int blocksNum)
{
	rootDir = realpath(root, NULL);
	mountDir = realpath(mount, NULL);
	blockSize = size;
	numOfBlocks = blocksNum;
}

CacheData::~CacheData()
{
	free(rootDir);
	free(mountDir);
}

char* CacheData::getRoot()
{
	return rootDir;
}

char* CacheData::getMount()
{
	return mountDir;
}

unsigned int CacheData::getBlockSize()
{
	return blockSize;
}

unsigned int CacheData::getNumOfBlocks()
{
	return numOfBlocks;
}