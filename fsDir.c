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
    DE *directory = (DE *)malloc(mallocSize); //TODO DE* or DE** and check malloc

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
        free(directory);
        directory = NULL;
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

    free(directory);
    directory = NULL;

    if (ret != numOfBlockNeeded)
    {
        printf("[ERROR] fsDir.c line 71: LBAwrite returned %d, but we need %d blocks...\n", ret, numOfBlockNeeded);
        return -1;
    }
    return startLocation;
}

bool pathParser(char *path, unsigned char condition, DE *tempWorkingDir)
{
    //cwd home
    if (path[0] != '/')
    {
        printf("[debug] User passed in relative path...\n");
        tempWorkingDir = mfs_cwd;
    }

    int tokenCount = 0;
    char **tokens = malloc(strlen(path)); //TODO check malloc
    char *theRest = path;
    char *token = strtok_r(path, "/", &theRest);
    while (token != NULL)
    {
        tokens[tokenCount++] = token;
        token = strtok_r(NULL, "/", &theRest);
    }
    // cwd = bar
    //cd folder/HW1
    //path = Documents/CSC415/HW
    //tokens = {"Documents", "CSC415", "HW"}
    //tokens = {"DocumentsCSC415HW"}
    bool found = 0;
    for (int tokIndex = 0; tokIndex < tokenCount - 1; tokIndex++)
    {
        //int DEcount = tempWorkingDir[0].size / sizeof(DE);
        for (int dirIndex = 0; dirIndex < DefaultDECount; dirIndex++)
        {
            if (strcmp(tempWorkingDir[dirIndex].name, tokens[tokIndex]) == 0)
            {
                printf("[debug] found entry at %dth index\n", dirIndex);
                found = 1;
                int blockCount = tempWorkingDir[dirIndex].size / mf_blockSize;
                int ret = LBAread(tempWorkingDir, tempWorkingDir[dirIndex].location, blockCount);
                if (ret != blockCount)
                {
                    printf("[ERROR] fsDir.c line 106: LBAread failed...\n");
                    free(tokens);
                    tokens = NULL;
                    return 0;
                }
                break;
            }
        }
        if (!found)
        {
            printf("[ERROR] fsDir.c line 117: directory does not exist...\n");
            free(tokens);
            tokens = NULL;
            return 0;
        }
        found = 0;
    }
    //tempWorkingDir = CSC415
    //last token = HW
    unsigned char thisCondition = 0x00;
    for (int i = 2; i < DefaultDECount; i++)
    {
        if (strcmp(tempWorkingDir[i].name, tokens[tokenCount - 1]) == 0)
        {
            if (tempWorkingDir[i].isDir)
            {
                thisCondition = EXIST_DIR;
            }
            else
            {
                thisCondition = EXIST_FILE;
            }
            break;
        }
        thisCondition = NOT_EXIST;
    }

    free(tokens);
    tokens = NULL;

    printDEInfo(tempWorkingDir[0]);

    if (condition == thisCondition)
    {
        return 1;
    }
    return 0;
}

void printDEInfo(DE de)
{
    printf("--- DE info ---\n");
    printf("- name: %s\n- size: %d\n- pointingLocation: %d\n", de.name, de.size, de.location);
    printf("- isDir: %d\n- createTime: %s\n- lastMod: %s\n- lastAccess: %s\n", de.isDir, ctime(&de.createTime), ctime(&de.lastModTime), ctime(&de.lastAccessTime));
}