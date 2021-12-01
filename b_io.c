/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: b_io.c
*
* Description: Basic File System - Key File I/O Operations
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "fsDir.h"
#include "mfs.h"
#include "fsLow.h"

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define mfs_default_file_block 1
#define mfs_extend_block_count 5

typedef struct b_fcb
{
	/** TODO add al the information you need in the file control block **/
	DE *fi;
	char *buf; //holds the open file buffer
	int bufIndex;
	int bufLen;
	int currentBlock;
	int accessMode;
} b_fcb;

b_fcb fcbArray[MAXFCBS];

int startup = 0; //Indicates that this has not been initialized

//Method to initialize our file system
void b_init()
{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
	{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
	}

	startup = 1;
}

//Method to get a free FCB element
b_io_fd b_getFCB()
{
	for (int i = 0; i < MAXFCBS; i++)
	{
		if (fcbArray[i].buf == NULL)
		{
			return i; //Not thread safe (But do not worry about it for this assignment)
		}
	}
	return (-1); //all in use
}

// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open(char *filename, int flags)
{
	printf("---------- INSIDE THE b_open() ----------\n");
	b_io_fd returnFd;

	//*** TODO ***:  Modify to save or set any information needed
	//
	//

	if (startup == 0)
		b_init(); //Initialize our system

	returnFd = b_getFCB(); // get our own file descriptor
						   // check for error - all used FCB's

	//1 malloc space for tempWorkingDir and lastToken
	int mallocSize = sizeof(DE) * mfs_defaultDECount;
	int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
	mallocSize = numOfBlockNeeded * mfs_blockSize;
	DE *tempWorkingDir = malloc(mallocSize);
	char *lastToken = malloc(256);

	//2 pathparser
	bool valid = pathParser(filename, EXIST_FILE, tempWorkingDir, lastToken);
	if (!valid && flags | O_CREAT)
	{
		printf("[debug] has O_CREAT flag and invalid, so creating new file...\n");
		int LBAindex = allocateFreeSpace(mfs_default_file_block);
		int DEindex = -1;
		for (int i = 2; i < mfs_defaultDECount; i++)
		{
			if (strcmp(tempWorkingDir[i].name, "\0") == 0)
			{
				DEindex = i;
				break;
			}
		}
		if (DEindex == -1)
		{
			printf("[ERROR] b_open: directory does not have enough space\n");
			return -1;
		}

		// printf("[debug] !!!!!!!found empty DEindex at %d\n", DEindex);

		strcpy(tempWorkingDir[DEindex].name, lastToken);
		// printf("[debug] name at %d is %s\n", DEindex, tempWorkingDir[DEindex].name);
		tempWorkingDir[DEindex].size = mfs_default_file_block * mfs_blockSize;
		tempWorkingDir[DEindex].actualSize = 0;
		tempWorkingDir[DEindex].blockCount = mfs_default_file_block;
		tempWorkingDir[DEindex].location = LBAindex;
		tempWorkingDir[DEindex].isDir = 0;
		time(&tempWorkingDir[DEindex].createTime);
		time(&tempWorkingDir[DEindex].lastModTime);
		time(&tempWorkingDir[DEindex].lastAccessTime);

		char *buffer = malloc(tempWorkingDir[DEindex].size);
		// printf("[debug] buf: %s\n", buffer);
		LBAwrite(buffer, tempWorkingDir[DEindex].blockCount, tempWorkingDir[DEindex].location);
		LBAwrite(tempWorkingDir, tempWorkingDir[0].blockCount, tempWorkingDir[0].location);
		free(buffer);
		buffer = NULL;
	}

	//3
	int FCBindex = b_getFCB();
	if (FCBindex == -1)
	{
		printf("[ERROR] b_open(): no more space in fcb array\n");
		return -1;
	}

	//4
	int DEindex = -1;
	// printf("[debug] finding %s\n", lastToken);
	for (int i = 2; i < mfs_defaultDECount; i++)
	{
		// printf("[debug] current name at %d is %s\n", i, tempWorkingDir[i].name);
		if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
		{
			DEindex = i;
			break;
		}
	}
	if (DEindex == -1)
	{
		printf("[ERROR] b_open(): DE does not exist\n");
		return -1;
	}

	// printf("[debug] found DEindex at %d\n", DEindex);

	fcbArray[FCBindex].fi = &tempWorkingDir[DEindex]; //unless we don't change
	fcbArray[FCBindex].buf = malloc(B_CHUNK_SIZE);
	fcbArray[FCBindex].bufIndex = 0;
	fcbArray[FCBindex].bufLen = 0;
	fcbArray[FCBindex].currentBlock = 0;
	fcbArray[FCBindex].accessMode = flags;

	//5
	if (flags & O_TRUNC)
	{
		printf("[debug] flags has O_TRUNC\n");
		char *emptyBuf = malloc(fcbArray[FCBindex].fi->actualSize);
		LBAwrite(emptyBuf, fcbArray[FCBindex].fi->blockCount, fcbArray[FCBindex].fi->location);
		fcbArray[FCBindex].fi->actualSize = 0;
		fcbArray[FCBindex].bufIndex = 0;
		LBAwrite(tempWorkingDir, tempWorkingDir[0].blockCount, tempWorkingDir[0].location);
		free(emptyBuf);
		emptyBuf = NULL;
	}

	if (flags & O_APPEND)
	{
		printf("[debug] flags has O_APPEND\n");
		b_seek(FCBindex, fcbArray[FCBindex].bufIndex, SEEK_END);
	}

	printf("[debug] printing out DE info\n");
	printDEInfo(tempWorkingDir[DEindex]);

	//6
	free(tempWorkingDir);
	tempWorkingDir = NULL;
	free(lastToken);
	lastToken = NULL;

	printf("---------- END OF THE b_open() ----------\n");

	return (returnFd); // all set
}

