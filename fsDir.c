/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: fsDir.c
*
* Description: The implementation file for directory. 
*
**************************************************************/

#include "fsDir.h"
#include "fsLow.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void printDEInfo(DE de);

int createDir(int parentLocation)
{
    int mallocSize = sizeof(DE) * DefaultDECount;
    int numOfBlockNeeded = (mallocSize / mf_blockSize) + 1; //TODO what if mallocSize is product of 512?
    mallocSize = numOfBlockNeeded * mf_blockSize;
    DefaultDECount = mallocSize / sizeof(DE);
    DE *directory = (DE *)malloc(mallocSize); //TODO DE* or DE**

    for (int i = 0; i < DefaultDECount; i++)
    {
        strcpy(directory[i].name, "\0");
        directory[i].size = 0;
        directory[i].location = 0;
        directory[i].isDir = 0; //TODO is this the init state?
        directory[i].createTime = 0;
        directory[i].lastModTime = 0;
        directory[i].lastAccessTime = 0;
    }

    int startLocation = allocateFreeSpace(numOfBlockNeeded);
    if (startLocation == -1)
    {
        printf("[ERROR] fsDir.c line 45: allocateFreeSpace() failed...\n");
        return -1;
    }

    strcpy(directory[0].name, ".");
    directory[0].size = mallocSize;
    directory[0].location = startLocation;
    directory[0].isDir = 1;
    time(&directory[0].createTime);
    time(&directory[0].lastModTime);
    time(&directory[0].lastAccessTime);

    strcpy(directory[1].name, "..");
    directory[1].size = mallocSize;
    if (parentLocation == 0)
        directory[1].location = startLocation;
    else
        directory[1].location = parentLocation;
    directory[1].isDir = 1;
    time(&directory[1].createTime);
    time(&directory[1].lastModTime);
    time(&directory[1].lastAccessTime);

    int ret = LBAwrite(directory, numOfBlockNeeded, startLocation);
    if (ret != numOfBlockNeeded)
    {
        printf("[ERROR] fsDir.c line 71: LBAwrite returned %d, but we need %d blocks...\n", ret, numOfBlockNeeded);
        return -1;
    }
    return startLocation;
}

void printDEInfo(DE de)
{
    printf("--- DE info ---\n");
    printf("- name: %s\n- size: %d\n- pointingLocation: %d\n", de.name, de.size, de.location);
    printf("- isDir: %d\n- createTime: %s\n- lastMod: %s\n- lastAccess: %s\n", de.isDir, ctime(&de.createTime), ctime(&de.lastModTime), ctime(&de.lastAccessTime));
}