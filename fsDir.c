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

    printf("\n--------- INSIDE THE CREATE DIR ---------\n");

    //printf("[debug] inside createDir\n");
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    mfs_defaultDECount = mallocSize / sizeof(DE);
    printf("[debug] mallocSize: %d\n", mallocSize);
    printf("[debug] err issue\n");
    DE *directory = malloc(mallocSize);
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
    directory[0].actualSize = sizeof(DE) * 2;
    directory[0].blockCount = numOfBlockNeeded;
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

    directory[0].actualSize = sizeof(DE) * 2;
    directory[0].blockCount = numOfBlockNeeded;
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

    printf("\n--------- END THE CREATE DIR ---------\n");

    return startLocation;
}

bool pathParser(char *path, unsigned char condition, DE *tempWorkingDir, char *lastToken)
{
    printf("\n--------- INSIDE THE PATH PARSER ---------\n");
    printf("[debug] arguments:\n- path: %s\n- condition: %d\n", path, condition);

    if (path[0] != '/')
    {
        printf("[debug] User passed in relative path... \n");
        int mallocSize = sizeof(DE) * mfs_defaultDECount;
        int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
        LBAread(tempWorkingDir, numOfBlockNeeded, mfs_cwd_location);
        printf("[debug] print out tempWorkingDir info\n");
        printDEInfo(tempWorkingDir[0]);
        // tempWorkingDir = mfs_cwd;
    }
    else
    {
        printf("[debug] User passed in absolute path... \n");
        printf("[debug] root has %d blocks at %d location\n", mfs_vcb->rootSize / mfs_blockSize, mfs_vcb->rootLocation);
        LBAread(tempWorkingDir, mfs_vcb->rootSize / mfs_blockSize, mfs_vcb->rootLocation);
    }

    int tokenCount = 0;
    char **tokens = malloc((strlen(path) / 2) * sizeof(char *)); //TODO check malloc
    char *theRest;
    char *token = strtok_r(path, "/", &theRest);

    printf("[debug] tokenizing the path...\n");
    while (token != NULL)
    {
        printf("[debug] token: %s\n", token);
        tokens[tokenCount] = token;
        printf("[debug] add tokens[%d]: %s\n", tokenCount, tokens[tokenCount]);
        tokenCount++;
        token = strtok_r(NULL, "/", &theRest);
    }
    printf("[debug] tokenCount: %d\n", tokenCount);
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
                    printf("[ERROR] fsDir.c line 136: %d != %d, LBAread failed...\n", ret, blockCount);
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
            return 0;
        }
        found = 0;
    }

    printf("[debug] print out the tempWorkingDir info (this is the info for second last token)\n");
    printDEInfo(tempWorkingDir[0]);
    unsigned char thisCondition = NOT_EXIST;
    if (tokenCount != 0)
    {
        strcpy(lastToken, tokens[tokenCount - 1]);
    }
    for (int i = 0; i < mfs_defaultDECount; i++)
    {
        if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
        {
            thisCondition = EXIST;
            if (tempWorkingDir[i].isDir)
            {
                printf("[debug] lastToken is existed dir\n");
                thisCondition = EXIST_DIR;
            }
            else
            {
                printf("[debug] lastToken is existed file\n");
                thisCondition = EXIST_FILE;
            }
            break;
        }
    }

    if (strcmp(path, "/") == 0)
    {
        printf("[debug] lastToken is root so always exist\n");
        thisCondition = EXIST_DIR;
    }

    printf("[debug] last token is %s, condition is %d\n", lastToken, thisCondition);

    free(tokens);
    tokens = NULL;
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] comparing the given condition with this condition, condition: %d, this condition: %d\n", condition, thisCondition);
    printf("--------- END OF THE PATH PARSER ---------\n");

    if (condition == (EXIST_FILE | EXIST_DIR))
    {
        printf("[debug] checking for exist dir or file\n");
        if (thisCondition != NOT_EXIST)
        {
            return 1;
        }
    }

    if ((condition) == thisCondition)
    {
        return 1;
    }
    return 0;
}

