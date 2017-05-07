/*
A complex if-else conditional flow that always results in a pointer dereference in the else closure
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    int number = 0;
    if (number > 25) {
        int value = *ptr;
    } else {
        int value = *ptr;
    }
}