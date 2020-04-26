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


// freemap
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
