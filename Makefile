.DEFAULT: joeb
joeb:
	gcc -c -fpic mem.c -Wall -Werror
	gcc -shared -o libmem.so mem.o