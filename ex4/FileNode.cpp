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

FileNode::FileNode ()
{
	
}

FileNode::~FileNode ()
{
	// TODO destroy blocks
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
	return (*_blocks.begin())->second->getUseCount();
}