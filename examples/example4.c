
void example4() {
    int *ptr1 = 0;
    int **ptr2 = &ptr1;
    int ***ptr3 = &ptr2;
    int ****ptr4 = &ptr3;

    int value = *(*(*(*(ptr4))));
}
