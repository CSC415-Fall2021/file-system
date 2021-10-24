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

int setBitUsed(int *array, int bitIndex);
int setBitFree(int *array, int bitIndex);
int checkBitUsed(int *array, int bitIndex);

//return positive num -> success, -1 -> failed
int initFreeSpace(int blockSize)
{
    printf("[debug] inside initFreeSpace...\n");

    //malloc space for bitmap
    int *bitMap = malloc(5 * blockSize);

    //mark first 6 bits to used: 0 -> VCB, 1~5 -> bitMap
    for (int i = 0; i < 6; i++)
    {
        if (setBitUsed(bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        };
    }

    //mark rest to be free: 2442 because it's the actual size we have
    for (int i = 6; i < 2442; i++)
    {
        if (setBitFree(bitMap, i) != 1)
        {
            printf("[ERROR] setBitFree() failed...\n");
            return -1;
        }
    }

    //mark the the extra bits to used
    for (int i = 2442; i < 2560; i++)
    {
        if (setBitUsed(bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        }
    }

    //write into disk
    LBAwrite(bitMap, 5, 1);

    //return the starting block
    return 1;
}

int allocateFreeSpace(int blockCount)
{
}

//[TBD] release the free space
int releaseFreeSpace()
{
}

//Some helper functions

//return 1 -> success, -1 -> failed
int setBitUsed(int *array, int bitIndex)
{
    int arrayIndex = bitIndex / 32;
    int arrayPosition = bitIndex % 32;
    unsigned int flag = 1;
    flag = flag << arrayPosition;
    array[arrayIndex] = array[arrayIndex] | flag;

    //double check if we set the bit successfully and return result
    return checkBitUsed(array, bitIndex);
}

//return 1 -> success, -1 -> failed
int setBitFree(int *array, int bitIndex)
{
    int arrayIndex = bitIndex / 32;
    int arrayPosition = bitIndex % 32;
    unsigned int flag = 1;
    flag = flag << arrayPosition;
    flag = ~flag;
    array[arrayIndex] = arrayIndex * flag;
    return (checkBitUsed(array, bitIndex) * -1);
}

//return 1-> used, -1 -> free
int checkBitUsed(int *array, int bitIndex)
{
    int arrayIndex = bitIndex / 32;
    int arrayPosition = bitIndex % 32;
    unsigned int flag = 1;
    flag = flag << arrayPosition;
    if (array[arrayIndex] & flag)
        return 1;
    return -1;
}