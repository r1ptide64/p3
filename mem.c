#include "mem.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

static void *regionPtr;

typedef struct _block {
	int size;
	void *next;
} Block;

Block *head;

void *Mem_Init(int sizeOfRegion) {
	int fd = open("/dev/zero", O_RDWR);
	int pageSize = getpagesize();
	sizeOfRegion = (1 + (sizeOfRegion/pageSize)) * pageSize;
	regionPtr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	close(fd);
	if (regionPtr == MAP_FAILED)
		return NULL;
	head = regionPtr;
	head->size = sizeOfRegion;
	return regionPtr;
}

//Block *mkNewBlock(Block *prevBlock, int size) {
//	Block *retBlock = prevBlock + sizeof(Block) + 1;
//	if (prevBlock != head)
//		retBlock += prevBlock->size;
//	prevBlock->next = retBlock;
//	retBlock->size = size;
//	retBlock->next = NULL;
//	return retBlock;
//}
//
void *getDataPtr(Block block) {
	return block + sizeof(Block) + 1;
}

void *getLastAddressUsedByBlock(Block block) {
	return getDataPtr(block) + block->size;
}
//
//int willNewBlockFit(Block prevBlock) {
//	if (prevBlock->next != NULL) {
//
//	}
//	return 0;
//}

int canSqueeze(Block currBlock, int size) {
	if (currBlock->next == NULL)
		return 0;
	return getLastAddressUsedByBlock(currBlock) + size + sizeof(Block) + 1 < currBlock->next;
}

void *Mem_Alloc(int size) {
	Block *newBlock = NULL;
	Block *currBlock = head->next;
	while (currBlock != NULL) {
		if (!canSqueeze(currBlock, size)) {
			currBlock = currBlock->next;
			continue;
		}
		newBlock = getLastAddressUsedByBlock(currBlock);
		newBlock->next = currBlock->next;
		currBlock->next = newBlock;
		newBlock->size = size;
	}
	if (newBlock == NULL && )
	return newBlock;
}

int main() {
	Mem_Init(100);
	int i;
	printf("sizeof(Block): %lu\n\n", sizeof(Block));
	printf("head: %p\n", head);
	for (i=1; i < 10; i++) {
		printf("block %d: %p\n", i, Mem_Alloc(100));
	}
	exit(0);
}
