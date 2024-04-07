/**************************************************************
* Class::  CSC-415-0# Spring 2024
* Name::
* Student IDs::
* GitHub-Name::
* Group-Name::
* Project:: Basic File System
*
* File:: fsInit.c
*
* Description:: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/


#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"

#define SIGNATURE 0x41444f4a353134 // 415JODA


typedef struct VCB{
	uint64_t signature;
	uint64_t numberOfBlocks;
	uint64_t blockSize;
	uint64_t freeStart;
	uint64_t rootStart;
};

// TODO: Define Directory Entry (DE) Struct

int initFreeSpaceMap (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	int bytesNeeded = (numberOfBlocks + 7) / 8;
	int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
	char *freeSpaceMap = malloc(blocksNeeded * blockSize);

	for (int i = 0; i < blocksNeeded + 1; i++)
	{
		freeSpaceMap[i] = 1;
	}
	memset(freeSpaceMap + (blocksNeeded + 1), 0, (blocksNeeded * blockSize) - (blocksNeeded + 1));


	LBAwrite(freeSpaceMap, blocksNeeded, 1);
	return 1;
	}

	// TODO: Write Allocate Space Function
	// Input: num blocks requested
	// MUST Update freeSpaceMap with blocks used and write it to disk.
	// Return starting block number

	// TODO BUT NOT REQUIRED FOR M1: Write release space function

	// TODO: Write init rootDirectory
	// Return starting block number of root directory

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	struct VCB *vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	if(vcb->signature == SIGNATURE){
		printf("Volume already initialized! \n");
		free(vcb);
		return 0;
	}

	vcb->signature = SIGNATURE;
	vcb->numberOfBlocks = numberOfBlocks;
	vcb->blockSize = blockSize;
	vcb->freeStart = initFreeSpaceMap(numberOfBlocks, blockSize);
	vcb->rootStart = 6; // eventually replace with a initRootDirectory

	LBAwrite(vcb,1,0);

	free(vcb);


	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}