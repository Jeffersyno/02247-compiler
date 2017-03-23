#include <stdio.h>

struct GreatStruct {
    int value;
};

int main() {
    struct GreatStruct *ptr;
    ptr = NULL;
    ptr = 0;
    //ptr = 1-1; // generates warning: expression which evaluates to zero treated as a null pointer constant of type

    ptr->value = 77; // ! ERROR !
    struct GreatStruct local = *ptr; // ! ERROR !
}
