#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include "fsLow.h"
#include "dirctoryAndVCB.h"

// dirctory 
#define DIRFLAG_FILE 0x00000001
#define DIRFLAG_UNUSED 0x00000002
#define DIRFLAG_SPECIAL 0x00000004
#define DERFILE_DIRECTORY 0x00000008
#define AVGDIRECTORYENTRIES 50
#define CONTIG 1
#define NOCONTIG 0

//video 3 for file directory open
#define FDOPENINUSE 0x00000001
#define FDOPENFREE 0x00000002
#define FDOPENMAX 50
#define FDOPENFORWRITE 0x00000010
#define FDOPENFORREAD 0x00000020

typedef struct  openFileEntry
{
	int flags;
	uint64_t pointer;
	uint64_t size;
	uint64_t Id;
	char *filebuffer;
} openFileEntry, *openFileEntry_p;

openFileEntry * openFileList;

// open file 
int fsOpen(char * filename, int method)
{
	int fd;
	for(int i =0; i < FDOPENMAX; i++)
	{
		if(openFileList[i].flags == FDOPENFREE)
		{
			fd =1;
			break;
		}
	}

	openFileList[i].flags = FDOPENINUSE | FDOPENFORREAD | FDOPENFORWRITE ;
	openFileList[i].filebuffer = malloc (vcurrentVCB_p->blockSize *2);
	openFileList[i].pointer =0;
	openFileList[i].size=0;
	return (fd);
}

#define SEEK_CUR 1
#define SEEK_POS 2
#define SEEK_END 3

int fsSeek( int fd, uint64_t position, int method)
{
	if(fd >= FDOPENMAX)
		return -1;
	if((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE)
		return -1;
	switch (method)
	{
		case SEEK_CUR:
			openFileList[fd].position += position;
			break;
		case SEEK_POS:
			openFileList[fd].position = position;	
			break;
		case SEEK_END:
			openFileList[fd].position = openFileList[fd].size +  position;
			break;

		default:
			break;		
	}
	return (openFileList[fd].position);
}

uint64_t fsWrite( int fd, char * src, uint64_t length)
{
	if (fd >=  FDOPENMAX)
		return -1;
	if ((openFileList[fd].flags & FDOPENINUSE) != FDOPENINUSE)
		return -1;
	uint64_t currentBlock = openFileList[fd].position / vcurrentVCB_p->blockSize;
	uint64_t currentOffset = openFileList[fd].position % vcurrentVCB_p->blockSize;

	if (length + currentOffset < vcurrentVCB_p->blockSize)
	{
		memcpy ( openFileList[fd].filebuffer +  currentOffset, src, length);
	}
	else if (length + currentOffset < (vcurrentVCB_p->blockSize *2))
	{
		memcpy ( openFileList[fd].filebuffer +  currentOffset, src, length);

		// writeblck = translateFileBlock(fd, currentBlock);
		LBAwrite(filebuffer, 1, currentBlock + openFileList[fd].blockStart);
		memcpy (filebuffer, filebuffer + vcurrentVCB_p->blockSize, vcurrentVCB_p->blockSize);
	}
	else
	{
		//need to be filled
	}

	openFileList[fd].position = openFileList[fd].position + length;
	currentBlock = openFileList[fd].position /vcurrentVCB_p->blockSize;
	currentOffset = openFileList[fd].position % vcurrentVCB_p->blockSize; 
}