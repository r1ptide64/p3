#include "mem.h"
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>

void *Mem_Init(int sizeOfRegion) {

	int pageSize = getpagesize();
//	printf("size of region: %d\n", sizeOfRegion);
//	printf("page size: %d\n", pageSize);
	sizeOfRegion = (1 + (sizeOfRegion/pageSize)) * pageSize;
//	sizeOfRegion += (getpagesize() % sizeOfRegion);
//	printf("new size: %d\n", sizeOfRegion);
	void *retVal = mmap(NULL, sizeOfRegion, PROT_READ | PROT_WRITE, MAP_ANONYMOUS);
	if (retVal == MAP_FAILED)
		return NULL;
	return retVal;
}

int main() {
	int i = 5;
//	int j = 3;
//	printf("i/j: %d\n", i/j);
//	printf("j/i: %d\n", j/i);
//	return(0);
	for (i = 1; i < 20; i++) {
		Mem_Init(i);
	}
}
