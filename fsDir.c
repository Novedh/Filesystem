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
char *cwdString;
extern VCB *vcb;
int blockSize = 0;



int createDirectory(int numEntries, DE *parent)
{
    
    printf("\n\nvcb blocksize: %ld\n\n",vcb->blockSize);

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
        cwdString ="/";
    }
    else
    {
        myDir[1].loc = parent[0].loc;
        myDir[1].size = parent[0].size;
    }

    LBAwrite(myDir, blocksNeeded, loc);
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

DE *loadDirByLoc(int loc)
{
    if (loc <0)
    {
        return NULL;
    }
    DE *dir = (DE *)malloc(blockSize);
    LBAread(dir, 1, loc);
    return dir;
}
//    printf("\n\n test: \n\n");

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

int findUnusedDE(DE *dir)
{
    for (int i = 2; i < MAX_ENTRIES; i++)
    {
        if (strcmp(dir[i].name, "") == 0)
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
    printf("\n\n test: %s", cwdString);

    int res = parsePath((char*)pathname,&ppInfo);
    printf("\n\n test: %s\n\n", ppInfo.lastElementName);

    if(res == -1){
        return -1;
    }
    if(ppInfo.lastElementIndex!=-1){
        return -1;
    }
    int dirLoc = createDirectory(MAX_ENTRIES, ppInfo.Parent);
    printf("\n\n test: %s\n\n", ppInfo.lastElementName);

    DE *newDir = loadDirByLoc(dirLoc);
    int index = findUnusedDE(ppInfo.Parent);
    strcpy(ppInfo.Parent[index].name,ppInfo.lastElementName);
    ppInfo.Parent[index].size = newDir[0].size;

    writeDir(ppInfo.Parent);
}

char *fs_getcwd(char *pathname, size_t size){
    strncpy(pathname, cwdString, size);
    return 0;
}

int fs_setcwd(char *pathname){
    ppRetStruct ppInfo;
    int res = parsePath((char *)pathname, &ppInfo);
    if (res == -1)
    {
        return -1;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return -1;
    }
    if(!ppInfo.Parent[ppInfo.lastElementIndex].isDir){
        return -1;
    }
    DE *temp = loadDir(&ppInfo.Parent[ppInfo.lastElementIndex]);

    if(cwd!=root){
        free(cwd);
    }
    cwd = temp;

    //update the stringcwd

    char *newPath;

    if(pathname[0]=='/'){
        newPath = strdup(pathname);
    }else{
        int len1 = strlen(cwdString);
        int len2 = strlen(pathname);
        newPath = malloc(len1+len2+2);
        strcpy(newPath,cwdString);
        if(newPath[len1-1]!= '/'){
            strcat(newPath,"/");
        }
        strcat(newPath,pathname);
    }
    
    char *tokenVector[MAX_ENTRIES];
    char *savePtr;
    char *token = strtok_r(newPath, "/", &savePtr);
    int index = 0;
    

    while(token!=NULL){
        if(strcmp(token, ".")==0){
            token = strtok_r(NULL, "/", &savePtr);
            continue;
        }else if(strcmp(token,"..")==0){
            if(index>0){
                index--;
            }
        }else{
            tokenVector[index] = token;
            index++;
        }
        token = strtok_r(NULL,"/",&savePtr);
    }

    char *returnPath = malloc(strlen(cwdString)+2);
    strcpy(returnPath,"/");
    for(int i =0; i<index;i++){
        strcat(returnPath,tokenVector[i]);
        strcat(returnPath,"/");
    }
    strcpy(cwdString, returnPath);
    free(returnPath);
    free(newPath);
    return 0;
}

int fs_rmdir(const char *pathname){
    ppRetStruct ppInfo;

    int res = parsePath(pathname,&ppInfo);

    if (res == -1)
    {
        return -1;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return -1;
    }
    if (ppInfo.Parent[ppInfo.lastElementIndex].isDir == 0)
    {
        return -1;
    }

    DE *rmDir = &ppInfo.Parent[ppInfo.lastElementIndex];
    int blockNum = rmDir[0].loc;
    int numBlocks = (rmDir[0].size + blockSize - 1) / blockSize;
    //free the blocks on freemap so that they can be reused;
    freeBlocks(blockNum, numBlocks);

    strcpy(rmDir->name, "");
    rmDir->size = 0;
    rmDir->loc = 0;
    rmDir->isDir = 0;

    int parentIndex = ppInfo.lastElementIndex;
    DE *parentDir = ppInfo.Parent;
    // "" means unused directory entry so we are clearing it to be reused
    strcpy(parentDir[parentIndex].name,"");

    writeDir(parentDir);

}

int fs_isFile(char *filename){
    ppRetStruct ppInfo;

    int res = parsePath(filename,&ppInfo);
    if(res ==-1){
        return 0;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return -1;
    }

    DE *de = &ppInfo.Parent[ppInfo.lastElementIndex];
    //  isDir 0 means file & 1 means directory
    if(de->isDir == 0){
        return 1;
    }else{
        return 0;
    }
    
}

int fs_isDir(char *pathname){

    ppRetStruct ppInfo;

    int res = parsePath(pathname, &ppInfo);
    if (res == -1)
    {
        return 0;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return -1;
    }

    DE *de = &ppInfo.Parent[ppInfo.lastElementIndex];
    //  isDir 0 means file & 1 means directory
    if (de->isDir == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int fs_delete(char *filename)
{
    ppRetStruct ppInfo;

    int res = parsePath(filename, &ppInfo);
    if (res == -1)
    {
        return -1;
    }
    if(ppInfo.lastElementIndex == -1){
        return -1;
    }
    //makes sure it is a file
    if(ppInfo.Parent[ppInfo.lastElementIndex].isDir == 1){
        return -1;
    }

    DE * de = &ppInfo.Parent[ppInfo.lastElementIndex];

    int numBlocks = (de->size + blockSize - 1) / blockSize;
    freeBlocks(de->loc, numBlocks);

    strcpy(de->name, "");
    de->size = 0;
    de->loc = 0;
    de->isDir = 0;

    writeDir(ppInfo.Parent);

    return 0;
}
