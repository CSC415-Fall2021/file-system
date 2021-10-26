/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: fsFree.h
*
* Description: This is the header file for free space management.
*
**************************************************************/

#include "mfs.h"

//initialize the free space
//returns the starting block number of bit map
//return value: 1 -> success, -1 -> failed
int initFreeSpace(VCB *vcb, int blockSize);

//reload the bitmap and keep it in memory
//returns the number of blocks being read
//return value: positive num -> success, -1 -> failed
int reloadFreeSpace(VCB *vcb, int blockSize);

//find the free space for the user with given number of block
//returns the starting  block number of free space
//return value: positive num -> success, -1 -> failed
int allocateFreeSpace(VCB *vcb, int blockCount);

//[TBD] release the free space
int releaseFreeSpace();