//finished implementing, still need to be tested
int fs_mkdir(const char *pathname, mode_t mode)
{
    printf("\n--------- INSIDE THE MKDIR ---------\n");

    //1 convert path to char array (to avoid the const warning)
    char *path = malloc(strlen(pathname) + 1);
    //char path[strlen(pathname)];
    strcpy(path, pathname);

    //2 malloc space for tempWorkingDir and lastToken
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256); //TODO hardcode

    //3 check validation by calling pathParser()
    bool valid = pathParser(path, NOT_EXIST, tempWorkingDir, lastToken);
    printf("[debug] valid: %d\n", valid);
    printf("[debug] lastToken: %s\n", lastToken);
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] printout info of tempWorkinDir\n");
    printDEInfo(tempWorkingDir[0]);

    //4 If invalid, return -1 and print error message
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 182: invalid path...\n");
        return -1;
    }

    //5 If valid, get the directory index of tempWorkingDir (if index = -1, means directory full. Print msg and return -1)
    int DEindex = -1;
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        printf("[debug] tempWorkingDir[%d].name: %s\n", i, tempWorkingDir[i].name);
        if (strcmp(tempWorkingDir[i].name, "\0") == 0)
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

    //6 Create directory for lastToken
    printf("[debug] parent location: %d\n", tempWorkingDir[0].location);
    int newLocation = createDir(tempWorkingDir[0].location);
    if (newLocation == -1)
    {
        printf("[ERROR] fsDir.c line 211: createDir error...\n");
    }

    //7 Set the DE to the index and also update the parent information
    strcpy(tempWorkingDir[DEindex].name, lastToken);
    tempWorkingDir[DEindex].size = ((mfs_defaultDECount * sizeof(DE)) / mfs_blockSize + 1) * mfs_blockSize;
    tempWorkingDir[DEindex].location = newLocation;
    tempWorkingDir[DEindex].actualSize = sizeof(DE) * 2;
    tempWorkingDir[DEindex].blockCount = numOfBlockNeeded;
    tempWorkingDir[DEindex].isDir = 1;
    time(&tempWorkingDir[DEindex].createTime);
    time(&tempWorkingDir[DEindex].lastModTime);
    time(&tempWorkingDir[DEindex].lastAccessTime);
    printf("[debug] printout directory info for new directory\n");
    printDEInfo(tempWorkingDir[DEindex]);

    tempWorkingDir[0].actualSize += sizeof(DE);

    //8 Write into disk
    int blockCount = tempWorkingDir[0].size / mfs_blockSize;
    int ret = LBAwrite(tempWorkingDir, blockCount, tempWorkingDir[0].location);
    if (ret != blockCount)
    {
        printf("[ERROR] LBAwrite failed...\n");
        return -1;
    }

    //9 Free tempworkingDir and last token
    free(tempWorkingDir);
    free(lastToken);
    tempWorkingDir = NULL;
    lastToken = NULL;

    //10 Return 1
    return 1;
}

//finished implementing, still need to be tested
int fs_rmdir(const char *pathname)
{

    printf("\n--------- INSIDE THE RMDIR ---------\n");
    printf("pathname is %s before strcpy\n", pathname);
    //1 Convert path to char array (to avoid the const warning)
    char path[strlen(pathname)];
    strcpy(path, pathname);
    printf("path is %s\npathname is %s\n", path, pathname);
    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path, EXIST_DIR, tempWorkingDir, lastToken);
    printf("[debug] valid: %d\n", valid);
    printf("[debug] lastToken: %s\n", lastToken);
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] printout info of tempWorkinDir\n");
    printDEInfo(tempWorkingDir[0]);

    //4
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 305: invalid path...\n");
        return -1;
    }

    //
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        printf("finding the directory of last token...\n");
        if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
        {
            LBAread(tempWorkingDir, tempWorkingDir[i].size / mfs_blockSize, tempWorkingDir[i].location);
            break;
        }
    }

    printf("[debug] print the tempWorkingDir info after updating it to lastToken\n");
    printDEInfo(tempWorkingDir[0]);

    //5
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        //5
        if (strcmp(tempWorkingDir[i].name, "\0") != 0)
        {
            printf("[ERROR] fsDir.c line 348: %s is not empty...\n", lastToken);
            return -1;
        }
    }

    //6
    int ret = fs_delete((char *)pathname);

    //7
    if (ret == -1)
    {
        printf("[ERROR] fsDir.c line 270: delete failed...\n");
        return -1;
    }

    //(missing on plan)
    int blockCount = tempWorkingDir[0].size / mfs_blockSize;
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

    printf("\n--------- END THE RMDIR ---------\n");

    //9
    return 1;
}

