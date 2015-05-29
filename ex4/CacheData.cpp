/* 
 * File:   CacheData
 * Author: ednussi
 *
 * Created on 25 May 2015, 22:40
 */

#include "CacheData.hpp"
#include <malloc.h>

#define INIT_LFU 0

string CacheData::stringRealPath(string path)
{
	char* realPath = realpath(path.c_str(), NULL);
	if (realPath == NULL)
	{
		// Could not get real path
		return "NULL";
	}
	
	string strRealPath(realPath);
	free(realPath);
	return strRealPath;
}

CacheData::CacheData(const string root, const string logfile, 
					 const unsigned long maxBlocksCount, const unsigned int blockSize)
            : rootDir(stringRealPath(root))
            , logFile(rootDir+"/"+logfile)
            , maxBlocksCount(maxBlocksCount)
			, blockSize(blockSize)
{
	if (rootDir == "NULL")
	{
		throw -errno;
	}
	_cacheSize = 0;
}

CacheData::~CacheData()
{
	for(CachedBlocksSet::iterator cachedBlockIter = cacheFreqSet.begin(); 
		cachedBlockIter != cacheFreqSet.end(); cachedBlockIter++)
	{
		//destroy all blocks
		delete *cachedBlockIter;
	}
	// Clears set and map
	cacheFreqSet.clear();
	cachePathMap.clear();
}

string CacheData::absolutePath(const char* path)
{
	// TODO validate that length does not exceed PATH_MAX
    return rootDir + string(path);
}

DataBlock *CacheData::readBlockFromDisk(uint64_t  fh, const string path,
										unsigned long blockNum)
{
	char* dataBuffer = (char*)malloc(sizeof(char) * (blockSize + 1));
	if (dataBuffer == NULL)
	{
		return NULL;
	}
	
	int result = pread(fh, dataBuffer, blockSize, blockNum * blockSize);
	if (result < 0 || dataBuffer == NULL) // Could not read
	{
		free(dataBuffer);
		return NULL;
	}
	
	string data = string(dataBuffer);

	free(dataBuffer);
	dataBuffer = NULL;

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
	cachePathMap.insert(pair<string, DataBlock*>(_blockKey(block), block));
	cacheFreqSet.insert(block);
	_cacheSize++;
}

void CacheData::log(string action)
{
	cout<<action<<endl;
	action = action.substr(8, action.length());
	ofstream logStream(logFile, ios_base::app);
	if (logStream.good())
	{
		time_t unixTime = std::time(nullptr);
		logStream << unixTime << " " << action << endl;
		logStream.close();
	}
	else
	{
		cout<<"cannot create logfile"<<endl;
		throw -1;
	}
}

void CacheData::deleteLeastUsedBlock() 
{
	CachedBlocksSet::iterator cachedBlocksIter = cacheFreqSet.begin();

	DataBlock* block = *cachedBlocksIter;

	cachePathMap.erase(cachePathMap.find(_blockKey (block)));
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
			cachePathMap.insert(pair<string, DataBlock*>(_blockKey(*blockIter), (*blockIter)));
		}
	}
}

unsigned long CacheData::cacheSize()
{
	return _cacheSize;
}

string CacheData::_blockKey(DataBlock* block)
{
	return block->path + ":" + to_string(block->num);
}