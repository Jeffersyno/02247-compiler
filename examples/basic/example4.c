/*
Assign null to a pointer and assign it a new valid value before dereference. Should not fail.
*/

int main() {
    int* ptr = 0;
    int value1 = 5;
    ptr = &value;
    int value2 = *ptr;
}
