/*
Dereference a multiple level pointer without getting to null. Should not fail
*/

int main() {
    int *ptr1 = 0;
    int **ptr2 = &ptr1;
    int ***ptr3 = &ptr2;
    int ****ptr4 = &ptr3;

    int* ptr5 = ***ptr4;
}
