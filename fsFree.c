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

#include "fsFree.h"
#include "fsLow.h"
#include <string.h>

int setBitUsed(unsigned char *array, int bitIndex);
int setBitFree(unsigned char *array, int bitIndex);
int checkBitUsed(unsigned char *array, int bitIndex);
void printManager(freeSpaceManager *manager);

unsigned char *bitMap;
unsigned char setArray[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};
unsigned char clearArray[8] = {0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE};

//bit map: 1 -> used, 0 -> free

//return positive num -> success, -1 -> failed
int initFreeSpace(VCB *vcb, int blockSize)
{
    printf("[debug] inside initFreeSpace...\n");

    //init data
    bitMap = malloc(5 * blockSize);
    vcb->nextFreeBlock = 0;
    int totalOfActBit = 5 * blockSize * 8;
    //printf("[debug] totalOfActBit: %d\n", totalOfActBit);

    //mark first 6 bits to used: 0 -> VCB, 1~5 -> bitMap
    for (int i = 0; i < 6; i++)
    {
        setBitUsed(bitMap, i);
    }

    //update nextFreeBlock
    vcb->nextFreeBlock += 6;

    //printf("[debuggg] numberOfBlocks: %d\n", vcb->numberOfBlocks);

    //mark rest to be free: 2442 because it's the actual size we have
    for (int i = 6; i < vcb->numberOfBlocks; i++)
    {
        setBitFree(bitMap, i);
    }

    //mark the the extra bits to used
    for (int i = vcb->numberOfBlocks; i < totalOfActBit; i++)
    {
        setBitUsed(bitMap, i);
    }

    //write into disk
    //TODO check return
    LBAwrite(bitMap, 5, 1);

    printf("[debug] ---------\n");
    printf("- nextFreeBlock: %d\n", vcb->nextFreeBlock);

    //update location
    //manager->location = 1;

    //[debug] print out manager
    //printManager(manager);

    //return the starting block
    return 1;
}

//need to modify vcb

int reloadFreeSpace(VCB *vcb, int blockSize)
{
    bitMap = malloc(5 * blockSize);
    int numOfBlockOfBitMap = LBAread(bitMap, 5, 1);
    printf("[debug] ---------\n");
    printf("- nextFreeBlock: %d\n", vcb->nextFreeBlock);

    return numOfBlockOfBitMap;
}

int allocateFreeSpace(VCB *vcb, int blockCount)
{
    printf("[debug] inside allocateFreeSpace...\n");

    int location = vcb->nextFreeBlock;
    printf("[debuggg] nextFreeBlock: %d\n", location);

    for (int i = location; i < location + blockCount; i++)
    {
        setBitUsed(bitMap, i);
    }

    vcb->nextFreeBlock += blockCount;

    //TODO check returns
    LBAwrite(bitMap, 5, 1);

    printf("[debug] ---------\n");
    printf("- location: %d\n- blockCount: %d\n- nextFreeBlock: %d\n", location, blockCount, vcb->nextFreeBlock);

    return location;
}

//[TBD] release the free space
int releaseFreeSpace()
{
}

//Some helper functions

//return 1 -> success, -1 -> failed
int setBitUsed(unsigned char *array, int bitIndex)
{
    //printf("[debug] setting bit to used...\n");

    array[bitIndex / 8] = array[bitIndex / 8] | setArray[bitIndex % 8];

    //double check if we set the bit successfully and return result
    return checkBitUsed(array, bitIndex);
}

//return 1 -> success, -1 -> failed
int setBitFree(unsigned char *array, int bitIndex)
{
    //printf("[debug] setting bit to free...\n");
    array[bitIndex / 8] = array[bitIndex / 8] & clearArray[bitIndex % 8];

    return (checkBitUsed(array, bitIndex) * -1);
}

//return 1-> used, -1 -> free
int checkBitUsed(unsigned char *array, int bitIndex)
{
    if (array[bitIndex] == '1')
    {
        return 1;
    }

    return -1;
}

//print out manager's data
void printManager(freeSpaceManager *manager)
{
    printf("--- Manager Data ---\n");
    printf("- blockSize: %d\n- totalOfBlock: %d\n", manager->blockSize, manager->totalOfBlock);
    printf("- location: %d\n- usedCount: %d\n- freeCount: %d\n", manager->location, manager->usedCount, manager->freeCount);
}