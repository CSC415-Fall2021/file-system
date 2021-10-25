/**************************************************************
* Class:  CSC-415-0#  Fall 2021
* Names: 
* Student IDs:
* GitHub Name:
* Group Name:
* Project: Basic File System
*
* File: 
*
* Description: 
*
**************************************************************/

#include "mfs.h"

//initialize the free space
//returns the starting block number of bit map
//return value: positive num -> success, -1 -> failed
int initFreeSpace(freeSpaceManager manager);

//find the free space for the user with given number of block
//returns the starting  block number of free space
int allocateFreeSpace(freeSpaceManager manager, int blockCount);

//[TBD] release the free space
int releaseFreeSpace();