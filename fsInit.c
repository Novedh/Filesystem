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

#define SIGNATURE 0x41444f4a353134 // 415JODA
#define MAX_FILENAME_LEN 255


typedef struct VCB{
	uint64_t signature;
	uint64_t numberOfBlocks;
	uint64_t blockSize;
	uint64_t freeStart;
	uint64_t rootStart;
}VCB;

// TODO: Define Directory Entry (DE) Struct
typedef struct DE
{
	char name[MAX_FILENAME_LEN]; // user-readable filename
	time_t createTime;			 // timestamp for file creation time
	time_t modTime;				 // timestamp for last modification time
	time_t accessTime;			 // timestamp for last access time
	int fileID;					 // a unique identifier for the file
	int loc;					 // represents the location of the file on the disk
	unsigned int size;			 // represents the size of the file in bytes
	unsigned int isDir;			 // 0 if it is a file, 1 if it is a directory
}DE;

unsigned char * freeSpaceMap;
struct VCB *vcb;


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

int initFreeSpaceMap (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	int bytesNeeded = (numberOfBlocks + 7) / 8;
	int blocksNeeded = (bytesNeeded + (blockSize - 1)) / blockSize;
	freeSpaceMap = (unsigned char*)malloc(blocksNeeded * blockSize);

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
				LBAwrite(freeSpaceMap,vcb->freeStart - 1,1);
				
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
int freeBlocks(int index, int numBlocks){
	LBAread(freeSpaceMap, vcb->freeStart - 1, 1);
	if(index<1){
		return -1;
	}
	for(int i = index; i < index + numBlocks ; i++){
		clearBit(i);
	}
	LBAwrite(freeSpaceMap, vcb->freeStart, 1);
}

	// TODO: Write init rootDirectory
	// Return starting block number of root directory
int createDirectory(int numEntries, DE *parent){
	int spaceNeeded = numEntries * sizeof(DE);
	int blocksNeeded = (spaceNeeded + vcb->blockSize - 1) / vcb->blockSize;
	int bytesNeeded = blocksNeeded * vcb->blockSize;
	int actEntries = bytesNeeded / sizeof(DE);
	int loc = allocateBlocks(blocksNeeded);
	DE *myDir = (DE *)malloc(bytesNeeded);

	//empty name means unused DE
	for(int i =2;i<actEntries;i++){
		strcpy(myDir[i].name, "");
	}

	strcpy(myDir[0].name, ".");
	myDir[0].size = actEntries * sizeof(DE);
	myDir[0].loc = loc;
	myDir[0].isDir= 1 ;

	strcpy(myDir[1].name, "..");
	myDir[1].isDir = 1;
	//if we are in root
	if(parent == NULL){
		myDir[1].loc = myDir[0].loc;
		myDir[1].size = myDir[0].size;
	}else{
		myDir[1].loc = parent[0].loc;
		myDir[1].size = parent[0].size;
	}

	LBAwrite(myDir,6,loc);
	free(myDir);

	return loc;

}

int initFileSystem (uint64_t numberOfBlocks, uint64_t blockSize)
	{
	printf ("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */
	
	vcb = malloc(blockSize);
	LBAread(vcb, 1, 0);

	if(vcb->signature == SIGNATURE){
		printf("Volume already initialized! \n");
		free(vcb);
		return 0;
	}

	vcb->signature = SIGNATURE;
	vcb->numberOfBlocks = numberOfBlocks;
	vcb->blockSize = blockSize;
	vcb->freeStart = initFreeSpaceMap(vcb->numberOfBlocks, vcb->blockSize);
	vcb->rootStart = createDirectory(50,NULL); // eventually replace with a initRootDirectory

	LBAwrite(vcb,1,0);

	free(vcb);

	return 0;
	}
	
	
void exitFileSystem ()
	{
	printf ("System exiting\n");
	}
