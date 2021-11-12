/**************************************************************
* Class:  CSC-415-02&03  Fall 2021
* Names: Tun-Ni Chiang, Jiasheng Li, Christopher Ling, Shixin Wang
* Student IDs: 921458769, 916473043, 918266861, 918663491
* GitHub Name: tunni-chiang, jiasheng-li, dslayer1392, uyguyguy
* Group Name: Bug Master
* Project: Basic File System
*
* File: fsDir.h
*
* Description: Main driver for file system assignment.
*
**************************************************************/

#include "fsFree.h"
//#include "mfs.h"

#define NOT_EXIST 0x00000000
#define EXIST_FILE 0x00000001
#define EXIST_DIR 0x00000002
#define EXIST 0x00000004

int mfs_defaultDECount;

//job: create directory, returns the starting block
//return value: positive num -> success, -1 -> failed
int createDir(int parentLocation);

//job: tokenize the path into an array of token
//then return back the working directory through param,
//and check validation of given condition
//returns 0 means invalid, 1 means valid
bool pathParser(char *path, unsigned char condition, DE *tempWorkingDir, char *lastToken);

void printDEInfo(DE de);