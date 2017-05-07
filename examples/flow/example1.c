/*
An if conditional flow that always results in a pointer dereference
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    if (true) {
        int value = *ptr;
    }
}