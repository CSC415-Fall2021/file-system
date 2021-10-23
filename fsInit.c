/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsFree.c"

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);

	//1. determine if we need to format the volume or not
	//		malloc a block of memory as our VCB pointer and LBAread block 0
	//		compare the signature in our struct with the one in block 0
	//		if same -> no need to do init (skip step 2)
	//		if not  -> init(jump to step 2)

	//2. initilizing our volume
	//		initialize the values in our VCB
	//		initialize our free space
	//		- malloc a bit map buffer for free space management
	//		- call initFreeSpace
	//		- LBAwrite the bit map to disk (block #1 - #5)
	unsigned char *bitMap = (unsigned char *)malloc(5 * 512); //[hardcode]
	initFreeSpace(bitMap);
	LBAwrite(bitMap, 5, 1); //[hardcode]

	//		initialize our root directory
	//		set the returned values of free space and root directory in VCB
	//		LBAwrite the VCB to block 0

	//Freeing malloced items
	free(bitMap);
	bitMap = NULL;

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}