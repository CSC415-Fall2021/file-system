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
#include "fsDir.c"

#define rootDECount 50 //number of blocks root contains

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	//1. check if it's our volume
	//   malloc space for VCB
	VCB *vcb = malloc(blockSize);
	//   read the block to malloc space
	LBAread(vcb, 1, 0);
	//   check signature
	int match = strcmp(vcb->signature, "101");

	//setting up the free space management.
	//only bitmap will be different
	freeSpaceManager *manager = malloc(sizeof(freeSpaceManager));
	manager->totalOfBlock = numberOfBlocks;
	manager->blockSize = blockSize;
	manager->blockRemains = numberOfBlocks;
	manager->location = 1;

	if (match == 0)
	{
		//load up the bitmap to our memory and set the location to 1
		LBAread(manager->bitMap, 5, manager->location);
	}

	//if not match, initialize the volume
	else
	{
		printf("[debug] not our volume!\n");
		//init free space
		manager->bitMap = malloc(5 * blockSize);
		int bitMapLocation = initFreeSpace(manager);

		//init root
		int rootLocation = createDir(0, rootDECount, blockSize, manager); //not sure bout passing in 0...

		//init VCB
		strcpy(vcb->signature, "101");
		vcb->numberOfBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;
		vcb->location = 0;
		vcb->bitMapLocation = bitMapLocation;
		vcb->rootLocation = rootLocation;
		vcb->freeSpaceStartLocation = rootLocation + 12; //hardcoded

		//write into disk
		LBAwrite(vcb, 1, 0);
	}

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}