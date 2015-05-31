/**
 * @file CacheData.hpp
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


#ifndef CACHE_DATA_H
#define	CACHE_DATA_H

#include <climits>
#include <cstdlib>
#include <string>
#include <set>
#include <unordered_map>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <ctime>
#include <fstream>
#include "DataBlock.hpp"

using namespace std;

typedef unordered_map<string, DataBlock*> CachedPathBlocksMap;
typedef set<DataBlock*, DataBlockCompare> CachedBlocksSet;

/**
 * @brief CacheData class represents a data structure containning all the cached
 * data of a fuse in order to save some reading from disk operations to save the user time.
 */
class CacheData
{
public:
    /**
     * @brief CacheData Constructor
     * @param rootdir - The root directory path
     * @param logFile - The log file's path
     * @param maxBlocksCount - the maximum amount of blocks allowed to cache
     * @param blockSize - the max size of each dataBlock
     * @param num - The num of dataBlock from the file
     */
    CacheData(const string rootDir, const string logFile, 
              const unsigned long setMaxBlocksCount, const unsigned int setBlockSize);
    
    /**
     * @brief CacheData Destructor - delete and clears all inner members
     */
    ~CacheData();
    
    /**
     * @param path - The path we wish to get it's real Path
     * @return The real of a realative path 
     */
    string stringRealPath(string path);
    
    /**
     * @param path - The path we wish to get it's absoulte Path
     * @return The absoulte of a relative path
     */
    string absolutePath(const char* path);
    
    /**
     * @param block - the block to add to cache
     * Adds to cache a new dataBlock
     */
    void pushDataBlock(DataBlock* block);
    
    /**
     * @param fh - the file handler
     * @param path - the path of the file the inforamtion is in
     * @param blockNum - the number of block in the file
     * @return A pointer to a dataBlock containning the requested data
     */
    DataBlock* readBlockFromDisk(uint64_t  fh, const string path, unsigned long blockNum);
    
    /**
     * Delete's the least used datablock in cache
     */
    void deleteLeastUsedBlock();
    
    /**
     * @param fh - the file handler
     * @param path - the path of the file the inforamtion is in
     * Renames all cached dataBlock's path
     */
    void renameCachedBlocks(const string oldPath, const string newPath);
    
    /**
     * @param action - the action to record
     * Log's in the requested action
     */  
    void log(string action);

    /* Map <file_path>:<block_number_in_file> of all cached blocks */
    CachedPathBlocksMap cachePathMap;
    /* Set of all cached blocks ordered by access count */
    CachedBlocksSet cacheFreqSet;
   
    /* Settings */
    const string rootDir; // Root dir path
    const string logFile; // Logfile path
    const unsigned long maxBlocksCount; // Maximum block number
    const unsigned int blockSize; // Block size
    
    /**
     * @return block's use count
     */
    unsigned long cacheSize();

private:
    CacheData();

    /**
     * @param block - the block we want its map key
     * @return The block's key
     */
    string _blockKey(DataBlock* block);
    
    // Current number of blocks in cache
    unsigned long _cacheSize;
};

#endif	/* CACHE_DATA_H */

