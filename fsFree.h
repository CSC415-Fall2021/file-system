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
int initFreeSpace(VCB *vcb, int blockSize);

//returns the number of blocks being read
int reloadFreeSpace(VCB *vcb, int blockSize);

//find the free space for the user with given number of block
//returns the starting  block number of free space
int allocateFreeSpace(VCB *vcb, int blockCount);

//[TBD] release the free space
int releaseFreeSpace();