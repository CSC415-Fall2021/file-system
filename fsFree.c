/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: fsFree.c
*
* Description: This is the implementation file for free space 
* management. There's implementation for the four function in 
* fsFree.h and some helper functions.
*
**************************************************************/

/** 
 * TODO List
 * 1. [negative return value] for init, allocate, and release
 *    this relates to the helper functions, which using thier
 *    return value to check if setUsed or setFree are success or not.
 * 
 * 2. [nextFreeBlock & allocate] this is just a temporary way of keep 
 *    on track for next available block. For future, we will update the 
 *    allocate function.
 * 
 * 3. [releaseFreeSpace()]
 * 
 * 4. [checkBitUsed()]
 * 
 **/

#include "fsFree.h"
#include "fsLow.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

//setArray & clearArray: for helper function's purpose
unsigned char setArray[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
unsigned char clearArray[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

//[Some helper functions]
//Still developing checkBitUsed
int setBitUsed(unsigned char *array, int bitIndex);
int setBitFree(unsigned char *array, int bitIndex);
int checkBitUsed(unsigned char *array, int bitIndex);

//initialize the free space
//returns the starting block number of bit map
//return value: 1 -> success, -1 -> failed
//(negative return value feature will be updated...)
int initFreeSpace(VCB *vcb)
{
    //printf("[debug] inside initFreeSpace...\n");

    //[Step 1] init the data
    //- bitmap
    //- nextFreeBlock in vcb: to keep on track next available block
    //- totalOfActBit: total of actual bit
    bitMap = malloc(5 * mfs_blockSize);
    vcb->nextFreeBlock = 0;
    int totalOfActBit = 5 * mfs_blockSize * 8;

    //[Step 2] mark first 6 bits to used
    //- 0 for VCB
    //- 1~5 for bitMap
    for (int i = 0; i < 6; i++)
    {
        setBitUsed(bitMap, i);
    }

    //[Step 3] update nextFreeBlock
    vcb->nextFreeBlock += 6;

    //[Step 4] mark rest to be free: because it's the actual size we have
    for (int i = 6; i < vcb->numberOfBlocks; i++)
    {
        setBitFree(bitMap, i);
    }

    //[Step 5] mark the the extra bits to used
    for (int i = vcb->numberOfBlocks; i < totalOfActBit; i++)
    {
        setBitUsed(bitMap, i);
    }

    //[Step 6] write onto disk
    if (LBAwrite(bitMap, 5, 1) != 5)
    {
        printf("[ERROR] LBAwrite failed...\n");
        return -1;
    }

    //printf("[debug] ---------\n");
    //printf("- nextFreeBlock: %d\n", vcb->nextFreeBlock);

    mfs_vcb = vcb;

    //[Step 7] return the starting block
    return 1;
}

//reload the bitmap and keep it in memory
//returns the number of blocks being read
//return value: positive num -> success, -1 -> failed
int reloadFreeSpace()
{
    //[Step 1] init the data
    bitMap = malloc(5 * mfs_blockSize);
    mfs_vcb = malloc(mfs_blockSize);

    //[Step 2] read the block to memory
    int numOfBlockOfBitMap = LBAread(bitMap, 5, 1);
    if (numOfBlockOfBitMap != 5)
    {
        printf("[ERROR] LBAread failed...\n");
        return -1;
    }

    //printf("[debug] ---------\n");
    //printf("- nextFreeBlock: %d\n", vcb->nextFreeBlock);

    LBAread(mfs_vcb, 1, 0);

    //[Step 3] return the number of block being read
    return numOfBlockOfBitMap;
}

//find the free space for the user with given number of block
//returns the starting  block number of free space
//return value: positive num -> success, -1 -> failed
int allocateFreeSpace(int blockCount)
{
    //printf("[debug] inside allocateFreeSpace...\n");

    //[Step 1] get the free space location from vcb field
    int location = mfs_vcb->nextFreeBlock;
    //printf("[debuggg] nextFreeBlock: %d\n", location);

    //[Step 2] from the location, flip number of blockCount bits to used
    for (int i = location; i < location + blockCount; i++)
    {
        setBitUsed(bitMap, i);
    }

    //[Step 3] update the nextFreeBlock field in VCB
    mfs_vcb->nextFreeBlock += blockCount;

    //[Step 4] write onto disk to update the bitmap
    if (LBAwrite(bitMap, 5, 1) != 5)
    {
        printf("[ERROR] LBAwrite failed...\n");
        return -1;
    }

    //printf("[debug] ---------\n");
    //printf("- current location: %d\n- blockCount: %d\n- nextFreeBlock: %d\n",
    //location, blockCount, vcb->nextFreeBlock);

    //[Step 5] return the free block location to user
    return location;
}

//[TBD] release the free space
int releaseFreeSpace()
{
}

//[Some helper functions]
//This will set the bit at bitIndex to 1 (used)
//return 1 -> success, -1 -> failed
int setBitUsed(unsigned char *array, int bitIndex)
{
    //printf("[debug] setting bit to used...\n");

    array[bitIndex / 8] = array[bitIndex / 8] | setArray[bitIndex % 8];

    //[still working] using another helper function to double check
    return checkBitUsed(array, bitIndex);
}

//This will set the bit at bitIndex to 0 (free)
//return 1 -> success, -1 -> failed
int setBitFree(unsigned char *array, int bitIndex)
{
    //printf("[debug] setting bit to free...\n");

    array[bitIndex / 8] = array[bitIndex / 8] & clearArray[bitIndex % 8];

    //[still working] using another helper function to double check
    return (checkBitUsed(array, bitIndex) * -1);
}

//[Still developing... ignore the implementation]
//This will check the bit at bitIndex is 1 or not
//return 1-> used, -1 -> free
int checkBitUsed(unsigned char *array, int bitIndex)
{
    if (array[bitIndex] == '1')
    {
        return 1;
    }

    return -1;
}