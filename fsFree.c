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

#include <string.h>
#include "fsFree.h"

//initialize the free space
int initFreeSpace(char *bitMapBuffer)
{
    printf("[debug] strlen(bitMapBuffer) = %ld\n", strlen(bitMapBuffer));

    int startLocation = -1; //still need to adjust
    for (int i = 0; i < 6; i++)
    {
        bitMapBuffer[i] = 0x00; //for VCB and bit map
    }

    for (int i = 6; i < strlen(bitMapBuffer); i++)
    {
        bitMapBuffer[i] = 0x01;
    }
}

//allocate
int getFreeSpace(char *bitMapBuffer, int numOfBlocks)
{
    int startLocation = -1;
    for (int i = 0; i < strlen(bitMapBuffer); i++)
    {
        if (bitMapBuffer[i] & 0X01)
        {
            printf("[debug] found free space!\n");
            startLocation = i;
            for (int j = 1; j < numOfBlocks; j++)
            {
                printf("[debug] double checking the rest of the blocks...\n");
                if (bitMapBuffer[++i] & 0x01)
                {
                    printf("[debug] the rest is not free...\n");
                    i = ++startLocation; //to keep finding next avaliable
                    startLocation = -1;  //set to the default
                    break;
                }
            }
        }
    }

    return startLocation;
}

//release
int releaseFreeSpace(char *bitMapBuffer, int numOfBlocks, int position)
{
}