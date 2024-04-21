/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: global.h
 *
 * Description:: file with vcb structure
 *
 **************************************************************/

#include <stdint.h>

#ifndef GLOBAL_H
#define GLOBAL_H

#define MAX_FILENAME_LEN 255

typedef struct VCB
{
    uint64_t signature;
    uint64_t numberOfBlocks;
    uint64_t blockSize;
    uint64_t freeStart;
    uint64_t rootStart;

} VCB;

typedef struct DE
{
    char name[MAX_FILENAME_LEN]; // user-readable filename
    time_t createTime;           // timestamp for file creation time
    time_t modTime;              // timestamp for last modification time
    time_t accessTime;           // timestamp for last access time
    int fileID;                  // a unique identifier for the file
    int loc;                     // represents the location of the file on the disk
    unsigned int size;           // represents the size of the file in bytes
    unsigned int isDir;          // 0 if it is a file, 1 if it is a directory
} DE;

extern VCB *vcb;
extern DE *root;
extern DE *cwd;


#endif