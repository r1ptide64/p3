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
	if (sizeOfRegion % pageSize)
		sizeOfRegion += pageSize - (sizeOfRegion % pageSize);
	void *regionPtr = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE,
						   MAP_PRIVATE, fd, 0);
	close(fd);
	if (regionPtr == MAP_FAILED)
		return NULL;
	head = regionPtr;
	head->size = sizeOfRegion;
	head->next = NULL;
	return regionPtr;
}

void *GetDataPointer(Block *block) {
	return block + 1;
}

void *GetNextAvailableAddress(Block *block) {
	if (block == head)
		return GetDataPointer(block);
	void *tmp = GetDataPointer(block);
	return tmp + block->size;
}

Block *MakeNewBlock(Block *currBlock, int size) {
	Block *retBlock = GetNextAvailableAddress(currBlock);
	void *lastNeededDataAddress = GetDataPointer(retBlock) + size;
	void *lastAvailableDataAdress;
	if (currBlock->next == NULL)
		lastAvailableDataAdress = (void *)head + head->size;
	else
		lastAvailableDataAdress = (void *)(currBlock->next);
	if (lastNeededDataAddress <= lastAvailableDataAdress)
		return retBlock;
	else
		return NULL;
}

void *Mem_Alloc(int size) {
	Block *currBlock = head;
	Block *newBlock = NULL;
	while (currBlock != NULL) {
		if ( (newBlock = MakeNewBlock(currBlock, size)) != NULL ) {
			break;
		}
		currBlock = currBlock->next;
	}
	if (newBlock == NULL)
		return NULL;
	newBlock->next = currBlock->next;
	currBlock->next = newBlock;
	newBlock->size = size;
	return GetDataPointer(newBlock);
}

int Mem_Free(void *ptr) {
	if (ptr == NULL || head == NULL)
		return -1;
	void *headPointer = (void *)head;
	void *lastDataPointer = headPointer + head->size;
	if (ptr <= headPointer || ptr > lastDataPointer)
		return -1;
	Block *prevBlock = head;
	Block *currBlock = head->next;
	while ( currBlock != NULL) {
		if (GetDataPointer(currBlock) == ptr) {
			prevBlock->next = currBlock->next;
			return 0;
		}
		prevBlock = currBlock;
		currBlock = currBlock->next;
	}
	return -1;
}

void PrintAvailableAddresses(Block *prevBlock, void *currAddr) {
	void *lastAddressUsedByPrevBlock = GetNextAvailableAddress(prevBlock);
	if (currAddr == NULL)
		currAddr = (void *)head + head->size;
	if (lastAddressUsedByPrevBlock < currAddr)
		printf("%p - %p\n", lastAddressUsedByPrevBlock, currAddr - 1);
}

void Mem_Dump() {
	Block *prevBlock = head;
	Block *currBlock;
	while (prevBlock != NULL) {
		currBlock = prevBlock->next;
		PrintAvailableAddresses(prevBlock, (void *)currBlock);
		prevBlock = currBlock;
	}
}
