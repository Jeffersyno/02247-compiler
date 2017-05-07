/*
A while flow that always results in a pointer dereference
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    while (true) {
        int value = *ptr;
    }
}