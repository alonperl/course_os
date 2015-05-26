/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#include "CacheData.hpp"

CacheData::CacheData(char* root, char* mount, string logfile, unsigned int size, unsigned int blocksNum)
{
	_rootDir = realpath(root, NULL);
	_mountDir = realpath(mount, NULL);
	char rootDirCopy[PATH_MAX];
	strcpy(rootDirCopy, _rootDir);
	strncat(strncat(rootDirCopy, string("/").c_str(), PATH_MAX), logfile.c_str(), PATH_MAX); // TODO check that not exceeding PATH_MAX
	_blockSize = size;
	_numOfBlocks = blocksNum;
}

CacheData::~CacheData()
{
	free(_rootDir);
	free(_mountDir);
	free(_logPath);
}

char* CacheData::getRoot()
{
	return _rootDir;
}

char* CacheData::getMount()
{
	return _mountDir;
}


char* CacheData::getLogPath()
{
	return _logPath;
}

unsigned int CacheData::getBlockSize()
{
	return _blockSize;
}

unsigned int CacheData::getNumOfBlocks()
{
	return _numOfBlocks;
}

char* CacheData::getFullPath(const char* path)
{
	// TODO validate that length does not exceed PATH_MAX
	char result[PATH_MAX];
	strcpy(result, _rootDir);
	return strncat(result, path, PATH_MAX);
}