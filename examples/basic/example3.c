/*
Make a null pointer and then assign it to another pointer. After, dereference both.
*/

int main() {
    int *ptr1 = 0;
    int *ptr2 = ptr1;
    int value1 = *ptr1;
    int value2 = *ptr2;
}
