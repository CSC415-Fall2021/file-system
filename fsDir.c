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

typedef struct directoryEntry
{
    int size;
    int location;
    char name[100];
} directoryEntry;

int initDir(int numberOfBlock)
{
    directoryEntry *directory = malloc(512 * numberOfBlock);
    printf("[debug] sizeof(directory) = %ld", sizeof(directoryEntry));
    int numOfDe = (512 * numberOfBlock) / sizeof(directoryEntry);
    for (int i = 0; i < numOfDe; i++)
    {
        directory[i].
    }
}