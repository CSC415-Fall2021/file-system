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

//bitmap: the unsiged char array for managing the free space management
unsigned char *bitMap;

//bit map: 1 -> used, 0 -> free

//initialize the free space
//returns the starting block number of bit map
//return value: 1 -> success, -1 -> failed
int initFreeSpace(VCB *vcb);

//reload the bitmap and keep it in memory
//returns the number of blocks being read
//return value: positive num -> success, -1 -> failed
int reloadFreeSpace();

//find the free space for the user with given number of block
//returns the starting  block number of free space
//return value: positive num -> success, -1 -> failed
int allocateFreeSpace(int blockCount);

//return value: 1 -> success, -1 -> failed
int releaseFreeSpace(int location, int blockCount);