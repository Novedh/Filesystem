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
 * Description::
 *
 **************************************************************/

#include "fsLow.h"



void setBit(int blockNum);
void clearBit(int blockNum);
int getBit(int blockNum);
int initFreeSpaceMap(uint64_t numberOfBlocks, uint64_t blockSize);
int allocateBlocks(uint64_t numBlocksRequested);
int freeBlocks(int index, int numBlocks);
