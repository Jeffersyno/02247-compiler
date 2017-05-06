/*
A complex if conditional flow that always results in a pointer dereference
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    while (ptr != 0) {
        int value = *ptr;
    }
}
