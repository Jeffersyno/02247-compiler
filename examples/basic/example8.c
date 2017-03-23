// Dereference a second level pointer. Its first level pointer was assigned null, and then assigned a valid value. Should not fail.
int main() {
    int *ptr1 = 0;
    int **ptr2 = &ptr1;
    int value = 5;
    ptr1 = &value;
    int value3 = **ptr2;
}