// Interface to seek function
int b_seek(b_io_fd fd, off_t offset, int whence)
{
	printf("---------- INSIDE OF THE b_seek() ----------\n");

	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	if (whence & SEEK_SET)
	{
		printf("[debug] SEEK_SET\n");
		fcbArray[fd].bufIndex = offset;
		printf("[debug] offset: %d\n", fcbArray[fd].bufIndex);
	}

	if (whence & SEEK_CUR)
	{
		printf("[debug] SEEK_CUR\n");
		fcbArray[fd].bufIndex += offset;
		printf("[debug] offset: %d\n", fcbArray[fd].bufIndex);
	}

	if (whence & SEEK_END)
	{
		printf("[debug] SEEK_SET\n");
		fcbArray[fd].bufIndex = fcbArray[fd].fi->actualSize / B_CHUNK_SIZE;
		printf("[debug] offset: %d\n", fcbArray[fd].bufIndex);
	}

	printf("---------- END OF THE b_seek() ----------\n");

	return (0); //Change this
}

// Interface to write function
int b_write(b_io_fd fd, char *buffer, int count)
{
	printf("---------- INSIDE OF THE b_write() ----------\n");

	int bufRemains, delieveredBytes, bytesWrite, bytesReturned;
	int part1, part2, part3, transferBlocks;

	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	//1 check if we have the write mode
	if (!(fcbArray[fd].accessMode | O_RDWR || fcbArray[fd].accessMode | O_WRONLY))
	{
		printf("[ERROR] does not have access to write!\n");
		return -1;
	}

	//2 check if we have enough space for count
	if (fcbArray[fd].fi->actualSize + count > fcbArray[fd].fi->size)
	{
		printf("[debug] not enough space, reallocating the memory location!\n");
		int LBAindex = allocateFreeSpace(fcbArray[fd].fi->blockCount + mfs_extend_block_count);
		char *extendBuffer = malloc((fcbArray[fd].fi->blockCount + mfs_extend_block_count) * mfs_blockSize);
		LBAread(extendBuffer, fcbArray[fd].fi->blockCount, fcbArray[fd].fi->location);
		// printf("[debug] after lba read's buffer: %s\n", extendBuffer);

		int mallocSize = sizeof(DE) * mfs_defaultDECount;
		int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
		mallocSize = numOfBlockNeeded * mfs_blockSize;
		DE *tempWorkingDir = malloc(mallocSize);
		char *lastToken = malloc(256);

		bool valid = pathParser(fcbArray[fd].fi->name, EXIST_FILE, tempWorkingDir, lastToken);
		if (!valid)
		{
			printf("[ERROR] b_write: invalid path\n");
			return -1;
		}

		int DEindex = -1;
		for (int i = 2; i < mfs_defaultDECount; i++)
		{
			if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
			{
				DEindex = i;
				break;
			}
		}

		if (DEindex == -1)
		{
			printf("[ERROR] DE does not exist!");
			return -1;
		}

		tempWorkingDir[DEindex].location = LBAindex;
		tempWorkingDir[DEindex].blockCount += mfs_extend_block_count;
		tempWorkingDir[DEindex].size = tempWorkingDir[DEindex].blockCount * mfs_blockSize;

		fcbArray[fd].currentBlock += LBAindex - fcbArray[fd].fi->location;
		fcbArray[fd].fi->location = LBAindex;
		fcbArray[fd].fi->blockCount += mfs_extend_block_count;
		fcbArray[fd].fi->size = fcbArray[fd].fi->blockCount * mfs_blockSize;

		printf("[debug] print out DE info\n");
		printDEInfo(tempWorkingDir[DEindex]);

		free(tempWorkingDir);
		tempWorkingDir = NULL;
		free(lastToken);
		lastToken = NULL;
		free(buffer);
		buffer = NULL;
	}

	//3 calculate the bufRemains
	bufRemains = B_CHUNK_SIZE - fcbArray[fd].bufIndex;
	// printf("[debug] bufRemains: %d\n", bufRemains);

	//4 calculate part1, 2, 3
	if (count <= bufRemains)
	{
		part1 = count;
		part2 = 0;
		part3 = 0;
	}
	else
	{
		part1 = bufRemains;
		part3 = count - bufRemains;
		transferBlocks = part3 / mfs_blockSize;
		part2 = transferBlocks * mfs_blockSize;
		part3 = part3 - part2;
	}

	// printf("[debug] part1: %d & part2: %d & part3: %d\n", part1, part2, part3);

	//5 part1
	if (part1 > 0)
	{
		printf("[debug] inside part 1\n");
		memcpy(fcbArray[fd].buf, buffer, part1);
		fcbArray[fd].bufIndex += part1;
		// printf("[debug] my current buffer: %s\n", fcbArray[fd].buf);
		if (fcbArray[fd].bufIndex == B_CHUNK_SIZE - 1)
		{
			// printf("[debug] block is full, need to flush!\n");
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fi->location);
			fcbArray[fd].currentBlock += 1;
			fcbArray[fd].bufIndex = 0;
			fcbArray[fd].fi->actualSize += B_CHUNK_SIZE;
		}
	}

	//6 part2
	if (part2 > 0)
	{
		printf("[debug] inside part 2\n");
		LBAwrite(buffer + part1, transferBlocks, fcbArray[fd].currentBlock + fcbArray[fd].fi->location);
		fcbArray[fd].currentBlock += transferBlocks;
		fcbArray[fd].fi->actualSize += part2;
	}

	//7 part3
	if (part3 > 0)
	{
		printf("[debug] inside part 3\n");
		memcpy(fcbArray[fd].buf, buffer + part1 + part2, part3);
		fcbArray[fd].bufIndex += part3;
		if (fcbArray[fd].bufIndex == B_CHUNK_SIZE - 1)
		{
			// printf("[debug] block is full\n");
			LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fi->location);
			fcbArray[fd].fi->actualSize += B_CHUNK_SIZE;
		}
	}

	//8 calculate returned byte
	bytesReturned = part1 + part2 + part3;
	// printf("[debug] bytesReturned: %d\n", bytesReturned);
	printf("[debug] printout DE info\n");
	printDEInfo(*fcbArray[fd].fi);

	printf("---------- END OF THE b_write() ----------\n");

	return (0); //Change this
}

// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read(b_io_fd fd, char *buffer, int count)
{
	printf("---------- INSIDE OF THE b_read() ----------\n");
	printf("[debug] count: %d\n", count);

	int bufRemains, delieveredBytes, bytesRead, bytesReturned;
	int part1, part2, part3, transferBlocks;

	if (startup == 0)
		b_init(); //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
	{
		return (-1); //invalid file descriptor
	}

	//1 check if the access mode has read
	if (!(fcbArray[fd].accessMode | O_RDONLY || fcbArray[fd].accessMode | O_RDWR))
	{
		printf("[ERROR] does not have access to read!\n");
		return -1;
	}

	//2 calculate the bufRemains
	bufRemains = fcbArray[fd].bufLen - fcbArray[fd].bufIndex;
	printf("[debug] bufRemains = %d - %d = %d\n", fcbArray[fd].bufLen, fcbArray[fd].bufIndex, bufRemains);

	//3 calculate delivered bytes to check EOF
	delieveredBytes = fcbArray[fd].currentBlock * B_CHUNK_SIZE - bufRemains;

	//4 update the count if user asking more than we have
	if ((count + delieveredBytes) > fcbArray[fd].fi->actualSize)
	{
		count = fcbArray[fd].fi->actualSize - delieveredBytes;

		//5 safety check
		if (count < 0)
		{
			printf("[ERROR] b_read(): count is acting weird...\n");
			return -1;
		}
	}

	printf("[debug] count: %d\n", count);
	//6 calculate part1, 2, 3
	if (count <= bufRemains)
	{
		part1 = count;
		part2 = 0;
		part3 = 0;
	}
	else
	{
		part1 = bufRemains;
		part3 = count - bufRemains;
		transferBlocks = part3 / B_CHUNK_SIZE;
		part2 = transferBlocks * mfs_blockSize;
		part3 = part3 - part2;
	}

	printf("[debug] part1: %d & part2: %d & part3: %d\n", part1, part2, part3);

	//7 part 1
	if (part1 > 0)
	{
		printf("[debug] inside part 1\n");
		memcpy(buffer, fcbArray[fd].buf, part1);
		fcbArray[fd].bufIndex += part1;
		printf("[debug] bufIndex: %d\n", fcbArray[fd].bufIndex);
		printf("[debug] user's buf: %s\n", buffer);
	}

	//8 part 2
	if (part2 > 0)
	{
		printf("[debug] inside part 2\n");
		bytesRead = LBAread(buffer + part1, transferBlocks, fcbArray[fd].currentBlock + fcbArray[fd].fi->location);
		fcbArray[fd].currentBlock += transferBlocks;
		bytesRead = bytesRead * B_CHUNK_SIZE;
		part2 = bytesRead;
		printf("[debug] bufIndex: %d\n", fcbArray[fd].bufIndex);
		printf("[debug] currentBlock: %d\n", fcbArray[fd].currentBlock);
		printf("[debug] user's buf: %s\n", buffer);
	}

	//9 part 3
	if (part3 > 0)
	{
		printf("[debug] inside part 3\n");
		printf("[debug] before doing LBAread, my buffer: %s\n", fcbArray[fd].buf);
		bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].currentBlock + fcbArray[fd].fi->location);
		bytesRead = bytesRead * mfs_blockSize;
		fcbArray[fd].currentBlock += 1;
		fcbArray[fd].bufIndex = 0;
		fcbArray[fd].bufLen = bytesRead;
		printf("[debug] bytesRead: %d\n", bytesRead); //TODO
		printf("[debug] our current buffer after LBAread: %s\n", fcbArray[fd].buf);
		if (bytesRead < part3)
		{
			part3 = bytesRead;
		}

		if (part3 > 0)
		{
			memcpy(buffer + part1 + part2, fcbArray[fd].buf, part3);
			fcbArray[fd].bufIndex += part3;
		}

		printf("[debug] bufIndex: %d\n", fcbArray[fd].bufIndex);
		printf("[debug] currentBlock: %d\n", fcbArray[fd].currentBlock);
		printf("[debug] user's buf: %s\n", buffer);
	}

	//10 calculate returned bytes
	bytesReturned = part1 + part2 + part3;
	printf("[debug] printout DE info\n");
	printDEInfo(*fcbArray[fd].fi);

	printf("---------- END OF THE b_read() ----------\n");

	return (bytesReturned); //Change this
}

