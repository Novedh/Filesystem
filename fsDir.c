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

// this function will load root from disk back into memory when the FS is restarted
// and also allocate and set the cwd to root
void loadRoot(){
    int blocksNeeded = ((sizeof(DE) * MAX_ENTRIES) + vcb->blockSize) / vcb->blockSize;
    root = (DE*)malloc(vcb->blockSize*blocksNeeded);
    cwdString = (char*)malloc(MAX_FILENAME_LEN);
    cwd = (DE *)malloc(vcb->blockSize * blocksNeeded);
    LBAread(root,blocksNeeded,vcb->rootStart);
    strcpy(cwdString,"/");
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

int createFile(int fileSize, DE *parent)
{

    int blocksNeeded = (fileSize + vcb->blockSize - 1) / vcb->blockSize;
    int bytesNeeded = blocksNeeded * vcb->blockSize;

    int loc = allocateBlocks(blocksNeeded);
    DE *myFile = (DE *)malloc(sizeof(DE));

    strcpy(myFile->name, "myFile");
    myFile->size = fileSize;
    myFile->loc = loc;
    myFile->isDir = 0;

    LBAwrite(myFile, blocksNeeded, loc);
    free(myFile);
    return loc;
}

int makeFile(char *pathname, int fileSize)
{
    ppRetStruct ppInfo;

    int res = parsePath((char *)pathname, &ppInfo);

    if (res == -1)
    {
        return -1;
    }
    if (ppInfo.lastElementIndex != -1)
    {
        return -1;
    }

    int fileLoc = createFile(fileSize, ppInfo.Parent);

    DE *newFile = loadDirByLoc(fileLoc);
    int index = findUnusedDE(ppInfo.Parent);

    strcpy(ppInfo.Parent[index].name, ppInfo.lastElementName);
    ppInfo.Parent[index].size = newFile[0].size;
    ppInfo.Parent[index].isDir = newFile[0].isDir;
    ppInfo.Parent[index].loc = newFile[0].loc;
    ppInfo.Parent[index].createTime = newFile[0].createTime;

    writeDir(ppInfo.Parent);
    return fileLoc;
}

DE *getDEInfo(char *filename)
{
    ppRetStruct ppInfo;

    int res = parsePath(filename, &ppInfo);
    if (res == -1)
    {
        return NULL; // Invalid Path
    }
    if (ppInfo.lastElementIndex == -1)
    {
        return NULL; // Not Found
    }

    DE *de = &ppInfo.Parent[ppInfo.lastElementIndex];

    return de;
}

// note: "." dirIndex[0] points to itself
// note: ".." direIndex[1] points to prev

// returns the DE location of a file/directory within the given directory
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

// these functions load the directory from disk to memory
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


// alters the ppInfo struct passed in the function to store needed information for directory
// traversing, creating, and removing DE's in the filesystem.
int parsePath( const char *path, ppRetStruct *ppInfo){

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
            ppInfo->lastElementIndex=-1;
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
            printf("Could not find in Dir \n");
            return -1;
        }
        if (&parent[index].isDir == 0)
        {
            printf("Parent is not a directory\n");
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

// makes a new directory and updates the parent directory 
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

    DE *newDir = loadDirByLoc(dirLoc);    
    int index = findUnusedDE(ppInfo.Parent);
    strcpy(ppInfo.Parent[index].name, ppInfo.lastElementName);
    ppInfo.Parent[index].size = newDir[0].size;
    ppInfo.Parent[index].isDir = newDir[0].isDir;
    ppInfo.Parent[index].loc = newDir[0].loc;
    ppInfo.Parent[index].createTime = newDir[0].createTime;
    ppInfo.Parent[index].size = newDir[0].size;
 
    writeDir(ppInfo.Parent);
}
// returns the path that the filesystem is currently on
char *fs_getcwd(char *pathname, size_t size){
    strncpy(pathname, cwdString, size);
    return cwdString;
}

// for cd, used to change the cwd and update stringcwd
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

    char *returnPath = (char *)malloc(MAX_FILENAME_LEN);
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

// removes the direcotry from the parent and frees the blocks 
// associated with it so they may be reused
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
    int numBlocks = (rmDir[0].size + vcb->blockSize - 1) / vcb->blockSize;
    //free the blocks on freemap so that they can be reused;
    freeBlocks(blockNum, numBlocks);


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

// removed the file from parent direcotry and frees blocks associated with it
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

// opens the directory so that it may be read and returns a pointer to that directory
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

// this reads and returns the next occupied DE from the directory
struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    if(dirp == NULL){
        return NULL;
    }
    if(dirp->directory == NULL){
        return NULL;
    }

    DE *dir = dirp->directory;

    if(dirp->dirEntryPosition >= MAX_ENTRIES){
        return NULL;
    }
    while(strcmp(dir[dirp->dirEntryPosition].name,"")==0){
        if(dirp->dirEntryPosition>=MAX_ENTRIES){
            return NULL;
        }
        dirp->dirEntryPosition++;
    }

    struct fs_diriteminfo * diriteminfo = dirp->di;
    strcpy(diriteminfo->d_name,dir[dirp->dirEntryPosition].name);
    diriteminfo->d_reclen = dir[dirp->dirEntryPosition].size;
    diriteminfo->fileType = dir[dirp->dirEntryPosition].isDir;

    dirp->dirEntryPosition++;
    return diriteminfo;
}


int fs_closedir(fdDir *dirp){
    if(dirp ==NULL){
        return -1;
    }
    if(dirp->directory){
        free(dirp->directory);
    }
    if(dirp->di){
        free(dirp->di);
    }
    free(dirp);
    return 0;
}

int fs_move(char *srcPath, char *destPath)
{
    ppRetStruct srcppInfo;
    int res = parsePath(srcPath, &srcppInfo);
    if (res == -1)
    {
        return -1;
    }
    if (srcppInfo.lastElementIndex == -1)
    {
        return -1;
    }

    DE *srcde = &srcppInfo.Parent[srcppInfo.lastElementIndex];

    ppRetStruct destppInfo;
    res = parsePath(destPath, &destppInfo);
    if (res == -1)
    {
        return -1;
    }
    if (destppInfo.lastElementIndex == -1)
    {
        return -1;
    }

    DE *destde = loadDir(&destppInfo.Parent[destppInfo.lastElementIndex]);
    int index = findUnusedDE(destde);

    strcpy(destde[index].name, srcde->name);
    destde[index].size = srcde->size;
    destde[index].loc = srcde->loc;
    destde[index].isDir = srcde->isDir;
    destde[index].createTime = srcde->createTime;

    strcpy(srcde->name, "");
    srcde[index].size = 0;
    srcde[index].loc = 0;
    srcde[index].isDir = 0;
    srcde[index].createTime = 0;

    writeDir(destde);
    writeDir(srcppInfo.Parent);

    return 0;
}

int fs_stat(const char *path, struct fs_stat *buf){

    ppRetStruct ppInfo;
    int res = parsePath(path, &ppInfo);
    if(res == -1){
        return -1;
    }
    if(ppInfo.lastElementIndex == -1){
        return -1;
    }

    DE *de = &ppInfo.Parent[ppInfo.lastElementIndex];

    buf->st_size = de->size;
    buf->st_accesstime = de->accessTime;
    buf->st_modtime = de->modTime;
    buf->st_createtime = de->createTime;

    return 0;
}