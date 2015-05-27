/* 
 * File:   FileNode.cpp
 * Author: griffonn
 * 
 * Created on 24 May 2015, 22:40
 */

#include "FileNode.hpp"
#include <cstring>
#include <errno.h>
#include <malloc.h>

FileNode::FileNode (char* path)
{
	int pathLength = strlen(path);
	if (pathLength == 0)
	{
		throw -ENOENT;
	}

	_path = (char*)malloc(sizeof(char) * pathLength);
	strcpy(_path, path);
}

FileNode::~FileNode ()
{
	// TODO destroy blocks
	free(_path);
}

char *FileNode::getPath()
{
	return _path;
}

unsigned int FileNode::getOpenCount()
{
	return _openCount;
}

unsigned int FileNode::getLowestFrequency()
{
	return (*_blocks.begin())->getUseCount();
}

set<DataBlock*, DataBlockComparator>::iterator FileNode::getBlocksIterator()
{
	return _blocks.begin();
}