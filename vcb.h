/**************************************************************
 * Class::  CSC-415-01 Spring 2024
 * Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
 * Student IDs:: 922440659, 922692586, 916940666, 922164097
 * GitHub-Name:: JD499, Jus1927, Novedh, ot409
 * Group-Name:: JODA
 * Project:: Basic File System
 *
 * File:: vcb.h
 *
 * Description:: file with vcb structure
 *
 **************************************************************/

#include <stdint.h>

typedef struct VCB
{
    uint64_t signature;
    uint64_t numberOfBlocks;
    uint64_t blockSize;
    uint64_t freeStart;
    uint64_t rootStart;

} VCB;