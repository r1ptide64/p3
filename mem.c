#include "mem.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct _block {
	int size;
	void *next;
} Block;

static Block *head;

void *Mem_Init(int sizeOfRegion) {
	int fd = open("/dev/zero", O_RDWR);
	int pageSize = getpagesize();
	sizeOfRegion = (1 + (sizeOfRegion/pageSize)) * pageSize;
	void *regionPtr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	close(fd);
	if (regionPtr == MAP_FAILED)
		return NULL;
	head = regionPtr;
	head->size = sizeOfRegion;
	return regionPtr;
}
void *GetDataPtr(Block *block) {
	return block + sizeof(Block);
}

void *GetNextBlockAddr(Block *block) {
	if (block == head)
		return GetDataPtr(block);
	return GetDataPtr(block) + block->size;
}

int WillNewBlockFit(Block *currBlock, int size) {
	void *lastDataAddr = GetDataPtr(GetNextBlockAddr(currBlock));
	if (currBlock->next == NULL)
		return lastDataAddr < head + head->size;
	else
		return lastDataAddr < currBlock->next;
}

void *Mem_Alloc(int size) {
	Block *currBlock = head;
	Block *newBlock = NULL;
	while (currBlock->next != NULL) {
		if (WillNewBlockFit(currBlock, size)) {
			break;
		}
		currBlock = currBlock->next;
	}
	if (!WillNewBlockFit(currBlock, size)) {
		return NULL;
	}
	Block *newBlock = GetLastAddressUsedByBlock(currBlock) + 1;
	newBlock->next = currBlock->next;
	currBlock->next = newBlock;
	newBlock->size = size;
	return GetDataPtr(newBlock);
}

int main() {
	Mem_Init(20000);
	int i;
	printf("head: %p\n", head);
	for (i=1; i < 10; i++) {
		printf("block %d: %p\n", i, Mem_Alloc(100));
	}
	Block *currBlock = head;
	i = 0;
	while (currBlock!=NULL) {
		printf("===block %d===\n", i);
		printf("blockAddr: %p\n", currBlock);
		printf("block->size: %d\n", currBlock->size);
		printf("block->next: %p\n", currBlock->next);
		printf("dataptr: %p\n", GetDataPtr(currBlock));
		printf("\n");
		i++;
		currBlock = currBlock->next;
	}
	exit(0);
}
