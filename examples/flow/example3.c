/*
A complex if conditional flow that always results in a pointer dereference
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    int number = 50;
    if (number > 25) {
        int value = *ptr;
    }
}