/*
A complex if conditional flow (with the condition in the global scope) that always results in a pointer dereference
*/

#include <stdbool.h>

int number = 50;

int main() {
    int* ptr = 0;
    if (number > 25) {
        int value = *ptr;
    }
}