fdDir *fs_opendir(const char *name)
{
    printf("\n--------- INSIDE THE OPENDIR ---------\n");
    printf("[debug] name is %s before strcpy\n", name);
    //1 Convert path to char array (to avoid the const warning)
    char path[strlen(name)];
    strcpy(path, name);
    printf("[debug] path is %s\nname is %s\n", path, name);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path, EXIST_DIR, tempWorkingDir, lastToken);
    printf("[debug] print out tempWorkingDir info\n");
    printDEInfo(tempWorkingDir[0]);
    printf("[debug] lastToken: %s\n", lastToken);

    //4
    if (!valid)
    {
        printf("[ERROR] Dir.c line 419: invalid path\n");
        return NULL;
    }

    //5
    printf("[debug] finding %s\n", lastToken);
    int DEindex;
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        printf("[debug] temp[%d] is %s\n", i, tempWorkingDir[i].name);
        if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
        {
            printf("[debug] found %s at %d\n", lastToken, i);
            DEindex = i;
            break;
        }
    }

    //special case for root
    if (strcmp(lastToken, "\0") == 0)
    {
        DEindex = 0;
    }

    printf("[debug] DEindex: %d\n", DEindex);
    printf("[debug] print out DE info in DEindex\n");
    printDEInfo(tempWorkingDir[DEindex]);

    //6
    fdDir *dirp = malloc(sizeof(dirp));

    //7
    dirp->d_reclen = tempWorkingDir[DEindex].size;
    dirp->directoryStartLocation = tempWorkingDir[DEindex].location;
    dirp->dirEntryPosition = DEindex;
    strcpy(dirp->d_name, tempWorkingDir[DEindex].name);
    if (tempWorkingDir[DEindex].isDir)
    {
        dirp->fileType = DT_DIR;
    }
    else
    {
        dirp->fileType = DT_REG;
    }

    free(tempWorkingDir);
    tempWorkingDir = NULL;
    free(lastToken);
    lastToken = NULL;

    printf("[debug] print out dirInfo\n");
    printfdDir(dirp);

    printf("\n--------- END THE OPENDIR ---------\n");

    //8
    return dirp;
}

struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{

    //printf("\n--------- INSIDE THE READDIR ---------\n");

    //1
    if (dirp == NULL)
    {
        printf("[ERROR] fsDir.c in opendir, line 452: invalid argument\n");
        return NULL;
    }

    /*if (dirp->dirEntryPosition > dirp->d_reclen / sizeof(DE))
    {
        printf("[debug] reached the end of directory\n");
        return NULL;
    }*/

    //2
    struct fs_diriteminfo *info = malloc(sizeof(struct fs_diriteminfo));

    //3 (read the working directory)
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *workingDir = malloc(mallocSize);
    LBAread(workingDir, numOfBlockNeeded, dirp->directoryStartLocation);

    while (strcmp(workingDir[dirp->dirEntryPosition].name, "\0") == 0 && dirp->dirEntryPosition < mfs_defaultDECount - 1)
    {
        dirp->dirEntryPosition++;
    }

    if (dirp->dirEntryPosition == mfs_defaultDECount - 1)
    {
        printf("[debug] end of the directory!\n");
        return NULL;
    }

    //3
    strcpy(info->d_name, workingDir[dirp->dirEntryPosition].name);
    info->d_reclen = dirp->d_reclen;
    info->fileType = dirp->fileType;

    //printf("[debug] print out dir item info\n");
    //printdirItemInfo(info);

    //4 (move to the next)
    dirp->dirEntryPosition++;

    //printf("\n--------- END THE READDIR ---------\n");

    //4
    return info;
}

int fs_closedir(fdDir *dirp)
{
    //1
    if (dirp == NULL)
    {
        printf("[ERROR] fsDir.c closedir, line 482: invalid argument\n");
        return -1;
    }

    //2
    free(dirp);
    dirp = NULL;

    //3
    return 1;
}

int fs_delete(char *filename)
{

    printf("\n--------- INSIDE THE DELETE ---------\n");
    printf("filename is %s before strcpy\n", filename);
    //1
    char path[strlen(filename)];
    strcpy(path, filename);
    printf("filename is %s before strcpy\n", filename);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path, EXIST_DIR | EXIST_FILE, tempWorkingDir, lastToken);
    printf("[debug] valid: %d\n", valid);
    printf("[debug] lastToken: %s\n", lastToken);
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] printout info of tempWorkinDir\n");
    printDEInfo(tempWorkingDir[0]);

    //4
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 376: invalid path...\n");
        return -1;
    }

    //5
    int DEindex = -1;
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        printf("[debug] tempWorkingDir[%d].name: %s\n", i, tempWorkingDir[i].name);
        if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
        {
            DEindex = i;
            break;
        }
    }

    printf("[debug] DEindex: %d\n", DEindex);

    //6
    int release_ret = releaseFreeSpace(tempWorkingDir[DEindex].location, tempWorkingDir[DEindex].size / mfs_blockSize);
    if (release_ret == -1)
    {
        printf("[ERROR] fsDir.c line 396: releaseFreeSpace() failed...\n");
        return -1;
    }

    //7
    strcpy(tempWorkingDir[DEindex].name, "\0");
    tempWorkingDir[DEindex].location = 0;
    tempWorkingDir[DEindex].size = 0;
    tempWorkingDir[DEindex].isDir = 0;
    tempWorkingDir[DEindex].createTime = 0;
    tempWorkingDir[DEindex].lastModTime = 0;
    tempWorkingDir[DEindex].lastAccessTime = 0;
    int lba_ret = LBAwrite(tempWorkingDir, tempWorkingDir[0].size / mfs_blockSize, tempWorkingDir[0].location);
    if (lba_ret != tempWorkingDir[0].size / mfs_blockSize)
    {
        printf("[ERROR] fsDir.c line 410: LBAwrite failed...\n");
        return -1;
    }

    printf("\n--------- END THE DELETE ---------\n");

    return 1;
}

