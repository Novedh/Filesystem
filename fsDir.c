/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: fsDir.c
 *
 * Description::
 *
 **************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include "global.h"
#include "fsDir.h"
#include "mfs.h"

//pointer to current directory stored in memory
DE *root;
DE *cwd;

int blockSize;


int createDirectory(int numEntries, DE *parent)
{
    blockSize = vcb->blockSize;
    int spaceNeeded = numEntries * sizeof(DE);
    int blocksNeeded = (spaceNeeded + vcb->blockSize - 1) / vcb->blockSize;
    int bytesNeeded = blocksNeeded * vcb->blockSize;
    int actEntries = bytesNeeded / sizeof(DE);
    int loc = allocateBlocks(blocksNeeded);
    DE *myDir = (DE *)malloc(bytesNeeded);

    // empty name means unused DE
    for (int i = 2; i < actEntries; i++)
    {
        strcpy(myDir[i].name, "");
    }

    strcpy(myDir[0].name, ".");
    myDir[0].size = actEntries * sizeof(DE);
    myDir[0].loc = loc;
    myDir[0].isDir = 1;

    strcpy(myDir[1].name, "..");
    myDir[1].isDir = 1;
    // if we are in root
    if (parent == NULL)
    {
        myDir[1].loc = myDir[0].loc;
        myDir[1].size = myDir[0].size;
        root = myDir;
        cwd = myDir;
    }
    else
    {
        myDir[1].loc = parent[0].loc;
        myDir[1].size = parent[0].size;
    }

    LBAwrite(myDir, 6, loc);
    free(myDir);

    return loc;
}


int findInDir(DE *parent, char *string){
    if(parent == NULL || string == NULL){
        return -1;
    }
    for (int i = 0; i < MAX_ENTRIES;i++){
        if (strcmp(parent[i].name, string) == 0){
            return i;
        }
    }
    return -1; //not found
}

DE *loadDir(DE *de){
    if (de == NULL){
        return NULL;
    }
    int blockNum = de->loc;
    DE *dir = (DE*)malloc(blockSize);
    LBAread(dir, 1,blockNum);
    return dir;
}

int parsePath(char *path, ppRetStruct *ppInfo){

    if(path == NULL){
        return 1;
    }
    if(ppInfo == NULL){
        return 1;
    }
    DE *startParent;
    //means given absolute path
    if(path[0]=='/'){
        startParent = root;
    }else{
        startParent = cwd;
    }
    DE *parent = startParent;

    char *token = strtok(path,"/");
    if(token == NULL){
        if(path[0]!='/'){
            return -1;
        }
        else{
            ppInfo->lastElementIndex=-2;
            ppInfo->lastElementName = NULL;
            ppInfo->Parent = parent;
            return 0;
        }
    }
    while(1){
        int index = findInDir(parent,token);
        char *token2 = strtok(NULL,"/");
        if (token2 == NULL)
        {
            ppInfo->lastElementName = token;
            ppInfo->lastElementIndex = index;
            ppInfo->Parent = parent;
            return 0;
        }
        if (index == -1)
        {
            return -1;
        }
        if (&parent[index].isDir == 0)
        {
            return -1;
        }
        DE *tempParent = loadDir(&parent[index]);
        if (parent != startParent)
        {
            free(parent);
        }
        parent = tempParent;
        token = token2;
    }
    
    
}

int findUnusedDE(DE *de)
{
    for (int i = 2; i < MAX_ENTRIES; i++)
    {
        if (strcmp(de[i].name, "") == 0)
        {
            return i;
        }
    }
    return -1; // no free directory entries
}

void writeDir(DE *de){
    if(de == NULL){
        return;
    }
    int blockNum = de->loc;

    LBAwrite(de,1,blockNum);

}

int fs_mkdir(const char *pathname, mode_t mode){
    ppRetStruct ppInfo;
    DE *res = parsePath(pathname,&ppInfo);
    if(res == -1){
        return -1;
    }
    if(ppInfo.lastElementIndex!=-1){
        return -1;
    }
    DE *newDir = createDirectory(MAX_ENTRIES,ppInfo.Parent);
    int index = findUnusedDE(ppInfo.Parent);
    strcpy(ppInfo.Parent[index].name,ppInfo.lastElementName);
    ppInfo.Parent[index].size = newDir[0].size;

    writeDir(ppInfo.Parent);
}