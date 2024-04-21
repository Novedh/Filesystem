/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: bitMap.c
 *
 * Description::
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "global.h"
#include "bitMap.h"


unsigned char *freeSpaceMap;

void setBit(int blockNum)
{
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    freeSpaceMap[byteNum] = freeSpaceMap[byteNum] | (1 << bitNum);
}

void clearBit(int blockNum)
{
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    freeSpaceMap[byteNum] = freeSpaceMap[byteNum] & ~(1 << bitNum);
}

int getBit(int blockNum)
{
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    return ((freeSpaceMap[byteNum] >> bitNum) & 1);
}

int initFreeSpaceMap(uint64_t numberOfBlocks, uint64_t blockSize)
{
    int bytesNeeded = (numberOfBlocks + 7) / 8;
    int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
    freeSpaceMap = (unsigned char *)malloc(blocksNeeded * blockSize);

    // Initialize freeSpaceMap to 0 ie all free
    memset(freeSpaceMap, 0, blocksNeeded * blockSize);

    // Set first bit as used for VCB
    setBit(0);
    // Set bits used for freeSpaceMap
    for (int i = 1; i <= blocksNeeded; i++)
    {
        setBit(i);
    }

    LBAwrite(freeSpaceMap, blocksNeeded, 1);

    return blocksNeeded + 1;
}

int allocateBlocks(uint64_t numBlocksRequested)
{
    uint64_t startFreeBlock = -1;
    uint64_t continuousFreeBlocks = 0;

    for (int i = 0; i < vcb->numberOfBlocks; i++)
    {
        // Find a free block
        if (getBit(i) == 0)
        {
            // Track first free block in series
            if (continuousFreeBlocks == 0)
            {
                startFreeBlock = i;
            }
            // Track number of free blocks in series
            continuousFreeBlocks++;

            // Found a series of free blocks that matches request
            if (continuousFreeBlocks == numBlocksRequested)
            {
                // Update BitMap
                for (int j = startFreeBlock; j < startFreeBlock + numBlocksRequested; j++)
                {
                    setBit(j);
                }

                // Write updated Map to disk
                LBAwrite(freeSpaceMap, vcb->freeStart - 1, 1);

                // Return first free block in series
                return startFreeBlock;
            }
        }
        // Reset Series
        else
        {
            continuousFreeBlocks = 0;
        }
    }
    // Return -1, No free space to accomodate call
    return -1;
}

// TODO BUT NOT REQUIRED FOR M1: Write release space function
int freeBlocks(int index, int numBlocks)
{
    LBAread(freeSpaceMap, vcb->freeStart - 1, 1);
    if (index < 1)
    {
        return -1;
    }
    for (int i = index; i < index + numBlocks; i++)
    {
        clearBit(i);
    }
    LBAwrite(freeSpaceMap, vcb->freeStart, 1);
}