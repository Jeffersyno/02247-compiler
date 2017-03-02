
void example8() {
    int value = 5;
    int *ptr1 = 0;
    int **ptr2 = &ptr1;
    ptr1 = &value;
    **ptr2;
}
