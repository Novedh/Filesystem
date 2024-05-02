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


void loadRoot(){
    int blocksNeeded = ((sizeof(DE) * MAX_ENTRIES) + vcb->blockSize) / vcb->blockSize;
    root = (DE*)malloc(vcb->blockSize*blocksNeeded);
    LBAread(root,blocksNeeded,vcb->rootStart);
    cwdString = "/";
    cwd = root;
}

int createDirectory(int numEntries, DE *parent)
{
    
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
    int blocksNeeded = ((sizeof(DE) * MAX_ENTRIES) + vcb->blockSize) / vcb->blockSize;
    int blockNum = de->loc;
    DE *dir = (DE*)malloc(vcb->blockSize*blocksNeeded);
    LBAread(dir, blocksNeeded,blockNum);
    return dir;
}

DE *loadDirByLoc(int loc)
{
    if (loc <0)
    {
        return NULL;
    }
    int blocksNeeded = ((sizeof(DE) * MAX_ENTRIES) + vcb->blockSize) / vcb->blockSize;
    DE *dir = (DE *)malloc(vcb->blockSize*blocksNeeded);
    LBAread(dir, blocksNeeded, loc);
    return dir;
}
//    printf("\n\n test: \n\n");

int parsePath( char *path, ppRetStruct *ppInfo){

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
    int blocksNeeded = ((sizeof(DE) * MAX_ENTRIES) + vcb->blockSize) / vcb->blockSize;
    LBAwrite(de,blocksNeeded,blockNum);

}


int fs_mkdir(const char *pathname, mode_t mode){
    ppRetStruct ppInfo;

    int res = parsePath((char*)pathname,&ppInfo);
    
    if(res == -1){
        return -1;
    }
    if(ppInfo.lastElementIndex!=-1){
        return -1;
    }
    int dirLoc = createDirectory(MAX_ENTRIES, ppInfo.Parent);
    printf("\ndeb: created Dir loc: %d\n",dirLoc);

    DE *newDir = loadDirByLoc(dirLoc);    
    int index = findUnusedDE(ppInfo.Parent);
    printf("deb: index: %d\n", index);
    strcpy(ppInfo.Parent[index].name, ppInfo.lastElementName);
    ppInfo.Parent[index].size = newDir[0].size;
    printf("deb: mkdir last element name: %s\n", ppInfo.lastElementName);
    printf("deb: mkdir new dir name in parent dir: %s\n\n", ppInfo.Parent[index].name);

    printf("deb: mkdir parent dir index 0 names: %s\n", ppInfo.Parent[0].name);
    printf("deb: mkdir parent dir index 1 names: %s\n", ppInfo.Parent[1].name);
    printf("deb: mkdir parent dir index 2 names: %s\n", ppInfo.Parent[2].name);
    printf("deb: mkdir parent dir index 3 names: %s\n", ppInfo.Parent[3].name);
    printf("deb: mkdir parent dir index 4 names: %s\n", ppInfo.Parent[4].name);
    printf("deb: mkdir parent dir index 5 names: %s\n", ppInfo.Parent[5].name);
    printf("deb: mkdir parent dir index 6 names: %s\n", ppInfo.Parent[6].name);
    printf("deb: mkdir parent dir index 7 names: %s\n", ppInfo.Parent[7].name);

    writeDir(ppInfo.Parent);
}

char *fs_getcwd(char *pathname, size_t size){
    strncpy(pathname, cwdString, size);
    return cwdString;
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
    printf("deb: rmdir start---\n");
    int res = parsePath(pathname,&ppInfo);

    if (res == -1)
    {
        printf("deb: rmdir res =-1---\n");

        return -1;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        printf("deb: rmdir index = -1 ---\n");

        return -1;
    }
    if (ppInfo.Parent[ppInfo.lastElementIndex].isDir == 0)
    {
        printf("deb: rmdir is file---\n");

        return -1;
    }
    printf("deb: rmdir point 1---\n");

    DE *rmDir = &ppInfo.Parent[ppInfo.lastElementIndex];
    int blockNum = rmDir[0].loc;
    int numBlocks = (rmDir[0].size + vcb->blockSize - 1) / vcb->blockSize;
    //free the blocks on freemap so that they can be reused;
    printf("deb: blocknum: %d\n number of blocks: %d\n",blockNum,numBlocks);
    freeBlocks(blockNum, numBlocks);

    printf("deb: rmdir point 2---\n");

    strcpy(rmDir->name, "");
    rmDir->size = 0;
    rmDir->loc = 0;
    rmDir->isDir = 0;

    DE *parentDir = ppInfo.Parent;
    strcpy(parentDir[ppInfo.lastElementIndex].name, "");
    strcpy(parentDir[0].name, ".");
    parentDir[0].loc = ppInfo.Parent[0].loc;
    parentDir[0].size = ppInfo.Parent[0].size;
    parentDir[1].loc = ppInfo.Parent[1].loc;
    parentDir[1].size = ppInfo.Parent[1].size;
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

    int numBlocks = (de->size + vcb->blockSize - 1) / vcb->blockSize;
    freeBlocks(de->loc, numBlocks);

    strcpy(de->name, "");
    de->size = 0;
    de->loc = 0;
    de->isDir = 0;

    writeDir(ppInfo.Parent);

    return 0;
}

fdDir *fs_opendir(const char *pathname)
{
    ppRetStruct ppInfo;
    int res = parsePath(pathname, &ppInfo);
    if (res == -1)
    {
        return NULL;
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return NULL;
    }
    if (ppInfo.Parent[ppInfo.lastElementIndex].isDir == 0)
    {
        return NULL;
    }

    fdDir *dirp = (fdDir *)malloc(sizeof(fdDir));
    if (dirp == NULL)
    {
        return NULL; 
    }

    dirp->d_reclen = sizeof(struct fs_diriteminfo);
    dirp->dirEntryPosition = 0;
    dirp->directory = loadDir(&ppInfo.Parent[ppInfo.lastElementIndex]);

    if (dirp->directory == NULL)
    {
        free(dirp); 
        return NULL;
    }
    dirp->di = (struct fs_diriteminfo *)malloc(sizeof(struct fs_diriteminfo));
    if (dirp->di == NULL)
    {
        free(dirp->directory); 
        free(dirp);           
        return NULL;          
    }

    return dirp;
}
