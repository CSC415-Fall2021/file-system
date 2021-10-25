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

int setBitUsed(char *array, int bitIndex);
int setBitFree(char *array, int bitIndex);
int checkBitUsed(char *array, int bitIndex);
void printManager(freeSpaceManager *manager);

//bit map: 1 -> used, 0 -> free

//return positive num -> success, -1 -> failed
int initFreeSpace(freeSpaceManager *manager)
{
    printf("[debug] inside initFreeSpace...\n");

    //mark first 6 bits to used: 0 -> VCB, 1~5 -> bitMap
    for (int i = 0; i < 6; i++)
    {
        if (setBitUsed(manager->bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        };
    }

    //update the blockRemains
    manager->freeCount -= 6;
    manager->usedCount += 6;

    //mark rest to be free: 2442 because it's the actual size we have
    for (int i = 6; i < 2442; i++)
    {
        if (setBitFree(manager->bitMap, i) != 1)
        {
            //printf("[ERROR] setBitFree() failed at %d...\n", i);
            return -1;
        }
    }

    //mark the the extra bits to used
    for (int i = 2442; i < 2560; i++)
    {
        if (setBitUsed(manager->bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        }
    }

    //write into disk
    //TODO check return
    LBAwrite(manager->bitMap, 5, 1);

    //update location
    manager->location = 1;

    //[debug] print out manager
    printManager(manager);

    //return the starting block
    return 1;
}

int reloadFreeSpace(freeSpaceManager *manager)
{
    int usedCount, freeCount;
    int numOfBlockOfBitMap = LBAread(manager->bitMap, 5, 1);
    for (int i = 0; i < strlen(manager->bitMap); i++)
    {
        if (manager->bitMap[i] == '1')
            usedCount++;
        else
            freeCount;
    }

    manager->usedCount = usedCount;
    manager->freeCount = freeCount;
    printManager(manager);
    return numOfBlockOfBitMap;
}

int allocateFreeSpace(freeSpaceManager *manager, int blockCount)
{
    int location = -1;
    //check if free space has enough block to allocate
    if (manager->freeCount < blockCount)
    {
        printf("[ERROR] not enough space to allocate in free space...\n");
        return location;
    }

    //loop through bitmap and use checkBitUsed to find free space
    for (int i = 0; i < manager->totalOfBlock; i++)
    {
        if (checkBitUsed(manager->bitMap, i) == -1)
        {
            location = i;
            for (int j = i; j <= blockCount; j++)
            {
                if (checkBitUsed(manager->bitMap, j) == 1)
                {
                    location = -1;
                    break;
                }
            }
            break;
        }
    }

    //update bit map
    for (int i = location; i < blockCount + location; i++)
    {
        if (setBitUsed(manager->bitMap, i) == -1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        }
    }

    //update blockRemains
    manager->freeCount -= blockCount;
    manager->usedCount += blockCount;

    //update disk
    //TODO check return
    LBAwrite(manager->bitMap, 5, manager->location);

    //[debug] print out manager
    printManager(manager);

    return location;
}

//[TBD] release the free space
int releaseFreeSpace()
{
}

//Some helper functions

//return 1 -> success, -1 -> failed
int setBitUsed(char *array, int bitIndex)
{
    printf("[debug] setting bit to used...\n");
    array[bitIndex] = '1';

    //double check if we set the bit successfully and return result
    return checkBitUsed(array, bitIndex);
}

//return 1 -> success, -1 -> failed
int setBitFree(char *array, int bitIndex)
{
    printf("[debug] setting bit to free...\n");
    array[bitIndex] = '0';
    return (checkBitUsed(array, bitIndex) * -1);
}

//return 1-> used, -1 -> free
int checkBitUsed(char *array, int bitIndex)
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

/*int setBitUsed(int *array, int bitIndex);
int setBitFree(int *array, int bitIndex);
int checkBitUsed(int *array, int bitIndex);
void printManager(freeSpaceManager *manager);

//bit map: 1 -> used, 0 -> free

//return positive num -> success, -1 -> failed
int initFreeSpace(freeSpaceManager *manager)
{
    printf("[debug] inside initFreeSpace...\n");

    //mark first 6 bits to used: 0 -> VCB, 1~5 -> bitMap
    for (int i = 0; i < 6; i++)
    {
        if (setBitUsed(manager->bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        };
    }

    //update the blockRemains
    manager->blockRemains -= 6;

    //mark rest to be free: 2442 because it's the actual size we have
    for (int i = 6; i < 2442; i++)
    {
        if (setBitFree(manager->bitMap, i) != 1)
        {
            printf("[ERROR] setBitFree() failed at %d...\n", i);
            return -1;
        }
    }

    //mark the the extra bits to used
    for (int i = 2442; i < 2560; i++)
    {
        if (setBitUsed(manager->bitMap, i) != 1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        }
    }

    //write into disk
    LBAwrite(manager->bitMap, 5, 1);

    //update location
    manager->location = 1;

    //[debug] print out manager
    printManager(manager);

    //return the starting block
    return 1;
}

int allocateFreeSpace(freeSpaceManager *manager, int blockCount)
{
    int location = -1;
    //check if free space has enough block to allocate
    if (manager->blockRemains < blockCount)
    {
        printf("[ERROR] not enough space to allocate in free space...\n");
        return location;
    }

    //loop through bitmap and use checkBitUsed to find free space
    for (int i = 0; i < manager->totalOfBlock; i++)
    {
        printf("[debug] i = %d\n", i);
        if (checkBitUsed(manager->bitMap, i) == -1)
        {
            printf("[debug] found free bits! ready to check rest!\n");
            location = i;
            for (int j = i; j <= blockCount; j++)
            {
                if (checkBitUsed(manager->bitMap, j) == 1)
                {
                    printf("[debug] not free... find the next one\n");
                    location = -1;
                    break;
                }
            }
            break;
        }
    }

    //update bit map
    for (int i = location; i <= blockCount; i++)
    {
        if (setBitUsed(manager->bitMap, i) == -1)
        {
            printf("[ERROR] setBitUsed() failed...\n");
            return -1;
        }
    }

    //update blockRemains
    manager->blockRemains -= blockCount;

    //update disk
    LBAwrite(manager->bitMap, 5, manager->location);

    //[debug] print out manager
    printManager(manager);

    return location;
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
    //if (bitIndex == 65)
    //printf("[debug] arrayIndex: %d\n", arrayIndex);
    int arrayPosition = bitIndex % 32;
    unsigned int flag = 1;
    flag = flag << arrayPosition;
    flag = ~flag;
    array[arrayIndex] = arrayIndex & flag;
    //printf("[debug]: %d\n", checkBitUsed(array, bitIndex) * -1);
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

//print out manager's data
void printManager(freeSpaceManager *manager)
{
    printf("--- Manager Data ---\n");
    printf("- blockSize: %d\n- totalOfBlock: %d\n", manager->blockSize, manager->totalOfBlock);
    printf("- location: %d\n- blockRemains: %d\n", manager->location, manager->blockRemains);
}*/