// Interface to Close the file
void b_close(b_io_fd fd)
{
	printf("---------- INSIDE OF THE b_close() ----------\n");
	// printf("[debug] bufIndex: %d\n", fcbArray[fd].bufIndex);
	printf("[debug] printout DE info\n");
	printDEInfo(*fcbArray[fd].fi);

	int tempActualSize;
	if (fcbArray[fd].bufIndex < B_CHUNK_SIZE && fcbArray[fd].accessMode | O_WRONLY)
	{
		printf("[debug] doing the last LBAwrite with buf: %s\n", fcbArray[fd].buf);
		LBAwrite(fcbArray[fd].buf, 1, fcbArray[fd].fi->location + fcbArray[fd].currentBlock);
		fcbArray[fd].fi->actualSize += fcbArray[fd].bufIndex;
		tempActualSize = fcbArray[fd].fi->actualSize;
		printf("[debug] actual size is %d\n", fcbArray[fd].fi->actualSize);
	}

	int mallocSize = sizeof(DE) * mfs_defaultDECount;
	int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
	mallocSize = numOfBlockNeeded * mfs_blockSize;
	DE *tempWorkingDir = malloc(mallocSize);
	char *lastToken = malloc(256);
	bool valid = pathParser(fcbArray[fd].fi->name, EXIST_FILE, tempWorkingDir, lastToken);

	if (!valid)
	{
		printf("[ERROR] b_close: invalid path\n");
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		fcbArray[fd].fi = NULL;
		return;
	}
	int DEindex = -1;
	for (int i = 2; i < mfs_defaultDECount; i++)
	{
		if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
		{
			DEindex = i;
			break;
		}
	}

	if (DEindex == -1)
	{
		printf("[ERROR] b_close: cannot find DEindex\n");
		free(fcbArray[fd].buf);
		fcbArray[fd].buf = NULL;
		fcbArray[fd].fi = NULL;
		return;
	}

	printf("[debug] !actual size is %d\n", tempActualSize);
	tempWorkingDir[DEindex].actualSize = tempActualSize; //TODO
	printf("[debug] print DE info\n");
	printDEInfo(tempWorkingDir[DEindex]);
	LBAwrite(tempWorkingDir, tempWorkingDir[0].blockCount, tempWorkingDir[0].location);

	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;
	fcbArray[fd].fi = NULL;
	printf("---------- END OF THE b_close() ----------\n");
}