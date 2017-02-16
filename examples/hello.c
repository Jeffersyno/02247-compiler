#include <stdio.h>

// Forward declarations
void simple_error_with_local_variable();

struct GreatStruct {
    int value;
};

int main(int argc, char** args) {
    float f = 0.23f;

    struct GreatStruct *ptr;
    ptr = NULL;
    ptr = 0;
    //ptr = 1-1; // generates warning: expression which evaluates to zero treated as a null pointer constant of type

    ptr->value = 77; // ! ERROR !
    struct GreatStruct local = *ptr; // ! ERROR !
}
