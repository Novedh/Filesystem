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
 * Description:: Functions in charge of the functions of our bitmap
 * such as seting, clearing, geting bits and initaliazing our bitmap.
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

// to check hexdump Hexdump/hexdump.linuxM1 SampleVolume --start 2 --count 5
unsigned char *freeSpaceMap;

// Load the free space map into memory
void loadFSM()
{
    // If the free space is already allocated, free it
    if (freeSpaceMap != NULL)
    {
        free(freeSpaceMap);
        freeSpaceMap = NULL;
    }

    // Allocate memory for the map
    freeSpaceMap = calloc(5, vcb->blockSize);
    if (freeSpaceMap == NULL)
    {
        printf("failed to load Free map.\n");
    }

    // Read the free space map from storage to memory
    LBAread(freeSpaceMap, vcb->freeSize, 1);
}

// Set a bit in the free space map, marking the block it represents as used
void setBit(int blockNum)
{
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    freeSpaceMap[byteNum] = freeSpaceMap[byteNum] | (1 << bitNum);
}

// Clear a bit in the free space map, marking the block it represents as free
void clearBit(int blockNum)
{
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    freeSpaceMap[byteNum] = freeSpaceMap[byteNum] & ~(1 << bitNum);
}

// Get the value of a bit in the free space map
int getBit(int blockNum)
{
    if (freeSpaceMap == NULL)
    {
        return -1;
    }
    if (blockNum >= vcb->numberOfBlocks)
    {
        return -1;
    }
    int byteNum = blockNum / 8;
    int bitNum = blockNum % 8;
    return ((freeSpaceMap[byteNum] >> bitNum) & 1);
}

// Initialize the free space map
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

    return blocksNeeded;
}

// Allocate numBlocks
int allocateBlocks(uint64_t numBlocksRequested)
{
    uint64_t startFreeBlock = -1;
    uint64_t continuousFreeBlocks = 0;

    // Iterate through all blocks until continuous blocks of free space for request are found
    for (int i = 0; i < vcb->numberOfBlocks; i++)
    {
        // Check if current block is free
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
                // Mark series as used
                for (int j = startFreeBlock; j < startFreeBlock + numBlocksRequested; j++)
                {
                    setBit(j);
                }

                // Write updated Map to disk
                LBAwrite(freeSpaceMap, vcb->freeSize, 1);

                // Return first free block in series
                return startFreeBlock;
            }
        }
        // Current block is used, reset tracker
        else
        {
            continuousFreeBlocks = 0;
        }
    }
    // Return -1, No free space to accomodate call
    return -1;
}

// Free a series of allocated blocks
int freeBlocks(int index, int numBlocks)
{

    // Load freeSpace Map
    LBAread(freeSpaceMap, vcb->freeSize, 1);

    // Index is invalid
    if (index < 1)
    {
        return -1;
    }

    // Mark series of blocks as free
    for (int i = index; i < index + numBlocks; i++)
    {
        clearBit(i);
    }

    LBAwrite(freeSpaceMap, vcb->freeSize, 1);
}

// Update free space map and free from memory
void exitFreeMap()
{
    LBAwrite(freeSpaceMap, vcb->freeSize, 1);
    free(freeSpaceMap);
}