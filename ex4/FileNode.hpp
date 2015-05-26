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
    
    struct fuse_file_info *getFileInfo();
    char *getPath();
    unsigned int getOpenCount();
    
private:
    struct fuse_file_info *_fi;
    char *_path;
    unsigned int _openCount;
    
    set<DataBlock*, DataBlockComparator> _blocks;
};

#endif	/* FILENODE_H */

