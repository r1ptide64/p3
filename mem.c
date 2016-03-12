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

int WillNewBlockFit(Block *currBlock, int size) {
	Block *tmpBlock = GetNextAvailableAddress(currBlock);
//	printf("tmpBlock: %p\n", tmpBlock);
	void *lastNeededDataAddress = GetDataPointer(tmpBlock) + size;
//	printf("lastNeededDataAddress: %p\n", lastNeededDataAddress);
	void *lastAvailableDataAdress;
	if (currBlock->next == NULL)
		lastAvailableDataAdress = (void *)head + head->size;
	else
		lastAvailableDataAdress = (void *)(currBlock->next);
//	printf("lastAvailableDataAddress: %p\n", lastAvailableDataAdress);
	return lastNeededDataAddress <= lastAvailableDataAdress;
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
	newBlock = GetNextAvailableAddress(currBlock);
	newBlock->next = currBlock->next;
	currBlock->next = newBlock;
	newBlock->size = size;
	return GetDataPointer(newBlock);
}

int Mem_Free(void *ptr) {
//	printf("in mem free!\n");
	if (ptr == NULL || head == NULL)
		return -1;
//	printf("line 75!\n");
	void *headPointer = (void *)head;
	void *lastDataPointer = headPointer + head->size;
	if (ptr <= headPointer || ptr > lastDataPointer)
		return -1;
//	printf("line 79!\n");
	Block *prevBlock = head;
	Block *currBlock = head->next;
//	printf("about to start loop!");
	while ( currBlock != NULL) {
//		printf("currBlock=%p\n", currBlock);
		if (GetDataPointer(currBlock) == ptr) {
			prevBlock->next = currBlock->next;
			return 0;
		}
		prevBlock = currBlock;
		currBlock = currBlock->next;
	}
	return -1;
}

void Mem_Dump() {
	Block *prevBlock = head;
	Block *currBlock = head->next;
	void *currAddr;
	void *lastAddressUsedByPrevBlock;
	while (currBlock != NULL) {
		currAddr = currBlock;
		lastAddressUsedByPrevBlock = GetNextAvailableAddress(prevBlock);
		if (lastAddressUsedByPrevBlock < currAddr)
			printf("%p - %p\n", lastAddressUsedByPrevBlock, currAddr - 1);
		prevBlock = currBlock;
		currBlock = currBlock->next;
	}
	lastAddressUsedByPrevBlock = GetNextAvailableAddress(prevBlock);
	currAddr = (void *)head + head->size;
	if (lastAddressUsedByPrevBlock < currAddr)
		printf("%p - %p\n", lastAddressUsedByPrevBlock, currAddr - 1);
}

int main() {
	Mem_Init(8000);
	printf("===initial dump===\n");
	Mem_Dump();
	void *tmp[16];
	int i;
	for (i = 0; i < 16; i++) {
		tmp[i] = Mem_Alloc(32);
	}
	printf("===post-malloc dump===\n");
	Mem_Dump();
	for (i = 0; i < 16; i += 3) {
		Mem_Free(tmp[i]);
	}
	printf("===post-free dump===\n");
	Mem_Dump();
}
