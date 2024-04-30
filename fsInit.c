/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
* Student IDs:: 922440659, 922692586, 916940666, 922164097
* GitHub-Name:: JD499, Jus1927, Novedh, ot409
* Group-Name:: JODA
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
#include <time.h>

#include "fsLow.h"
#include "mfs.h"
#include "fsDir.h"
#include "bitMap.h"
#include "global.h"

#define SIGNATURE 0x41444f4a353134 // 415JODA

VCB *vcb;

// TODO: Define Directory Entry (DE) Struct

	// TODO: Write init rootDirectory
	// Return starting block number of root directory

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	if(vcb->signature == SIGNATURE){
		printf("Volume already initialized! \n");
		loadFSM();
		loadRoot();
		return 0;
	}

	vcb->signature = SIGNATURE;
	vcb->numberOfBlocks = numberOfBlocks;
	vcb->blockSize = blockSize;
	vcb->freeStart = 1;
	vcb->freeSize = initFreeSpaceMap(vcb->numberOfBlocks, vcb->blockSize);
	vcb->rootStart = createDirectory(MAX_ENTRIES, NULL); // eventually replace with a initRootDirectory

	LBAwrite(vcb,1,0);

	return 0;
	}
	
	
void exitFileSystem ()
	{
		exitFreeMap();
		LBAwrite(vcb, 1, 0);
		free(vcb);
		printf("System exiting\n");
	}
