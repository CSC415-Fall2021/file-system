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
		int freeSpaceBitmapLocation;
		int freeSpaceBitmapEndLocation;
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
	volumeControlBlock *VCB = malloc(blockSize);

	// need to do LBAread(); here to check VCB's signature



	// if VCB signature doesn't match we need to intialize
	memcpy(VCB->VCBsignature, testSignature, 4);
	printf("VCB's signature is %s\n", VCB->VCBsignature);
	VCB->numberOfBlocks = numberOfBlocks;
	VCB->blockSize = blockSize;
	VCB->location = 0; // Always starts at location 0
	// freeSpaceBitMap will take in   
	// [ (numberofBlocks which is same as number of bits)/ [(8 bits in an byte)(blockSize which is already in bytes)] ] +1
	VCB->freeSpaceBitmapLocation = 1; 
	VCB->freeSpaceBitmapEndLocation = ( numberOfBlocks/(8*blockSize) )+1 ;
	VCB->rootStartLocation = VCB->freeSpaceBitmapEndLocation + 1;  // letting root start after VCB block spanning 10 blocks
	VCB->freeSpaceStartLocation = VCB->rootStartLocation + 10; // free space will start after root's block
	


	// for our group to test and see where it starts and ends
	printf("VCB's location is at block %d\n", VCB->location);
	printf("freeSpace start is %d ending at %d \nroot starts at %d\nfreespace starts at %d\n", 
	VCB->freeSpaceBitmapLocation,VCB->freeSpaceBitmapEndLocation, VCB->rootStartLocation, VCB->freeSpaceStartLocation);

	freeSpace *freeSpaceTracker = malloc( ( (numberOfBlocks/(8*blockSize)) +1 )* blockSize ); // malloc uses bytes
	printf("malloc'ed freespace is %ld bytes\n",  ( (numberOfBlocks/(8*blockSize)) +1 )* blockSize );
	//freeSpace freeSpaceTracker[numberOfBlocks]; 
	for(int i=0; i< VCB->freeSpaceStartLocation; i++){ // setting 0 for not free
		freeSpaceTracker[i].free = 0;
		//printf("free space location is i=%d\n", i);
	}
	for(int i = VCB->freeSpaceStartLocation; i < numberOfBlocks; i++){ // setting 1 for free
		freeSpaceTracker[i].free = 1;
	}
	printf("freespace ends at %ld\n", numberOfBlocks-1);


	// Freeing malloced items
	free(VCB);
	VCB = NULL;
	free(freeSpaceTracker);
	freeSpaceTracker = NULL;
	printf("done using free for VCB and freeSpaceTracker\n");

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}