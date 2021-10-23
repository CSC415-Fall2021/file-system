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

//init
int initFreeSpace(char *bitMapBuffer);

//allocate
int getFreeSpace(char *bitMapBuffer, int numOfBlocks);

//release
int releaseFreeSpace(char *bitMapBuffer, int numOfBlocks, int position);