/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: bitMap.h
 *
 * Description:: Function declarations for our bitmap source file
 *
 **************************************************************/

#include "fsLow.h"
#include "global.h"

// Load the free space map into memory
void loadFSM();

// Set a bit in the free space map, marking the block it represents as used
void setBit(int blockNum);

// Clear a bit in the free space map, marking the block it represents as free
void clearBit(int blockNum);

// Get the value of a bit in the free space map
int getBit(int blockNum);

// Initialize the free space map
int initFreeSpaceMap(uint64_t numberOfBlocks, uint64_t blockSize);

// Allocate numBlocks
int allocateBlocks(uint64_t numBlocksRequested);

// Free a series of allocated blocks
int freeBlocks(int index, int numBlocks);

// Update free space map and free from memory
void exitFreeMap();