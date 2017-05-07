/*
Malloc example. When there is not enough memory, it returns NULL
*/

#include <stdlib.h>

int main() {
    int* pointer=(int*)malloc(100000000);
    int value = *pointer;
}
