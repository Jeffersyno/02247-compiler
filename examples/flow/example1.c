/*
An if conditional flow that never results in a pointer dereference
*/

#include <stdbool.h>

int main() {
    int* ptr = 0;
    if (false) {
        int value = *ptr;
    }
}
