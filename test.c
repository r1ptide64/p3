#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

int main2() {
	printf("%p\n", Mem_Init(40960));
	void *ptr = Mem_Alloc(1024);
	printf("%p\n", ptr);
	void *tmp;
	while ((tmp = Mem_Alloc(1)) != NULL)
		;
	printf("%d\n",Mem_Free(ptr));
	printf("%p\n",Mem_Alloc(10));
	printf("%p\n",Mem_Alloc(10));
	return 0;
}
