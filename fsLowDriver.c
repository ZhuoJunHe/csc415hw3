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


// dirctory 
#define DIRFLAG_FILE 0x00000001
#define DIRFLAG_UNUSED 0x00000002
#define DIRFLAG_SPECIAL 0x00000004
#define AVGDIRECTORYENTRIES 50

typedef struct dirEntry
{
	uint64_t Id;
	char name[128];
	uint64_t date;
	uint64_t location;
	uint64_t sizeInBytes;
	uint32_t flags;
}dirEntry, * dirEntry_p;

#ifdef DIRECTORY
void initRootDir (uint64_t startLocation, uint64_t blockSize)
{
	dirEntry_p rootDirBuffer_p;

	uint64_t entrySize = sizeof(dirEntry);
	uint64_t bytesNeeded = AVGDIRECTORYENTRY * entrySize;
	uint64_t blocksNeeded =(bytesNeeded + (blockSize -1)) / blockSize;
	uint64_t actualDirEntries = (blocksNeeded * blockSize) / entrySize;

	printf("for %d entries, we need %llu bytes, each entry is %llu bytes\n", AVGDIRECTORYENTRY, bytesNeeded, entrySize);
	printf("Actual directory entries = %llu\n", actualDirEntries);
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
#endif




// freemap
#ifdef FREEMAP
void flipFreeBlockBit (char * freeBuffer, uint64_t start, uint64_t count)
{
	unsigned char *p;
	uint64_t byteStart;
	uint64_t bitnum;
	unsigned char flipper;

	while (count > 0)
	{
		byteStart = start /8;
		bitnum = start % 8;
		flipper = 0x80;

		if (bitnum >0)
			flipper = flipper >> bitnum;
		printf("Changing bit #%llu of the %llu byte in free block with flipper %X\n", bitnum, byteStart, flipper);

		p = (unsigned char *)freeBuffer + byteStart;
		*p = *p ^ flipper;
		++start;
		--count;		
	}
}


char * initFreemap(uint64_t volumeSize, uint64_t blockSize, uint64_t startPos)
{
	uint64_t blocks = volumeSize / blockSize;
	uint64_t freeBytesNeeded = (blocks / 8) +1;
	uint64_t freeBlocksNeeded = (freeBytesNeeded /blockSize)+1;
	printf("total blocks: %llu; Bytes: %llu; freeBlocksNeeded: %llu\n", blocks, freeBytesNeeded, freeBlocksNeeded);

	char * freeBuffer = malloc (freeBlocksNeeded * blockSize);
	memset (freeBuffer, 0xFF, freeBlocksNeeded, freeBlocksNeeded);

	flipFreeBlockBit (freeBuffer, 0, 1);
	flipFreeBlockBit (freeBuffer, startPos, freeBlocksNeeded);

	LBAwrite(freeBuffer, freeBlocksNeeded, startPos);
	return (freeBuffer);
}

void freemap (uint64_t volumeSize, uint64_t blockSize)
{
	char *freeBuffer =  initFreemap(volumeSize, blockSize, 1);
}
#endif





// main program, writing in block, 
typedef struct Struct       //good sturct
{
	int counter;
	int size;
	char spacer[256-(sizeof(int)*2)];
	char name[]; //value on disk ignored
}Struct, * Struct_p;


int main (int argc, char *argv[])
	{	
	char * filename;
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    
	if (argc > 3)
		{
		filename = argv[1];
		volumeSize = atoll (argv[2]);
		blockSize = atoll (argv[3]);
		}
		
	retVal = startPartitionSystem (filename, &volumeSize, &blockSize);	
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", filename, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);
	memset (buf, 0, blockSize*2);
	strcpy (buf, "Now is the time for all good people to come to the aid of their countrymen\n");
	strcpy (&buf[blockSize+10], "Four score and seven years ago our fathers brought forth onto this continent a new nation\n");
	LBAwrite (buf, 2, 0);
	LBAwrite (buf, 2, 3);
	LBAread (buf2, 2, 0);
	if (memcmp(buf, buf2, blockSize*2)==0)
		{
		printf("Read/Write worked\n");
		}
	else
		printf("FAILURE on Write/Read\n");
		
	free (buf);
	free(buf2); 


	//very good write
	Struct_p pBuf = malloc (blockSize * 2);
	pBuf->counter = 100000;
	pBuf->size = 80000000;	
	strcpy(pBuf->name, "title here");
	
	retVal = LBAwrite(pBuf, 2, 12);
	printf("return %d\n", retVal);
	free(pBuf);

	//very good read
	Struct_p pBuf = malloc (blockSize * 2);
	retVal = LBAread(pBuf, 2, 12);
	
	printf("The counter is %d\n", pBuf->counter);
	printf("The size is %d\n", pBuf->size);
	printf("The name is %s\n", pBuf->name);
	free(pBuf);

	closePartitionSystem();
	return 0;	
	}
	
