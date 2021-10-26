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
#include "fsFree.c"
#include "fsDir.c"

#define rootDECount 50 //number of blocks root contains

VCB *vcb;

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	//1. check if it's our volume
	//   malloc space for VCB
	vcb = malloc(blockSize);
	//   read the block to malloc space
	LBAread(vcb, 1, 0);
	//   check signature
	int match = strcmp(vcb->signature, "101");

	if (match == 0)
	{
		//load up the bitmap to our memory and set the location to 1
		reloadFreeSpace(vcb, blockSize);
	}

	//if not match, initialize the volume
	else
	{
		printf("[debug] not our volume!\n");

		//init first half of VCB
		strcpy(vcb->signature, "101");
		vcb->numberOfBlocks = numberOfBlocks;
		vcb->blockSize = blockSize;
		vcb->location = 0;

		//init free space
		int bitMapLocation = initFreeSpace(vcb, blockSize);

		//init root
		int rootLocation = createDir(0, rootDECount, blockSize, vcb); //not sure bout passing in 0...

		//init rest of the data of VCB
		vcb->bitMapLocation = bitMapLocation;
		vcb->rootLocation = rootLocation;
		vcb->freeSpaceStartLocation = rootLocation + vcb->rootSize;

		//write into disk
		LBAwrite(vcb, 1, 0);
	}

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
	LBAwrite(vcb, 1, 0);
}