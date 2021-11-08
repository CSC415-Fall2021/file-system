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
#include "mfs.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int createDir(int parentLocation)
{
    //printf("[debug] inside createDir\n");
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1; //TODO what if mallocSize is product of 512?
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    mfs_defaultDECount = mallocSize / sizeof(DE);
    DE *directory = (DE *)malloc(mallocSize); //TODO DE* or DE** and check malloc
    //printf("[debug] root size is: %d\n", mallocSize);

    for (int i = 0; i < mfs_defaultDECount; i++)
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
    // printf("[debug] startLoation: %d\n", startLocation);
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

bool pathParser(char *path, unsigned char condition, DE *tempWorkingDir, char *lastToken)
{
    printf("\n--------- INSIDE THE PATH PARSER ---------\n");
    printf("[debug] arguments:\n- path: %s\n- condition: %d\n", path, condition);

    if (path[0] != '/')
    {
        printf("[debug] User passed in relative path... \n");
        tempWorkingDir = mfs_cwd;
    }
    else
    {
        printf("[debug] User passed in absolute path... \n");
        printf("[debug] root has %d blocks at %d location\n", mfs_vcb->rootSize / mfs_blockSize, mfs_vcb->rootLocation);
        LBAread(tempWorkingDir, mfs_vcb->rootSize / mfs_blockSize, mfs_vcb->rootLocation);
    }

    printf("[debug] print out the tempWorkingDir info\n");
    printDEInfo(tempWorkingDir[0]);

    int tokenCount = 0;
    char **tokens = malloc(strlen(path)); //TODO check malloc
    char *theRest = path;
    char *token = strtok_r(path, "/", &theRest); //seg fault

    printf("[debug] tokenizing the path...\n");
    while (token != NULL)
    {
        printf("[debug] token: %s\n", token);
        tokens[tokenCount] = token;
        printf("[debug] add tokens[%d]: %s\n", tokenCount, tokens[tokenCount]);
        tokenCount++;
        token = strtok_r(NULL, "/", &theRest);
    }

    bool found = 0;
    for (int tokIndex = 0; tokIndex < tokenCount - 1; tokIndex++)
    {
        printf("[debug] see if path is valid\n");
        printf("[debug] checking %s: ", tokens[tokIndex]);
        //int DEcount = tempWorkingDir[0].size / sizeof(DE);
        for (int dirIndex = 0; dirIndex < mfs_defaultDECount; dirIndex++)
        {
            if (strcmp(tempWorkingDir[dirIndex].name, tokens[tokIndex]) == 0)
            {
                printf("found entry at %dth index\n", dirIndex);
                found = 1;
                int blockCount = tempWorkingDir[dirIndex].size / mfs_blockSize;
                int ret = LBAread(tempWorkingDir, blockCount, tempWorkingDir[dirIndex].location);
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
            printf("entry does not exist.\n");
            printf("[ERROR] fsDir.c line 117: directory does not exist...\n");
            free(tokens);
            tokens = NULL;
            return -1;
        }
        found = 0;
    }

    printf("[debug] print out the tempWorkingDir info (this is the info for second last token)\n");
    printDEInfo(tempWorkingDir[0]);
    unsigned char thisCondition = NOT_EXIST;

    strcpy(lastToken, tokens[tokenCount - 1]);

    for (int i = 2; i < mfs_defaultDECount; i++)
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
    }

    printf("[debug] last token is %s, condition is %d\n", lastToken, thisCondition);

    free(tokens);
    tokens = NULL;

    if (condition == thisCondition)
    {
        return 1;
    }
    return 0;
}

//finished implementing, still need to be tested
int fs_mkdir(const char *pathname, mode_t mode)
{
    printf("inside the mkdir\n");

    char path[strlen(pathname)];
    strcpy(path, pathname);

    //1
    DE *tempWorkingDir = malloc(mfs_cwd[0].size);
    char *lastToken = malloc(256); //TODO hardcode

    //2
    bool valid = pathParser(path, NOT_EXIST, tempWorkingDir, lastToken);

    printf("[debug] valid: %d, tempWorkingDir's name: %s, lastToken: %s\n", valid, tempWorkingDir[0].name, lastToken);

    //3
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 182: invalid path...\n");
        return -1;
    }

    //4
    int DEindex = -1;
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        if (strcmp(tempWorkingDir[DEindex].name, "\0") == 0)
        {
            DEindex = i;
            break;
        }
    }

    if (DEindex == -1)
    {
        printf("[ERROR] fsDir.c line 204: directory full...\n");
        return -1;
    }

    //5
    int newLocation = createDir(tempWorkingDir[0].location);
    if (newLocation == -1)
    {
        printf("[ERROR] fsDir.c line 211: createDir error...\n");
    }

    //6
    printf("[debug] DEindex: %d\n", DEindex);
    strcpy(lastToken, tempWorkingDir[DEindex].name);
    tempWorkingDir[DEindex].size = ((mfs_defaultDECount * sizeof(DE)) / mfs_blockSize + 1) * mfs_blockSize;
    tempWorkingDir[DEindex].location = newLocation;
    tempWorkingDir[DEindex].isDir = 1;
    time(&tempWorkingDir[DEindex].createTime);
    time(&tempWorkingDir[DEindex].lastModTime);
    time(&tempWorkingDir[DEindex].lastAccessTime);
    printDEInfo(tempWorkingDir[DEindex]);

    //7
    printDEInfo(tempWorkingDir[0]);
    int blockCount = tempWorkingDir[0].size / mfs_blockSize;
    printf("[debug] blockCount: %d location: %d\n", blockCount, tempWorkingDir[0].location);
    int ret = LBAwrite(tempWorkingDir, blockCount, tempWorkingDir[0].location);
    if (ret != blockCount)
    {
        printf("[ERROR] LBAwrite failed...\n");
        return -1;
    }

    //8
    free(tempWorkingDir);
    free(lastToken);
    tempWorkingDir = NULL;
    lastToken = NULL;

    //9
    return 1;
}

