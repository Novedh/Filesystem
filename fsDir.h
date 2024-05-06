/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: fsDir.h
 *
 * Description:: file with DE struct and fuction declarations
 *
 **************************************************************/

#include "fsLow.h"
#include "mfs.h"
#include "bitMap.h"
#include "global.h"


#define MAX_ENTRIES 50



typedef struct ppRetStruct{
    char *lastElementName;
    int lastElementIndex;
    DE * Parent;

}ppRetStruct;

int createDirectory(int numEntries, DE *parent);
int findInDir(DE *parent, char *string);
DE *loadDir(DE *de);
DE *loadDirByLoc(int loc);
DE *getDEInfo(char * filename);
void loadRoot();
int parsePath(const char *path, ppRetStruct *ppInfo);
int findUnusedDE(DE *dir);
void writeDir(DE *de);
int fs_move(char *srcPath, char *destPath);
int makeFile(char *pathname, int fileSize);