int fs_isFile(char *path)
{

    printf("\n--------- INSIDE THE ISFILE ---------\n");

    //1 Convert path to char array (to avoid the const warning)
    char path_array[strlen(path)];
    strcpy(path_array, path);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path_array, EXIST_FILE, tempWorkingDir, lastToken);
    printf("[debug] valid: %d\n", valid);
    printf("[debug] lastToken: %s\n", lastToken);
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] printout info of tempWorkinDir\n");
    printDEInfo(tempWorkingDir[0]);

    printf("\n--------- END THE ISFILE ---------\n");

    return valid;
}

int fs_isDir(char *path)
{

    printf("\n--------- INSIDE THE ISDIR ---------\n");

    //1 Convert path to char array (to avoid the const warning)
    char path_array[strlen(path) + 1];
    strcpy(path_array, path);
    printf("[debug] Before calling pathParser, path is %s\npath_array is %s\n", path, path_array);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path_array, EXIST_DIR, tempWorkingDir, lastToken);
    printf("[debug] After calling pathParser, path is %s\npath_array is %s\n", path, path_array);
    printf("[debug] valid: %d\n", valid);
    printf("[debug] lastToken: %s\n", lastToken);
    printf("[debug] tempWorkingDir @ %p\n", tempWorkingDir);
    printf("[debug] printout info of tempWorkinDir\n");
    printDEInfo(tempWorkingDir[0]);

    printf("\n--------- END THE ISDIR ---------\n");

    return valid;
}

char *fs_getcwd(char *buf, size_t size)
{

    printf("\n--------- INSIDE THE GETCWD ---------\n");

    //1
    printf("[debug] size: %ld\n", size);
    char **tokenArray = malloc(size);
    int tokenCount = 0;
    int findNameOf = 0;
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);

    //2 temp = "bar"
    LBAread(tempWorkingDir, numOfBlockNeeded, mfs_cwd_location);
    findNameOf = tempWorkingDir[0].location;
    printf("[debug] print the init DE info of tempWorkingDir\n");
    printDEInfo(tempWorkingDir[0]);
    printf("[debug] print the parent DE info\n");
    printDEInfo(tempWorkingDir[1]);
    //"bar"
    //3
    while (tempWorkingDir[1].location != tempWorkingDir[0].location)
    {
        LBAread(tempWorkingDir, numOfBlockNeeded, tempWorkingDir[1].location); //"foo"
        printf("[debug] print the current tempWorkingDir DE info, should be same as parent\n");
        printDEInfo(tempWorkingDir[0]);
        for (int i = 2; i < mfs_defaultDECount; i++)
        {
            if (tempWorkingDir[i].location == findNameOf)
            {
                printf("[debug] found %s at %d\n", tempWorkingDir[i].name, i);
                printf("[debug] malloc size: %ld\n", sizeof(tokenArray) * strlen(tempWorkingDir[i].name) + 1);
                tokenArray[tokenCount] = malloc(sizeof(tokenArray) * strlen(tempWorkingDir[i].name) + 1);
                printf("[debug] sizeOf(tokenArray): %ld\n", strlen(tokenArray[0]));
                strcpy(tokenArray[tokenCount], tempWorkingDir[i].name);
                printf("[debug] tokenArray[%d]: %s\n", tokenCount, tokenArray[tokenCount]);
                tokenCount++;
                break;
            }
        }
        findNameOf = tempWorkingDir[0].location;
        printf("[debug] print the parent DE info (if location is 6, should stop)\n");
        printDEInfo(tempWorkingDir[1]);
    }

    //4
    char *path = malloc(size + tokenCount - 1);
    printf("[debug] tokenCount = %d\n", tokenCount);
    strcpy(path, "/"); //paht = "/"
    for (int i = tokenCount - 1; i >= 0; i--)
    {
        printf("[debug] path: %s\n[debug] tokenArray[%d]: %s\n", path, i, tokenArray[i]);
        strcat(path, tokenArray[i]); //path = "/foo"
        if (i > 0)
            strcat(path, "/");
        printf("[debug] current path is %s\n", path);
    }

    //5
    strcpy(buf, path);

    //6
    free(tempWorkingDir);
    free(tokenArray);
    tempWorkingDir = NULL;
    tokenArray = NULL;

    printf("\n--------- END THE GETCWD ---------\n");

    return path;
}

