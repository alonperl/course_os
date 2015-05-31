/**
 * @file CacheData.cpp
 * @author  griffonn ednussi
 * @version 1.0
 * @date 30 May 2015
 * 
 * @brief Defeniation of the CacheData structure class
 *
 * @section LICENSE
 * This program is a free software. You can freely redistribute it.
 *
 * @section DESCRIPTION
 * A CachedData struture is one that holds according to LFU logic as many
 * dataBlock and of size the user decide in order to help the fuse saving time
 * by holding data which is used frequently by the user - so less access to memory would occur
 */
#include "CacheData.hpp"
#include <malloc.h>
#include <string.h>

#define INIT_LFU 0

/**
* @param path - The path we wish to get it's real Path
* @return The real of a realative path 
*/
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

/**
 * @brief CacheData Constructor
 * @param rootdir - The root directory path
 * @param logFile - The log file's path
 * @param maxBlocksCount - the maximum amount of blocks allowed to cache
 * @param blockSize - the max size of each dataBlock
 * @param num - The num of dataBlock from the file
 */
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

/**
 * @brief CacheData Destructor - delete and clears all inner members
 */
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

/**
 * @param path - The path we wish to get it's absoulte Path
 * @return The absoulte of a relative path
 */
string CacheData::absolutePath(const char* path)
{
    string absolute("");
    absolute = rootDir + string(path);
	return absolute.length() > PATH_MAX ? "" : absolute;
}

/**
 * @param fh - the file handler
 * @param path - the path of the file the inforamtion is in
 * @param blockNum - the number of block in the file
 * @return A pointer to a dataBlock containning the requested data
 */
DataBlock *CacheData::readBlockFromDisk(uint64_t  fh, const string path,
										unsigned long blockNum)
{
	char* dataBuffer = (char*)malloc(sizeof(char) * (blockSize + 1));
	// char* dataBuffer = new char[blockSize]();
	if (dataBuffer == NULL)
	{
		return NULL;
	}
	dataBuffer[0] = '\0';
	
	int result = pread(fh, dataBuffer, blockSize, blockNum * blockSize);
	// int result = memcpy()
	if (result < 0) // Could not read
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

/**
 * @param block - the block to add to cache
 * Adds to cache a new dataBlock
 */
void CacheData::pushDataBlock(DataBlock* block)
{
	cachePathMap.insert(pair<string, DataBlock*>(_blockKey(block), block));
	cacheFreqSet.insert(block);
	_cacheSize++;
}

/**
 * @param action - the action to record
 * Log's in the requested action
 */  
void CacheData::log(string action)
{
//	cout<<action<<endl;
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

/**
 * Delete the least used datablock in cache
 */
void CacheData::deleteLeastUsedBlock() 
{
	CachedBlocksSet::iterator cachedBlocksIter = cacheFreqSet.begin();

	DataBlock* block = *cachedBlocksIter;

	cachePathMap.erase(cachePathMap.find(_blockKey (block)));
	cacheFreqSet.erase(cachedBlocksIter);

	delete block;

	_cacheSize--;
}

/**
 * @param fh - the file handler
 * @param path - the path of the file the inforamtion is in
 * Renames all cached dataBlock's path
 */
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

/**
 * @return block's use count
 */
unsigned long CacheData::cacheSize()
{
	return _cacheSize;
}

/**
 * @param block - the block we want it's key
 * @return The block's key
 */
string CacheData::_blockKey(DataBlock* block)
{
	return block->path + ":" + to_string(block->num);
}