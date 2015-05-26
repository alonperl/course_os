/*
 * CachingFileSystem.cpp
 *
 *  Created on: 15 April 2015
 *  Author: Netanel Zakay, HUJI, 67808  (Operating Systems 2014-2015).
 */

#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <errno.h>
#include <iostream>
#include <unistd.h>
#include <dirent.h>
#include <ctime>
#include <fstream>

#include "CacheData.hpp"

#define CACHE_DATA ((CacheData*) fuse_get_context()->private_data)
 

#define RIGHT_PARAM_AMOUNT 5
#define SUCCESS 0
#define ROOT_DIR 1
#define MOUNT_DIR 2
#define BLOCKS_NUMBER 3
#define BLOCK_SIZE 4

#define USAGE_ERROR "usage: CachingFileSystem rootdir mountdir numberOfBlocks blockSize\n"


using namespace std;

struct fuse_operations caching_oper;

/**
 * 
 * @param message
 * @return 
 */
void log(const char* action)
{
	ofstream logStream(CACHE_DATA->getMount(), ios_base::app);
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

// /**
//  * Get absolute path for given relative path from mountdir.
//  *
//  * @param absolutePath - container to put new the result into
//  * @param path - relative path.
//  * @return -errno if the absolute path exceeds PATH_MAX, 0 otherwise
//  */
// int absolutePath(char[PATH_MAX] absolutePath, char* path)
// {
// 	strcpy(absolutePath, CACHE_DATA->getMount());

// 	if (strlen(absolutePath) + strlen(path) > PATH_MAX)
// 	{
// 		return -ENAMETOOLONG;
// 	}
	
// 	strncat(absolutePath, path, PATH_MAX);

// 	return 0;
// }

/** Get file attributes.
 *
 * Similar to stat().  The 'st_dev' and 'st_blksize' fields are
 * ignored.  The 'st_ino' field is ignored except if the 'use_ino'
 * mount option is given.
 */
int caching_getattr(const char *path, struct stat *statbuf)
{
	cout<<__FUNCTION__<<endl;
	
	int result = 0;

	char* absPath = realpath(path, NULL);
	if (absPath == NULL)
	{
		if (errno != ENOMEM)
		{
			free(absPath);
		}

		result = -errno;
	}
	else
	{
		result = lstat(absPath, statbuf);

		free(absPath);
	}

	return result;
}

/**
 * Get attributes from an open file
 *
 * This method is called instead of the getattr() method if the
 * file information is available.
 *
 * Currently this is only called after the create() method if that
 * is implemented (see above).  Later it may be called for
 * invocations of fstat() too.
 *
 * Introduced in version 2.5
 */
int caching_fgetattr(const char *path, struct stat *statbuf, struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;

	int result = fstat(fi->fh, statbuf);

	return result;
}

/**
 * Check file access permissions
 *
 * This will be called for the access() system call.  If the
 * 'default_permissions' mount option is given, this method is not
 * called.
 *
 * This method is not called under Linux kernel versions 2.4.x
 *
 * Introduced in version 2.5
 */
int caching_access(const char *path, int mask)
{
	char* absPath = realpath(path, NULL);
	int accessStats = access(absPath, mask);
	if (accessStats != 0)
	{
		return -errno;
	}
	return accessStats;
}



/** File open operation
 *
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC)
 * will be passed to open().  Open should check if the operation
 * is permitted for the given flags.  Optionally open may also
 * return an arbitrary filehandle in the fuse_file_info structure,
 * which will be passed to all file operations.

 * pay attention that the max allowed path is PATH_MAX (in limits.h).
 * if the path is longer, return error.

 * Changed in version 2.2
 */
int caching_open(const char *path, struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;
	// what TODO when opening same file twice or more

	int result = 0;

	char* absPath = realpath(path, NULL);
	if (absPath == NULL)
	{
		if (errno != ENOMEM)
		{
			free(absPath);
		}

		result = -errno;
	}
	else
	{
		int fd = open(absPath, fi->flags);

		if (fd < 0)
		{
			result = -errno;
		}
		else
		{
			fi->fh = fd;
		}

		free(absPath);
	}

	return result;
}


/** Read data from an open file
 *
 * Read should return exactly the number of bytes requested except
 * on EOF or error, otherwise the rest of the data will be
 * substituted with zeroes. 
 *
 * Changed in version 2.2
 */
int caching_read(const char *path, char *buf, size_t size, off_t offset,
		struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;

	int result = pread(fi->fh, buf, size, offset);
	if (result < 0)
	{
		result = -errno;
	}

	return result;
}

/** Possibly flush cached data
 *
 * BIG NOTE: This is not equivalent to fsync().  It's not a
 * request to sync dirty data.
 *
 * Flush is called on each close() of a file descriptor.  So if a
 * filesystem wants to return write errors in close() and the file
 * has cached dirty data, this is a good place to write back data
 * and return any errors.  Since many applications ignore close()
 * errors this is not always useful.
 *
 * NOTE: The flush() method may be called more than once for each
 * open().  This happens if more than one file descriptor refers
 * to an opened file due to dup(), dup2() or fork() calls.  It is
 * not possible to determine if a flush is final, so each flush
 * should be treated equally.  Multiple write-flush sequences are
 * relatively rare, so this shouldn't be a problem.
 *
 * Filesystems shouldn't assume that flush will always be called
 * after some writes, or that if will be called at all.
 *
 * Changed in version 2.2
 */
int caching_flush(const char *path, struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;
	log("flush");
    return SUCCESS;
}

/** Release an open file
 *
 * Release is called when there are no more references to an open
 * file: all file descriptors are closed and all memory mappings
 * are unmapped.
 *
 * For every open() call there will be exactly one release() call
 * with the same flags and file descriptor.  It is possible to
 * have a file opened more than once, in which case only the last
 * release will mean, that no more reads/writes will happen on the
 * file.  The return value of release is ignored.
 *
 * Changed in version 2.2
 */
int caching_release(const char *path, struct fuse_file_info *fi)
{
	log("release");
	int result = close(fi->fh);
	return result;
}

/** Open directory
 *
 * This method should check if the open operation is permitted for
 * this  directory
 *
 * Introduced in version 2.3
 */
int caching_opendir(const char *path, struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;

	int result = 0;

	char* absPath = realpath(path, NULL);
	if (absPath == NULL)
	{
		if (errno != ENOMEM)
		{
			free(absPath);
		}

		result = -errno;
	}
	else
	{
		DIR *dirPointer = opendir(absPath);

		if (dirPointer == NULL)
		{
			result = -errno;
		}

		fi->fh = (intptr_t)dirPointer;

		free(absPath);
	}

	return result;
}

/** Read directory
 *
 * This supersedes the old getdir() interface.  New applications
 * should use this.
 *
 * The readdir implementation ignores the offset parameter, and
 * passes zero to the filler function's offset.  The filler
 * function will not return '1' (unless an error happens), so the
 * whole directory is read in a single readdir operation.  This
 * works just like the old getdir() method.
 *
 * Introduced in version 2.3
 */
int caching_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset,
		struct fuse_file_info *fi)
{
  // TODO why do we need fi? this is a directory.
	cout<<__FUNCTION__<<endl;