//finished implementing, still need to be tested
/*int fs_rmdir(const char *pathname)
{
    //1
    DE *tempWorkingDir;
    char *lastToken;

    //2
    bool valid = pathParser(pathname, NOT_EXIST, tempWorkingDir, lastToken);

    printf("[debug] valid: %d, tempWorkingDir's name: %s, lastToken: %s\n", valid, tempWorkingDir[0].name, lastToken);

    //3
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 182: invalid path...\n");
        return -1;
    }

    //4
    for (int i = 2; i < DefaultDECount; i++)
    {
        //5
        if (tempWorkingDir[i].name != "\0")
        {
            printf("[ERROR] fsDir.c line 261: tempWorkingDir is not empty...\n");
            return -1;
        }
    }

    //6
    int ret = fs_delete(lastToken);

    //7
    if (ret == -1)
    {
        printf("[ERROR] fsDir.c line 270: delete failed...\n");
        return -1;
    }

    //(missing on plan)
    int blockCount = tempWorkingDir[0].size / mf_blockSize;
    ret = LBAwrite(tempWorkingDir, blockCount, tempWorkingDir[0].location);
    if (ret != blockCount)
    {
        printf("[ERROR] fsDir.c line 281: LBAwrite failed...\n");
    }

    //8
    free(tempWorkingDir);
    free(lastToken);
    tempWorkingDir = NULL;
    lastToken = NULL;

    //9
    return 1;
}*/

void printDEInfo(DE de)
{
    printf("--- DE info ---\n");
    printf("- name: %s\n- size: %d\n- pointingLocation: %d\n", de.name, de.size, de.location);
    printf("- isDir: %d\n- createTime: %s\n- lastMod: %s\n- lastAccess: %s\n", de.isDir, ctime(&de.createTime), ctime(&de.lastModTime), ctime(&de.lastAccessTime));
}