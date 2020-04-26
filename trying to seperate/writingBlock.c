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

// main program, writing in block
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
	
