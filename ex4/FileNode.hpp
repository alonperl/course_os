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
typedef map<int, DataBlock*, DataBlockComparator> BlockMap;

using namespace std;

class FileNode {
public:
    FileNode();
    virtual ~FileNode();
    
    unsigned int getLowestFrequency();
    BlockMap blocks;
};

#endif	/* FILENODE_H */

