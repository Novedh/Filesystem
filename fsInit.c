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

#define SIGNATURE1 0x353134435343 // CSC415
#define SIGNATURE2 0x41444f4a // JODA

typedef struct VCB{
	uint64_t signaturePt1;
	uint64_t signaturePt2;
	uint64_t rootStart;
	uint64_t freeStart;
};

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	struct VCB *vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	if(vcb->signaturePt1 == SIGNATURE1 && vcb->signaturePt2 == SIGNATURE2){
		printf("Volume already initialized! \n");
		free(vcb);
		return 0;
	}

	vcb->signaturePt1 = SIGNATURE1;
	vcb->signaturePt2 = SIGNATURE2;
	vcb->freeStart = 1;
	vcb->rootStart = 6;

	LBAwrite(vcb,1,0);

	free(vcb);

	char *freeSpaceMap = malloc(5 * blockSize);

	for (int i = 0; i < 6; i++)
	{
		freeSpaceMap[i] = 1;
	}
	memset(freeSpaceMap + 6, 0, 5 * blockSize - 6);


	LBAwrite(freeSpaceMap, 5, 1);
	vcb->freeStart = 1;

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}