	int result = 0;

	DIR *dirPointer = (DIR*)(uintptr_t)fi->fh;
	struct dirent *dirEntry;
	dirEntry = readdir(dirPointer);

	if (dirEntry == NULL)
	{
		result = -errno;
	}
	else
	{
	    do 
	    {
			if (filler(buf, dirEntry->d_name, NULL, 0) != 0)
			{
			    result = -errno; // TODO does filler update errno? if not - return -ENOMEM
			    break;
			}
	    } while ((dirEntry = readdir(dirPointer)) != NULL);
	}

	return result;
}

/** Release directory
 *
 * Introduced in version 2.3
 */
int caching_releasedir(const char *path, struct fuse_file_info *fi)
{
	cout<<__FUNCTION__<<endl;

    closedir((DIR *)(uintptr_t)fi->fh);

	return 0;
}

/** Rename a file */
int caching_rename(const char *path, const char *newpath)
{
	cout<<__FUNCTION__<<endl;
	return 0;
}

/**
 * Initialize filesystem
 *
 * The return value will passed in the private_data field of
 * fuse_context to all file operations and as a parameter to the
 * destroy() method.
 *
 * Introduced in version 2.3
 * Changed in version 2.6
 */
void *caching_init(struct fuse_conn_info *conn)
{
	cout<<__FUNCTION__<<endl;
	return CACHE_DATA;
}


/**
 * Clean up filesystem
 *
 * Called on filesystem exit.
 *
 * Introduced in version 2.3
 */
void caching_destroy(void *userdata)
{
	cout<<__FUNCTION__<<endl;
}


