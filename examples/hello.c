#include <stdio.h>

// Forward declarations
void simple_error_with_local_variable();

struct GreatStruct {
    int f;
};

int main(int argc, char** args) {
    simple_error_with_local_variable();
}

void simple_error_with_local_variable() {
    struct GreatStruct *ptr;
    ptr = NULL;

    ptr->f = 77; // ! ERROR !
}
