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


typedef struct volumeControlBlock{
		char VCBsignature[100];
		int numberOfBlocks;
		int blockSize;
		int location;
		int rootStartLocation;
		int freeSpaceStartLocation;
}volumeControlBlock;
		
typedef struct directoryEntry{
	int size;
	int location;
	char name[100];
}directoryEntry;

typedef struct freeSpace{ // might not need this struct, maybe just a helper function to init entry to free to be true
	unsigned free:1;
}freeSpace;



int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//assuming VCB hasn't been made yet
	char testSignature[100] = "101";

	// Initialize VCB
	printf("initializing VCB");
	volumeControlBlock VCB;
	memcpy(VCB.VCBsignature, testSignature, strlen(testSignature));
	printf("VCB's signature is %s\n", VCB.VCBsignature);
	VCB.numberOfBlocks = numberOfBlocks;
	VCB.blockSize = blockSize;
	VCB.location = 0; // Always starts at location 0
	VCB.rootStartLocation = 1;  // letting root start after VCB block spanning 10 blocks
	VCB.freeSpaceStartLocation = 10; // free space will start after root's block
	
	freeSpace freeSpaceTracker[numberOfBlocks];
	for(int i=0; i< VCB.freeSpaceStartLocation; i++){ // setting 0 for not free
		freeSpaceTracker[i].free = 0;
	}
	for(int i = VCB.freeSpaceStartLocation; i < numberOfBlocks; i++){ // setting 1 for free
		freeSpaceTracker[i].free = 1;
	}

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}