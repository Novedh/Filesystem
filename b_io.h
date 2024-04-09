/**************************************************************
* Class::  CSC-415-01 Spring 2024
* Name:: James Dixon, Justin Hellweg, Devon Huang, Omar Thiongane
* Student IDs:: 922440659, 922692586, 916940666, 922164097
* GitHub-Name:: JD499, Jus1927, Novedh, ot409
* Group-Name:: JODA
* Project:: Basic File System
*
* File:: b_io.h
*
* Description:: Interface of basic I/O Operations
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif

