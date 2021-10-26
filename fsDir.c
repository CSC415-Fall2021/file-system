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

#include "fsDir.h"
#include "fsLow.h"
#include <string.h>

void printDEInfo(DE de);

int createDir(int parentLocation, int DEcount, int blockSize, freeSpaceManager *manager)
{
    //malloc space for directory
    //  determine how much space we need for desired DEcount
    //  calculate the number of block we need for sizeOfSpace (temporary)
    //  update DEcount when needed
    //  malloc with mallocSize
    int sizeOfSpace = sizeof(DE) * DEcount;               //what we think we need to allocate
    int numOfBlockNeeded = (sizeOfSpace / blockSize) + 1; //number of block to fit what we think we need to allocate
    printf("[debug] root will need %d blocks\n", numOfBlockNeeded);
    int mallocSize = numOfBlockNeeded * blockSize; //what we will really allocate (based on numOfBlock)
    DEcount += (mallocSize - sizeOfSpace) / sizeof(DE);
    printf("[debug] root will have %d DE in total.\n", DEcount);
    printf("[debug] DE size is %ld\n", sizeof(DE));
    DE *directory = (DE *)malloc(mallocSize);

    //loop through the directory and init each DE struct
    for (int i = 0; i < DEcount; i++)
    {
        strcpy(directory[i].name, "\0");
        directory[i].size = 0;
        directory[i].pointingLocation = 0;
        directory[i].isDir = 0;
        directory[i].createTime = 0;
        directory[i].lastModTime = 0;
        directory[i].lastModTime = 0;
    }

    //allocate blocks from free space
    int location = allocateFreeSpace(manager, numOfBlockNeeded);
    printf("[debug] got free location at %d\n", location);
    if (location == -1)
    {
        printf("[ERROR] Line 47 : fsDir.c allocateFreeSpace() failed...\n");
        return -1;
    }

    //set first DE as itself
    strcpy(directory[0].name, ".");
    directory[0].size = mallocSize;
    directory[0].pointingLocation = 6;
    directory[0].isDir = 1; //it is a directory
    time(&directory[0].createTime);
    //printf("created at: %s\n", ctime(&directory[0].createTime));
    time(&directory[0].lastModTime);
    time(&directory[0].lastAccessTime);

    printDEInfo(directory[0]);

    //set second DE as parent
    strcpy(directory[1].name, "..");
    directory[1].size = mallocSize;
    if (parentLocation == 0) //not sure bout this...
    {
        directory[1].pointingLocation = location;
    }
    else
    {
        directory[1].pointingLocation = parentLocation;
    }
    directory[1].isDir = 1; //it is a directory
    time(&directory[1].createTime);
    time(&directory[1].lastModTime);
    time(&directory[1].lastAccessTime);

    printDEInfo(directory[1]);

    printf("[debug] numberOfBlockNeeded: %d -> should be 11\n[debug] location: %d -> should be 6\n", numOfBlockNeeded, location);

    //write into disk
    LBAwrite(directory, numOfBlockNeeded, location);

    return location;
}

void printDEInfo(DE de)
{
    printf("--- DE info ---\n");
    printf("- name: %s\n- size: %d\n- pointingLocation: %d\n", de.name, de.size, de.pointingLocation);
    printf("- isDir: %d\n- createTime: %s\n- lastMod: %s\n- lastAccess: %s\n", de.isDir, ctime(&de.createTime), ctime(&de.lastModTime), ctime(&de.lastAccessTime));
}