int fs_setcwd(char *buf)
{
    printf("\n--------- INSIDE THE SETCWD ---------\n");

    //1 Convert path to char array (to avoid the const warning)
    char path[strlen(buf)];
    strcpy(path, buf);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(path, EXIST_DIR, tempWorkingDir, lastToken);

    //4
    if (!valid)
    {
        printf("[ERROR] fsDir.c line 549: invalid path...\n");
        return -1;
    }

    //5
    for (int i = 0; i < mfs_defaultDECount; i++)
    {
        if (strcmp(tempWorkingDir[i].name, lastToken) == 0)
        {
            mfs_cwd_location = tempWorkingDir[i].location;
            break;
        }
    }
    printf("[debug] mfs_cwd_location: %d\n", mfs_cwd_location);
    printf("\n--------- END THE SETCWD ---------\n");

    //6
    free(tempWorkingDir);
    tempWorkingDir = NULL;
    free(lastToken);
    lastToken = NULL;

    //6
    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf)
{
    //1 Convert path to char array (to avoid the const warning)
    char convertedPath[strlen(path)];
    strcpy(convertedPath, path);

    //2
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    DE *tempWorkingDir = malloc(mallocSize);
    char *lastToken = malloc(256);

    //3
    bool valid = pathParser(convertedPath, EXIST_DIR | EXIST_FILE, tempWorkingDir, lastToken);

    //4
    if (!valid)
    {
        printf("[ERROR] fs_stat: invalid path\n");
        return -1;
    }

    //5 loop through the tempWorkingDir to find lastToken
    int DEindex;
    for (int i = 2; i < mfs_defaultDECount; i++)
    {
        if (strcmp(lastToken, tempWorkingDir[i].name) == 0)
        {
            DEindex = i;
            break;
        }
    }

    //6 fill in the buf
    buf->st_size = tempWorkingDir[DEindex].size;
    buf->st_blksize = mfs_blockSize;
    buf->st_blocks = numOfBlockNeeded;
    buf->st_createtime = tempWorkingDir[DEindex].createTime;
    buf->st_modtime = tempWorkingDir[DEindex].lastModTime;
    buf->st_accesstime = tempWorkingDir[DEindex].lastAccessTime;

    //7
    free(tempWorkingDir);
    tempWorkingDir = NULL;
    free(lastToken);
    lastToken = NULL;

    return 1;
}

bool allocateDirectory(DE *directory)
{
    //1
    int mallocSize = sizeof(DE) * mfs_defaultDECount;
    int numOfBlockNeeded = (mallocSize / mfs_blockSize) + 1;
    mallocSize = numOfBlockNeeded * mfs_blockSize;
    directory = malloc(mallocSize);

    if (directory == NULL)
    {
        printf("[ERROR] allocateDirectory: malloc failed...\n");
        return 0;
    }

    return 1;
}

void printDEInfo(DE de)
{
    printf("--- DE info ---\n");
    printf("- name: %s\n- size: %d\n- pointingLocation: %d\n- blockCount: %d\n- actual size: %d\n", de.name, de.size, de.location, de.blockCount, de.actualSize);
    printf("- isDir: %d\n- createTime: %s\n- lastMod: %s\n- lastAccess: %s\n", de.isDir, ctime(&de.createTime), ctime(&de.lastModTime), ctime(&de.lastAccessTime));
    printf("*note size of DE is: %ld\n", sizeof(DE));
}

void printdirItemInfo(struct fs_diriteminfo *info)
{
    printf("--- dir item info ---\n");
    printf("- d_name: %s\n- d_reclen: %d\n- fileType: %d\n",
           info->d_name, info->d_reclen, info->fileType);
}

void printfdDir(fdDir *dirp)
{
    printf("--- fdDir info ---\n");
    printf("- d_name: %s\n- d_reclen: %d\n- dirEntryPosition: %d\n- directoryStartLocation: %ld\n- fileType: %d\n",
           dirp->d_name, dirp->d_reclen, dirp->dirEntryPosition, dirp->directoryStartLocation, dirp->fileType);
}