/**
 * Ioctl from the FUSE sepc:
 * flags will have FUSE_IOCTL_COMPAT set for 32bit ioctls in
 * 64bit environment.  The size and direction of data is
 * determined by _IOC_*() decoding of cmd.  For _IOC_NONE,
 * data will be NULL, for _IOC_WRITE data is out area, for
 * _IOC_READ in area and if both are set in/out area.  In all
 * non-NULL cases, the area is of _IOC_SIZE(cmd) bytes.
 *
 * However, in our case, this function only needs to print cache table to the log file.
 * 
 * Introduced in version 2.8
 */
int caching_ioctl (const char *, int cmd, void *arg,
		struct fuse_file_info *, unsigned int flags, void *data)
{
	//print to log:
	//log()
	//1 2 3
	//1 name of file relative to mountdir
	//2 number of the block in the enumartion itself
	//3 number of time it was accessed

	log("ioctl");
	return 0;
	//TODO iterates through all fileNodes and blocks
}


// Initialise the operations. 
// You are not supposed to change this function.
void init_caching_oper()
{
	caching_oper.getattr = caching_getattr;
	caching_oper.access = caching_access;
	caching_oper.open = caching_open;
	caching_oper.read = caching_read;
	caching_oper.flush = caching_flush;
	caching_oper.release = caching_release;
	caching_oper.opendir = caching_opendir;
	caching_oper.readdir = caching_readdir;
	caching_oper.releasedir = caching_releasedir;
	caching_oper.rename = caching_rename;
	caching_oper.init = caching_init;
	caching_oper.destroy = caching_destroy;
	caching_oper.ioctl = caching_ioctl;
	caching_oper.fgetattr = caching_fgetattr;

	caching_oper.readlink = NULL;
	caching_oper.getdir = NULL;
	caching_oper.mknod = NULL;
	caching_oper.mkdir = NULL;
	caching_oper.unlink = NULL;
	caching_oper.rmdir = NULL;
	caching_oper.symlink = NULL;
	caching_oper.link = NULL;
	caching_oper.chmod = NULL;
	caching_oper.chown = NULL;
	caching_oper.truncate = NULL;
	caching_oper.utime = NULL;
	caching_oper.write = NULL;
	caching_oper.statfs = NULL;
	caching_oper.fsync = NULL;
	caching_oper.setxattr = NULL;
	caching_oper.getxattr = NULL;
	caching_oper.listxattr = NULL;
	caching_oper.removexattr = NULL;
	caching_oper.fsyncdir = NULL;
	caching_oper.create = NULL;
	caching_oper.ftruncate = NULL;
}


bool checkArgs(int argc, char* argv[])
{
	// Check correct param amount
	if (argc != RIGHT_PARAM_AMOUNT)
	{
		return false;
	}

	// Check if paths exists
	struct stat rootStatBuf;
	struct stat mountStatBuf;
	int isRootExists = 0, isMountExists = 0;

	char* absRootPath = realpath(argv[ROOT_DIR], NULL);
	if (absRootPath == NULL)
	{
		if (errno != ENOMEM)
		{
			free(absRootPath);
		}

		return false;
	}
	else
	{
		isRootExists = stat(absRootPath, &rootStatBuf);
		free(absRootPath);
	}

	char* absMountPath = realpath(argv[MOUNT_DIR], NULL);
	if (absMountPath == NULL)
	{
		if (errno != ENOMEM)
		{
			free(absMountPath);
		}

		return false;
	}
	else
	{
		isMountExists = stat(absMountPath, &mountStatBuf);
		free(absMountPath);
	}

	if (isRootExists != 0 || isMountExists != 0)
	{
		return false;
	}

	// Check if blockSize & numberOfBlocks are positive int
	if (!(argv[BLOCKS_NUMBER] > 0 && argv[BLOCK_SIZE] > 0))
	{
		return false;
	}

	return true;
}


int main(int argc, char* argv[])
{ 
	// Checking the received parameters
	if(!checkArgs(argc, argv))
	{
		cout << USAGE_ERROR;
		exit(1);
	}

	CacheData *cacheData = new CacheData(argv[ROOT_DIR], argv[MOUNT_DIR], atoi(argv[BLOCK_SIZE]), atoi(argv[BLOCKS_NUMBER]));
	
	init_caching_oper();
	argv[1] = argv[2];
	for (int i = 2; i< (argc - 1); i++){
		argv[i] = NULL;
	}
        argv[2] = (char*) "-s";
        argv[3] = (char*) "-f";
	argc = 4;

	int fuse_stat = fuse_main(argc, argv, &caching_oper, cacheData);
	return fuse_stat;
}
