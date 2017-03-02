int* get_ptr();

void example9() {
    int *ptr = 0;
    while (!(ptr = get_ptr())) {}
    *ptr;
}
