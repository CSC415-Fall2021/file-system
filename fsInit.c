/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
**************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsFree.h"
#include "fsDir.h"

//since we are using it in different functions, we declared it globally
VCB *mfs_vcb;

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	//printf("Initializing File System with %ld blocks with a block size of %ld\n",
	//numberOfBlocks, blockSize);

	mfs_blockSize = blockSize;
	mfs_defaultDECount = 50;

	//[Step 1] check if it's our volume
	//malloc space for VCB
	mfs_vcb = malloc(blockSize);
	//read the block to malloc space
	LBAread(mfs_vcb, 1, 0);
	//check signature
	int match = strcmp(mfs_vcb->signature, "101");

	//[Step 2A] If match, load up the bitmap to our memory and set the location to 1
	if (match == 0)
	{
		reloadFreeSpace(mfs_vcb, blockSize);
		mfs_cwd_location = mfs_vcb->rootLocation;
	}

	//[Step 2B] If not match, initialize the volume.
	else
	{
		//printf("[debug] not our volume!\n");

		//init first half of VCB
		strcpy(mfs_vcb->signature, "101");
		mfs_vcb->numberOfBlocks = numberOfBlocks;
		mfs_vcb->blockSize = blockSize;
		mfs_vcb->location = 0;

		//init free space
		int bitMapLocation = initFreeSpace(mfs_vcb);
		printf("[debug] bitMapLocation starts at %d\n", bitMapLocation);

		//init root
		int rootLocation = createDir(0); //not sure bout passing in 0...
		printf("[debug] rootLocation starts at %d\n", rootLocation);

		//set the root as current working directory
		mfs_cwd_location = rootLocation;
		printf("[debug] cwd location: %d\n", mfs_cwd_location);

		//init rest of the data of VCB
		mfs_vcb->rootSize = mfs_defaultDECount * sizeof(DE);
		mfs_vcb->bitMapLocation = bitMapLocation;
		mfs_vcb->rootLocation = rootLocation;
		mfs_vcb->freeSpaceStartLocation = rootLocation + mfs_vcb->rootSize;

		//write into disk
		if (LBAwrite(mfs_vcb, 1, 0) != 1)
		{
			printf("[ERROR] LBAwrite() failed...\n");
		}
	}

	//char *path = malloc(100);
	//strcpy(path, "foo/bar");

	//fs_mkdir(path, NULL);

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
	if (LBAwrite(mfs_vcb, 1, 0) != 1)
	{
		printf("[ERROR] LBAwrite() failed...\n");
	}
}