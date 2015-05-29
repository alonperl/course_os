/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#include "CacheData.hpp"

#define INIT_LFU 0

CacheData::CacheData(const string root, const string mount, const string logfile, 
					 const unsigned long maxBlocksCount, const unsigned int blockSize)
            : rootDir(realpath(root.c_str(), NULL))
            , mountDir(realpath(mount.c_str(), NULL))
            , logFile(root+logfile)
            , maxBlocksCount(maxBlocksCount)
			, blockSize(blockSize)
{
	_cacheSize = 0;
}

string CacheData::absolutePath(const char* path)
{
	// TODO validate that length does not exceed PATH_MAX
    return rootDir + string(path);
}

DataBlock *CacheData::readBlockFromDisk(uint64_t  fh, const string path,
										unsigned long blockNum)
{
	string data;
	char* dataBuffer = (char*)malloc(sizeof(char) * blockSize);

	int result = pread(fh, dataBuffer, blockSize, blockNum * blockSize);
	data = string(dataBuffer);

	free(dataBuffer);
	dataBuffer = NULL;

	if (result < 0) // Could not read
	{
		return NULL;
	}

	// Check if there is more place in cache
	if (_cacheSize >= maxBlocksCount)
	{
		deleteLeastUsedBlock();
	}

	DataBlock* block = new DataBlock(data, path, blockNum);

	pushDataBlock(block);
	
	return block;
}

void CacheData::pushDataBlock(DataBlock* block)
{
	cachePathMap.insert(pair<string, DataBlock*>(block->path, block));
	cacheFreqSet.insert(block);
	_cacheSize++;
}

void CacheData::log(string action)
{
	action = action.substr(8, action.length());
	ofstream logStream(logFile, ios_base::app);
	if (logStream.good())
	{
		time_t unixTime = std::time(nullptr); //TODO nullptr??
		logStream << unixTime << " " << action << endl;
		logStream.close();
	}
	else
	{
		//TODO error?>
	}
}

void CacheData::deleteLeastUsedBlock() 
{
	CachedBlocksSet::iterator cachedBlocksIter = cacheFreqSet.begin();

	DataBlock* block = *cachedBlocksIter;

	cachePathMap.erase(cachePathMap.find(block->path));
	cacheFreqSet.erase(cachedBlocksIter);

	delete block;

	_cacheSize--;
}

void CacheData::renameCachedBlocks(const string oldPath, const string newPath)
{
	for (CachedBlocksSet::iterator blockIter = cacheFreqSet.begin(); 
		 blockIter != cacheFreqSet.end(); blockIter++)
	{
		if ((*blockIter) != NULL && 
			(*blockIter)->path.compare(0, oldPath.length(), oldPath))
		{
			// Found relevant block
			cachePathMap.erase((*blockIter)->path);
			(*blockIter)->path.replace(0, oldPath.length(), newPath);
			cachePathMap.insert(pair<string, DataBlock*>((*blockIter)->path, (*blockIter)));
		}
	}
}

unsigned long CacheData::cacheSize()
{
	return _cacheSize;
}