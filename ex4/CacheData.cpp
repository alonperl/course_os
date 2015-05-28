/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#include "CacheData.hpp"

#define INIT_LFU 0

CacheData::CacheData(char* root, char* mount, string logfile, unsigned int blocksNum)
{
	rootDir = realpath(root, NULL);
	mountDir = realpath(mount, NULL);
	char logPath[PATH_MAX];
	strcpy(logPath, rootDir);
	strncat(strncat(logPath, string("/").c_str(), PATH_MAX), logfile.c_str(), PATH_MAX); // TODO check that not exceeding PATH_MAX
	maxBlocksNum = blocksNum;
	totalCachedBlocks = 0;
}

CacheData::~CacheData()
{
	free(rootDir);
	free(mountDir);
	free(logPath);
}

void CacheData::getFullPath(char absPath[PATH_MAX], const char* path)
{
	// TODO validate that length does not exceed PATH_MAX
	strcpy(absPath, rootDir);
	strncat(absPath, path, PATH_MAX);
}

void CacheData::addDataBlock(size_t hash, DataBlock* block)
{
	filesByHash.insert(pair<size_t, DataBlock*>(hash, block));
	filesByLFU.insert(block);
}

size_t CacheData::hashd(string absFilePath, int blockNum)
{
	return hash_fn(absFilePath + to_string(blockNum));
}