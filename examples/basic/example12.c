/*
Dereference a multiple level pointer, but the null is in one of the intermediate pointers
*/

int main() {
    int value1 = 5;
    int *ptr1 = &value1;
    int **ptr2 = &ptr1;
    int ***ptr3 = 0;
    int ****ptr4 = &ptr3;

    int value2 = **ptr2;
    int value3 = ***ptr3;
    int value4 = ****ptr4;
}