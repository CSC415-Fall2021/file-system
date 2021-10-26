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

//create directory, returns the starting block
//return value: positive num -> success, -1 -> failed
int createDir(int parentLocation, int DEcount, int blockSize, VCB *vcb);