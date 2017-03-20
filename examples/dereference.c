#include <stdio.h>

int main() {
	int number = 5;
	int* ptr1 = &number;
	printf("Pointer address = %p\n", (void *) ptr1); //0x7fff54d4ab0c
	printf("Now we make the pointer null\n");
	ptr1 = 0;
	printf("Pointer address = %p\n", (void *) ptr1); //0x00
}
