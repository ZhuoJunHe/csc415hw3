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

typedef struct dirEntry
{
	uint64_t Id;
	char name[128];
	uint64_t date;
	uint64_t location;
	uint64_t sizeInBytes;
	uint32_t flags;
}dirEntry, * dirEntry_p;

// volume control block
typedef struct vcb_t
{
	uint64_t volumeSize;
	uint64_t blockSize;
	uint64_t numberOfBlocks;
	uint64_t freeBlockLocation;
	uint64_t freeBlockBlocks;
	uint64_t freeBlockLastAllocatedBit;
	uint64_t freeBlockEndBlocksRemaining;
	uint64_t freeBlockTotalFreeBlocks;
	char * freeBuffer;
	uint64_t rootDirectoryStart;
	uint64_t nextIdToIssue;
} vcb_t, * vcb_p;

vcb_p currentVCB_p;


void initRootDir (uint64_t startLocation, uint64_t blockSize)
{
	dirEntry_p rootDirBuffer_p;

	uint64_t entrySize = sizeof(dirEntry);
	uint64_t bytesNeeded = AVGDIRECTORYENTRY * entrySize;
	uint64_t blocksNeeded =(bytesNeeded + (blockSize -1)) / blockSize;
	uint64_t actualDirEntries = (blocksNeeded * blockSize) / entrySize;

	printf("for %d entries, we need %llu bytes, each entry is %llu bytes\n", AVGDIRECTORYENTRY, bytesNeeded, entrySize);
	printf("Actual directory entries = %llu\n", actualDirEntries);

	//asking for freesystem
	uint64_t startLocation = getFreeSpace (blocksNeeded, CONTIG);

	rootDirBuffer_p = malloc(blocksNeeded * blockSize);

	//init all the directory entries
	for (int i =0; i< actualDirEntries; i++)
	{
		rootDirBuffer_p[i]->id =0;
		rootDirBuffer_p[i]->flags = DIRFLAG_UNUSED;
		strcpy (rootDirBuffer_p[i]->name, "");
		rootDirBuffer_p[i]->date = 0;
		rootDirBuffer_p[i]->location =0;
		rootDirBuffer_p[i]->sizeInBytes =0;
	}

	rootDirBuffer_p[0]->id =1000;
	rootDirBuffer_p[0]->flags = 0;
	strcpy (rootDirBuffer_p[i]->name, "..");
	rootDirBuffer_p[0]->date = 1234;
	rootDirBuffer_p[0]->location = startLocation;
	rootDirBuffer_p[0]->sizeInBytes =actualDirEntries * entrySize;

	LBAwrite(rootDirBuffer_p, blocksNeeded, startLocation);

}

uint64_t getNewFileID()
{
	uint64_t retVal;
	retVal = currentVCB_p-> nextIdToIssue;
	currentVCB_p->nextIdToIssue = currentVCB_p->nextIdToIssue + FILEIDINCREMENT;
	return (reVal);
}