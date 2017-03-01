
#define NULL 0

/*
 * This file list a number of ways to dereference a pointer value.
 * The bitcode for this file can give us insight into the type of instructions
 * we have to worry about.
 */

void deref1() {
    int *ptr = NULL;
    int wonderful_value = *ptr; // error
}

struct Struct1 { int value; };
void deref2() {
    struct Struct1 *s = NULL;
    int value = s->value; // error
}

void deref3() {
    int value = 5;
    int *ptr = &value;
    ptr = NULL;
    value = *ptr; // error
}

void deref4() {
    int arr[] = {1,2,3};
    int *arr_ptr = &arr[2];
    int value = *arr_ptr; // fine, 3 expected
}
