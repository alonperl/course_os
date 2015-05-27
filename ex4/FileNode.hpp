/* 
 * File:   FileNode.h
 * Author: griffonn
 *
 * Created on 24 May 2015, 22:40
 */

#ifndef FILENODE_H
#define	FILENODE_H

#include <set>
#include "DataBlock.hpp"

using namespace std;

class FileNode {
public:
    FileNode();
    virtual ~FileNode();
    
    char *getPath();
    unsigned int getOpenCount();
    unsigned int getLowestFrequency();
    unsigned int getBlocksIterator();
    
private:
    char *_path;
    unsigned int _openCount;
    
    set<DataBlock*, DataBlockComparator> _blocks;
};

#endif	/* FILENODE_H */

