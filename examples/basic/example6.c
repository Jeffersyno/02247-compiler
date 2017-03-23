// Dereference a multiple level pointer, but from one of the intermediate pointers
int main() {
    int *ptr1 = 0;
    int **ptr2 = &ptr1;
    int ***ptr3 = &ptr2;
    int ****ptr4 = &ptr3;

    int value = **ptr2;
}
